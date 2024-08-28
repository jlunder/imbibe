#ifndef __RODATA_H_INCLUDED
#define __RODATA_H_INCLUDED

#include "imbibe.h"

#define logf_immutable(...) disable_logf("IMMUTABLE: " __VA_ARGS__)

class immutable {
public:
  static uint16_t const max_reclaimable = UINT8_MAX;

  enum prealloc_t { prealloc };
  typedef void (*reclaim_func_t)(void __far *p);

  immutable() : m_seg(0), m_index(0), m_offset(0) {}

  immutable(prealloc_t policy, void const *p) {
    (void)policy;
    if (p) {
      void const *norm_p = normalize(p);
      m_seg = FP_SEG(norm_p);
      assert(m_seg != 0);
      m_index = 0;
      assert(FP_OFF(norm_p) < 0x10);
      m_offset = (uint8_t)FP_OFF(norm_p);
    } else {
      m_seg = 0;
      m_index = 0;
      m_offset = 0;
    }
  }

  immutable(reclaim_func_t f, void const *p) {
    if (p) {
      void const *norm_p = normalize(p);
      m_seg = FP_SEG(norm_p);
      assert(m_seg != 0);
      assert(FP_OFF(norm_p) < 0x10);
      m_offset = (uint8_t)FP_OFF(norm_p);
      assert(denormalize(FP_SEG(p), MK_FP(m_seg, m_offset)) == p);
      init(f, p);
    } else {
      m_seg = 0;
      m_index = 0;
      m_offset = 0;
    }
  }

  immutable(const immutable &other)
      : m_seg(other.m_seg), m_index(other.m_index), m_offset(other.m_offset) {
    if (m_index != 0) {
      ref();
    }
  }

  ~immutable() {
    if (m_index != 0) {
      unref();
    }
  }

  operator bool() const { return !!m_seg; }
  immutable &operator=(immutable const &other) {
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

  bool operator==(immutable const &other) {
#if defined(SIMULATE)
    return (m_index && (m_index == other.m_index)) ||
           ((m_seg == other.m_seg) && (m_offset == other.m_offset));
#else
    assert(sizeof(immutable) == sizeof(uint32_t));
    return *(uint32_t *)this == *(uint32_t *)&other;
#endif
  }

  bool operator==(void *other) {
    assert(other == NULL);
    return !operator bool();
  }

  void const __far *data() const {
    return MK_FP(m_seg, (void *)(uintptr_t)m_offset);
  }

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_offset;

  void init(reclaim_func_t f, void const *orig_p);
  void ref();
  void unref();

  static void const *normalize(void const *p) {
#if defined(SIMULATE)
    void const *norm_p = p;
#else
    if ((uint16_t)FP_OFF(p) <= UINT8_MAX) {
      return p;
    }
    void const *norm_p = MK_FP(FP_SEG(p) + (FP_OFF(p) >> 4), FP_OFF(p) & 0xF);
#endif
    logf_immutable("normalized %p to %p\n", p, norm_p);
    assert(FP_SEG(norm_p) - FP_SEG(p) < 0x8000);
    return norm_p;
  }

  static void const *denormalize(__segment orig_seg, void const *norm_p) {
#if defined(SIMULATE)
    void const *p = norm_p;
#else
    void const *p =
        MK_FP(orig_seg, FP_OFF(norm_p) + ((FP_SEG(norm_p) - orig_seg) << 4));
#endif
    logf_immutable("denormalized %p to %p\n", norm_p, p);
    assert(FP_SEG(p) == orig_seg);
    assert(FP_SEG(norm_p) - orig_seg < 0x8000);
    return p;
  }
};

// Shared pointer for generic read-only data blocks; this is optimized for
// the case where sometimes the block is statically allocated and doesn't
// need to be freed. If your data is always static, just use bare pointers --
// and if it's never static, use actual shared_ptr.
template <class T> class im_ptr : private immutable {
public:
  static const prealloc_t prealloc = immutable::prealloc;

  static im_ptr null() { im_ptr(prealloc, NULL); }

  explicit im_ptr() : immutable() {}
  explicit im_ptr(T const *n_p) : immutable(n_p ? reclaim : NULL, n_p) {}
  explicit im_ptr(immutable::prealloc_t policy, T const *n_p)
      : immutable(policy, n_p) {}
  explicit im_ptr(immutable::reclaim_func_t custom_reclaim, T const *n_p)
      : immutable(custom_reclaim, n_p) {}

  T const __far *get() const { return (T const *)data(); }
  T const __far &operator*() const { return *(T const *)data(); }
  T const __far *operator->() const { return (T const *)data(); }
  operator bool() const { return immutable::operator bool(); }

  bool operator==(im_ptr const &other) { return immutable::operator==(other); }

  bool operator==(void *other) { return immutable::operator==(other); }

private:
  static void reclaim(void *p) { delete (T const *)p; }
};

#endif // __RODATA_H_INCLUDED
