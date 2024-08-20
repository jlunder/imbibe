#!/usr/bin/python

__appname__ = "mk_cdata"
__author__ = "Joseph Lunderville <joe.lunderville@gmail.com>"
__version__ = "0.1"


import argparse
import dataclasses
import logging
import os
import sys

logger = logging.getLogger(__appname__)


MAX_INPUT_SIZE = 1 << 20


@dataclasses.dataclass
class Args:
    verbose: bool = False
    input_path: str = None
    output_path: str = None
    header: str = (
        '#include "imbibe.h"\\n'
        + "\\n"
        + '#include "data.h"\\n'
        + "\\n"
        + "uint8_t const inline_data::data[\\S] = {\\n"
    )
    line_prefix: str = "  "
    chunk_size: int = 12
    footer: str = "};\\n"


def main(args: Args):
    input_path = args.input_path[0]
    logger.info("Opening %s", input_path)
    try:
        with open(input_path, "rb") as f:
            data = f.read(MAX_INPUT_SIZE)
    except Exception as e:
        logger.error("%s", e)
        return 2

    if args.output_path != None:
        output_path = args.output_path
    else:
        output_path = os.path.splitext(input_path)[0] + ".cpp"
        if output_path == input_path:
            logger.error(
                "Output must be specified -- default would overwrite input file"
            )
            return 2
    logger.info("Writing '%s'", output_path)
    try:
        with open(output_path, "w") as f:

            def expand(s: str) -> str:
                return (
                    s.replace("\\n", "\n")
                    .replace("\\S", str(len(data)))
                    .replace("\\\\", "\\")
                )

            f.write(expand(args.header))
            prefix = expand(args.line_prefix)
            for i in range(0, len(data) + args.chunk_size - 1, args.chunk_size):
                chunk = data[i : i + args.chunk_size]
                f.write(
                    prefix
                    + ", ".join(("0x%02X" % (c,) for c in chunk))
                    + ("," if i + len(chunk) < len(data) else "")
                    + "\n"
                )
            f.write(expand(args.footer))
    except Exception as e:
        logger.error("%s", e)
        return 2

    return 0


if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Convert a data file to a C/C++ array initializer for inline inclusion"
        )
        parser.add_argument(
            "-v", "--verbose", action="store_true", help="verbose message output"
        )
        parser.add_argument(
            "--header",
            help="emit this string first, before data lines",
        )
        parser.add_argument(
            "-p",
            "--prefix",
            dest="line_prefix",
            metavar="PREFIX",
            help="prefix each line of data with this string (e.g. for indentation)",
        )
        parser.add_argument(
            "-l",
            "--bytesperline",
            dest="chunk_size",
            metavar="BYTES",
            type=int,
            help="how many bytes to emit before starting a new line of data",
        )
        parser.add_argument(
            "--footer",
            help="emit this string after all data lines",
        )
        parser.add_argument(
            "-o",
            "--output",
            dest="output_path",
            metavar="OUTPUT",
            help="output source file path",
        )
        parser.add_argument(
            "input_path", metavar="INPUT", nargs=1, help="input data path"
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
