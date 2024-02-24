#pragma once

#include <stdint.h>

#include "command_formats.h"

// Initialize OpenLST command processor
void openlst_init();

// ISR to handle receiving data from UART
void openlst_uart_isr();

// Process data in buffer to extract packets
void openlst_process();

// Decode packets and handle based on command
void openlst_handle_packet(uint8_t *buf, uint8_t len);

/**
 * @brief Get a free TX buffer to fill with data.
 * 
 * @return openlst_packet_t* 
 */
openlst_packet_t *openlst_get_tx_buffer();

/**
 * @brief Get next sequence number for non-reply messages
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

// ISR to handle end of TX DMA transfer
void openlst_dma_isr();
