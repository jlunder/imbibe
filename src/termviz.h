#ifndef __TERMVIZ_H_INCLUDED
#define __TERMVIZ_H_INCLUDED

#include "base.h"

#define assert_margin(x, mag)                                                  \
  assert(((x) >= -((mag) / 4)) && ((x) <= (mag) / 4))

typedef int16_t coord_t;
typedef uint8_t color_t;
typedef uint8_t attribute_t;
typedef uint16_t termel_t;
typedef int16_t anim_time_t;
typedef int32_t large_anim_time_t;

struct point {
  coord_t x;
  coord_t y;

  point() {}
#if defined(__WATCOMC__)
  point(point const &other) : x(other.x), y(other.y) {}
#else
  point(point const &other) = default;
#endif
#if !BUILD_FAR_DATA
  point(point const __far &other) : x(other.x), y(other.y) {}
#endif
  point(coord_t n_x, coord_t n_y) : x(n_x), y(n_y) {}

#if defined(__WATCOMC__)
  point &operator=(point const &other) {
    x = other.x;
    y = other.y;
    return *this;
  }
#else
  point &operator=(point const &other) = default;
#endif
#if !BUILD_FAR_DATA
  point &operator=(point const __far &other) {
    x = other.x;
    y = other.y;
    return *this;
  }
#endif

  point &operator+=(point p) {
    x += p.x;
    y += p.y;
    return *this;
  }

  point &operator-=(point p) {
    x -= p.x;
    y -= p.y;
    return *this;
  }

  bool operator==(point const &r) const { return (x == r.x) && (y == r.y); }
  bool operator!=(point const &r) const { return (x != r.x) || (y != r.y); }

  bool reasonable() const {
    // The >> 1 gets around an issue where abs(INT16_MIN) == INT16_MIN
    return (((abs(x >> 1) | abs(y >> 1)) & ~(INT16_MAX / 4)) == 0);
  }
};

struct rect {
  coord_t x1;
  coord_t y1;
  coord_t x2;
  coord_t y2;

  rect() {}
#if defined(__WATCOMC__)
// rect(rect const & other): x1(other.x1), y1(other.y1), x2(other.x2),
// y2(other.y2) {}
#else
  rect(rect const &other) = default;
  rect(rect const __far &other)
      : x1(other.x1), y1(other.y1), x2(other.x2), y2(other.y2) {}
#endif
  rect(coord_t n_x1, coord_t n_y1, coord_t n_x2, coord_t n_y2)
      : x1(n_x1), y1(n_y1), x2(n_x2), y2(n_y2) {}

  void set_origin(coord_t n_x, coord_t n_y) {
    x2 = x2 - x1 + n_x;
    y2 = y2 - y1 + n_y;
    x1 = n_x;
    y1 = n_y;
  }

  void set_size(coord_t n_width, coord_t n_height) {
    x2 = x1 + n_width;
    y2 = y1 + n_height;
  }

  rect &assign(coord_t n_x1, coord_t n_y1, coord_t n_x2, coord_t n_y2) {
    x1 = n_x1;
    y1 = n_y1;
    x2 = n_x2;
    y2 = n_y2;
    return *this;
  }

#if defined(__WATCOMC__)
  rect &operator=(rect const &other) {
    x1 = other.x1;
    y1 = other.y1;
    x2 = other.x2;
    y2 = other.y2;
    return *this;
  }
#else
  rect &operator=(rect const &other) = default;
#endif
#if !BUILD_FAR_DATA
  rect &operator=(rect const __far &other) {
    x1 = other.x1;
    y1 = other.y1;
    x2 = other.x2;
    y2 = other.y2;
    return *this;
  }
#endif

  rect &operator+=(point p) {
    x1 += p.x;
    y1 += p.y;
    x2 += p.x;
    y2 += p.y;
    return *this;
  }

  rect &operator-=(point p) {
    x1 -= p.x;
    y1 -= p.y;
    x2 -= p.x;
    y2 -= p.y;
    return *this;
  }

  rect &operator+=(rect const &r) {
    x1 += r.x1;
    y1 += r.y1;
    x2 += r.x2;
    y2 += r.y2;
    return *this;
  }

  rect &operator-=(rect const &r) {
    x1 -= r.x1;
    y1 -= r.y1;
    x2 -= r.x2;
    y2 -= r.y2;
    return *this;
  }

  rect &operator&=(rect const &r) {
    x1 = max(x1, r.x1);
    y1 = max(y1, r.y1);
    x2 = min(x2, r.x2);
    y2 = min(y2, r.y2);
    return *this;
  }

  rect &operator|=(rect const &r) {
    x1 = min(x1, r.x1);
    y1 = min(y1, r.y1);
    x2 = max(x2, r.x2);
    y2 = max(y2, r.y2);
    return *this;
  }

  bool operator==(rect const &r) const {
    return (x1 == r.x1) && (x2 == r.x2) && (y1 == r.y1) && (y2 == r.y2);
  }

  bool operator!=(rect const &r) const {
    return (x1 != r.x1) || (x2 != r.x2) | (y1 != r.y1) || (y2 != r.y2);
  }

  int32_t area() const { return (int32_t)(x2 - x1) * (y2 - y1); }
  coord_t width() const { return x2 - x1; }
  coord_t height() const { return y2 - y1; }
  bool trivial() const { return (x2 <= x1) || (y2 <= y1); }
  bool reasonable() const {
    // The >> 1 gets around an issue where abs(INT16_MIN) == INT16_MIN
    return (((abs(x1 >> 1) | abs(y1 >> 1) | abs(x2 >> 1) | abs(y2 >> 1)) &
             ~(INT16_MAX / 4)) == 0);
  }

