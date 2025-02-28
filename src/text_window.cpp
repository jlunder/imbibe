#include "imbibe.h"

#include "text_window.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "bitmap.h"
#include "element.h"
#include "graphics.h"

#define logf_text_window(...) disable_logf("TEXT_WINDOW: " __VA_ARGS__)

namespace aux_text_window {

static coord_t const s_screen_width = 80;
static coord_t const s_screen_height = 25;
static uint8_t const s_text_mode_color_80_25 = 3;
static termel_t __far *const s_screen_buffer =
    reinterpret_cast<termel_t __far *>(MK_FP(0xB800, 0));

static termel_t __based(__segname("screen_save_DATA")) s_screen_save[80 * 25];
static termel_t __based(__segname("backbuffer_DATA")) s_backbuffer[80 * 25];

enum {
  cursor_visible = 0x00,
  cursor_invisible = 0x20,
};

extern void read_screen_buffer(bitmap *out_b);
extern void set_video_mode(uint8_t mode);
extern void set_cursor_style(uint8_t visible, uint8_t start_row,
                             uint8_t end_row);

} // namespace aux_text_window

#if BUILD_MSDOS

/*
  "   mov     ah, 003h          " \
  "   push    bp                " \
  "   int     010h              " \
  "   pop     bp                " \
  "   mov     pos_row, dh       " \
  "   mov     pos_col, dl       " \
*/
struct asm_video_details {
  uint8_t mode;
  uint8_t cols;
  uint8_t page;
  uint8_t pad;
};

extern asm_video_details asm_bios_get_video_details();
extern void asm_bios_set_video_mode(uint8_t mode);
extern void asm_bios_set_cursor_style(uint8_t start_opts, uint8_t end);

#if BUILD_MSDOS_WATCOMC

#pragma aux asm_bios_get_video_details =                                       \
    "   mov ah, 00Fh              "                                            \
    "   push    bp                "                                            \
    "   int     10h               "                                            \
    "   pop     bp                "                                            \
    "   mov     bl, bh            " modify[ax bx] nomemory value[bx ax]

#pragma aux asm_bios_set_video_mode =                                          \
    "   push    bp                "                                            \
    "   mov ah, 000h              "                                            \
    "   int     10h               "                                            \
    "   pop     bp                " parm[al] modify[ax] nomemory;

#pragma aux asm_bios_set_cursor_style =                                        \
    "   push    bp                "                                            \
    "   mov ah, 001h              "                                            \
    "   int     10h               "                                            \
    "   pop     bp                " parm[ch][cl] modify[ax] nomemory;

#elif BUILD_POSIX
// TODO?
#else
#error New platform support needed?
#endif

inline void aux_text_window::read_screen_buffer(bitmap *out_b) {
  asm_video_details details = asm_bios_get_video_details();
  logf_text_window("read details: mode %d, %d cols, page %d\n", details.mode,
                   details.cols, details.page);
  if (details.mode == 2 || details.mode == 3) {
    out_b->assign(aux_text_window::s_screen_width,
                  aux_text_window::s_screen_height, s_screen_save);
    segsize_t offset = details.page * 4096 / sizeof(termel_t);
    segsize_t size = out_b->width() * out_b->height();
    logf_text_window("computed page offset: %u\n", offset);
    _fmemcpy(out_b->data(), aux_text_window::s_screen_buffer + offset,
             size * sizeof(termel_t));
  } else {
    out_b->assign(aux_text_window::s_screen_width,
                  aux_text_window::s_screen_height, s_screen_save);
    for (segsize_t i = 0; i < LENGTHOF(s_screen_save); ++i) {
      s_screen_save[i] = termel::from(' ', color::white, color::black);
    }
  }
}

inline void aux_text_window::set_video_mode(uint8_t mode) {
  assert(mode == 3);
  asm_bios_set_video_mode(mode);
}

inline void aux_text_window::set_cursor_style(uint8_t visible,
                                              uint8_t start_row,
                                              uint8_t end_row) {
  assert((visible == cursor_visible) || (visible == cursor_invisible));
  assert(start_row <= end_row);
  assert(end_row <= 7);
  asm_bios_set_cursor_style(visible | start_row, end_row);
}

#else

inline void aux_text_window::read_screen_buffer(bitmap *out_b) {
  out_b->assign(aux_text_window::s_screen_width,
                aux_text_window::s_screen_height, s_screen_save);
  _fmemcpy(s_screen_save, aux_text_window::s_screen_buffer,
           sizeof s_screen_save);
}

inline void aux_text_window::set_video_mode(uint8_t mode) { assert(mode == 3); }

inline void aux_text_window::set_cursor_style(uint8_t visible,
                                              uint8_t start_row,
                                              uint8_t end_row) {
  assert((visible == cursor_visible) || (visible == cursor_invisible));
  assert(start_row <= end_row);
  assert(end_row <= 7);
}

#endif

text_window::text_window()
    : window(), m_element(NULL), m_backbuffer(aux_text_window::s_screen_width,
                                              aux_text_window::s_screen_height,
                                              aux_text_window::s_backbuffer),
      m_capture(), m_lock_count(0), m_need_repaint(false), m_dirty(false) {}

