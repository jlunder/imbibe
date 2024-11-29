from dataclasses import dataclass
import logging
import numpy as np
import struct
from typing import Iterator

from ansi import *
from sauce import *

import __main__

if "__appname__" in dict(__main__):
    logging_name = dict(__main__)["__appname__"] + ".rle"
else:
    logging_name = "rle"

logger = logging.getLogger(logging_name)


class RleEncoder:
    runs: list[tuple]
    copy_buf: list[int]
    last: int
    rep: int

    def __init__(self):
        self.runs = []
        self.copy_buf = []
        self.last = None
        self.rep = 0

    def commit_copy(self):
        if len(self.copy_buf) > 0:
            self.runs.append((0, self.copy_buf))
            self.copy_buf = []

    def commit_fill(self):
        self.commit_copy()
        if self.rep > 0:
            self.runs.append((1, self.last, self.rep))
            self.rep = 0

    MAX_RUN = 128

    def encode(self, src):

        for cur in src:
            if cur != self.last:
                if self.rep >= 3:
                    self.commit_copy()
                    self.commit_fill()
                else:
                    while self.rep > 0:
                        self.copy_buf.append(self.last)
                        self.rep -= 1
                        if len(self.copy_buf) >= RleEncoder.MAX_RUN:
                            self.commit_copy()
                self.last = cur
                self.rep = 1
            else:
                self.rep += 1
            if len(self.copy_buf) >= RleEncoder.MAX_RUN:
                self.commit_copy()
            if self.rep >= RleEncoder.MAX_RUN:
                self.commit_copy()
                self.commit_fill()
        self.commit_copy()
        self.commit_fill()

    def data(self):
        def run_bytes(run):
            if run[0] == 0:
                return (
                    struct.pack("<B", (len(run[1]) & 0x7F) | 0x00)
                    + np.array(run[1], dtype=np.uint16).tobytes()
                )
            elif run[0] == 1:
                return struct.pack("<BH", (run[2] & 0x7F) | 0x80, run[1])

        return b"".join(map(run_bytes, self.runs))

    @staticmethod
    def decode(rle_data: bytes) -> bytes:
        decoded = b""
        p = 0
        while p < len(rle_data):
            l = rle_data[p]
            run_len = ((l - 1) & 0x7F) + 1
            p += 1
            if l & 0x80:
                assert len(rle_data) >= p + 2
                decoded += rle_data[p : p + 2] * run_len
                p += 2
            else:
                assert len(rle_data) >= p + run_len * 2
                decoded += rle_data[p : p + run_len * 2]
                p += l * 2
        return decoded


def encode_rle(img: Image) -> tuple[int, bytes]:
    lines = []
    pos = 2 * img.data.shape[0]
    rle_bytes = bytes()
    for i in range(img.data.shape[0]):
        lines.append(pos)
        encoder = RleEncoder()
        encoder.encode(img.data[i])
        data = encoder.data()
        assert RleEncoder.decode(data) == img.data[i].tobytes()
        rle_bytes += data
        pos += len(data)
    encoded = np.array(lines, np.uint16).tobytes() + rle_bytes
    assert len(lines) == img.data.shape[0]
    assert pos < 30000
    return (TbmFormat.RLE, encoded)


def encode_mask_rle(img: Image) -> tuple[int, bytes]:
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
            while (j < img.data.shape[1]) and (skip < max_skip) and not img.mask[i][j]:
                skip += 1
                j += 1
            while (j < img.data.shape[1]) and (len(span) < max_skip) and img.mask[i][j]:
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
    encoded = np.array(lines, np.uint16).tobytes() + rle_bytes
    assert len(lines) == img.data.shape[0]
    assert pos < 30000
    return (TbmFormat.MASK_RLE, encoded)


@dataclass(frozen=True, order=True)
class EncoderState:
    encoded: bytes


