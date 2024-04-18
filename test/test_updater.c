#include "unity.h"

#include "bl_common.h"
#include "updater.h"

#include "hardware/flash.h"

#include <string.h>

#define LINK_DROP_RATE 0.1

// Function declarations to use internal functions of updater - a bit hacky
void set_update_status(uint16_t addr);
uint16_t count_remaining_chunks();

void test_update_status() {
    // Start with empty flash
    flash_range_erase(0, PICO_FLASH_SIZE_BYTES);

    // Write update size
    uint32_t buf[64];
    memset(buf, 0xFF, 256);

    buf[0] = 128 * 77;  // 2 loops of 32 + 13 remainder
    buf[1] = 0;

    flash_range_program(BL_UPDATE_HEADER_UPDATE_SIZE, (uint8_t *)buf, 256);

    TEST_ASSERT_EQUAL(77, count_remaining_chunks());

    for (int i = 0; i < 12; i++) set_update_status(i);
    TEST_ASSERT_EQUAL(77 - 12, count_remaining_chunks());

    for (int i = 12; i < 77; i++) set_update_status(i);
    TEST_ASSERT_EQUAL(0, count_remaining_chunks());
}

void do_updater_init(uint32_t size, uint32_t crc) {
    packet_t pkt;
    openlst_packet_t lst_pkt;
    pkt.lst_pkt = &lst_pkt;

    pkt.lst_pkt->pld.gnd_cmd.msg.update_init.size = size;
    pkt.lst_pkt->pld.gnd_cmd.msg.update_init.crc32 = crc;

    updater_start_init(&pkt);
}

void do_send_chunk(uint8_t *buf, uint32_t addr) {
    // Randomly skip some packets to simulate unreliable link
    float x = (float)rand() / (float)(RAND_MAX);
    if (x < LINK_DROP_RATE) return;

    packet_t pkt;
    openlst_packet_t lst_pkt;
    pkt.lst_pkt = &lst_pkt;

    memcpy(pkt.lst_pkt->pld.gnd_cmd.msg.update_chunk.data, buf + addr, 128);

    pkt.lst_pkt->pld.gnd_cmd.msg.update_chunk.addr = addr / 128;

    updater_write_chunk(&pkt);
}

void do_update(uint32_t update_size) {
    // Generate random update data
    uint8_t *update_data = malloc(update_size);

    for (uint32_t i = 0; i < update_size; i++) {
        update_data[i] = (uint8_t)rand();
    }

    // TODO: implement CRC here and in DMA sim, both are always 0 right now
    uint32_t ref_crc = 0;

    // Initialize
    do_updater_init(update_size, ref_crc);
    while (updater_try_init() == 0);

    TEST_ASSERT_EQUAL_UINT32(update_size,
                             sim_flash_read32(BL_UPDATE_HEADER_UPDATE_SIZE));
    TEST_ASSERT_EQUAL_UINT32(ref_crc, sim_flash_read32(BL_UPDATE_HEADER_CRC));

    // Send chunks
    int sent_count = 0;
    for (uint32_t i = 0; i < update_size; i += 128) {
        do_send_chunk(update_data, i);
        sent_count++;
    }

    int last_remaining = INT_MAX;
    int stuck = 0;

    // Send remaining chunks based on status messages
    while (1) {
        // Get status
        openlst_packet_t reply;
        updater_populate_status(&reply);

        int remaining = reply.pld.gnd_cmd.msg.update_status.chunks_remaining;

        // Done if no chunks are remaining
        if (remaining == 0) {
            break;
        }

        // Broke if remaining stayed the same (not technically but whatever)
        if (last_remaining == remaining) {
            stuck++;
        } else {
            stuck = 0;
        }

        if (stuck == 100) {
            TEST_FAIL_MESSAGE("Remaining chunks didn't decrease");
        }

        // Send chunks based on included list of addresses
        for (int i = 0; i < ((remaining > 96) ? 96 : remaining); i++) {
            uint16_t chunk_addr =
                reply.pld.gnd_cmd.msg.update_status.chunk_addr[i];
            do_send_chunk(update_data, chunk_addr << 7);
        }

        last_remaining = remaining;
    }

    // Verify update is correct
    TEST_ASSERT_EQUAL_HEX8_ARRAY(update_data, sim_flash_buf + BL_UPDATE_START,
                                 update_size);

    // Apply update: TODO

    // Clean up
    free(update_data);
}

void test_update_process() {
    // Start with empty flash
    flash_range_erase(0, PICO_FLASH_SIZE_BYTES);

    do_update(128);
    do_update(256);
    do_update(16*1024);

    for (int i=0; i<100; i++) {
        // Random update size
        uint32_t update_size = rand() % (512 * 1024);
        update_size -= update_size % 128;  // Make it a multiple of 128 bytes

        do_update(update_size);
    }
}

void test_updater() {
    test_update_status();
    test_update_process();
}
