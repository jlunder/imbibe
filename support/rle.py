from dataclasses import dataclass, field
from enum import IntEnum, auto
import logging
from itertools import chain
import numpy as np
import struct
from typing import Iterator

from ansi import *
from sauce import *

import __main__

if "__appname__" in dir(__main__):
    logging_name = __main__.__appname__ + ".rle"
else:
    logging_name = "rle"

logger = logging.getLogger(logging_name)


class RunType(IntEnum):
    EMPTY = auto()
    SKIP = auto()
    UNCOMPRESSED = auto()
    CHAR_REPEAT = auto()
    ATTRIBUTE_REPEAT = auto()
    TERMEL_REPEAT = auto()


@dataclass(frozen=True)
class Equivalence:
    # canonical representatives for the equivalence class of each termel
    reps: np.ndarray
    skip_val: int | None = None
    # for each equivalence, all the termels belonging to it
    compatibility: dict[int, set[int]] = field(default_factory=dict)

    def __post_init__(self):
        assert len(self.reps) == 1 << 16
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
        # if there are multiple, any is fine
        chosen = next(iter(compatible)) & bitmask
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


@dataclass(frozen=True)
class RunEncoding:
    equiv: Equivalence
    window: int

    def run_type(self, encoded: bytes) -> RunType:
        raise NotImplementedError()

    def decoded_len(self, encoded: bytes) -> int:
        raise NotImplementedError()

    def encoded_len(self, encoded: bytes) -> int:
        return len(encoded)

    def valid(self, encoded: bytes) -> bool:
        raise NotImplementedError()

    def try_encode(self, termels: list[int | None]) -> bytes | None:
        raise NotImplementedError()

    def decode(self, encoded: bytes) -> list[int]:
        raise NotImplementedError()

    def identify(self, decoded: list[int | None]) -> tuple[int | None] | int:
        if all((tm == None for tm in decoded[-self.window :])):
            return max(self.window, len(decoded))
        return tuple(
            [
                None if tm is None else self.equiv.normalize(tm)
                for tm in decoded[-self.window :]
            ]
        )

    def decode_all(self, encoded: bytes) -> list[int]:
        remain = encoded
        decoded = []
        while len(remain) > 0:
            run_len = self.encoded_len(remain)
            run, remain = remain[:run_len], remain[run_len:]
            assert self.valid(run)
            decoded += self.decode(run)
        return decoded

    def equivalent(self, decoded: list[int | None], expected: list[int | None]) -> bool:
        if len(decoded) != len(expected):
            return False
        return all(
            (
                (y == None) or (self.equiv.normalize(x) == self.equiv.normalize(y))
                for x, y in zip(decoded, expected)
            )
        )

    def termel_from_bytes(self, b0: int, b1: int) -> int | None:
        tm = b0 | (b1 << 8)
        if tm == self.skip_tm:
            return None
        return tm

    def bytes_from_termel(self, tm: int | None) -> bytes:
        if tm is None:
            return bytes((self.skip_tm & 0xFF, (self.skip_tm >> 8) & 0xFF))
        return bytes((tm & 0xFF, (tm >> 8) & 0xFF))


@dataclass(frozen=True, order=True)
class EncoderState:
    encoding: RunEncoding
    fixed: bytes = b""
    last_run: bytes = b""
    last_decoded: list[int | None] = field(default_factory=list)
    identity: tuple[int | None] | int = ()  # int for skip
    cost: float = 0

    def __post_init__(self):
        # encoding must be a bytes
        assert type(self.fixed) == bytes
        assert type(self.last_run) == bytes
        # there must be a non-empty last run if fixed is not empty
        assert (len(self.fixed) == 0) or (len(self.last_run) > 0)
        # any last run must be valid
        assert (len(self.last_run) == 0) or self.encoding.valid(self.last_run)
        # there must be last_run_data iff there is a non-skip last run
        assert (self.encoding.run_type(self.last_run) == RunType.SKIP) == (
            (len(self.last_decoded) > 0)
            and all((tm is None for tm in self.last_decoded))
        )
        # last run must actually be the last run
        assert (len(self.fixed) + len(self.last_run) == 0) or (
            self.encoding.encoded_len(self.last_run) == len(self.last_run)
        )
        # any last run must decode equivalent to last_decoded
        assert self.encoding.equivalent(
            self.encoding.decode(self.last_run), self.last_decoded
        )

    @property
    def encoded(self) -> bytes:
        return self.fixed + self.last_run

    def decode(self) -> list[int | None]:
        return self.encoding.decode_all(self.encoded)


