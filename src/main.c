#include <stdint.h>
#include <stdio.h>
#include "hardware/gpio.h"

#include "config.h"
#include "scheduler.h"
#include "openlst.h"

int main() {
    printf("Hello\n");

    gpio_init(DEBUG_PIN);
    gpio_set_dir(DEBUG_PIN, true);
    gpio_put(DEBUG_PIN, false);

    openlst_init();

    scheduler_init();

    // 1024 byte buffer fills up in 88ms at 115200 baud
    scheduler_add_task(openlst_process, 50000);

    while (1) {
        scheduler_run();
    }
}
