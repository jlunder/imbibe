from enum import Enum
import re
import sys


class Tile:
    """Represents a single tile in the terminal"""
    def __init__(self):
        self.color = self.glyph = None
        self.reset()


    def reset(self):
        """Resets the tile to a white-on-black space"""
        self.color = {'fg': 37, 'bg': 40, 'reverse': False, 'bold': False}
        self.glyph = ' '


    def set(self, glyph, color):
        self.glyph = glyph
        self.color['fg'] = color['fg']
        self.color['bg'] = color['bg']
        self.color['reverse'] = color['reverse']
        self.color['bold'] = color['bold']


class AnsiTerm:
    # Special graphics characters for box-drawing, etc.
    _special_graphics_chars = '◆▒␉␌␍␊°±␤␋┘┐┌└┼⎺⎻─⎼⎽├┤┴┬│≤≥π≠£·'

    # Character types in an ANSI escape sequence
    _intermediate = re.escape(''.join([chr(val) for val in range(0x20, 0x30)]))
    _esc_final = re.escape(''.join([chr(val) for val in range(0x30, 0x7f)]))

    # CSI sequence-specific character types (subsets of ANSI final characters)
    _csi_param = re.escape(''.join([chr(val) for val in range(0x30, 0x40)]))
    _csi_final = re.escape(''.join([chr(val) for val in range(0x40, 0x7f)]))

    # CSI private param / final characters
    _csi_priv_param = re.escape("<=>?")
    _csi_priv_final = re.escape(''.join([chr(val) for val in range(0x70, 0x7f)]))

    # Escape sequence:  { ESC [I]* F }; capture intermediates and final.
    # CSE sequence "[": { [P]* [I]* F }; capture params, intermediates, and final.
    _escape_parser = re.compile('^\\x1b([%s]*)([%s])' % (_intermediate, _esc_final))
    _csi_parser = re.compile('^([%s]*)([%s]*)([%s])' % (_csi_param, _intermediate, _csi_final))
    _csi_priv_param_pattern = re.compile('[%s]' % _csi_priv_param)
    _csi_priv_final_pattern = re.compile('[%s]' % _csi_priv_final)


    def __init__(self, rows, cols, linefeed_is_newline=True, silent=False):
        """Initializes the AnsiTerm with rows*cols white-on-black spaces"""
        self.rows = rows
        self.cols = cols
        self.silent = silent
        self.tiles = [Tile() for _ in range(rows * cols)]
        self.linefeed_is_newline = linefeed_is_newline
        self.nl = '\n' if linefeed_is_newline else '\r\n'
        self.reset()


    def reset(self):
        self.graphics_mode = False
        self.cursor = {'x': 0, 'y': 0}
        self.color = {'fg': 37, 'bg': 40, 'bold': False, 'reverse': False}
        [tile.reset() for tile in self.tiles]


    def errlog(self, text):
        if not self.silent:
            sys.stderr.write(text + "\n")


    def get_screen(self):
        return ''.join([tile.glyph + self.nl if (index + 1) % self.cols == 0 else tile.glyph for index, tile in enumerate(self.tiles)])


    def get_string(self, from_, to):
        """Returns the character of a section of the screen"""
        return ''.join([tile.glyph for tile in self.get_tiles(from_, to)])


    def get_tiles(self, from_, to):
        """Returns the tileset of a section of the screen"""
        return [tile for tile in self.tiles[from_:to]]


    def get_cursor(self):
        """Returns the current position of the curser"""
        return self.cursor.copy()


    def _parse_sgr(self, params):
        """Handles <escape code>n[;k]m, which changes the graphic rendition"""
        param = params.pop(0)

        if param == 0:
            self.color['fg'] = 37
            self.color['bg'] = 40
            self.color['bold'] = False
            self.color['reverse'] = False
        elif param == 1:
            self.color['bold'] = True
        elif param == 7:
            self.color['reverse'] = True
        # Special text decoration; could be supported in a future version potentially.
        elif param > 7 and param < 30:
            pass
        elif param >= 30 and param <= 37:
            self.color['fg'] = param
        # Extended foreground color set command
        elif param == 38:
            subtype = params.pop(0)
            # Colors in T.416; implement later? See https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
            if subtype == 5:
                color_code = params.pop(0)
            # 24-bit colors; implement later? See https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit
            elif subtype == 2:
                red, green, blue = params[0:3]
                del param[:3]
        elif param == 39:
            self.color['fg'] = 37

        elif param >= 40 and param <= 47:
            self.color['bg'] = param
        # Extended foreground color set command
        elif param == 48:
            subtype = params.pop(0)
            # Colors in T.416; implement later? See https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
            if subtype == 5:
                color_code = params.pop(0)
            # 24-bit colors; implement later? See https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit
            elif subtype == 2:
                red, green, blue = params[0:3]
                del param[:3]
        elif param == 49:
            self.color['bg'] = 40

        # Additional docorative codes
        elif param > 49 and param < 56:
            pass
        # Additional docorative codes / color codes
        elif param > 57 and param < 66:
            pass
        # Superscript / subscript codes
        elif param == 73 or param == 74:
            pass
        # Bright foreground colors
        elif param > 89 and param < 98:
            pass
        # Bright background colors
        elif param > 99 and param < 108:
            pass
        # Unknown mode code
        else:
            pass

        return params


    def _evaluate_private_csi(self, params, interm, final, data):
        self.errlog("WARNING: unimplemented private CSI sequence - %s(%s)%s" % (interm + ":" if interm else "", params, final))
        if final == 'r': # TODO?
            pass
        # Save / restore xterm icon; we can ignore this.
        elif final == 't':
            pass
        return data


    def _parse_csi(self, data):
        csi_match = AnsiTerm._csi_parser.match(data)

        if not csi_match:
            self.errlog("ERROR: invalid CSI sequence; data[:20] = %r" % data[:20])
            return data

        params, interm, final = csi_match.groups()
        csi_text = csi_match[0]
        data = data[csi_match.end():]

        # Is this a private sequence? We can recognize some of them. We'll try to parse.
        if AnsiTerm._csi_priv_param_pattern.findall(params) or AnsiTerm._csi_priv_final_pattern.findall(final):
            return self._evaluate_private_csi(params, interm, final, data)

        # The colon is not in any valid CSI sequences (currently).
        if ':' in params:
            self.errlog("ERROR: CSI parameters contain invalid character: %r" % csi_text)
            return data

        # Standard CSI codes do not have intermediate characters; show an error if we find them.
        if interm:
            self.errlog("ERROR: CSI code with invalid intermediate characters: %r" % csi_text)

        # If arguments are omitted, add the default argument for this sequence.
        if not params:
            if final in '@ABCDEFGILMPSTXZ`abde':
                numbers = [1]
            elif final in 'Hf':
                numbers = [1, 1]
            else:
                numbers = [0]
        else:
            numbers = list(map(int, params.split(';')))

        return self._evaluate_csi_sequence(final, numbers, data)


    def _fix_cursor(self):
        """
        Makes sure the cursor are within the boundaries of the current terminal
        size.
        """
        while self.cursor['x'] >= self.cols:
            self.cursor['y'] += 1
            self.cursor['x'] = self.cursor['x'] - self.cols

        if self.cursor['y'] >= self.rows:
            self.cursor['y'] = self.rows - 1


    def _parse_sequence(self, data):
        """
        This method parses the input into the numeric arguments and
        the type of sequence. If no numeric arguments are supplied,
        we manually insert a 0 or a 1 depending on the sequence type,
        because different types have different default values.

        Example 1: \x1b[1;37;40m -> numbers=[1, 37, 40] char=m
        Example 2: \x1b[m = numbers=[0] char=m
        """
        esc_match = AnsiTerm._escape_parser.match(data)

        # If there's no match, but there was an escape character, send warning and try to recover.
        if not esc_match:
            self.errlog('ERROR: invalid escape sequence; data = %r...' % data[:20])
            return data[1:] # Skip the escape character and return.

        # Extract the intermediate and final characters, along with sequence text (for errors)
        esc_interm, esc_final = esc_match.groups()
        esc_text = esc_match[0]
        data = data[esc_match.end():]

        # If this is a CSI sequence, parse the CSI elements.
        if esc_final == '[':
            if esc_interm:
                self.errlog('WARNING: Intermediate chars in CSI escape: %r' % esc_text)
            return self._parse_csi(data)

        elif esc_interm == '':
            if esc_final == '7': # Save cursor position / attributes
                pass
            elif esc_final == '8': # Restore cursor position / attributes
                pass
            elif esc_final in 'ABCIJK': # VT52 are sequences not supported due to conflicts
                self.errlog('WARNING: skipping VT52 sequence: %r' % esc_text)
            elif esc_final == 'D': # Line feed
                pass
            elif esc_final == 'E': # New line
                pass
            elif esc_final == 'F': # Move cursor to lower left corner of screen
                pass
            elif esc_final == 'G': # Select UTF-8 (ignore?)
                pass
            elif esc_final == 'H': # Set tab stop... implement?
                pass
            elif esc_final == 'M': # Reverse line fee
                pass
            elif esc_final == 'c': # Reset terminal
                self.reset()
            elif esc_final in '=>NOPXZ\\]^_lm': # Ignored codes (not relevant for state / unused)
                pass
            elif esc_final in 'no|}~': # Character set commands
                self.errlog('WARNING: Character set command not implemented: %r' % esc_text)
            elif esc_final in '01234569:;<?': # Private codes not used by Linux terminal
                self.errlog('WARNING: Skipping unknown private sequence: %r' % esc_text)
            else: # Non-private, invalid codes
                self.errlog('ERROR: Skipping unknown public sequence: %r' % esc_text)

            if esc_final in "78DEFGHMc":
                self.errlog('WARNING: Unimplemented public sequence: %r' % esc_text)

        # TODO: If this is a graphcs mode change, catch it.
        elif esc_final in '012ABKU' and esc_interm in '()*+':
            if esc_interm in ')*"':
                self.errlog('WARNING: G1,G2,G3 character sets not implemented: %r' % esc_text)
                return data

            if esc_final in '12':
                self.errlog('WARNING: No support for no-standard graphics; using default: %r' % esc_text)
                final = '0'
            if esc_final in 'AUK':
                self.errlog('WARNING: No support for UK, null, or user mapping; using ASCII: %r' % esc_text)
                final = 'B'

            if esc_final == '0': # Switch to special graphics (line drawing) mode for G0
                self.graphics_mode = True
            else: # Only case is ASCII move for G0
                self.graphics_mode = False


        elif esc_final in '@G8' and interm == '%':
            pass

        # Character size / alignment
        elif esc_final in "34568" and interm == "#":
            pass

        elif esc_final in '0123456789:;<=>?': # Ignore unknown private sequences
            self.errlog("WARNING: Skipping unknown private sequence: %r" % esc_text)

        else:
            self.errlog("ERROR: Skipping unknown public sequence: %r" % esc_text)

        return data


    def get_cursor_idx(self):
        return self.cursor['y'] * self.cols + self.cursor['x']


    def _evaluate_csi_sequence(self, final, numbers, data):
        """
        Evaluates a sequence (i.e., this changes the state of the terminal).
        Is meant to be called with the return values from _parse_sequence as arguments.
        """
        # Translate the cursor into an index into our 1-dimensional tileset.
        curidx = self.get_cursor_idx()

        # Insert indicated number of blanks
        if final == '@':
            pass # TODO?
        # Move cursor up
        elif final == 'A':
            self.cursor['y'] -= numbers[0]
        # Move cursor down
        elif final in 'Be':
            self.cursor['y'] += numbers[0]
        # Move cursor right
        elif final in 'Ca':
            self.cursor['x'] += numbers[0]
        # Move cursor left
        elif final == 'D':
            self.cursor['x'] -= numbers[0]
        # Move cursor down by # and over to column 1
        elif final == 'E':
            self.cursor['y'] += numbers[0]
            self.cursor['x'] = 0
        # Move cursor up by # and over to column 1
        elif final == 'F':
            self.cursor['y'] -= numbers[0]
            self.cursor['x'] = 0
        # Move cursor to indicated column in the current row
        elif final in 'G`':
            self.cursor['x'] = numbers[0] - 1
        # Sets cursor position
        elif final in 'Hf':
            self.cursor['y'] = numbers[0] - 1 # 1-based indexes
            self.cursor['x'] = numbers[1] - 1 #

        # Move forward by number of tabs (implement?)
        elif final == 'I':
            pass

        # Clears (parts of) the screen.
        elif final == 'J':
            # From cursor to end of screen
            if numbers[0] == 0:
                range_ = (curidx, self.cols - self.cursor['x'] - 1)
            # From beginning to cursor
            elif numbers[0] == 1:
                range_ = (0, curidx)
            # The whole screen
            elif numbers[0] == 2:
                range_ = (0, self.cols * self.rows - 1)
            else:
                self.errlog('ERROR: Unknown argument(s) in CSI command J%s' % numbers)
                return data
            for i in range(*range_):
                self.tiles[i].reset()

        # Clears (parts of) the line
        elif final == 'K':
            # From cursor to end of line
            if numbers[0] == 0:
                range_ = (curidx, curidx + self.cols - self.cursor['x'] - 1)
            # From beginning of line to cursor
            elif numbers[0] == 1:
                range_ = (curidx % self.cols, curidx)
            # The whole line
            elif numbers[0] == 2:
                range_ = (curidx % self.cols, curidx % self.cols + self.cols)
            else:
                self.errlog('ERROR: Unknown argument(s) in CSI command K%s' % numbers)
                return data
            for i in range(*range_):
                self.tiles[i].reset()

        # Insert # of blank lines at current row
        elif final == 'L':
            pass
        # Delete # of blank lines at current row *OR* get mouse click... if no params and mouse on?
        elif final == 'M':
            pass
        # Delete indicated number of characters
        elif final == 'P':
            pass
        # Erase indicated number of characters to the right
        elif final == 'X':
            pass

        # Move backward by number of tabs (implement?)
        elif final == 'Z':
            pass

        # Repeat preceding character indicated number of times
        elif final == 'b':
            pass

        # Move cursor to indicated row
        elif final == 'd':
            self.cursor['y'] = numbers[0] - 1

        elif final == 'g': # TODO?
            pass
        elif final == 'h': # TODO?
            pass
        elif final == 'l': # TODO?
            pass

        # Sets color/boldness
        elif final == 'm':
            while numbers:
                numbers = self._parse_sgr(numbers)

        elif final in 'STcin': # Ignore? Scrolling, etc
            pass

        else:
            self.errlog('ERROR: Unknown CSI sequence: %s%s' % (final, numbers))

        if final in 'ILMPXZbghl':
            self.errlog("WARNING: CSI sequence not implemented: %s%s" % (final, numbers))

        return data


    def feed(self, data):
        """Feeds the terminal with input."""
        while data:
            # If the data starts with \x1b, try to parse end evaluate a sequence.
            if data[0] == '\x1b':
                data = self._parse_sequence(data)
                continue

            # If we end up here, the character should should just be
            # added to the current tile and the cursor should be updated.
            # Some characters such as \r, \n will only affect the cursor.
            # TODO: Find out exactly what should be accepted here.
            #       Only ASCII-7 perhaps?
            a = data[0]
            if a == '\r':
                self.cursor['x'] = 0
            elif a == '\b':
                self.cursor['x'] -= 1
            elif a == '\n':
                self.cursor['y'] += 1
                if self.linefeed_is_newline:
                    self.cursor['x'] = 0
            elif a == '\x0f' or a == '\x00':
                pass
            else:
                # If we're in graphics mode & the character is above 0x5f, use the special glyph.
                if self.graphics_mode and ord(a) >= ord('`'):
                    a = AnsiTerm._special_graphics_chars[ord(a) - ord('`')]

                self.tiles[self.get_cursor_idx()].set(a, self.color)
                self.cursor['x'] += 1
            data = data[1:]
        self._fix_cursor()