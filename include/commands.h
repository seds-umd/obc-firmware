#pragma once

#include <stdint.h>

#define COMMANDS_MAX_ENTRIES UINT8_MAX+1

/**
 * @brief Initialize command handler.
 * 
 */
void command_init();

/**
 * @brief Register a new command.
 * 
 * The opcode must be greater than or equal to 0, and less than
 * COMMAND_MAX_ENTRIES.
 * 
 * The command handler must return 0 on success or a negative int on failure
 * to differentiate from command handler return values.
 * 
 * Returns 0 for success, 1 if opcode is out of range and 2 if opcode is
 * already used.
 * 
 * @param opcode Command opcode
 * @param handler Pointer to command handler function
 * @return int 
 */
int command_register(int opcode, int (*handler)(uint8_t *buf, int len));

/**
 * @brief Remove a command after registering it.
 * 
 * Returns 0 for success, 1 if opcode is out of range and 2 if command
 * isn't already registered.
 * 
 * @param opcode Command opcode
 * @return int 
 */
int command_remove(int opcode);

/**
 * @brief Process a packet and call the correct command.
 * 
 * The first byte in the buffer is the opcode that will determine which
 * command handler is called. The entire buffer (including opcode) will be
 * passed to the handler.
 * 
 * Returns 1 if buffer is empty or NULL, 2 if opcode does not exist, otherwise
 * the return value of the handler (which should be 0 for success and negative
 * for failure).
 * 
 * @param buf 
 * @param len 
 * @return int 
 */
int command_process(uint8_t *buf, int len);

/**
 * @brief Print all opcodes that have valid handlers
 * 
 */
void command_debug_print_opcodes();
