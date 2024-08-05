#ifndef __IMBIBE_H_INCLUDED
#define __IMBIBE_H_INCLUDED


#include <assert.h>
#include <stdint.h>
#include <stddef.h>


#if !defined(M_I86) || !defined(__WATCOMC__)

// This is to make the editor happy when it tries to parse our code thinking
// we have some kind of normal compiler

#define __based(o)
#define __segment uint16_t
#define __segname(n)
// __segname gets the named segment -- probably "_CODE", "_CONST", "_DATA"
#define __self

#define __far

#define FP_SEG(p) 0
#define FP_OFF(p) NULL

#define MK_FP(s, o) NULL

#include <stdio.h>

#define cprintf printf

#else

// This part is for real

#include <i86.h>
#include <conio.h>
#include <dos.h>

#endif


#define LENGTHOF(a) (sizeof (a) / sizeof (a[0]))


#endif // __IMBIBE_H_INCLUDED

