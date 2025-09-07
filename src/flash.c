// Code modified from pico-examples
#include "flash.h"

#include "config.h"

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/spi.h"

static uint cs_pin;
static bool fast = false;

/** \brief Assert CS pin to select flash chip.
 *
 * Taken from Pico examples, not sure what FIXME comment is about.
 */
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");  // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");  // FIXME
}

/** \brief De-assert CS pin to select flash chip.
 *
 * Taken from Pico examples, not sure what FIXME comment is about.
 */
static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");  // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");  // FIXME
}

/** \brief Helper function to send a single byte SPI command.
 *
 * \param cmd Command byte to be sent
 */
static inline void single_cmd(uint8_t cmd) {
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, &cmd, 1);
    cs_deselect();
}

void flash_setup(uint cs) {
    cs_pin = cs;

    // Enable SPI at 62.5 MHz and connect to GPIOs
    uint actual = spi_init(DATA_FLASH_SPI, DATA_FLASH_BAUD);
    spi_set_format(DATA_FLASH_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(DATA_FLASH_RX, GPIO_FUNC_SPI);
    gpio_set_function(DATA_FLASH_SCK, GPIO_FUNC_SPI);
    gpio_set_function(DATA_FLASH_TX, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(cs_pin);
    gpio_put(cs_pin, 1);
    gpio_set_dir(cs_pin, GPIO_OUT);

    // Use fast reads if faster than 50 MHz
    if (actual > 50 * 1000 * 1000) {
        fast = true;
    }
}

void flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len) {
    uint8_t cmdbuf[5];

    if (fast) {
        cmdbuf[0] = FLASH_CMD_READ_FAST;
        cmdbuf[4] = 0;
    } else {
        cmdbuf[0] = FLASH_CMD_READ;
    }

    cmdbuf[1] = addr >> 16;
    cmdbuf[2] = addr >> 8;
    cmdbuf[3] = addr;

    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, fast ? 5 : 4);
    spi_read_blocking(DATA_FLASH_SPI, 0, buf, len);
    cs_deselect();
}

void flash_write_bytes(uint32_t addr, uint8_t *buf, size_t len) {
    uint8_t cmdbuf[4];

    cmdbuf[0] = FLASH_CMD_PAGE_PROGRAM;
    cmdbuf[1] = addr >> 16;
    cmdbuf[2] = addr >> 8;
    cmdbuf[3] = addr;

    flash_write_enable();
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 4);
    spi_write_blocking(DATA_FLASH_SPI, buf, len);
    cs_deselect();
}

void flash_erase_4k(uint32_t addr) {
    uint8_t cmdbuf[4];

    cmdbuf[0] = FLASH_CMD_SECTOR_ERASE;
    cmdbuf[1] = addr >> 16;
    cmdbuf[2] = addr >> 8;
    cmdbuf[3] = addr;

    flash_write_enable();
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 4);
    cs_deselect();
}

void flash_erase_32k(uint32_t addr) {
    uint8_t cmdbuf[4];

    cmdbuf[0] = FLASH_CMD_32K_ERASE;
    cmdbuf[1] = addr >> 16;
    cmdbuf[2] = addr >> 8;
    cmdbuf[3] = addr;

    flash_write_enable();
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 4);
    cs_deselect();
}

void flash_erase_64k(uint32_t addr) {
    uint8_t cmdbuf[4];

    cmdbuf[0] = FLASH_CMD_64K_ERASE;
    cmdbuf[1] = addr >> 16;
    cmdbuf[2] = addr >> 8;
    cmdbuf[3] = addr;

    flash_write_enable();
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 4);
    cs_deselect();
}

void flash_erase_chip() { single_cmd(FLASH_CMD_CHIP_ERASE); }

void flash_power_down() { single_cmd(FLASH_CMD_POWER_DOWN); }

void flash_power_up() { single_cmd(FLASH_CMD_POWER_UP); }

int flash_is_busy() {
    uint8_t cmdbuf[2];

    cmdbuf[0] = FLASH_CMD_STATUS;
    cmdbuf[1] = 0;

    cs_select();
    // Write happens before read so we can reuse the same buffer
    spi_write_read_blocking(DATA_FLASH_SPI, cmdbuf, cmdbuf, 2);
    cs_deselect();

    return cmdbuf[1] & FLASH_STATUS_BUSY_MASK;
}

void flash_wait_done() {
    while (flash_is_busy()) {
        // tight_loop_contents();
        sleep_us(1);
    }
}

void flash_write_enable() { single_cmd(FLASH_CMD_WRITE_EN); }

void flash_get_id(uint8_t *mf_id, uint16_t *dev_id) {
    uint8_t cmdbuf[1];

    cmdbuf[0] = FLASH_CMD_JEDEC_ID;
    uint8_t id_buf[3];

    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 1);
    spi_read_blocking(DATA_FLASH_SPI, 0, id_buf, 3);
    cs_deselect();

    *mf_id = id_buf[0];
    *dev_id = (id_buf[1] << 8) | id_buf[2];
}

uint64_t flash_unique_id() {
    uint8_t cmdbuf[8];

    // One instruction byte and 4 dummy bytes
    cmdbuf[0] = FLASH_CMD_UNIQUE_ID;

    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 5);
    spi_read_blocking(DATA_FLASH_SPI, 0, cmdbuf, 8);
    cs_deselect();

    uint64_t id = ((uint64_t)cmdbuf[0] << 56) | ((uint64_t)cmdbuf[1] << 48) |
                  ((uint64_t)cmdbuf[2] << 40) | ((uint64_t)cmdbuf[3] << 32) |
                  ((uint64_t)cmdbuf[4] << 24) | ((uint64_t)cmdbuf[5] << 16) |
                  ((uint64_t)cmdbuf[6] << 8) | ((uint64_t)cmdbuf[7] << 0);

    return id;
}

uint32_t flash_read_status() {
    uint8_t cmdbuf[1];
    uint32_t status = 0;

    cmdbuf[0] = FLASH_CMD_READ_SR1;
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 1);
    spi_read_blocking(DATA_FLASH_SPI, 0, cmdbuf, 1);
    cs_deselect();
    status |= cmdbuf[0];

    cmdbuf[0] = FLASH_CMD_READ_SR2;
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 1);
    spi_read_blocking(DATA_FLASH_SPI, 0, cmdbuf, 1);
    cs_deselect();
    status |= cmdbuf[0] << 8;

    cmdbuf[0] = FLASH_CMD_READ_SR3;
    cs_select();
    spi_write_blocking(DATA_FLASH_SPI, cmdbuf, 1);
    spi_read_blocking(DATA_FLASH_SPI, 0, cmdbuf, 1);
    cs_deselect();
    status |= cmdbuf[0] << 16;

    return status;
}
