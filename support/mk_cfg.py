#!/usr/bin/python

__appname__ = "mk_menu_cfg"
__author__ = "Joseph Lunderville <joe.lunderville@gmail.com>"
__version__ = "0.1"


import argparse
import dataclasses
import logging
import os
import json
import struct
import sys

logger = logging.getLogger(__appname__)


@dataclasses.dataclass
class Args:
    verbose: bool = False
    root_path: str = None
    input_path: str = None
    output_path: str = None


def normalize_path(path: str, args: Args, input_path: str) -> str:
    base, ext = os.path.splitext(path)
    if ext == ".bin":
        cvt = base + ".tbm"
    elif ext == ".json":
        cvt = base + ".cfg"
    else:
        cvt = path
    if cvt.startswith("."):
        root = os.path.split(input_path)[0]
    else:
        root = args.root_path
    norm = os.path.normpath(os.path.join(root, "./" + cvt))
    rel = os.path.relpath(norm, args.root_path)
    if os.path.isabs(rel):
        logger.warning(
            "Couldn't normalize path '%s' -- tried '%s' -> '%s', root '%s'",
            path,
            norm,
            rel,
            root,
        )
        return cvt
    return rel


@dataclasses.dataclass
class IntroConfig:
    logos: list[str]

    def normalize_paths(self, args: Args, input_path: str):
        self.logos = [normalize_path(l, args, input_path) for l in self.logos]
        return self


@dataclasses.dataclass
class MenuConfigOption:
    hot: tuple[int, int, int, int]
    selected_pos: tuple[int, int]
    selected_overlay: str
    submenu: str

    def normalize_paths(self, args: Args, input_path: str):
        self.selected_overlay = normalize_path(self.selected_overlay, args, input_path)
        self.submenu = normalize_path(self.submenu, args, input_path)
        return self


@dataclasses.dataclass
class MenuConfig:
    background: str
    options: list[MenuConfigOption]

    def normalize_paths(self, args: Args, input_path: str):
        self.background = normalize_path(self.background, args, input_path)
        self.options = [o.normalize_paths(args, input_path) for o in self.options]
        return self


@dataclasses.dataclass
class SubmenuConfigOption:
    label: str
    viewer_file: str

    def normalize_paths(self, args: Args, input_path: str):
        self.viewer_file = normalize_path(self.viewer_file, args, input_path)
        return self


@dataclasses.dataclass
class SubmenuConfig:
    menu_header: str
    menu_footer: str
    option_unselected_background: str
    option_selected_background: str
    option_selected_offset: tuple[int, int]
    option_label_offset: tuple[int, int]
    option_unselected_label_attribute: int
    option_selected_label_attribute: int
    options: list[SubmenuConfigOption]

    def normalize_paths(self, args: Args, input_path: str):
        self.menu_header = normalize_path(self.menu_header, args, input_path)
        self.menu_footer = normalize_path(self.menu_footer, args, input_path)
        self.option_unselected_background = normalize_path(
            self.option_unselected_background, args, input_path
        )
        self.option_selected_background = normalize_path(
            self.option_selected_background, args, input_path
        )
        self.options = [o.normalize_paths(args, input_path) for o in self.options]
        return self


@dataclasses.dataclass
class OutroConfig:

    def normalize_paths(self, args: Args, input_path: str):
        # self.logos = [normalize_path(l, args, input_path) for l in self.logos]
        return self


def make_arg_parser():
    parser = argparse.ArgumentParser(
        description="Convert a JSON config to a binary .cfg"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="verbose message output"
    )
    parser.add_argument(
        "-o",
        "--output",
        dest="output_path",
        metavar="OUTPUT",
        help="output CFG path",
    )
    parser.add_argument(
        "-r",
        "--root",
        dest="root_path",
        metavar="ROOT",
        help="root data folder path (used to adjust path references)",
    )
    parser.add_argument("input_path", metavar="INPUT", nargs=1, help="input JSON path")

    return parser


arg_parser: argparse.ArgumentParser = make_arg_parser()


def parse_color(s: str) -> int:
    colors = {
        "black": 0,
        "blue": 1,
        "green": 2,
        "cyan": 3,
        "red": 4,
        "magenta": 5,
        "brown": 6,
        "yellow": 6,
        "white": 7,
        "light-gray": 7,
        "hi-black": 8,
        "dark-gray": 8,
        "hi-blue": 9,
        "hi-green": 10,
        "hi-cyan": 11,
        "hi-red": 12,
        "hi-magenta": 13,
        "hi-yellow": 14,
        "hi-white": 15,
    }
    return colors.get(s.strip().lower(), None)


def parse_attribute(s: str) -> int | None:
    fg_s, bg_s = s.split(":")
    fg = parse_color(fg_s)
    bg = parse_color(bg_s)
    if fg == None:
        logger.error("Couldn't parse foreground color '%s'", fg_s)
        return None
    if bg == None:
        logger.error("Couldn't parse background color '%s'", bg_s)
        return None
    if bg >= 8:
        logger.error(
            "Background color '%s' uses intensity bit (which maps to blink)", bg_s
        )
        return None
    return fg | (bg << 4)


def parse_menu_config_options(opts_data: list[dict]) -> list[MenuConfigOption]:
    return [
        MenuConfigOption(
            hot=tuple(d["hot"]),
            selected_pos=tuple(d["selected-pos"]),
            selected_overlay=str(d["selected-overlay"]),
            submenu=str(d["submenu"]),
        )
        for d in opts_data
    ]


