#include "commands.h"

#include "bl_common.h"
#include "command_handler.h"
#include "flash.h"
#include "logging.h"
#include "macros.h"
#include "openlst.h"
#include "openlst_driver.h"
#include "updater.h"

#include "hardware/gpio.h"
#include "hardware/watchdog.h"

#include <string.h>

// Functions are declared as static because they should only ever be called
// by the command handler.
static int command_ping(packet_t *pkt);
static int command_reboot(packet_t *pkt);
static int command_gpio(packet_t *pkt);
static int command_flash(packet_t *pkt);
static int command_telem(packet_t *pkt);
static int command_openlst_pwr(packet_t *pkt);
static int command_update_read(packet_t *pkt);
static int command_deploy_antenna(packet_t *pkt);

void command_setup() {
    command_init();

    // Misc
    command_register(0x01, command_ping);
    command_register(0x03, command_reboot);

    // Telemetry
    command_register(0x10, command_telem);

    // Updater
    command_register(0x30, updater_start_init);
    command_register(0x31, updater_write_chunk);
    command_register(0x32, updater_send_status);
    command_register(0x34, updater_apply_update);
    command_register(0x35, command_update_read);

    // Control commands
    command_register(0x40, command_openlst_pwr);
    command_register(0x50, command_deploy_antenna);

    // Hardware
    command_register(0x80, command_gpio);

    // Drivers
    command_register(0xA0, command_flash);
}

static int command_deploy_antenna(packet_t *pkt) {
    UNUSED(pkt); 

    gpio_init(ANTENNA_DEPLOY_PIN);
    gpio_init(DEPLOYMENT_SWITCH);
    gpio_set_dir(ANTENNA_DEPLOY_PIN, GPIO_OUT);
    gpio_set_dir(DEPLOYMENT_SWITCH, GPIO_IN);
    
    int is_high = gpio_get(DEPLOYMENT_SWITCH);

    if (is_high) {
        return 1;
    }

    gpio_put(ANTENNA_DEPLOY_PIN, 1);
    sleep_ms(2000); 
    gpio_put(ANTENNA_DEPLOY_PIN, 0);

    return 0;
}

static int command_ping(packet_t *pkt) {
    openlst_packet_t *reply = openlst_get_tx_buffer();

    // TODO: what should address be? broadcast for now
    reply->hdr.hwid = 0x0000;

    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->hdr.system = 0x01;
    reply->hdr.command = ASCII;
    reply->len = pkt->lst_pkt->len;

    // Reply with original message
    memcpy(reply->pld.buf, pkt->lst_pkt->pld.buf, pkt->lst_pkt->len);

    openlst_tx(reply);

    return 0;
}

static int command_reboot(packet_t *pkt) {
    UNUSED(pkt);

    watchdog_reboot(0, 0, 0);

    // Should never get here
    return 0;
}

static int command_gpio(packet_t *pkt) {
    uint8_t op = pkt->lst_pkt->pld.gnd_cmd.msg.gpio.pin_op;
    uint32_t pins = pkt->lst_pkt->pld.gnd_cmd.msg.gpio.pin;

    // Functions without bitmask support
    for (int i = 0; i < 32; i++) {
        if ((pins >> i) & 0x1) {
            switch (op) {
                case 0x00:
                    gpio_deinit(i);
                    break;
                case 0x01:
                    gpio_init(i);
                    break;
                default:
                    break;
            }
        }
    }

    openlst_packet_t *reply;

    // Functions with bitmask support
    switch (op) {
        case 0x02:  // input
            gpio_set_dir_in_masked(pins);
            break;
        case 0x03:  // output
            gpio_set_dir_out_masked(pins);
            break;
        case 0x04:  // high
            gpio_put_masked(pins, 0xFFFFFFFF);
            break;
        case 0x05:  // low
            gpio_put_masked(pins, 0x00000000);
            break;
        case 0xFF:  // read
            reply = openlst_get_tx_buffer();

            reply->hdr.hwid = 0x0000;
            reply->hdr.seq = pkt->lst_pkt->hdr.seq;
            reply->hdr.system = 0x01;
            reply->hdr.command = ASCII;
            reply->len = 9 + OPENLST_HEADER_SIZE;

            reply->pld.gnd_cmd.opcode = 0x81;  // GPIO_STATE command
            reply->pld.gnd_cmd.msg.gpio_state.pin_mode = sio_hw->gpio_oe;
            reply->pld.gnd_cmd.msg.gpio_state.pin_state = sio_hw->gpio_in;

            openlst_tx(reply);
            break;

        default:
            break;
    }

    return 0;
}

