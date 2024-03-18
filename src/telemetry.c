#include "telemetry.h"
#include "logging.h"

#include "pico/time.h"

static struct telem_struct telem;

void telem_update() {
    telem.uptime_ms = to_ms_since_boot(get_absolute_time());

    // Update analog telemetry

    // Update OpenLST telemetry
}

struct telem_struct *telem_get() {
    // Set telem age
    telem.telem_age_ms =
        to_ms_since_boot(get_absolute_time()) - telem.uptime_ms;
    telem.uptime_ms = to_ms_since_boot(get_absolute_time());

    return &telem;
}
