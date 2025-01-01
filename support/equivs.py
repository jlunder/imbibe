from dataclasses import dataclass, field
import logging
import numpy as np

import __main__

if "__appname__" in dir(__main__):
    logging_name = __main__.__appname__ + ".equivs"
else:
    logging_name = "equivs"

logger = logging.getLogger(logging_name)

chars_r8 = [
    b"\x00\x00\x00\x00\x00\x00\x00\x00",  # Character 0 (0x00)
    b"\x7E\x81\xA5\x81\xBD\x99\x81\x7E",  # Character 1 (0x01)
    b"\x7E\xFF\xDB\xFF\xC3\xE7\xFF\x7E",  # Character 2 (0x02)
    b"\x6C\xFE\xFE\xFE\x7C\x38\x10\x00",  # Character 3 (0x03)
    b"\x10\x38\x7C\xFE\x7C\x38\x10\x00",  # Character 4 (0x04)
    b"\x38\x7C\x38\xFE\xFE\xD6\x10\x38",  # Character 5 (0x05)
    b"\x10\x10\x38\x7C\xFE\x7C\x10\x38",  # Character 6 (0x06)
    b"\x00\x00\x18\x3C\x3C\x18\x00\x00",  # Character 7 (0x07)
    b"\xFF\xFF\xE7\xC3\xC3\xE7\xFF\xFF",  # Character 8 (0x08)
    b"\x00\x3C\x66\x42\x42\x66\x3C\x00",  # Character 9 (0x09)
    b"\xFF\xC3\x99\xBD\xBD\x99\xC3\xFF",  # Character 10 (0x0A)
    b"\x0F\x07\x0F\x7D\xCC\xCC\xCC\x78",  # Character 11 (0x0B)
    b"\x3C\x66\x66\x66\x3C\x18\x7E\x18",  # Character 12 (0x0C)
    b"\x3F\x33\x3F\x30\x30\x70\xF0\xE0",  # Character 13 (0x0D)
    b"\x7F\x63\x7F\x63\x63\x67\xE6\xC0",  # Character 14 (0x0E)
    b"\x18\xDB\x3C\xE7\xE7\x3C\xDB\x18",  # Character 15 (0x0F)
    b"\x80\xE0\xF8\xFE\xF8\xE0\x80\x00",  # Character 16 (0x10)
    b"\x02\x0E\x3E\xFE\x3E\x0E\x02\x00",  # Character 17 (0x11)
    b"\x18\x3C\x7E\x18\x18\x7E\x3C\x18",  # Character 18 (0x12)
    b"\x66\x66\x66\x66\x66\x00\x66\x00",  # Character 19 (0x13)
    b"\x7F\xDB\xDB\x7B\x1B\x1B\x1B\x00",  # Character 20 (0x14)
    b"\x3E\x63\x38\x6C\x6C\x38\xCC\x78",  # Character 21 (0x15)
    b"\x00\x00\x00\x00\x7E\x7E\x7E\x00",  # Character 22 (0x16)
    b"\x18\x3C\x7E\x18\x7E\x3C\x18\xFF",  # Character 23 (0x17)
    b"\x18\x3C\x7E\x18\x18\x18\x18\x00",  # Character 24 (0x18)
    b"\x18\x18\x18\x18\x7E\x3C\x18\x00",  # Character 25 (0x19)
    b"\x00\x18\x0C\xFE\x0C\x18\x00\x00",  # Character 26 (0x1A)
    b"\x00\x30\x60\xFE\x60\x30\x00\x00",  # Character 27 (0x1B)
    b"\x00\x00\xC0\xC0\xC0\xFE\x00\x00",  # Character 28 (0x1C)
    b"\x00\x24\x66\xFF\x66\x24\x00\x00",  # Character 29 (0x1D)
    b"\x00\x18\x3C\x7E\xFF\xFF\x00\x00",  # Character 30 (0x1E)
    b"\x00\xFF\xFF\x7E\x3C\x18\x00\x00",  # Character 31 (0x1F)
    b"\x00\x00\x00\x00\x00\x00\x00\x00",  # Character 32 (0x20): ' '
    b"\x30\x78\x78\x30\x30\x00\x30\x00",  # Character 33 (0x21): '!'
    b"\x6C\x6C\x6C\x00\x00\x00\x00\x00",  # Character 34 (0x22): '"'
    b"\x6C\x6C\xFE\x6C\xFE\x6C\x6C\x00",  # Character 35 (0x23): '#'
    b"\x30\x7C\xC0\x78\x0C\xF8\x30\x00",  # Character 36 (0x24): '$'
    b"\x00\xC6\xCC\x18\x30\x66\xC6\x00",  # Character 37 (0x25): '%'
    b"\x38\x6C\x38\x76\xDC\xCC\x76\x00",  # Character 38 (0x26): '&'
    b"\x60\x60\xC0\x00\x00\x00\x00\x00",  # Character 39 (0x27): '''
    b"\x18\x30\x60\x60\x60\x30\x18\x00",  # Character 40 (0x28): '('
    b"\x60\x30\x18\x18\x18\x30\x60\x00",  # Character 41 (0x29): ')'
    b"\x00\x66\x3C\xFF\x3C\x66\x00\x00",  # Character 42 (0x2A): '*'
    b"\x00\x30\x30\xFC\x30\x30\x00\x00",  # Character 43 (0x2B): '+'
    b"\x00\x00\x00\x00\x00\x30\x30\x60",  # Character 44 (0x2C): ','
    b"\x00\x00\x00\xFC\x00\x00\x00\x00",  # Character 45 (0x2D): '-'
    b"\x00\x00\x00\x00\x00\x30\x30\x00",  # Character 46 (0x2E): '.'
    b"\x06\x0C\x18\x30\x60\xC0\x80\x00",  # Character 47 (0x2F): '/'
    b"\x7C\xC6\xCE\xDE\xF6\xE6\x7C\x00",  # Character 48 (0x30): '0'
    b"\x30\x70\x30\x30\x30\x30\xFC\x00",  # Character 49 (0x31): '1'
    b"\x78\xCC\x0C\x38\x60\xCC\xFC\x00",  # Character 50 (0x32): '2'
    b"\x78\xCC\x0C\x38\x0C\xCC\x78\x00",  # Character 51 (0x33): '3'
    b"\x1C\x3C\x6C\xCC\xFE\x0C\x1E\x00",  # Character 52 (0x34): '4'
    b"\xFC\xC0\xF8\x0C\x0C\xCC\x78\x00",  # Character 53 (0x35): '5'
    b"\x38\x60\xC0\xF8\xCC\xCC\x78\x00",  # Character 54 (0x36): '6'
    b"\xFC\xCC\x0C\x18\x30\x30\x30\x00",  # Character 55 (0x37): '7'
    b"\x78\xCC\xCC\x78\xCC\xCC\x78\x00",  # Character 56 (0x38): '8'
    b"\x78\xCC\xCC\x7C\x0C\x18\x70\x00",  # Character 57 (0x39): '9'
    b"\x00\x30\x30\x00\x00\x30\x30\x00",  # Character 58 (0x3A): ':'
    b"\x00\x30\x30\x00\x00\x30\x30\x60",  # Character 59 (0x3B): ';'
    b"\x18\x30\x60\xC0\x60\x30\x18\x00",  # Character 60 (0x3C): '<'
    b"\x00\x00\xFC\x00\x00\xFC\x00\x00",  # Character 61 (0x3D): '='
    b"\x60\x30\x18\x0C\x18\x30\x60\x00",  # Character 62 (0x3E): '>'
    b"\x78\xCC\x0C\x18\x30\x00\x30\x00",  # Character 63 (0x3F): '?'
    b"\x7C\xC6\xDE\xDE\xDE\xC0\x78\x00",  # Character 64 (0x40): '@'
    b"\x30\x78\xCC\xCC\xFC\xCC\xCC\x00",  # Character 65 (0x41): 'A'
    b"\xFC\x66\x66\x7C\x66\x66\xFC\x00",  # Character 66 (0x42): 'B'
    b"\x3C\x66\xC0\xC0\xC0\x66\x3C\x00",  # Character 67 (0x43): 'C'
    b"\xF8\x6C\x66\x66\x66\x6C\xF8\x00",  # Character 68 (0x44): 'D'
    b"\xFE\x62\x68\x78\x68\x62\xFE\x00",  # Character 69 (0x45): 'E'
    b"\xFE\x62\x68\x78\x68\x60\xF0\x00",  # Character 70 (0x46): 'F'
    b"\x3C\x66\xC0\xC0\xCE\x66\x3E\x00",  # Character 71 (0x47): 'G'
    b"\xCC\xCC\xCC\xFC\xCC\xCC\xCC\x00",  # Character 72 (0x48): 'H'
    b"\x78\x30\x30\x30\x30\x30\x78\x00",  # Character 73 (0x49): 'I'
    b"\x1E\x0C\x0C\x0C\xCC\xCC\x78\x00",  # Character 74 (0x4A): 'J'
    b"\xE6\x66\x6C\x78\x6C\x66\xE6\x00",  # Character 75 (0x4B): 'K'
    b"\xF0\x60\x60\x60\x62\x66\xFE\x00",  # Character 76 (0x4C): 'L'
    b"\xC6\xEE\xFE\xFE\xD6\xC6\xC6\x00",  # Character 77 (0x4D): 'M'
    b"\xC6\xE6\xF6\xDE\xCE\xC6\xC6\x00",  # Character 78 (0x4E): 'N'
    b"\x38\x6C\xC6\xC6\xC6\x6C\x38\x00",  # Character 79 (0x4F): 'O'
    b"\xFC\x66\x66\x7C\x60\x60\xF0\x00",  # Character 80 (0x50): 'P'
    b"\x78\xCC\xCC\xCC\xDC\x78\x1C\x00",  # Character 81 (0x51): 'Q'
    b"\xFC\x66\x66\x7C\x6C\x66\xE6\x00",  # Character 82 (0x52): 'R'
    b"\x78\xCC\x60\x30\x18\xCC\x78\x00",  # Character 83 (0x53): 'S'
    b"\xFC\xB4\x30\x30\x30\x30\x78\x00",  # Character 84 (0x54): 'T'
    b"\xCC\xCC\xCC\xCC\xCC\xCC\xFC\x00",  # Character 85 (0x55): 'U'
    b"\xCC\xCC\xCC\xCC\xCC\x78\x30\x00",  # Character 86 (0x56): 'V'
    b"\xC6\xC6\xC6\xD6\xFE\xEE\xC6\x00",  # Character 87 (0x57): 'W'
    b"\xC6\xC6\x6C\x38\x38\x6C\xC6\x00",  # Character 88 (0x58): 'X'
    b"\xCC\xCC\xCC\x78\x30\x30\x78\x00",  # Character 89 (0x59): 'Y'
    b"\xFE\xC6\x8C\x18\x32\x66\xFE\x00",  # Character 90 (0x5A): 'Z'
    b"\x78\x60\x60\x60\x60\x60\x78\x00",  # Character 91 (0x5B): '['
    b"\xC0\x60\x30\x18\x0C\x06\x02\x00",  # Character 92 (0x5C): '\'
    b"\x78\x18\x18\x18\x18\x18\x78\x00",  # Character 93 (0x5D): ']'
    b"\x10\x38\x6C\xC6\x00\x00\x00\x00",  # Character 94 (0x5E): '^'
    b"\x00\x00\x00\x00\x00\x00\x00\xFF",  # Character 95 (0x5F): '_'
    b"\x30\x30\x18\x00\x00\x00\x00\x00",  # Character 96 (0x60): '`'
    b"\x00\x00\x78\x0C\x7C\xCC\x76\x00",  # Character 97 (0x61): 'a'
    b"\xE0\x60\x60\x7C\x66\x66\xDC\x00",  # Character 98 (0x62): 'b'
    b"\x00\x00\x78\xCC\xC0\xCC\x78\x00",  # Character 99 (0x63): 'c'
    b"\x1C\x0C\x0C\x7C\xCC\xCC\x76\x00",  # Character 100 (0x64): 'd'
    b"\x00\x00\x78\xCC\xFC\xC0\x78\x00",  # Character 101 (0x65): 'e'
    b"\x38\x6C\x60\xF0\x60\x60\xF0\x00",  # Character 102 (0x66): 'f'
    b"\x00\x00\x76\xCC\xCC\x7C\x0C\xF8",  # Character 103 (0x67): 'g'
    b"\xE0\x60\x6C\x76\x66\x66\xE6\x00",  # Character 104 (0x68): 'h'
    b"\x30\x00\x70\x30\x30\x30\x78\x00",  # Character 105 (0x69): 'i'
    b"\x0C\x00\x0C\x0C\x0C\xCC\xCC\x78",  # Character 106 (0x6A): 'j'
    b"\xE0\x60\x66\x6C\x78\x6C\xE6\x00",  # Character 107 (0x6B): 'k'
    b"\x70\x30\x30\x30\x30\x30\x78\x00",  # Character 108 (0x6C): 'l'
    b"\x00\x00\xCC\xFE\xFE\xD6\xC6\x00",  # Character 109 (0x6D): 'm'
    b"\x00\x00\xF8\xCC\xCC\xCC\xCC\x00",  # Character 110 (0x6E): 'n'
    b"\x00\x00\x78\xCC\xCC\xCC\x78\x00",  # Character 111 (0x6F): 'o'
    b"\x00\x00\xDC\x66\x66\x7C\x60\xF0",  # Character 112 (0x70): 'p'
    b"\x00\x00\x76\xCC\xCC\x7C\x0C\x1E",  # Character 113 (0x71): 'q'
    b"\x00\x00\xDC\x76\x66\x60\xF0\x00",  # Character 114 (0x72): 'r'
    b"\x00\x00\x7C\xC0\x78\x0C\xF8\x00",  # Character 115 (0x73): 's'
    b"\x10\x30\x7C\x30\x30\x34\x18\x00",  # Character 116 (0x74): 't'
    b"\x00\x00\xCC\xCC\xCC\xCC\x76\x00",  # Character 117 (0x75): 'u'
    b"\x00\x00\xCC\xCC\xCC\x78\x30\x00",  # Character 118 (0x76): 'v'
    b"\x00\x00\xC6\xD6\xFE\xFE\x6C\x00",  # Character 119 (0x77): 'w'
    b"\x00\x00\xC6\x6C\x38\x6C\xC6\x00",  # Character 120 (0x78): 'x'
    b"\x00\x00\xCC\xCC\xCC\x7C\x0C\xF8",  # Character 121 (0x79): 'y'
    b"\x00\x00\xFC\x98\x30\x64\xFC\x00",  # Character 122 (0x7A): 'z'
    b"\x1C\x30\x30\xE0\x30\x30\x1C\x00",  # Character 123 (0x7B): '{'
    b"\x18\x18\x18\x00\x18\x18\x18\x00",  # Character 124 (0x7C): '|'
    b"\xE0\x30\x30\x1C\x30\x30\xE0\x00",  # Character 125 (0x7D): '}'
    b"\x76\xDC\x00\x00\x00\x00\x00\x00",  # Character 126 (0x7E): '~'
    b"\x00\x10\x38\x6C\xC6\xC6\xFE\x00",  # Character 127 (0x7F)
    b"\x78\xCC\xC0\xCC\x78\x18\x0C\x78",  # Character 128 (0x80)
    b"\x00\xCC\x00\xCC\xCC\xCC\x7E\x00",  # Character 129 (0x81)
    b"\x1C\x00\x78\xCC\xFC\xC0\x78\x00",  # Character 130 (0x82)
    b"\x7E\xC3\x3C\x06\x3E\x66\x3F\x00",  # Character 131 (0x83)
    b"\xCC\x00\x78\x0C\x7C\xCC\x7E\x00",  # Character 132 (0x84)
    b"\xE0\x00\x78\x0C\x7C\xCC\x7E\x00",  # Character 133 (0x85)
    b"\x30\x30\x78\x0C\x7C\xCC\x7E\x00",  # Character 134 (0x86)
    b"\x00\x00\x78\xC0\xC0\x78\x0C\x38",  # Character 135 (0x87)
    b"\x7E\xC3\x3C\x66\x7E\x60\x3C\x00",  # Character 136 (0x88)
    b"\xCC\x00\x78\xCC\xFC\xC0\x78\x00",  # Character 137 (0x89)
    b"\xE0\x00\x78\xCC\xFC\xC0\x78\x00",  # Character 138 (0x8A)
    b"\xCC\x00\x70\x30\x30\x30\x78\x00",  # Character 139 (0x8B)
    b"\x7C\xC6\x38\x18\x18\x18\x3C\x00",  # Character 140 (0x8C)
    b"\xE0\x00\x70\x30\x30\x30\x78\x00",  # Character 141 (0x8D)
    b"\xC6\x38\x6C\xC6\xFE\xC6\xC6\x00",  # Character 142 (0x8E)
    b"\x30\x30\x00\x78\xCC\xFC\xCC\x00",  # Character 143 (0x8F)
    b"\x1C\x00\xFC\x60\x78\x60\xFC\x00",  # Character 144 (0x90)
    b"\x00\x00\x7F\x0C\x7F\xCC\x7F\x00",  # Character 145 (0x91)
    b"\x3E\x6C\xCC\xFE\xCC\xCC\xCE\x00",  # Character 146 (0x92)
    b"\x78\xCC\x00\x78\xCC\xCC\x78\x00",  # Character 147 (0x93)
    b"\x00\xCC\x00\x78\xCC\xCC\x78\x00",  # Character 148 (0x94)
    b"\x00\xE0\x00\x78\xCC\xCC\x78\x00",  # Character 149 (0x95)
    b"\x78\xCC\x00\xCC\xCC\xCC\x7E\x00",  # Character 150 (0x96)
    b"\x00\xE0\x00\xCC\xCC\xCC\x7E\x00",  # Character 151 (0x97)
    b"\x00\xCC\x00\xCC\xCC\x7C\x0C\xF8",  # Character 152 (0x98)
    b"\xC3\x18\x3C\x66\x66\x3C\x18\x00",  # Character 153 (0x99)
    b"\xCC\x00\xCC\xCC\xCC\xCC\x78\x00",  # Character 154 (0x9A)
    b"\x18\x18\x7E\xC0\xC0\x7E\x18\x18",  # Character 155 (0x9B)
    b"\x38\x6C\x64\xF0\x60\xE6\xFC\x00",  # Character 156 (0x9C)
    b"\xCC\xCC\x78\xFC\x30\xFC\x30\x30",  # Character 157 (0x9D)
    b"\xF8\xCC\xCC\xFA\xC6\xCF\xC6\xC7",  # Character 158 (0x9E)
    b"\x0E\x1B\x18\x3C\x18\x18\xD8\x70",  # Character 159 (0x9F)
    b"\x1C\x00\x78\x0C\x7C\xCC\x7E\x00",  # Character 160 (0xA0)
    b"\x38\x00\x70\x30\x30\x30\x78\x00",  # Character 161 (0xA1)
    b"\x00\x1C\x00\x78\xCC\xCC\x78\x00",  # Character 162 (0xA2)
    b"\x00\x1C\x00\xCC\xCC\xCC\x7E\x00",  # Character 163 (0xA3)
    b"\x00\xF8\x00\xF8\xCC\xCC\xCC\x00",  # Character 164 (0xA4)
    b"\xFC\x00\xCC\xEC\xFC\xDC\xCC\x00",  # Character 165 (0xA5)
    b"\x3C\x6C\x6C\x3E\x00\x7E\x00\x00",  # Character 166 (0xA6)
    b"\x38\x6C\x6C\x38\x00\x7C\x00\x00",  # Character 167 (0xA7)
    b"\x30\x00\x30\x60\xC0\xCC\x78\x00",  # Character 168 (0xA8)
    b"\x00\x00\x00\xFC\xC0\xC0\x00\x00",  # Character 169 (0xA9)
    b"\x00\x00\x00\xFC\x0C\x0C\x00\x00",  # Character 170 (0xAA)
    b"\xC3\xC6\xCC\xDE\x33\x66\xCC\x0F",  # Character 171 (0xAB)
    b"\xC3\xC6\xCC\xDB\x37\x6F\xCF\x03",  # Character 172 (0xAC)
    b"\x18\x18\x00\x18\x18\x18\x18\x00",  # Character 173 (0xAD)
    b"\x00\x33\x66\xCC\x66\x33\x00\x00",  # Character 174 (0xAE)
    b"\x00\xCC\x66\x33\x66\xCC\x00\x00",  # Character 175 (0xAF)
    b"\x22\x88\x22\x88\x22\x88\x22\x88",  # Character 176 (0xB0)
    b"\x55\xAA\x55\xAA\x55\xAA\x55\xAA",  # Character 177 (0xB1)
    b"\xDB\x77\xDB\xEE\xDB\x77\xDB\xEE",  # Character 178 (0xB2)
    b"\x18\x18\x18\x18\x18\x18\x18\x18",  # Character 179 (0xB3)
    b"\x18\x18\x18\x18\xF8\x18\x18\x18",  # Character 180 (0xB4)
    b"\x18\x18\xF8\x18\xF8\x18\x18\x18",  # Character 181 (0xB5)
    b"\x36\x36\x36\x36\xF6\x36\x36\x36",  # Character 182 (0xB6)
    b"\x00\x00\x00\x00\xFE\x36\x36\x36",  # Character 183 (0xB7)
    b"\x00\x00\xF8\x18\xF8\x18\x18\x18",  # Character 184 (0xB8)
    b"\x36\x36\xF6\x06\xF6\x36\x36\x36",  # Character 185 (0xB9)
    b"\x36\x36\x36\x36\x36\x36\x36\x36",  # Character 186 (0xBA)
    b"\x00\x00\xFE\x06\xF6\x36\x36\x36",  # Character 187 (0xBB)
    b"\x36\x36\xF6\x06\xFE\x00\x00\x00",  # Character 188 (0xBC)
    b"\x36\x36\x36\x36\xFE\x00\x00\x00",  # Character 189 (0xBD)
    b"\x18\x18\xF8\x18\xF8\x00\x00\x00",  # Character 190 (0xBE)
    b"\x00\x00\x00\x00\xF8\x18\x18\x18",  # Character 191 (0xBF)
    b"\x18\x18\x18\x18\x1F\x00\x00\x00",  # Character 192 (0xC0)
    b"\x18\x18\x18\x18\xFF\x00\x00\x00",  # Character 193 (0xC1)
    b"\x00\x00\x00\x00\xFF\x18\x18\x18",  # Character 194 (0xC2)
    b"\x18\x18\x18\x18\x1F\x18\x18\x18",  # Character 195 (0xC3)
    b"\x00\x00\x00\x00\xFF\x00\x00\x00",  # Character 196 (0xC4)
    b"\x18\x18\x18\x18\xFF\x18\x18\x18",  # Character 197 (0xC5)
    b"\x18\x18\x1F\x18\x1F\x18\x18\x18",  # Character 198 (0xC6)
    b"\x36\x36\x36\x36\x37\x36\x36\x36",  # Character 199 (0xC7)
    b"\x36\x36\x37\x30\x3F\x00\x00\x00",  # Character 200 (0xC8)
    b"\x00\x00\x3F\x30\x37\x36\x36\x36",  # Character 201 (0xC9)
    b"\x36\x36\xF7\x00\xFF\x00\x00\x00",  # Character 202 (0xCA)
    b"\x00\x00\xFF\x00\xF7\x36\x36\x36",  # Character 203 (0xCB)
    b"\x36\x36\x37\x30\x37\x36\x36\x36",  # Character 204 (0xCC)
    b"\x00\x00\xFF\x00\xFF\x00\x00\x00",  # Character 205 (0xCD)
    b"\x36\x36\xF7\x00\xF7\x36\x36\x36",  # Character 206 (0xCE)
    b"\x18\x18\xFF\x00\xFF\x00\x00\x00",  # Character 207 (0xCF)
    b"\x36\x36\x36\x36\xFF\x00\x00\x00",  # Character 208 (0xD0)
    b"\x00\x00\xFF\x00\xFF\x18\x18\x18",  # Character 209 (0xD1)
    b"\x00\x00\x00\x00\xFF\x36\x36\x36",  # Character 210 (0xD2)
    b"\x36\x36\x36\x36\x3F\x00\x00\x00",  # Character 211 (0xD3)
    b"\x18\x18\x1F\x18\x1F\x00\x00\x00",  # Character 212 (0xD4)
    b"\x00\x00\x1F\x18\x1F\x18\x18\x18",  # Character 213 (0xD5)
    b"\x00\x00\x00\x00\x3F\x36\x36\x36",  # Character 214 (0xD6)
    b"\x36\x36\x36\x36\xFF\x36\x36\x36",  # Character 215 (0xD7)
    b"\x18\x18\xFF\x18\xFF\x18\x18\x18",  # Character 216 (0xD8)
    b"\x18\x18\x18\x18\xF8\x00\x00\x00",  # Character 217 (0xD9)
    b"\x00\x00\x00\x00\x1F\x18\x18\x18",  # Character 218 (0xDA)
    b"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",  # Character 219 (0xDB)
    b"\x00\x00\x00\x00\xFF\xFF\xFF\xFF",  # Character 220 (0xDC)
    b"\xF0\xF0\xF0\xF0\xF0\xF0\xF0\xF0",  # Character 221 (0xDD)
    b"\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F",  # Character 222 (0xDE)
    b"\xFF\xFF\xFF\xFF\x00\x00\x00\x00",  # Character 223 (0xDF)
    b"\x00\x00\x76\xDC\xC8\xDC\x76\x00",  # Character 224 (0xE0)
    b"\x00\x78\xCC\xF8\xCC\xF8\xC0\xC0",  # Character 225 (0xE1)
    b"\x00\xFC\xCC\xC0\xC0\xC0\xC0\x00",  # Character 226 (0xE2)
    b"\x00\xFE\x6C\x6C\x6C\x6C\x6C\x00",  # Character 227 (0xE3)
    b"\xFC\xCC\x60\x30\x60\xCC\xFC\x00",  # Character 228 (0xE4)
    b"\x00\x00\x7E\xD8\xD8\xD8\x70\x00",  # Character 229 (0xE5)
    b"\x00\x66\x66\x66\x66\x7C\x60\xC0",  # Character 230 (0xE6)
    b"\x00\x76\xDC\x18\x18\x18\x18\x00",  # Character 231 (0xE7)
    b"\xFC\x30\x78\xCC\xCC\x78\x30\xFC",  # Character 232 (0xE8)
    b"\x38\x6C\xC6\xFE\xC6\x6C\x38\x00",  # Character 233 (0xE9)
    b"\x38\x6C\xC6\xC6\x6C\x6C\xEE\x00",  # Character 234 (0xEA)
    b"\x1C\x30\x18\x7C\xCC\xCC\x78\x00",  # Character 235 (0xEB)
    b"\x00\x00\x7E\xDB\xDB\x7E\x00\x00",  # Character 236 (0xEC)
    b"\x06\x0C\x7E\xDB\xDB\x7E\x60\xC0",  # Character 237 (0xED)
    b"\x38\x60\xC0\xF8\xC0\x60\x38\x00",  # Character 238 (0xEE)
    b"\x78\xCC\xCC\xCC\xCC\xCC\xCC\x00",  # Character 239 (0xEF)
    b"\x00\xFC\x00\xFC\x00\xFC\x00\x00",  # Character 240 (0xF0)
    b"\x30\x30\xFC\x30\x30\x00\xFC\x00",  # Character 241 (0xF1)
    b"\x60\x30\x18\x30\x60\x00\xFC\x00",  # Character 242 (0xF2)
    b"\x18\x30\x60\x30\x18\x00\xFC\x00",  # Character 243 (0xF3)
    b"\x0E\x1B\x1B\x18\x18\x18\x18\x18",  # Character 244 (0xF4)
    b"\x18\x18\x18\x18\x18\xD8\xD8\x70",  # Character 245 (0xF5)
    b"\x30\x30\x00\xFC\x00\x30\x30\x00",  # Character 246 (0xF6)
    b"\x00\x76\xDC\x00\x76\xDC\x00\x00",  # Character 247 (0xF7)
    b"\x38\x6C\x6C\x38\x00\x00\x00\x00",  # Character 248 (0xF8)
    b"\x00\x00\x00\x18\x18\x00\x00\x00",  # Character 249 (0xF9)
    b"\x00\x00\x00\x00\x18\x00\x00\x00",  # Character 250 (0xFA)
    b"\x0F\x0C\x0C\x0C\xEC\x6C\x3C\x1C",  # Character 251 (0xFB)
    b"\x78\x6C\x6C\x6C\x6C\x00\x00\x00",  # Character 252 (0xFC)
    b"\x70\x18\x30\x60\x78\x00\x00\x00",  # Character 253 (0xFD)
    b"\x00\x00\x3C\x3C\x3C\x3C\x00\x00",  # Character 254 (0xFE)
    b"\x00\x00\x00\x00\x00\x00\x00\x00",  # Character 255 (0xFF)
]


