# Ground Commands

This document defines the structure of transactions between the OBC and the ground station through the OpenLST. Both command uplink and status/telemetry downlink will follow this format. The commands are the same whether sent through the OpenLST or sent directly to the OBC.

## Format

At the UART interface, all commands will be inside of an OpenLST packet of type ASCII (0x11). See OpenLST documentation for more details. The OpenLST packet allows a maximum data payload of 245 bytes, so that is the largest possible ground command packet.

Ground commands will start with a command opcode byte which will dictate the format of the rest of the command. The opcodes are grouped as follows:

* 0x00-0x0F - misc (ack, ping, etc)
* 0x10-0x1F - telemetry
* 0x20-0x2F - configuration and settings
* 0x30-0x3F - updates
* 0x40-0x7F - reserved for future high level commands
* 0x80-0x9F - hardware access
* 0xA0-0xBF - driver access
* 0xC0-0xFF - reserved for future access to internals

All multi byte fields have the least significant byte first, ie little endian if bytes are transmitted in the order they appear in memory.

### Future Format Changes

Planned but not yet implemented format changes

Command authentication support:

| Field   | Size |
|---------|------|
| Hash    | 32   |
| Counter | 4    |
| Unused  | 3    |
| Opcode  | 1    |

* Limit command data size to 205 bytes to allow room for command authentication
* Separate openLST packet types for uplink and downlink packets (no authentication on downlink)

## Commands

For each command format, an OPCODE field is implied at the start, for example:

| Field            | Size |
|------------------|------|
| OPCODE           | 1    |
| *rest of packet* | N    |

If no packet structure is listed for a command, then it will contain no data after the opcode.

The sizes listed for commands are the total sizes, including the opcode but not including the OpenLST header.

### 0x00 - ACK

| Field | Size |
|-------|------|
| NACK  | 1    |

If NACK is set to 0, response is ACK. If 1, response is NACK. Generally, ACK is sent after a command is successful and NACK if unsuccessful.

### 0x01 - PING

| Field | Size |
|-------|------|
| DATA  | N    |

If a ping is received, the receiver will respond with an ACK message that has the same sequence ID. The rest of contents of the message will be repeated with no changes.

### 0x02 - MSG

| Field   | Size |
|---------|------|
| LEVEL   | 1    |
| MESSAGE | N    |

Contains an ASCII message intended to be read by humans, such as errors, warnings, or log messages.

TODO: implement different log levels

### 0x03 - REBOOT

Reboots OBC immediately.

TODO: would it be useful to add a delay?

### 0x10 - TELEM_REQ

Message is empty.

### 0x11 - TELEM_RESP

| Field           | Size |
|-----------------|------|
| UPTIME_MS       | 4    |
| TELEM_AGE_MS    | 2    |
| V_MB_BATT_MV    | 2    |
| V_MB_4V2_MV     | 2    |
| V_MB_3V3_MV     | 2    |
| V_LST_4V2_MV    | 2    |
| V_LST_3V3_MV    | 2    |
| I_MB_3V3_OBC_MA | 2    |
| I_MB_3V3_GPS_MA | 2    |
| I_MB_3V3_LST_MA | 2    |
| I_MB_4V2_LST_MA | 2    |
| T_LST0_CC       | 2    |
| T_LST1_CC       | 2    |
| T_OBC0_CC       | 2    |
| T_OBC1_CC       | 2    |
| T_RP2040_CC     | 2    |
| T_CC1110_CC     | 2    |
| ... | ... |

UPTIME_MS is the number of milliseconds since boot. TELEM_AGE_MS is the number of milliseconds since the telemetry was updated, ie UPTIME_MS(now) - UPTIME_MS(telem updated). UPTIME_MS(now) is measured when the response packet is being assembled, and UPTIME_MS(telem updated) is updated when the telemetry packet is updated.

Field name prefix indicated type: V_ means voltage, I_ means current, T_ means temperature. Suffix indicates units: _MS is milliseconds, _MV is millivolts, _MA is milliamps, _CC is centiCelsius.

### 0x80 - GPIO

| Field  | Size |
|--------|------|
| PIN    | 4    |
| PIN_OP | 1    |

PIN - bitmask of pins to apply operation to. Pin 0 is the LSB of the first byte, 31 is MSB of last byte.