def parse_submenu_config_options(opts_data: list[dict]) -> list[SubmenuConfigOption]:
    return [
        SubmenuConfigOption(label=d["label"], viewer_file=d["viewer-file"])
        for d in opts_data
    ]


def read_json_config(args: Args, input_path: str):
    logger.info("Opening %s", input_path)
    try:
        with open(input_path, "rb") as f:
            data = json.load(f)
        if "__type" in data:
            t = data["__type"]
            logger.info("Has explicit type '%s'", t)
        else:
            t, _ = os.path.splitext(os.path.split(input_path)[-1])
            logger.info("Inferring type '%s' from filename", t)
        if t == "intro":
            return IntroConfig(logos=data["logos"]).normalize_paths(args, input_path)
        elif t == "menu":
            return MenuConfig(
                background=data["background"],
                options=parse_menu_config_options(data["options"]),
            ).normalize_paths(args, input_path)
        elif t == "submenu":
            return SubmenuConfig(
                menu_header=data["menu-header"],
                menu_footer=data["menu-footer"],
                option_unselected_background=data["option-unselected-background"],
                option_selected_background=data["option-selected-background"],
                option_selected_offset=tuple(data["option-selected-offset"]),
                option_label_offset=tuple(data["option-label-offset"]),
                option_unselected_label_attribute=parse_attribute(
                    data["option-unselected-label-attribute"]
                ),
                option_selected_label_attribute=parse_attribute(
                    data["option-selected-label-attribute"]
                ),
                options=parse_submenu_config_options(data["options"]),
            ).normalize_paths(args, input_path)
        elif t == "outro":
            return OutroConfig()
        else:
            logger.error("Invalid type: %s", t)
    except Exception as e:
        logger.exception("Error while reading config:")
        return None


def write_intro_cfg(args: Args, output_path: str, cfg: IntroConfig) -> bool | None:
    logger.info("Writing intro config '%s'", output_path)
    with open(output_path, "wb") as f:
        for l in cfg.logos:
            f.write(l.encode() + b"\000")
    return 0


def write_menu_cfg(args: Args, output_path: str, cfg: MenuConfig) -> bool | None:
    logger.info("Writing menu config '%s'", output_path)
    with open(output_path, "wb") as f:
        rec = (
            (cfg.background.encode() + b"\0")
            + struct.pack("<H", len(cfg.options))
            + b"".join(
                [
                    struct.pack("<HHHH", o.hot[0], o.hot[1], o.hot[2], o.hot[3])
                    + struct.pack(
                        "<HH",
                        o.selected_pos[0],
                        o.selected_pos[1],
                    )
                    + (o.selected_overlay.encode() + b"\0")
                    + (o.submenu.encode() + b"\0")
                    for o in cfg.options
                ]
            )
        )

        f.write(b"CFmn")
        f.write(struct.pack("<I", len(rec)))
        f.write(rec)
    return 0


def write_submenu_cfg(args: Args, output_path: str, cfg: SubmenuConfig) -> bool | None:
    logger.info("Writing submenu config '%s'", output_path)
    with open(output_path, "wb") as f:
        rec = (
            (cfg.menu_header.encode() + b"\0")
            + (cfg.menu_footer.encode() + b"\0")
            + (cfg.option_unselected_background.encode() + b"\0")
            + (cfg.option_selected_background.encode() + b"\0")
            + struct.pack(
                "<hh", cfg.option_selected_offset[0], cfg.option_selected_offset[1]
            )
            + struct.pack("<hh", cfg.option_label_offset[0], cfg.option_label_offset[1])
            + struct.pack(
                "<BB",
                cfg.option_unselected_label_attribute,
                cfg.option_selected_label_attribute,
            )
            + struct.pack("<H", len(cfg.options))
            + b"".join(
                [
                    (o.label.encode() + b"\0") + (o.viewer_file.encode() + b"\0")
                    for o in cfg.options
                ]
            )
        )

        f.write(b"CFsm")
        f.write(struct.pack("<I", len(rec)))
        f.write(rec)
    return 0


def write_outro_cfg(args: Args, output_path: str, cfg: OutroConfig) -> bool | None:
    logger.info("Writing outro config '%s'", output_path)
    with open(output_path, "wb") as f:
        rec = b""

        f.write(b"CFsm")
        f.write(struct.pack("<I", len(rec)))
        f.write(rec)
    return 0


def main(args: Args):
    assert len(args.input_path) == 1
    input_path = args.input_path[0]

    if args.root_path == None:
        args.root_path = os.path.split(input_path)[0]
    args.root_path = os.path.normpath(args.root_path)

    if args.output_path != None:
        output_path = args.output_path
    else:
        output_path = os.path.splitext(input_path)[0] + ".cfg"
        if output_path == input_path:
            logger.error(
                "Output must be specified -- default would overwrite input file"
            )
            return None
    if os.path.exists(output_path) and os.path.samefile(output_path, input_path):
        logger.warning("Output path is the same file as input path")

    cfg = read_json_config(args, input_path)
    if cfg == None:
        return 1
    if isinstance(cfg, IntroConfig):
        write_res = write_intro_cfg(args, output_path, cfg)
    elif isinstance(cfg, MenuConfig):
        write_res = write_menu_cfg(args, output_path, cfg)
    elif isinstance(cfg, SubmenuConfig):
        write_res = write_submenu_cfg(args, output_path, cfg)
    elif isinstance(cfg, OutroConfig):
        write_res = write_outro_cfg(args, output_path, cfg)
    return write_res if write_res != None else 1


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
