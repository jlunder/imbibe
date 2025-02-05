#ifndef __IFF_H_INCLUDED
#define __IFF_H_INCLUDED

#include "imbibe.h"

#include "unpacker.h"

#define FOURCC(s)                                                              \
  ((uint32_t)(s)[0] | ((uint32_t)(s)[1] << 8) | ((uint32_t)(s)[2] << 16) |     \
   ((uint32_t)(s)[3] << 24))

wcc_packed struct gcc_packed iff_header {
  union {
    char magic_s[4];
    uint32_t magic_i;
  };
  uint32_t data_size;
};

static_assert(sizeof (iff_header) == 8);

namespace iff {

bool try_expect_magic(unpacker *data, uint32_t expected_magic,
                      uint32_t *out_data_size);
bool try_expect_magic(unpacker *data, uint32_t expected_magic,
                      uint32_t expected_data_size);

} // namespace iff

#endif // __IFF_H_INCLUDED
