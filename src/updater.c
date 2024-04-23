#include "updater.h"

#include "bl_common.h"
#include "command_formats.h"
#include "config.h"
#include "crc32.h"
#include "macros.h"
#include "openlst.h"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"

#include <string.h>

static enum UpdaterState state = UPDATER_IDLE;
static int init_state = 0;

static uint32_t current_update_crc = 0;
static uint32_t current_update_size = 0;

// Returns 0 if update has been written
static inline int get_update_status(uint16_t addr) {
    uint8_t status_byte = *(flash_read + BL_UPDATE_HEADER_STATUS + (addr >> 3));

    return (status_byte >> (addr & 0x7)) & 0x1;
}

// Set update status bit to 0 for a given address. Address is for half page
void set_update_status(uint16_t addr) {
    uint8_t page_addr = (addr >> 11) & 0x3;  // 4 pages, 2 bit address
    uint8_t byte_addr = (addr >> 3) & 0xFF;  // 256 bytes/page, 8 bit address
    uint8_t bit_addr = addr & 0x7;           // 8 bits/byte, 3 bit address

    // Set to all ones so no other data is overwritten
    uint8_t buf[256];
    memset(buf, 0xFF, 256);

    buf[byte_addr] &= ~(1 << bit_addr);

    uint32_t phys_addr = BL_UPDATE_HEADER_STATUS + page_addr * 256;

    if ((phys_addr >= BL_UPDATE_HEADER_STATUS + BL_UPDATE_HEADER_SIZE) |
        (phys_addr < BL_UPDATE_HEADER_STATUS)) {
        state = UPDATER_ERR_STATUS_ADDR_OOB;
        return;
    }

    // TODO: use custom flash commands so we only have to write a single byte
    flash_range_program(phys_addr, buf, 256);

    // Verify bit is written correctly
    int verify = get_update_status(addr);

    // TODO: make this slightly more recoverable
    if (verify != 0) {
        state = UPDATER_ERR_SET_STATUS;
    }
}

uint8_t get_update_valid() {
    uint8_t valid = *(flash_read + BL_UPDATE_HEADER_VALID);

    if (valid == 0)
        return 8;
    else
        return __builtin_ctz(valid);
}

void set_update_valid(uint8_t state) {
    if (state > 8) {
        state = 8;
    }

    uint8_t buf[256];
    memset(buf, 0xFF, 256);

    // Set state bits low
    uint8_t valid = ~((1 << state) - 1);
    buf[255] &= valid;

    flash_range_program((BL_UPDATE_HEADER_VALID) & ~(0xFFu), buf, 256);

    if (*(flash_read + BL_UPDATE_HEADER_VALID) != valid) {
        // Try again I guess? Not much else we can do here
        flash_range_program((BL_UPDATE_HEADER_VALID) & ~(0xFFu), buf, 256);
    }
}

uint16_t count_remaining_chunks() {
    uint32_t update_size = get_update_size();
    uint16_t update_size_bits = update_size >> 7;    // Number of status bits
    uint16_t update_size_bytes = update_size >> 10;  // Number of status bytes

    const volatile uint8_t *status_ptr = flash_read + BL_UPDATE_HEADER_STATUS;

    // Count ones - remaining blocks that are unwritten
    uint16_t remaining = 0;

    for (int i = 0; i < update_size_bytes; i++) {
        remaining += __builtin_popcount(*status_ptr);
        status_ptr++;
    }

    // remainder: 3 bits for partially used last byte
    uint8_t rem_count = update_size_bits & 0x7;
    if (rem_count != 0) {
        uint32_t rem = *status_ptr & ((1 << rem_count) - 1);
        remaining += __builtin_popcount(rem);
    }

    return remaining;
}

void updater_process() {
    switch (state) {
        case UPDATER_IDLE:;
            // Check to see if there was an update that was interrupted
            uint8_t valid = get_update_valid();

            if (valid == 1) {
                state = UPDATER_INIT;
                init_state = 2;
            } else if (valid == 2) {
                state = UPDATER_WAITING;
            }
            break;

        case UPDATER_INIT:
            if (updater_try_init() == 1) {
                state = UPDATER_WAITING;
                // TODO: send ACK here
            }
            break;

        case UPDATER_WAITING:
            break;

        default:
            break;
    }
}

