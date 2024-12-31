#include "imbibe.h"

#include "graphics.h"

#include "bitmap.h"
#include "tbm.h"
#include "unpacker.h"

#define logf_graphics(...) disable_logf("GRAPHICS: " __VA_ARGS__)

namespace aux_graphics {

struct clip_params {
  point dest;
  rect source;

  bool compute_clip(graphics const *g, rect const &r);
};

bool clip_params::compute_clip(graphics const *g, rect const &r) {
  rect g_clip = g->clip();
  assert(g_clip.reasonable());
  assert(g_clip.x1 >= 0);
  assert(g_clip.x2 <= g->b()->width());
  assert(g_clip.y1 >= 0);
  assert(g_clip.y2 <= g->b()->height());
  assert(r.reasonable());

  if (g->clip().trivial() || r.trivial()) {
    return false;
  }

  dest = g->origin() + point(r.x1, r.y1);
  source.assign(0, 0, r.width(), r.height());

  if ((dest.y + source.y2) > g_clip.y2) {
    source.y2 = g_clip.y2 - dest.y;
  }
  if (dest.y < g_clip.y1) {
    source.y1 = g_clip.y1 - dest.y;
    dest.y = g_clip.y1;
  }
  if (source.y2 <= source.y1) {
    return false;
  }

  if ((dest.x + source.x2) > g_clip.x2) {
    source.x2 = g_clip.x2 - dest.x;
  }
  if (dest.x < g_clip.x1) {
    source.x1 = g_clip.x1 - dest.x;
    dest.x = g_clip.x1;
  }
  if (source.x2 <= source.x1) {
    return false;
  }

  assert(rect(0, 0, g->b()->width(), g->b()->height()).contains(dest));
  assert((rect(0, 0, g->b()->width(), g->b()->height()) &
          (dest + rect(0, 0, source.width(), source.height()))) ==
         (dest + rect(0, 0, source.width(), source.height())));
  assert(source.reasonable());
  assert((source & rect(0, 0, r.width(), r.height())) == source);

  return true;
}

struct bitmap_transform_params : public clip_params {
  coord_t termels_per_line;
  coord_t lines;
  termel_t const __far *source_p;
  termel_t __far *dest_p;
  segsize_t source_stride;
  segsize_t dest_stride;

  bool compute_transform(graphics *g, coord_t x, coord_t y, coord_t width,
                         coord_t height, termel_t const __far *data);

  template <class TLineOp> void transform(TLineOp const &op) {
    for (coord_t i = 0; i < lines; ++i) {
      op.transfer_line(dest_p, source_p, termels_per_line);
      next_line();
    }
  }

  void next_line() {
    dest_p += dest_stride;
    source_p += source_stride;
  }
};

class copy_line_op {
public:
  void transfer_line(termel_t __far *dest, termel_t const __far *src,
                     segsize_t count) const {
    _fmemcpy(dest, src, count * sizeof(termel_t));
  }

  void fill_line(termel_t __far *dest, termel_t src, segsize_t count) const {
    for (segsize_t i = 0; i < count; ++i) {
      dest[i] = src;
    }
  }

  void char_fill_attr_transfer_line(termel_t __far *dest, char src_c,
                                    attribute_t const __far *src_attrs,
                                    segsize_t count) const {
    for (segsize_t i = 0; i < count; ++i) {
      dest[i] = termel::from(src_c, src_attrs[i]);
    }
  }

  void char_transfer_attr_fill_line(termel_t __far *dest,
                                    char const __far *src_cs,
                                    attribute_t src_attr,
                                    segsize_t count) const {
    for (segsize_t i = 0; i < count; ++i) {
      dest[i] = termel::from(src_cs[i], src_attr);
    }
  }
};

class fade_line_op {
public:
  explicit fade_line_op(uint8_t n_fade) {
    assert(n_fade <= termviz::fade_steps);
    m_fade_lut = termviz::fade_seqs[n_fade];
  }

  void transfer_line(termel_t __far *dest, termel_t const __far *src,
                     segsize_t count) const {
    for (segsize_t j = 0; j < count; ++j) {
      termel_t te = src[j];
      dest[j] = termel::with_attribute(te, m_fade_lut[termel::foreground(te)],
                                       m_fade_lut[termel::background(te)]);
    }
  }

