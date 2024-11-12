#!/usr/bin/python

__appname__ = "mk_tbm"
__author__ = "Joseph Lunderville <joe.lunderville@gmail.com>"
__version__ = "0.1"


import ast
import argparse
import dataclasses
import logging
import os
import numpy as np
import re
import struct
import sys

from sauce import *

logger = logging.getLogger(__appname__)

OUT_AUTO = "auto"
OUT_FLAT = "flat"
OUT_RLE = "rle"
OUT_MASK_FLAT = "mask_flat"
OUT_MASK_KEY = "mask_key"
OUT_MASK_RLE = "mask_rle"

IN_DETECT = "detect"
IN_ASCII = "ascii"
IN_ANSI = "ansi"
IN_BINTEXT = "bintext"
IN_TBM = "tbm"
IN_OTHER = "other"

MAX_INPUT_SIZE = 1 << 20

TBM_FORMAT_FLAT = 0x0100
TBM_FORMAT_RLE = 0x0200
TBM_FORMAT_MASK_FLAT = 0x0300
TBM_FORMAT_MASK_KEY = 0x0400
TBM_FORMAT_MASK_RLE = 0x0500


@dataclasses.dataclass
class Args:
    verbose: bool = False
    normalize: bool = False
    output_subtype: str = OUT_AUTO
    input_format: str = IN_DETECT
    input_width: int = None
    key: str = None
    opaque: bool = False
    input_path: str = None
    output_path: str = None


@dataclasses.dataclass
class ImageInfo:
    format: str = IN_OTHER
    data_size: int = None
    width: int = 80
    height: int = 25
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
        choices=["auto", "flat", "rle", "mask_flat", "mask_key", "mask_rle"],
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


def read_bintext(args: Args, input_path: str):
    logger.info("Opening %s", input_path)
    try:
        with open(input_path, "rb") as f:
            data = f.read(MAX_INPUT_SIZE)
    except Exception as e:
        logger.error("%s", e)
        return None
    sauce = parse_sauce(data)
    if not (sauce is None):
        equals_arg_re = re.compile(b"^(--?[a-zA-Z_-]+)=(.*)$")
        if (len(sauce.title) > 0) and (sauce.title[-1] == ord(b":")):
            img_args = []
            for a in sauce.title.split(b":")[1:-1]:
                m = equals_arg_re.match(a)
                if m:
                    img_args.append(m.group(1))
                    img_args.append(m.group(2))
                else:
                    img_args.append(a)
            img_args = [b.decode("ascii") for b in img_args]
        else:
            img_args = None
        if sauce.format == Format.ASCII:
            img_format = IN_ASCII
        elif sauce.format == Format.ANSI:
            img_format = IN_ANSI
        elif sauce.format == Format.BINTEXT:
            img_format = IN_BINTEXT
        else:
            img_format = IN_DETECT
        inf = ImageInfo(
            format=img_format,
            data_size=sauce.data_size,
            width=sauce.width,
            height=sauce.height,
            flags=sauce.flags,
            args=img_args,
        )
    else:
        logger.warning(
            "Input file '%s' does not have valid SAUCE, assuming BINTEXT", input_path
        )
        inf = ImageInfo(format=IN_BINTEXT)
    if inf.args != None:
        logger.info("Parsing extra image args: %r", inf.args)
        args = arg_parser.parse_args(
            inf.args + [input_path], namespace=dataclasses.replace(args)
        )
    if inf.format != IN_BINTEXT:
        logger.error("Only BINTEXT input is supported, this is %s", inf.format.upper())
        return None
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
    key = None
    mask = np.ones(img_data.shape)
    if args.key:
        key_mask, key = parse_args_key(args.key)
        logger.info("Using key value %04x, mask %04x", key, key_mask)
        if key_mask == 0:
            logger.warning(
                "The key mask is 0, which means it will match anything "
                + "and your output will be transparent"
            )
        mask = np.vectorize(lambda x: int((x & key_mask) != key))(img_data)
    return Image(flags=inf.flags, data=img_data, mask_key=key, mask=mask)


