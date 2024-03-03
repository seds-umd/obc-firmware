#!/usr/bin/env python

from __future__ import annotations

import binascii
import logging
import struct
import time

from obc_commands import ObcCmds
from openlst_tools.commands import OpenLstCmds, MAX_DATA_LEN
from openlst_tools.handler import LstHandler, Packet
from openlst_tools.utils import unpack_cint, pack_cint

SHELL_HEADER = """\
OBC shell

Commands can be accessed through the `obc` object

Ex: `obc.ping()`"""


class Gpio:
    """Commands for controlling GPIO pins remotely."""

    def __init__(self, obc: Obc):
        self.obc = obc

    def init_mask(self, mask: int):
        """Initialize pins using bitmask.

        Args:
            mask (int): Pin mask
        """

        msg = bytearray()
        msg.extend(pack_cint(mask, 4, False))
        msg.append(0x01)

        self.obc.obc_cmd(ObcCmds.GPIO, msg)

    def init(self, pin: int):
        """Initialize a pin.

        Args:
            pin (int): Pin number
        """

        self.init_mask(1 << pin)

    def mode_mask(self, mask: int, mode: bool):
        """Set pin mode using bitmask.

        Args:
            mask (int): Pin mask
            mode (bool): Pin mode. True for output, False for input.
        """

        msg = bytearray()
        msg.extend(pack_cint(mask, 4, False))
        msg.append(0x03 if mode else 0x02)

        self.obc.obc_cmd(ObcCmds.GPIO, msg)

    def mode(self, pin: int, mode: bool, init: bool = True):
        """Set pin mode.

        Args:
            pin (int): Pin number
            mode (bool): Pin mode. True for output, False for input.
            init (bool, optional): Initialize pin before setting mode. Defaults to True.
        """

        if init:
            self.init(pin)

        self.mode_mask(1 << pin, mode)

    def set_mask(self, mask: int, value: bool):
        """Set pin output using bitmask. A single output value will be applied
        to all pins in the mask.

        Args:
            mask (int): Pin mask
            value (bool): Output value
        """

        msg = bytearray()
        msg.extend(pack_cint(mask, 4, False))
        msg.append(0x04 if value else 0x05)

        self.obc.obc_cmd(ObcCmds.GPIO, msg)

    def set(self, pin: int, value: bool):
        """Set pin output.

        Args:
            pin (int): Pin number
            value (bool): Output value
        """

        self.set_mask(1 << pin, value)

    def get_all(self):
        """Get mode and state of all pins.

        Returns:
            tuple(int, int): Tuple of (mode, state) where each bit represents a pin
        """

        msg = bytearray()
        msg.extend([0x00] * 4)  # Pin field is ignored on read commands
        msg.append(0xFF)

        reply: Packet = self.obc.obc_cmd(ObcCmds.GPIO, msg, True)

        pin_mode = unpack_cint(reply["data"][1:5], 4, False)
        pin_state = unpack_cint(reply["data"][5:9], 4, False)

        return pin_mode, pin_state

    def get_value(self, pin: int) -> bool:
        """Read the value of a pin. Works for both inputs and outputs.

        Args:
            pin (int): Pin number

        Returns:
            bool: Pin state
        """

        assert pin >= 0 and pin < 32, f"Invalid pin {pin}"

        _, state = self.get_all()

        return bool((state >> pin) & 0x1)

    def get_mode(self, pin: int) -> bool:
        """Get pin mode. Probably broken right now.

        Args:
            pin (int): Pin number

        Returns:
            bool: Pin mode
        """

        assert pin >= 0 and pin < 32, f"Invalid pin {pin}"

        mode, _ = self.get_all()

        return bool((mode >> pin) & 0x1)


class Obc(LstHandler):
    def __init__(
        self,
        port: str,
        hwid: int,
        baud: int = 115200,
        rtscts: bool = False,
        timeout: float = 1,
    ) -> None:
        super().__init__(port, hwid, baud, rtscts, timeout)

        self.gpio = Gpio(self)

    def obc_cmd(self, opcode: int, data: bytes = bytes(), resp: bool = False):
        # Send command in OBC command format, returns response or sequence
        # number if response not expected

        assert opcode >= 0 and opcode < 256, "Command opcode invalid"

        msg = bytearray()
        msg.append(opcode)
        msg.extend(data)

        seq = self._send(self.hwid, OpenLstCmds.ASCII, msg)

        if resp:
            return self.get_packet_timeout(seqnum=seq)
        else:
            return seq

    def ping(self, msg: bytes = bytes()):
        start = time.time()
        reply = self.obc_cmd(ObcCmds.PING, msg, True)
        end = time.time()

        print(f"Received reply after {end - start:3f} seconds")

        return reply

    def reboot(self) -> int:
        return self.obc_cmd(ObcCmds.REBOOT)


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
