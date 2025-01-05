#!/usr/bin/python3 -O

__appname__ = "mk_tbm"
__author__ = "Joseph Lunderville <joe.lunderville@gmail.com>"
__version__ = "0.1"


import argparse
import dataclasses
import logging
import os
from random import Random
import struct
import sys
from typing import BinaryIO, Callable, Iterator

from ansi import *
import crc
from equivs import *
from rle import *
from sauce import *

logger = logging.getLogger(__appname__)


@dataclasses.dataclass
class Args:
    verbose: bool = False
    input_path: str = None
    output_path: str = None


def make_arg_parser():
    parser = argparse.ArgumentParser(
        description="Convert a BinText (ANSI .BIN file) to a .TBM"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="verbose message output"
    )
    parser.add_argument(
        "-o",
        "--output",
        default="data.tya",
        dest="output_path",
        metavar="OUTPUT",
        help="output TBM path",
    )
    parser.add_argument("input_path", metavar="INPUT", nargs=1, help="input BIN path")

    return parser


arg_parser: argparse.ArgumentParser = make_arg_parser()


def fletcher16(data: bytes, seed: int = 0) -> int:
    s0 = seed
    s1 = seed
    for b in data:
        s0 += b
        s1 = (s1 + s0) % 255
    s0 %= 255
    return (s1 << 8) + s0


config_c9d204f5 = crc.Configuration(
    width=32,
    polynomial=0xC9D204F5,
    init_value=0xFFFFFFFF,
    final_xor_value=0xFFFFFFFF,
    reverse_input=True,
    reverse_output=True,
)

calc_c9d204f5 = crc.Calculator(config_c9d204f5)
reg_c9d204f5 = crc.TableBasedRegister(config_c9d204f5)


def crc_c9d204f5(data: bytes) -> int:
    return calc_c9d204f5.checksum(data)


def crc_blocks_c9d204f5(data: Iterator[bytes]) -> int:
    reg_c9d204f5.init()
    for b in data:
        reg_c9d204f5.update(b)
    return reg_c9d204f5.digest()


def read_chunks(r: BinaryIO) -> Iterator[bytes]:
    while True:
        chunk = r.read(1 << 20)  # 1 MB
        if chunk is None:
            continue
        if len(chunk) == 0:
            return
        yield chunk


# 1 megabyte is *quite enough* for the old computers we're targeting
MAX_ARCHIVE_SIZE = 1 << 20

# This is just a constant string, but it's somewhat carefully selected.
# The magic value can't realistically be protected by the header check value
# because we don't know how long the header is until we've checked the magic
# to figure out the file format -- so we have a chicken-and-egg problem. To
# illustrate the magnitude of the problem, the difference between "TYAR0"
# and "TYAR1" is a single bit flip, not so unlikely at all if we presuppose
# some data corruption! We could get around this by CRC'ing the whole file
# with a check value tacked on at the end, but of course then you have to
# read the entire file before you can process ANY of it, which is s-l-o-w on
# old min spec computers. We would like to be able to check stuff just-in-
# time right before we process it.
#
# One way to get around this is to ensure that any valid magic value will be
# at least some Hamming distance from the ones we're looking for, and that's
# the plan here.
#
# So, although to the module user these are just constants, if any future
# magic value is defined, please ensure it's at least HD 20 from any
# previously defined constants.
tyar_header_magic_v0: bytes = b"TYAR0\x1A\x00\x00\xD2\x6E\xD5\xCD"


@dataclasses.dataclass(frozen=True)
class FileData:
    name: bytes
    size: int
    read_chunks: Callable[[], Iterator[bytes]]
    data_offset: int | None = None
    crc: int | None = None
    name_offset: int | None = None
    toc_index: int | None = None

    # toc_entry           : 16 bytes
    #   name_offset       :  2 -- From TOC begin
    #   name_size         :  2 -- Includes null terminator, which must be present and 0
    #   file_offset       :  4 -- From data area
    #   file_size         :  4 -- Only the file, there may be padding between files
    #   file_check_value  :  4 -- CRC of file data without padding
    def toc_entry(self) -> bytes:
        return struct.pack(
            "<HHLLL",
            self.name_offset,
            len(self.name),
            self.data_offset,
            self.size,
            self.crc,
        )

    def append_file_data(self, w: BinaryIO) -> "FileData":
        read_size = 0
        reg_c9d204f5.init()
        for b in self.read_chunks():
            read_size += len(b)
            reg_c9d204f5.update(b)
            w.write(b)
        assert read_size == self.size
        if self.crc is not None:
            assert reg_c9d204f5.digest() == crc
            return self
        else:
            return dataclasses.replace(self, crc=reg_c9d204f5.digest())


@dataclasses.dataclass(frozen=True)
class HashEntry:
    hash: int
    toc_index: int
    neighborhood: int
    empty: bool = False

    # hash_entry_fletcher16 : 4 bytes
    #   name_hash           : 2 -- Fletcher-16, 0 seed
    #   toc_index           : 2 -- 0xFFFF for an unoccupied entry
    def hash_entry(self) -> bytes:
        return struct.pack("<HH", self.hash, self.toc_index)


