#include "logging.h"

#include "command_formats.h"
#include "openlst.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void log_msg(const char *msg) {
    openlst_packet_t *pkt = openlst_get_tx_buffer();;

    // Drop log message rather than blocking forever
    if (pkt == NULL) {
        return;
    }

    int len = strlen(msg);

    pkt->len = len + OPENLST_HEADER_SIZE + 1;
    pkt->pld.buf[0] = 0x02;
    memcpy(pkt->pld.buf + 1, msg, len);
    pkt->hdr.command = ASCII;
    pkt->hdr.seq = openlst_get_seq();

    openlst_tx(pkt);
}

int log_fmt(const char *format, ...) {
    char buf[250];

    va_list va;
    va_start(va, format);
    const int ret = vsnprintf(buf, 250, format, va);
    va_end(va);

    log_msg(buf);

    return ret;
}
