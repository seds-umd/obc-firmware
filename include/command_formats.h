/**
 * This file contains all command definitions and related structures.
 */

#pragma once

#include <stdint.h>

// Maximum size of an OpenLST packet, not including start bytes or size bytes
#define OPENLST_MAX_PAYLOAD 251
#define OPENLST_START_0 0x22
#define OPENLST_START_1 0x69

////////// Ground Commands //////////

// typedef struct {};

////////// OpenLST Packets //////////

// OpenLST commands
typedef enum {
    // Bootloader commands
    BOOTLOADER_PING = 0x00,
    BOOTLOADER_ACK = 0x01,
    BOOTLOADER_WRITE_PAGE = 0x02,
    BOOTLOADER_ERASE = 0x0C,

    // Normal commands
    ACK = 0x10,
    NACK = 0xFF,
    ASCII = 0x11,
    REBOOT = 0x12,
    GET_TIME = 0x13,
    SET_TIME = 0x14,
    RANGING = 0x15,
    RANGING_ACK = 0x16,
    GET_TELEM = 0x17,
    TELEM = 0x18

    // TODO: other commands and custom commands
} openlst_command_t;

typedef struct {
    uint16_t hwid;
    uint16_t seq;
    uint8_t system;
    uint8_t command;
} openlst_packet_header_t;

typedef union {
    uint8_t buf[OPENLST_MAX_PAYLOAD - sizeof(openlst_packet_header_t)];

    // OpenLST commands
    // TODO

    // Ground commands
} openlst_packet_payload_t;

/// @brief OpenLST packet structure
typedef struct {
    openlst_packet_header_t hdr;
    openlst_packet_payload_t pld;
} openlst_packet_t;