static int command_flash(packet_t *pkt) {
    uint8_t op = pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.cmd;
    uint32_t addr;
    uint8_t size;
    openlst_packet_t *reply;

    // TODO: don't block if flash is busy
    switch (op) {
        // Read
        case 0x00:
            addr = flash_addr_conv(
                pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.read.addr);
            size = pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.read.size;

            // Ignore request with size too large
            // TODO: figure out actual max size
            if (size > OPENLST_MAX_PAYLOAD) {
                return 0;
            }

            reply = openlst_get_tx_buffer();
            reply->hdr.seq = pkt->lst_pkt->hdr.seq;
            reply->len = OPENLST_HEADER_SIZE + 2 + size;
            reply->pld.gnd_cmd.opcode = 0xA1;

            flash_wait_done();
            flash_read_bytes(
                addr, reply->pld.gnd_cmd.msg.flash_cmd.read_resp.data, size);
            reply->pld.gnd_cmd.msg.flash_cmd.cmd = 0x00;  // READ response

            openlst_tx(reply);
            break;

        // Program
        case 0x01:
            addr = flash_addr_conv(
                pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.program.addr);
            size = pkt->lst_pkt->len - OPENLST_HEADER_SIZE - 5;

            flash_wait_done();
            flash_write_bytes(
                addr, pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.program.data,
                size);
            break;

        // Erase
        case 0x02:
            addr = flash_addr_conv(
                pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.erase.addr);
            size = pkt->lst_pkt->pld.gnd_cmd.msg.flash_cmd.erase.size;

            flash_wait_done();
            if (size == 0x00) {
                flash_erase_4k(addr);
            } else if (size == 0x01) {
                flash_erase_32k(addr);
            } else if (size == 0x02) {
                flash_erase_64k(addr);
            }
            break;

        // Unique ID
        case 0x03:
            reply = openlst_get_tx_buffer();
            reply->hdr.seq = pkt->lst_pkt->hdr.seq;
            reply->len = OPENLST_HEADER_SIZE + 10;
            reply->pld.gnd_cmd.opcode = 0xA1;

            uint64_t id = flash_unique_id();
            reply->pld.gnd_cmd.msg.flash_cmd.unique_id_resp.unique_id = id;
            reply->pld.gnd_cmd.msg.flash_cmd.cmd = 0x01;  // UNIQUE_ID response

            openlst_tx(reply);
            break;

        default:
            break;
    }

    return 0;
}

static int command_telem(packet_t *pkt) {
    struct telem_struct *telem = telem_get();

    openlst_packet_t *reply = openlst_get_tx_buffer();
    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->len = OPENLST_HEADER_SIZE + 1 + sizeof(*telem);
    reply->pld.gnd_cmd.opcode = 0x11;

    memcpy(&reply->pld.gnd_cmd.msg.telem, telem, sizeof(*telem));

    openlst_tx(reply);

    return 0;
}

static int command_openlst_pwr(packet_t *pkt) {
    UNUSED(pkt);

    openlst_power_cycle();

    return 0;
}

static int command_update_read(packet_t *pkt) {
    uint32_t addr = pkt->lst_pkt->pld.gnd_cmd.msg.update_read.addr;

    openlst_packet_t *reply = openlst_get_tx_buffer();
    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->len = OPENLST_HEADER_SIZE + 1 + sizeof(reply->pld.gnd_cmd.msg.update_chunk);
    reply->pld.gnd_cmd.opcode = 0x31;

    // Copy data into packet
    memcpy(reply->pld.gnd_cmd.msg.update_chunk.data, flash_read + addr, 128);

    openlst_tx(reply);
}
