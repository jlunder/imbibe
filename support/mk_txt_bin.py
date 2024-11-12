#!/usr/bin/python

__appname__ = "mk_menu_cfg"
__author__ = "Joseph Lunderville <joe.lunderville@gmail.com>"
__version__ = "0.1"


import argparse
import dataclasses
import logging
import os
from pathlib import Path
import struct
import sys
from typing import BinaryIO, Iterable

from ansi import DosAnsi
from sauce import *


logger = logging.getLogger(__appname__)


@dataclasses.dataclass
class Args:
    verbose: bool = False
    input_path: Path = None
    output_path: Path = None
    default_attr: int = 0x07
    page_width: int = 80


def make_arg_parser():
    parser = argparse.ArgumentParser(
        description="Convert a text file to a BINTEXT .bin file"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="verbose message output"
    )
    parser.add_argument(
        "-o",
        "--output",
        dest="output_path",
        metavar="BINTEXT",
        type=Path,
        help="converted BINTEXT file",
    )
    parser.add_argument(
        "input_path", metavar="TEXT", type=Path, help="text file to convert"
    )

    return parser


arg_parser: argparse.ArgumentParser = make_arg_parser()


def tab_dist(pos: int, tab_size: int = 8) -> int:
    return tab_size - (pos % tab_size)


def convert_simple_wrap(args: Args, inf: BinaryIO) -> Iterable[bytes]:
    TAB = ord("\t")
    SPACE = ord(" ")
    CR = ord("\r")
    NUL = 0
    LF = ord("\n")
    EOF = 26
    attr = args.default_attr
    page_width = args.page_width
    line = b""
    for c in inf.read():
        pos = len(line) // 2
        assert pos < page_width
        if c == TAB:
            expanded = bytes([SPACE, attr]) * tab_dist(pos)
        elif c in [CR, NUL]:
            expanded = b""
        elif c == LF:
            expanded = bytes([SPACE, attr]) * (page_width - pos)
        elif c == EOF:
            break
        else:
            expanded = bytes([c, attr])
        line += expanded
        while len(line) >= page_width * 2:
            yield line[: page_width * 2]
            line = line[page_width * 2 :]
    if len(line) > 0:
        yield line + bytes([SPACE, attr]) * (page_width - len(line) // 2)


# def convert_fancy_wrap(args: Args, input_path: Path, output_path: Path):
#     with open(output_path, "wb") as outf:
#         wrap_attr = args.default_attr
#         page_width = args.page_width
#         for l in open(args.input_path, "rb").readlines():
#             bin_line = b''
#             last_word_end = 0
#             this_word_start = 0
#             did_wrap = False
#             in_word = False
#             def flush_line():
#                 pass
#             for c in l:
#                 pos = len(bin_line) // 2
#                 if c in '\r\n\x00':
#                     # ignore these chars
#                     pass
#                 elif c in '\t ':
#                     # whitespace
#                     if in_word:
#                         if pos >= page_width:

#                     in_word = False
#                     if c == '\t':
#                         ws += ' ' * (8 - (pos % 8))
#                     else:
#                         ws = ' '
#                     if pos + len(ws) >= page_width:
#                         flush_line

#                     bin_line += (b' ' + cur_attr) *


def convert_ansi(args: Args, data: bytes) -> Iterable[bytes]:
    da = DosAnsi(cols=args.page_width, logger=logger)
    da.feed(data)
    b = da.tiles.tobytes()
    for i in range(0, len(b), args.page_width * 2):
        yield b[i : i + args.page_width * 2]


def main(args: Args):
    if args.output_path != None:
        output_path = args.output_path
    else:
        output_path = args.input_path.with_suffix(".bin")
        if output_path.resolve() == args.input_path.resolve():
            logger.error(
                "Output must be specified -- default would overwrite input file"
            )
            return None
    if os.path.exists(output_path) and os.path.samefile(output_path, args.input_path):
        logger.warning("Output path is the same file as input path")

    assert args.page_width % 2 == 0
    with open(args.input_path, "rb") as inf:
        data = inf.read()
        sauce = parse_sauce(data, logger=logger)
        gen = convert_ansi(args, data)

        with open(output_path, "wb") as outf:
            file_size = 0
            lines = 0

            for l in gen:
                assert len(l) == args.page_width * 2
                file_size += len(l)
                lines += 1
                outf.write(l)

            if not (sauce is None):
                flags = sauce.flags
            else:
                flags = TextFlags.FONT_9PX | TextFlags.ASPECT_LEGACY

            outf.write(make_bintext_sauce(file_size, args.page_width, flags))


if __name__ == "__main__":
    try:
        args: Args = arg_parser.parse_args(namespace=Args())
        logger.setLevel(logging.INFO if args.verbose else logging.DEBUG)
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter("%(message)s"))
        logger.addHandler(handler)

        res = main(args)

        sys.exit(res)

    except KeyboardInterrupt as e:  # Ctrl-C
        raise e

    except SystemExit as e:  # sys.exit()
        raise e

    except Exception as e:
        logger.exception("Conversion failed due to exception:")
        sys.exit(2)
