#ifndef __IMBIBE_H_INCLUDED
#define __IMBIBE_H_INCLUDED


#if defined(__WATCOMC__)
#define __STDC_LIMIT_MACROS
#pragma warning 549 9
#endif


#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>


#define fourcc(arr) ((uint32_t)arr[0] | ((uint32_t)arr[1] << 8) \
  | ((uint32_t)arr[2] << 16) | ((uint32_t)arr[3] << 24))


#define assert_margin(x, mag) assert(((x) >= -((mag) / 4)) && ((x) <= (mag) / 4))


typedef int16_t coord_t;
typedef uint8_t color_t;
typedef uint8_t attribute_t;
typedef uint16_t termel_t;
typedef int16_t anim_time_t;
typedef int32_t large_anim_time_t;


#define COORD_MIN INT16_MIN
#define COORD_MAX INT16_MAX
#define ANIM_TIME_MAX INT16_MAX
#define ANIM_TIME_MIN INT16_MIN
#define LARGE_ANIM_TIME_MAX INT32_MAX
#define LARGE_ANIM_TIME_MIN INT32_MIN

#if !(defined(M_I86) && defined(__WATCOMC__))

// This is to make the editor happy when it tries to parse our code thinking
// we have some kind of normal compiler

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
#define MK_FP(s, o) \
  ((uintptr_t)(s) == 0xB800 ? (void *)dummy_screen \
    : (void *)((uintptr_t)(s) + (uintptr_t)(o)))

extern uint16_t dummy_screen[16384];

#define __interrupt

#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


#define cprintf(...) fprintf(stderr, __VA_ARGS__)


inline void failsafe_textmode() { }

extern void _dos_setvect(int, void (*)());
extern void (*_dos_getvect(int))();
extern void _chain_intr(void (*)());

extern unsigned _dos_open(const char * path, unsigned mode, int * handle);
extern unsigned _dos_close(int handle);
extern unsigned _dos_read(int handle, void * buf, unsigned count,
  unsigned * bytes);
extern unsigned _dos_lseek(int handle, long offset, int whence,
    unsigned long __far * where);

extern unsigned _dos_allocmem(unsigned size, unsigned * seg);
extern unsigned _dos_freemem(unsigned seg);


inline void * operator new (size_t size, void * p) { (void)size; return p; }

extern void step_simulator();

#define SIMULATE

#else

// This part is for real

#include <i86.h>
#include <conio.h>
#include <dos.h>

extern void failsafe_textmode();

#pragma aux failsafe_textmode = \
  "mov ax, 03h" "int 010h" modify [ax] nomemory

// seemingly missing from dos.h??
extern unsigned _dos_lseek(int handle, long offset, int whence,
    unsigned long __far * where);

#define __packed__

#endif


#define LENGTHOF(a) (sizeof (a) / sizeof (a[0]))

#define logf(...) cprintf(__VA_ARGS__)
#define disable_logf(...) do {} while (false)

#define abortf(...) do { failsafe_textmode(); cprintf("fatal error: "); \
  cprintf(__VA_ARGS__); abort(); } while(false)

template<class T>
inline T min(T x, T y) { return (x < y) ? x : y; }

template<class T>
inline T min(T x, T y, T z) { return min(min(x, y), z); }

template<class T>
inline T max(T x, T y) { return (x > y) ? x : y; }

template<class T>
inline T max(T x, T y, T z) { return max(max(x, y), z); }


#endif // __IMBIBE_H_INCLUDED

