#include "imbibe.h"

#include "data.h"

//uint8_t test_pkg_data[test_pkg_length] = { 4, 0, 0, 0 };

uint8_t const inline_data::data[432] = {
  0x54, 0x42, 0x4D, 0x61, 0xA8, 0x01, 0x00, 0x00, 0x1E, 0x07, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xDC, 0x02, 0x00, 0x00, 0xDE, 0x02, 0xDD, 0x02, 0x00, 0x00, 0xDC, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xDF, 0x02, 0xB2, 0x02, 0xDC, 0x02, 0xDF, 0x02, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x02,
  0xDF, 0x02, 0xDF, 0x02, 0xDF, 0x02, 0xDC, 0x2C, 0xDC, 0x0C, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xDF, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE, 0x05, 0xDD, 0x05, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xDE, 0x0C, 0xB2, 0x4C, 0xDB, 0x0C, 0xDD, 0x0C,
  0x00, 0x00, 0x00, 0x00, 0xB0, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xDC, 0x05, 0xDC, 0x05, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xB0, 0x5D, 0x00, 0x00, 0xDB, 0x05, 0xFE, 0x05, 0xDF, 0x05,
  0xDC, 0x05, 0xDF, 0x05, 0xDD, 0x05, 0xDE, 0x05, 0xDD, 0x05, 0xDF, 0x05,
  0xDF, 0x05, 0xDC, 0x05, 0x00, 0x00, 0xDF, 0x0C, 0xB2, 0x4C, 0xDB, 0x0C,
  0x00, 0x00, 0x00, 0x00, 0xDB, 0x05, 0xFE, 0x05, 0xDF, 0x05, 0xDC, 0x05,
  0x00, 0x00, 0xB0, 0x5D, 0xDC, 0x05, 0xDC, 0x05, 0xDB, 0x05, 0x00, 0x00,
  0x00, 0x00, 0xDE, 0x05, 0xDD, 0x05, 0xDE, 0x05, 0xDD, 0x05, 0x00, 0x00,
  0xDD, 0x05, 0x00, 0x00, 0xB0, 0x54, 0x20, 0x04, 0xB0, 0x54, 0xDC, 0x05,
  0xDC, 0x05, 0xDF, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDF, 0x4C,
  0xDD, 0x0C, 0x00, 0x00, 0xDE, 0x05, 0xDC, 0x05, 0xDC, 0x05, 0xDF, 0x05,
  0x00, 0x00, 0xDF, 0x05, 0xDC, 0x05, 0xDC, 0x05, 0xDC, 0x05, 0xFE, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xDF, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