  bool contains(point p) const {
    return (p.y >= y1) && (p.y < y2) && (p.x >= x1) && (p.x < x2);
  }
  bool contains(coord_t n_x, coord_t n_y) const {
    return (n_y >= y1) && (n_y < y2) && (n_x >= x1) && (n_x < x2);
  }
  bool overlaps(rect const &r) const {
    return (y1 < r.y2) && (r.y1 < y2) && (x1 < r.x2) && (r.x1 < x2);
  }
  bool overlaps(coord_t n_x1, coord_t n_y1, coord_t n_x2, coord_t n_y2) const {
    return (y1 < n_y2) && (n_y1 < y2) && (x1 < n_x2) && (n_x1 < x2);
  }
};

inline point operator+(point a, point b) { return point(a.x + b.x, a.y + b.y); }
inline point operator-(point a, point b) { return point(a.x - b.x, a.y - b.y); }

inline rect operator+(point p, rect const &r) {
  return rect(r.x1 + p.x, r.y1 + p.y, r.x2 + p.x, r.y2 + p.y);
}

inline rect operator-(point p, rect const &r) {
  return rect(r.x1 - p.x, r.y1 - p.y, r.x2 - p.x, r.y2 - p.y);
}

inline rect operator+(rect const &r, point p) {
  return rect(r.x1 + p.x, r.y1 + p.y, r.x2 + p.x, r.y2 + p.y);
}

inline rect operator-(rect const &r, point p) {
  return rect(r.x1 - p.x, r.y1 - p.y, r.x2 - p.x, r.y2 - p.y);
}

inline rect operator&(rect const &a, rect const &b) {
  return rect(max(a.x1, b.x1), max(a.y1, b.y1), min(a.x2, b.x2),
              min(a.y2, b.y2));
}

inline rect operator|(rect const &a, rect const &b) {
  return rect(min(a.x1, b.x1), min(a.y1, b.y1), max(a.x2, b.x2),
              max(a.y2, b.y2));
}

#define COORD_MIN INT16_MIN
#define COORD_MAX INT16_MAX
#define ANIM_TIME_MAX INT16_MAX
#define ANIM_TIME_MIN INT16_MIN
#define LARGE_ANIM_TIME_MAX INT32_MAX
#define LARGE_ANIM_TIME_MIN INT32_MIN

class color {
public:
  enum {
    black = 0x00,
    blue = 0x01,
    green = 0x02,
    cyan = 0x03,
    red = 0x04,
    magenta = 0x05,
    yellow = 0x06,
    white = 0x07,
    hi_black = 0x08,
    hi_blue = 0x09,
    hi_green = 0x0A,
    hi_cyan = 0x0B,
    hi_red = 0x0C,
    hi_magenta = 0x0D,
    hi_yellow = 0x0E,
    hi_white = 0x0F,

    fg_intensity = 0x08,
    bg_intensity = 0x80, // If we're doing ICE color
    blink = 0x80,        // ...otherwise
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
  static attribute_t from(color_t n_fg, color_t n_bg) {
    return (attribute_t)(n_fg | (n_bg << 4));
  }
  static attribute_t from(color_t n_fg, color_t n_bg, bool n_blink) {
    return (attribute_t)(n_fg | (n_bg << 4) | (!!n_blink << 7));
  }
  static attribute_t from_foreground(color_t n_fg) { return (attribute_t)n_fg; }
  static attribute_t from_background(color_t n_bg) {
    return (attribute_t)(n_bg << 4);
  }
  static attribute_t with_foreground(attribute_t attr, color_t n_fg) {
    return (attribute_t)((attr & 0xF0) | n_fg);
  }
  static attribute_t with_background(color_t n_bg, attribute_t attr) {
    return (attribute_t)((attr & 0x0F) | (n_bg << 4));
  }
  static attribute_t from_blink(bool n_blink) {
    return (attribute_t)(!!n_blink << 7);
  }
  static attribute_t with_blink(bool n_blink, attribute_t attr) {
    return (attribute_t)((attr & 0x0F) | (!!n_blink << 7));
  }

  static color_t foreground(attribute_t attr) { return attr & 0x0F; }
  static color_t background(attribute_t attr) { return (attr >> 4) & 0x07; }
  static bool blink(attribute_t attr) { return (attr >> 7) & 0x01; }
};

class termel {
public:
  static termel_t from(char n_ch, attribute_t n_attr) {
    return n_ch | ((termel_t)n_attr << 8);
  }
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
  static termel_t with_ch(termel_t te, char n_ch) {
    return (te & 0xFF00) | (termel_t)n_ch;
  }
  static termel_t with_attribute(termel_t te, color_t n_fg, color_t n_bg) {
    return (te & 0x00FF) | from_attribute(n_fg, n_bg);
  }
  static termel_t with_attribute(termel_t te, color_t n_fg, color_t n_bg,
                                 bool n_blink) {
    return (te & 0x00FF) | from_attribute(n_fg, n_bg, n_blink);
  }
  static char ch(termel_t te) { return (char)(te & 0xFF); }
  static color_t foreground(termel_t te) { return (color_t)((te >> 8) & 0x0F); }
  static color_t background(termel_t te) {
    return (color_t)((te >> 12) & 0x07);
  }
  static attribute_t attribute(termel_t te) { return (attribute_t)(te >> 8); }
  static bool blink(termel_t te) { return (bool)((te >> 15) & 0x01); }
};

#endif // __TERMVIZ_H_INCLUDED