@dataclass(frozen=True)
class SearchEncoder:
    encoding: RunEncoding

    def fresh_state(self) -> EncoderState:
        return EncoderState(self.encoding)

    def generate_candidates(
        self, state: EncoderState, data_el: int, mask_el: bool
    ) -> Iterator[EncoderState]:
        tm = data_el if mask_el else None
        any_gen = False
        # try _not_ merging the candidates
        lone_dec = [tm]
        lone_enc = self.encoding.try_encode(lone_dec)
        if lone_enc is not None:
            any_gen = True
            ident = self.encoding.identify(lone_dec)
            merged_fixed = state.fixed + state.last_run
            cost = len(merged_fixed) + len(lone_enc)
            yield EncoderState(
                self.encoding, merged_fixed, lone_enc, lone_dec, ident, cost
            )
        # try merging the candidates
        merged_dec = state.last_decoded + lone_dec
        merged_enc = self.encoding.try_encode(merged_dec)
        if merged_enc is not None:
            any_gen = True
            ident = self.encoding.identify(merged_dec)
            cost = len(state.fixed) + len(merged_enc)
            yield EncoderState(
                self.encoding, state.fixed, merged_enc, merged_dec, ident, cost
            )
        assert any_gen

    def encode_row(self, data: np.ndarray, mask: np.ndarray) -> bytes:
        assert len(mask.shape) == 1
        assert mask.shape[0] > 0
        assert mask.shape == data.shape

        best: dict[any, EncoderState] = {None: self.fresh_state()}
        for i in range(len(mask)):
            new_best: dict[any, EncoderState] = {}
            for s in best.values():
                for c in self.generate_candidates(s, data[i], mask[i]):
                    assert self.encoding.equivalent(
                        [tm if m else None for tm, m in zip(c.decode(), mask)],
                        [tm if m else None for tm, m in zip(data[: i + 1], mask)],
                    )
                    if c.identity in new_best:
                        if c.cost >= new_best[c.identity].cost:
                            continue
                    new_best[c.identity] = c
            assert len(new_best) > 0
            best = new_best

        best_encoding: EncoderState | None = None
        for v in new_best.values():
            if (best_encoding is None) or (v.cost < best_encoding.cost):
                best_encoding = v

        return best_encoding.fixed + best_encoding.last_run

    def encode_file(self, data: np.ndarray, mask: np.ndarray) -> bytes:
        lines = []
        pos = 2 * data.shape[0]
        encoded_data = bytes()
        for i in range(data.shape[0]):
            encoded_row = self.encode_row(data[i], mask[i])
            if len(data) == 0:
                lines.append(0)
            else:
                lines.append(pos)
            encoded_data = encoded_data + encoded_row
            pos += len(encoded_row)
        encoded = np.array(lines, np.uint16).tobytes() + encoded_data
        assert len(lines) == data.shape[0]
        if pos >= 30000:
            raise Exception("Encoded data too large")
        return encoded


