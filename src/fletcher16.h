#ifndef __FLETCHER16_H_INCLUDED
#define __FLETCHER16_H_INCLUDED

#include "imbibe.h"

// For when you need a hash or check value that's mostly alright, and really
// fast to compute

// This seed is for when you have data which (a) could plausibly be corrupted
// or otherwise distinguished by shifting and (b) likely to have leading zeros
static uint8_t const fletcher16_seed = 0x39;

uint16_t fletcher16_buf(void const __far *s, segsize_t size, uint8_t seed = 0);
uint16_t fletcher16_str(char const __far *s);

#endif // __FLETCHER16_H_INCLUDED
