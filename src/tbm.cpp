#include "imbibe.h"

#include "tbm.h"

#include "bitmap.h"
#include "graphics.h"
#include "unpacker.h"

#define logf_tbm(...) logf_any("TBM: " __VA_ARGS__)

#if BUILD_DEBUG

namespace tbm_aux {

static const size_t s_tbm_area_max = 1u << 15;

bool validate(unpacker const &tbm_data);
bool validate_data_rle(tbm_header const __far &tbm_h, unpacker *tbm);
bool validate_data_xbin(tbm_header const __far &tbm_h, unpacker *tbm);

} // namespace tbm_aux

#endif

tbm::tbm(immutable const &n_raw, segsize_t raw_size) : m_raw(n_raw) {
  if (!m_raw) {
    assert(!valid());
    return;
  }
#if BUILD_DEBUG
  if (!tbm_aux::validate(unpacker(n_raw.data(), raw_size))) {
    m_raw = NULL;
    assert(!valid());
    return;
  }
#else
  (void)raw_size;
#endif
  assert(raw_size >= sizeof(iff_header));
  assert(
      (reinterpret_cast<iff_header const __far *>(m_raw.data()))->data_size <=
      raw_size - sizeof(iff_header));
}

#if BUILD_DEBUG

bool tbm_aux::validate(unpacker const &tbm_data) {
  unpacker tbm(tbm_data);
  uint32_t iff_data_size;
  if (!iff::try_expect_magic(&tbm, FOURCC("TBMa"), &iff_data_size)) {
    logf_tbm("invalid IFF header, unpacking %" PRpF "\n", tbm.peek_untyped());
    return false;
  }
  tbm = unpacker(tbm.peek_untyped(),
                 (segsize_t)min<uint32_t>(iff_data_size, tbm.remain()));

  tbm_header const __far &tbm_h = tbm.unpack<tbm_header>();
  uint16_t plane_size = (uint16_t)tbm_h.width * tbm_h.height;
  if ((tbm_h.width == 0) || (tbm_h.height == 0) ||
      (plane_size > s_tbm_area_max)) {
    logf_tbm("suspicious TBM dimensions %u x %u, unpacking %" PRpF "\n",
             tbm_h.width, tbm_h.height, tbm.base());
    return false;
  }

  if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_flat) {
    if (!tbm.fits_array<termel_t>(plane_size)) {
      logf_tbm("TBM data %u too small for %u x %u, unpacking %" PRpF "\n",
               tbm.remain(), tbm_h.width, tbm_h.height, tbm.base());
      return false;
    }
  } else if ((tbm_h.flags & tbm_flags::flags_format) ==
             tbm_flags::fmt_mask_xbin) {
    return validate_data_xbin(tbm_h, &tbm);
  }

  logf_tbm("TBM format %u not supported, unpacking %" PRpF "\n",
           tbm_h.flags & tbm_flags::flags_format, tbm.base());
  return false;
}

bool tbm_aux::validate_data_xbin(tbm_header const __far &tbm_h, unpacker *tbm) {
  if (!tbm->fits_array<uint16_t>(tbm_h.height)) {
    logf_tbm("TBM line index truncated, unpacking %" PRpF "\n", tbm->base());
    return false;
  }
  segsize_t lines_origin = tbm->pos();
  uint16_t const __far *lines = tbm->unpack_array<uint16_t>(tbm_h.height);
  for (uint16_t i = 0; i < tbm_h.height; ++i) {
    if (lines[i] >= (tbm->size() - lines_origin)) {
      logf_tbm("TBM line %u outside data (at %u), unpacking %" PRpF "\n", i,
               lines[i], tbm->base());
      return false;
    }
    tbm->seek_to(lines[i] + lines_origin);
    coord_t x = 0;
    while (x < (coord_t)tbm_h.width) {
      if (!tbm->fits<uint8_t>()) {
        logf_tbm("TBM line %u, col %u (at %u+%u) truncated, unpacking %" PRpF
                 "\n",
                 i, (unsigned)x, lines[i],
                 tbm->pos() - (lines[i] + lines_origin), tbm->base());
        return false;
      }
      uint8_t run_info = tbm->unpack<uint8_t>();
      uint8_t run_length = (run_info & 0x3F) + 1;
      if ((run_length > tbm_h.width) || (x > tbm_h.width - run_length)) {
        logf_tbm("TBM line %u, col %u (at %u+%u) leaves image, unpacking %" PRpF
                 "\n",
                 i, (unsigned)x, lines[i],
                 tbm->pos() - (lines[i] + lines_origin), tbm->base());
        return false;
      }
      switch (run_info & 0xC0) {
      case 0x00:
        // uncompressed data
        if (!tbm->fits_array<termel_t>(run_length)) {
          logf_tbm("TBM line %u, col %u (at %u+%u) uncompressed run truncated, "
                   "unpacking %" PRpF "\n",
                   i, (unsigned)x, lines[i],
                   tbm->pos() - (lines[i] + lines_origin), tbm->base());
          return false;
        }
        tbm->skip_array<termel_t>(run_length);
        break;
      case 0x40:
        // char repeat + atttribute data
      case 0x80:
        // char data + attribute repeat
        if (!tbm->fits_array<uint8_t>(run_length + 1)) {
          logf_tbm("TBM line %u, col %u (at %u+%u) char/attr repeat run "
                   "truncated, unpacking %" PRpF "\n",
                   i, (unsigned)x, lines[i],
                   tbm->pos() - (lines[i] + lines_origin), tbm->base());
          return false;
        }
        tbm->skip<uint8_t>();
        tbm->skip_array<uint8_t>(run_length);
        break;
      case 0xC0:
        // termel repeat
        if (!tbm->fits<termel_t>()) {
          logf_tbm("TBM line %u, col %u (at %u+%u) termel repeat run "
                   "truncated, unpacking %" PRpF "\n",
                   i, (unsigned)x, lines[i],
                   tbm->pos() - (lines[i] + lines_origin), tbm->base());
          return false;
        }
        tbm->skip<termel_t>();
        break;
      }
      x += run_length;
    }
  }
  return true;
}

#endif
