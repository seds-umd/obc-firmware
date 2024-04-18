#include "updater.h"

#include "bl_common.h"
#include "command_formats.h"
#include "openlst.h"

#include "hardware/flash.h"

#include <string.h>

static enum UpdaterState state = UPDATER_IDLE;
static int init_state = 0;

static uint32_t current_update_crc = 0;
static uint32_t current_update_size = 0;

// Pointer to no-cache no-alloc section of flash. We don't want to waste cache
// on this because whenever we want to read it, we'll want the latest version.
// Made const so it errors if we try to write (which will fail at runtime).
static volatile const uint8_t *const flash_read =
    (uint8_t *)FLASH_ADDR_NOCACHE_NOALLOC;

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

    // TODO: use custom flash commands so we only have to write a single byte
    flash_range_program(phys_addr, buf, 256);

    // TODO: verify bit is written
}

uint16_t count_remaining_chunks() {
    uint32_t update_size =
        *((uint32_t *)(flash_read + BL_UPDATE_HEADER_UPDATE_SIZE));
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
        case UPDATER_IDLE:
            break;

        case UPDATER_INIT:
            if (updater_try_init() == 1) {
                state = UPDATER_WAITING;
            }
            break;

        case UPDATER_WAITING:
            break;

        default:
            break;
    }
}

void updater_start_init(packet_t *pkt) {
    // Get update header info and start initialization
    state = UPDATER_INIT;
    init_state = 0;

    current_update_crc = pkt->lst_pkt->pld.gnd_cmd.msg.update_init.crc32;
    current_update_size = pkt->lst_pkt->pld.gnd_cmd.msg.update_init.size;
}

int updater_try_init() {
    init_state++;

    switch (init_state) {
        case 0:  // Erase staging slot
            // TODO: break this up into multiple calls
            flash_range_erase(BL_UPDATE_START, BL_UPDATE_SIZE);
            break;

        case 1:  // Erase header slot
            flash_range_erase(BL_UPDATE_HEADER_START, BL_UPDATE_HEADER_SIZE);
            break;

        case 2:;  // Program header
            uint32_t buf[64];
            memset(buf, 0xFF, 256);

            buf[0] = current_update_crc;
            buf[1] = current_update_size;

            flash_range_program(BL_UPDATE_HEADER_START, (uint8_t *)buf, 256);
            break;

        default:
            return 1;
            break;
    }

    return 0;
}

void updater_write_chunk(packet_t *pkt) {
    uint8_t buf[256];
    uint8_t *half_page = pkt->lst_pkt->pld.gnd_cmd.msg.update_chunk.data;

    uint16_t addr = pkt->lst_pkt->pld.gnd_cmd.msg.update_chunk.addr;
    uint8_t addr_half_page = addr & 0x1;  // Get position within page

    // Actual flash hardware address
    uint32_t addr_flash = BL_UPDATE_START + (addr << 7);

    // Fill half of page with received chunk
    memcpy(buf + addr_half_page * 128, half_page, 128);

    // Fill other half with 1s so it doesn't overwrite existing data
    memset(buf + (addr_half_page ? 0 : 1) * 128, 0xFF, 128);

    flash_range_program(addr_flash, buf, 256);

    // Read back data from flash and compare to packet
    int match = 0;
    uint32_t *expected = (uint32_t *)(half_page);
    volatile uint32_t *actual = (uint32_t *)(flash_read + addr_flash);

    for (uint8_t i = 0; i < 128 / sizeof(uint32_t); i++) {
        // If any bit is different, match won't be 0
        match |= *actual ^ *expected;
    }

    if (match == 0) {
        // Write successful, update status
        set_update_status(addr);
    } else {
        // TODO: figure out what to do here
    }
}

void updater_send_status(packet_t *pkt) {
    // Ignore incoming packet, contents doesn't matter

    // Save last address between calls
    static uint16_t last_addr_checked = 0;

    uint16_t remaining = count_remaining_chunks();
    uint32_t crc_expected = *(flash_read + BL_UPDATE_HEADER_CRC);

    if (remaining == 0) {
        // Only check CRC if all data has already been written
    } else {
        // Only record addresses if there's unwritten chunks
    }

    // Generate reply packet
    openlst_packet_t *reply = openlst_get_tx_buffer();
    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->pld.gnd_cmd.opcode = 0x33;

    reply->pld.gnd_cmd.msg.update_status.update_status = state;
    // reply->pld.gnd_cmd.msg.update_status.crc_matched =
    reply->pld.gnd_cmd.msg.update_status.crc_expected = crc_expected;
    reply->pld.gnd_cmd.msg.update_status.chunks_remaining = remaining;
    // reply->pld.gnd_cmd.msg.update_status.chunk_addr =

    openlst_tx(reply);
}
