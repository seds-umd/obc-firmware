#!/usr/bin/env python

from __future__ import annotations

from typing import TypedDict

from obc import Obc
from obc_commands import ObcCmds
from openlst_tools.handler import Packet
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

class UpdateStatus(TypedDict):
    status: int
    crc_match: bool
    crc_expected: int
    chunks_remaining: int
    chunk_addr: list

    def decode(msg: bytes):
        msg = bytearray(msg)

        status = UpdateStatus()

        def pop(n):
            return bytes([msg.pop(0) for _ in range(n)])
        
        status["status"] = unpack_cint(pop(1), 1, True) # TODO: error string
        status["crc_match"] = unpack_cint(pop(1), 1, False)
        status["crc_expected"] = unpack_cint(pop(4), 4, False)
        status["chunks_remaining"] = unpack_cint(pop(2), 2, False)

        addrs = []
        while len(msg) >= 2: addrs.append(unpack_cint(pop(2), 2, False))
        status["chunk_addr"] = addrs

        return status

class Updater:
    def __init__(self, obc: Obc) -> None:
        self.obc = obc

    def do_update(self, image: bytes):
        # Pad with zeros to fit into chunks
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
        reply: Packet = self.obc.obc_cmd(ObcCmds.UPDATE_STATUS_REQ, resp=True)

        return UpdateStatus.decode(reply["data"][1:])
