#include "hardware/gpio.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "command_handler.h"
#include "commands.h"
#include "logging.h"
#include "scheduler.h"
#include "openlst.h"

int main() {
    // Debug only, for printf
    stdio_init_all();
    sleep_ms(1000);

    // First run of PRNG takes longer than normal because it has to generate a
    // seed so we run this first to get it out of the way.
    get_rand_32();

    gpio_init(DEBUG_PIN);
    gpio_set_dir(DEBUG_PIN, true);
    gpio_put(DEBUG_PIN, false);

    openlst_init();

    scheduler_init();

    log_msg("booted");

    // 1024 byte buffer fills up in 88ms at 115200 baud
    scheduler_add_task(openlst_process, 50*1000);

    command_setup();

    while (1) {
        scheduler_run();
    }
}
