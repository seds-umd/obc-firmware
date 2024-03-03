#include "commands.h"

#include "command_handler.h"
#include "logging.h"
#include "macros.h"
#include "openlst.h"

#include "hardware/gpio.h"
#include "hardware/watchdog.h"

#include <string.h>

// Functions are declared as static because they should only ever be called
// by the command handler.
static int command_ping(packet_t *pkt);
static int command_reboot(packet_t *pkt);
static int command_gpio(packet_t *pkt);

void command_setup() {
    command_init();

    // Misc
    command_register(0x01, command_ping);
    command_register(0x03, command_reboot);

    // Hardware
    command_register(0x80, command_gpio);
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