int updater_start_init(packet_t *pkt) {
    // Get update header info and start initialization
    state = UPDATER_INIT;
    init_state = 0;

    current_update_crc = pkt->lst_pkt->pld.gnd_cmd.msg.update_init.crc32;
    current_update_size = pkt->lst_pkt->pld.gnd_cmd.msg.update_init.size;

    if (current_update_size > BL_UPDATE_SIZE) {
        current_update_size = BL_UPDATE_SIZE;
        state = UPDATER_ERR_SIZE_OOB;
    }

    return 0;
}

int updater_try_init() {
    switch (init_state) {
        case 0:  // Erase header slot
            flash_range_erase(BL_UPDATE_HEADER_START, BL_UPDATE_HEADER_SIZE);
            break;

        case 1:  // Erase staging slot
            // TODO: break this up into multiple calls
            flash_range_erase(BL_UPDATE_START, BL_UPDATE_SIZE);
            set_update_valid(1);
            break;

        case 2:;  // Program header
            uint32_t buf[64];
            memset(buf, 0xFF, 256);

            buf[0] = current_update_size;
            buf[1] = current_update_crc;

            flash_range_program(BL_UPDATE_HEADER_START, (uint8_t *)buf, 256);
            set_update_valid(2);
            break;

        default:
            state = UPDATER_WAITING;
            return 1;
            break;
    }

    init_state++;

    return 0;
}

int updater_write_chunk(packet_t *pkt) {
    uint8_t *half_page = pkt->lst_pkt->pld.gnd_cmd.msg.update_chunk.data;
    uint16_t addr = pkt->lst_pkt->pld.gnd_cmd.msg.update_chunk.addr;

    int ret = write_chunk(BL_UPDATE_START, addr, half_page);

    if (ret == 0) {
        // Write successful, update status
        set_update_status(addr);
    } else if (ret == 1) {
        // TODO: at least attempt to recover
        state = UPDATER_ERR_CHUNK_FAILED;
    } else if (ret == 2) {
        state = UPDATER_ERR_CHUNK_OOB;
    } else {
        state = UPDATER_ERR_UNKNOWN;
    }

    return 0;
}

void updater_populate_status(openlst_packet_t *reply) {
    // Save last address between calls
    static uint16_t last_addr_checked = 0;

    uint32_t update_size = get_update_size();
    uint16_t remaining = count_remaining_chunks();
    uint32_t crc_expected = *((uint32_t *)(flash_read + BL_UPDATE_HEADER_CRC));
    uint8_t crc_match = 0;  // 1 if CRC matches
    uint8_t addr_count = 0;

    if ((remaining == 0) & (state == UPDATER_WAITING)) {
        // Only check CRC if all data has already been written

        uint32_t crc_actual =
            calc_crc32((uint8_t *)flash_read + BL_UPDATE_START, update_size);

        crc_match = (crc_expected == crc_actual) ? 1 : 0;

        if (crc_match == 0) {
            state = UPDATER_ERR_CRC_MISMATCH;
        } else if (crc_match & (remaining == 0)) {
            state = UPDATER_READY;
            set_update_valid(8);
        }
    } else if (state == UPDATER_WAITING) {
        // Only record addresses if there's unwritten chunks
        // Include up to 96 chunk addresses
        uint16_t addr = last_addr_checked;
        uint16_t *addr_list = reply->pld.gnd_cmd.msg.update_status.chunk_addr;

        while (1) {
            // Done if list is full
            if (addr_count == 96) break;

            addr++;

            if (addr >= update_size / UPDATER_CHUNK_SIZE) {
                addr %= update_size / UPDATER_CHUNK_SIZE;
            }

            // Add to list if not yet written
            if (get_update_status(addr) == 1) {
                *((uint8_t *)(addr_list + addr_count)) = *((uint8_t *)&addr);
                *(((uint8_t *)(addr_list + addr_count)) + 1) =
                    *(((uint8_t *)&addr) + 1);
                addr_count++;
            }

            // Done if all addresses have been checked
            if (addr == last_addr_checked) break;
        }

        last_addr_checked = addr;
    }

    // Populate reply packet
    reply->pld.gnd_cmd.msg.update_status.update_status = state;
    reply->pld.gnd_cmd.msg.update_status.crc_matched = crc_match;
    reply->pld.gnd_cmd.msg.update_status.crc_expected = crc_expected;
    reply->pld.gnd_cmd.msg.update_status.chunks_remaining = remaining;

    // Size changes depending on the number of chunk addresses included
    reply->len = OPENLST_HEADER_SIZE + 1 +  // Header + opcode
                 sizeof(reply->pld.gnd_cmd.msg.update_status) -  // Max size
                 2 * (96 - addr_count);  // Number of addresses actually used
}