empty_hash_entry = HashEntry(0, 0xFFFF, 0, empty=True)


def make_read_f(path: str) -> Callable[[], Iterator[bytes]]:
    return lambda: read_chunks(open(path, "rb"))


def read_files(args: Args, input_path: str) -> list[FileData] | None:
    files = []
    for base_path in input_path:
        for dirpath, _, filenames in os.walk(base_path):
            for fn in filenames:
                path = os.path.join(dirpath, fn)
                stat_res = os.stat(path)
                if not stat_res:
                    logger.warning(f"Could not stat '{fn}', skipping")
                    continue
                name = os.path.relpath(path, base_path).encode()
                logger.info(f"Adding file {repr(name)} ({stat_res.st_size} bytes)")

                files.append(FileData(name, stat_res.st_size, make_read_f(path)))
    return files


def page_pad_size(size: int) -> int:
    return (16 - (size & 15)) & 15


def page_pad(offset: int) -> bytes:
    return b"6" * page_pad_size(offset)


def plan_toc(args: Args, files: list[FileData]) -> tuple[list[FileData], int, bytes]:
    toc_size = len(files) * 16
    by_name = sorted(files, key=lambda f: f.name)
    del files
    toc_names_bytes = bytes()
    toc_files: list[FileData] = []
    for i, f in enumerate(by_name):
        name_ofs = toc_size + len(toc_names_bytes)
        toc_names_bytes += f.name + b"\0"
        toc_files.append(dataclasses.replace(f, name_offset=name_ofs, toc_index=i))
    return (toc_files, toc_size + len(toc_names_bytes), toc_names_bytes)


def write_toc(args: Args, toc: list[FileData], toc_names_bytes: bytes, w: BinaryIO):
    for i, f in enumerate(sorted(toc, key=lambda f: f.toc_index)):
        assert i == f.toc_index
        w.write(f.toc_entry())
    w.write(toc_names_bytes)


def write_file_data(args: Args, files: list[FileData], w: BinaryIO):
    data_start = w.tell()
    cur_data_end = 0
    with_data_offset: list[FileData] = []
    for f in files:
        pad = page_pad(cur_data_end)
        w.write(pad)
        cur_data_end += len(pad)
        data_ofs = cur_data_end

        reg_c9d204f5.init()
        for b in f.read_chunks():
            reg_c9d204f5.update(b)
            cur_data_end += len(b)
            w.write(b)

        assert cur_data_end - data_ofs == f.size
        with_data_offset.append(
            dataclasses.replace(f, data_offset=data_ofs, crc=reg_c9d204f5.digest())
        )
    assert w.tell() == data_start + cur_data_end
    return with_data_offset


def make_hopscotch_hash(
    args: Args, toc_order: list[HashEntry]
) -> tuple[list[HashEntry], int]:
    # 852 rounds up to 1024, 853 rounds up to 2048 -- a little wasteful
    bin_count = 1 << round(len(toc_order) * 1.2).bit_length()
    assert len(toc_order) < 0xFFFF
    bins: list[HashEntry]
    neighborhood_size: int

    def index(pos: int, n: int) -> int:
        return (pos + n) % bin_count

    def lookup(pos: int, n: int) -> HashEntry:
        return bins[index(pos, n)]

    all_hashed = False
    neighborhood_size: int

    # start with an optimistic neighborhood size -- this will (likely) expand
    for neighborhood_size in range(2, bin_count.bit_length()):
        bins = [empty_hash_entry] * bin_count
        for entry in toc_order:
            h = entry.neighborhood
            # neighborhood full, try to bring an empty bin back into
            # the neighborhood! first, find such a bin..
            for e in range(bin_count):
                if lookup(h, e).empty:
                    break
            else:
                assert not "all bins full?"
            # bring the empty bin back towards our neighborhood
            while e >= neighborhood_size:
                # find an item that can be moved into the empty bin
                for j in range(1, neighborhood_size):
                    # start with the nearest item to our neighborhood
                    k = e - neighborhood_size + j
                    k_hood = index(lookup(h, k).neighborhood, bin_count - h)
                    if k_hood + neighborhood_size > e:
                        # bingo! swap k into e
                        bins[index(h, k)], bins[index(h, e)] = (
                            lookup(h, e),
                            lookup(h, k),
                        )
                        e = k
                else:
                    # an impassable barrier! we failed
                    # this should get us out of the enclosing loop, Java's
                    # labeled break would be real nice here
                    break
            else:
                assert e < neighborhood_size
                bins[index(h, e)] = entry
                continue
            # we failed to hash this item, break out of the for... loop
            break
        else:
            # if we get here, everything was hashed properly
            all_hashed = True
            break

    assert all_hashed

    # assert all(
    #     (
    #         (hash, index) in [bins[lookup(hood, n)] for n in range(neighborhood_size)]
    #         for hood, hash, index in zip(neighborhoods, hashes, range(len(hashes)))
    #     )
    # )

    return (bins, neighborhood_size)


