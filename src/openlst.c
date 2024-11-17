#include "openlst.h"

#include "command_handler.h"
#include "command_formats.h"
#include "config.h"
#include "macros.h"

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/rand.h"
#include "pico/time.h"
#include "pico/util/queue.h"
#include "sha256.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//static variable ---> only this file can see
// volatile --> may unexpectedly change
// receiving buffer --> continuously reading data
// packet buffer --> holds complete buffer

// RX buffer
static uint8_t rx_buf[OPENLST_RX_BUF_LEN]; // 
static uint8_t pkt_buf[OPENLST_MAX_PAYLOAD];
static volatile uint16_t rx_buf_wr;  // Next byte to be written
static uint16_t rx_buf_rd;           // Next byte to be consumed

// TX packet buffer
static openlst_packet_t tx_buf[OPENLST_TX_BUF_COUNT];

// Queue of packets
static queue_t tx_buf_queue;

// Status of packets stored as bitmask, 1 if in use
static uint32_t tx_buf_status;

// TX DMA
static int tx_dma_chan;

static uint16_t tx_seq;

inline static uint16_t rx_buffer_len() {
    return ((uint16_t)(rx_buf_wr - rx_buf_rd)) % OPENLST_RX_BUF_LEN;
}

void openlst_init() {
    uart_init(OPENLST_UART_ID, OPENLST_UART_BAUD); // Initialized the baud rate for UART communicatioj

    // Configure pins
    gpio_set_function(OPENLST_UART_TX, GPIO_FUNC_UART); 
    gpio_set_function(OPENLST_UART_RX, GPIO_FUNC_UART);
    gpio_set_function(OPENLST_UART_CTS, GPIO_FUNC_UART); // CTS --> clear to send, receiver indicates they are ready to receive data
    gpio_set_function(OPENLST_UART_RTS, GPIO_FUNC_UART); // RTS -->  ready to send, transmitter indicates they want to send data

    // Initialize RX buffer
    rx_buf_wr = 0; // data is being put into the receiving buffer while data is being read from the receiving buffer
    rx_buf_rd = 0;

    // Configure UART settings
    uart_set_hw_flow(OPENLST_UART_ID, OPENLST_FLOW, OPENLST_FLOW); // sets hardware contorl flow
    uart_set_format(OPENLST_UART_ID, 8, 1, UART_PARITY_NONE); // UART format: 8 data bits, 1 stop bit, and no parity
    uart_set_fifo_enabled(OPENLST_UART_ID, true); // enables FIFO (first in, first out)


    // Configure interrupt
    irq_set_exclusive_handler(OPENLST_UART_IRQ, openlst_uart_isr);
    irq_set_enabled(OPENLST_UART_IRQ, true);
    uart_set_irq_enables(OPENLST_UART_ID, true, false);

    // Set interrupt to trigger when FIFO is 7/8 full
    hw_write_masked(&uart_get_hw(OPENLST_UART_ID)->ifls, 0b100,
                    UART_UARTIFLS_RXIFLSEL_BITS);

    // Initialize TX buffer
    tx_buf_status = 0;

    // Queue uses dynamic memory allocation which is not great, but it's only
    // 16 bytes. A queue may be overkill for this but it needs to be thread
    // safe since it's accessed inside the ISR.
    queue_init(&tx_buf_queue, sizeof(int), OPENLST_TX_BUF_COUNT);

    // TX DMA
    tx_dma_chan = dma_claim_unused_channel(true);  // TODO: handle error
    dma_channel_config dma_cfg = dma_channel_get_default_config(tx_dma_chan);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_dreq(
        &dma_cfg, (OPENLST_UART_ID == uart0) ? DREQ_UART0_TX : DREQ_UART1_TX);

    dma_channel_configure(
        tx_dma_chan, &dma_cfg,
        &uart_get_hw(OPENLST_UART_ID)->dr,  // Write to UART DR
        NULL,                               // No read address yet
        0,                                  // Unknown transfer size
        false                               // Don't start yet
    );

    // DMA IRQ
    dma_channel_set_irq0_enabled(tx_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, openlst_dma_isr);
    irq_set_enabled(DMA_IRQ_0, true);

    tx_seq = get_rand_32();
}

void openlst_deinit() { queue_free(&tx_buf_queue); } //clears the transmission buffer queue

void __not_in_flash_func(openlst_uart_isr)() {
    while (uart_is_readable(OPENLST_UART_ID)) {
        // Access register directly to speed things up
        rx_buf[rx_buf_wr++] = UART_DR(OPENLST_UART_ID);
        rx_buf_wr %= OPENLST_RX_BUF_LEN;
    }
}