  void fill_line(termel_t __far *dest, termel_t src, segsize_t count) const {
    termel_t te =
        termel::with_attribute(src, m_fade_lut[termel::foreground(src)],
                               m_fade_lut[termel::background(src)]);
    for (segsize_t i = 0; i < count; ++i) {
      dest[i] = te;
    }
  }

  void char_fill_attr_transfer_line(termel_t __far *dest, char src_c,
                                    attribute_t const __far *src_attrs,
                                    segsize_t count) const {
    for (segsize_t i = 0; i < count; ++i) {
      dest[i] = termel::from(src_c, src_attrs[i]);
    }
  }

  void char_transfer_attr_fill_line(termel_t __far *dest,
                                    char const __far *src_cs,
                                    attribute_t src_attr,
                                    segsize_t count) const {
    for (segsize_t i = 0; i < count; ++i) {
      dest[i] = termel::from(src_cs[i], src_attr);
    }
  }

private:
  color_t const *m_fade_lut;
};

static const uint8_t s_brightness[16] = {
    0,     2,     3,     2 + 3,     1,     1 + 2,     1 + 3,     1 + 2 + 3,
    1 + 0, 2 + 2, 2 + 3, 3 + 2 + 3, 2 + 1, 3 + 1 + 2, 3 + 1 + 3, 4 + 1 + 2 + 3};

class blend_termel_line_op {
public:
  explicit blend_termel_line_op(uint8_t n_fade) {
    assert(n_fade <= termviz::fade_steps);
    m_fade_src_lut = termviz::fade_seqs[termviz::fade_steps - n_fade];
    m_fade_dest_lut = termviz::fade_seqs[n_fade];
  }

  void transfer_line(termel_t __far *dest, termel_t const __far *src,
                     segsize_t count) const {
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      uint8_t dest_brt = s_brightness[dest_fg] + s_brightness[dest_bg];
      termel_t src_te = src[j];
      uint8_t src_fg = m_fade_src_lut[termel::foreground(src_te)];
      uint8_t src_bg = m_fade_src_lut[termel::background(src_te)];
      uint8_t src_brt = s_brightness[src_fg] + s_brightness[src_bg];
      dest[j] =
          termel::with_attribute(((dest_brt > src_brt) ? dest_te : src_te),
                                 dest_fg | src_fg, dest_bg | src_bg);
    }
  }

  void fill_line(termel_t __far *dest, termel_t src, segsize_t count) const {
    uint8_t src_fg = m_fade_src_lut[termel::foreground(src)];
    uint8_t src_bg = m_fade_src_lut[termel::background(src)];
    uint8_t src_brt = s_brightness[src_fg] + s_brightness[src_bg];
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      uint8_t dest_brt = s_brightness[dest_fg] + s_brightness[dest_bg];
      dest[j] = termel::with_attribute(((dest_brt > src_brt) ? dest_te : src),
                                       dest_fg | src_fg, dest_bg | src_bg);
    }
  }

  void char_fill_attr_transfer_line(termel_t __far *dest, char src_c,
                                    attribute_t const __far *src_attrs,
                                    segsize_t count) const {
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      uint8_t dest_brt = s_brightness[dest_fg] + s_brightness[dest_bg];
      termel_t src_attr = src_attrs[j];
      uint8_t src_fg = m_fade_src_lut[termel::foreground(src_attr)];
      uint8_t src_bg = m_fade_src_lut[termel::background(src_attr)];
      uint8_t src_brt = s_brightness[src_fg] + s_brightness[src_bg];
      dest[j] =
          termel::from(((dest_brt > src_brt) ? termel::ch(dest_te) : src_c),
                       dest_fg | src_fg, dest_bg | src_bg);
    }
  }

  void char_transfer_attr_fill_line(termel_t __far *dest,
                                    char const __far *src_cs,
                                    attribute_t src_attr,
                                    segsize_t count) const {
    uint8_t src_fg = m_fade_src_lut[attribute::foreground(src_attr)];
    uint8_t src_bg = m_fade_src_lut[attribute::background(src_attr)];
    uint8_t src_brt = s_brightness[src_fg] + s_brightness[src_bg];
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      uint8_t dest_brt = s_brightness[dest_fg] + s_brightness[dest_bg];
      dest[j] =
          termel::from(((dest_brt > src_brt) ? termel::ch(dest_te) : src_cs[j]),
                       dest_fg | src_fg, dest_bg | src_bg);
    }
  }