def normalizer(img: Image) -> list[list[int]]:
    blank = set([0x00, 0x20, 0xFF])
    inverse_blank = set([0xDB])
    blank_c = 0
    inverse = {0xDE: 0xDD, 0xDF: 0xDC}
    # if img.flags & FLAGS_FONT_8PX:
    #     inverse |= {0x08: 0x07, 0x0A: 0x09, 0xB2: 0xB0}

    def normalize_tm(termel: int) -> int:
        c = termel & 0xFF
        fg = (termel >> 8) & 0xF
        if img.flags & TextFlags.ICE_COLOR:
            bg = (termel >> 12) & 0xF
            blink = 0
            can_invert = True
        else:
            bg = (termel >> 12) & 0x7
            blink = (termel >> 15) & 0x1
            can_invert = (fg & 0x80) == 0
        if fg == bg:
            # character not visible over background
            c = blank_c
            fg = 0
            blink = 0
        elif c in blank:
            # foreground not used by this character
            c = blank_c
            fg = 0
        elif not blink:
            if c in inverse:
                c = inverse[c]
                fg, bg = bg, fg
            elif c in inverse_blank:
                if can_invert:
                    c = blank_c
                    fg, bg = 0, fg
                else:
                    # background entirely covered by this character
                    bg = 0
        return c | (fg << 8) | (bg << 12) | (blink << 15)

    norms = [None] * (1 << 16)
    for tm in range(len(norms)):
        norm_tm = normalize_tm(tm)
        if norm_tm == tm:
            norms[tm] = [tm]
        else:
            if norms[norm_tm] == None:
                norms[norm_tm] = []
            norms[tm] = norms[norm_tm]
            norms[tm].append(tm)
    for tm in range(len(norms)):
        assert tm in norms[tm]
        for n in norms[tm]:
            assert norms[n] is norms[tm]
    # for tm in range(len(norms)):
    #     if (len(norms[tm]) > 1) and (norms[tm][0] == tm):
    #         c = lambda x: "%02X %X %X %d" % (
    #             x & 0xFF,
    #             (x >> 8) & 0xF,
    #             (x >> 12) & 0x7,
    #             x >> 15,
    #         )
    #         print("%s: %s" % (c(tm), ", ".join(map(c, norms[tm][1:]))))
    return norms


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

    norms = normalizer(img)
    zero = norms[0x0000][0]
    normalize = np.vectorize(lambda x, m: norms[x][0] if m else zero, [np.uint16])
    norm_data: np.ndarray = normalize(img.data, img.mask)
    assert (norm_data.shape[1] & 1) == 0
    assert (norm_data.shape[1] > 0) and (norm_data.shape[1] // 2) < 256
    norm_bytes = norm_data.tobytes()
    file_size = len(norm_bytes)
    with open(output_path, "wb") as f:
        f.write(norm_bytes)
        f.write()
        f.write(make_bintext_sauce(file_size, norm_data.shape[1], img.flags & 0xFF))


def encode_flat(img: Image) -> tuple[int, bytes]:
    return (TBM_FORMAT_FLAT, img.data.tobytes())


class RleEncoder:
    runs: list[tuple]
    copy_buf: list[int]
    last: int
    rep: int

    def __init__(self):
        self.runs = []
        self.copy_buf = []
        self.last = None
        self.rep = 0

    def commit_copy(self):
        if len(self.copy_buf) > 0:
            self.runs.append((0, self.copy_buf))
            self.copy_buf = []

    def commit_fill(self):
        self.commit_copy()
        if self.rep > 0:
            self.runs.append((1, self.last, self.rep))
            self.rep = 0

    def encode(self, src):
        max_run = 127
        for cur in src:
            if cur != self.last:
                if self.rep >= 3:
                    self.commit_copy()
                    self.commit_fill()
                else:
                    while self.rep > 0:
                        self.copy_buf.append(self.last)
                        self.rep -= 1
                        if len(self.copy_buf) >= max_run:
                            self.commit_copy()
                self.last = cur
                self.rep = 1
            else:
                self.rep += 1
            if len(self.copy_buf) >= max_run:
                self.commit_copy()
            if self.rep >= max_run:
                self.commit_copy()
                self.commit_fill()
        self.commit_copy()
        self.commit_fill()

    def data(self):
        def run_bytes(run):
            if run[0] == 0:
                return (
                    struct.pack("<B", (len(run[1]) & 0x7F) | 0x00)
                    + np.array(run[1], dtype=np.uint16).tobytes()
                )
            elif run[0] == 1:
                return struct.pack("<BH", (run[2] & 0x7F) | 0x80, run[1])

        return b"".join(map(run_bytes, self.runs))


def encode_rle(img: Image) -> tuple[int, bytes]:
    lines = []
    pos = 2 * img.data.shape[0]
    rle_bytes = bytes()
    for i in range(img.data.shape[0]):
        lines.append(pos)
        encoder = RleEncoder()
        encoder.encode(img.data[i])
        data = encoder.data()
        rle_bytes += data
        pos += len(data)

        # decoded = []
        # p = 0
        # while p < len(data):
        #     l = data[p]
        #     p += 1
        #     if l & 0x80:
        #         decoded += data[p : p + 2] * (l & 0x7F)
        #         p += 2
        #     else:
        #         decoded += data[p : p + l * 2]
        #         p += l * 2
        # for r in encoder.runs:
        #     if r[0] == 0:
        #         print("data: " + (", ".join(["%04X" % (t,) for t in r[1]])))
        #     elif r[0] == 1:
        #         print("run: %04X x %d" % (r[1], r[2]))
        # print("original: " + " ".join(["%02X" % (b,) for b in img.data[i].tobytes()]))
        # print("decoded:  " + " ".join(["%02X" % (b,) for b in bytes(decoded)]))
        # assert bytes(decoded) == img.data[i].tobytes()
    encoded = np.array(lines, np.uint16).tobytes() + rle_bytes
    assert len(lines) == img.data.shape[0]
    assert pos < 30000
    return (TBM_FORMAT_RLE, encoded)


def encode_mask_flat(img: Image) -> tuple[int, bytes]:
    encoded = np.packbits(img.mask, axis=-1).tobytes() + img.data.tobytes()
    return (TBM_FORMAT_MASK_FLAT, encoded)


def encode_mask_key(img: Image) -> tuple[int, bytes]:
    encoded = (
        struct.pack("<H", img.mask_key if img.mask_key != None else 0x0000)
        + img.data.tobytes()
    )
    return (TBM_FORMAT_MASK_KEY, encoded)


def encode_mask_rle(img: Image) -> tuple[int, bytes]:
    lines = []
    pos = 2 * img.data.shape[0]
    max_skip = 255
    rle_bytes = bytes()
    for i in range(img.data.shape[0]):
        data = []
        maybe_data = []
        j = 0
        # logger.info(
        #     "RLE line %d: input mask %r",
        #     i,
        #     list(np.asarray(img.mask[i], np.uint8)),
        # )
        while j < img.data.shape[1]:
            skip = 0
            span = []
            while (j < img.data.shape[1]) and (skip < max_skip) and not img.mask[i][j]:
                skip += 1
                j += 1
            while (j < img.data.shape[1]) and (len(span) < max_skip) and img.mask[i][j]:
                span.append(img.data[i][j])
                j += 1
            if len(span) > 0:
                data += maybe_data
                maybe_data = []
                data.append(skip)
                data.append(len(span))
                data += list(np.array(span, np.uint16).tobytes())
                # logger.info(
                #     "RLE line %d: j=%d; skip %d, span %d", i, j, skip, len(span)
                # )
            else:
                maybe_data.append(skip)
                maybe_data.append(0)
                # logger.info("RLE line %d: j=%d; MAYBE skip %d", i, j, skip)
        # logger.info("RLE line %d: adding %r", i, data)
        # logger.info("RLE line %d: discarding %r", i, maybe_data)
        if len(data) == 0:
            lines.append(0)
        else:
            lines.append(pos)
            data += [0, 0]
        rle_bytes += bytes(data)
        pos += len(data)
    encoded = np.array(lines, np.uint16).tobytes() + rle_bytes
    assert len(lines) == img.data.shape[0]
    assert pos < 16000
    return (TBM_FORMAT_MASK_RLE, encoded)


def encode_smallest(img: Image) -> tuple[int, bytes]:
    if img.mask.all():
        encodings = [encode_flat, encode_rle]
    else:
        encodings = [encode_mask_flat, encode_mask_key, encode_mask_rle]
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
    norms = normalizer(img)
    if args.key:
        _, zero = parse_args_key(args.key)
    else:
        zero = 0
    zero = norms[zero][0]
    normalize = np.vectorize(lambda x, m: norms[x][0] if m else zero, [np.uint16])
    img.data = normalize(img.data, img.mask)
    if args.output_subtype == OUT_AUTO:
        logger.info("Autodetecting smallest encoding")
        fmt, encoded = encode_smallest(img)
    if args.output_subtype == OUT_FLAT:
        logger.info("Encoding plain TBM")
        fmt, encoded = encode_flat(img)
    elif args.output_subtype == OUT_MASK_FLAT:
        logger.info("Encoding masked TBM")
        fmt, encoded = encode_mask_flat(img)
    elif args.output_subtype == OUT_MASK_KEY:
        logger.info("Encoding key masked TBM")
        fmt, encoded = encode_mask_key(img)
    elif args.output_subtype == OUT_MASK_RLE:
        logger.info("Encoding RLE masked TBM")
        fmt, encoded = encode_mask_key(img)
    logger.info("Writing TBM '%s'", output_path)
    with open(output_path, "wb") as f:
        assert len(img.data.shape) == 2
        f.write(b"TBMa")
        f.write(
            struct.pack(
                "<LBBH",
                len(encoded) + 4,
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
    img = read_bintext(args, input_path)
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
