#include "imbibe.h"

#include "fletcher16.h"

#define logf_fletcher16(...) disable_logf(__VA_ARGS__)

#if 0
#if BUILD_MSDOS_WATCOMC

extern uint16_t asm_fletcher16_buf(void const __far *buf, segsize_t size,
                                   uint8_t seed);

// clang-format off
#pragma aux asm_fletcher16_buf = \
    /*  ; BX is the lower counter                            */ \
    "   xor     bh, bh                      "                   \
    /*  ; DX is the upper (dual) counter                     */ \
    "   xor     dx, dx                      "                   \
    "   cld                                 "                   \
    "   jcxz    @done                       "                   \
    /*  ; check alignment of pointer in SI                   */ \
    "   test    si, 0x0001                  "                   \
    "   jz      @loop1                      "                   \
    /*  ; SI has LSB set: align reads to word boundary by    */ \
    /*  ; prerolling and starting the loop in the middle     */ \
    "   lodsb                               "                   \
    "   mov     ah, al                      "                   \


    "   dec     cx                          "                   \
    "   jz      @done                       "                   \
    "   jmp     @loop2                      "                   \
    "@loop1:                                "                   \
    /*  ; read 2 bytes -- this may go past end of string     */ \
    "   lodsw                               " /* 5           */ \
    "   add     bx, ax                      " /* 2           */ \
    "   add     dx, bx                      " /* 2           */ \
    "   dec     cx                          "                   \
    "   jz      @done                       "                   \
    /*  ; sum in the first byte                              */ \
    /*  ; check overflow in the dual counter                 */ \
    "   jc      @adj1                       " /* 3 (jump +4) */ \
    "@loop2:                                                  " \
    "   mov     cl, ah                      " /* 2           */ \
    /*  ; check second for NUL                               */ \
    "   jcxz    @done                       " /* 4 (jump +4) */ \
    /*  ; sum in the second byte                             */ \
    "   add     bx, cx                      " /* 2           */ \
    "   add     dx, bx                      " /* 2           */ \
    /*  ; check overflow in the dual counter                 */ \
    "   jnc     @loop1                      " /* 7 (njmp -4) */ \
    "@adj2:                                 "                   \
    /*  ; it takes 3x ADC to fully add BH with carry to BL   */ \
    /*  ; -- the limit case is BL=0xFF, BH=0xFF, CF set      */ \
    "   adc     dl, dh                      " /*  2          */ \
    "   adc     dl, ch                      " /*  2          */ \
    "   adc     dl, ch                      " /*  2          */ \
    /*  ; clear DH once it's fully added in; note DX could   */ \
    /*  ; still be 255, so the modulo isn't necessarily done */ \
    "   xor     dh, dh                      " /*  2          */ \
    /*  ; do similar for BX, but because BX < ~sqrt(DX), it  */ \
    /*  ; has definitely not overflowed                      */ \
    "   add     bl, bh                      " /*  2          */ \
    "   adc     bl, ch                      " /*  2          */ \
    "   xor     bh, bh                      " /*  2          */ \
    "   jmp     @loop1                      " /*  7          */ \
    "                                       "                   \
    "@adj1:                                 "                   \
    /*  ; same as @adj2 above                                */ \
    "   adc     dl, dh                      " /*  2          */ \
    "   adc     dl, ch                      " /*  2          */ \
    "   adc     dl, ch                      " /*  2          */ \
    "   xor     dh, dh                      " /*  2          */ \
    "   add     bl, bh                      " /*  2          */ \
    "   adc     bl, ch                      " /*  2          */ \
    "   xor     bh, bh                      " /*  2          */ \
    "   jmp     @loop2                      " /*  7          */ \
    "                                       "                   \
    "@done:                                 "                   \
    /*  ; finalize the counters to exactly mod 255           */ \
    "   add     bl, bh                     " /*  2           */ \
    "   adc     bl, ch                     " /*  2           */ \
    /*  ; this ADD/ADC/DEC normalizes 255 -> 0 without jumps */ \
    "   add     bl, 1                      " /*  3           */ \
    "   adc     bl, ch                     " /*  2           */ \
    "   dec     bl                         " /*  2           */ \
    "                                      "                    \
    /*  ; same as above, for the upper counter               */ \
    "   add     dl, dh                     " /*  2           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   add     dl, 1                      " /*  3           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   dec     dl                         " /*  2           */ \
    "                                      "                    \
    /*  ; consolidate the result into AX                     */ \
    "   mov     al, bl                     " /*  2           */ \
    "   mov     ah, dl                     " /*  2           */ \
    modify [ax bx cx dx si] parm[ds si, ] value[ax]
// clang-format on

#endif