private:
  color_t const *m_fade_src_lut;
  color_t const *m_fade_dest_lut;
};

class blend_color_line_op {
public:
  explicit blend_color_line_op(uint8_t n_fade) {
    assert(n_fade <= termviz::fade_steps);
    m_fade_src_lut = termviz::fade_seqs[n_fade];
    m_fade_dest_lut = termviz::fade_seqs[termviz::fade_steps - n_fade];
  }

  void transfer_line(termel_t __far *dest, termel_t const __far *src,
                     segsize_t count) const {
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      uint8_t src_fg = m_fade_src_lut[termel::foreground(src[j])];
      uint8_t src_bg = m_fade_src_lut[termel::background(src[j])];
      dest[j] =
          termel::with_attribute(dest_te, dest_fg | src_fg, dest_bg | src_bg);
    }
  }

  void fill_line(termel_t __far *dest, termel_t src, segsize_t count) const {
    uint8_t src_fg = m_fade_src_lut[termel::foreground(src)];
    uint8_t src_bg = m_fade_src_lut[termel::background(src)];
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      dest[j] =
          termel::with_attribute(dest_te, dest_fg | src_fg, dest_bg | src_bg);
    }
  }

  void char_fill_attr_transfer_line(termel_t __far *dest, char src_c,
                                    attribute_t const __far *src_attrs,
                                    segsize_t count) const {
    (void)src_c;
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      uint8_t src_fg = m_fade_src_lut[attribute::foreground(src_attrs[j])];
      uint8_t src_bg = m_fade_src_lut[attribute::background(src_attrs[j])];
      dest[j] =
          termel::with_attribute(dest_te, dest_fg | src_fg, dest_bg | src_bg);
    }
  }

  void char_transfer_attr_fill_line(termel_t __far *dest,
                                    char const __far *src_cs,
                                    attribute_t src_attr,
                                    segsize_t count) const {
    (void)src_cs;
    uint8_t src_fg = m_fade_src_lut[attribute::foreground(src_attr)];
    uint8_t src_bg = m_fade_src_lut[attribute::background(src_attr)];
    for (segsize_t j = 0; j < count; ++j) {
      termel_t dest_te = dest[j];
      uint8_t dest_fg = m_fade_dest_lut[termel::foreground(dest_te)];
      uint8_t dest_bg = m_fade_dest_lut[termel::background(dest_te)];
      dest[j] =
          termel::with_attribute(dest_te, dest_fg | src_fg, dest_bg | src_bg);
    }
  }

private:
  color_t const *m_fade_src_lut;
  color_t const *m_fade_dest_lut;
};

bool bitmap_transform_params::compute_transform(graphics *g, coord_t x,
                                                coord_t y, coord_t width,
                                                coord_t height,
                                                termel_t const __far *data) {
  if (!clip_params::compute_clip(g, rect(x, y, x + width, y + height))) {
    return false;
  }

  termels_per_line = (source.x2 - source.x1);
  lines = source.y2 - source.y1;
  source_p = data + source.y1 * width + source.x1;
  dest_p = g->b()->data() + dest.y * g->b()->width() + dest.x;
  source_stride = width;
  dest_stride = g->b()->width();

  return true;
}

bool prepare_plain_tbm_transform_params(graphics *g, tbm const &t, coord_t x,
                                        coord_t y, coord_t width,
                                        coord_t height,
                                        bitmap_transform_params &p) {
  if (!p.compute_transform(
          g, x, y, width, height,
          reinterpret_cast<const termel_t __far *>(t.data()))) {
    return false;
  }

  return true;
}

