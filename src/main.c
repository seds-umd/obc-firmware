#include "hardware/gpio.h"
#include "pico/rand.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "command_handler.h"
#include "commands.h"
#include "scheduler.h"
#include "openlst.h"

void send_packets() {
    char *s = "hello";

    openlst_packet_t *pkt;

    do {
        pkt = openlst_get_tx_buffer();
    } while (pkt == NULL);

    pkt->len = 5 + OPENLST_HEADER_SIZE;
    memcpy(pkt->pld.buf, s, 5);

    openlst_tx(pkt);
}

int main() {
    // First run of PRNG takes longer than normal because it has to generate a
    // seed so we run this first to get it out of the way.
    get_rand_32();

    printf("Hello\n");

    gpio_init(DEBUG_PIN);
    gpio_set_dir(DEBUG_PIN, true);
    gpio_put(DEBUG_PIN, false);

    openlst_init();

    scheduler_init();

    // 1024 byte buffer fills up in 88ms at 115200 baud
    scheduler_add_task(openlst_process, 50*1000);

    scheduler_add_task(send_packets, 1*1000*1000);

    command_setup();

    while (1) {
        scheduler_run();
    }
}
