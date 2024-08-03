#include "imbibe.hh"

#include "bin_bitmap.hh"

#include <iostream.h>

#include "bitmap.hh"

#include "bin_bitmap.ii"

#include "bitmap.ii"


bin_bitmap::bin_bitmap(int n_width, int n_height, istream & i):
  bitmap(n_width, n_height)
{
  i.read((unsigned char *)data(), width() * height() * sizeof(unsigned short));
}


