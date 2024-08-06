#include "imbibe.h"

// #include "text_window.h"
#include "text_win.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "bitmap.h"
// #include "bitmap_graphics.h"
#include "bitmap_g.h"
#include "element.h"


text_window::text_window()
  : m_backbuffer(80, 25), m_locked(false) {
}


text_window::~text_window() {
}


void text_window::setup() {
  save_mode();
  set_text_mode();
}


void text_window::teardown() {
  restore_mode();
}


void text_window::lock() {
  ++m_locked;
}


void text_window::unlock() {
  assert(m_locked);
  --m_locked;
  if(!m_locked)
  {
    if(m_need_repaint) {
      if(m_repaint_z_minus_infinity) {
        repaint(m_repaint_x1, m_repaint_y1, m_repaint_x2, m_repaint_y2);
      } else {
        repaint(m_repaint_x1, m_repaint_y1, m_repaint_x2, m_repaint_y2, m_repaint_z);
      }
      m_need_repaint = false;
    }
  }
}


void text_window::repaint() {
  repaint(0, 0, m_backbuffer.width(), m_backbuffer.height());
}


void text_window::repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  assert(x1 >= 0); assert(x1 <= m_backbuffer.width());
  assert(y1 >= 0); assert(y1 <= m_backbuffer.width());
  assert(x2 >= x1); assert(x2 <= m_backbuffer.width());
  assert(y2 >= y1); assert(y2 <= m_backbuffer.width());

#ifndef NDEBUG
  // Make a
  static uint16_t const ugly_px =
    pixel('x', color(color::hi_cyan, color::hi_magenta));
  for(int16_t y = y1; y < y2; ++y) {
    uint16_t * row = m_backbuffer.data() + (y * m_backbuffer.width());
    for(int16_t x = x1; x < x2; ++x) {
      row[x] = ugly_px;
    }
  }
#endif

  if(!m_locked) {
    for(element_list_iterator i = m_elements.begin(); i != m_elements.end();
        ++i) {
      repaint_element(*i->ref, x1, y1, x2, y2);
    }
    flip(&m_backbuffer);
  } else {
    locked_repaint(x1, y1, x2, y2);
  }
}


void text_window::repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    int16_t z) {
  element_list_iterator i;

  if(!m_locked) {
    for(i = m_elements.lower_bound(z); i != m_elements.end(); ++i) {
      repaint_element(*i->ref, x1, y1, x2, y2);
    }
    flip(&m_backbuffer);
  } else {
    locked_repaint(x1, y1, x2, y2, z);
  }
}


void text_window::add_element(element & e) {
  m_elements.insert(element_list_value(e.frame_z(), &e));
  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2(), e.frame_z());
}


void text_window::remove_element(element & e) {
  element_list_iterator i = m_elements.begin();
  while((i != m_elements.end()) && (i->ref != &e)) {
    ++i;
  }
  m_elements.erase(i);

  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
}


void text_window::element_frame_pos_changed(element & e, int16_t old_x1,
    int16_t old_y1) {
  int16_t x1;
  int16_t y1;
  int16_t x2;
  int16_t y2;

  if(old_x1 < e.frame_x1()) {
    x1 = old_x1;
    x2 = e.frame_x2();
  } else {
    x1 = e.frame_x1();
    x2 = e.frame_x2() - e.frame_x1() + old_x1;
  }
  if(old_y1 < e.frame_y1()) {
    y1 = old_y1;
    y2 = e.frame_y2();
  } else {
    y1 = e.frame_y1();
    y2 = e.frame_y2() - e.frame_y1() + old_y1;
  }
  repaint(x1, y1, x2, y2);
}


void text_window::element_frame_size_changed(element & e, int16_t old_width,
    int16_t old_height)
{
  int16_t x1 = e.frame_x1();
  int16_t y1 = e.frame_y1();
  int16_t x2 = (old_width < e.frame_x2() - e.frame_x1())
    ? e.frame_x2() : e.frame_x1() + old_width;
  int16_t y2 = (old_height < e.frame_y2() - e.frame_y1())
    ? e.frame_y2() : e.frame_y1() + old_height;

  repaint(x1, y1, x2, y2);
}