uint16_t fletcher16_buf(void const __far *buf, segsize_t size, uint8_t seed) {
#if BUILD_DEBUG || !BUILD_MSDOS_WATCOMC
  // This is essentially a reimplementation of the optimized Fletcher16 from
  // Understanding Checksums and Cyclic Redundancy Checks (Koopman, 2024)
  uint16_t s1 = 0;
  uint16_t s2 = 0;
  for (uint8_t const __far *p = reinterpret_cast<uint8_t const __far *>(buf);
       *p; ++p) {
    s1 += (uint8_t)*p;
    s2 += s1;
    if (s2 >= 0x8000u) {
      s2 = (s2 & 0xFF) + (s2 >> 8);
      s1 = (s1 & 0xFF) + (s1 >> 8);
    }
  }
  s2 = (s2 & 0xFF) + (s2 >> 8);
  s2 = (s2 & 0xFF) + (s2 >> 8);
  if (s2 == 0xFF) {
    s2 = 0;
  }
  s1 = (s1 & 0xFF) + (s1 >> 8);
  s1 = (s1 & 0xFF) + (s1 >> 8);
  if (s1 == 0xFF) {
    s1 = 0;
  }
#endif
#if BUILD_MSDOS_WATCOMC
  uint16_t result = asm_fletcher16_buf(buf, size, seed);
#if BUILD_DEBUG
  if (result != ((s2 << 8) | s1)) {
    logf_fletcher16(
        "FLETCHER16 ASM for string \"%s\", expected %04X, got %04X", s,
        (unsigned)((s2 << 8) | s1), (unsigned)result);
  }
  assert(result == ((s2 << 8) | s1));
#endif
  return result;
#else
  return (s2 << 8) | s1;
#endif
}

#endif

uint16_t fletcher16_buf(void const __far *buf, segsize_t size, uint8_t seed) {
  // This is essentially a reimplementation of the optimized Fletcher16 from
  // Understanding Checksums and Cyclic Redundancy Checks (Koopman, 2024)
  uint16_t s1 = seed;
  uint16_t s2 = 0;
  uint8_t const __far *p = reinterpret_cast<uint8_t const __far *>(buf);
  for (segsize_t i = 0; i < size; ++i, ++p) {
    s1 += (uint8_t)*p;
    s2 += s1;
    if (s2 >= 0x8000u) {
      s2 = (s2 & 0xFF) + (s2 >> 8);
      s1 = (s1 & 0xFF) + (s1 >> 8);
    }
  }
  s2 = (s2 & 0xFF) + (s2 >> 8);
  s2 = (s2 & 0xFF) + (s2 >> 8);
  if (s2 == 0xFF) {
    s2 = 0;
  }
  s1 = (s1 & 0xFF) + (s1 >> 8);
  s1 = (s1 & 0xFF) + (s1 >> 8);
  if (s1 == 0xFF) {
    s1 = 0;
  }
  return (s2 << 8) | s1;
}

#if BUILD_MSDOS_WATCOMC

extern uint16_t asm_fletcher16_str(char const __far *s);

/*
#pragma aux asm_fletcher16_str =                                               \
    "   dec     di              "                                              \
    "   xor     ax, ax          "                                              \
    "   xor     bx, bx          "                                              \
    "@loop:                     "                                              \
    "   inc     di              "                                              \
    "   mov     bl, es:[di]     "                                              \
    "   or      bx, bx          "                                              \
    "   je      @done           "                                              \
    "   add     al, bl          "                                              \
    "   jc      @roll_l         "                                              \
    "   cmp     al, 255         "                                              \
    "   jne     @noroll_l       "                                              \
    "@roll_l:                   "                                              \
    "   inc     al              "                                              \
    "@noroll_l:                 "                                              \
    "   add     ah, al          "                                              \
    "   jc      @roll_h         "                                              \
    "   cmp     ah, 255         "                                              \
    "   jne     @loop           "                                              \
    "@roll_h:                   "                                              \
    "   inc     ah              "                                              \
    "   jmp     @loop           "                                              \
    "@done:                     " modify[ax bx di] parm[es di] value[ax]
*/

