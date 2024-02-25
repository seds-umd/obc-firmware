#include "commands.h"

#include "command_handler.h"
#include "logging.h"
#include "openlst.h"

#include <string.h>

void command_setup() {
    command_init();

    // Misc
    command_register(0x01, command_ping);
}

int command_ping(packet_t *pkt) {
    openlst_packet_t *reply = openlst_get_tx_buffer();

    // TODO: what should address be? broadcast for now
    reply->hdr.hwid = 0x0000;

    reply->hdr.seq = pkt->lst_pkt->hdr.seq;
    reply->hdr.system = 0x01;
    reply->hdr.command = ASCII;

    // Reply with original message
    memcpy(reply->pld.buf, pkt->lst_pkt->pld.buf, pkt->lst_pkt->len);

    openlst_tx(reply);

    return 0;
}
