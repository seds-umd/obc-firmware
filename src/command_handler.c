#include "command_handler.h"

#include "command_formats.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// A struct isn't really necessary when the handler is the only thing we're
// using, but it adds no performance overhead and might be useful later.
typedef struct {
    int (*handler)(packet_t *pkt);
    // TODO: do we want a pointer for context data to be passed to the handler?
} command_entry_t;

// Store command handlers in a LUT using opcode
static command_entry_t command_table[COMMANDS_MAX_ENTRIES];

void command_init() {
    // Make sure all handlers are initialized to NULL
    for (int i = 0; i < COMMANDS_MAX_ENTRIES; i++) {
        command_table[i].handler = NULL;
    }
}

int command_register(int opcode, int (*handler)(packet_t *pkt)) {
    if ((opcode >= COMMANDS_MAX_ENTRIES) || (opcode < 0)) {
        // Opcode out of range
        return 1;
    }

    if (command_table[opcode].handler != NULL) {
        // Opcode already used by another command
        return 2;
    }

    // Add handler to command table
    command_table[opcode].handler = handler;

    return 0;
}

int command_remove(int opcode) {
    if ((opcode >= COMMANDS_MAX_ENTRIES) || (opcode < 0)) {
        // Opcode out of range
        return 1;
    }

    if (command_table[opcode].handler == NULL) {
        // Opcode isn't in use
        return 2;
    }

    // Remove handler
    command_table[opcode].handler = NULL;

    return 0;
}

int command_process(packet_t *pkt) {
    int len;
    uint8_t *buf;

    if (pkt == NULL) {
        // Packet is invalid
        return 1;
    }

    if (pkt->type == PACKET_TYPE_OPENLST) {
        buf = pkt->lst_pkt->pld.buf;
        len = pkt->lst_pkt->len - OPENLST_HEADER_SIZE;
    } else {
        // TODO
    }

    if ((len == 0) || (buf == NULL)) {
        // Buffer is invalid
        return 1;
    }

    uint8_t opcode = buf[0];
    int (*handler)(packet_t *pkt) = command_table[opcode].handler;

    if (handler == NULL) {
        // Command handler doesn't exist
        return 2;
    }

    // Run command and return it's return value
    return (*handler)(pkt);
}

void command_debug_print_opcodes() {
    for (int i = 0; i < COMMANDS_MAX_ENTRIES; i++) {
        if (command_table[i].handler != NULL) {
            printf("Command found at opcode %#02x\n", i);
        }
    }
}
