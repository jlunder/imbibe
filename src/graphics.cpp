#include "imbibe.h"

#include "graphics.h"

#include "bitmap.h"
#include "tbm.h"
#include "unpacker.h"


#define logf_graphics(...) disable_logf("GRAPHICS: " __VA_ARGS__)


namespace aux_graphics {

  struct clip_params {
    coord_t dest_x;
    coord_t dest_y;
    coord_t source_x1;
    coord_t source_y1;
    coord_t source_x2;
    coord_t source_y2;

    bool compute_clip(graphics const & g, coord_t x1, coord_t y1, coord_t x2,
      coord_t y2);
  };

  bool clip_params::compute_clip(graphics const & g, coord_t x1, coord_t y1,
      coord_t x2, coord_t y2) {
    assert(g.clip_x1() >= 0); assert(g.clip_x1() <= g.clip_x2());
    assert(g.clip_x2() <= g.b().width());
    assert(g.clip_y1() >= 0); assert(g.clip_y1() <= g.clip_y2());
    assert(g.clip_y2() <= g.b().height());
    assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);
    assert_margin(x2, COORD_MAX); assert_margin(y2, COORD_MAX);

    dest_x = g.origin_x() + x1;
    dest_y = g.origin_y() + y1;
    source_x1 = 0;
    source_y1 = 0;
    source_x2 = x2 - x1;
    source_y2 = y2 - y1;

    if ((dest_y + source_y2) > g.clip_y2()) {
      source_y2 = g.clip_y2() - dest_y;
    }
    if (dest_y < g.clip_y1()) {
      source_y1 = g.clip_y1() - dest_y;
      dest_y = g.clip_y1();
    }
    if (source_y2 <= source_y1) {
      return false;
    }

    if ((dest_x + source_x2) > g.clip_x2()) {
      source_x2 = g.clip_x2() - dest_x;
    }
    if (dest_x < g.clip_x1()) {
      source_x1 = g.clip_x1() - dest_x;
      dest_x = g.clip_x1();
    }
    if (source_x2 <= source_x1) {
      return false;
    }

    assert(dest_x >= 0); assert(dest_x < g.b().width());
    assert(dest_x + (source_x2 - source_x1) <= g.b().width());
    assert(dest_y >= 0); assert(dest_y < g.b().height());
    assert(dest_y + (source_y2 - source_y1) <= g.b().height());
    assert(source_x1 >= 0); assert(source_x1 < source_x2);
    assert(source_x2 <= (x2 - x1));
    assert(source_y1 >= 0); assert(source_y1 < source_y2);
    assert(source_y2 <= (y2 - y1));

