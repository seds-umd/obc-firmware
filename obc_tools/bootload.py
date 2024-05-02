#!/usr/bin/env python

import time
from pathlib import Path
from typing import TypedDict

from openlst_tools.handler import LstHandler, Packet
from openlst_tools.utils import unpack_cint, pack_cint
from updater import crc_poly, get_chunk


class BlCmds:
    PING = 0x00
    ACK = 0x01
    WRITE = 0x02
    STATUS_REQ = 0x03
    STATUS = 0x04
    HEADER = 0x05
    FORCE_BOOT = 0x06
    ERASE = 0x0C


class BlStatus(TypedDict):
    size: int
    crc_expected: int
    crc_actual: int
    match: int

    def decode(msg: bytes):
        msg = bytearray(msg)
        status = BlStatus()

        def pop(n):
            return bytes([msg.pop(0) for _ in range(n)])

        status["size"] = unpack_cint(pop(4), 4, False)
        status["crc_expected"] = unpack_cint(pop(4), 4, False)
        status["crc_actual"] = unpack_cint(pop(4), 4, False)
        status["match"] = unpack_cint(pop(1), 1, False)

        return status


class Bootloader(LstHandler):
    def __init__(
        self,
        port: str,
        hwid: int,
        baud: int = 115200,
        rtscts: bool = False,
        timeout: float = 1,
    ) -> None:
        super().__init__(port, hwid, baud, rtscts, timeout)

    def bl_cmd(self, cmd: int, msg: bytes = bytes()):
        seq = self._send(self.hwid, cmd, msg)
        reply = self.get_packet_timeout(seqnum=seq)
        assert reply["command"] == BlCmds.ACK, "Didn't receive ACK"
        assert reply["data"][0] == 0, "Received NACK"

    def ping(self):
        start = time.time()
        self.bl_cmd(BlCmds.PING)
        end = time.time()

        return end - start

    def write_chunk(self, addr: int, chunk: bytes):
        assert len(chunk) == 128

        msg = bytearray()

        msg.extend(pack_cint(addr, 2, False))
        msg.extend(chunk)

        self.bl_cmd(BlCmds.WRITE, msg)

    def get_status(self) -> BlStatus:
        seq = self._send(self.hwid, BlCmds.STATUS_REQ)
        reply = self.get_packet_timeout(seqnum=seq)
        status = BlStatus.decode(reply["data"])

        return status

    def write_header(self, size: int, crc: int):
        msg = bytearray()
        msg.extend(pack_cint(size, 4, False))
        msg.extend(pack_cint(crc, 4, False))

        self.bl_cmd(BlCmds.HEADER, msg)

    def force_boot(self):
        self.bl_cmd(BlCmds.FORCE_BOOT)

    def erase_all(self):
        self.bl_cmd(BlCmds.ERASE)

    def write_image(self, *, image: bytes = bytes(), path: str = ""):
        # Read from path if no image given
        if len(image) == 0:
            image = Path(path).read_bytes()

        # Pad with zeros to fit into chunks
        if len(image) % 256 != 0:
            image = bytearray(image)
            image.extend([0] * (256 - len(image) % 256))

        print("Erasing flash")
        self.erase_all()

        print("Writing header")
        self.write_header(len(image), crc_poly(image))

        print("Writing data")
        for i in range(len(image) >> 7):
            self.write_chunk(i, get_chunk(image, i))

        status = self.get_status()
        assert status["match"] == 1, "Image did not match"


if __name__ == "__main__":
    import click
    import IPython
    from traitlets.config import get_config

    @click.command()
    @click.option("--port", default=None, help="Serial port")
    @click.option("--image", type=click.Path(exists=True))
    def main(port, image):
        bl = Bootloader(port, 0, timeout=3)

        if image is not None:
            image = Path(image).read_bytes()

        with bl:
            if image is not None:
                bl.write_image(image=image)
            else:
                c = get_config()
                c.InteractiveShellEmbed.colors = "Linux"
                IPython.embed(config=c)

    main()
