#include "imbibe.h"

//#include "bin_bitmap.h"
#include "bin_bitm.h"

#include <iostream.h>

#include "bitmap.h"


bin_bitmap::bin_bitmap(int n_width, int n_height, istream & i):
  bitmap(n_width, n_height)
{
  i.read((uint8_t *)data(), width() * height() * sizeof(uint16_t));
}


