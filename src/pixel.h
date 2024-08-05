#ifndef __PIXEL_H_INCLUDED
#define __PIXEL_H_INCLUDED


#include "imbibe.h"


class pixel;


#include "color.h"


class pixel
{
public:
  pixel();
  pixel(pixel const & n_pixel);
  pixel(char n_character);
  pixel(char n_character, color n_attribute);
  pixel(uint16_t us);
  void character(char n_character);
  char character() const;
  void attribute(color n_attribute);
  color attribute() const;
  operator uint16_t();

private:
  char m_character;
  color m_attribute;
};


inline pixel::pixel()
{
}


inline pixel::pixel(pixel const & n_pixel):
  m_character(n_pixel.m_character), m_attribute(n_pixel.m_attribute)
{
}


inline pixel::pixel(char n_character):
  m_character(n_character), m_attribute(color::white, color::black)
{
}


inline pixel::pixel(char n_character, color n_attribute):
  m_character(n_character), m_attribute(n_attribute)
{
}


inline pixel::pixel(uint16_t us):
  m_character(us & 0xFF), m_attribute((us & 0xF00) >> 8, (us & 0xF000) >> 12)
{
}


inline void pixel::character(char n_character)
{
  m_character = n_character;
}


inline char pixel::character() const
{
  return m_character;
}


inline void pixel::attribute(color n_attribute)
{
  m_attribute = n_attribute;
}


inline color pixel::attribute() const
{
  return m_attribute;
}


inline pixel::operator uint16_t()
{
  return (uint16_t)m_character | ((uint16_t)m_attribute.value() << 8);
}


#endif //__PIXEL_H_INCLUDED