template <class TLineOp>
void fill(graphics *g, rect const &r, termel_t t, TLineOp op) {
  rect cr = g->clip() & (g->origin() + r);
  if (cr.trivial()) {
    return;
  }

  segsize_t rows = cr.height();
  segsize_t stride = g->b()->width();
  segsize_t cols = cr.width();
  termel_t __far *pp = g->b()->data() + cr.y1 * stride + cr.x1;
  for (segsize_t r = 0; r < rows; ++r) {
    op.fill_line(pp, t, cols);
    pp += stride;
  }
}

template <class TLineOp>
void draw_tbm(graphics *g, coord_t x, coord_t y, tbm const &t, TLineOp op) {
  tbm_header const __far &tbm_h = t.header();

  if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_flat) {
    bitmap_transform_params p;
    if (prepare_plain_tbm_transform_params(g, t, x, y, tbm_h.width,
                                           tbm_h.height, p)) {
      p.transform(op);
    }
  } else {
    aux_graphics::clip_params p;
    if (!p.compute_clip(g, rect(x, y, x + tbm_h.width, y + tbm_h.height))) {
      return;
    }
    assert(p.source.y1 < tbm_h.height);
    assert(p.source.y2 <= tbm_h.height);
    switch (tbm_h.flags & tbm_flags::flags_format) {
    case tbm_flags::fmt_mask_xbin:
      draw_mask_xbin_tbm(g, p, t, op);
      break;
    }
  }
}

template <class TLineOp>
void draw_mask_xbin_tbm(graphics *g, aux_graphics::clip_params const &p,
                        tbm const &t, TLineOp op) {
  unpacker d(t.data_unpacker());
  segsize_t const __far *lines = d.unpack_array<segsize_t>(p.source.y1);
  termel_t __far *dest_p =
      g->b()->data() + p.dest.y * g->b()->width() + p.dest.x;
  segsize_t dest_stride = g->b()->width();
  for (coord_t i = p.source.y1; i < p.source.y2; ++i) {
    d.seek_to(lines[i]);
    coord_t run_end = -1; // should be set in loop
    for (coord_t run_start = 0; run_start < p.source.x2; run_start = run_end) {
      uint8_t run_info = d.unpack<uint8_t>();
      uint8_t run_length = (run_info & 0x3F) + 1;
      run_end = run_start + run_length;
      run_end = min(run_end, p.source.x2);
      switch (run_info & 0xC0) {
      case 0x00: {
        // uncompressed run
        termel_t const __far *run_termels =
            d.unpack_array<termel_t>(run_length);
        if (run_end > p.source.x1) {
          if (run_start < p.source.x1) {
            run_termels += p.source.x1 - run_start;
            run_start = p.source.x1;
          }
          op.transfer_line(dest_p + run_start - p.source.x1, run_termels,
                           run_end - run_start);
        }
      } break;
      case 0x40: {
        // char fill run
        char run_c = d.unpack<char>();
        attribute_t const __far *run_attrs =
            d.unpack_array<attribute_t>(run_length);
        if (run_end > p.source.x1) {
          if (run_start < p.source.x1) {
            run_attrs += p.source.x1 - run_start;
            run_start = p.source.x1;
          }
          op.char_fill_attr_transfer_line(dest_p + run_start - p.source.x1,
                                          run_c, run_attrs,
                                          run_end - run_start);
        }
      } break;
      case 0x80: {
        // attr fill run
        attribute_t run_attr = d.unpack<attribute_t>();
        char const __far *run_cs = d.unpack_array<char>(run_length);
        if (run_end > p.source.x1) {
          if (run_start < p.source.x1) {
            run_cs += p.source.x1 - run_start;
            run_start = p.source.x1;
          }
          op.char_transfer_attr_fill_line(dest_p + run_start - p.source.x1,
                                          run_cs, run_attr,
                                          run_end - run_start);
        }
      } break;
      case 0xC0: {
        // termel fill run
        termel_t run_tm = d.unpack<termel_t>();
        if (run_end > p.source.x1) {
          if (run_start < p.source.x1) {
            run_start = p.source.x1;
          }
          if (run_tm != tbm_skip_tm) {
            op.fill_line(dest_p + run_start - p.source.x1, run_tm,
                         run_end - run_start);
          }
        }
      } break;
      }
      assert(run_end > 0);
    }
    dest_p += dest_stride;
  }
}

} // namespace aux_graphics

