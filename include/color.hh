#ifndef __COLOR_HH_INCLUDED
#define __COLOR_HH_INCLUDED


#include "imbibe.hh"


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
  color(unsigned char n_fore = white, unsigned char n_back = black);
  void value(unsigned char n_value);
  unsigned char value() const;
  void foreground(unsigned char n_fore);
  void background(unsigned char n_back);

private:
  unsigned char m_value;
};


//#include "color.ii"


#endif //__COLOR_HH_INCLUDED


