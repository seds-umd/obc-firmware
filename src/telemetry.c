#include "telemetry.h"
#include "logging.h"

#include "pico/time.h"
#include "amux.h"

static struct telem_struct telem;

void telem_update() {
    telem.telem_age_ms = to_ms_since_boot(get_absolute_time()) - telem.uptime_ms;
    telem.uptime_ms = to_ms_since_boot(get_absolute_time());

    // Update analog telemetry
    telem.v_mb_batt_mv = read_and_convert(4);
    telem.v_mb_3v3_mv = read_and_convert(5);
    telem.v_mb_4v2_mv = read_and_convert(8);

    telem.i_mb_3v3_gps_ma = read_and_convert(0);
    telem.i_mb_3v3_obc_ma = read_and_convert(1);
    
    telem.t_obc0_cc = read_and_convert(3);
    telem.t_obc1_cc = read_and_convert(2);

    // Update OpenLST telemetry
}

struct telem_struct *telem_get() {
    // Set telem age
    telem.telem_age_ms =
        to_ms_since_boot(get_absolute_time()) - telem.uptime_ms;
    telem.uptime_ms = to_ms_since_boot(get_absolute_time());

    return &telem;
}
