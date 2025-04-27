#include "config.h"
#include "command_handler.h"
#include "commands.h"
#include "flash.h"
#include "logging.h"
#include "openlst.h"
#include "openlst_driver.h"
#include "scheduler.h"
#include "telemetry.h"
#include "updater.h"

#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint16_t seq;

int main() {
    watchdog_enable(WATCHDOG_TIMEOUT_MS, true);

    // First run of PRNG takes longer than normal because it has to generate a
    // seed so we run this first to get it out of the way.
    get_rand_32();

    gpio_init(DEBUG_PIN);
    gpio_set_dir(DEBUG_PIN, true);
    gpio_put(DEBUG_PIN, false);

    openlst_driver_init();

    flash_setup(DATA_FLASH_CS);
    openlst_init();

    scheduler_init();
    seq = get_rand_32();

    log_fmt("Booted. Git hash: %s. Compiled at %s %s", GIT_HASH, __TIME__, __DATE__);

    // Check if last reset was due to watchdog
    if (watchdog_caused_reboot()) {
        log_msg("Watchdog caused last reboot.");
    }

    // Check if update was applied
    if (watchdog_hw->scratch[0] == UPDATER_REBOOT_MAGIC) {
        log_msg("Update applied successfully.");
    }

    watchdog_hw->scratch[0] = 0;

    // 1024 byte buffer fills up in 88ms at 115200 baud
    scheduler_add_task(openlst_process, 50 * 1000);

    // Update telemetry every second
    scheduler_add_task(telem_update, 1 * 1000 * 1000);

    // Set to 1s for now, if this needs to do anything more complicated we can
    // decrease this
    scheduler_add_task(openlst_driver_process, 1 * 1000 * 1000);

    // Updater
    scheduler_add_task(updater_process, 100 * 1000);
    scheduler_add_task(telemetry_beacon, 5*1000*1000 );

    command_setup();
    

    while (1) {
        // Update watchdog every loop
        watchdog_update();

        // Run any scheduled tasks
        scheduler_run();
    }
}

void telemetry_beacon(){  
    packet_t temp_pkt;
    temp_pkt->lst_pkt->hdr.seq = seq;
    seq=seq+1;
    command_telem(temp_pkt);
}