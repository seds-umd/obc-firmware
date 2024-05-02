#include "pico/stdlib.h"

// #define FLASH_PAGE_SIZE 256
// #define FLASH_SECTOR_SIZE 4096

#define FLASH_CMD_PAGE_PROGRAM 0x02
#define FLASH_CMD_READ 0x03
#define FLASH_CMD_READ_FAST 0x0B
#define FLASH_CMD_STATUS 0x05
#define FLASH_CMD_WRITE_EN 0x06
#define FLASH_CMD_SECTOR_ERASE 0x20
#define FLASH_CMD_32K_ERASE 0x52
#define FLASH_CMD_64K_ERASE 0xD8
#define FLASH_CMD_CHIP_ERASE 0xC7
#define FLASH_CMD_POWER_DOWN 0xB9
#define FLASH_CMD_POWER_UP 0xAB
#define FLASH_CMD_JEDEC_ID 0x9F
#define FLASH_CMD_UNIQUE_ID 0x4B
#define FLASH_CMD_READ_SR1 0x05
#define FLASH_CMD_READ_SR2 0x35
#define FLASH_CMD_READ_SR3 0x15
#define FLASH_CMD_WRITE_SR3 0x11
// No definitions for writing SR1 and SR2 to prevent accidental writing to OTP bits

#define FLASH_STATUS_BUSY_MASK 0x01

// Take 3 address bytes from buffer and convert to uint32_t
static inline uint32_t flash_addr_conv(uint8_t *buf) {
    uint32_t addr = 0;

    addr |= buf[0];
    addr |= (buf[1] << 8);
    addr |= (buf[2] << 16);

    return addr;
}

/**
 * @brief Set up flash chip.
 * 
 * @param cs Flash chip select pin
 */
void flash_setup(uint cs_pin);

/**
 * @brief Read bytes from the flash chip.
 * 
 * @param addr 24 bit address to start read at
 * @param buf Pointer to store bytes at
 * @param len Number of bytes to read
 */
void __not_in_flash_func(flash_read_bytes)(uint32_t addr, uint8_t *buf,
                                           size_t len);

/**
 * @brief Write bytes to the flash chip.
 * 
 * If a write crosses a page boundary (256 bytes), it will wrap around to the
 * beginning of the page.
 * 
 * @param addr 24 bit address to start write at
 * @param buf Pointer to read bytes from
 * @param len Number of bytes to write
 */
void __not_in_flash_func(flash_write_bytes)(uint32_t addr, uint8_t *buf,
                                            size_t len);

/**
 * @brief Erase 4 kB sector of flash.
 * 
 * @param addr 24 bit address of sector
 */
void __not_in_flash_func(flash_erase_4k)(uint32_t addr);

/**
 * @brief Erase 32 kB block of flash.
 * 
 * @param addr 24 bit address of block
 */
void __not_in_flash_func(flash_erase_32k)(uint32_t addr);

/**
 * @brief Erase 64 kB block of flash.
 * 
 * @param addr 24 bit address of block
 */
void __not_in_flash_func(flash_erase_64k)(uint32_t addr);

/**
 * @brief Erase entire flash chip.
 */
void __not_in_flash_func(flash_erase_chip)();

// Power down
void __not_in_flash_func(flash_power_down)();

// Power up
void __not_in_flash_func(flash_power_up)();

// Check if busy
int __not_in_flash_func(flash_is_busy)();

// Wait for done
void __not_in_flash_func(flash_wait_done)();

// Write enable
void __not_in_flash_func(flash_write_enable)();

// Get JEDEC ID
void flash_get_id(uint8_t *mf_id, uint16_t *dev_id);

// Get unique ID
uint64_t flash_unique_id();

uint32_t flash_read_status();
