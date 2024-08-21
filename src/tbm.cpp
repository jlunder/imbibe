#include "imbibe.h"

#include "tbm.h"

#include "bitmap.h"
#include "termviz.h"


bitmap * tbm::to_bitmap(tbm_header const * header) {
  assert(header->width > 0 && header->height > 0);
  assert(header->magic[0] == 'T'); assert(header->magic[1] == 'B');
  assert(header->magic[2] == 'M'); assert(header->magic[3] == 'a');

  bitmap * b = new bitmap(header->width, header->height);
  size_t data_size =
    (size_t)header->width * header->height * sizeof (termel_t);
  assert(header->data_size == data_size + 4);

  switch(header->flags & tbm_flags::flags_format) {
  case tbm_flags::fmt_plain:
    memcpy(b->data(), header->data_start, data_size);
    break;
  default:
    assert(!"Unrecognized TBM format");
  }
  return b;
}


