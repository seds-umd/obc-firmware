#!/usr/bin/env python

import binascii
import logging
import struct

from openlst_tools.commands import OpenLstCmds, MAX_DATA_LEN
from openlst_tools.handler import LstHandler
from openlst_tools.utils import unpack_cint, pack_cint

from obc_commands import ObcCmds

SHELL_HEADER = """\
OBC shell

Commands can be accessed through the `obc` object

Ex: `obc.ping()`"""

class Obc(LstHandler):
    def __init__(self, port: str, hwid: int, baud: int = 115200, rtscts: bool = False, timeout: float = 1) -> None:
        super().__init__(port, hwid, baud, rtscts, timeout)

    def obc_cmd(self, opcode: int, msg: bytes = bytes(), resp: bool = False):
        # Send command in OBC command format, returns response or sequence
        # number if response not expected

        assert opcode >= 0 and opcode < 256, "Command opcode invalid"

        msg = bytes([opcode] + list(msg))
        seq = self._send(self.hwid, OpenLstCmds.ASCII, msg)

        if resp:
            return self.get_packet_timeout(seqnum=seq)
        else:
            return seq

    def ping(self, msg: bytes = bytes()):
        return self.obc_cmd(ObcCmds.PING, msg, True)

    def reboot(self) -> int:
        return self.obc_cmd(ObcCmds.REBOOT)

    def gpio_get_all(self) -> int:
        reply = self.obc_cmd(ObcCmds.GPIO, bytes([0xFF, 0xFF, 0xFF, 0xFF, 0xFF]))

    def gpio_get(self, pin) -> bool:
        pass

if __name__ == "__main__":
    import click
    import IPython
    from traitlets.config import get_config

    @click.command()
    @click.option("--port", default=None, help="Serial port")
    @click.option(
        "--rtscts", is_flag=True, default=False, help="Use RTS/CTS flow control"
    )
    @click.argument("hwid")
    def main(hwid, port, rtscts):
        logging.basicConfig(level="INFO")

        hwid = binascii.unhexlify(hwid)
        hwid = struct.unpack(">H", hwid)[0]

        obc = Obc(port, hwid, rtscts=rtscts)

        with obc:
            c = get_config()
            c.InteractiveShellEmbed.colors = "Linux"
            IPython.embed(header=SHELL_HEADER, config=c)

    main()
