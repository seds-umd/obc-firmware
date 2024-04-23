#include "bl_common.h"
#include "config.h"
#include "crc32.h"

#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/resets.h"
#include "hardware/watchdog.h"
#include "pico/time.h"
#include "RP2040.h"

#include <stdint.h>

void boot() {
    // Reset peripherals
    // https://github.com/usedbytes/rp2040-serial-bootloader/blob/f6f0d61fc03447a1401d3f9d158f35331bf0eb98/main.c#L70
    reset_block(~(RESETS_RESET_IO_QSPI_BITS | RESETS_RESET_PADS_QSPI_BITS |
                  RESETS_RESET_SYSCFG_BITS | RESETS_RESET_PLL_SYS_BITS));

    // Jump to application
    // https://github.com/usedbytes/rp2040-serial-bootloader/blob/f6f0d61fc03447a1401d3f9d158f35331bf0eb98/main.c#L80
    uint32_t vtor = XIP_BASE + BL_APP_START;
    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);

    SCB->VTOR = (volatile uint32_t)(vtor);

    asm volatile("msr msp, %0" ::"g"(*(volatile uint32_t *)vtor));
    asm volatile("bx %0" ::"r"(reset_vector));
}

// Returns 1 if app is valid, 0 if not
int ready_to_boot() {
    uint32_t app_size = get_application_size();

    // Check valid byte
    if (get_application_valid() != 8) {
        return 0;
    }

    // Check CRC
    uint32_t crc_expected = get_application_crc();
    uint32_t crc_actual = calc_crc32(flash_read + BL_APP_START, app_size);

    if (crc_expected != crc_actual) {
        return 0;
    }

    // Check watchdog magic
    if (watchdog_hw->scratch[0] == BL_MAGIC) {
        return 0;
    }

    return 1;
}

// Return 1 to reset timeout, 0 otherwise
int process_commands() {
    // TODO

    return 0;
}

int main() {
    // Configure PIB UART
    uart_init(PIB_UART_ID, PIB_UART_BAUD);
    gpio_set_function(PIB_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIB_UART_RX, GPIO_FUNC_UART);

    // Run checks to see if image is ready to boot
    int ready = ready_to_boot();

    // Set boot timeout to 500ms by default
    uint32_t timeout_ms = 500;

    if (!ready) {
        timeout_ms = UINT32_MAX;
    }

    // Process commands until timeout
    absolute_time_t boot_at = make_timeout_time_ms(timeout_ms);

    while (absolute_time_diff_us(boot_at, get_absolute_time()) < 0) {
        if (!(uart_is_readable(PIB_UART_ID))) {
            continue;
        }

        // Process uart data
        int ret = process_commands();

        if (ret == 1) {
            boot_at = make_timeout_time_ms(timeout_ms);
        }
    }
    sleep_ms(1000);

    // Boot
    boot();
}
