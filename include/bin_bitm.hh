#ifndef __BIN_BITMAP_HH_INCLUDED
#define __BIN_BITMAP_HH_INCLUDED


#include <iostream.h>

#include "bitmap.hh"


class bin_bitmap: public bitmap
{
public:
  bin_bitmap(int n_width, int n_height, istream & i);
};


#endif //__BIN_BITMAP_HH_INCLUDED


