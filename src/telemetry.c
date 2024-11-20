#include "telemetry.h"
#include "logging.h"
#include "amux.h"
#include "pico/time.h"


static struct telem_struct telem;

void telem_update() {
    //Set telem age
    telem.telem_age_ms = to_ms_since_boot(get_absolute_time()) - telem.uptime_ms;
    telem.uptime_ms = to_ms_since_boot(get_absolute_time());

    // Update analog telemetry
    telem.v_mb_batt_mv = read_and_convert(4);
    telem.v_mb_3v3_mv = read_and_convert(5);
    telem.v_mb_4v2_mv = read_and_convert(8);

    telem.i_mb_3v3_gps_ma = read_and_convert(0);
    telem.i_mb_3v3_obc_ma = read_and_convert(1);
    telem.i_mb_batt_ma = read_and_convert(6);
    telem.i_mb_3v3_lst_ma = read_and_convert(9);
    telem.i_mb_4v2_lst_ma = read_and_convert(10);

    telem.t_obc0_cc = read_and_convert(3);
    telem.t_obc1_cc = read_and_convert(2);
    telem.t_lst0_cc = read_and_convert(11);
    telem.t_lst1_cc = read_and_convert(12);

    telem.t_rp2040_cc = readrp2040Temp();

    // Update OpenLST telemetry
    

    //Boot Count
    //Requires read to filesystem
    
}

struct telem_struct *telem_get() {
    telem_update();
    return &telem;
}
