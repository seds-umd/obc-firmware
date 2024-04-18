#!/usr/bin/env python

from __future__ import annotations

from obc import Obc
from obc_commands import ObcCmds

from openlst_tools.utils import unpack_cint, pack_cint

# Modified from: https://gist.github.com/Lauszus/6c787a3bc26fea6e842dfb8296ebd630
# Setting should match crc32.c
def crc_poly(data, n=32, poly=0x04C11DB7, crc=0xFFFFFFFF):
    g = 1 << n | poly  # Generator polynomial

    # Loop over the data
    for d in data:
        # XOR the top byte in the CRC with the input byte
        crc ^= d << (n - 8)

        # Loop over all the bits in the byte
        for _ in range(8):
            # Start by shifting the CRC, so we can check for the top bit
            crc <<= 1

            # XOR the CRC if the top bit is 1
            if crc & (1 << n):
                crc ^= g

    # Return the CRC value
    return crc

class Updater:
    def __init__(self, obc: Obc) -> None:
        self.obc = obc

    def do_update(self, image: bytes):
        if len(image) % 128 != 0:
            image = bytearray(image)
            image.extend([0] * len(image) % 128)

        # TODO
    
    def _init(self, image: bytes):
        msg = bytearray()

        msg.extend(pack_cint(len(image), 4, False))
        msg.extend(pack_cint(crc_poly(image), 4, False))

        self.obc.obc_cmd(ObcCmds.UPDATE_INIT, msg, False)
        # TODO: manually check for ACK since it will take a long time

    def _send_chunk(self, addr: int, chunk: bytes):
        assert len(chunk) == 128

        msg = bytearray()

        msg.extend(pack_cint(addr, 2, False))
        msg.extend(chunk)

        self.obc.obc_cmd(ObcCmds.UPDATE_CHUNK, msg, False)
    
    def _get_status(self):
        pass
