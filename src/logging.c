#include <string.h>

#include "openlst.h"
#include "command_formats.h"

void log_msg(const char *msg) {
    openlst_packet_t *pkt;

    do {
        pkt = openlst_get_tx_buffer();
    } while (pkt == NULL);

    int len = strlen(msg);

    pkt->len = len + OPENLST_HEADER_SIZE + 1;
    pkt->pld.buf[0] = 0x02;
    memcpy(pkt->pld.buf + 1, msg, len);
    pkt->hdr.command = ASCII;
    pkt->hdr.seq = openlst_get_seq();

    openlst_tx(pkt);
}
