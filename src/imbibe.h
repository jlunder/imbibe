#ifndef __IMBIBE_H_INCLUDED
#define __IMBIBE_H_INCLUDED


#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <malloc.h>


#if !(defined(M_I86) && defined(__WATCOMC__))

// This is to make the editor happy when it tries to parse our code thinking
// we have some kind of normal compiler

#define __far

#define __based(o)
#define __segment uintptr_t
#define __segname(n)
// __segname gets the named segment -- probably "_CODE", "_CONST", "_DATA"
#define __self

#define FP_SEG(p) ((uintptr_t)(p))
#define FP_OFF(p) 0
#define MK_FP(s, o) ((void *)(s))

#define __interrupt

#include <stdio.h>

#define cprintf printf

inline void _dos_setvect(int, void (*)()) { }
inline void (*_dos_getvect(int))() { return nullptr; }
inline void _chain_intr(void (*)()) { }

inline void * operator new (size_t size, void * p) { (void)size; return p; }

#define SIMULATE

#else

// This part is for real

#include <i86.h>
#include <conio.h>
#include <dos.h>

#endif


#define LENGTHOF(a) (sizeof (a) / sizeof (a[0]))

#define logf(...) do {} while(false)

template<class T>
inline T min(T x, T y) { return (x < y) ? x : y; }

template<class T>
inline T min(T x, T y, T z) { return min(min(x, y), z); }

template<class T>
inline T max(T x, T y) { return (x < y) ? x : y; }

template<class T>
inline T max(T x, T y, T z) { return max(max(x, y), z); }


#endif // __IMBIBE_H_INCLUDED

