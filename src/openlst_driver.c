#include "openlst_driver.h"

#include "config.h"

#include "hardware/gpio.h"
#include "pico/time.h"

static absolute_time_t when_power_cycled;

void openlst_driver_init() {
    gpio_init(OPENLST_PWR_PIN);
    gpio_set_dir(OPENLST_PWR_PIN, true);
    gpio_put(OPENLST_PWR_PIN, false);
}

void openlst_driver_process() {
    if (gpio_get(OPENLST_PWR_PIN) == true) {
        if (absolute_time_diff_us(when_power_cycled, get_absolute_time()) >
            OPENLST_POWER_CYCLE_DELAY_US) {
            gpio_put(OPENLST_PWR_PIN, false);
        }
    }
}

void openlst_power_cycle() {
    gpio_put(OPENLST_PWR_PIN, true);

    when_power_cycled = get_absolute_time();
}