PIN_OP

* 0x00 - Set pin function to null
* 0x01 - Set pin function to GPIO (calls init function so pin will also be set as input)
* 0x02 - Set pin mode to input
* 0x03 - Set pin mode to output
* 0x04 - Set pin high
* 0x05 - Set pin low
* 0xFF - Read pin state (ignores pin field, will return GPIO_STATE message with all pins)

TODO: other pin features (like pullup/pulldown)

### 0x81 - GPIO_STATE

| Field     | Size |
|-----------|------|
| PIN_MODE  | 4    |
| PIN_STATE | 4    |

GPIO_STATE is only to be returned in response to certain GPIO commands. The OBC will ignore any GPIO_STATE packets it receives.

PIN_MODE contains the state of each pin. For each pin, a 1 represents output and 0 input.

PIN_STATE is the actual state of the pin, regardless of whether it's an input or an output (I think, the datasheet doesn't actually specifically say this).

### 0x82 - ADC_START

| Field   | Size |
|---------|------|
| CHANNEL | 1    |

Initiates an ADC read. CHANNEL is a bit field to select which channel to measure, ie 0b00000011 selects channels 0 and 1. There are 5 ADC channels total (4 external inputs and 1 internal temperature sensor), see RP2040 datasheet for details.

An ADC_READING packet will be returned with the results of the conversion.

### 0x83 - ADC_READING

| Field     | Size |
|-----------|------|
| DATA[i]   | 2    |
| DATA[i+1] | 2    |
| ...       | ...  |

Results of an ADC reading. The DATA field is repeated depending on how many channels were selected in the ADC command. The DATA fields are in increasing order of index, but may not necessarily be consecutive, depending on which channels were selected.

### 0xA0 - FLASH_CMD

| Field    | Size     |
|----------|----------|
| CMD      | 1        |
| *Varies* | *Varies* |

The CMD field determines which flash operation will be performed. The contents of the rest of the message depends on the specific operation.

The possible values for CMD are:

* 0x00 - Read
* 0x01 - Program
* 0x02 - Erase
* 0x03 - Read unique ID
* TODO: commands for status and config registers

The subsections below describe the message contents for each specific operation.

#### 0x00 - READ

Size: 6

| Field | Size |
|-------|------|
| ADDR  | 3    |
| SIZE  | 1    |

The READ operation performs a flash read and returns the data received. The ADDR field indicates the address to start the read. The address can be any value, there are no alignment requirements. SIZE indicates the number of bytes to read. If it is greater than the maximum value of TBD bytes, only TBD bytes will be returned.

#### 0x01 - PROGRAM

Size: 5 + N

| Field | Size |
|-------|------|
| ADDR  | 3    |
| DATA  | N    |

The PROGRAM operation performs a page program. ADDR is the address to start programming at and DATA is the data to be programmed. DATA can be any length up to the maximum allowed size of TBD bytes. ADDR does not have alignment requirements, however if the program operation crosses a page boundary (pages are 256 bytes) it will wrap around to the beginning of the page.

For example, a program operation starting at 0x0F0 and containing 0x20 bytes will program bytes 0x0F0-0x0FF and then bytes 0x000-0x00F. The leading zeros are left off for simplicity; addresses are normally 24 bits long.

#### 0x02 - ERASE

Size: 6

| Field | Size |
|-------|------|
| ADDR  | 3    |
| SIZE  | 1    |

Erase a sector or block. ADDR refers to the address of the block being erase. SIZE indicates the erase size to use (sector or block).

Valid values for SIZE:
* 0x00 - sector (4 kB)
* 0x01 - block (32 kB)
* 0x02 - block (64 kB)

#### 0x03 - UNIQUE_ID

Returns the flash unique ID.

### 0xA1 - FLASH_RESP

| Field    | Size |
|----------|------|
| CMD_RESP | 1    |

CMD_RESP can be:
* 0x00 - READ response
* 0x01 - UNIQUE_ID response

#### 0x00 - READ Response

Size: 2 + N

| Field | Size |
|-------|------|
| DATA  | N    |

#### 0x01 - UNIQUE_ID Response

Size: 10

| Field     | Size |
|-----------|------|
| UNIQUE_ID | 8    |
