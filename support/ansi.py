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

    logger: logging.Logger

    tiles: np.ndarray

    start_row: int
    screen_rows: int
    total_rows: int | None
    cols: int
    linefeed_is_newline: bool

    cur_weight: Weight
    cur_italic: bool
    cur_underline: bool
    cur_strike: bool
    cur_blink: bool
    cur_reverse: bool
    cur_conceal: bool
    cur_fg: int
    cur_bg: int

    cur_attribute: int

    def __init__(
        self,
        screen_rows: int = 25,
        total_rows: int | None= None,
        cols: int = 80,
        linefeed_is_newline: bool = False,
        logger: logging.Logger = None,
    ):
        """Initializes the DosAnsi with rows*cols white-on-black spaces"""
        self.logger = logger

        self.screen_rows = screen_rows
        self.total_rows = total_rows
        self.cols = cols
        self.linefeed_is_newline = linefeed_is_newline

        self.reset()

    def reset(self):
        self.start_row = 0
        self.cur_x = 0
        self.cur_y = 0

        self.reset_attribute_state()
        self.derive_attribute_code()

        self.tiles = np.full(
            self.screen_rows * self.cols,
            0 | self.attribute,
            dtype=np.uint16,
        )

    def reset_attribute_state(self):
        self.cur_weight = Weight.NORMAL
        self.cur_italic = False
        self.cur_underline = False
        self.cur_strike = False
        self.cur_blink = False
        self.cur_reverse = False
        self.cur_conceal = False
        self.cur_fg = 7
        self.cur_bg = 0

    def derive_attribute_code(self):
        bright = (
            (self.cur_weight == Weight.BOLD)
            or self.cur_italic
            or self.cur_underline
            or self.cur_strike
        )
        fg = self.cur_fg
        bg = self.cur_bg
        if self.cur_reverse:
            fg, bg = bg, fg
        if bright:
            fg |= 8
        bg &= 7
        if self.cur_blink:
            bg |= 8
        if self.cur_conceal:
            fg &= 7
            bg = fg

        self.attribute = (fg | (bg << 4)) << 8

    _trivial_sgr: dict[int, tuple[str, any]] = {
        1: ("cur_weight", Weight.BOLD),  #!
        # 2: ("cur_weight", Weight.FAINT),
        # 3: ("cur_italic", True),
        # 4: ("cur_underline", True),
        5: ("cur_blink", True),
        # 6: ("cur_blink", True),  # fast blink
        # 7: ("cur_reverse", True),
        # 8: ("cur_conceal", True),
        # 9: ("cur_strike", True),
        # # 10 .. 20: Font selection
        # 21: ("cur_underline", True),  # double underline
        # 22: ("cur_weight", Weight.NORMAL),
        # 23: ("cur_italic", False),
        # 24: ("cur_underline", False),
        # 25: ("cur_blink", False),
        # # 26: use proportional font?
        # 27: ("cur_reverse", False),
        # 28: ("cur_conceal", False),
        # 29: ("cur_strike", False),
        # # 50: disable proportional font?
        # # 38: set fg color extended
        # 39: ("cur_fg", 7),
        # # 48: set bg color extended
        # 49: ("cur_bg", 0),
    }

    _color_table = [0, 4, 2, 6, 1, 5, 3, 7]

    def _parse_sgr(self, params):
        """Handles <escape code>n[;k]m, which changes the graphic rendition"""
        param = params.pop(0)

        if param == 0:
            self.reset_attribute_state()
        elif param in DosAnsi._trivial_sgr:
            name, value = DosAnsi._trivial_sgr[param]
            self.__setattr__(name, value)
        elif param >= 30 and param <= 37:
            self.cur_fg = DosAnsi._color_table[param - 30]
        elif param >= 40 and param <= 47:
            self.cur_bg = DosAnsi._color_table[param - 40]
        # elif param >= 90 and param <= 97:
        #     self.cur_fg = DosAnsi._color_table[param - 90] + 8
        # elif param >= 100 and param <= 107:
        #     self.cur_bg = DosAnsi._color_table[param - 100]
        else:
            self.logger.warning(f"Unhandled SGR {param}")

        self.derive_attribute_code()

        return params

    def _evaluate_private_csi(self, params, interm, final, data):
        self.logger.warning(
            "unimplemented private CSI sequence - %s(%s)%s",
            interm + ":" if interm else "",
            params,
            final,
        )
        # if final == b"r":  # TODO?
        #     pass
        # # Save / restore xterm icon; we can ignore this.
        # elif final == b"t":
        #     pass
        return data

    def _parse_csi(self, data):
        csi_match = DosAnsi._csi_parser.match(data)

        if not csi_match:
            self.logger.error(f"invalid CSI sequence; data[:20] = {data[:20]:r}")
            return data

        params, interm, final = csi_match.groups()
        csi_text = csi_match[0]
        data = data[csi_match.end() :]

        # Is this a private sequence? We can recognize some of them. We'll try to parse.
        if DosAnsi._csi_priv_param_pattern.findall(
            params
        ) or DosAnsi._csi_priv_final_pattern.findall(final):
            return self._evaluate_private_csi(params, interm, final, data)

        # The colon is not in any valid CSI sequences (currently).
        if b":" in params:
            self.logger.error(f"CSI parameters contain invalid character: {csi_text:r}")
            return data

        # Standard CSI codes do not have intermediate characters; show an error if we find them.
        if interm:
            self.logger.error(
                f"CSI code with invalid intermediate characters: {csi_text:r}"
            )

        # If arguments are omitted, add the default argument for this sequence.
        if not params:
            if final in b"@ABCDEFGILMPSTXZ`abde":
                numbers = [1]
            elif final in b"Hf":
                numbers = [1, 1]
            else:
                numbers = [0]
        else:
            numbers = list(map(int, params.split(b";")))

        return self._evaluate_csi_sequence(final, numbers, data)

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
            self.logger.error(f"invalid escape sequence; data = {data[:20]:r}...")
            return data[1:]  # Skip the escape character and return.

        # Extract the intermediate and final characters, along with sequence text (for errors)
        esc_interm, esc_final = esc_match.groups()
        esc_text = esc_match[0]
        data = data[esc_match.end() :]

        # If this is a CSI sequence, parse the CSI elements.
        if esc_final == b"[":
            if esc_interm:
                self.logger.warning(f"Intermediate chars in CSI escape: {esc_text:r}")
            return self._parse_csi(data)

        elif esc_interm == b"":
            # if esc_final == b"7":  # Save cursor position / attributes
            #     pass
            # elif esc_final == b"8":  # Restore cursor position / attributes
            #     pass
            # elif (
            #     esc_final in b"ABCIJK"
            # ):  # VT52 are sequences not supported due to conflicts
            #     self.logger.warning(f"skipping VT52 sequence: {esc_text:r}")
            # elif esc_final == b"D":  # Line feed
            #     pass
            # elif esc_final == b"E":  # New line
            #     pass
            # elif esc_final == b"F":  # Move cursor to lower left corner of screen
            #     pass
            # elif esc_final == b"G":  # Select UTF-8 (ignore?)
            #     pass
            # elif esc_final == b"H":  # Set tab stop... implement?
            #     pass
            # elif esc_final == b"M":  # Reverse line fee
            #     pass
            # elif esc_final == b"c":  # Reset terminal
            #     self.reset()
            # elif (
            #     esc_final in b"=>NOPXZ\\]^_lm"
            # ):  # Ignored codes (not relevant for state / unused)
            #     pass
            if False:
                pass
            elif esc_final in b"no|}~":  # Character set commands
                self.logger.warning(
                    f"Character set command not implemented: {esc_text:r}"
                )
            elif (
                esc_final in b"01234569:;<?"
            ):  # Private codes not used by Linux terminal
                self.logger.warning(f"Skipping unknown private sequence: {esc_text:r}")
            else:  # Non-private, invalid codes
                self.logger.error(f"Skipping unknown public sequence: {esc_text:r}")

            if esc_final in b"78DEFGHMc":
                self.logger.warning(f"Unimplemented public sequence: {esc_text:r}")

        # TODO: If this is a graphcs mode change, catch it.
        elif esc_final in b"012ABKU" and esc_interm in b"()*+":
            if esc_interm in b')*"':
                self.logger.warning(
                    f"G1,G2,G3 character sets not implemented: {esc_text:r}"
                )
                return data

            if esc_final in b"12":
                self.logger.warning(
                    f"No support for no-standard graphics; using default: {esc_text:r}"
                )
                esc_final = b"0"
            if esc_final in b"AUK":
                self.logger.warning(
                    f"No support for UK, null, or user mapping; using ASCII: {esc_text:r}"
                )
                esc_final = b"B"

            if esc_final == b"0":
                # Switch to special graphics (line drawing) mode for G0
                self.logger.warning("No support for DEC special line drawing mode")
                pass
            else:  # Only case is ASCII move for G0
                pass

        # elif esc_final in b"@G8" and esc_interm == b"%":
        #     pass

        # # Character size / alignment
        # elif esc_final in b"34568" and esc_interm == b"#":
        #     pass

        elif esc_final in b"0123456789:;<=>?":  # Ignore unknown private sequences
            self.logger.warning(f"Skipping unknown private sequence: {esc_text:r}")

        else:
            self.logger.error(f"Skipping unknown public sequence: {esc_text:r}")

        return data

    def _screen_index(self, x, y):
        return (self.start_row + y) * self.cols + x

    def _evaluate_csi_sequence(self, final, numbers, data):
        """
        Evaluates a sequence (i.e., this changes the state of the terminal).
        Is meant to be called with the return values from _parse_sequence as arguments.
        """

        # Insert indicated number of blanks
        # if final == b"@":
        #     pass  # TODO?
        if False:
            pass
        # Move cursor up
        elif final == b"A":
            self.cur_y -= numbers[0]
        # Move cursor down
        elif final in b"B": # also "e"
            self.cur_y += numbers[0]
        # Move cursor right
        elif final in b"C": # also "a"
            self.cur_x += numbers[0]
        # Move cursor left
        elif final == b"D":
            self.cur_x -= numbers[0]
        # # Move cursor down by # and over to column 1
        # elif final == b"E":
        #     self.cur_y += numbers[0]
        #     self.cur_x = 0
        # # Move cursor up by # and over to column 1
        # elif final == b"F":
        #     self.cur_y -= numbers[0]
        #     self.cur_x = 0
        # # Move cursor to indicated column in the current row
        # elif final in b"G`":
        #     self.cur_x = numbers[0] - 1
        # # Sets cursor position
        # elif final in b"Hf":
        #     self.cur_y = numbers[0] - 1  # 1-based indexes
        #     self.cur_x = numbers[1] - 1  #

        # # Move forward by number of tabs (implement?)
        # elif final == b"I":
        #     pass

        # # Clears (parts of) the screen.
        # elif final == b"J":
        #     from_index = self._screen_index(0, 0)
        #     to_index = self._screen_index(self.cols, self.rows)
        #     # From cursor to end of screen
        #     if numbers[0] == 0:
        #         from_index = self._screen_index(self.cur_x, self.cur_y)
        #     # From beginning to cursor
        #     elif numbers[0] == 1:
        #         to_index = self._screen_index(self.cur_x, self.cur_y)
        #     # The whole screen
        #     elif numbers[0] == 2:
        #         pass
        #     else:
        #         self.logger.error(f"Unknown argument(s) in CSI command J{numbers}")
        #         return data
        #     self.tiles[from_index:to_index].fill(0 | self.attribute)

        # # Clears (parts of) the line
        # elif final == b"K":
        #     from_index = self._screen_index(0, self.cur_y)
        #     to_index = self._screen_index(self.cols, self.cur_y)
        #     # From cursor to end of line
        #     if numbers[0] == 0:
        #         from_index = self._screen_index(self.cur_x, self.cur_y)
        #     # From beginning of line to cursor
        #     elif numbers[0] == 1:
        #         to_index = self._screen_index(self.cur_x, self.cur_y)
        #     # The whole line
        #     elif numbers[0] == 2:
        #         pass
        #     else:
        #         self.logger.error(f"Unknown argument(s) in CSI command J{numbers}")
        #         return data
        #     self.tiles[from_index:to_index].fill(0 | self.attribute)

        # # Insert # of blank lines at current row
        # elif final == b"L":
        #     pass
        # # Delete # of blank lines at current row *OR* get mouse click... if no params and mouse on?
        # elif final == b"M":
        #     pass
        # # Delete indicated number of characters
        # elif final == b"P":
        #     pass
        # # Erase indicated number of characters to the right
        # elif final == b"X":
        #     pass

        # # Move backward by number of tabs (implement?)
        # elif final == b"Z":
        #     pass

        # # Repeat preceding character indicated number of times
        # elif final == b"b":
        #     pass

        # # Move cursor to indicated row
        # elif final == b"d":
        #     self.cur_y = numbers[0] - 1

        # elif final == b"g":  # TODO?
        #     pass
        # elif final == b"h":  # TODO?
        #     pass
        # elif final == b"l":  # TODO?
        #     pass

        # Sets color/boldness
        elif final == b"m":
            while numbers:
                numbers = self._parse_sgr(numbers)

        # elif final in b"STcin":  # Ignore? Scrolling, etc
        #     pass

        else:
            self.logger.error(f"Unknown CSI sequence: {final}{numbers}")

        if final in b"ILMPXZbghl":
            self.logger.warning(f"CSI sequence not implemented: {final}{numbers}")

        return data

    def feed(self, data):
        """Feeds the terminal with input."""
        while data:
            assert self.cur_x >= 0
            assert self.cur_x < self.cols
            assert self.cur_y >= 0
            assert self.cur_y < self.screen_rows
            assert self.tiles.shape[0] == (self.start_row + self.screen_rows) * self.cols

            if data[0] == 0x1A:  # 26, DOS EOF
                break

            # If the data starts with \x1b, try to parse end evaluate a sequence.
            if data[0] == 0x1B:  # ESC
                data = self._parse_sequence(data)
                self._fix_cursor()
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
            data = data[1:]

    def _scroll_if_needed(self):
        if self.cur_y >= self.screen_rows:
            self.start_row += 1
            self.tiles = np.concatenate(
                [self.tiles, np.full(self.cols, 0 | self.attribute, dtype=np.uint16)]
            )
            self.cur_y -= 1
        assert self.cur_y <= self.screen_rows

    def _fix_cursor(self):
        """
        Makes sure the cursor are within the boundaries of the current terminal
        size.
        """
        if self.cur_x < 0:
            self.cur_x = 0
        while self.cur_x >= self.cols:
            self.cur_y += 1
            self.cur_x = self.cur_x - self.cols
        if self.cur_y < 0:
            self.cur_y = 0
        if self.cur_y >= self.screen_rows:
            self.cur_y = self.screen_rows - 1
