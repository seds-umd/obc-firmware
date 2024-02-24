#include "hardware/gpio.h"
#include "pico/rand.h"

#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "command_handler.h"
#include "scheduler.h"
#include "openlst.h"

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
    scheduler_add_task(openlst_process, 50000);

    // Commands
    command_init();
    command_register(0x01, NULL); // TODO: handle ping

    while (1) {
        scheduler_run();
    }
}
