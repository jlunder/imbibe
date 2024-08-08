#include "imbibe.h"

#include "element.h"

#include "window.h"


void element::set_frame_pos(int16_t x1, int16_t y1) {
  assert_margin(x1, INT16_MAX); assert_margin(y1, INT16_MAX);
  int16_t old_x1 = m_x1;
  int16_t old_y1 = m_y1;

  m_x2 = x1 + frame_width();
  m_y2 = y1 + frame_height();
  m_x1 = x1;
  m_y1 = y1;
  if(m_visible) {
    m_owner->element_frame_pos_changed(*this, old_x1, old_y1);
  }
}


void element::set_frame_size(int16_t width, int16_t height) {
  assert_margin(width, INT16_MAX); assert_margin(height, INT16_MAX);
  int16_t old_width = frame_width();
  int16_t old_height = frame_height();

  m_x2 = m_x1 + width;
  m_y2 = m_y1 + height;
  assert_margin(m_x2, INT16_MAX); assert_margin(m_y2, INT16_MAX);
  if(m_visible) {
    m_owner->element_frame_size_changed(*this, old_width, old_height);
  }
}


void element::set_frame_depth(int16_t z) {
  int16_t old_z = m_z;

  m_z = z;
  if(m_visible)
  {
    m_owner->element_frame_depth_changed(*this, old_z);
  }
}


void element::set_frame(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    int16_t z) {
  assert_margin(x1, INT16_MAX); assert_margin(y1, INT16_MAX);
  assert_margin(x2, INT16_MAX); assert_margin(y2, INT16_MAX);
  assert_margin(z, INT16_MAX);
  int16_t old_x1 = m_x1;
  int16_t old_y1 = m_y1;
  int16_t old_x2 = m_x2;
  int16_t old_y2 = m_y2;
  int16_t old_z = m_z;

  m_x1 = x1;
  m_y1 = y1;
  m_x2 = x2;
  m_y2 = y2;
  m_z = z;
  if(m_visible) {
    m_owner->element_frame_changed(*this, old_x1, old_y1, old_x2, old_y2,
      old_z);
  }
}


void element::set_owner(window & n_owner) {
  if (m_visible && m_owner) {
    m_owner->remove_element(*this);
  }
  m_owner = &n_owner;
  if (m_visible && m_owner) {
    m_owner->add_element(*this);
  }
}


void element::show() {
  if(!m_visible && m_owner) {
    m_owner->add_element(*this);
  }
  m_visible = true;
}


void element::hide() {
  if(m_visible && m_owner) {
    m_owner->remove_element(*this);
  }
  m_visible = false;
}


void element::repaint() {
  repaint(0, 0, frame_width(), frame_height());
}


void element::repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  if(m_visible) {
    m_owner->repaint(x1+m_x1, y1+m_y1, x2+m_x1, y2+m_y1, m_z);
  }
}