@dataclass(frozen=True)
class Equivalence:
    # canonical representatives for the equivalence class of each termel
    reps: np.ndarray
    skip_val: int | None = None
    # for each equivalence, all the termels belonging to it
    compatibility: dict[int, set[int]] = field(default_factory=dict)

    def __post_init__(self):
        assert len(self.reps) == 1 << 16

        # all this jazz with tm_equivs and equivs is just checks!
        # tm_equivs maps each termel to an equivalence class
        tm_equivs: list[set[int]] = [None] * (1 << 16)
        for tm in range(len(tm_equivs)):
            # the normalized termel should be the unique representative
            rep = self.reps[tm]
            assert rep == self.reps[rep]
            # make a new container for the equivalence if there isn't one
            if tm_equivs[rep] == None:
                tm_equivs[rep] = set()
            # if this isn't the equivalence class representative already, map the
            # equivalence class from this termel
            if rep != tm:
                tm_equivs[tm] = tm_equivs[rep]
            # add this termel to the equivalence class
            tm_equivs[tm].add(tm)
        # equivs is just all the equivalence classes
        equivs: set[tuple[int]] = set(map(tuple, tm_equivs))
        for e in equivs:
            assert e is not None
            assert len(e) > 0
            rep = self.reps[next(iter(e))]
            assert rep in e
            # make double sure everything normalizes to the same representative
            for tm in e:
                assert self.reps[tm] == rep

        for tm, rep in enumerate(self.reps):
            # a poorly selected skip_val could leave some class empty
            if tm != self.skip_val:
                compat_termels = self.compatibility.setdefault(rep, set())
                compat_termels.add(tm)

    def normalize(self, tm: int) -> int:
        assert tm is not None
        return self.reps[tm]

    def _compatible(self, tms: list[int], bitmask: int) -> list[int] | None:
        if len(tms) == 0:
            return []
        tm_reps = self.reps[tms]
        if len(tms) == 1:
            return tm_reps
        compatible = set((ctm & bitmask for ctm in self.compatibility[tm_reps[0]]))
        for rep in tm_reps[1:]:
            compatible.intersection_update(
                (ctm & bitmask for ctm in self.compatibility[rep])
            )
            if len(compatible) == 0:
                return None
        # If there are multiple, prefer to encode as lower-value termels,
        # because this (a) makes a nice tie-breaker producing a very predictable
        # encoding and (b) incidentally avoids unnecessarily making things blink
        # (..guess the ACTUAL reason for this change)
        chosen = min(compatible) & bitmask
        if bitmask == 0xFFFF:
            # if we're checking literal termels, they will all be the same
            # we can get the representative for the chosen termel -- this will
            # avoid choosing the skip termel
            chosen = self.reps[chosen]
            assert chosen != self.skip_val
            return [chosen] * len(tms)
        res = []
        for rep in tm_reps:
            for tm in self.compatibility[rep]:
                if tm & bitmask == chosen:
                    assert tm != self.skip_val
                    res.append(tm)
                    break
            else:
                assert not "compatible chars but no actual termels for them?"
                return None
        assert len(res) == len(tms)
        assert all((r == tm for r, tm in zip(self.reps[res], tm_reps)))
        return res

    def compatible_chars(self, tms: list[int]) -> list[int] | None:
        return self._compatible(tms, 0x00FF)

    def compatible_attrs(self, tms: list[int]) -> list[int] | None:
        return self._compatible(tms, 0xFF00)

    def compatible_termels(self, tms: list[int]) -> list[int] | None:
        return self._compatible(tms, 0xFFFF)


