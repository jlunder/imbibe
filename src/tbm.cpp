#include "imbibe.h"

#include "tbm.h"

#include "bitmap.h"
#include "graphics.h"
#include "unpacker.h"

#define logf_tbm(...) logf("TBM: " __VA_ARGS__)

namespace tbm {

bool validate_data_rle(tbm_header const &tbm_h, unpacker &tbm);
bool validate_data_mask_rle(tbm_header const &tbm_h, unpacker &tbm);

} // namespace tbm

bool tbm::validate(unpacker const &tbm_data) {
  unpacker tbm(tbm_data);
  if (!tbm.fits_untyped(sizeof(iff_header) + sizeof(tbm_header))) {
    logf_tbm("too-small TBM, unpacking %p", tbm.peek_untyped());
    return false;
  }

  {
    iff_header const &iff_h = tbm.unpack<iff_header>();
    if (fourcc(iff_h.magic) != fourcc("TBMa")) {
      logf_tbm("bad TBM fourcc, unpacking %p", tbm.base());
      return false;
    }
    tbm = unpacker(tbm.peek_untyped(),
                   (uint16_t)min<uint32_t>(iff_h.data_size, tbm.remain()));
  }

  tbm_header tbm_h = tbm.unpack<tbm_header>();
  uint16_t plane_size = (uint16_t)tbm_h.width * tbm_h.height;
  if ((tbm_h.width == 0) || (tbm_h.height == 0) ||
      (plane_size > s_tbm_area_max)) {
    logf_tbm("suspicious TBM dimensions %u x %u, unpacking %p", tbm_h.width,
             tbm_h.height, tbm.base());
    return false;
  }

  if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_flat) {
    if (!tbm.fits_array<termel_t>(plane_size)) {
      logf_tbm("TBM data %u too small for %u x %u, unpacking %p", tbm.remain(),
               tbm_h.width, tbm_h.height, tbm.base());
      return false;
    }
  } else if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_rle) {
    return validate_data_rle(tbm_h, tbm);
  } else if ((tbm_h.flags & tbm_flags::flags_format) ==
             tbm_flags::fmt_mask_rle) {
    return validate_data_mask_rle(tbm_h, tbm);
  }

  return true;
}

bool tbm::validate_data_rle(tbm_header const &tbm_h, unpacker &tbm) {
  if (!tbm.fits_array<uint16_t>(tbm_h.height)) {
    logf_tbm("TBM line index truncated, unpacking %p", tbm.base());
    return false;
  }
  uint16_t lines_origin = tbm.pos();
  uint16_t const *lines = tbm.unpack_array<uint16_t>(tbm_h.height);
  for (uint16_t i = 0; i < tbm_h.height; ++i) {
    if (lines[i] >= (tbm.size() - lines_origin)) {
      logf_tbm("TBM line %u outside data (at %u), unpacking %p", i, lines[i],
               tbm.base());
      return false;
    }
    tbm.seek_to(lines[i] + lines_origin);
    uint16_t x = 0;
    while (x < tbm_h.width) {
      if (!tbm.fits<uint8_t>()) {
        logf_tbm("TBM line %u run (at %u) truncated, unpacking %p", i,
                 lines[i], tbm.base());
        return false;
      }
      uint8_t run_info = tbm.unpack<uint8_t>();
      uint8_t run_length = ((run_info - 1) & 0x7F) + 1;
      if ((run_length > tbm_h.width) || (x > tbm_h.width - run_length)) {
        logf_tbm("TBM line %u run (at %u) leaves image, unpacking %p", i,
                 (unsigned)(tbm.pos() - sizeof(tbm_span)), tbm.base());
        return false;
      }
      if ((run_info & 0x80) == 0) {
        // data run
        if (!tbm.fits_array<termel_t>(run_length)) {
          logf_tbm("TBM line %u data run (at %u) truncated, unpacking %p", i,
                   lines[i], tbm.base());
          return false;
        }
        tbm.unpack_array<termel_t>(run_length);
      } else {
        // fill run
        if (!tbm.fits<termel_t>()) {
          logf_tbm("TBM line %u fill run (at %u) truncated, unpacking %p", i,
                   lines[i], tbm.base());
          return false;
        }
        tbm.unpack<termel_t>();
      }
      x += run_length;
    }
  }
  return true;
}

bool tbm::validate_data_mask_rle(tbm_header const &tbm_h, unpacker &tbm) {
  if (!tbm.fits_array<uint16_t>(tbm_h.height)) {
    logf_tbm("TBM line index truncated, unpacking %p", tbm.base());
    return false;
  }
  uint16_t lines_origin = tbm.pos();
  uint16_t const *lines = tbm.unpack_array<uint16_t>(tbm_h.height);
  for (uint16_t i = 0; i < tbm_h.height; ++i) {
    if (lines[i] == 0) {
      continue;
    }
    if (lines[i] >= (tbm.size() - lines_origin)) {
      logf_tbm("TBM line %u outside data (at %u), unpacking %p", i, lines[i],
               tbm.base());
      return false;
    }
    tbm.seek_to(lines[i] + lines_origin);
    uint16_t x = 0;
    for (;;) {
      if (!tbm.fits<tbm_span>()) {
        logf_tbm("TBM line %u span (at %u) truncated, unpacking %p", i,
                 lines[i], tbm.base());
        return false;
      }
      tbm_span span = tbm.unpack<tbm_span>();
      if ((span.skip == 0) && (span.termel_count == 0)) {
        // end marker
        break;
      }
      if ((span.skip > tbm_h.width) || (x > tbm_h.width - span.skip)) {
        logf_tbm("TBM line %u skip (at %u) leaves image, unpacking %p", i,
                 (unsigned)(tbm.pos() - sizeof(tbm_span)), tbm.base());
        return false;
      }
      x += span.skip;
      if ((span.termel_count > tbm_h.width) ||
          (x > tbm_h.width - span.termel_count)) {
        logf_tbm("TBM line %u data (at %u) leaves image, unpacking %p", i,
                 (unsigned)(tbm.pos() - sizeof(tbm_span)), tbm.base());
        return false;
      }
      if (!tbm.fits_array<termel_t>(span.termel_count)) {
        logf_tbm("TBM line %u data (at %u) truncated, unpacking %p", i,
                 (unsigned)(tbm.pos() - sizeof(tbm_span)), tbm.base());
        return false;
      }
      tbm.unpack_array<termel_t>(span.termel_count);
      x += span.termel_count;
    }
  }
  return true;
}

void tbm::dimensions(unpacker const &tbm_data, coord_t &width,
                     coord_t &height) {
  assert(validate(tbm_data));
  unpacker tbm(tbm_data);
  tbm.skip<iff_header>();
  tbm_header const &tbm_h = tbm.unpack<tbm_header>();
  width = tbm_h.width;
  height = tbm_h.height;
}

void tbm::to_bitmap(unpacker const &tbm_data, bitmap &b) {
  coord_t width;
  coord_t height;
  dimensions(tbm_data, width, height);
  b.assign(width, height);
  graphics g(b);
  g.draw_tbm(0, 0, tbm_data);
}