def write_tyar(args: Args, output_path: str, files: list[FileData]) -> bool | None:
    if len(files) >= 0x8000:
        logger.error(f"Too many files ({len(files)}) to hash")
        return None

    toc_entries, toc_size, toc_names_bytes = plan_toc(args, files)

    hash_entries: list[HashEntry] = []
    for f in toc_entries:
        hash = fletcher16(f.name)
        # note we swap low/high because fletcher16 puts stuff in the top byte
        # that we'd rather avoid truncating
        neighborhood = (hash >> 8) | ((hash & 0xFF) << 8)
        hash_entries.append(HashEntry(hash, f.toc_index, neighborhood))

    (hash_bins, hash_neighbor_count) = make_hopscotch_hash(args, hash_entries)
    hash_bins_size = len(hash_bins) * 4

    def page_pad_total(size: int) -> int:
        return page_pad_size(size) + size

    toc_entries_offset = 64
    hash_bins_offset = toc_entries_offset + page_pad_total(toc_size)
    data_offset = hash_bins_offset + page_pad_total(hash_bins_size)

    archive_size_estimate = data_offset + sum((page_pad_total(f.size) for f in files))
    if archive_size_estimate >= MAX_ARCHIVE_SIZE:
        logger.error(
            f"Archive will be too big (estimate {archive_size_estimate} bytes)"
        )
        return None

    with open(output_path, "w+b") as w:
        w.seek(data_offset)
        toc_entries = write_file_data(args, toc_entries, w)
        assert archive_size_estimate == page_pad_total(w.tell())

        if w.tell() >= MAX_ARCHIVE_SIZE:
            logger.error(
                f"Archive is too big ({w.tell()}) -- also shouldn't this have been detected earlier?"
            )
            os.remove(output_path)
            return None

        w.seek(toc_entries_offset)
        toc_data = b"".join(f.toc_entry() for f in toc_entries) + toc_names_bytes
        assert len(toc_data) == toc_size
        w.write(toc_data)
        toc_crc = crc_c9d204f5(toc_data)
        w.write(page_pad(len(toc_data)))
        assert w.tell() == hash_bins_offset
        
        hash_bins_data = b"".join((b.hash_entry() for b in hash_bins))
        assert len(hash_bins_data) == hash_bins_size
        w.write(hash_bins_data)
        hash_bins_crc = crc_c9d204f5(hash_bins_data)
        w.write(page_pad(len(hash_bins_data)))
        assert w.tell() == data_offset

        # archive_header                  : 64 bytes total
        #   header_magic                  : 12 = 'TYAR0\x1A\x00\x00\xD2\x6E\xD5\xCD' -- 0xB49C1C96
        #   archive_check_value           :  4 -- CRC of the whole shebang from +16 to end
        #
        #   header_check_value            :  4 -- CRC of this header from +20 to +60
        #
        #                                    TOC header, 16 bytes
        #   toc_entry_count               :  2 -- toc_entry_count * 32 == toc_entries_size
        #   pad                           :  2 = 0x36 '6'
        #   toc_entries_offset            :  4 -- Relative to this header
        #   toc_entries_size              :  4 -- Size in bytes of the TOC entries
        #   toc_entries_check_value       :  4 -- CRC of full TOC
        #
        #                                    hash header, 16 bytes
        #   hash_bin_count                :  2 -- Should always be a power of 2
        #   hash_neighbor_count           :  2 -- How far we might have to search for any given hash
        #   hash_bins_offset              :  4 -- Relative to this header
        #   hash_bins_size                :  4 -- Size in bytes of the header and entries
        #   hash_bins_check_value         :  4 -- CRC of hash table including header and entries
        #
        #   pad                           : 12 = 0x36 '6'

        # Generate the archive signature and headers, with placeholder CRCs
        archive_header_sig = tyar_header_magic_v0 + b"\0" * 4
        archive_header_main = struct.pack(
            "<LHHLLLHHLLL",
            0,
            len(toc_entries),
            0x3636,
            toc_entries_offset,
            toc_size,
            toc_crc,
            len(hash_bins),
            hash_neighbor_count,
            hash_bins_offset,
            hash_bins_size,
            hash_bins_crc,
        )
        # Compute the CRC for the main part of the header and write it
        header_crc = crc_c9d204f5(archive_header_main[4:])
        archive_header_main = struct.pack("<L", header_crc) + archive_header_main[4:]
        w.seek(len(archive_header_sig))
        w.write(archive_header_main)
        w.write(b"6" * (64 - w.tell()))
        assert w.tell() == toc_entries_offset

        # Now, compute the full archive CRC including the header
        w.seek(len(archive_header_sig))
        archive_crc = crc_blocks_c9d204f5(read_chunks(w))

        # Write the full CRC into the signature right at the beginning
        w.seek(0)
        w.write(archive_header_sig[:-4] + struct.pack("<L", archive_crc))

        # Everything is buttoned up!

        return True


def main(args: Args):
    assert len(args.input_path) == 1
    files = read_files(args, args.input_path)
    if files == None:
        return 1
    write_res = write_tyar(args, args.output_path, files)
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
