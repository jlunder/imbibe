#!/usr/bin/python3 -O

__appname__ = "mk_tbm"
__author__ = "Joseph Lunderville <joe.lunderville@gmail.com>"
__version__ = "0.1"


import ast
import argparse
from enum import IntEnum, StrEnum
import dataclasses
import logging
import os
import numpy as np
import re
import struct
import sys

from ansi import *
from equivs import *
from rle import *
from sauce import *

logger = logging.getLogger(__appname__)


class ImageEnc(StrEnum):
    DEFAULT = ""
    AUTO = "auto"
    FLAT = "flat"
    RLE = "rle"
    XBIN = "xbin"
    MASK_FLAT = "mask_flat"
    MASK_KEY = "mask_key"
    MASK_RLE = "mask_rle"
    MASK_XBIN = "mask_xbin"


class InputType(StrEnum):
    DETECT = "detect"
    ASCII = "ascii"
    ANSI = "ansi"
    BINTEXT = "bintext"
    TBM = "tbm"
    OTHER = "other"


class TbmFormat(IntEnum):
    FLAT = 0x0100
    RLE = 0x0200
    XBIN = 0x0600
    MASK_FLAT = 0x0300
    MASK_KEY = 0x0400
    MASK_RLE = 0x0500
    MASK_XBIN = 0x0700


# 1 megabyte is *quite enough* for the old computers we're targeting
MAX_INPUT_SIZE = 1 << 20


@dataclasses.dataclass
class Args:
    verbose: bool = False
    normalize: bool = False
    output_subtype: str = ImageEnc.DEFAULT
    input_format: str = InputType.DETECT
    input_width: int = None
    key: str = None
    fill: str = None
    opaque: bool = False
    input_path: str = None
    output_path: str = None


@dataclasses.dataclass
class ImageInfo:
    format: str = InputType.OTHER
    data_size: int = None
    width: int = 80
    height: int | None = None
    flags: int = 0
    args: list[str] | None = None


@dataclasses.dataclass
class Image:
    flags: int = 0
    data: np.ndarray = None
    mask: np.ndarray = None
    mask_key: int = None


def make_arg_parser():
    parser = argparse.ArgumentParser(
        description="Convert a BinText (ANSI .BIN file) to a .TBM"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="verbose message output"
    )
    parser.add_argument(
        "-n",
        "--normalize",
        action="store_true",
        help="output a normalized BinText",
    )
    parser.add_argument(
        "-t",
        "--type",
        dest="output_subtype",
        metavar="SUBTYPE",
        choices=[
            "auto",
            "flat",
            "rle",
            "xbin",
            "mask_flat",
            "mask_key",
            "mask_rle",
            "mask_xbin",
        ],
        help="output specific TBM subtype",
    )
    parser.add_argument("-k", "--key", type=str, help="attribute key for masked BINs")
    parser.add_argument(
        "-p", "--opaque", action="store_true", help="don't generate a mask"
    )
    parser.add_argument(
        "-o",
        "--output",
        dest="output_path",
        metavar="OUTPUT",
        help="output TBM path",
    )
    parser.add_argument("input_path", metavar="INPUT", nargs=1, help="input BIN path")

    return parser


arg_parser: argparse.ArgumentParser = make_arg_parser()


def parse_args_key(args_key: str | None) -> tuple[int, int]:
    if (args_key == None) or (args_key == ""):
        return (0, -1)
    ch, fg, bg = args_key.split(",")
    key_mask = 0
    key = 0
    ch = ch.strip()
    if ch != "" and ch != "*":
        ch = ast.literal_eval(ch)
        if type(ch) == str:
            ch = ord(ch)
        else:
            ch = int(ch)
        key |= ch & 0xFF
        key_mask |= 0x00FF
    if fg != "" and fg != "*":
        fg = ast.literal_eval(fg)
        key |= (fg << 8) & 0x0F00
        key_mask |= 0x0F00
    if bg != "" and bg != "*":
        bg = ast.literal_eval(bg)
        key |= (bg << 12) & 0xF000
        key_mask |= 0xF000
    return key_mask, key