int updater_send_status(packet_t *pkt) {
    openlst_packet_t *reply = openlst_get_tx_buffer();

    if (reply == NULL) {
        return 0;
    }

    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->pld.gnd_cmd.opcode = 0x33;

    updater_populate_status(reply);

    openlst_tx(reply);

    return 0;
}

void __not_in_flash_func(finalize_update)() {
    for (int attempt = 0; attempt < 3; attempt++) {
        // Erase header
        flash_range_erase(BL_APP_HEADER_START, BL_APP_HEADER_SIZE);

        // Erase application slot
        flash_range_erase(BL_APP_START, BL_APP_SIZE);

        // Set valid to indicate slot is erased
        set_application_valid(1);

        // Program header with size and CRC
        uint32_t buf[64];
        memset(buf, 0xFF, 256);

        uint32_t size = get_update_size();
        uint32_t crc = get_update_crc();

        buf[0] = size;
        buf[1] = crc;

        flash_range_program(BL_APP_HEADER_START, (uint8_t *)buf, 256);

        // Set valid to indicate header is written
        set_application_valid(2);

        // Copy update slot into application slot
        for (uint32_t i = 0; i < size; i += 256) {
            uint8_t buf[256];
            memcpy(buf, (uint8_t *)flash_read + BL_UPDATE_START + i, 256);
            flash_range_program(BL_APP_START + i, buf, 256);
        }

        // Verify CRC
        uint32_t crc_actual =
            calc_crc32((uint8_t *)flash_read + BL_APP_START, size);

        if (crc == crc_actual) {
            // Set valid byte to indicate update is successful
            set_application_valid(8);
            watchdog_hw->scratch[0] = UPDATER_REBOOT_MAGIC;
            break;
        } else {
            // If CRC doesn't match, retry
            continue;
        }
    }

    // If update is successful or attempted 3 time, set watchdog scratch
    // register to indicate update worked and then reboot
    watchdog_reboot(0, 0, 0);
}

int updater_apply_update(packet_t *pkt) {
    UNUSED(pkt);

    int ready = 1;

    // Check if update slot is valid
    if (get_update_valid() != 8) {
        ready &= 0;
    }

    // Check if size is reasonable
    uint32_t update_size = get_update_size();
    if (update_size > 16 * 1024 * 1024) {
        ready &= 0;
    }

    // Check if update CRC matches, but only if the other checks pass
    if (ready) {
        uint32_t crc_expected =
            *((uint32_t *)(flash_read + BL_UPDATE_HEADER_CRC));
        uint32_t crc_actual =
            calc_crc32((uint8_t *)flash_read + BL_UPDATE_START, update_size);

        if (crc_actual != crc_expected) {
            ready &= 0;
        }
    }

    // Send ACK (or NACK if update isn't ready)
    openlst_packet_t *reply = openlst_get_tx_buffer();
    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->hdr.command = 0x00;
    reply->pld.gnd_cmd.opcode = 0x00;
    reply->pld.gnd_cmd.msg.ack = (ready == 1) ? 0 : 1;  // 0 is ACK, 1 is NACK
    reply->len = OPENLST_HEADER_SIZE + 2;

    // Send ACK and wait for it to transmit
    openlst_tx(reply);
    while (!openlst_done())
        ;

    // Continue normally if update is not ready
    if (!ready) {
        return 0;
    }

    // Turn off interrupts, ignoring UART data after this
    save_and_disable_interrupts();

    finalize_update();

    return 0;
}