    return true;
  }

  struct bitmap_transform_params: public clip_params {
    coord_t termels_per_line;
    coord_t lines;
    termel_t const * source_p;
    termel_t * dest_p;
    uint16_t source_stride;
    uint16_t dest_stride;

    bool compute_transform(graphics & g, coord_t x, coord_t y,
      coord_t width, coord_t height, termel_t const __far * data);

    template<class TLineOp>
    void transform(TLineOp const & op) {
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
    void transfer_line(termel_t __far * dest, termel_t const __far * src,
        uint16_t count) const {
      memcpy(dest, src, count * sizeof (termel_t));
    }
  };

  class fade_line_op {
  public:
    explicit fade_line_op(uint8_t n_fade) {
      assert(n_fade <= termviz::fade_steps);
      m_fade_lut = termviz::fade_seqs[n_fade];
    }

    void transfer_line(termel_t __far * dest, termel_t const __far * src,
        uint16_t count) const {
      for (uint16_t j = 0; j < count; ++j) {
        termel_t te = src[j];
        dest[j] = termel::with_attribute(te,
          m_fade_lut[termel::foreground(te)],
          m_fade_lut[termel::background(te)]);
      }
    }

  private:
    color_t const * m_fade_lut;
  };

  bool bitmap_transform_params::compute_transform(graphics & g,
      coord_t x, coord_t y, coord_t width, coord_t height,
      termel_t const __far * data) {
    if (!clip_params::compute_clip(g, x, y, x + width, y + height)) {
      return false;
    }

    termels_per_line = (source_x2 - source_x1);
    lines = source_y2 - source_y1;
    source_p = data + source_y1 * width + source_x1;
    dest_p = g.b().data() + dest_y * g.b().width() + dest_x;
    source_stride = width;
    dest_stride = g.b().width();

    return true;
  }

  bool prepare_plain_tbm_transform_params(graphics & g, unpacker & tbm,
       coord_t x, coord_t y, coord_t width, coord_t height,
       bitmap_transform_params & p) {
    uint16_t plane_size = width * height;
    if (!p.compute_transform(g, x, y, width, height,
        tbm.unpack_array<termel_t>(plane_size))) {
      return false;
    }

    return true;
  }

  template<class TLineOp>
  void draw_tbm(graphics & g, coord_t x, coord_t y,
      unpacker const & tbm_data, TLineOp op) {
    unpacker tbm(tbm_data);
    iff_header const & iff_h = tbm.unpack<iff_header>();
    tbm = unpacker(tbm.peek_untyped(),
      (uint16_t)min<uint32_t>(tbm.remain(), iff_h.data_size));
    tbm_header tbm_h = tbm.unpack<tbm_header>();

    if ((tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_plain) {
      bitmap_transform_params p;
      if (prepare_plain_tbm_transform_params(g, tbm, x, y, tbm_h.width,
          tbm_h.height, p)) {
        p.transform(op);
      }
    } else if (
        (tbm_h.flags & tbm_flags::flags_format) == tbm_flags::fmt_mask_rle) {
      aux_graphics::clip_params p;
      if (!p.compute_clip(g, x, y, x + tbm_h.width, y + tbm_h.height)) {
        return;
      }
      assert(p.source_y1 < tbm_h.height); assert(p.source_y2 <= tbm_h.height);
      draw_mask_rle_tbm(g, p, tbm, op);
    }
  }

  template<class TLineOp>
  void draw_mask_rle_tbm(graphics & g, aux_graphics::clip_params const & p,
      unpacker const & tbm_data, TLineOp op) {
    unpacker tbm(tbm_data);
    uint16_t lines_origin = tbm.pos();
    uint16_t const * lines = tbm.unpack_array<uint16_t>(p.source_y1);
    termel_t * dest_p = g.b().data() + p.dest_y * g.b().width() + p.dest_x;
    uint16_t dest_stride = g.b().width();
    for (coord_t i = p.source_y1; i < p.source_y2; ++i) {
      if (lines[i] == 0) {
        continue;
      }
      tbm.seek_to(lines[i] + lines_origin);
      uint16_t span_start = 0;
      for (;;) {
        tbm_span span = tbm.unpack<tbm_span>();
        if ((span.skip == 0) && (span.termel_count == 0)) {
          break;
        }
        span_start += span.skip;
        if (span_start >= p.source_x2) {
          break;
        }
        termel_t const __far * span_data =
          tbm.unpack_array<termel_t>(span.termel_count);
        uint16_t span_end = span_start + span.termel_count;
        if (span_start < p.source_x1) {
          span_data += p.source_x1 - span_start;
          span_start = p.source_x1;
        }
        if (span_end >= p.source_x2) {
          span_end = p.source_x2;
        }
        op.transfer_line(dest_p + span_start - p.source_x1, span_data,
          span_end - span_start);
        if (span_end >= p.source_x2) {
          break;
        }
        span_start = span_end;
      }
      dest_p += dest_stride;
    }
  }
}


graphics::graphics(bitmap & n_b)
  : m_b(n_b), m_x(0), m_y(0), m_clip_x1(0), m_clip_y1(0),
    m_clip_x2(n_b.width()), m_clip_y2(n_b.height()) {
#ifndef NDEBUG
  m_subregion_depth = 0;
#endif
}


void graphics::enter_subregion(subregion_state & save, coord_t x, coord_t y,
    coord_t clip_x1, coord_t clip_y1, coord_t clip_x2, coord_t clip_y2) {
  assert_margin(x, COORD_MAX); assert_margin(y, COORD_MAX);
  assert_margin(clip_x1, COORD_MAX); assert_margin(clip_y1, COORD_MAX);
  assert_margin(clip_x2, COORD_MAX); assert_margin(clip_y2, COORD_MAX);
  assert(clip_x1 <= clip_x2); assert(clip_y1 <= clip_y2);

  logf_graphics("graphics %p enter_subregion %d, %d, %d, %d, %d, %d\n", this,
    x, y, clip_x1, clip_y1, clip_x2, clip_y2);

  save.m_x = m_x;
  save.m_y = m_y;
  save.m_clip_x1 = m_clip_x1;
  save.m_clip_y1 = m_clip_y1;
  save.m_clip_x2 = m_clip_x2;
  save.m_clip_y2 = m_clip_y2;
#ifndef NDEBUG
  save.m_subregion_depth = m_subregion_depth;
  ++m_subregion_depth;
#endif

  m_clip_x1 = max<coord_t>(m_clip_x1, m_x + clip_x1);
  m_clip_y1 = max<coord_t>(m_clip_y1, m_y + clip_y1);
  m_clip_x2 = min<coord_t>(m_clip_x2, m_x + clip_x2);
  m_clip_y2 = min<coord_t>(m_clip_y2, m_y + clip_y2);
  m_x += x;
  m_y += y;

  logf_graphics("  clip %d, %d, %d, %d\n", m_clip_x1, m_clip_y1, m_clip_x2,
    m_clip_y2);
}


void graphics::leave_subregion(subregion_state const & restore) {
#ifndef NDEBUG
  --m_subregion_depth;
  assert(restore.m_subregion_depth == m_subregion_depth);
#endif
  m_x = restore.m_x;
  m_y = restore.m_y;
  m_clip_x1 = restore.m_clip_x1;
  m_clip_y1 = restore.m_clip_y1;
  m_clip_x2 = restore.m_clip_x2;
  m_clip_y2 = restore.m_clip_y2;

  logf_graphics("graphics %p leave_subregion\n", this);
  logf_graphics("  clip %d, %d, %d, %d\n", m_clip_x1, m_clip_y1,
    m_clip_x2, m_clip_y2);
}


void graphics::draw_rectangle(coord_t x1, coord_t y1, coord_t x2,
    coord_t y2, termel_t p) {
  assert(clip_x1() >= 0); assert(clip_y1() >= 0);
  assert(clip_x2() >= 0); assert(clip_y2() >= 0);
  assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);
  assert_margin(x2, COORD_MAX); assert_margin(y2, COORD_MAX);

  coord_t cx1 = max<coord_t>(origin_x() + x1, clip_x1());
  coord_t cx2 = min<coord_t>(origin_x() + x2, clip_x2());
  if (cx1 >= cx2) {
    return;
  }
  coord_t cy1 = max<coord_t>(origin_y() + y1, clip_y1());
  coord_t cy2 = min<coord_t>(origin_y() + y2, clip_y2());
  if (cy1 >= cy2) {
    return;
  }

  logf_graphics("clipped draw_rectangle %d, %d, %d, %d; %c, %02X\n",
    cx1, cy1, cx2, cy2, p.character(), p.attribute().value());

  uint16_t rows = cy2 - cy1;
  uint16_t stride = m_b.width();
  uint16_t cols = cx2 - cx1;
  termel_t * pp = m_b.data() + cy1 * stride + cx1;
  for (uint16_t r = 0; r < rows; ++r) {
    for (uint16_t c = 0; c < cols; ++c) {
      pp[c] = p;
    }
    pp += stride;
  }
}