def read_image(args: Args, input_path: str) -> Image:
    logger.info("Opening %s", input_path)
    try:
        with open(input_path, "rb") as f:
            data = f.read(MAX_INPUT_SIZE)
    except Exception as e:
        logger.error("%s", e)
        return None
    sauce = parse_sauce(data)
    if sauce is not None:
        equals_arg_re = re.compile("^(--?[a-zA-Z_-]+)=(.*)$")
        if (len(sauce.title) > 0) and (sauce.title[-1] == ":"):
            img_args = []
            for a in sauce.title.split(":")[1:-1]:
                m = equals_arg_re.match(a)
                if m:
                    img_args.append(m.group(1))
                    img_args.append(m.group(2))
                else:
                    img_args.append(a)
        else:
            img_args = None
        if sauce.format == Format.ASCII:
            img_format = InputType.ASCII
        elif sauce.format == Format.ANSI:
            img_format = InputType.ANSI
        elif sauce.format == Format.BINTEXT:
            img_format = InputType.BINTEXT
        else:
            img_format = InputType.DETECT
        inf = ImageInfo(
            format=img_format,
            data_size=sauce.data_size,
            width=sauce.width,
            height=sauce.height,
            flags=sauce.flags,
            args=img_args,
        )
    else:
        _, ext = os.path.splitext(input_path)
        if ext.lower() == ".bin":
            inf = ImageInfo(format=InputType.BINTEXT)
        else:
            inf = ImageInfo(format=InputType.ANSI)
        logger.warning(
            f"Input file '{input_path}' does not have valid SAUCE, assuming {inf.format} based on extension '{ext}'",
        )
    if inf.args != None:
        logger.info("Parsing extra image args: %r", inf.args)
        inf_ns = arg_parser.parse_args(inf.args + [input_path], Args())
        args.fill = inf_ns.fill if args.fill is None else args.fill
        args.key = inf_ns.key if args.key is None else args.key
        args.opaque = inf_ns.opaque if args.opaque is None else args.opaque
        args.input_width = (
            inf_ns.input_width if args.input_width is None else args.input_width
        )
        args.input_format = (
            inf_ns.input_format if args.input_format is None else args.input_format
        )

    if inf.format == InputType.BINTEXT:
        inf, img_data = parse_bintext_data(inf, data)
    elif inf.format == InputType.ASCII or inf.format == InputType.ANSI:
        fill = None
        if args.fill:
            _, fill = parse_args_key(args.fill)
        inf, img_data = parse_ansi_data(inf, fill, data)
    else:
        assert not "WTF format?"

    key = None
    mask = np.ones(img_data.shape, dtype=np.bool_)
    if args.key:
        key_mask, key = parse_args_key(args.key)
        logger.info("Using key value %04x, mask %04x", key, key_mask)
        if key_mask == 0:
            logger.warning(
                "The key mask is 0, which means it will match anything "
                + "and your output will be transparent"
            )
        mask = np.vectorize(lambda x: int((x & key_mask) != key), [np.uint16])(img_data)
    return Image(flags=inf.flags, data=img_data, mask_key=key, mask=mask)