void text_window::setup(bool capture_screen) {
  if (capture_screen) {
    aux_text_window::read_screen_buffer(&m_capture);
  }
  aux_text_window::set_video_mode(aux_text_window::s_text_mode_color_80_25);
  aux_text_window::set_cursor_style(aux_text_window::cursor_invisible, 0, 7);
  if (capture_screen) {
    // Minimize display gaps
    present_copy(m_capture.data(), m_capture.width(), m_capture.height(),
                 rect(0, 0, m_capture.width(), m_capture.height()));
  }
}

void text_window::teardown() {
  // Leave the last screen state behind when exiting -- assumes compatible
  // display modes!
  aux_text_window::read_screen_buffer(&m_capture);
  aux_text_window::set_video_mode(aux_text_window::s_text_mode_color_80_25);
  present_copy(m_capture.data(), m_capture.width(), m_capture.height(),
               rect(0, 0, m_capture.width(), m_capture.height()));
}

void text_window::present() {
  if (m_dirty) {
    present_copy(m_backbuffer.data(), m_backbuffer.width(),
                 m_backbuffer.height(), m_dirty_bb);
    m_dirty = false;
  }
}

void text_window::lock_repaint() {
  ++m_lock_count;
  assert_margin(m_lock_count, INT8_MAX);
}

void text_window::unlock_repaint() {
  assert(m_lock_count > 0);
  --m_lock_count;
  if (!m_lock_count) {
    if (m_need_repaint) {
      repaint(m_repaint);
      m_need_repaint = false;
    }
  }
}

void text_window::repaint(rect const &r) {
  assert(r.reasonable());

  if (!m_element) {
    return;
  }

  if (m_lock_count == 0) {
#if BUILD_DEBUG
    // Make a hideous background to highlight unpainted areas
    static termel_t const ugly_px =
        termel::from('x', color::hi_cyan, color::hi_magenta);
    for (coord_t y = r.y1; y < r.y2; ++y) {
      termel_t __far *row = m_backbuffer.data() + (y * m_backbuffer.width());
      for (coord_t x = r.x1; x < r.x2; ++x) {
        row[x] = ugly_px;
      }
    }
#endif
    graphics g(&m_backbuffer);
    graphics::subregion_state s;
    g.enter_subregion(point(0, 0), r, &s);
    if (!g.subregion_trivial()) {
      paint_element(&g, m_element);
    }
    // graphics will be discarded, don't need to leave_subregion()
  } else {
    locked_repaint(r);
  }
}

void text_window::add_element(element *e) {
  assert(m_element == NULL);
  m_element = e;
  repaint(e->frame());
}

void text_window::remove_element(element *e) {
  (void)e;
  assert(e == m_element);
  m_element = NULL;
}

void text_window::element_frame_changed(element *e, rect const &old_frame,
                                        coord_t old_z) {
  (void)old_z;
  assert(e == m_element);
  if (old_frame.overlaps(e->frame())) {
    repaint(old_frame | e->frame());
  } else {
    repaint(old_frame);
    repaint(e->frame());
  }
}

void text_window::paint_element(graphics *g, element *e) {
  assert(e->visible());

  graphics::subregion_state s;

  g->enter_subregion(point(e->frame().x1, e->frame().y1), e->frame(), &s);
  if (!g->subregion_trivial()) {
    logf_text_window("text_window paint %p\n", e);
    e->paint(g);
  }
  if (m_dirty) {
    m_dirty_bb |= g->clip();
  } else {
    m_dirty = true;
    m_dirty_bb = g->clip();
  }
  g->leave_subregion(&s);
}

void text_window::locked_repaint(rect const &r) {
  if (m_need_repaint) {
    m_repaint |= r;
  } else {
    m_need_repaint = true;
    m_repaint = r;
  }
}

void text_window::present_copy(termel_t const __far *backbuffer, coord_t width,
                               coord_t height, rect const &r) {
  (void)height;
  assert(r.reasonable());
  assert(r.x1 >= 0);
  assert(r.y1 >= 0);
  assert(r.x2 <= width);
  assert(r.y2 <= height);

  logf_text_window(
      "present_copy region (%d,%d-%d,%d) to %" PRpF ", corner %04X -> %04X\n",
      x1, y1, x2, y2,
      reinterpret_cast<void __far *>(aux_text_window::s_screen_buffer),
      *backbuffer, *aux_text_window::s_screen_buffer);

  coord_t bytes_per_line = r.width() * sizeof(termel_t);
  coord_t lines = r.y2 - r.y1;
  termel_t const __far *source_p = backbuffer + r.y1 * width + r.x1;
  termel_t __far *dest_p =
      aux_text_window::s_screen_buffer + r.y1 * width + r.x1;

  for (coord_t i = 0; i < lines; ++i) {
    _fmemcpy(dest_p, source_p, bytes_per_line);
    dest_p += width;
    source_p += width;
  }
}
