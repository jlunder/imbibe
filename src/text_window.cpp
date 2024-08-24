#include "imbibe.h"

#include "text_window.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "bitmap.h"
#include "graphics.h"
#include "element.h"


#define logf_text_window(...) logf("WINDOW_ELEMENT: " __VA_ARGS__)


namespace aux_text_window {

  enum {
    cursor_visible = 0x00,
    cursor_invisible = 0x20,
  };

  extern uint8_t get_video_mode();
  extern void set_video_mode(uint8_t mode);
  extern void set_cursor_style(uint8_t visible, uint8_t start_row,
    uint8_t end_row);

}


#if !defined(SIMULATE)

/*
uint8_t cols;
uint8_t mode;
uint8_t page;
uint8_t pos_row;
uint8_t pos_col;
extern "C" void bios_get_video_details();
#pragma aux bios_get_video_details = \
  "   mov ah, 00Fh              " \
  "   push    bp                " \
  "   int     10h               " \
  "   pop     bp                " \
  "   mov     cols, ah          " \
  "   mov     mode, al          " \
  "   mov     page, bh          " \
  "   mov     ah, 003h          " \
  "   push    bp                " \
  "   int     010h              " \
  "   pop     bp                " \
  "   mov     pos_row, dh       " \
  "   mov     pos_col, dl       " \
  modify [ax bx cx dx];
bios_get_video_details();
return mode;
*/

extern void asm_bios_set_video_mode(uint8_t mode);
#pragma aux asm_bios_set_video_mode = \
  "   push    bp                " \
  "   mov ah, 000h              " \
  "   int     10h               " \
  "   pop     bp                " \
  parm [al] \
  modify [ax] nomemory;

extern void asm_bios_set_cursor_style(uint8_t start_opts, uint8_t end);
#pragma aux asm_bios_set_cursor_style = \
  "   push    bp                " \
  "   mov ah, 001h              " \
  "   int     10h               " \
  "   pop     bp                " \
  parm [ch] [cl] \
  modify [ax] nomemory;

inline uint8_t aux_text_window::get_video_mode() {
  return 3;
}

inline void aux_text_window::set_video_mode(uint8_t mode) {
  assert(mode == 3);
  asm_bios_set_video_mode(mode);
}

inline void aux_text_window::set_cursor_style(uint8_t visible,
    uint8_t start_row, uint8_t end_row) {
  assert((visible == cursor_visible) || (visible == cursor_invisible));
  assert(start_row <= end_row); assert(end_row <= 7);
  asm_bios_set_cursor_style(visible | start_row, end_row);
}

#else

inline uint8_t aux_text_window::get_video_mode() {
  return 0x03;
}

inline void aux_text_window::set_video_mode(uint8_t mode) {
  assert(mode == 3);
}

inline void aux_text_window::set_cursor_style(uint8_t visible,
    uint8_t start_row, uint8_t end_row) {
  assert((visible == cursor_visible) || (visible == cursor_invisible));
  assert(start_row <= end_row); assert(end_row <= 7);
}

#endif


text_window text_window_instance;


text_window::text_window()
  : window(), m_element(NULL), m_focus(NULL), m_backbuffer(80, 25),
    m_lock_count(0), m_need_repaint(false) {
}


text_window::~text_window() {
}


void text_window::setup() {
  aux_text_window::set_video_mode(3);
  aux_text_window::set_cursor_style(aux_text_window::cursor_invisible, 0, 7);
}


void text_window::teardown() {
  aux_text_window::set_video_mode(3);
}


void text_window::present() {
  if (m_dirty){
    present_copy(m_backbuffer.data(), m_backbuffer.width(),
      m_backbuffer.height(), m_dirty_bb_x1, m_dirty_bb_y1, m_dirty_bb_x2,
      m_dirty_bb_y2);
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
  if (!m_lock_count)
  {
    if (m_need_repaint) {
      repaint(m_repaint_x1, m_repaint_y1, m_repaint_x2, m_repaint_y2);
      m_need_repaint = false;
    }
  }
}


void text_window::repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
  assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);
  assert_margin(x2, COORD_MAX); assert_margin(y2, COORD_MAX);

  if (!m_element) {
    return;
  }

  if (m_lock_count == 0) {
#ifndef NDEBUG
    // Make a hideous background to highlight unpainted areas
    static termel_t const ugly_px =
      termel::from('x', color::hi_cyan, color::hi_magenta);
    for (coord_t y = y1; y < y2; ++y) {
      termel_t * row = m_backbuffer.data() + (y * m_backbuffer.width());
      for (coord_t x = x1; x < x2; ++x) {
        row[x] = ugly_px;
      }
    }
#endif
    graphics g(m_backbuffer);
    graphics::subregion_state s;
    g.enter_subregion(s, 0, 0, x1, y1, x2, y2);
    if (!g.subregion_trivial()) {
      paint_element(g, *m_element);
    }
    // graphics will be discarded, don't need to leave_subregion()
  } else {
    locked_repaint(x1, y1, x2, y2);
  }
}


