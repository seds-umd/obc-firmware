#include "config.h"
#include "flash.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

int dma_channel = -1;

void fpga_config_init() {
    // Configuring SPI pins
    spi_init(FPGA_LOADER_SPI, FPGA_LOADER_SCK_RATE * 1000);
    gpio_set_function(FPGA_LOADER_RX, GPIO_FUNC_SPI);
    gpio_set_function(FPGA_LOADER_SCK, GPIO_FUNC_SPI);
    gpio_set_function(FPGA_LOADER_TX, GPIO_FUNC_SPI);
    gpio_init(FPGA_LOADER_CS);
    gpio_set_dir(FPGA_LOADER_CS, GPIO_OUT);
    gpio_init(FPGA_LOADER_PROGRAM_B);
    gpio_set_dir(FPGA_LOADER_PROGRAM_B, GPIO_OUT);
    gpio_init(FPGA_LOADER_INIT_B);
    gpio_set_dir(FPGA_LOADER_INIT_B, false);  // input
}

void dma_handler() {
    if (!gpio_get(FPGA_LOADER_INIT_B)) return;

    absolute_time_t start = get_absolute_time();
    absolute_time_t timeout = delayed_by_ms(start, 100);  // 100 ms timeout

    while (!gpio_get(FPGA_LOADER_DONE)) {
        if (absolute_time_diff_us(get_absolute_time(), timeout) >= 0) {
            gpio_put(FPGA_LOADER_CS, 1);
            dma_channel_unclaim(dma_channel);
            return;  // timeout occurred
        }
    };  // Wait for done

    // Special start condition
    uint8_t dummy = 0;
    spi_write_blocking(FPGA_LOADER_SPI, &dummy, 1);
    gpio_put(FPGA_LOADER_CS, 1);
    dma_channel_unclaim(dma_channel);
}

int fpga_config_start(uint8_t* data) {
    // Unused DMA channel

    gpio_put(FPGA_LOADER_PROGRAM_B, 0);
    sleep_ms(1000);  // 1 second delay
    gpio_put(FPGA_LOADER_PROGRAM_B, 1);

    absolute_time_t start = get_absolute_time();
    absolute_time_t timeout = delayed_by_ms(start, 100);  // 100 ms timeout

    while (!gpio_get(FPGA_LOADER_INIT_B)) {
        if (absolute_time_diff_us(get_absolute_time(), timeout) >= 0) {
            return 1;  // timeout occurred
        }
    }

    dma_channel = dma_claim_unused_channel(false);

    // No dma channel found
    if (dma_channel < 0) {
        return 1;
    }

    dma_channel_config c = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(FPGA_LOADER_SPI, true));
    dma_channel_configure(dma_channel, &c, &spi_get_hw(FPGA_LOADER_SPI)->dr,
                          data, FPGA_BITSTREAM_SIZE, false);

    // Setting interrupts
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Start the transfer
    gpio_put(FPGA_LOADER_CS, 0);
    dma_channel_start(dma_channel);
    return 0;
}
