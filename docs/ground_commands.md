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

| Hex  | Opcode     | Firmware Status | Python Status | Testing |
|------|------------|-----------------|---------------|---------|
| 0x00 | ACK        | TODO            | TODO          | TODO    |
| 0x01 | PING       | Done            | Done          | Working |
| 0x02 | MSG        | Done            | Done          | Working |
| 0x03 | REBOOT     | Done            | Done          | Working |
| 0x80 | GPIO       | TODO            | TODO          | TODO    |
| 0x81 | GPIO_STATE | TODO            | TODO          | TODO    |

All multi byte fields have the least significant byte first, ie little endian if bytes are transmitted in the order they appear in memory.

## Commands

### 0x00 - ACK

| Field  | Size |
| ------ | ---- |
| OPCODE | 1    |
| NACK   | 1    |

If NACK is set to 0, response is ACK. If 1, response is NACK. Generally, ACK is sent after a command is successful and NACK if unsuccessful.

### 0x01 - PING

| Field  | Size |
| ------ | ---- |
| OPCODE | 1    |

If a ping is received, the receiver will respond with an ACK message that has the same sequence ID. The rest of contents of the message will be repeated with no changes.

### 0x02 - MSG

| Field   | Size |
|---------|------|
| OPCODE  | 1    |
| LEVEL   | 1    |
| MESSAGE | N    |

Contains an ASCII message intended to be read by humans, such as errors, warnings, or log messages.

TODO: implement different log levels

### 0x03 - REBOOT

| Field   | Size |
|---------|------|
| OPCODE  | 1    |

Reboots OBC immediately.

TODO: would it be useful to add a delay?

### 0x80 - GPIO

| Field  | Size |
|--------|------|
| OPCODE | 1    |
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
* 0xFF - Read pin state (will return GPIO_STATE message)

### 0x81 - GPIO_STATE

| Field     | Size |
|-----------|------|
| OPCODE    | 1    |
| PIN_MODE  | 4    |
| PIN_STATE | 4    |

GPIO_STATE is only to be returned in response to certain GPIO commands. The OBC will ignore any GPIO_STATE packets it receives.

PIN_MODE contains the state of each pin. For each pin, a 1 represents output and 0 input.

PIN_STATE is the actual state of the pin, regardless of whether it's an input or an output (I think, the datasheet doesn't actually specifically say this).

### 0x82 - ADC
