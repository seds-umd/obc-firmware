#pragma once

#include "command_formats.h"

#include <stdint.h>

/**
 * @brief Initialize OpenLST command processor.
 * 
 * This function uses dynamic memory allocation, so can only be safely called
 * once. Multiple calls will cause a memory leak.
 */
void openlst_init();

/**
 * @brief UART RX interrupt service routine.
 * 
 * Grabs bytes from UART and stores them in a buffer for later processing.
 */
void openlst_uart_isr();

/**
 * @brief Process data in buffer and extract packets.
 */
void openlst_process();

/**
 * @brief Decode a packet and process commands.
 * 
 * @param buf Buffer containing packet
 * @param len Length of packet in buffer
 */
void openlst_handle_packet(uint8_t *buf, uint8_t len);

/**
 * @brief Get a free TX buffer to fill with data.
 * 
 * Returns NULL if no buffers are free. Header is by default set to HWID 0000,
 * system 0x01, command ASCII.
 * 
 * @return openlst_packet_t* 
 */
openlst_packet_t *openlst_get_tx_buffer();

/**
 * @brief Get next sequence number for non-reply messages.
 * 
 * Reply messages should instead use the same sequence number as the message
 * being replied to.
 * 
 * @return uint16_t
 */
uint16_t openlst_get_seq();

/**
 * @brief Transmit message.
 * 
 * Returns 0 for success.
 * 
 * @param pkt Pointer to packet
 * @param len Length of packet, including header (6 + length of payload)
 * 
 * @return int
 */
int openlst_tx(openlst_packet_t *pkt);

/**
 * @brief DMA interrupt service routine.
 * 
 * Runs after a DMA transfer completes. The buffer is marked as available and a
 * new packet is transmitted if one is waiting in the queue.
 * 
 */
void openlst_dma_isr();
