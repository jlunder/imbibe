#ifndef __TERMVIZ_H_INCLUDED
#define __TERMVIZ_H_INCLUDED


#include "imbibe.h"


typedef int16_t coord_t;
typedef uint8_t color_t;
typedef uint8_t attribute_t;
typedef uint16_t termel_t;


#define COORD_MIN INT16_MIN
#define COORD_MAX INT16_MAX


class color {
public:
  enum
  {
    black      = 0x00,
    blue       = 0x01,
    green      = 0x02,
    cyan       = 0x03,
    red        = 0x04,
    magenta    = 0x05,
    yellow     = 0x06,
    white      = 0x07,
    hi_black   = 0x08,
    hi_blue    = 0x09,
    hi_green   = 0x0A,
    hi_cyan    = 0x0B,
    hi_red     = 0x0C,
    hi_magenta = 0x0D,
    hi_yellow  = 0x0E,
    hi_white   = 0x0F,

    fg_intensity  = 0x08,
    bg_intensity  = 0x80, // If we're doing ICE color
    blink         = 0x80, // ...otherwise
  };
};


class termviz {
public:
  static uint8_t const fade_steps = 16;

  static uint8_t const fade_masks[fade_steps];
  static uint8_t const fade_seqs[fade_steps][16];
  static uint8_t const dissolve_masks[9][2][4];
};


class attribute {
public:
  static attribute_t from(color_t n_fg, color_t n_bg)
    { return (attribute_t)(n_fg | (n_bg << 4)); }
  static attribute_t from(color_t n_fg, color_t n_bg, bool n_blink)
    { return (attribute_t)(n_fg | (n_bg << 4) | (!!n_blink << 7)); }
  static attribute_t from_foreground(color_t n_fg)
    { return (attribute_t)n_fg; }
  static attribute_t from_background(color_t n_bg)
    { return (attribute_t)(n_bg << 4); }
  static attribute_t with_foreground(attribute_t attr, color_t n_fg)
    { return (attribute_t)((attr & 0xF0) | n_fg); }
  static attribute_t with_background(color_t n_bg, attribute_t attr)
    { return (attribute_t)((attr & 0x0F) | (n_bg << 4)); }
  static attribute_t from_blink(bool n_blink)
    { return (attribute_t)(!!n_blink << 7); }
  static attribute_t with_blink(bool n_blink, attribute_t attr)
    { return (attribute_t)((attr & 0x0F) | (!!n_blink << 7)); }

  static color_t foreground(attribute_t attr) { return attr & 0x0F; }
  static color_t background(attribute_t attr) { return (attr >> 4) & 0x07; }
  static bool blink(attribute_t attr) { return (attr >> 7) & 0x01; }
};


class termel {
public:
  static termel_t from(char n_ch, attribute_t n_attr)
    { return n_ch | ((termel_t)n_attr << 8); }
  static termel_t from(char n_ch, color_t n_fg, color_t n_bg) {
    return n_ch | ((termel_t)attribute::from(n_fg, n_bg) << 8);
  }
  static termel_t from(char n_ch, color_t n_fg, color_t n_bg, bool n_blink) {
    return n_ch | ((termel_t)attribute::from(n_fg, n_bg, n_blink) << 8);
  }
  static termel_t from_ch(char n_ch) { return (termel_t)n_ch; }
  static termel_t from_attribute(attribute_t n_attr) {
    return (termel_t)n_attr << 8;
  }
  static termel_t from_attribute(color_t n_fg, color_t n_bg) {
    return (termel_t)attribute::from(n_fg, n_bg) << 8;
  }
  static termel_t from_attribute(color_t n_fg, color_t n_bg, bool n_blink) {
    return (termel_t)attribute::from(n_fg, n_bg, n_blink) << 8;
  }
  static termel_t with_ch(termel_t te, char n_ch)
    { return (te & 0xFF00) | (termel_t)n_ch; }
  static termel_t with_attribute(termel_t te, color_t n_fg, color_t n_bg) {
    return (te & 0x00FF) | from_attribute(n_fg, n_bg);
  }
  static termel_t with_attribute(termel_t te, color_t n_fg, color_t n_bg,
      bool n_blink) {
    return (te & 0x00FF) | from_attribute(n_fg, n_bg, n_blink);
  }
  static char ch(termel_t te) { return (char)(te & 0xFF); }
  static color_t foreground(termel_t te)
    { return (color_t)((te >> 8) & 0x0F); }
  static color_t background(termel_t te)
    { return (color_t)((te >> 12) & 0x07); }
  static bool blink(termel_t te) { return (bool)((te >> 15) & 0x01); }
};


#endif // __TERMVIZ_H_INCLUDED


