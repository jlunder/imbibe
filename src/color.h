#ifndef __COLOR_H_INCLUDED
#define __COLOR_H_INCLUDED


#include "imbibe.h"


class color;


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

  color(color const & n_color);
  color(uint8_t n_fore = white, uint8_t n_back = black);
  void value(uint8_t n_value);
  uint8_t value() const;
  void foreground(uint8_t n_fore);
  void background(uint8_t n_back);

private:
  uint8_t m_value;
};


inline color::color(color const & n_color):
  m_value(n_color.m_value)
{
}


inline color::color(uint8_t n_fore, uint8_t n_back):
  m_value(n_fore | (n_back << 4))
{
}


inline void color::value(uint8_t n_value)
{
  m_value = n_value;
}


inline uint8_t color::value() const
{
  return m_value;
}


inline void color::foreground(uint8_t n_fore)
{
  m_value = (m_value & 0xF0) | n_fore;
}


inline void color::background(uint8_t n_back)
{
  m_value = (m_value & 0x0F) | (n_back << 4);
}


#endif //__COLOR_H_INCLUDED


