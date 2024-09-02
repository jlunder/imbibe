#ifndef __BASE_H_INCLUDED
#define __BASE_H_INCLUDED

#if defined(__WATCOMC__)
#define __STDC_LIMIT_MACROS
#pragma warning 549 9
#pragma warning 446 9
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#if (defined(M_I86) && !defined(MSDOS)) || (!defined(M_I86) && defined(MSDOS))
#error Inconsistent (unsupported) platform macros defined!
#endif

#if !defined(M_I86) || !defined(MSDOS)

#define __near
#define __far

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

namespace sim {
extern uint16_t dummy_screen[16384];
}

#define __interrupt

#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define cprintf(...) fprintf(stderr, __VA_ARGS__)

inline void failsafe_textmode() {}

extern void _dos_setvect(int, void (*)());
extern void (*_dos_getvect(int))();
extern void _chain_intr(void (*)());

extern unsigned _dos_open(const char *path, unsigned mode, int *handle);
extern unsigned _dos_close(int handle);
extern unsigned _dos_read(int handle, void *buf, unsigned count,
                          unsigned *bytes);
extern unsigned _dos_lseek(int handle, long offset, int whence,
                           unsigned long __far *where);

extern unsigned _dos_allocmem(unsigned size, unsigned *seg);
extern unsigned _dos_freemem(unsigned seg);

inline void *operator new(size_t size, void *p) {
  (void)size;
  return p;
}

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

#endif

// #define __static_assert(con, id) static int assert_ ## id [2 * !!(con) - 1];
// #define _static_assert(con, id) __static_assert(con, id)
// #define static_assert(con) _static_assert(con, __COUNTER__)

template <bool> struct base_static_assert;
template <> struct base_static_assert<true>;

#define static_assert(e) extern ::base_static_assert<!!(e)> __the_sa

static_assert(true);
static_assert(sizeof(uint8_t) == 1);
static_assert(sizeof(uint16_t) == 2);
static_assert(sizeof(uint32_t) == 4);

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
