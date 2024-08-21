#ifndef __IMBIBE_H_INCLUDED
#define __IMBIBE_H_INCLUDED


#if defined(__WATCOMC__)
#define __STDC_LIMIT_MACROS
#pragma warning 549 9
#endif


#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>


#define assert_margin(x, mag) assert(((x) >= -((mag) / 4)) && ((x) <= (mag) / 4))


#if !(defined(M_I86) && defined(__WATCOMC__))

// This is to make the editor happy when it tries to parse our code thinking
// we have some kind of normal compiler

#define __far

#define _Packed
#define __packed__ __attribute__((packed))

#define __based(o)
#define __segment uintptr_t
#define __segname(n)
// __segname gets the named segment -- probably "_CODE", "_CONST", "_DATA"
#define __self

#define FP_SEG(p) ((uintptr_t)(p))
#define FP_OFF(p) 0
#define MK_FP(s, o) \
  ((uintptr_t)s == 0xB800 ? (void *)dummy_screen : (void *)(s))

extern uint16_t dummy_screen[16384];

#define __interrupt

#include <stdio.h>

#define cprintf(...) fprintf(stderr, __VA_ARGS__)

extern void _dos_setvect(int, void (*)());
extern void (*_dos_getvect(int))();
extern void _chain_intr(void (*)());

inline void * operator new (size_t size, void * p) { (void)size; return p; }

extern void step_simulator();

#define SIMULATE

#else

// This part is for real

#include <i86.h>
#include <conio.h>
#include <dos.h>

#define __packed__

#endif


#define LENGTHOF(a) (sizeof (a) / sizeof (a[0]))

#define logf(...) cprintf(__VA_ARGS__)
#define disable_logf(...) do {} while(false)

template<class T>
inline T min(T x, T y) { return (x < y) ? x : y; }

template<class T>
inline T min(T x, T y, T z) { return min(min(x, y), z); }

template<class T>
inline T max(T x, T y) { return (x > y) ? x : y; }

template<class T>
inline T max(T x, T y, T z) { return max(max(x, y), z); }


#endif // __IMBIBE_H_INCLUDED

