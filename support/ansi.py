# Copyright (c) 2010 Helge Milde
# Copyright (C) 2018 Jonathan Hayase
# Copyright (C) 2020 Jeremiah Blanchard, Cacti Council Inc.
# Copyright (C) 2024 Joseph Lunderville

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


# http://www.roysac.com/learn/ansisys.html


from dataclasses import dataclass
from enum import IntEnum, auto
import logging
import numpy as np
import re
from typing import Callable

import __main__

if "__appname__" in dir(__main__):
    logging_name = __main__.__appname__ + ".ansi"
else:
    logging_name = "ansi"

logger = logging.getLogger(logging_name)


class Weight(IntEnum):
    NORMAL = auto()
    BOLD = auto()
    FAINT = auto()


class DosAnsi:
    # Character types in an ANSI escape sequence
    _intermediate = re.escape(bytes(range(0x20, 0x30)))
    _esc_final = re.escape(bytes(range(0x30, 0x7F)))

    # CSI sequence-specific character types (subsets of ANSI final characters)
    _csi_param = re.escape(bytes(range(0x30, 0x40)))
    _csi_final = re.escape(bytes(range(0x40, 0x7F)))

    # CSI private param / final characters
    _csi_priv_param = re.escape(b"<=>?")
    _csi_priv_final = re.escape(bytes(range(0x70, 0x7F)))

    # Escape sequence:  { ESC [I]* F }; capture intermediates and final.
    # CSE sequence "[": { [P]* [I]* F }; capture params, intermediates, and final.
    _escape_parser = re.compile(b"^\\x1b([%s]*)([%s])" % (_intermediate, _esc_final))
    _csi_parser = re.compile(
        b"^([%s]*)([%s]*)([%s])" % (_csi_param, _intermediate, _csi_final)
    )
    _csi_priv_param_pattern = re.compile(b"[%s]" % _csi_priv_param)
    _csi_priv_final_pattern = re.compile(b"[%s]" % _csi_priv_final)

    _color_table = [0, 4, 2, 6, 1, 5, 3, 7]

    tiles: np.ndarray

    start_row: int
    screen_rows: int
    rows_used: int
    cols: int
    fill: int
    linefeed_is_newline: bool

    attribute_default: bool

    cur_bold: bool
    cur_blink: bool
    cur_fg: int
    cur_bg: int

    cur_attribute: int

    _already_warned: set[str]

    def already_warned(self, name: str) -> None:
        result = name in self._already_warned
        self._already_warned.add(name)
        return result

    def __init__(
        self,
        screen_rows: int = 25,
        cols: int = 80,
        fill: int = 0 | 0x0700,
        linefeed_is_newline: bool = False,
    ):
        """Initializes the DosAnsi with rows*cols white-on-black spaces"""
        self._already_warned = set()

        self.screen_rows = screen_rows
        self.rows_used = 1
        self.cols = cols
        self.fill = fill
        self.linefeed_is_newline = linefeed_is_newline

        self.reset()

    def reset(self):
        self.start_row = 0
        self.cur_x = 0
        self.cur_y = 0

        self._reset_attribute_state()
        self._derive_attribute_code()

        self.tiles = np.full(
            self.screen_rows * self.cols,
            0 | self.attribute,
            dtype=np.uint16,
        )

    def feed(self, data):
        """Feeds the terminal with input."""
        while data:
            assert self.cur_x >= 0
            assert self.cur_x < self.cols
            assert self.cur_y >= 0
            assert self.cur_y < self.screen_rows
            assert (
                self.tiles.shape[0] == (self.start_row + self.screen_rows) * self.cols
            )

            if data[0] == 0x1A:  # 26, DOS EOF
                break

            # If the data starts with \x1b, try to parse end evaluate a sequence.
            if data[0] == 0x1B:  # ESC
                data = self._parse_sequence(data)
                continue

            # If we end up here, the character should should just be
            # added to the current tile and the cursor should be updated.
            # Some characters such as \r, \n will only affect the cursor.
            a = data[0]
            if a == 0x0D:  # CR
                self.cur_x = 0
            elif a == 0x08:  # BS
                if self.cur_x > 0:
                    self.cur_x -= 1
                else:
                    if self.cur_y > 0:
                        self.cur_y -= 1
                        self.cur_x = self.cols - 1
                    else:
                        self.cur_x = 0
            elif a == 0x0A:  # LF/NL
                self.cur_y += 1
                self._scroll_if_needed()
                if self.linefeed_is_newline:
                    self.cur_x = 0
            elif a == 0x0F or a == 0:
                pass
            else:
                self.tiles[self._screen_index(self.cur_x, self.cur_y)] = (
                    a | self.attribute
                )
                self.cur_x += 1
                if self.cur_x >= self.cols:
                    self.cur_y += 1
                    self.cur_x -= self.cols
                    self._scroll_if_needed()
            self.rows_used = max(self.rows_used, self.start_row + self.cur_y + 1)
            data = data[1:]

    def _parse_sequence(self, data):
        """
        This method parses the input into the numeric arguments and
        the type of sequence. If no numeric arguments are supplied,
        we manually insert a 0 or a 1 depending on the sequence type,
        because different types have different default values.

        Example 1: \x1b[1;37;40m -> numbers=[1, 37, 40] char=m
        Example 2: \x1b[m = numbers=[0] char=m
        """
        esc_match = DosAnsi._escape_parser.match(data)

        # If there's no match, but there was an escape character, send warning and try to recover.
        if not esc_match:
            logger.error(f"invalid escape sequence; data = {data[:20]:r}...")
            return data[1:]  # Skip the escape character and return.

        # Extract the intermediate and final characters, along with sequence text (for errors)
        esc_interm, esc_final = esc_match.groups()
        esc_text = esc_match[0]
        data = data[esc_match.end() :]

        # If this is a CSI sequence, parse the CSI elements.
        if esc_final == b"[":
            if esc_interm:
                logger.warning(f"Intermediate chars in CSI escape: {esc_text:r}")
            return self._parse_csi(data)
        else:
            logger.error(f"Skipping unknown sequence: {esc_text:r}")

        return data

    def _parse_csi(self, data):
        csi_match = DosAnsi._csi_parser.match(data)

        if not csi_match:
            logger.error(f"invalid CSI sequence; data[:20] = {data[:20]:r}")
            return data

        params, interm, final = csi_match.groups()
        csi_text = csi_match[0]
        data = data[csi_match.end() :]

        # Is this a private sequence? We can recognize some of them. We'll try to parse.
        if final == b"t":
            if not self.already_warned("csi_t"):
                logger.warning("ignoring 't' private CSI sequences for RGB colors")
            return data
        elif DosAnsi._csi_priv_param_pattern.findall(
            params
        ) or DosAnsi._csi_priv_final_pattern.findall(final):
            logger.warning(
                "unimplemented private CSI sequence - %r",
                (interm + b":" if interm else b"") + b"(" + params + b")" + final,
            )
            return data

        # The colon is not in any valid CSI sequences (currently).
        if b":" in params:
            logger.error(f"CSI parameters contain invalid character: {csi_text:r}")
            return data

        # Standard CSI codes do not have intermediate characters; show an error if we find them.
        if interm:
            logger.error(f"CSI code with invalid intermediate characters: {csi_text:r}")

        # If arguments are omitted, add the default argument for this sequence.
        if params:
            numbers = list(map(int, params.split(b";")))
        else:
            numbers = None

        # Move cursor up
        if final == b"A":
            self.cur_y = max(self.cur_y - (numbers or [1])[0], 0)
        # Move cursor down
        elif final in b"B":  # also "e"
            self.cur_y = min(self.cur_y + (numbers or [1])[0], self.screen_rows - 1)
        # Move cursor right
        elif final in b"C":  # also "a"
            self.cur_x = min(self.cur_x + (numbers or [1])[0], self.cols - 1)
        # Move cursor left
        elif final == b"D":
            self.cur_x = max(self.cur_x - (numbers or [1])[0], 0)
        # Sets color/boldness
        elif final == b"m":
            while numbers:
                numbers = self._parse_sgr(numbers or [0])
        else:
            logger.error(f"Unknown CSI sequence: {final}{numbers}")

        return data

    def _parse_sgr(self, params):
        """Handles <escape code>n[;k]m, which changes the graphic rendition"""
        param = params.pop(0)

        if param == 0:
            self._reset_attribute_state()
        elif param == 1:
            self.attribute_default = False
            self.cur_bold = True
        elif param == 5:
            self.attribute_default = False
            self.cur_blink = True
        elif param >= 30 and param <= 37:
            self.attribute_default = False
            self.cur_fg = DosAnsi._color_table[param - 30]
        elif param >= 40 and param <= 47:
            self.attribute_default = False
            self.cur_bg = DosAnsi._color_table[param - 40]
        else:
            logger.warning(f"Unhandled SGR {param}")

        self._derive_attribute_code()

        return params

    def _scroll_if_needed(self):
        if self.cur_y >= self.screen_rows:
            self.start_row += 1
            if self.attribute_default and (self.fill is not None):
                fill = self.fill
            else:
                fill = 0 | self.attribute
            self.tiles = np.concatenate(
                [self.tiles, np.full(self.cols, fill, dtype=np.uint16)]
            )
            self.cur_y -= 1
        assert self.cur_y <= self.screen_rows

    def _screen_index(self, x, y):
        return (self.start_row + y) * self.cols + x

    def _reset_attribute_state(self):
        self.attribute_default = True
        self.cur_bold = False
        self.cur_blink = False
        self.cur_fg = 7
        self.cur_bg = 0

    def _derive_attribute_code(self):
        fg = self.cur_fg
        bg = self.cur_bg
        if self.cur_bold:
            fg |= 8
        bg &= 7
        if self.cur_blink:
            bg |= 8

        self.attribute = (fg | (bg << 4)) << 8