void graphics::draw_text(coord_t x, coord_t y, attribute_t attr,
    char const * s) {
  assert(clip_x1() >= 0); assert(clip_y1() >= 0);
  assert(clip_x2() >= 0); assert(clip_y2() >= 0);
  coord_t i;
  coord_t s_len = strlen(s);

  x += origin_x();
  y += origin_y();

  if ((y < clip_y2()) && (y >= clip_y1())) {
    if ((x < clip_x2()) && (x + s_len > clip_x1())) {
      if (x < clip_x1()) {
        s += clip_x1() - x;
        s_len -= clip_x1() - x;
        x = clip_x1();
      }
      if (x + s_len > clip_x2()) {
        s_len = clip_x2() - x;
      }
      for (i = 0; i < s_len; ++i) {
        m_b.set_at(x + i, y, termel::from(*s, attr));
        ++s;
      }
    }
  }
}


void graphics::draw_bitmap(coord_t x, coord_t y, bitmap const & b) {
  aux_graphics::bitmap_transform_params p;
  if (p.compute_transform(*this, x, y, b.width(), b.height(), b.data())) {
    p.transform(aux_graphics::copy_line_op());
  }
}


void graphics::draw_bitmap_fade(coord_t x, coord_t y, bitmap const & b, uint8_t fade) {
  aux_graphics::bitmap_transform_params p;
  if (p.compute_transform(*this, x, y, b.width(), b.height(), b.data())) {
    p.transform(aux_graphics::fade_line_op(fade));
  }
}


void graphics::draw_tbm(coord_t x, coord_t y, unpacker const & tbm_data) {
  aux_graphics::draw_tbm(*this, x, y, tbm_data,
    aux_graphics::copy_line_op());
}


void graphics::draw_tbm_fade(coord_t x, coord_t y, unpacker const & tbm_data,
    uint8_t fade) {
  aux_graphics::draw_tbm(*this, x, y, tbm_data,
    aux_graphics::fade_line_op(fade));
}

