#include "config.h"
#include "command_handler.h"
#include "commands.h"
#include "flash.h"
#include "logging.h"
#include "openlst.h"
#include "scheduler.h"
#include "telemetry.h"

#include "hardware/gpio.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main() {
    // First run of PRNG takes longer than normal because it has to generate a
    // seed so we run this first to get it out of the way.
    get_rand_32();

    gpio_init(DEBUG_PIN);
    gpio_set_dir(DEBUG_PIN, true);
    gpio_put(DEBUG_PIN, false);

    flash_setup(DATA_FLASH_CS);
    openlst_init();

    scheduler_init();

    log_msg("booted");

    // 1024 byte buffer fills up in 88ms at 115200 baud
    scheduler_add_task(openlst_process, 50 * 1000);

    // Update telemetry every second
    scheduler_add_task(telem_update, 1 * 1000 * 1000);

    command_setup();

    while (1) {
        scheduler_run();
    }
}
