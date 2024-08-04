#include "imbibe.hh"

#include "bin_bitmap.hh"

#include <iostream.h>

#include "bitmap.hh"


bin_bitmap::bin_bitmap(int n_width, int n_height, istream & i):
  bitmap(n_width, n_height)
{
  i.read((uint8_t *)data(), width() * height() * sizeof(uint16_t));
}