def make_font_c8(chars: list[bytes]) -> np.ndarray:
    assert len(chars) == 256
    return np.array(
        [
            np.unpackbits(
                np.frombuffer(b, dtype=np.uint8).reshape((len(b), 1)),
                axis=1,
                bitorder="big",
            )
            for b in chars
        ],
        dtype=np.bool_,
    )


def make_font_c9(chars: list[bytes]) -> np.ndarray:
    f8 = make_font_c8(chars)
    f8_right_edge = f8[:, :, -1:]
    zero_edge = np.zeros((f8.shape[1], 1), dtype=np.bool_)
    f9_right_edge = np.array(
        [
            e if i >= 0xC0 and i < 0xF0 else zero_edge
            for i, e in enumerate(f8_right_edge)
        ]
    )
    return np.concatenate([f8, f9_right_edge], axis=2, dtype=np.bool_)


def make_font_equivalence(
    font: np.ndarray,
    ice_color: bool,
    allow_fg_remap: bool,
    skip_val: int | None = 0x0000,
    alt_val: int | None = 0x0020,
) -> Equivalence:
    equivs_by_sig: dict[tuple, set[int]] = {}
    equivs: list[set[int]] = []
    for tm in range((1 << 16)):
        blink = 0 if ice_color else (tm & 0x8000) >> 15
        c = tm & 0xFF
        fg = (tm & 0x0F00) >> 8
        bg = (tm & 0xF000) >> 12 if ice_color else (tm & 0x7000) >> 12
        assert tm == (c | (fg << 8) | (bg << 12) | (blink << 15))
        eff_fg = (fg | (bg << 4) | (blink << 7)) if blink else fg
        render = np.array((font[c] * eff_fg) + ((~font[c]) * bg), dtype=np.uint8)
        # print(f"0x{tm:04X} [{c:02X}, {fg:2}/{eff_fg:3}, {bg:2}, {blink}] =\n{str(render)}\n")
        render_sig = tuple(render.reshape((font.shape[1] * font.shape[2],)))
        sig = render_sig if allow_fg_remap else (eff_fg,) + render_sig
        eq_set = equivs_by_sig.setdefault(sig, set())
        eq_set.add(tm)
        equivs.append(eq_set)

    rep_table = list([None] * (1 << 16))
    for eq_set in equivs_by_sig.values():
        # Usually there's only one thing in the eq_set, so we have to return that
        if len(eq_set) == 1:
            rep = next(iter(eq_set))
        # If the alt_val is exactly in the set, that's best
        if alt_val in eq_set:
            rep = alt_val
        else:
            eqs = sorted(eq_set)
            # Find something close to alt_val that's not skip_val
            for tm in eqs:
                if (tm != skip_val) and ((tm & 0xFF) == (alt_val & 0xFF)):
                    rep = tm
                    break
            else:
                # Okay just find something that's not close to skip_val
                for tm in eqs:
                    if tm & 0xFF != skip_val & 0xFF:
                        rep = tm
                        break
                else:
                    # Fine just find ANYTHING that's not exactly skip_val
                    for tm in eqs:
                        if tm != skip_val:
                            rep = tm
                            break
                    else:
                        assert (
                            not "skip_val doesn't have an alternative we can normalize to"
                        )
        assert rep != skip_val
        for tm in eq_set:
            rep_table[tm] = rep
    assert all([rep is not None for rep in rep_table])

    # print("classes:", len(equivs_by_sig))

    return Equivalence(np.array(rep_table), skip_val)