// clang-format off
#pragma aux asm_fletcher16_str = \
    "   push    ds                         "                    \
    "   mov     ds, dx                     "                    \
    /*  ; BX is the lower counter                            */ \
    "   xor     bx, bx                     "                    \
    /*  ; DX is the upper (dual) counter                     */ \
    "   xor     dx, dx                     "                    \
    /*  ; CX is used for unpacking and test (CH always 0)    */ \
    "   xor     cx, cx                     "                    \
    "   cld                                "                    \
    /*  ; check alignment of pointer in SI                   */ \
    "   test    si, 0x0001                 "                    \
    "   jz      @loop1                     "                    \
    /*  ; SI has LSB set: align reads to word boundary by    */ \
    /*  ; prerolling and starting the loop in the middle     */ \
    "   lodsb                              "                    \
    "   mov     ah, al                     "                    \
    "   jmp     @loop2                     "                    \
    "@loop1:                               "                    \
    /*  ; read 2 bytes -- this may go past end of string     */ \
    "   lodsw                              " /*  5           */ \
    "   mov     cl, al                     " /*  2           */ \
    /*  ; check first for NUL                                */ \
    "   jcxz    @done                      " /*  4 (jump +4) */ \
    /*  ; sum in the first byte                              */ \
    "   add     bx, cx                     " /*  2           */ \
    "   add     dx, bx                     " /*  2           */ \
    /*  ; check overflow in the dual counter                 */ \
    "   jc      @adj1                      " /*  3 (jump +4) */ \
    "@loop2:                                                  " \
    "   mov     cl, ah                     " /*  2           */ \
    /*  ; check second for NUL                               */ \
    "   jcxz    @done                      " /*  4 (jump +4) */ \
    /*  ; sum in the second byte                             */ \
    "   add     bx, cx                     " /*  2           */ \
    "   add     dx, bx                     " /*  2           */ \
    /*  ; check overflow in the dual counter                 */ \
    "   jnc     @loop1                     " /*  7 (njmp -4) */ \
    "@adj2:                                "                    \
    /*  ; it takes 3x ADC to fully add BH with carry to BL   */ \
    /*  ; -- the limit case is BL=0xFF, BH=0xFF, CF set      */ \
    "   adc     dl, dh                     " /*  2           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   adc     dl, ch                     " /*  2           */ \
    /*  ; clear DH once it's fully added in; note DX could   */ \
    /*  ; still be 255, so the modulo isn't necessarily done */ \
    "   xor     dh, dh                     " /*  2           */ \
    /*  ; do similar for BX, but because BX < ~sqrt(DX), it  */ \
    /*  ; has definitely not overflowed                      */ \
    "   add     bl, bh                     " /*  2           */ \
    "   adc     bl, ch                     " /*  2           */ \
    "   xor     bh, bh                     " /*  2           */ \
    "   jmp     @loop1                     " /*  7           */ \
    "                                      "                    \
    "@adj1:                                "                    \
    /*  ; same as @adj2 above                                */ \
    "   adc     dl, dh                     " /*  2           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   xor     dh, dh                     " /*  2           */ \
    "   add     bl, bh                     " /*  2           */ \
    "   adc     bl, ch                     " /*  2           */ \
    "   xor     bh, bh                     " /*  2           */ \
    "   jmp     @loop2                     " /*  7           */ \
    "                                      "                    \
    "@done:                                "                    \
    /*  ; finalize the counters to exactly mod 255           */ \
    "   add     bl, bh                     " /*  2           */ \
    "   adc     bl, ch                     " /*  2           */ \
    /*  ; this ADD/ADC/DEC normalizes 255 -> 0 without jumps */ \
    "   add     bl, 1                      " /*  3           */ \
    "   adc     bl, ch                     " /*  2           */ \
    "   dec     bl                         " /*  2           */ \
    "                                      "                    \
    /*  ; same as above, for the upper counter               */ \
    "   add     dl, dh                     " /*  2           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   add     dl, 1                      " /*  3           */ \
    "   adc     dl, ch                     " /*  2           */ \
    "   dec     dl                         " /*  2           */ \
    "                                      "                    \
    /*  ; consolidate the result into AX                     */ \
    "   mov     al, bl                     " /*  2           */ \
    "   mov     ah, dl                     " /*  2           */ \
    "   pop     ds                         " /*              */ \
    modify [bx cx] \
    parm [dx si] \
    value [ax]
// clang-format on

#endif

uint16_t fletcher16_str(char const __far *s) {
#if BUILD_DEBUG || !BUILD_MSDOS_WATCOMC
  // See note in fletcher16_buf RE sourcing
  uint16_t s1 = 0;
  uint16_t s2 = 0;
  for (char const __far *p = s; *p; ++p) {
    s1 += (uint8_t)*p;
    s2 += s1;
    if (s2 >= 0x8000u) {
      s2 = (s2 & 0xFF) + (s2 >> 8);
      s1 = (s1 & 0xFF) + (s1 >> 8);
    }
  }
  s2 = (s2 & 0xFF) + (s2 >> 8);
  s2 = (s2 & 0xFF) + (s2 >> 8);
  if (s2 == 0xFF) {
    s2 = 0;
  }
  s1 = (s1 & 0xFF) + (s1 >> 8);
  s1 = (s1 & 0xFF) + (s1 >> 8);
  if (s1 == 0xFF) {
    s1 = 0;
  }
#endif
#if BUILD_MSDOS_WATCOMC
  uint16_t result = asm_fletcher16_str(s);
#if BUILD_DEBUG
  if (result != ((s2 << 8) | s1)) {
    logf_fletcher16("FLETCHER16 ASM for string \"%s\", expected %04X, got %04X",
                    s, (unsigned)((s2 << 8) | s1), (unsigned)result);
  }
  assert(result == ((s2 << 8) | s1));
#endif
  return result;
#else
  return (s2 << 8) | s1;
#endif
}
