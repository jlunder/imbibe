#ifndef __TBM_H_INCLUDED
#define __TBM_H_INCLUDED

#include "imbibe.h"

class bitmap;
class unpacker;

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
  };
};

_Packed struct __packed__ iff_header {
  char magic[4]; // "TBMa"
  uint32_t data_size;
};

_Packed struct __packed__ tbm_header {
  uint8_t width;
  uint8_t height;
  uint16_t flags;
};

_Packed struct __packed__ tbm_span {
  uint8_t skip;
  uint8_t termel_count;
};

namespace tbm {

static const size_t s_tbm_area_max = 1u << 14;

bool validate(unpacker const &tbm_data);
void dimensions(unpacker const &tbm_data, coord_t &width, coord_t &height);
void to_bitmap(unpacker const &tbm_data, bitmap &b);

} // namespace tbm

#endif // __TBM_H_INCLUDED