cache_font_c8 = None
cache_font_c9 = None
# equiv_precise = Equivalence(np.array(range(1 << 16)))
# equiv_precise_skip = Equivalence(np.array([32] + list(range(1 << 16))[1:]), 0x0000)
cache_equiv_w9_noice_allowfg_skip00 = None


def default_font_c8() -> np.ndarray:
    global cache_font_c8
    if cache_font_c8 is None:
        cache_font_c8 = make_font_c8(chars_r8)
    return cache_font_c8


def default_font_c9() -> np.ndarray:
    global cache_font_c9
    if cache_font_c9 is None:
        cache_font_c9 = make_font_c9(chars_r8)
    return cache_font_c9


def default_equivalence() -> Equivalence:
    global cache_equiv_w9_noice_allowfg_skip00
    if cache_equiv_w9_noice_allowfg_skip00 is None:
        cache_equiv_w9_noice_allowfg_skip00 = make_font_equivalence(
            default_font_c9(),
            ice_color=False,
            allow_fg_remap=True,
            skip_val=0x0000,
            alt_val=0x0020,
        )
    return cache_equiv_w9_noice_allowfg_skip00


# f8x14 = make_font_c8(chars_r14)
# f8x8 = make_font_c8(chars_r8)
# f8x8_thin = make_font_c8(chars_r8_thin)
# f9x14 = make_font_c9(chars_r14)
# f9x8 = make_font_c9(chars_r8)
# f9x8_thin = make_font_c9(chars_r8_thin)

# for f, fn in [(f8x8, "f8x8"), (f9x8, "f9x8")]:
#     for ic in [False, True]:
#         for fr in [False, True]:
#             equiv = make_font_equivalence(f, ic, fr, 0x0000)
#             print(f"Equivalence({fn}, {ic}, {fr}, 0x0000) =")
#             for tm, eq in enumerate(equiv.reps):
#                 if tm != eq:
#                     print(f"    [0x{tm:04X}] = 0x{eq:04X}")
#             print()
