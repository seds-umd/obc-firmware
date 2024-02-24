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

| Hex  | Opcode | Firmware Status | Python Status |
| ---- | ------ | --------------- | ------------- |
| 0x00 | ACK    | TODO            | TODO          |
| 0x01 | PING   | TODO            | TODO          |
| 0x80 | GPIO   | TODO            | TODO          |


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

### 0x80 - GPIO

| Field | Size |
| - | - |
| OPCODE | 1 |
| PIN | 1 |
| 

### 0x81 - ADC