@dataclass(frozen=True)
class RleEncoding(RunEncoding):
    window: int = 1
    skip_tm: int | None = 0x0000

    def run_type(self, encoded: bytes) -> RunType:
        assert encoded is not None
        if len(encoded) == 0:
            return RunType.EMPTY
        elif encoded[0] & 0b10000000 == 0b00000000:
            return RunType.UNCOMPRESSED
        else:
            assert encoded[0] & 0b10000000 == 0b10000000
            assert len(encoded) >= 3
            if self.termel_from_bytes(encoded[1], encoded[2]) is None:
                return RunType.SKIP
            return RunType.TERMEL_REPEAT

    def decoded_len(self, encoded: bytes) -> int:
        assert encoded is not None and len(encoded) > 0
        return (encoded[0] & 0b01111111) + 1

    def encoded_len(self, encoded: bytes) -> int:
        assert encoded is not None and len(encoded) > 0
        t = self.run_type(encoded)
        if t == RunType.UNCOMPRESSED:
            return 1 + self.decoded_len(encoded) * 2
        else:
            assert (t == RunType.SKIP) or (t == RunType.TERMEL_REPEAT)
            return 1 + 2

    def valid(self, encoded: bytes) -> bool:
        assert encoded is not None
        return (len(encoded) > 0) and (len(encoded) == self.encoded_len(encoded))

    def try_encode(self, termels: list[int | None]) -> bytes | None:
        if len(termels) - 1 >= 0b10000000:
            return None
        if any((tm is None for tm in termels)):
            assert self.skip_tm is not None
            if not all((tm is None for tm in termels)):
                return None
            return bytes((0b10000000 | (len(termels) - 1),)) + self.bytes_from_termel(
                None
            )
        compat = self.equiv.compatible_termels(termels)
        if compat is not None:
            return bytes((0b10000000 | (len(termels) - 1),)) + self.bytes_from_termel(
                compat[0]
            )
        # else...
        res = bytes((0b00000000 | (len(termels) - 1),))
        for tm in termels:
            res += self.bytes_from_termel(tm)
        return res

    def decode(self, encoded: bytes) -> list[int]:
        t = self.run_type(encoded)
        if t == RunType.EMPTY:
            return b""
        elif t == RunType.SKIP:
            return [None] * self.decoded_len(encoded)
        elif t == RunType.UNCOMPRESSED:
            return [
                self.termel_from_bytes(encoded[1 + i * 2 + 0], encoded[1 + i * 2 + 1])
                for i in range(self.decoded_len(encoded))
            ]
        else:
            assert t == RunType.TERMEL_REPEAT
            return [self.termel_from_bytes(encoded[1], encoded[2])] * self.decoded_len(
                encoded
            )


@dataclass(frozen=True)
class XbinEncoding(RunEncoding):
    window: int = 3
    skip_tm: int | None = 0x0000

    def run_type(self, encoded: bytes) -> RunType:
        assert encoded is not None
        if len(encoded) == 0:
            return RunType.EMPTY
        elif encoded[0] & 0b11000000 == 0b00000000:
            assert len(encoded) >= 1 + ((encoded[0] & 0b00111111) + 1) * 2
            return RunType.UNCOMPRESSED
        elif encoded[0] & 0b11000000 == 0b01000000:
            assert len(encoded) >= 1 + 1 + ((encoded[0] & 0b00111111) + 1)
            return RunType.CHAR_REPEAT
        elif encoded[0] & 0b11000000 == 0b10000000:
            assert len(encoded) >= 1 + 1 + ((encoded[0] & 0b00111111) + 1)
            return RunType.ATTRIBUTE_REPEAT
        else:
            assert encoded[0] & 0b11000000 == 0b11000000
            assert len(encoded) >= 3
            if self.termel_from_bytes(encoded[1], encoded[2]) is None:
                return RunType.SKIP
            return RunType.TERMEL_REPEAT

    def decoded_len(self, encoded: bytes) -> int:
        assert encoded is not None and len(encoded) > 0
        return (encoded[0] & 0b00111111) + 1

    def encoded_len(self, encoded: bytes) -> int:
        assert encoded is not None and len(encoded) > 0
        t = self.run_type(encoded)
        if t == RunType.UNCOMPRESSED:
            return 1 + self.decoded_len(encoded) * 2
        elif (t == RunType.CHAR_REPEAT) or (t == RunType.ATTRIBUTE_REPEAT):
            return 1 + 1 + self.decoded_len(encoded)
        else:
            assert (t == RunType.SKIP) or (t == RunType.TERMEL_REPEAT)
            return 1 + 2

    def valid(self, encoded: bytes) -> bool:
        assert encoded is not None
        return (len(encoded) > 0) and (len(encoded) == self.encoded_len(encoded))

    def try_encode(self, termels: list[int | None]) -> bytes | None:
        if len(termels) - 1 >= 0b01000000:
            return None
        if any((tm is None for tm in termels)):
            assert self.skip_tm is not None
            if not all((tm is None for tm in termels)):
                return None
            return bytes((0b11000000 | (len(termels) - 1),)) + self.bytes_from_termel(
                None
            )
        if len(termels) > 1:
            compat = self.equiv.compatible_termels(termels)
            if compat is not None:
                return bytes((0b11000000 | (len(termels) - 1),)) + self.bytes_from_termel(
                    compat[0]
                )
            compat = self.equiv.compatible_attrs(termels)
            if compat is not None:
                return bytes(
                    (0b10000000 | (len(termels) - 1), (compat[0] >> 8) & 0xFF)
                ) + bytes((tm & 0xFF for tm in compat))
            compat = self.equiv.compatible_chars(termels)
            if compat is not None:
                return bytes((0b01000000 | (len(termels) - 1), compat[0] & 0xFF)) + bytes(
                    ((tm >> 8) & 0xFF for tm in compat)
                )
        # else...
        res = bytes((0b00000000 | (len(termels) - 1),))
        for tm in termels:
            res += self.bytes_from_termel(tm)
        return res

    def decode(self, encoded: bytes) -> list[int]:
        t = self.run_type(encoded)
        if t == RunType.EMPTY:
            return []
        elif t == RunType.SKIP:
            return [None] * self.decoded_len(encoded)
        elif t == RunType.UNCOMPRESSED:
            return [
                self.termel_from_bytes(encoded[1 + i * 2 + 0], encoded[1 + i * 2 + 1])
                for i in range(self.decoded_len(encoded))
            ]
        elif t == RunType.CHAR_REPEAT:
            return [
                self.termel_from_bytes(encoded[1], attr)
                for attr in encoded[2 : self.decoded_len(encoded) + 2]
            ]
        elif t == RunType.ATTRIBUTE_REPEAT:
            return [
                self.termel_from_bytes(char, encoded[1])
                for char in encoded[2 : self.decoded_len(encoded) + 2]
            ]
        else:
            assert t == RunType.TERMEL_REPEAT
            return [self.termel_from_bytes(encoded[1], encoded[2])] * self.decoded_len(
                encoded
            )


