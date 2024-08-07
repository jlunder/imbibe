#ifndef __PIXEL_H_INCLUDED
#define __PIXEL_H_INCLUDED


#include "imbibe.h"

#include "color.h"


class pixel
{
public:
  pixel() { }
  pixel(pixel const & n_pixel):
    m_character(n_pixel.m_character), m_attribute(n_pixel.m_attribute) { }
  pixel(char n_character):
    m_character(n_character), m_attribute(color::white, color::black) { }
  pixel(char n_character, color n_attribute):
    m_character(n_character), m_attribute(n_attribute) { }
  pixel(uint16_t us)
    : m_character((char)(us & 0xFF)),
      m_attribute((uint8_t)((us & 0xF00) >> 8),
        (uint8_t)((us & 0xF000) >> 12))
    { }

  void character(char n_character) { m_character = n_character; }
  char character() const { return m_character; }
  void attribute(color n_attribute)
    { m_attribute = n_attribute; }
  color attribute() const { return m_attribute; }
  operator uint16_t() {
    return (uint16_t)m_character | ((uint16_t)m_attribute.value() << 8);
  }

private:
  char m_character;
  color m_attribute;
};


#endif // __PIXEL_H_INCLUDED