//This function transfer data from the receiving buffer to the buffer responsible for the handling
void openlst_process() {
    uint16_t buf_len = rx_buffer_len();

    // Loop until buffer is empty or only a partial packet remains
    while (1) {
        // Loop until start bytes are found or we run out of bytes
        while (1) {
            uint8_t byte1 = rx_buf[rx_buf_rd];
            uint8_t byte2 = rx_buf[(rx_buf_rd + 1) % OPENLST_RX_BUF_LEN];

            // Less than 3 bytes left, leave them for next time
            if (buf_len < 3) {
                return;
            }

            // Check for start bytes
            if ((byte1 == OPENLST_START_0) && (byte2 == OPENLST_START_1)) {
                break;
            }

            rx_buf_rd++;
            rx_buf_rd %= OPENLST_RX_BUF_LEN;
            buf_len--;
        }

        // At this point, we should have at least the 2 start bytes and the
        // length byte in the buffer. The read pointer will stay at the start
        // of the packet until the entire packet is in the buffer and able to
        // be processed.
        // is the length byte already in the data

        uint8_t pkt_len = rx_buf[(rx_buf_rd + 2) % OPENLST_RX_BUF_LEN];

        // If the entire packet isn't fully received yet, wait until next time
        if (buf_len < pkt_len + 3) {
            return;
        }

        // Copy packet into a new buffer to prevent overwriting rx_buf
        if ((rx_buf_rd + 3) % OPENLST_RX_BUF_LEN + pkt_len >=
            OPENLST_RX_BUF_LEN) {
            // Handle wraparound
            int count = OPENLST_RX_BUF_LEN - (rx_buf_rd + 3);
            memcpy(pkt_buf, rx_buf + rx_buf_rd + 3, count);
            memcpy(pkt_buf + count, rx_buf, pkt_len - count);
        } else {
            memcpy(pkt_buf, rx_buf + (rx_buf_rd + 3) % OPENLST_RX_BUF_LEN, pkt_len);
        }

        // Move read pointer to after packet
        rx_buf_rd = (rx_buf_rd + pkt_len + 3) % OPENLST_RX_BUF_LEN;
        buf_len -= pkt_len + 3;

        openlst_handle_packet(pkt_buf, pkt_len);
    }
}

void openlst_handle_packet(uint8_t *buf, uint8_t len) {

    //decrypt
    // Take a packet and process it based on the command in the header

    packet_t pkt;
    pkt.type = PACKET_TYPE_OPENLST;
    pkt.lst_pkt = (openlst_packet_t *)buf;
    
    uint8_t message[sizeof(pkt.lst_pkt->pld)+sizeof(pkt.lst_pkt->hdr)+16];
    uint8_t finalHash[sizeof(pkt.lst_pkt->pld)+sizeof(pkt.lst_pkt->hdr)+16];

    uint8_t receivedHash[32];
    memcpy(&receivedHash, &(pkt.lst_pkt->pld.gnd_cmd_uplink.hash), 32);
    memset(&(pkt.lst_pkt->pld.gnd_cmd_uplink.hash), 0, 32);
    

    uint8_t key[16] = {0x1A, 0x3F, 0x57, 0xA1, 0x8C, 0xC2, 0xD4, 0xE5, 0x07, 0x19, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x80};
    memcpy(&message[0], &(pkt.lst_pkt->pld), sizeof(pkt.lst_pkt->pld));
    memcpy(&message[0]+ sizeof(pkt.lst_pkt->pld), &(pkt.lst_pkt->hdr), sizeof(pkt.lst_pkt->hdr));
    memcpy(&message[0] + sizeof(pkt.lst_pkt->pld)+sizeof(pkt.lst_pkt->hdr), key, sizeof(key));

    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, pkt.lst_pkt->pld.gnd_cmd_uplink.hash, sizeof(pkt.lst_pkt->pld.gnd_cmd_uplink.hash));
    sha256_final(&ctx, finalHash);

    //if(memcmp(receivedHash, finalHash, 32) == 0){
        pkt.lst_pkt->len = len;

        switch (pkt.lst_pkt->hdr.command) {
            // ASCII messages are our custom commands
            case ASCII:
                // TODO: handle command return value
                command_process(&pkt);
                break;

            // Ignore these commands
            case BOOTLOADER_PING:
            case BOOTLOADER_ACK:  // TODO: handle ACK
            case BOOTLOADER_WRITE_PAGE:
            case BOOTLOADER_ERASE:
            case ACK:   // TODO: handle ACK
            case NACK:  // TODO: handle NACK
            case REBOOT:
            default:
                break;
        }
    //}




    
}



