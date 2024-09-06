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
  uint16_t source_stride;
  uint16_t dest_stride;

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
                     uint16_t count) const {
    _fmemcpy(dest, src, count * sizeof(termel_t));
  }
  void fill_line(termel_t __far *dest, termel_t src, uint16_t count) const {
    for (uint16_t i = 0; i < count; ++i) {
      dest[i] = src;
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
                     uint16_t count) const {
    for (uint16_t j = 0; j < count; ++j) {
      termel_t te = src[j];
      dest[j] = termel::with_attribute(te, m_fade_lut[termel::foreground(te)],
                                       m_fade_lut[termel::background(te)]);
    }
  }

  void fill_line(termel_t __far *dest, termel_t src, uint16_t count) const {
    termel_t te =
        termel::with_attribute(src, m_fade_lut[termel::foreground(src)],
                               m_fade_lut[termel::background(src)]);
    for (uint16_t i = 0; i < count; ++i) {
      dest[i] = te;
    }
  }

private:
  color_t const *m_fade_lut;
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
void draw_tbm(graphics *g, coord_t x, coord_t y, tbm const &t, TLineOp op) {
  tbm_header const __far &tbm_h = t.header();

  if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_flat) {
    bitmap_transform_params p;
    if (prepare_plain_tbm_transform_params(g, t, x, y, tbm_h.width,
                                           tbm_h.height, p)) {
      p.transform(op);
    }
  } else if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_rle) {
    aux_graphics::clip_params p;
    if (!p.compute_clip(g, rect(x, y, x + tbm_h.width, y + tbm_h.height))) {
      return;
    }
    assert(p.source.y1 < tbm_h.height);
    assert(p.source.y2 <= tbm_h.height);
    draw_rle_tbm(g, p, t, op);
  } else if ((tbm_h.flags & tbm_flags::flags_format) ==
             tbm_flags::fmt_mask_rle) {
    aux_graphics::clip_params p;
    if (!p.compute_clip(g, rect(x, y, x + tbm_h.width, y + tbm_h.height))) {
      return;
    }
    assert(p.source.y1 < tbm_h.height);
    assert(p.source.y2 <= tbm_h.height);
    draw_mask_rle_tbm(g, p, t, op);
  }
}

template <class TLineOp>
void draw_rle_tbm(graphics *g, aux_graphics::clip_params const &p, tbm const &t,
                  TLineOp op) {
  unpacker d(t.data_unpacker());
  uint16_t const __far *lines = d.unpack_array<uint16_t>(p.source.y1);
  termel_t __far *dest_p =
      g->b()->data() + p.dest.y * g->b()->width() + p.dest.x;
  uint16_t dest_stride = g->b()->width();
  for (coord_t i = p.source.y1; i < p.source.y2; ++i) {
    d.seek_to(lines[i]);
    coord_t run_end = -1; // should be set in loop
    for (coord_t run_start = 0; run_start < p.source.x2; run_start = run_end) {
      uint8_t run_info = d.unpack<uint8_t>();
      uint8_t run_length = ((run_info - 1) & 0x7F) + 1;
      run_end = run_start + run_length;
      run_end = min(run_end, p.source.x2);
      if ((run_info & 0x80) == 0) {
        // copy run
        termel_t const __far *run_data = d.unpack_array<termel_t>(run_length);
        if (run_end > p.source.x1) {
          if (run_start < p.source.x1) {
            run_data += p.source.x1 - run_start;
            run_start = p.source.x1;
          }
          op.transfer_line(dest_p + run_start - p.source.x1, run_data,
                           run_end - run_start);
        }
      } else {
        // fill run
        termel_t run_tm = d.unpack<termel_t>();
        if (run_end > p.source.x1) {
          if (run_start < p.source.x1) {
            run_start = p.source.x1;
          }
          op.fill_line(dest_p + run_start - p.source.x1, run_tm,
                       run_end - run_start);
        }
      }
      assert(run_end > 0);
    }
    dest_p += dest_stride;
  }
}

template <class TLineOp>
void draw_mask_rle_tbm(graphics *g, aux_graphics::clip_params const &p,
                       tbm const &t, TLineOp op) {
  unpacker d(t.data_unpacker());
  uint16_t const __far *lines = d.unpack_array<uint16_t>(p.source.y1);
  termel_t __far *dest_p =
      g->b()->data() + p.dest.y * g->b()->width() + p.dest.x;
  uint16_t dest_stride = g->b()->width();
  for (coord_t i = p.source.y1; i < p.source.y2; ++i) {
    if (lines[i] == 0) {
      continue;
    }
    d.seek_to(lines[i]);
    coord_t span_end = -1; // should be set in loop
    for (coord_t span_start = 0; span_start < p.source.x2;
         span_start = span_end) {
      tbm_span const __far &span = d.unpack<tbm_span>();
      if ((span.skip == 0) && (span.termel_count == 0)) {
        // early end-of-line
        break;
      }
      span_start += span.skip;
      if (span_start >= p.source.x2) {
        break;
      }
      termel_t const __far *span_data =
          d.unpack_array<termel_t>(span.termel_count);
      span_end = span_start + span.termel_count;
      if (span_end > p.source.x1) {
        if (span_start < p.source.x1) {
          span_data += p.source.x1 - span_start;
          span_start = p.source.x1;
        }
        if (span_end > p.source.x2) {
          span_end = p.source.x2;
        }
        op.transfer_line(dest_p + span_start - p.source.x1, span_data,
                         span_end - span_start);
      }
      assert(span_end > 0);
      assert(span_end > span_start);
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

void graphics::draw_rectangle(rect const &r, termel_t p) {
  assert(m_clip.x1 >= 0);
  assert(m_clip.y1 >= 0);
  assert(m_clip.reasonable());
  assert(r.reasonable());

  rect cr = m_clip & (m_origin + r);
  if (cr.trivial()) {
    return;
  }

  logf_graphics("clipped draw_rectangle %d, %d, %d, %d; %c, %02X\n", cx1, cy1,
                cx2, cy2, p.character(), p.attribute().value());

  uint16_t rows = cr.height();
  uint16_t stride = m_b->width();
  uint16_t cols = cr.width();
  termel_t __far *pp = m_b->data() + cr.y1 * stride + cr.x1;
  for (uint16_t r = 0; r < rows; ++r) {
    for (uint16_t c = 0; c < cols; ++c) {
      pp[c] = p;
    }
    pp += stride;
  }
}

void graphics::draw_text(coord_t x, coord_t y, attribute_t attr,
                         char const *s) {
  assert(m_clip.x1 >= 0);
  assert(m_clip.y1 >= 0);
  assert(m_clip.x2 >= 0);
  assert(m_clip.y2 >= 0);
  coord_t i;
  coord_t s_len = strlen(s);

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
