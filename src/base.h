#ifndef __BASE_H_INCLUDED
#define __BASE_H_INCLUDED

#if defined(NDEBUG)
#define BUILD_DEBUG 0
#else
#define BUILD_DEBUG 1
#endif

#if defined(__WATCOMC__)

#define __STDC_LIMIT_MACROS
#pragma warning 549 9
#pragma warning 446 9
#pragma warning 14 9

#if !defined(M_I86) || !defined(MSDOS)
#error Inconsistent (unsupported) platform macros defined
#endif

#define BUILD_MSDOS 1
#define BUILD_MSDOS_GCC_IA16 0
#define BUILD_MSDOS_WATCOMC 1
#define BUILD_POSIX 0
#define BUILD_POSIX_SIM 0
#define BUILD_POSIX_SDL_GL 0

#elif defined(__ia16__)

#if !defined(_M_I86)
#error Inconsistent (unsupported) platform macros defined
#endif

#define BUILD_MSDOS 1
#define BUILD_MSDOS_GCC_IA16 1
#define BUILD_MSDOS_WATCOMC 0
#define BUILD_POSIX_SIM 0
#define BUILD_POSIX_SDL_GL 0
#define BUILD_POSIX_SDL_WEBGL 0

#else

#define BUILD_MSDOS 0
#define BUILD_MSDOS_GCC_IA16 0
#define BUILD_MSDOS_WATCOMC 0
#define BUILD_POSIX 1

#if defined(GLIMBIBE)

#define BUILD_POSIX_SIM 0
#define BUILD_POSIX_SDL_GL 1
#define BUILD_POSIX_SDL_WEBGL 0

#elif defined(WIMBIBE)

#define BUILD_POSIX_SIM 0
#define BUILD_POSIX_SDL_GL 1
#define BUILD_POSIX_SDL_WEBGL 1

#else

#define BUILD_POSIX_SIM 1
#define BUILD_POSIX_SDL_GL 0
#define BUILD_POSIX_SDL_WEBGL 0

#endif

#endif

#if BUILD_POSIX_SIM || BUILD_POSIX_SDL_GL

#define BUILD_NEAR_DATA 0
#define BUILD_FAR_DATA 0

#elif (defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)) &&      \
    !(defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__))

#define BUILD_NEAR_DATA 1
#define BUILD_FAR_DATA 0

#elif !(defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)) &&     \
    (defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__))

#define BUILD_NEAR_DATA 0
#define BUILD_FAR_DATA 1

#else

#error Pointer model platform macros not defined or not consistent

#endif

#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_M_I86) && !defined(M_I86)
#define M_I86
#define MSDOS
#endif

#if (defined(M_I86) && !defined(MSDOS)) || (!defined(M_I86) && defined(MSDOS))
#error Inconsistent (unsupported) platform macros defined!
#endif

#if BUILD_MSDOS

#include <conio.h>
#include <dos.h>
#include <i86.h>

#if BUILD_MSDOS_WATCOMC
#define MK_FP_O32(s, o) _mk_fp_o32((s), (o))
static inline void __far * _mk_fp_o32(unsigned s, unsigned long o) {
  return MK_FP(s + (o >> 4), o & 0xF);
}
#else
#error New platform support needed?
#endif

extern void failsafe_textmode();

#if BUILD_MSDOS_WATCOMC
#pragma aux failsafe_textmode =                                                \
    "   mov     ah, 003h                "                                      \
    "   mov     bh, 0                   "                                      \
    "   int     010h                    "                                      \
    "   mov     ax, 00003h              "                                      \
    "   int     010h                    "                                      \
    "   mov     ah, 002h                "                                      \
    "   mov     bh, 0                   "                                      \
    "   int     010h                    " modify[ax bx cx dx] nomemory
#else
#error New platform support needed?
#endif

// seemingly missing from dos.h??
// supposedly DM C implements a _dos_seek that returns where directly;
// MS C may implement it with this interface;
// I choose to name my version _dos_lseek to avoid the mess.
extern unsigned _dos_lseek(int handle, long offset, int whence,
                           unsigned long __far *where);

#define wcc_packed _Packed
#define gcc_packed

#define PRpF "Fp"
#define PRpN "Np"
#define PRp "p"

#define PRsF "Fs"
#define PRsN "Ns"
#define PRs "s"

extern void __sync_synchronize();
#pragma aux __sync_synchronize = "" modify;

#elif BUILD_POSIX

#define __near
#define __far volatile
// #define __far

#define wcc_packed
#define gcc_packed __attribute__((__packed__))

#define __based(o)
#define __segment uintptr_t
#define __segname(n)
// __segname gets the named segment -- probably "_TEXT", "_CONST", "_DATA"
#define __self

#define FP_SEG(p) (reinterpret_cast<uintptr_t>(p) & ~0xFLLU)
#define FP_OFF(p) (reinterpret_cast<uintptr_t>(p) & 0xFLLU)
#define MK_FP(s, o)                                                            \
  (reinterpret_cast<void __far *>(sim::map_segment(s) + (uintptr_t)(o)))

#define MK_FP_O32(s, o) MK_FP((s), (o))

#define PRpF "p"
#define PRpN "p"
#define PRp "p"

#define PRsF "s"
#define PRsN "s"
#define PRs "s"

namespace sim {

extern uintptr_t map_segment(uintptr_t seg);

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

extern unsigned _dos_allocmem(unsigned size, unsigned __far *segment);
extern unsigned _dos_freemem(unsigned segment);

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
extern int _fmemcmp(void const __far *x, void const __far *y, size_t size);
extern void _fmemcpy(void __far *dest, void const __far *src, size_t size);

#else
#error New platform support needed?
#endif

// #define __static_assert(con, id) static int assert_ ## id [2 * !!(con) - 1];
// #define _static_assert(con, id) __static_assert(con, id)
// #define static_assert(con) _static_assert(con, __COUNTER__)

template <bool> struct base_static_assert;
template <> struct base_static_assert<true> {};

#if defined(__WATCOMC__)
#define static_assert(e) extern ::base_static_assert<!!(e)> __the_sa;
#endif

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

// template <class T> inline T min(T x, T y, T z) { return min(min(x, y), z); }

template <class T> inline T max(T x, T y) { return (x > y) ? x : y; }

// template <class T> inline T max(T x, T y, T z) { return max(max(x, y), z); }

template <class T> inline T clamp(T x, T lower, T upper) {
  assert(lower <= upper);
  return max(min(x, upper), lower);
}

#endif // __BASE_H_INCLUDED