openlst_packet_t *openlst_get_tx_buffer() {
    // Return first free buffer
    int i = __builtin_ctz(~tx_buf_status);
    /*This line uses the __builtin_ctz function, which counts the number of trailing zeros in the binary representation of the value.
The expression ~tx_buf_status inverts the bitmask that indicates which transmission buffers are in use.
The result is the index i of the first free buffer. If all buffers are in use, i would equal OPENLST_TX_BUF_COUNT.*/

    if (i >= OPENLST_TX_BUF_COUNT) {
        // No free buffers
        return NULL;
    /*This check ensures that there is at least one free buffer available. If i is greater than or equal to OPENLST_TX_BUF_COUNT, it means no buffers are free, and the function returns NULL.*/
    } else {
        // Mark buffer as in use until packet is sent
        tx_buf_status |= (1 << i);
        /*If a free buffer is found, this line marks it as "in use" by setting the corresponding bit in tx_buf_status. This prevents the same buffer from being used for another transmission until it is released.*/

        // Use some reasonable defaults
        tx_buf[i].hdr.hwid = 0x0000;
        tx_buf[i].hdr.system = 0x01;
        tx_buf[i].hdr.command = ASCII;
        /*Here, the function initializes some fields of the packet header in the selected buffer (tx_buf[i]) with default values. This includes:
hwid (hardware ID) set to 0x0000
system set to 0x01
command set to ASCII
*/

        return &tx_buf[i];
        /*Finally, the function returns a pointer to the allocated and initialized transmission buffer. The caller can then fill this buffer with data to be transmitted.*/
    }
}

//gives a sequence number to each packet, returns the sequence numebr and increments it for next call
uint16_t openlst_get_seq() { return tx_seq++; }

static void openlst_tx_dma(int pkt_idx) {
    // Send first 3 bytes, DMA won't be running yet
    uart_putc_raw(OPENLST_UART_ID, OPENLST_START_0);
    uart_putc_raw(OPENLST_UART_ID, OPENLST_START_1);
    uart_putc_raw(OPENLST_UART_ID, tx_buf[pkt_idx].len);

    // Start DMA transfer
    dma_channel_set_trans_count(tx_dma_chan, tx_buf[pkt_idx].len, false);
    dma_channel_set_read_addr(tx_dma_chan, &tx_buf[pkt_idx], true);
}

/*The openlst_tx(openlst_packet_t *pkt) function is responsible for queuing a packet for transmission and initiating the transfer if the DMA channel is available. Here's a breakdown of how this function works:*/
int openlst_tx(openlst_packet_t *pkt) {
    // Assuming GCC puts tx_buf in sequential memory with no gaps, we can
    // calculate the index of the packet from it's pointer
    int pkt_idx = pkt - tx_buf;
    /*This line calculates the index of the packet in the tx_buf array by subtracting the base address of the tx_buf from the address of the pkt. This works under the assumption that tx_buf is laid out in contiguous memory with no gaps.*/


    /*This check ensures that the calculated index (pkt_idx) is valid (i.e., it falls within the range of available buffers in tx_buf). If it is invalid, the function returns 1, indicating an error.*/
    if ((pkt_idx < 0) | (pkt_idx >= OPENLST_TX_BUF_COUNT)) {
        // pkt does not point to a struct within the TX buffer
        return 1;
    }
    
    /*This line adds the index of the packet to the transmission queue (tx_buf_queue). The function is blocking, meaning it will wait if the queue is full until a space becomes available.*/
    // Add packet to queue


    
    
    
    
    queue_add_blocking(&tx_buf_queue, &pkt_idx);

    /*Here, the function checks if the DMA channel is currently busy. If it is not, it calls openlst_tx_dma(pkt_idx) to initiate the transmission of the packet. This starts the actual process of sending the packet data over UART.
*/
    // Start transfer if DMA is currently idle
    if (!dma_channel_is_busy(tx_dma_chan)) {
        openlst_tx_dma(pkt_idx);
    }

    return 0;
}

void __not_in_flash_func(openlst_dma_isr)() {
    // Clear request
    dma_hw->ints0 = 1 << tx_dma_chan;

    int pkt;

    // Remove packet that just completed
    queue_remove_blocking(&tx_buf_queue, &pkt);

    // Mark status as free
    tx_buf_status &= ~(1 << pkt);

    // Start another transfer
    if (!queue_is_empty(&tx_buf_queue)) {
        queue_peek_blocking(&tx_buf_queue, &pkt);
        openlst_tx_dma(pkt);
    }
}

int openlst_done() {
    return queue_get_level(&tx_buf_queue) == 0;
}