def parse_bintext_data(inf: ImageInfo, data: bytes) -> tuple[ImageInfo, np.ndarray]:
    if not inf.data_size:
        inf.data_size = len(data)
    if not inf.width:
        inf.width = min(80, inf.data_size // 2)
        logger.info("No width specified, guessing %d", inf.width)
    if inf.width * inf.height * 2 > inf.data_size:
        logger.error(
            "Specified size %dx%d exceeds available data (%d)",
            inf.width,
            inf.height,
            inf.data_size,
        )
        inf.height = 0
    if not inf.height:
        inf.height = inf.data_size // (2 * inf.width)
        logger.info("No width specified, guessing %d", inf.height)
    logger.info("Reading %dx%d BINTEXT", inf.width, inf.height)
    img_data = np.reshape(
        np.frombuffer(data, dtype=np.uint16, count=inf.width * inf.height),
        (inf.height, inf.width),
    )
    return (inf, img_data)


def parse_ansi_data(
    inf: ImageInfo, fill: int | None, data: bytes
) -> tuple[ImageInfo, np.ndarray]:
    if not inf.width:
        inf.width = 80
        logger.info("No width specified, guessing %d", inf.width)

    da = DosAnsi(cols=inf.width, fill=fill)
    da.feed(data)
    if not inf.height:
        inf.height = da.rows_used
        assert da.tiles.shape[0] == inf.width * max(inf.height, da.screen_rows)
    size = inf.width * inf.height
    if da.tiles.shape[0] < size:
        da.tiles = np.concatenate(
            [
                da.tiles,
                np.full(size - da.tiles.shape[0], 0 | da.attribute, dtype=np.uint16),
            ]
        )
    tiles = np.reshape(da.tiles[: inf.width * inf.height], (inf.height, inf.width))
    assert da.cols == inf.width
    assert da.rows_used <= da.tiles.shape[0] // da.cols
    return (inf, tiles)


# def img_equivalence(img: Image) -> Equivalence:
#     return make_font_equivalence(
#         default_font_c8() if img.flags & TextFlags.FONT_8PX != 0 else default_font_c9(),
#         ice_color=img.flags & TextFlags.ICE_COLOR != 0,
#         allow_fg_remap=True,
#         skip_val=0x0000,
#         alt_val=0x0020,
#     )


def write_bintext(args: Args, input_path: str, img: Image) -> bool | None:
    if args.output_path != None:
        output_path = args.output_path
    else:
        output_path = os.path.splitext(input_path)[0] + ".bin"
        if output_path == input_path:
            logger.error(
                "Output must be specified -- default would overwrite input file"
            )
            return None
    logger.info("Writing normalized BINTEXT '%s'", output_path)

    equiv = default_equivalence()  # img_equivalence(img)
    if args.key:
        _, skip = parse_args_key(args.key)
    else:
        skip = 0x0000
    normalize = np.vectorize(lambda x, m: equiv.reps[x] if m else skip, [np.uint16])
    norm_data: np.ndarray = normalize(img.data, img.mask)
    assert (norm_data.shape[1] & 1) == 0
    assert (norm_data.shape[1] > 0) and (norm_data.shape[1] // 2) < 256
    norm_bytes = norm_data.tobytes()
    file_size = len(norm_bytes)
    with open(output_path, "wb") as f:
        f.write(norm_bytes)
        f.write(make_bintext_sauce(file_size, norm_data.shape[1], img.flags & 0xFF))


def tbm_encode_flat(img: Image) -> tuple[int, bytes]:
    return (TbmFormat.FLAT, img.data.tobytes())


def tbm_encode_rle(img: Image) -> tuple[int, bytes]:
    return (TbmFormat.RLE, encode_rle(img.data))


def tbm_encode_xbin(img: Image) -> tuple[int, bytes]:
    return (TbmFormat.XBIN, encode_xbin(img.data))


def tbm_encode_mask_flat(img: Image) -> bytes:
    encoded = np.packbits(img.mask, axis=-1).tobytes() + img.data.tobytes()
    return (TbmFormat.MASK_FLAT, encoded)


def tbm_encode_mask_key(img: Image) -> bytes:
    encoded = (
        # struct.pack("<H", img.mask_key if img.mask_key != None else 0x0000)
        struct.pack("<H", img.mask_key if img.mask_key != None else 0x0000)
        + img.data.tobytes()
    )
    return (TbmFormat.MASK_KEY, encoded)


def tbm_encode_mask_rle(img: Image) -> tuple[int, bytes]:
    return (TbmFormat.MASK_RLE, encode_rle(img.data, img.mask))


def tbm_encode_mask_xbin(img: Image) -> tuple[int, bytes]:
    return (
        TbmFormat.MASK_XBIN,
        encode_xbin(
            img.data, img.mask, default_equivalence()
        ),  # img_equivalence(img)),
    )


def tbm_encode_smallest(img: Image) -> tuple[int, bytes]:
    if img.mask.all():
        encodings = [tbm_encode_flat, tbm_encode_rle, tbm_encode_xbin]
    else:
        encodings = [
            tbm_encode_mask_flat,
            tbm_encode_mask_key,
            tbm_encode_mask_rle,
            tbm_encode_mask_xbin,
        ]
    alts = [e(img) for e in encodings]
    smallest = None
    for a in alts:
        fmt, encoded = a
        if (smallest == None) or (len(encoded) < len(smallest[1])):
            logger.info("Format %X: size=%d" % (fmt, len(encoded)))
            smallest = a
    logger.info("Choosing %X" % (smallest[0],))
    return smallest


def write_tbm(args: Args, input_path: str, img: Image) -> bool | None:
    assert img.mask.shape == img.data.shape
    if args.output_path != None:
        output_path = args.output_path
    else:
        output_path = os.path.splitext(input_path)[0] + ".tbm"
        if output_path == input_path:
            logger.error(
                "Output must be specified -- default would overwrite input file"
            )
            return None
    equiv = default_equivalence()  # img_equivalence(img)
    if args.key:
        _, skip = parse_args_key(args.key)
    else:
        skip = 0
    ones = np.ones(img.mask.shape, dtype=np.bool_)
    img.data = equiv.reps[img.data] * img.mask + (ones * skip) * (ones & ~img.mask)
    if args.output_subtype == ImageEnc.DEFAULT:
        logger.info("Encoding XBIN RLE masked TBM (default)")
        fmt, encoded = tbm_encode_mask_xbin(img)
    if args.output_subtype == ImageEnc.AUTO:
        logger.info("Autodetecting smallest encoding")
        fmt, encoded = tbm_encode_smallest(img)
    if args.output_subtype == ImageEnc.FLAT:
        logger.info("Encoding plain TBM")
        fmt, encoded = tbm_encode_flat(img)
    if args.output_subtype == ImageEnc.RLE:
        logger.info("Encoding RLE TBM")
        fmt, encoded = tbm_encode_flat(img)
    if args.output_subtype == ImageEnc.XBIN:
        logger.info("Encoding XBIN RLE TBM")
        fmt, encoded = tbm_encode_flat(img)
    elif args.output_subtype == ImageEnc.MASK_FLAT:
        logger.info("Encoding masked TBM")
        fmt, encoded = tbm_encode_mask_flat(img)
    elif args.output_subtype == ImageEnc.MASK_KEY:
        logger.info("Encoding key masked TBM")
        fmt, encoded = tbm_encode_mask_key(img)
    elif args.output_subtype == ImageEnc.MASK_RLE:
        logger.info("Encoding RLE masked TBM")
        fmt, encoded = tbm_encode_mask_rle(img)
    elif args.output_subtype == ImageEnc.MASK_XBIN:
        logger.info("Encoding XBIN RLE masked TBM")
        fmt, encoded = tbm_encode_mask_xbin(img)
    logger.info("Writing TBM '%s'", output_path)
    with open(output_path, "wb") as f:
        assert len(img.data.shape) == 2
        f.write(b"TBMa")
        f.write(
            struct.pack(
                "<LHHH",
                len(encoded) + 6,
                img.data.shape[1],
                img.data.shape[0],
                img.flags | fmt,
            )
        )
        f.write(encoded)
        return True


def main(args: Args):
    assert len(args.input_path) == 1
    input_path = args.input_path[0]
    img = read_image(args, input_path)
    if img == None:
        return 1
    if args.normalize:
        write_bintext(args, input_path, img)
    else:
        write_res = write_tbm(args, input_path, img)
        if write_res == None:
            return 1
    return 0


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
