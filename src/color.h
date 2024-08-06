#ifndef __COLOR_H_INCLUDED
#define __COLOR_H_INCLUDED


#include "imbibe.h"


class color
{
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
    hi_white   = 0x0F
  };

  color() { }
  color(color const & n_color): m_value(n_color.m_value) { }
  color(uint8_t n_fore, uint8_t n_back): m_value(n_fore | (n_back << 4)) { }

  void value(uint8_t n_value) { m_value = n_value; }
  uint8_t value() const { return m_value; }
  void foreground(uint8_t n_fore) { m_value = (m_value & 0xF0) | n_fore; }
  void background(uint8_t n_back)
    { m_value = (m_value & 0x0F) | (n_back << 4); }

  color & operator = (color const & other)
    { m_value = other.m_value; return *this; }

private:
  uint8_t m_value;
};


#endif // __COLOR_H_INCLUDED


