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

enum CommandState {
    IDLE,
    START,
    LENGTH,
    HWID1,
    HWID2,
    SEQ1,
    SEQ2,
    SYS,
    CMD,
    DATA,
};

enum BootloaderCommands {
    BL_PING = 0x00,
    BL_ACK = 0x01,
    BL_WRITE = 0x02,
    BL_STATUS_REQ = 0x03,
    BL_STATUS = 0x04,
    BL_HEADER = 0x05,
    BL_FORCE_BOOT = 0x06,
    BL_ERASE = 0x0C,
};

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

// ack is 0 if command was successful
void send_ack(int ack, uint16_t seq) {
    uint8_t buf[] = {
        // Start bytes
        OPENLST_START_0,
        OPENLST_START_1,
        // Length (initialized later)
        0,
        // HWID
        0x00,
        0x00,
        // Sequence
        seq & 0xFF,
        (seq >> 8) & 0xFF,
        // System
        0x01,
        // Command
        BL_ACK,
        // Payload
        ack & 0xFF,
    };

    buf[2] = sizeof(buf) / sizeof(uint8_t) - 3;

    for (uint8_t i = 0; i < sizeof(buf); i++) {
        uart_putc_raw(PIB_UART_ID, buf[i]);
    }
}

void send_status(uint16_t seq) {
    uint32_t size = get_application_size();
    uint32_t crc_expected = 0;
    uint32_t crc_actual = 0;

    // Don't check CRC if it's too big
    if (size < 16 * 1024 * 1024) {
        crc_expected = get_application_crc();
        crc_actual = calc_crc32(flash_read + BL_APP_START, size);
    }

    uint8_t match = (crc_actual == crc_expected) ? 1 : 0;

    uint8_t buf[] = {
        // Start bytes
        OPENLST_START_0,
        OPENLST_START_1,
        // Length (initialized later)
        0,
        // HWID
        0x00,
        0x00,
        // Sequence
        seq & 0xFF,
        (seq >> 8) & 0xFF,
        // System
        0x01,
        // Command
        BL_ACK,
        // Payload
        size & 0xFF,
        (size >> 8) & 0xFF,
        (size >> 16) & 0xFF,
        (size >> 24) & 0xFF,
        crc_expected & 0xFF,
        (crc_expected >> 8) & 0xFF,
        (crc_expected >> 16) & 0xFF,
        (crc_expected >> 24) & 0xFF,
        crc_actual & 0xFF,
        (crc_actual >> 8) & 0xFF,
        (crc_actual >> 16) & 0xFF,
        (crc_actual >> 24) & 0xFF,
        match,
    };

    buf[2] = sizeof(buf) / sizeof(uint8_t) - 3;

    for (uint8_t i = 0; i < sizeof(buf); i++) {
        uart_putc_raw(PIB_UART_ID, buf[i]);
    }

    // Update valid byte and boot if valid
    if (match) {
        set_application_valid(8);
        sleep_ms(25);
        boot();
    }
}

int parse_command(uint8_t cmd, uint16_t seq, uint8_t *payload) {
    switch (cmd) {
        case BL_PING:
            send_ack(0, seq);
            return 1;
            break;

        case BL_WRITE:;
            // Decode address
            uint16_t addr = payload[0];
            addr |= payload[1] << 8;

            // Write data
            int ret = write_chunk(BL_APP_START, addr, payload + 2);

            if (ret == 0) {
                send_ack(0, seq);
            } else {
                send_ack(1, seq);
            }
            return 1;
            break;

        case BL_STATUS_REQ:
            send_status(seq);
            return 1;
            break;

        case BL_HEADER:;
            uint8_t buf[256];

            memset(buf, 0xFF, 256);
            memcpy(buf, payload, 8);  // Packet and header layout is the same
            flash_range_program(BL_APP_HEADER_START, (uint8_t *)buf, 256);

            // Set valid to indicate header is written
            set_application_valid(2);

            send_ack(0, seq);
            return 1;
            break;

        case BL_FORCE_BOOT:
            // Send ACK, wait for it to finish, then boot
            send_ack(0, seq);
            sleep_ms(5);
            boot();
            return 1;
            break;

        case BL_ERASE:
            // Erase header
            flash_range_erase(BL_APP_HEADER_START, BL_APP_HEADER_SIZE);

            // Erase application slot
            flash_range_erase(BL_APP_START, BL_APP_SIZE);

            // Set valid to indicate slot is erased
            set_application_valid(1);

            send_ack(0, seq);
            return 1;
            break;

        default:
            // Unrecognized command, return NACK
            send_ack(1, seq);
            break;
    }

    return 1;
}

// Return 1 to reset timeout, 0 otherwise
int parse_uart(uint8_t c) {
    static enum CommandState state = IDLE;

    static uint8_t length;
    static uint16_t hwid;
    static uint16_t seq;
    static uint8_t cmd;

    static uint8_t payload[OPENLST_MAX_PAYLOAD];
    static uint8_t payload_ptr;

    switch (state) {
        case IDLE:
            if (c == OPENLST_START_0) state = START;

            length = 0;
            payload_ptr = 0;
            break;

        case START:
            if (c == OPENLST_START_1)
                state = LENGTH;
            else
                state = IDLE;
            break;

        case LENGTH:
            length = c;
            state = HWID1;
            break;

        case HWID1:
            hwid = c;
            state = HWID2;
            break;

        case HWID2:
            hwid |= c << 8;
            state = SEQ1;
            break;

        case SEQ1:
            seq = c;
            state = SEQ2;
            break;

        case SEQ2:
            seq |= c << 8;
            state = SYS;
            break;

        case SYS:
            state = CMD;
            break;

        case CMD:
            cmd = c;
            state = DATA;

            // Empty payload
            if (length == 6) {
                state = IDLE;
                return parse_command(cmd, seq, payload);
            }
            break;

        case DATA:
            payload[payload_ptr++] = c;

            if (payload_ptr == length - 6) {
                state = IDLE;
                return parse_command(cmd, seq, payload);
            }
            break;

        default:
            state = IDLE;
            break;
    }

    return 0;
}

int main() {
    // Configure PIB UART
    uart_init(PIB_UART_ID, PIB_UART_BAUD);
    gpio_set_function(PIB_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIB_UART_RX, GPIO_FUNC_UART);
    uart_set_hw_flow(PIB_UART_ID, false, false);
    uart_set_format(PIB_UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(PIB_UART_ID, true);

    // Add pulldowns to OpenLST UART to prevent false data
    gpio_set_pulls(OPENLST_UART_TX, true, false);
    gpio_set_pulls(OPENLST_UART_RX, true, false);

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
        int ret = parse_uart(uart_getc(PIB_UART_ID));

        if (ret == 1) {
            boot_at = make_timeout_time_ms(timeout_ms);
        }
    }

    // Boot
    boot();
}