@dataclass(frozen=True)
class SearchEncoder:
    def fresh_state(self) -> EncoderState:
        raise NotImplementedError()

    def equivalent(self, state_x: EncoderState, state_y: EncoderState) -> bool:
        raise NotImplementedError()

    def generate_candidates(
        self, state: EncoderState, data_el: int, mask_el: bool
    ) -> Iterator[EncoderState]:
        raise NotImplementedError()

    def encode(self, data: np.ndarray, mask: np.ndarray, beam_width: int = 10) -> bytes:
        assert len(mask.shape) == 1
        assert mask.shape[0] > 0
        assert mask.shape == data.shape

        best: list[EncoderState] = [self.fresh_state()]
        for i in range(len(mask)):
            new_best: list[EncoderState] = []
            for s in best:
                for c in self.generate_candidates(s, data[i], mask[i]):
                    for i, d in enumerate(new_best):
                        if self.equivalent(c, d):
                            # c and d are equivalent in the final portion of
                            # their encoding (which is all we manipulate), so
                            # we only want to keep the shorter of them
                            if len(c.encoded) < len(d.encoded):
                                # c is shorter, remove d and substitute
                                new_best = new_best[:i] + new_best[i + 1 :]
                                new_best.append(c)
                            # otherwise do nothing and c will be dropped
                            break
                    else:
                        # if we get here, no equivalence was found, so add c
                        new_best.append(c)
            assert len(new_best) > 0
            new_best.sort(key=lambda s: len(s.encoded))
            best = new_best[:beam_width]
        return best[0].encoded


@dataclass(frozen=True, order=True)
class XbinEncoderState(EncoderState):
    last_run_enc_pos: int
    last_run_data: list[int] | None  # either list of termels, or None for a skip

    def replace_last_run(
        self,
        new_encoded: bytes,
        new_last_run_enc_pos: int | None = None,
        new_last_run_data: list[int] | None = None,
    ) -> "XbinEncoderState":
        # new_last_run_data should only be None if we're encoding a skip
        assert (new_last_run_data is None) == (
            (new_encoded == b"")
            or (
                (len(new_encoded) == 3)
                and (new_encoded[0] & 0b11000000 == 0b11000000)
                and (new_encoded[1] == 0)
                and (new_encoded[2] == 0)
            )
        )

        return XbinEncoderState(
            encoded=self.encoded[: self.last_run_enc_pos] + new_encoded,
            last_run_enc_pos=(
                new_last_run_enc_pos
                if new_last_run_enc_pos is not None
                else self.last_run_enc_pos
            ),
            last_run_data=new_last_run_data,
        )

    def append_last_run(
        self,
        new_encoded: bytes,
        new_last_run_enc_pos: int | None = None,
        new_last_run_data: list[int] | None = None,
    ) -> "XbinEncoderState":
        # new_last_run_data should only be None if we're encoding a skip
        assert (new_last_run_data is None) == (
            (new_encoded == b"")
            or (
                (len(new_encoded) == 3)
                and (new_encoded[0] & 0b11000000 == 0b11000000)
                and (new_encoded[1] == 0)
                and (new_encoded[2] == 0)
            )
        )

        return XbinEncoderState(
            encoded=self.encoded + new_encoded,
            last_run_enc_pos=(
                new_last_run_enc_pos
                if new_last_run_enc_pos is not None
                else len(self.encoded)
            ),
            last_run_data=new_last_run_data,
        )

    @property
    def last_run_len(self) -> int:
        assert self.last_run_enc_pos >= 0
        if len(self.encoded) == 0:
            return 0
        assert (len(self.encoded) - self.last_run_enc_pos) >= 3
        return (self.encoded[self.last_run_enc_pos] & 0b00111111) + 1

    @property
    def last_run_is_skip(self) -> bool:
        assert self.last_run_enc_pos >= 0
        if len(self.encoded) - self.last_run_enc_pos != 3:
            return False
        e = self.encoded[self.last_run_enc_pos :]
        assert len(e) == 3
        return ((e[0] & 0b11000000) == 0b11000000) and (e[1] == 0) and (e[2] == 0)

    @property
    def last_run_is_uncompressed(self) -> bool:
        assert self.last_run_enc_pos >= 0
        if len(self.encoded) - self.last_run_enc_pos < 3:
            return False
        return (self.encoded[self.last_run_enc_pos] & 0b11000000) == 0b00000000

    @property
    def last_run_is_char_repeat(self) -> bool:
        assert self.last_run_enc_pos >= 0
        if len(self.encoded) - self.last_run_enc_pos < 3:
            return False
        return (self.encoded[self.last_run_enc_pos] & 0b11000000) == 0b01000000

    @property
    def last_run_is_attribute_repeat(self) -> bool:
        assert self.last_run_enc_pos >= 0
        if len(self.encoded) - self.last_run_enc_pos < 3:
            return False
        return (self.encoded[self.last_run_enc_pos] & 0b11000000) == 0b10000000

    @property
    def last_run_is_termel_repeat(self) -> bool:
        assert self.last_run_enc_pos >= 0
        if len(self.encoded) - self.last_run_enc_pos != 3:
            return False
        e = self.encoded[self.last_run_enc_pos :]
        assert len(e) == 3
        return ((e[0] & 0b11000000) == 0b11000000) and ((e[1] != 0) or (e[2] != 0))


