#!/usr/bin/env python

from __future__ import annotations
import subprocess
import shlex

TEST = True
BASE_ADDRESS = "0x0300 0000"


def run_cmd(cmd: str | list[str]) -> str:
    """Run command and return stderr + stdout

    Args:
        cmd (str | list[str]): Command in string or list of string

    Raises:
        TypeError: Raised when wrong type of cmd is provided
        RuntimeError: Raised when cmd has non-zero return code

    Returns:
        str: Stderr + stdout
    """
    if isinstance(cmd, str):
        cmds = shlex.split(cmd)
    elif isinstance(cmd, list):
        cmds = cmd
    else:
        raise TypeError(f"cmd with type {type(cmd)} is not supported.")

    if TEST:
        print(" ".join(cmds))
        return ""

    result = subprocess.run(cmds, capture_output=True, encoding="utf-8", check=False)
    if result.returncode:
        raise RuntimeError(result.stderr)
    return result.stderr + "\n" + result.stdout


class Number:

    value: int

    def __init__(self, value: Number | str | int, width: int = 32) -> None:
        if isinstance(value, Number):
            str_value = str(value.value)
        else:
            str_value = str(value)

        str_value = str_value.replace(" ", "")

        if str_value.startswith("0x") or str_value.startswith("0h"):
            self.value = int(str_value[2:], 16)
        elif str_value.startswith("0o"):
            self.value = int(str_value[2:], 8)
        elif str_value.startswith("0b"):
            self.value = int(str_value[2:], 2)
        else:
            self.value = int(str_value, 10)

        self.width = width

    @property
    def hex(self) -> str:
        sign, hex_str = hex(self.value).split("0x", maxsplit=1)
        return f"{sign}0x{hex_str.upper().rjust(int(self.width/4), '0')}"

    @property
    def bin(self) -> str:
        sign, bin_str = bin(self.value).split("0b", maxsplit=1)
        return f"{sign}0x{bin_str.rjust(int(self.width), '0')}"

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


def read(address: Number | str | int, read_one_word: bool = True) -> str:
    """Read address

    Args:
        address (Number | str | int): Address to read
        read_one_word (bool, optional): Read only one word otherwise read a block of 32 words. Defaults to True.

    Returns:
        str: stderr + stdout
    """
    cmd = f"fpga read {Number(address).hex}"
    if read_one_word:
        cmd += " -32"
    return run_cmd(cmd)


def write(address: Number | str | int, value: Number | str | int) -> str:
    """Write one word.

    Args:
        address (Number | str | int): Address to write.
        value (Number | str | int): Value to write.

    Returns:
        str: stderr + stdout
    """
    cmd = f"fpga write {Number(address).hex} {Number(value).hex}"
    return run_cmd(cmd)


base_address = Number(BASE_ADDRESS)
control_reg = base_address + "0x5000"
mask_lo = base_address + "0x5008"
mask_hi = base_address + "0x500C"
reset_counters = Number("0b10100")
start_writing = Number("0b01001")
all_ones = Number("0hFFFFFFFF")
index_reg = base_address + "0x5010"

def restart_event_writing():
    """Restart event writing by stop writing, stop counting and write all ones to mask
    """
    read(control_reg)
    write(control_reg, 0)
    write(mask_lo, all_ones)
    write(mask_hi, all_ones)
    write(control_reg, reset_counters)
    write(control_reg, start_writing)
    read(control_reg)


def event_address(current_idx: Number | str | int) -> Number:
    """Translate current index read from IADDR_EVT_CURRENT_IDX

    Args:
        current_idx (Number | str | int): Current index

    Returns:
        Number: Address to read for the event with current index
    """
    shifted = Number(current_idx).hex + "00"
    width = Number(0).width
    if isinstance(current_idx, Number):
        width = current_idx.width
    return Number(shifted, width) + base_address + "0x38000"


def read_event_reg(current_idx: Number | str | int) -> str:
    """Read the event register with current_idx

    Args:
        current_idx (Number | str | int): Index of the event

    Returns:
        str: stderr + stdout
    """
    return read(event_address(current_idx))

if __name__ == "__main__":
    restart_event_writing()
    read(index_reg)
