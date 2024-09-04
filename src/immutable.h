#ifndef __IMMUTABLE_H_INCLUDED
#define __IMMUTABLE_H_INCLUDED

#include "imbibe.h"

#define logf_immutable(...) disable_logf("IMMUTABLE: " __VA_ARGS__)

class immutable {
public:
  enum prealloc_t { prealloc };
  typedef void (*reclaim_func_t)(void __far *p);

  immutable() : m_seg(0), m_index(0), m_ofs(0) {}

  immutable(prealloc_t policy, void const *p) : m_index(0) {
    assign(policy, p);
  }

  immutable(reclaim_func_t f, void const *p) : m_index(0) { assign(f, p); }

#ifdef M_I86
  immutable(immutable const &other) {
    set_handle(other.handle());
    if (m_index != 0) {
      ref();
    }
  }
#else
  immutable(immutable const &other)
      : m_seg(other.m_seg), m_index(other.m_index), m_ofs(other.m_ofs) {
    if (m_index != 0) {
      ref();
    }
  }
#endif

  ~immutable() {
    if (m_index != 0) {
      unref();
    }
  }

  void assign(prealloc_t policy, void const __far *p);
  void assign(reclaim_func_t f, void const __far *p);

  operator bool() const { return !!m_seg; }

  immutable &operator=(immutable const &other);

  immutable &operator=(void const *p) {
    assert(p == NULL);
    (void)p;
    if (m_index != 0) {
      unref();
    }
    m_index = 0;
    return *this;
  }

#ifdef M_I86
  bool operator==(immutable const &other) { return handle() == other.handle(); }
#else
  bool operator==(immutable const &other) {
    return (m_seg == other.m_seg) && (m_index == other.m_index) &&
           (m_ofs == other.m_ofs);
  }
#endif

  bool operator==(void *p) {
    assert(p == NULL);
    (void)p;
    return !operator bool();
  }

  void const __far *data() const {
    return MK_FP(m_seg, (void *)(uintptr_t)m_ofs);
  }

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_ofs;

  void init(reclaim_func_t f, void const __far *orig_p);
  void ref();
  void unref();

#ifdef M_I86
  explicit immutable(uint32_t n_handle) {
    static_assert(sizeof(immutable) == sizeof(uint32_t));
    set_handle(n_handle);
    if (m_index != 0) {
      ref();
    }
  }

  uint32_t handle() const { return *(uint32_t *)this; }
  void set_handle(uint32_t n_handle) { *(uint32_t *)this = n_handle; }
#endif

  friend class weak_immutable;
};

class weak_immutable {
public:
  weak_immutable() : m_seg(0), m_index(0), m_ofs(0) {}
#ifdef M_I86
  weak_immutable(immutable const &strong) { set_handle(strong.handle()); }
  weak_immutable(weak_immutable const &other) { set_handle(other.handle()); }
#else
  weak_immutable(immutable const &strong)
      : m_seg(strong.m_seg), m_index(strong.m_index), m_ofs(strong.m_ofs) {}
  weak_immutable(weak_immutable const &other)
      : m_seg(other.m_seg), m_index(other.m_index), m_ofs(other.m_ofs) {}
#endif

  immutable lock();

  weak_immutable &operator=(immutable const &strong) {
#ifdef M_I86
    set_handle(strong.handle());
#else
    m_seg = strong.m_seg;
    m_index = strong.m_index;
    m_ofs = strong.m_ofs;
#endif
    return *this;
  }

  weak_immutable &operator=(weak_immutable const &other) {
#ifdef M_I86
    set_handle(other.handle());
#else
    m_seg = other.m_seg;
    m_index = other.m_index;
    m_ofs = other.m_ofs;
#endif
    return *this;
  }

  weak_immutable &operator=(void const *p) {
    assert(p == NULL);
    (void)p;
#ifdef M_I86
    set_handle(0);
#else
    m_seg = 0;
    m_index = 0;
    m_ofs = 0;
#endif
    return *this;
  }

#ifndef NDEBUG
  // This is conservative: false means false, true is ambiguous; it's only
  // really safe if you want to overapproximate "were we explicitly set null"
  // and therefore it's only really appropriate inside assert()
  operator bool() const { return (m_index != 0) || m_seg; }
#endif

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_ofs;

#ifdef M_I86
  uint32_t handle() const { return *(uint32_t *)this; }
  void set_handle(uint32_t n_handle) { *(uint32_t *)this = n_handle; }
#endif
};

#endif // __IMMUTABLE_H_INCLUDED
