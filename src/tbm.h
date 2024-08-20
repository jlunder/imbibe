#ifndef __TBM_H_INCLUDED
#define __TBM_H_INCLUDED


#include "imbibe.h"


class bitmap;


class tbm_flags {
public:
  enum {
    flags_ansi = 0x00FF,
    ice_color = 0x0001,
    font_8px = 0x0002, // disable inter-character gap
    font_9px = 0x0004, // add inter-character gap (dup last col for C0-DF)
    aspect_legacy = 0x0008, // scale font to make 80x25 cells 1:2.4
    aspect_square = 0x0010, // scale font to make character pixels square
    flags_format = 0x0F00,
    fmt_plain = 0x0000,
    // fmt_plain_qc = 0x0100,
    // fmt_mask_key = 0x0400,
    // fmt_mask_rle = 0x0500,
  };
};


struct tbm_header {
  char magic[4]; // "TBMa"
  uint32_t data_size;
  uint8_t width;
  uint8_t height;
  uint16_t flags;
  uint8_t data_start[0];
} _Packed;


class tbm {
public:
  static bitmap * to_bitmap(tbm_header const * header);
};


#endif // __TBM_H_INCLUDED


