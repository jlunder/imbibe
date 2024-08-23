#ifndef __RODATA_H_INCLUDED
#define __RODATA_H_INCLUDED


#include "imbibe.h"


class immutable {
public:
  static uint16_t const max_reclaimable = UINT8_MAX;

  enum prealloc_t { prealloc };
  typedef void (*reclaim_func_t)(void __far * p);

  immutable(prealloc_t policy, void const * p) {
    (void)policy;
    if(p) {
      p = normalize(p);
      m_seg = FP_SEG(p);
      assert(m_seg != 0);
      m_index = 0;
      m_offset = (uint8_t)FP_OFF(p);
    } else {
      m_seg = 0;
      m_index = 0;
      m_offset = 0;
    }
  }

  immutable(reclaim_func_t f, void const * p) {
    assert(p == normalize(p));
    if(p) {
      p = normalize(p);
      m_seg = FP_SEG(p);
      assert(m_seg != 0);
      m_offset = (uint8_t)FP_OFF(p);
      init(f);
    } else {
      m_seg = 0;
      m_index = 0;
      m_offset = 0;
    }
  }

  immutable(const immutable & other): m_seg(other.m_seg), m_index(other.m_index),
      m_offset(other.m_offset) {
    if (m_index != 0) {
      ref();
    }
  }

  ~immutable() {
    if (m_index != 0) {
      unref();
    }
  }

  operator bool () const { return !!m_seg; }
  immutable & operator = (immutable const & other) {
    if (m_index != 0) {
      if (m_index == other.m_index) {
        return *this;
      }
      unref();
    }
    m_seg = other.m_seg;
    m_index = other.m_index;
    m_offset = other.m_offset;
    if (m_index != 0) {
      ref();
    }
    return *this;
  }

  void __far * data() const { return MK_FP(m_seg, (void *)(uintptr_t)m_offset); }

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_offset;

  void init(reclaim_func_t f);
  void ref();
  void unref();

  static void const * normalize(void const * p) {
    if((uint16_t)FP_OFF(p) <= UINT8_MAX) {
      return p;
    }
    return MK_FP(FP_SEG(p) + (FP_OFF(p) >> 4), FP_OFF(p) & 0xF);
  }
};


// Shared pointer for generic read-only data blocks; this is optimized for
// the case where sometimes the block is statically allocated and doesn't
// need to be freed. If your data is always static, just use bare pointers --
// and if it's never static, use actual shared_ptr.
template<class T>
class im_ptr: private immutable {
public:
  static const prealloc_t prealloc = immutable::prealloc;

  static im_ptr null() { im_ptr(prealloc, NULL); }

  explicit im_ptr(T const * n_p) : immutable(n_p ? reclaim : NULL, n_p) { }
  explicit im_ptr(immutable::prealloc_t policy, T const * n_p)
    : immutable(n_p ? reclaim : NULL, n_p) {
  }
  explicit im_ptr(immutable::reclaim_func_t custom_reclaim, T const * n_p)
    : immutable(custom_reclaim, n_p) {
  }

  T const * get() const { return (T const *)data(); }
  T const & operator * () const { return *(T const *)data(); }
  T const * operator -> () const { return (T const *)data(); }
  operator bool () const { return immutable::operator bool(); }

private:
  static void reclaim(void * p) {
    delete (T const *)p;
  }
};


#endif // __RODATA_H_INCLUDED

