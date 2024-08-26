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
import struct
import sys

logger = logging.getLogger(__appname__)

OUT_PLAIN = "plain"
OUT_MASK = "mask"
OUT_MASK_KEY = "key"
OUT_MASK_RLE = "rle"

IN_DETECT = "detect"
IN_ASCII = "ascii"
IN_ANSI = "ansi"
IN_BINTEXT = "bintext"
IN_TBM = "tbm"
IN_OTHER = "other"

MAX_INPUT_SIZE = 1 << 20


@dataclasses.dataclass
class Args:
    verbose: bool = False
    output_subtype: str = OUT_PLAIN
    input_format: str = IN_DETECT
    input_width: int = None
    key: str = None
    input_path: str = None
    output_path: str = None


@dataclasses.dataclass
class ImageInfo:
    format: str = IN_OTHER
    data_size: int = None
    width: int = 80
    height: int = 25
    flags: int = 0


@dataclasses.dataclass
class Image:
    flags: int = 0
    data: np.ndarray = None
    mask: np.ndarray = None
    mask_key: int = None


def parse_sauce(data: bin):
    sauce_rec_len = 128
    sauce_rec_id = b"SAUCE"
    if len(data) < sauce_rec_len:
        logger.info("File is too short (%d) to have a SAUCE record", len(data))
        return None
    sauce_rec = data[-sauce_rec_len:]
    if sauce_rec[: len(sauce_rec_id)] != sauce_rec_id:
        logger.info("File does not have sauce header magic")
        return None
    (
        id,
        version,
        title,
        author,
        group,
        date,
        fsize,
        dtype,
        ftype,
        tinfo1,
        tinfo2,
        tinfo3,
        tinfo4,
        ncomments,
        flags,
        tinfos,
    ) = struct.unpack("<5s2s35s20s20s8sLBBHHHHBB22s", sauce_rec)

    if version != b"00":
        logger.warning(
            "SAUCE header has unexpected version (%02X %02X, expected %02X %02X)",
            version[0],
            version[1],
            ord("0"),
            ord("0"),
        )

    logger.info(
        "id = %s; version = %s; title = %r; author = %r; group = %r; "
        + "date = %r; fsize = %d; dtype = %d; ftype = %d; tinfo1 = %d; "
        + "tinfo2 = %d; tinfo3 = %d; tinfo4 = %d; ncomments = %d; "
        + "flags = %02X; tinfos = %r",
        id,
        version,
        title.rstrip(),
        author.rstrip(),
        group.rstrip(),
        date,
        fsize,
        dtype,
        ftype,
        tinfo1,
        tinfo2,
        tinfo3,
        tinfo4,
        ncomments,
        flags,
        tinfos.rstrip(b"\0"),
    )

    sauce_comment_id = b"COMNT"
    sauce_comment_len = 64
    sauce_full_len = sauce_rec_len + (
        len(sauce_comment_id) + sauce_comment_len * ncomments if ncomments > 0 else 0
    )

    if sauce_full_len >= len(data):
        logger.warning(
            "File is too short (%d) to fit full SAUCE with %d comments",
            len(data),
            sauce_full_len,
            ncomments,
        )
        ncomments = 0
        sauce_full_len = sauce_rec_len
    if ncomments and (
        data[-sauce_full_len:][: len(sauce_comment_id)] != sauce_comment_id
    ):
        logger.warning(
            "File does not have a valid SAUCE comment header (expected at %d)",
            len(data) - sauce_full_len,
        )
        ncomments = 0
        sauce_full_len = sauce_rec_len

    if fsize > 0 and len(data) - sauce_full_len <= fsize:
        logger.warning(
            "SAUCE file size (%d) is inconsistent with the actual file "
            + "(%d - %d SAUCE = %d)",
            fsize,
            len(data),
            sauce_full_len,
            len(data) - sauce_full_len,
        )
        fsize = 0

    computed_eof_pos = len(data) - sauce_full_len - 1
    if data[computed_eof_pos] == 0x1A:
        actual_fsize = computed_eof_pos
    else:
        actual_fsize = len(data) - sauce_full_len
        logger.warning("Did not find expected EOF at %d", computed_eof_pos)

    if fsize and (actual_fsize != fsize):
        logger.info(
            "SAUCE filesize %d does not match actual data length %d",
            fsize,
            actual_fsize,
        )

    if dtype == 1:
        if ftype == 0:
            return ImageInfo(
                format=IN_ASCII,
                data_size=used_fsize,
                width=tinfo1 or None,
                height=tinfo2 or None,
                flags=flags,
            )
        elif ftype == 1:
            return ImageInfo(
                format=IN_ASCII,
                data_size=used_fsize,
                width=tinfo1 or None,
                height=tinfo2 or None,
                flags=flags,
            )
        elif ftype == 2:
            return ImageInfo(
                format=IN_ASCII,
                data_size=used_fsize,
                width=tinfo1 or None,
                height=tinfo2 or None,
                flags=flags,
            )
    elif dtype == 5:
        w = ftype * 2
        if w == 0:
            logger.warning("SAUCE has no width for BINTEXT, defaulting to 80")
        if fsize:
            used_fsize = fsize
        else:
            logger.warning(
                "SAUCE has no filesize for BINTEXT, using actual data length"
            )
            used_fsize = actual_fsize
        h = used_fsize // (w * 2)
        if used_fsize != w * h * 2:
            logger.warning(
                "Computed dimensions (%dx%d) do not match filesize (%d)",
                w,
                h,
                used_fsize,
            )

        return ImageInfo(
            format=IN_BINTEXT,
            data_size=used_fsize,
            width=w or None,
            height=h or None,
            flags=flags,
        )
    else:
        logger.warning("Unsupported datatype %d in SAUCE", dtype)
        return ImageInfo(format=IN_OTHER, data_size=used_fsize)