@dataclass(frozen=True)
class XbinSearchEncoder(SearchEncoder):
    def fresh_state(self) -> XbinEncoderState:
        return XbinEncoderState(b"")

    def equivalent(self, state_x: XbinEncoderState, state_y: XbinEncoderState) -> bool:
        return (
            # both have the same encoding for the last run, viz,
            # the encoding for the last run starts at the same spot and
            (state_x.last_run_enc_pos == state_y.last_run_enc_pos)
            and (
                # it is zero length or
                (state_x.last_run_enc_pos == len(state_x.encoded))
                or (
                    # the first byte of the encoding is the same, which means
                    # it's using the same method
                    state_x.encoded[state_x.last_run_enc_pos]
                    == state_y.encoded[state_y.last_run_enc_pos]
                )
            )
        )

    def generate_candidates(
        self, state: XbinEncoderState, data_el: int, mask_el: bool
    ) -> Iterator[EncoderState]:
        if not mask_el:
            yield XbinSearchEncoder.generate_skip(state)
            return
        if data_el == 0x0000:
            # Sus: canonicalizer should already have avoided this termel
            # We should avoid it because that's our transparency value
            data_el = 0x0020
        yield XbinSearchEncoder.generate_uncompressed(state, data_el)
        if XbinSearchEncoder.can_generate_char_repeat(state, data_el):
            yield XbinSearchEncoder.generate_char_repeat(state, data_el)
        if XbinSearchEncoder.can_generate_attribute_repeat(state, data_el):
            yield XbinSearchEncoder.generate_attribute_repeat(state, data_el)
        if XbinSearchEncoder.can_generate_termel_repeat(state, data_el):
            yield XbinSearchEncoder.generate_termel_repeat(state, data_el)

    @staticmethod
    def can_generate_char_repeat(state: XbinEncoderState, data_el: int) -> bool:
        return (
            (not state.last_run_is_skip)
            and (len(state.last_run_data) >= 1)
            and ((state.last_run_data[-1] & 0x00FF) == (data_el & 0x00FF))
            # Slightly tricky: we don't want to generate 1-item repeats, so we
            # have to disallow creating a repeat in the case where we would try
            # to extend a repeat past its max length. In this situation, we
            # will still generate a (same-size) 1-item uncompressed block. So
            # far so equivalent, but what if the repeat continues after that?
            # In that case, one of the options generated later will be a
            # transmutation of the uncompressed block to a 2-item repeat, so in
            # the end we don't lose out...
            # Note that it's fine to be at the max run len if we're converting
            # from any other run type, because in that case we at least have a
            # potentially interesting and distinct case to track. If it's a
            # char repeat, there's no possible advantage to stealing between
            # the same run types, we really want to be greedy. So we shouldn't
            # steal unless we can put the termel back in the very next round --
            # and we forget all but the last run, so we can't, practically.
            and ((state.last_run_len < 64) or not state.last_run_is_char_repeat)
        )

    @staticmethod
    def generate_char_repeat(state: XbinEncoderState, data_el: int) -> XbinEncoderState:
        assert not state.last_run_is_skip and (len(state.last_run_data) >= 1)
        if state.last_run_is_char_repeat:
            # can_generate should ensure this -- see the note
            assert state.last_run_len < 64
            new_last_run_data = state.last_run_data + [data_el]
            new_encoded = XbinSearchEncoder.encode_char_repeat(new_last_run_data)
            return state.replace_last_run(
                new_encoded=new_encoded, new_last_run_data=new_last_run_data
            )
        else:
            steal_state, stolen_el = XbinSearchEncoder.generate_steal_from(state)
            return steal_state.append_last_run(
                XbinSearchEncoder.encode_char_repeat([stolen_el, data_el])
            )

    @staticmethod
    def can_generate_attribute_repeat(state: XbinEncoderState, data_el: int) -> bool:
        return (
            (not state.last_run_is_skip)
            and (len(state.last_run_data) >= 1)
            and ((state.last_run_data[-1] & 0xFF00) == (data_el & 0xFF00))
            # see note in can_generate_char_repeat
            and ((state.last_run_len < 64) or not state.last_run_is_attribute_repeat)
        )

    @staticmethod
    def generate_attribute_repeat(
        state: XbinEncoderState, data_el: int
    ) -> XbinEncoderState:
        assert not state.last_run_is_skip and (len(state.last_run_data) >= 1)
        if state.last_run_is_attribute_repeat:
            # can_generate should ensure this -- see the note
            assert state.last_run_len < 64
            new_last_run_data = state.last_run_data + [data_el]
            new_encoded = XbinSearchEncoder.encode_attribute_repeat(new_last_run_data)
            return state.replace_last_run(
                new_encoded=new_encoded, new_last_run_data=new_last_run_data
            )
        else:
            steal_state, stolen_el = XbinSearchEncoder.generate_steal_from(state)
            return steal_state.append_last_run(
                XbinSearchEncoder.encode_attribute_repeat([stolen_el, data_el])
            )

    @staticmethod
    def can_generate_termel_repeat(state: XbinEncoderState, data_el: int) -> bool:
        return (
            (not state.last_run_is_skip)
            and (len(state.last_run_data) >= 1)
            and (state.last_run_data[-1] == data_el)
            # see note in can_generate_char_repeat
            and ((state.last_run_len < 64) or not state.last_run_is_termel_repeat)
        )

    @staticmethod
    def generate_termel_repeat(
        state: XbinEncoderState, data_el: int
    ) -> XbinEncoderState:
        assert not state.last_run_is_skip and (len(state.last_run_data) >= 1)
        if state.last_run_is_termel_repeat:
            # can_generate should ensure this -- see the note
            assert state.last_run_len < 64
            new_last_run_data = state.last_run_data + [data_el]
            new_encoded = XbinSearchEncoder.encode_termel_repeat(new_last_run_data)
            return state.replace_last_run(
                new_encoded=new_encoded, new_last_run_data=new_last_run_data
            )
        else:
            steal_state, stolen_el = XbinSearchEncoder.generate_steal_from(state)
            return steal_state.append_last_run(
                XbinSearchEncoder.encode_termel_repeat([stolen_el, data_el])
            )

    @staticmethod
    def generate_uncompressed(
        state: XbinEncoderState, data_el: int
    ) -> XbinEncoderState:
        if state.last_run_is_uncompressed and (state.last_run_len < 64):
            new_last_run_data = state.last_run_data + [data_el]
            new_encoded = XbinSearchEncoder.encode_uncompressed(new_last_run_data)
            return state.replace_last_run(
                new_encoded=new_encoded, new_last_run_data=new_last_run_data
            )
        else:
            return state.append_last_run(
                XbinSearchEncoder.encode_uncompressed([data_el])
            )

    @staticmethod
    def generate_skip(state: XbinEncoderState) -> XbinEncoderState:
        if state.last_run_is_skip and (state.last_run_len < 64):
            new_encoded = XbinSearchEncoder.encode_skip(state.last_run_len + 1)
            return state.replace_last_run(new_encoded=new_encoded)
        else:
            return state.append_last_run(XbinSearchEncoder.encode_skip(1))

    @staticmethod
    def generate_steal_from(state: XbinEncoderState) -> tuple[XbinEncoderState, int]:
        assert len(state.last_run_data) >= 1
        stolen = state.last_run_data[-1]
        if len(state.last_run_data) == 1:
            return (
                XbinEncoderState(
                    state.encoded[: state.last_run_enc_pos], state.last_run_enc_pos, []
                ),
                stolen,
            )

        if state.last_run_is_uncompressed or (len(state.last_run_data) == 2):
            steal_reencoded = XbinSearchEncoder.encode_uncompressed(
                state.last_run_data[-1]
            )
        elif state.last_run_is_attribute_repeat:
            steal_reencoded = XbinSearchEncoder.encode_attribute_repeat(
                state.last_run_data[-1]
            )
        elif state.last_run_is_termel_repeat:
            steal_reencoded = XbinSearchEncoder.encode_termel_repeat(
                state.last_run_data[-1]
            )

        return (
            state.replace_last_run(
                new_encoded=steal_reencoded, new_last_run_data=state.last_run_data[:-1]
            ),
            stolen,
        )

    @staticmethod
    def encode_char_repeat(termels: list[int]) -> bytes:
        assert len(termels) >= 2
        assert len(termels) <= 64
        data = [0b01000000 | (len(termels) - 1), (termels[0] & 0x00FF) >> 0]
        for t in termels:
            assert ((t & 0x00FF) >> 0) == data[1]
            data.append((t & 0xFF00) >> 8)
        return bytes(data)

    @staticmethod
    def encode_attribute_repeat(termels: list[int]) -> bytes:
        assert len(termels) >= 2
        assert len(termels) <= 64
        data = [0b10000000 | (len(termels) - 1), (termels[0] & 0xFF00) >> 8]
        for t in termels:
            assert ((t & 0xFF00) >> 8) == data[1]
            data.append((t & 0x00FF) >> 0)
        return bytes(data)

    @staticmethod
    def encode_termel_repeat(termels: list[int]) -> bytes:
        assert len(termels) >= 2
        assert len(termels) <= 64
        data = [
            0b11000000 | (len(termels) - 1),
            (termels[0] & 0x00FF) >> 0,
            (termels[0] & 0xFF00) >> 8,
        ]
        for t in termels[1:]:
            assert ((t & 0x00FF) >> 0) == data[1]
            assert ((t & 0xFF00) >> 8) == data[2]
        return bytes(data)

    @staticmethod
    def encode_uncompressed(termels: list[int]) -> bytes:
        assert len(termels) >= 1
        assert len(termels) <= 64
        # encode character 0 with black fg and bg
        data = [0b00000000 | (len(termels) - 1)]
        for t in termels:
            data.append((t & 0x00FF) >> 0)
            data.append((t & 0xFF00) >> 8)
        return bytes(data)

    @staticmethod
    def encode_skip(skip_len: int) -> bytes:
        assert skip_len >= 1
        assert skip_len <= 64
        # encode character 0 with black fg and bg
        return bytes([0b11000000 | (skip_len - 1), 0, 0])


def encode_mask_xbin_row(data: np.ndarray, mask: np.ndarray) -> bytes:
    encoder = XbinSearchEncoder()
    rle_bytes = encoder.encode(data, mask)
    return rle_bytes
