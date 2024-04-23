# Updater/Bootloader Specification

## File Structure

WIP, but general idea is:

* CMakeLists.txt configured to produce both a bootloader and application image
* Bootloader and application share some code (settings, drivers, etc), but kept to a minimum
* `bl_main.c` file contains bootloader main function
  * `bl_` files indicate it's only used in the bootloader (except `bl_common.h` which is specifically for things that need to be shared between bootloader and application)

## Flash Layout

Page size is 256 bytes, min erase size is 4 kB.

When changing the flash layout, also update `bl_common.h`.

| Start     | End       | Size     | Access | Description                |
| --------- | --------- | -------- | ------ | -------------------------- |
| 0x00_0000 | 0x00_00FF | 256 B    | R      | boot2, pico-sdk bootloader |
| 0x00_0100 | 0x00_7FFF | 31.75 kB | RX     | Our bootloader             |
| 0x00_8000 | 0x00_8FFF | 4 kB     | RW     | Program header             |
| 0x00_9000 | 0x00_9FFF | 4 kB     | RW     | Update header              |
| 0x00_A000 | 0x07_FFFF | 472 kB   | RW     | Misc config data           |
| 0x08_0000 | 0x0F_FFFF | 512 kB   | RWX    | Application slot           |
| 0x10_0000 | 0x17_FFFF | 512 kB   | RW     | Update staging slot        |

In each section below, the addresses are relative to the start of the section.

Much of the space in the program header and update header is reserved and unused because they need to be in separate erase blocks, which are 4 kB in size.

### Application Header

| Start  | End    | Size | Description |
| ------ | ------ | ---- | ----------- |
| 0x0000 | 0x0003 | 4 B  | Size        |
| 0x0004 | 0x0007 | 4 B  | CRC32       |
| 0x0008 | 0x0FFE | ...  | Reserved    |
| 0x0FFF | 0x0FFF | 1 B  | Valid       |

Valid byte:
* 0b1111_1111 - default erase state, assume everything is invalid
* 0b1111_1110 - application slot has been erased but not written to yet
* 0b1111_1100 - application header is written, application may be partially written
* 0b0000_0000 - application slot is valid

### Update Header

| Start  | End    | Size  | Description   |
| ------ | ------ | ----- | ------------- |
| 0x0000 | 0x0003 | 4 B   | Size          |
| 0x0004 | 0x0007 | 4 B   | CRC32         |
| 0x0008 | 0x03FF | ...   | Reserved      |
| 0x0400 | 0x05FF | 512 B | Update status |
| 0x0600 | 0x0FFE | ...   | Reserved      |
| 0x0FFF | 0x0FFF | 1 B   | Valid         |

In the update status, each bit corresponds to a half page (128 bytes) in the update slot. If the bit is a 1, the half page has not been written to yet. If it's a 0, the page has been written. 512 bytes * 8 bits/byte * 128 bytes/bit = 512 kB.

Valid byte:
* 0b1111_1111 - default erase state, assume slot is invalid
* 0b1111_1110 - update slot erased but not written to
* 0b1111_1100 - update header valid, some update data may be written
* 0b0000_0000 - update ready to be applied

## Update Process

1. Receive update initialization command (includes size, CRC)
   1. Erase update staging slot
   2. Erase update header
   3. Populate header with size, CRC
2. Ground requests status to see if update initialization was successful
3. Receive update packets, each containing a 128 byte chunk and corresponding address (with address 0 being the start of the image, not the flash address 0)
   1. Chunks are 128 byte aligned and addresses have bottom 7 bits truncated (because they will always be 0)
   2. Chunk is written to flash and read back to verify
   3. Set bit in update status to confirm that update is written successfully
4. After all chunks are sent, a status check will be requested
   1. Status check returns whether CRC of update matches, how many half pages are unwritten, and a list of addresses of half pages that are unwritten. The list of addresses may be larger than the available space in the packet, so a subset is returned. Subsequent status checks will return the next N addresses, and the list will overflow back to 0 when the end is reached. No assumptions can be made about the ordering of addresses other than that they will be unique within a single status packet.
5. Receive command to apply update
   1. Send ACK confirming update will be applied. After this, no further packets will be sent and no received packets will be processed until the update is applied.
   2. All subsequent steps will run in a function stored in RAM and with interrupts turned off because no external events will be more important than finishing the update.
   3. Erase application slot and header
   4. Program header with size and CRC of new update (so that power loss after this point will indicate an incomplete update)
   5. Copy entire contents of update slot to application slot
   6. Verify contents of application slot against CRC in header
      1. If no match, erase and retry 2 more times, then go to bootloader for PIB recover
   7. Write valid byte to header to indicate success
   8. Write a magic value to the watchdog scratch registers to indicate an update was successfully applied
   9. Restart to boot into new image

TBD:
* Do we send an ACK after each chunk? The writing/verifying process will probably take longer than the time it takes to send a single packet (especially on the PIB interface), so some kind of flow control is needed. Maybe request an ACK every N (4-16ish) packets and don't send any packets until it's received (or a timeout is reached)?

## Potential Faults

