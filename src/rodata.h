#ifndef __RODATA_H_INCLUDED
#define __RODATA_H_INCLUDED


#include "imbibe.h"


// Generic read-only data block
class rodata {
public:
  static const uint16_t max_reclaimable = UINT8_MAX;

  enum prealloc_t { prealloc };
  typedef void (*reclaim_func_t)(void __far * p);

  rodata(prealloc_t policy, void const * p) {
    p = normalize(p);
    m_seg = FP_SEG(p);
    m_index = 0;
    m_offset = FP_OFF(p);
  }

  rodata(reclaim_func_t f, void const * p) {
    assert(p == normalize(p));
    p = normalize(p);
    m_seg = FP_SEG(p);
    m_offset = FP_OFF(p);
    init(f);
  }

  rodata(const rodata & other): m_seg(other.m_seg), m_index(other.m_index),
      m_offset(other.m_offset) {
    if (m_index != 0) {
      ref();
    }
  }

  ~rodata() {
    if (m_index != 0) {
      unref();
    }
  }

  void __far * data() { return MK_FP(m_seg, (void __based(m_seg) *)m_offset); }

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_offset;

  void init(reclaim_func_t f);
  void ref();
  void unref();

  static void const * normalize(void const * p) {
    if(FP_OFF(p) < 256) {
      return p;
    }
    p = MK_FP(FP_SEG(p) + (FP_OFF(p) >> 4), FP_OFF(p) & 0xF);
    return p;
  }
};


#endif __RODATA_H_INCLUDED

