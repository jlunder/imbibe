from dataclasses import dataclass
from enum import Enum, IntFlag, auto
import logging
import struct

import __main__

if "__appname__" in dir(__main__):
    logging_name = __main__.__appname__ + ".sauce"
else:
    logging_name = "sauce"

logger = logging.getLogger(logging_name)


class TextFlags(IntFlag):
    ICE_COLOR = 0x01
    FONT_8PX = 0x02
    FONT_9PX = 0x04
    ASPECT_LEGACY = 0x08
    ASPECT_SQUARE = 0x10


class Format(Enum):
    UNKNOWN = auto()
    ASCII = auto()
    ANSI = auto()
    BINTEXT = auto()


@dataclass
class SauceInfo:
    format: Format = Format.UNKNOWN
    data_size: int = 0
    width: int | None = 80
    height: int | None = 25
    flags: TextFlags = 0
    title: str = ""


def parse_sauce(data: bin):
    sauce_rec_len = 128
    sauce_rec_id = b"SAUCE"

    if len(data) < sauce_rec_len:
        logger.info("File is too short (%d) to have a SAUCE record", len(data))
        return None
    sauce_rec = data[-sauce_rec_len:]
    if sauce_rec[: len(sauce_rec_id)] != sauce_rec_id:
        logger.info("File does not have SAUCE header magic")
        return None
    title: bytes
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

    title, author, group = (title.rstrip(), author.rstrip(), group.rstrip())
    logger.info(
        "id = %s; version = %s; title = %r; author = %r; group = %r; "
        + "date = %r; fsize = %d; dtype = %d; ftype = %d; tinfo1 = %d; "
        + "tinfo2 = %d; tinfo3 = %d; tinfo4 = %d; ncomments = %d; "
        + "flags = %02X; tinfos = %r",
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
            format = Format.ASCII
        elif ftype == 1:
            format = Format.ANSI
        elif ftype == 2:
            format = Format.ANSI
        else:
            format = Format.UNKNOWN
        if not fsize:
            fsize = actual_fsize
        return SauceInfo(
            format=format,
            data_size=fsize,
            width=tinfo1 or None,
            height=tinfo2 or None,
            flags=flags,
            title=title,
        )
    elif dtype == 5:
        w = ftype * 2
        if w == 0:
            logger.warning("SAUCE has no width for BINTEXT, defaulting to 80")
        if not fsize:
            # This is more of a problem for bintext so complain about it
            logger.warning(
                "SAUCE has no filesize for BINTEXT, using actual data length"
            )
            fsize = actual_fsize
        h = fsize // (w * 2)
        if fsize != w * h * 2:
            logger.warning(
                "Computed dimensions (%dx%d) do not match filesize (%d)",
                w,
                h,
                fsize,
            )

        return SauceInfo(
            format=Format.BINTEXT,
            data_size=fsize,
            width=w or None,
            height=h or None,
            flags=flags,
            title=title.decode("cp437"),
        )
    else:
        logger.warning("Unsupported datatype %d in SAUCE", dtype)
        return SauceInfo(
            format=Format.UNKNOWN,
            data_size=actual_fsize,
            width=None,
            height=None,
            flags=flags,
            title="",
        )


def make_bintext_sauce(file_size: int, width: int, flags: int) -> bytes:
    return make_sauce(file_size, 5, width // 2, flags)


def make_sauce(file_size: int, dtype: int, ftype: int, flags: int) -> bytes:
    sauce = struct.pack(
        "<5s2s35s20s20s8sLBBHHHHBB22s",
        b"SAUCE",
        b"00",
        b" " * 35,
        b" " * 20,
        b" " * 20,
        b"20240831",
        file_size,
        dtype,
        ftype,
        0,
        0,
        0,
        0,
        0,
        flags,
        b"",
    )
    assert len(sauce) == 128
    return b"0x1A" + sauce