* Power loss during a write or erase
  * Page will be corrupted, need to restart the entire update process
  * Last byte in application header and update header is a valid byte, only written after the section is successfully programmed and verified. If this is not set, it would indicate an update process was interrupted.
  * If this happens while writing a chunk, the next time the chunk is written to it will not pass verification, see below.
* Power loss after a chunk is written but before it is verified
  * The chunk will be assumed to be all 1s (the erased state of flash). Writing the same data to it will not change anything, so it will still pass verification.
* Power loss while applying update
  * Application code is corrupted and must be recovered by the bootloader through the USNA PIB interface
* Chunk doesn't pass verification
  * Need to restart update process
  * TODO: if this is a big issue, we should add more error checking and robustness. If it happens too often we will never be able to actually complete an update.
* CRC doesn't match after update is entirely written
  * Need to restart update process

Several potential faults result in the need to restart the entire update process, which is time consuming due to the need to erase and reprogram large sections of flash. If it happens often enough, we should add more error checking and robustness so that any given update has a very high chance of succeeding on the first try.

## Bootloader

The bootloader is intended to be as simple as possible. It should not use DMA, interrupts, or any other complex feature.

### Boot Process

1. Don't boot if any of the conditions occur:
  1. Application header valid byte isn't valid
  2. Application CRC doesn't match
  3. Watchdog scratch matches bootloader magic
2. If booting, wait for 1s then boot
  1. If ping command received, set timeout to 5s and reset after each command received
3. While idle, wait for and process commands
4. When ready to boot:
   1. Set VTOR to application slot location
   2. Jump to reset vector from VTOR

#### Recovery

If an application image needs to be written by the PIB:

1. Bootloader doesn't boot application due to one of the reasons above
2. PIB pings the bootloader to reset the watchdog, and will continue to do so at least every 5s to prevent it from resetting. If this is not done, it may be reset during a program or erase, which could corrupt data (but will not corrupt the bootloader itself).
3. PIB sends BL_ERASE command to erase application slot and header
4. PIB sends BL_HEADER command to write application header with size and CRC
5. PIB sends BL_WRITE commands to write image
6. PIB gets status using BL_STATUS_REQ
   1. If CRC doesn't match, attempt writing entire image again

### Commands

Reuses the [OpenLST command protocol](https://github.com/seds-umd/openlst-software/tree/dev?tab=readme-ov-file#uart-protocol). HWID and system commands are ignored. Sequence number is respected, so replies to command should use the same sequence number and non-reply messages should start from a random sequence number and increment after each non-reply message.

Commands are also similar to the OpenLST bootloader. If no message fields are describes, the message must be empty.

If a command has no reply message defined, it will return an ACK/NACK message upon completion. No other command can be sent until then. If no ACK/NACK is received, the board must be power cycled.

#### 0x00 - BL_PING

Pings bootloader. Returns BL_ACK and resets bootloader watchdog to 5s (without a ping it's set to 1s).

#### 0x01 - BL_ACK

ACK returned by bootloader.

#### 0x02 - BL_WRITE

Writes a section of data to the application image.

| Field | Size |
| ----- | ---- |
| ADDR  | 2    |
| DATA  | 128  |

ADDR is the address of the data to write. The address is relative to the start of the application slot. The bottom 7 bits of the address are not included.

DATA is the data to write to a given address.

#### 0x03 - BL_STATUS_REQ

Request bootloader status. If in the process of calculating the CRC for the status it matches the CRC written in the header, the header valid byte will be written to indicate a valid application.

#### 0x04 - BL_STATUS

Status returned by bootloader.

| Field        | Size |
| ------------ | ---- |
| SIZE         | 4    |
| CRC_EXPECTED | 4    |
| CRC_ACTUAL   | 4    |
| MATCH        | 1    |

SIZE is the size of the update in bytes, read from application header.

CRC_EXPECTED is the expected CRC, read from the application header.

CRC_ACTUAL is the CRC calculated across the application slot from the given size. If SIZE is greater than 16 MB (the size of the flash used), the CRC is not calculated and this field is set to all 0s.

MATCH is 1 if the CRCs match and 0 otherwise. This will also be 0 if SIZE is invalid.

#### 0x05 - BL_HEADER

Write the application header.

| Field | Size |
| ----- | ---- |
| SIZE  | 4    |
| CRC   | 4    |

SIZE is the size of the image to be written, in bytes.

CRC is the checksum of the image.

#### 0x06 - BL_FORCE_BOOT

Force the bootloader to boot the current image, ignoring any checks.

#### 0x0C - BL_ERASE

Erase the entire application slot and header.

## Misc Notes

* CRC32 uses IEEE802.3 polynomial (so it can use the CRC hardware in the RP2040 DMA) - 0x04C11DB7 initialized to all 1s
* Before starting the update process, zeros are appended to the end of the image to align it to 128 bytes. This means there will be no half filled chunks and simplifies everything.
* 8192 chunks to fill a slot - addresses must be 16 bits
* Flash erases and programs will block and prevent access to flash. Any interrupts must be executed from RAM and not access any flash.
* Watchdog magics:
  * scratch[0] = 0x5aede6a9 if last reboot was due to update
  * scratch[0] = 0xacb09b3c if we want to stay in bootloader
