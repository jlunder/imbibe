#ifndef __BASE_H_INCLUDED
#define __BASE_H_INCLUDED

#if defined(__WATCOMC__)
#define __STDC_LIMIT_MACROS
#pragma warning 549 9
#pragma warning 446 9
#pragma warning 14 9
#endif

#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>

#if (defined(M_I86) && !defined(MSDOS)) || (!defined(M_I86) && defined(MSDOS))
#error Inconsistent (unsupported) platform macros defined!
#endif

#if !defined(M_I86) || !defined(MSDOS)

#define __near
#define __far volatile
// #define __far

#define _Packed
#define __packed__ __attribute__((packed))

#define __based(o)
#define __segment uintptr_t
#define __segname(n)
// __segname gets the named segment -- probably "_CODE", "_CONST", "_DATA"
#define __self

/*
#define FP_SEG(p) (((uintptr_t)(p) & ~0xFFF) >> 4)
#define FP_OFF(p) ((uintptr_t)(p) & 0xFFF)
#define MK_FP(s, o) \
  ((uintptr_t)s == 0xB800 ? (void *)dummy_screen \
    : (void *)(((uintptr_t)(s) << 4) + (uintptr_t)(o)))
*/
#define FP_SEG(p) ((uintptr_t)(p) & ~0xFLLU)
#define FP_OFF(p) ((uintptr_t)(p) & 0xFLLU)
#define MK_FP(s, o)                                                            \
  ((uintptr_t)(s) == 0xB800 ? (void *)(sim::dummy_screen)                      \
                            : (void *)((uintptr_t)(s) + (uintptr_t)(o)))

#define PRpF "%p"
#define PRpN "%p"
#define PRp "%p"

namespace sim {
extern uint16_t dummy_screen[16384];
}

#define __interrupt

#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <new>

#define cprintf(...) fprintf(stderr, __VA_ARGS__)

inline void failsafe_textmode() {}

extern void _dos_setvect(int, void (*)());
extern void (*_dos_getvect(int))();
extern void _chain_intr(void (*)());

extern unsigned _dos_open(char const __far *path, unsigned mode,
                          int __far *handle);
extern unsigned _dos_close(int handle);
extern unsigned _dos_read(int handle, void __far *buf, unsigned count,
                          unsigned __far *bytes);
extern unsigned _dos_lseek(int handle, long offset, int whence,
                           unsigned long __far *where);

extern void __far *_fmalloc(size_t size);
extern void __far *_fexpand(void __far *p, size_t size);
extern void _ffree(void __far *p);
extern int _fstrcmp(char const __far *x, char const __far *y);
extern size_t _fstrlen(char const __far *s);
extern void _fmemcpy(void __far *dest, void const __far *src, size_t size);

#define SIMULATE

#else

#include <conio.h>
#include <dos.h>
#include <i86.h>

extern void failsafe_textmode();

#pragma aux failsafe_textmode = "mov ax, 03h"                                  \
                                "int 010h" modify[ax] nomemory

// seemingly missing from dos.h??
extern unsigned _dos_lseek(int handle, long offset, int whence,
                           unsigned long __far *where);

#define __packed__

#define PRpF "%Fp"
#define PRpN "%Np"
#define PRp "%p"

#endif

// #define __static_assert(con, id) static int assert_ ## id [2 * !!(con) - 1];
// #define _static_assert(con, id) __static_assert(con, id)
// #define static_assert(con) _static_assert(con, __COUNTER__)

template <bool> struct base_static_assert;
template <> struct base_static_assert<true> {};

#define static_assert(e)                                                       \
  struct __the_sa {                                                            \
    ::base_static_assert<!!(e)> sa;                                            \
  }

static_assert(true);
// static_assert(sizeof(uint8_t) == 1);
// static_assert(sizeof(uint16_t) == 2);
// static_assert(sizeof(uint32_t) == 4);

#define LENGTHOF(a) (sizeof(a) / sizeof(a[0]))

template <class T, size_t N> inline size_t length(T x[N]) {
  (void)x;
  return N;
}

template <class T> inline T min(T x, T y) { return (x < y) ? x : y; }

template <class T> inline T min(T x, T y, T z) { return min(min(x, y), z); }

template <class T> inline T max(T x, T y) { return (x > y) ? x : y; }

template <class T> inline T max(T x, T y, T z) { return max(max(x, y), z); }

#endif // __BASE_H_INCLUDED
