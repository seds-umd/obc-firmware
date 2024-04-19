#pragma once

#include "command_formats.h"

#ifdef __arm__
// RP2040 no-cache no-alloc flash alias
#define FLASH_ADDR_NOCACHE_NOALLOC 0x13000000
#else
// simulated flash
#define FLASH_ADDR_NOCACHE_NOALLOC sim_flash_buf
#endif

enum UpdaterState {
    UPDATER_IDLE = 0,                  // No updates in progress
    UPDATER_INIT = 1,                  // Initialization process
    UPDATER_WAITING = 2,               // Waiting for data to come in
    UPDATER_READY = 3,                 // Update is ready to be applied
    UPDATER_ERR_SET_STATUS = -1,       // Chunk status not set correctly
    UPDATER_ERR_SIZE_OOB = -2,         // Update size out of bounds
    UPDATER_ERR_CHUNK_FAILED = -3,     // Chunk wasn't written successfully
    UPDATER_ERR_STATUS_ADDR_OOB = -4,  // Status write address out of bounds
    UPDATER_ERR_CHUNK_OOB = -5,        // Chunk address out of bounds
    UPDATER_ERR_CRC_MISMATCH = -6,     // CRC doesn't match
};

/**
 * @brief Run any updater tasks that need to be run.
 */
void updater_process();

/**
 * @brief Start updater initialization.
 *
 * @param pkt Packet that commanded initialization.
 */
void updater_start_init(packet_t *pkt);

/**
 * @brief Attempt to complete updater initialization.
 *
 * Several erase commands are needed to complete the update initialization
 * process, so this function will need to be called multiple times.
 *
 * @return int 1 if initialization is complete, 0 otherwise
 */
int updater_try_init();

/**
 * @brief Write a chunk of the update to flash.
 *
 * @param pkt Incoming command packet
 */
void updater_write_chunk(packet_t *pkt);

/**
 * @brief Populate status packet with data.
 *
 * This is a separate function from updater_send_status to make testing easier.
 *
 * @param reply Reply packet to fill with status
 */
void updater_populate_status(openlst_packet_t *reply);

/**
 * @brief Send update status packet.
 *
 * @param pkt Incoming command packet
 */
void updater_send_status(packet_t *pkt);
