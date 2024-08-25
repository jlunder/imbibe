#include "imbibe.h"

#include "tbm.h"

#include "bitmap.h"
#include "termviz.h"


void tbm::to_bitmap(bitmap & b, tbm_header const * header, uint16_t size) {
  assert(size >= sizeof (*header));
  assert(header->width > 0 && header->height > 0);
  assert(header->magic[0] == 'T'); assert(header->magic[1] == 'B');
  assert(header->magic[2] == 'M'); assert(header->magic[3] == 'a');

  size_t data_size =
    (size_t)header->width * header->height * sizeof (termel_t);
  assert(header->data_size == data_size + 4);
  assert(size - sizeof (*header) >= data_size);

  switch(header->flags & tbm_flags::flags_format) {
  case tbm_flags::fmt_plain:
    b.assign(header->width, header->height,
      (termel_t const *)header->data_start);
    break;
  default:
    assert(!"Unrecognized TBM format");
    b = bitmap(header->width, header->height);
    for (size_t i = 0; i < header->width * header->height; ++i) {
      (b.data())[i] = termel::from('B', color::black, color::yellow, true);
    }
    break;
  }
}


