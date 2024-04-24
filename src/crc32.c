#include "hardware/dma.h"

#include <stdint.h>

uint32_t __not_in_flash_func(calc_crc32)(const volatile uint8_t *const buf,
                                         uint8_t size) {
    int chan = dma_claim_unused_channel(true);

    uint32_t dummy_write;

    // DMA setup
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    // CRC setup
    channel_config_set_sniff_enable(&c, true);
    dma_sniffer_set_data_accumulator(0xFFFFFFFF);  // CRC initial value
    dma_sniffer_enable(chan, DMA_SNIFF_CTRL_CALC_VALUE_CRC32, true);

    // Run DMA and wait for it to finish
    dma_channel_configure(chan, &c, &dummy_write, buf, size, true);
    dma_channel_wait_for_finish_blocking(chan);

    uint32_t crc = dma_sniffer_get_data_accumulator();

    dma_channel_unclaim(chan);

    return crc;
}