graphics::graphics(bitmap *n_b)
    : m_b(n_b), m_origin(0, 0), m_clip(0, 0, n_b->width(), n_b->height()) {
#if BUILD_DEBUG
  m_subregion_depth = 0;
#endif
}

void graphics::enter_subregion(point sub_o, rect const &sub_clip,
                               subregion_state *save) {
  assert(sub_o.reasonable());
  assert(sub_clip.reasonable());

  logf_graphics("graphics %p enter_subregion %d, %d, %d, %d, %d, %d\n", this, x,
                y, sub_clip.x1, sub_clip.y1, sub_clip.x2, sub_clip.y2);

  save->m_origin = m_origin;
  save->m_clip = m_clip;
#if BUILD_DEBUG
  save->m_subregion_depth = m_subregion_depth;
  ++m_subregion_depth;
#endif

  m_clip &= m_origin + sub_clip;
  m_origin += sub_o;

  logf_graphics("  clip %d, %d, %d, %d\n", m_clip.x1, m_clip.y1, m_clip.x2,
                m_clip.y2);
}

void graphics::leave_subregion(subregion_state const *restore) {
#if BUILD_DEBUG
  --m_subregion_depth;
  assert(restore->m_subregion_depth == m_subregion_depth);
#endif
  m_origin = restore->m_origin;
  m_clip = restore->m_clip;

  logf_graphics("graphics %p leave_subregion\n", this);
  logf_graphics("  clip %d, %d, %d, %d\n", m_clip.x1, m_clip.y1, m_clip.x2,
                m_clip.y2);
}

void graphics::draw_rectangle(rect const &r, termel_t t) {
  aux_graphics::fill(this, r, t, aux_graphics::copy_line_op());
}

void graphics::blend_rectangle(rect const &r, termel_t t, uint8_t fade) {
  aux_graphics::fill(this, r, t, aux_graphics::blend_color_line_op(fade));
}

void graphics::draw_text(coord_t x, coord_t y, attribute_t attr,
                         char const __far *s) {
  assert(m_clip.x1 >= 0);
  assert(m_clip.y1 >= 0);
  assert(m_clip.x2 >= 0);
  assert(m_clip.y2 >= 0);
  coord_t i;
  coord_t s_len = _fstrlen(s);

  x += m_origin.x;
  y += m_origin.y;

  if ((y < m_clip.y2) && (y >= m_clip.y1)) {
    if ((x < m_clip.x2) && (x + s_len > m_clip.x1)) {
      if (x < m_clip.x1) {
        s += m_clip.x1 - x;
        s_len -= m_clip.x1 - x;
        x = m_clip.x1;
      }
      if (x + s_len > m_clip.x2) {
        s_len = m_clip.x2 - x;
      }
      for (i = 0; i < s_len; ++i) {
        m_b->set_at(x + i, y, termel::from(*s, attr));
        ++s;
      }
    }
  }
}

void graphics::draw_bitmap(coord_t x, coord_t y, bitmap const &b) {
  aux_graphics::bitmap_transform_params p;
  if (p.compute_transform(this, x, y, b.width(), b.height(), b.data())) {
    p.transform(aux_graphics::copy_line_op());
  }
}

void graphics::draw_bitmap_fade(coord_t x, coord_t y, bitmap const &b,
                                uint8_t fade) {
  aux_graphics::bitmap_transform_params p;
  if (p.compute_transform(this, x, y, b.width(), b.height(), b.data())) {
    p.transform(aux_graphics::fade_line_op(fade));
  }
}

void graphics::draw_tbm(coord_t x, coord_t y, tbm const &t) {
  aux_graphics::draw_tbm(this, x, y, t, aux_graphics::copy_line_op());
}

void graphics::draw_tbm_fade(coord_t x, coord_t y, tbm const &t, uint8_t fade) {
  aux_graphics::draw_tbm(this, x, y, t, aux_graphics::fade_line_op(fade));
}