void text_window::add_element(element & e) {
  assert(m_element == NULL);
  m_element = &e;
  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
}


void text_window::remove_element(element & e) {
  (void)e;
  assert(&e == m_element);
  m_element = NULL;
}


void text_window::element_frame_changed(element & e, coord_t old_x1,
    coord_t old_y1, coord_t old_x2, coord_t old_y2, coord_t old_z)
{
  (void)old_z;
  assert(&e == m_element);
  repaint(min(old_x1, e.frame_x1()), min(old_y1, e.frame_y1()),
    max(old_x2, e.frame_x2()), max(old_y2, e.frame_y2()));
}


bool text_window::is_element() {
  return false;
}


element & text_window::as_element() {
  assert(!"not valid");
  return *(element *)NULL;
}


void text_window::set_focus(element & e) {
  m_focus = &e;
}


void text_window::clear_focus() {
  m_focus = NULL;
}


bool text_window::has_focus() {
  return !!m_focus;
}


element & text_window::focus() {
  return *m_focus;
}


void text_window::paint_element(graphics & g, element & e) {
  assert(e.visible());

  graphics::subregion_state s;

  g.enter_subregion(s, e.frame_x1(), e.frame_y1(), e.frame_x1(),
    e.frame_y1(), e.frame_x2(), e.frame_y2());
  if (!g.subregion_trivial()) {
    logf_text_window("text_window paint %p\n", &e);
    e.paint(g);
  }
  if (m_dirty) {
    m_dirty_bb_x1 = min(m_dirty_bb_x1, g.clip_x1());
    m_dirty_bb_y1 = min(m_dirty_bb_y1, g.clip_y1());
    m_dirty_bb_x2 = max(m_dirty_bb_x2, g.clip_x2());
    m_dirty_bb_y2 = max(m_dirty_bb_y2, g.clip_y2());
  } else {
    m_dirty = true;
    m_dirty_bb_x1 = g.clip_x1();
    m_dirty_bb_y1 = g.clip_y1();
    m_dirty_bb_x2 = g.clip_x2();
    m_dirty_bb_y2 = g.clip_y2();
  }
  g.leave_subregion(s);
}


void text_window::locked_repaint(coord_t x1, coord_t y1, coord_t x2,
    coord_t y2) {
  if (m_need_repaint) {
    m_repaint_x1 = min(m_repaint_x1, x1);
    m_repaint_y1 = min(m_repaint_y1, y1);
    m_repaint_x2 = max(m_repaint_x2, x2);
    m_repaint_y2 = max(m_repaint_y2, y2);
  } else {
    m_need_repaint = true;
    m_repaint_x1 = x1;
    m_repaint_y1 = y1;
    m_repaint_x2 = x2;
    m_repaint_y2 = y2;
  }
}


void text_window::present_copy(termel_t const * backbuffer, coord_t width,
    coord_t height, coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
  assert(x1 <= x2); assert(y1 <= y2); assert(x1 >= 0); assert(y1 >= 0);
  assert(x2 <= width); assert(y2 <= height);

  termel_t * screen_buffer = (termel_t *)MK_FP(0xB800, (void *)0);

  logf_text_window(
    "present_copy region (%d,%d-%d,%d) to %p, corner %04X -> %04X\n",
    x1, y1, x2, y2, (void *)screen_buffer, *backbuffer, *screen_buffer);

  coord_t i;
  coord_t bytes_per_line = (x2 - x1) * sizeof (termel_t);
  coord_t lines = min(y2, height) - min(y1, height);
  uint8_t const * source_p = (uint8_t const *)(backbuffer + y1 * width + x1);
  uint8_t * dest_p = (uint8_t *)(screen_buffer + y1 * width + x1);
  uint16_t stride = width * sizeof (termel_t);

  for (i = 0; i < lines; ++i) {
    memcpy(dest_p, source_p, bytes_per_line);
    dest_p += stride;
    source_p += stride;
  }
}


