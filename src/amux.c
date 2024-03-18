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

void amux_init() 
{
    //https://github.com/raspberrypi/pico-examples/blob/master/adc/hello_adc/hello_adc.c
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    gpio_init(SELECT_0);//Don't know if gpio_init will take this since it's not uint, will find out in testing
    gpio_init(SELECT_1);
    gpio_init(SELECT_2);
    gpio_init(SELECT_3);
    gpio_set_dir(SELECT_0, true);
    gpio_set_dir(SELECT_1, true);
    gpio_set_dir(SELECT_2, true);
    gpio_set_dir(SELECT_3, true);
}

void set_select(uint8_t select_number){
    //initialize the select lines to their respective bits
    uint32_t select[4] = { (select_number>>3), ((select_number%8)>>2),
                            ((select_number%4)>>1), (select_number%2) };

    //slowly take the bits of the number with masking
    gpio_put(SELECT_0, select[0]);
    gpio_put(SELECT_1, select[1]);
    gpio_put(SELECT_2, select[2]);
    gpio_put(SELECT_3, select[3]);
}

void select_all()
{
    uint16_t val;
    for (int i = 0; i < 16; i++)
    {
        set_select(i);
        sleep_ms(50);
        val = read_adc();
        log_fmt("value at channel %d: %f", i, val);
        sleep_ms(50);
    }
}

uint16_t read_adc()
{
    const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    return result * conversion_factor;
}