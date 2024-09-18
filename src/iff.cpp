#include "imbibe.h"

#include "iff.h"

bool iff::try_expect_magic(unpacker *data, uint32_t expected_magic,
                           uint32_t *out_data_size) {
  if (!data->fits<iff_header>()) {
    return false;
  }
  iff_header const __far &h = data->unpack<iff_header>();
  uint32_t data_size = h.data_size;
  if ((data_size > SEGSIZE_MAX / 2) ||
      !data->fits_untyped((segsize_t)data_size) ||
      (h.magic_i != expected_magic)) {
    return false;
  }
  if (out_data_size) {
    *out_data_size = data_size;
  }
  return true;
}

bool iff::try_expect_magic(unpacker *data, uint32_t expected_magic,
                           uint32_t expected_data_size) {
  assert(expected_data_size <= SEGSIZE_MAX / 2);
  if (!data->fits<iff_header>()) {
    return false;
  }
  iff_header const __far &h = data->unpack<iff_header>();
  uint32_t data_size = h.data_size;
  return (h.magic_i == expected_magic) && (data_size == expected_data_size) &&
         (data_size <= SEGSIZE_MAX / 2) && data->fits_untyped((segsize_t)data_size);
}
