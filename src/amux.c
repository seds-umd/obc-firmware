#define SELECT_0 22
#define SELECT_1 23
#define SELECT_2 24
#define SELECT_3 25
#define AMUX_IN 26

#define OBC_CURR 0
#define GPS_CURR 1
#define TEMP_OBC_1 2
#define TEMP_OBC_0 3
#define VOLTAG_BATT 4
#define VOLTAGE_3v3 5

#include "hardware/gpio.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdint.h>
#include "logging.h"
#include "amux.h"
#include <math.h>

void amux_init() {
    // https://github.com/raspberrypi/pico-examples/blob/master/adc/hello_adc/hello_adc.c
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    gpio_init(SELECT_0);
    gpio_init(SELECT_1);
    gpio_init(SELECT_2);
    gpio_init(SELECT_3);
    gpio_set_dir(SELECT_0, true);
    gpio_set_dir(SELECT_1, true);
    gpio_set_dir(SELECT_2, true);
    gpio_set_dir(SELECT_3, true);
}

void set_select(uint8_t select_number) {
    // slowly take the bits of the number with masking
    gpio_put(SELECT_3, (select_number >> 3) & 1);
    gpio_put(SELECT_2, (select_number >> 2) & 1);
    gpio_put(SELECT_1, (select_number >> 1) & 1);
    gpio_put(SELECT_0, select_number & 1);
}

void select_all() {
    uint32_t val;
    for (int i = 0; i < 16; i++) {
        set_select(i);
        sleep_ms(50);
        val = read_adc();
        log_fmt("value at channel %d: %f", i, val / 1000000.0);
        sleep_ms(50);
    }
}

uint32_t read_adc() {
    uint32_t conversion_factor = 3300000 / (1 << 12);
    uint32_t result = adc_read() * conversion_factor;
    return result;
}

void read_temp(int channel) {
    set_select(channel);
    sleep_ms(50);

    float R0 = 10000;
    float T0 = 273.15 + 25;  // kelvin
    float B = 3435;
    float out = adc_read() * 3.3 / (1 << 12);
    float R = out * R0 / (3.3 - out);
    log_fmt("resistance: %f", R);
    float T = 1 / (1.0 / T0 + 1.0 / B * logf(R / R0)) - 273.15;
    log_fmt("temp at channel %d: (volt) %f, (final) %f", channel, out, T);
}

uint16_t read_and_convert(uint8_t amux_input) {
    const float conversion_factor = 3.3f / (1 << 12);

    set_select(amux_input);
    // selecting which pin from amux to read from

    uint16_t result = adc_read();
    uint16_t voltage_in_mv = ((float)result * conversion_factor * 1000);

    // this block is for when reading from voltage sensors
    if (amux_input == 4 || amux_input == 8 || amux_input == 5) {
        return voltage_in_mv;
    }

    // this block is for when reading from current sensors
    if (amux_input == 0 || amux_input == 1 || amux_input == 9 ||
        amux_input == 10) {
        return getOutputCurrent((voltage_in_mv), amux_input);
    }

    // this block is for when reading from temperature senesors
    if (amux_input == 2 || amux_input == 3 ||
        (amux_input <= 15 && amux_input >= 11)) {
        const float B = 3435;
        const float R0 = 10000;
        const float T0 = 273.15 + 25;  // kelvin
        float out = result * 3.3 / (1 << 12);
        float R = out * R0 / (3.3 - out);

        return (1 / (1.0 / T0 + 1.0 / B * logf(R / R0)) - 273.15) * 100;
    }

    return -1.0;
}
