/**
 * This file contains all command definitions and related structures.
 */

#pragma once

#include "telemetry.h"

#include "pico/platform.h"

#include <stdint.h>

// Maximum size of an OpenLST packet, not including start bytes or size bytes
#define OPENLST_MAX_PAYLOAD 251
#define OPENLST_START_0 0x22
#define OPENLST_START_1 0x69

////////// Ground Commands //////////

typedef union {
    __packed struct {
        uint32_t pin;
        uint8_t pin_op;
    } gpio;

    __packed struct {
        uint32_t pin_mode;
        uint32_t pin_state;
    } gpio_state;

    __packed struct {
        uint8_t cmd;

        union {
            __packed struct {
                uint8_t addr[3];
                uint8_t size;
            } read;

            __packed struct {
                uint8_t addr[3];
                // Data length dictated by buffer in openlst_packet_payload_t
                uint8_t data[1];
            } program;

            __packed struct {
                uint8_t addr[3];
                uint8_t size;
            } erase;

            __packed struct {
                uint8_t data[1];
            } read_resp;

            __packed struct {
                uint64_t unique_id;
            } unique_id_resp;
        };
    } flash_cmd;

    struct telem_struct telem;

    __packed struct {
        uint32_t size;
        uint32_t crc32;
    } update_init;

    __packed struct {
        uint16_t addr;
        uint8_t data[128];
    } update_chunk;

    __packed struct {
        int8_t update_status;
        uint8_t crc_matched;
        uint32_t crc_expected;
        uint16_t chunks_remaining;
        uint16_t chunk_addr[96];
    } update_status;
} command_t;

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

typedef __packed struct {
    uint16_t hwid;
    uint16_t seq;
    uint8_t system;
    uint8_t command;
} openlst_packet_header_t;

#define OPENLST_HEADER_SIZE sizeof(openlst_packet_header_t)

typedef union {
    uint8_t buf[OPENLST_MAX_PAYLOAD - OPENLST_HEADER_SIZE];

    // OpenLST commands
    // TODO

    // Ground commands
    __packed struct {
        uint8_t opcode;
        command_t msg;
    } gnd_cmd;
} openlst_packet_payload_t;

/// @brief OpenLST packet structure
typedef __packed struct {
    // Actual packet is 251 bytes long
    openlst_packet_header_t hdr;
    openlst_packet_payload_t pld;

    // 5 extra bytes for padding to 256 bytes and metadata
    uint8_t len;  // Length as sent over UART, includes header and payload
    uint8_t _padding[4];
} openlst_packet_t;

#define PACKET_TYPE_OPENLST 1
#define PACKET_TYPE_PIB 2

typedef __packed struct {
    int type;
    union {
        openlst_packet_t *lst_pkt;
        // TODO: PIB packet
    };
} packet_t;
