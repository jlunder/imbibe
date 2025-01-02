#ifndef __CRC32_H_INCLUDED
#define __CRC32_H_INCLUDED

#include "imbibe.h"

// A check value that's HD4 up to 511MB -- pretty good for large blocks
uint32_t crc32_buf_hd4_c9d204f5(void const __far *s, segsize_t size);

// A check value that's HD5 up to just under 8KB -- better for small blocks
uint32_t crc32_buf_hd5_d419cc15(void const __far *s, segsize_t size);

// A very widely used CRC check value
uint32_t crc32_buf_hd3_ieee_82608edb(void const __far *s, segsize_t size);

#endif // __CRC32_H_INCLUDED
