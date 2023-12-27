# Ground Commands

This document defines the structure of transactions between the OBC and the ground station through the OpenLST. Both command uplink and status/telemetry downlink will follow this format.

## Format

At the UART interface, all commands will be inside of an OpenLST packet of type ASCII (0x11). See OpenLST documentation for more details. The OpenLST packet allows a maximum data payload of 245 bytes, so that is the largest possible ground command packet.

Ground commands will start with a command opcode byte which will dictate the format of the rest of the command. The opcodes are grouped as follows:

* 0x00-0x0F - misc (nop, ping, etc)
* 0x10-0x1F - telemetry related
* 0x20-0x2F - configuration and settings
* 0x30-0x3F - updates
* 0x40-0x47F - reserved for future high level commands
* 0x80-0x9F - hardware access
* 0xA0-0xBF - driver access
* 0xC0-0xFF - reserved for future access to internals

| Hex | Opcode | Firmware Status | Python Status |
| - | - | - | - |
| 0x00 | NOP | TODO | TODO |
| 0x01 | PING | TODO | TODO |
| 0x80 | UART_CFG | TODO | TODO |
| 0x81 | UART_DATA | TODO | TODO |
| xxx | SPI_CFG | TODO | TODO |
| xxx | SPI_DATA | TODO | TODO |
| xxx | GPIO_CFG | TODO | TODO |



## Commands

### 