# Updater/Bootloader Specification

## Flash Layout

Page size is 256 bytes, min erase size is 4 kB.

| Start       | End         | Size     | Access | Description                |
| ----------- | ----------- | -------- | ------ | -------------------------- |
| 0x0000_0000 | 0x0000_00FF | 256 B    | R      | boot2, pico-sdk bootloader |
| 0x0000_0100 | 0x0000_3FFF | 15.75 kB | RX     | Our bootloader             |
| 0x0000_4000 | 0x0000_4FFF | 4 kB     | RW     | Program header             |
| 0x0000_5000 | 0x0000_5FFF | 4 kB     | RW     | Update header            |
| 0x0000_6000 | 0x000F_FFFF | 1000 kB  | RW     | Misc config data           |
| 0x0010_0000 | 0x001F_FFFF | 1024 kB  | RWX    | Application slot           |
| 0x0020_0000 | 0x002F_FFFF | 1024 kB  | RW     | Update staging slot        |

In each section below, the addresses are relative to the start of the section.

Much of the space in the program header and update header is reserved and unused because they need to be in separate erase blocks, which are 4 kB in size.

### Program Header

* CRC and size of currently loaded application image

| Start  | End    | Size | Description |
| ------ | ------ | ---- | ----------- |
| 0x0000 | 0x0003 | 4 B  | Size        |
| 0x0004 | 0x0007 | 4 B  | CRC32       |
| 0x0008 | 0x0FFD | ...  | Reserved    |
| 0x0FFE | 0x0FFF | 1 B | Valid |

### Update Hetadata

Addresses are relative to the start of the 

| Start  | End    | Size | Description   |
| ------ | ------ | ---- | ------------- |
| 0x0000 | 0x0013 | 20 B | Git hash      |
| 0x0014 | 0x0017 | 4 B  | CRC32         |
| 0x0018 | 0x001B | 4 B  | Size          |
| 0x001C | 0x03FF | ...  | Reserved      |
| 0x0400 | 0x07FF | 1 kB | Update status |
| 0x0800 | 0x0FFD | ...  | Reserved |
| 0x0FFE | 0x0FFF | 1 B | Valid |

In the update status, each bit corresponds to a half page (128 bytes) in the update slot. If the bit is a 1, the half page has not been written to yet. If it's a 0, the page has been written. 1024 bytes * 8 bits/byte * 128 bytes/bit = 1 MB.

## Update Process

1. Receive update initialization command (includes size, CRC, git hash)
   1. Erase update staging slot
   2. Erase update header
   3. Populate header with size, CRC, git hash
2. Receive update packets, each containing a 128 byte chunk and corresponding address (with address 0 being the start of the image, not the flash address 0)
   1. Chunks are 128 byte aligned and addresses have bottom 7 bits truncated (because they will always be 0)
   2. Chunk is written to flash and read back to verify
   3. Set bit in update status to confirm that update is written successfully
3. After all chunks are sent, a status check will be requested
   1. Status check returns whether CRC of update matches, how many half pages are unwritten, and a list of addresses of half pages that are unwritten. The list of addresses may be larger than the available space in the packet, so a subset is returned. Subsequent status checks will return the next N addresses, and the list will overflow back to 0 when the end is reached. No assumptions can be made about the ordering of addresses other than that they will be unique within a single status packet.
4. Receive command to apply update
   1. Send ACK confirming update will be applied. After this, no further packets will be sent and no received packets will be processed until the update is applied.
   2. All subsequent steps will run in a function stored in RAM and with interrupts turned off because no external events will be more important than finishing the update.
   3. Erase application slot and header
   4. Program header with size and CRC of new update (so that power loss after this point will indicate an incomplete update)
   5. Copy entire contents of update slot to application slot
   6. Verify contents of application slot against CRC in header
      1. If no match, erase and retry 2 more times, then go to bootloader for PIB recover
   7. Write valid byte to header to indicate success
   8. Restart to boot into new image

TBD:
* Do we send an ACK after each chunk? The writing/verifying process will probably take longer than the time it takes to send a single packet (especially on the PIB interface), so some kind of flow control is needed. Maybe request an ACK every N (4-16ish) packets and don't send any packets until it's received (or a timeout is reached)?

## Potential Faults

* Power loss during a write or erase
  * Page will be corrupted, need to restart the entire update process
  * Last byte in program header and update header is a valid byte, only written after the section is successfully programmed and verified. If this is not set, it would indicate an update process was interrupted.
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

Design philosophy
* Simple as possible
* No DMA, scheduling, any complex functionality

### Function

* Checks valid byte in program header before running code so an incomplete update won't be ran
  * TODO: do we check CRC too? How long will that take?
* If application code is invalid, attempt to complete update copying process. If this fails, tell PIB that we need a new image. PIB will upload new image to complete the update process.
* PIB recovery process will write directly to the application slot because there's no point in using the update slot, but will still use the CRC check and valid byte in the program header

TODO: valid byte indicate whether it's in the middle of a normal update or a PIB update