void text_window::element_frame_depth_changed(element & e, int16_t old_z) {
  element_list_iterator i = m_elements.begin();
  while((i != m_elements.end()) && (i->ref != &e)) {
    ++i;
  }
  m_elements.erase(i);
  m_elements.insert(element_list_value(e.frame_z(), &e));
  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2(), e.frame_z());
}


void text_window::element_frame_changed(element & e, int16_t old_x1,
    int16_t old_y1, int16_t old_x2, int16_t old_y2, int16_t old_z)
{
  int16_t x1 = (old_x1 < e.frame_x1()) ? old_x1 : e.frame_x1();
  int16_t y1 = (old_y1 < e.frame_y1()) ? old_y1 : e.frame_y1();
  int16_t x2 = (old_x2 > e.frame_x2()) ? old_x2 : e.frame_x2();
  int16_t y2 = (old_y2 > e.frame_y2()) ? old_y2 : e.frame_y2();

  if(e.frame_z() != old_z)
  {
    element_list_iterator i = m_elements.begin();
    while((i != m_elements.end()) && (i->ref != &e)) {
      ++i;
    }
    m_elements.erase(i);
    m_elements.insert(element_list_value(e.frame_z(), &e));
  }
  repaint(x1, y1, x2, y2);
}


void text_window::repaint_element(element const & e, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2) {
  if((x1 < e.frame_x2()) && (x2 > e.frame_x1()) && (y1 < e.frame_y2()) && (y2 > e.frame_y1())) {
    int16_t t_x1 = (x1 > e.frame_x1()) ? x1 : e.frame_x1();
    int16_t t_y1 = (y1 > e.frame_y1()) ? y1 : e.frame_y1();
    int16_t t_x2 = (x2 < e.frame_x2()) ? x2 : e.frame_x2();
    int16_t t_y2 = (y2 < e.frame_y2()) ? y2 : e.frame_y2();
    bitmap_graphics g(m_backbuffer);

    if(t_x1 < 0) {
      t_x1 = 0;
    }
    if(t_y1 < 0) {
      t_y1 = 0;
    }
    if(t_x2 > m_backbuffer.width()) {
      t_x2 = m_backbuffer.width();
    }
    if(t_y2 > m_backbuffer.height()) {
      t_y2 = m_backbuffer.height();
    }

    g.set_bounds(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
    g.set_clip(t_x1, t_y1, t_x2, t_y2);
    e.paint(g);
  }
}


void text_window::locked_repaint(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2) {
  if(m_need_repaint) {
    if(x1 < m_repaint_x1) m_repaint_x1 = x1;
    if(y1 < m_repaint_y1) m_repaint_y1 = y1;
    if(x2 > m_repaint_x2) m_repaint_x2 = x2;
    if(y2 > m_repaint_y2) m_repaint_y2 = y2;
    m_repaint_z_minus_infinity = true;
  } else {
    m_need_repaint = true;
    m_repaint_x1 = x1;
    m_repaint_y1 = y1;
    m_repaint_x2 = x2;
    m_repaint_y2 = y2;
    m_repaint_z_minus_infinity = true;
  }
}


void text_window::locked_repaint(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2, int16_t z) {
  if(m_need_repaint) {
    if(x1 < m_repaint_x1) m_repaint_x1 = x1;
    if(y1 < m_repaint_y1) m_repaint_y1 = y1;
    if(x2 > m_repaint_x2) m_repaint_x2 = x2;
    if(y2 > m_repaint_y2) m_repaint_y2 = y2;
    if(z < m_repaint_z) m_repaint_z = z;
  } else {
    m_need_repaint = true;
    m_repaint_x1 = x1;
    m_repaint_y1 = y1;
    m_repaint_x2 = x2;
    m_repaint_y2 = y2;
    m_repaint_z = z;
    m_repaint_z_minus_infinity = false;
  }
}


extern void set_text_asm();
#pragma aux set_text_asm = "mov ax, 00003h" "int 010h" modify exact [ax] nomemory


extern void restore_text_asm();
#pragma aux restore_text_asm = "mov ax, 00003h" "int 010h" modify exact [ax] nomemory


void text_window::save_mode() {
}


void text_window::restore_mode() {
  restore_text_asm();
}


void text_window::set_text_mode() {
  set_text_asm();
}


void text_window::flip(bitmap * backbuffer) {
  memcpy(MK_FP(0xB800, (void *)0), backbuffer->data(),
    backbuffer->width() * backbuffer->height() * sizeof(uint16_t));
}


