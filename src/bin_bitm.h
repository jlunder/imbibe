#ifndef __BIN_BITMAP_H_INCLUDED
#define __BIN_BITMAP_H_INCLUDED


#include <iostream.h>

#include "imbibe.h"

#include "bitmap.h"


class bin_bitmap: public bitmap
{
public:
  bin_bitmap(int n_width, int n_height, istream & i);
};


#endif //__BIN_BITMAP_H_INCLUDED