def make_cp437_equivalence(font_w8: bool, ice_color: bool, skip: bool) -> list[int]:
    blank = set([0x00, 0x20, 0xFF])
    blank_c = 0x20
    inverse_blank = set([0xDB])
    inverse = {0xDE: 0xDD, 0xDF: 0xDC}
    # 8-pixel wide font has some additional equivalences
    if font_w8:
        inverse |= {0x08: 0x07, 0x0A: 0x09, 0xB2: 0xB0}

    def normalize_tm(termel: int) -> int:
        c = termel & 0xFF
        fg = (termel >> 8) & 0xF
        if ice_color:
            bg = (termel >> 12) & 0xF
            blink = 0
            can_invert = True
        else:
            bg = (termel >> 12) & 0x7
            blink = (termel >> 15) & 0x1
            can_invert = ((fg & 0x8) == 0) and ((bg & 0x8) == 0)
        if fg == bg:
            # character not visible over background
            c = blank_c
            fg = 0
            blink = 0
        elif c in blank:
            # foreground not used by this character
            c = blank_c
            fg = 0
            blink = 0
        elif not blink:
            if c in inverse:
                if can_invert:
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

    # all this jazz with tm_equivs and equivs is actually just checks!
    # norm_equivs maps each termel to an equivalence class
    tm_equivs: list[set[int]] = [None] * (1 << 16)
    for tm in range(len(tm_equivs)):
        # the normalized termel should be the unique representative
        rep = normalize_tm(tm)
        assert rep == normalize_tm(rep)
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
        rep = normalize_tm(next(iter(e)))
        assert rep in e
        # make double sure everything normalizes to the same representative
        for tm in e:
            assert normalize_tm(tm) == rep

    # this is the real business -- just map things straight up
    return Equivalence(
        np.array([normalize_tm(tm) for tm in range(1 << 16)]), 0x0000 if skip else None
    )


equiv_precise = Equivalence(np.array(range(1 << 16)))
equiv_precise_skip = Equivalence(np.array([32] + list(range(1 << 16))[1:]), 0x0000)
equiv_cp437_w9_skip = make_cp437_equivalence(font_w8=False, ice_color=False, skip=True)


def encode_rle(
    data: np.ndarray, mask: np.ndarray = None, equiv: Equivalence = None
) -> bytes:
    if mask is None:
        mask = np.ones(data.shape, dtype=np.bool_)
    if equiv is None:
        equiv = equiv_cp437_w9_skip
    encoder = SearchEncoder(RleEncoding(equiv=equiv))
    return encoder.encode_file(data, mask)


def encode_xbin(
    data: np.ndarray, mask: np.ndarray = None, equiv: Equivalence = None
) -> bytes:
    if mask is None:
        mask = np.ones(data.shape, dtype=np.bool_)
    if equiv is None:
        equiv = equiv_cp437_w9_skip
    encoder = SearchEncoder(XbinEncoding(equiv=equiv))
    return encoder.encode_file(data, mask)