def parse_args_key(args_key: str) -> tuple[int, int]:
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


def read_input_image(args: Args, input_path: str):
    logger.info("Opening %s", input_path)
    try:
        with open(input_path, "rb") as f:
            data = f.read(MAX_INPUT_SIZE)
    except Exception as e:
        logger.error("%s", e)
        return None
    inf = parse_sauce(data)
    if inf == None:
        logger.warning(
            "Input file '%s' does not have valid SAUCE, assuming BINTEXT", input_path
        )
        inf = ImageInfo(format=IN_BINTEXT)
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


def write_output_image(args: Args, input_path: str, img: Image) -> bool | None:
    if args.output_path != None:
        output_path = args.output_path
    else:
        output_path = os.path.splitext(input_path)[0] + ".tbm"
        if output_path == input_path:
            logger.error(
                "Output must be specified -- default would overwrite input file"
            )
            return None
    logger.info("Writing '%s'", output_path)
    with open(output_path, "wb") as f:
        assert len(img.data.shape) == 2
        if args.output_subtype == OUT_PLAIN:
            format_flags = 0x0000
        elif args.output_subtype == OUT_MASK:
            format_flags = 0x0100
        elif args.output_subtype == OUT_MASK_KEY:
            format_flags = 0x0400
        elif args.output_subtype == OUT_MASK_RLE:
            format_flags = 0x0500
        f.write(b"TBMa")
        if args.output_subtype == OUT_PLAIN:
            logger.info("Encoding plain TBM")
            data_bytes = img.data.tobytes()
        elif args.output_subtype == OUT_MASK:
            logger.info("Encoding masked TBM")
            assert img.mask.shape == img.data.shape
            data_bytes = np.packbits(img.mask, axis=-1).tobytes() + img.data.tobytes()
        elif args.output_subtype == OUT_MASK_KEY:
            logger.info("Encoding key masked TBM")
            data_bytes = (
                struct.pack("<H", img.mask_key if img.mask_key != None else 0x0000)
                + img.data.tobytes()
            )
        elif args.output_subtype == OUT_MASK_RLE:
            logger.info(
                "Encoding RLE masked TBM, mask %r, image %r",
                img.mask.shape,
                img.data.shape,
            )
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
                    while (
                        (j < img.data.shape[1])
                        and (skip < max_skip)
                        and not img.mask[i][j]
                    ):
                        skip += 1
                        j += 1
                    while (
                        (j < img.data.shape[1])
                        and (len(span) < max_skip)
                        and img.mask[i][j]
                    ):
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
            data_bytes = np.array(lines, np.uint16).tobytes() + rle_bytes
            assert len(lines) == img.data.shape[0]
            assert pos < 16000
        f.write(
            struct.pack(
                "<LBBH",
                len(data_bytes) + 4,
                img.data.shape[1],
                img.data.shape[0],
                img.flags | format_flags,
            )
        )
        f.write(data_bytes)
        return True


def main(args: Args):
    assert len(args.input_path) == 1
    input_path = args.input_path[0]
    img = read_input_image(args, input_path)
    if img == None:
        return 1
    write_res = write_output_image(args, input_path, img)
    if write_res == None:
        return 1
    return 0


if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Convert a BinText (ANSI .BIN file) to a .TBM"
        )
        parser.add_argument(
            "-v", "--verbose", action="store_true", help="verbose message output"
        )
        parser.add_argument(
            "-t",
            "--type",
            dest="output_subtype",
            metavar="SUBTYPE",
            choices=["plain", "mask", "key", "rle"],
            help="output specific TBM subtype",
        )
        parser.add_argument(
            "-k", "--key", type=str, help="attribute key for masked BINs"
        )
        parser.add_argument(
            "-o",
            "--output",
            dest="output_path",
            metavar="OUTPUT",
            help="output TBM path",
        )
        parser.add_argument(
            "input_path", metavar="INPUT", nargs=1, help="input BIN path"
        )

        args: Args = parser.parse_args(namespace=Args())
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
