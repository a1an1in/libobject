#!/usr/bin/env python

"""Parse log file content to event messages
"""

from __future__ import annotations
from dataclasses import dataclass

from pathlib import Path

LOG_FILE = "events_shuo.log"
LOG_FILE = "afry_shuo.log"


class Number:
    """Number representation"""

    value: int
    base: int

    def __str__(self) -> str:
        """Override magic method __str__

        Returns:
            str: number string
        """
        match self.base:
            case 16:
                return self.hex
            case 8:
                return self.oct
            case 2:
                return self.bin
        return str(self.value)

    def __repr__(self) -> str:
        """Override magic method __repr__

        Returns:
            str: number string
        """
        return self.__str__()

    def __init__(
        self, value: Number | str | int, width: int = 32, base: int = 10
    ) -> None:
        if isinstance(value, Number):
            str_value = str(value.value)
        else:
            str_value = str(value)

        str_value = str_value.replace(" ", "")

        if str_value.startswith("0x") or str_value.startswith("0h"):
            self.base = 16
            self.value = int(str_value[2:], 16)
        elif str_value.startswith("0o"):
            self.base = 8
            self.value = int(str_value[2:], 8)
        elif str_value.startswith("0b"):
            self.base = 2
            self.value = int(str_value[2:], 2)
        else:
            self.base = base
            self.value = int(str_value, base=self.base)

        self.width = width

    @property
    def hex(self) -> str:
        """Return hexadecimal value as string

        Returns:
            str: hexadecimal value string
        """
        sign, hex_str = hex(self.value).split("0x", maxsplit=1)
        return f"{sign}0x{hex_str.upper().rjust(int(self.width/4), '0')}"

    @property
    def oct(self) -> str:
        """Return octave value as string

        Returns:
            str: octave value string
        """
        sign, oct_str = oct(self.value).split("0o", maxsplit=1)
        return f"{sign}0o{oct_str.upper().rjust(int(self.width/3), '0')}"

    @property
    def bin(self) -> str:
        """Return binary value as string

        Returns:
            str: binary value string
        """
        sign, bin_str = bin(self.value).split("0b", maxsplit=1)
        return f"{sign}0b{bin_str.rjust(int(self.width), '0')}"

    def __add__(self, another_number: Number | str | int) -> Number:
        """Add Number with Number|str|int

        Args:
            another_number (Number | str | int): _description_

        Raises:
            TypeError: Raised when an invalid type object is added to Number

        Returns:
            Number: _description_
        """
        if isinstance(another_number, (str, int)):
            number_obj = Number(another_number)
        elif isinstance(another_number, Number):
            number_obj = another_number
        else:
            raise TypeError("Number object can only add Number|str|int")

        return Number(self.value + number_obj.value)

    def __iadd__(self, another_number: Number | str | int) -> Number:
        return self.__add__(another_number)


@dataclass
class EventRegister:
    """Event register class"""

    high: Number | None = None
    low: Number | None = None

    _data: Number | None = None

    REGISTER_DEFINITION = {
        0: "Locking",
        1: "Unlocking",
        2: "Reserved",
        3: "Reserved",
        4: "evt_etile_rst_assert_wait",
        5: "evt_etile_rst_deassert_wait",
        6: "etile_rst_restart",
        7: "fec_adapt tx block lock enter",
        8: "fec_adapt tx block lock leave",
        9: "fec_adapt tx_block lock lost enter",  # overridden by j*16 + 25
        10: "fec_adapt tx_block lock lost leave",  # overridden by j*16 + 26
        11: "fec_adapt tx synch header error enter",  # overridden by j*16 + 27
        12: "fec_adapt tx synch header error leave",  # overridden by j*16 + 29
        13: "fec_adapt rx overflow",
        14: "fec_adapt rx overflow unlocking",
        15: "fec_adapt rx underrun",
        16: "fec_adapt rx_underrun unlocking",
        17: "evt_freqlock",
        18: "evt_frequnlock",
        19: "evt_link_fail",
        20: "evt_ical_sig_ok",
        21: "evt_ical_det_ok",
        22: "evt_ical_det_start",
        23: "MCPRI locking",
        24: "MCPRI unlocking",
        25: "CDC underflow",  # j*16 + 25: 41 overrides 32 + 9
        26: "CDC underflow",  # j*16 + 26: 42 overrides 32 + 10
        27: "CDC overflow",  # j*16 + 27: 43 overrides 32 + 11
        28: "CDC overflow",  # j*16 + 29: 44 overrides 32 + 12
        29: "SW 0",
        30: "SW 1",
        31: "SW 2",
    }

    def __post_init__(self):
        self.register_definition = {}
        for k, v in EventRegister.REGISTER_DEFINITION.items():
            if isinstance(v, str):
                self.register_definition[k] = ("...", v)
            else:
                self.register_definition[k] = v

    @property
    def data(self) -> Number:
        """Construct all data when low or high is updated

        Raises:
            ValueError: Raised when high or low part is None

        Returns:
            Number: full 64-bit data
        """
        if self.high is None:
            raise ValueError("High part is None.")
        if self.low is None:
            raise ValueError("Low part is None.")
        data_num = self.high.value << 32
        data_num += self.low.value
        # print("high", self.high.hex, self.high.bin)
        # print("low ", self.low.hex, self.low.bin)
        self._data = Number(data_num, width=64, base=10)
        return self._data

    def parse(self) -> list[str]:
        """Parse event bits to markdown table

        Returns:
            list[str]: List of event messages
        """
        skips = ["...", "evt_etile_rst_deassert_wait"]

        raw_binary = self.data.bin[2:]
        result_str = []
        result_str += [
            f"event binary: {raw_binary}",
        ]

        for index in range(32):
            flow_0_index = 31 - index
            flow_1_index = 63 - index
            flow_0_value = int(raw_binary[63 - flow_0_index])
            flow_1_value = int(raw_binary[63 - flow_1_index])
            flow_0_description = self.register_definition[flow_0_index][flow_0_value]
            flow_1_description = self.register_definition[flow_1_index - 32][
                flow_1_value
            ]
            if flow_0_description not in skips:
                result_str += [f"flow 0: {flow_0_index} {flow_0_description}"]
            if flow_1_description not in skips:
                result_str += [f"flow 1: {flow_1_index} {flow_1_description}"]

        return result_str


def parse_log(log_file: Path | str = LOG_FILE) -> list[str]:
    """Parse log file

    Args:
        log_file (Path | str, optional): Log file path. Defaults to LOG_FILE.

    Returns:
        list[str]: Parsed event messages
    """
    result = []
    current_register = EventRegister()
    with open(log_file, "r+", encoding="utf-8", errors="ignore") as fp:
        for line in fp.readlines():
            if not (line := line.strip()):
                continue
            addr_str, data_str, cycles, seconds = line.replace(" ", "").split(":")
            data_str = data_str[2:]
            addr = Number(addr_str, width=32, base=16).hex[-2:]
            current_register.high = Number(data_str[:32], width=32, base=2)
            current_register.low = Number(data_str[32:], width=32, base=2)
            result += [f"Time: {seconds} second(s), {cycles} cycle(s), index {addr}"]
            result += current_register.parse()
    return result


if __name__ == "__main__":
    parsed_log = parse_log()
    print("\n".join(parsed_log))
