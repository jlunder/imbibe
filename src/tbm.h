#ifndef __TBM_H_INCLUDED
#define __TBM_H_INCLUDED

#include "imbibe.h"

#include "iff.h"
#include "immutable.h"
#include "unpacker.h"

class bitmap;

class tbm_flags {
public:
  enum {
    flags_ansi = 0x00FF,
    ice_color = 0x0001,
    font_8px = 0x0002,      // disable inter-character gap
    font_9px = 0x0004,      // add inter-character gap (dup last col for C0-DF)
    aspect_legacy = 0x0008, // scale font to make 80x25 cells 1:2.4
    aspect_square = 0x0010, // scale font to make character pixels square
    flags_format = 0x0F00,
    fmt_flat = 0x0100,
    fmt_rle = 0x0200,
    fmt_mask_flat = 0x0300, // not supported yet
    fmt_mask_key = 0x0400,  // not supported yet
    fmt_mask_rle = 0x0500,
    fmt_xbin = 0x0600,
    fmt_mask_xbin = 0x0700,
  };
};

static uint16_t const tbm_skip_tm = 0x0000;

_Packed struct __packed__ tbm_header {
  uint16_t width;
  uint16_t height;
  uint16_t flags;
};

class tbm {
public:
  tbm() : m_raw() { assert(!m_raw); }
  tbm(immutable const &n_raw, segsize_t raw_size);
  ~tbm() {}

  bool valid() const { return m_raw; }

  void to_bitmap(bitmap *out_b) const;

  tbm_header const __far &header() const {
    assert(valid());
    return *reinterpret_cast<tbm_header const __far *>(
        reinterpret_cast<iff_header const __far *>(m_raw.data()) + 1);
  }
  void const __far *data() const {
    return reinterpret_cast<void const __far *>(&header() + 1);
  }
  unpacker data_unpacker() const {
    assert(valid());
    iff_header const __far *ih =
        reinterpret_cast<iff_header const __far *>(m_raw.data());
    tbm_header const __far *th =
        reinterpret_cast<tbm_header const __far *>(ih + 1);
    return unpacker(reinterpret_cast<void const __far *>(th + 1),
                    (segsize_t)(ih->data_size - sizeof(tbm_header)));
  }

  coord_t width() const {return (coord_t)header().width;}
  coord_t height() const {return (coord_t)header().height;}

private:
  immutable m_raw;
};

#endif // __TBM_H_INCLUDED
