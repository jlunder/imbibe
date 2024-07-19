#ifndef __PIXEL_HH_INCLUDED
#define __PIXEL_HH_INCLUDED


class pixel;


#include "color.hh"


class pixel
{
public:
  pixel();
  pixel(pixel const & n_pixel);
  pixel(char n_character);
  pixel(char n_character, color n_attribute);
  pixel(unsigned short us);
  void character(char n_character);
  char character() const;
  void attribute(color n_attribute);
  color attribute() const;
  operator unsigned short();

private:
  char m_character;
  color m_attribute;
};


//#include "pixel.ii"


#endif //__PIXEL_HH_INCLUDED


