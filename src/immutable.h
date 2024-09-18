#ifndef __IMMUTABLE_H_INCLUDED
#define __IMMUTABLE_H_INCLUDED

#include "imbibe.h"

#define logf_immutable(...) disable_logf("IMMUTABLE: " __VA_ARGS__)

class immutable {
public:
  enum prealloc_t { prealloc };
  typedef void (*reclaim_func_t)(void __far *p);

  immutable() : m_seg(0), m_index(0), m_ofs(0) {}

  immutable(prealloc_t policy, void const __far *p) : m_index(0) {
    assign(policy, p);
  }

  immutable(reclaim_func_t f, void const __far *p) : m_index(0) {
    assign(f, p);
  }

#if BUILD_POSIX_SIM
  immutable(immutable const &other)
      : m_seg(other.m_seg), m_index(other.m_index), m_ofs(other.m_ofs) {
    if (m_index != 0) {
      ref();
    }
  }
#else
  immutable(immutable const &other) {
    set_handle(other.handle());
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
    m_seg = 0;
    m_index = 0;
    m_ofs = 0;
    return *this;
  }

#if BUILD_MSDOS
  bool operator==(immutable const &other) { return handle() == other.handle(); }
#elif BUILD_POSIX_SIM
  bool operator==(immutable const &other) {
    return (m_seg == other.m_seg) && (m_index == other.m_index) &&
           (m_ofs == other.m_ofs);
  }
#else
#error New platform support needed?
#endif

  bool operator==(void const *p) {
    assert(p == NULL);
    (void)p;
    return !operator bool();
  }

  void const __far *data() const { return MK_FP(m_seg, m_ofs); }

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_ofs;

  void init(reclaim_func_t f, void const __far *orig_p);
  void ref();
  void unref();

  immutable(__segment n_seg, uint8_t n_index, uint8_t n_ofs)
      : m_seg(n_seg), m_index(n_index), m_ofs(n_ofs) {
    if (m_index != 0) {
      ref();
    }
  }

#if BUILD_MSDOS
  explicit immutable(uint32_t n_handle) {
    static_assert(sizeof(immutable) == sizeof(uint32_t));
    set_handle(n_handle);
    if (m_index != 0) {
      ref();
    }
  }

  uint32_t handle() const { return *reinterpret_cast<uint32_t const *>(this); }
  void set_handle(uint32_t n_handle) {
    *reinterpret_cast<uint32_t *>(this) = n_handle;
  }
#endif

  friend class weak_immutable;
};

class weak_immutable {
public:
  weak_immutable() : m_seg(0), m_index(0), m_ofs(0) {}
#if BUILD_MSDOS
  weak_immutable(immutable const &strong) { set_handle(strong.handle()); }
  weak_immutable(weak_immutable const &other) { set_handle(other.handle()); }
#elif BUILD_POSIX_SIM
  weak_immutable(immutable const &strong)
      : m_seg(strong.m_seg), m_index(strong.m_index), m_ofs(strong.m_ofs) {}
  weak_immutable(weak_immutable const &other)
      : m_seg(other.m_seg), m_index(other.m_index), m_ofs(other.m_ofs) {}
#else
#error New platform support needed?
#endif

  immutable lock();

  weak_immutable &operator=(immutable const &strong) {
#if BUILD_MSDOS
    set_handle(strong.handle());
#elif BUILD_POSIX_SIM
    m_seg = strong.m_seg;
    m_index = strong.m_index;
    m_ofs = strong.m_ofs;
#else
#error New platform support needed?
#endif
    return *this;
  }

  weak_immutable &operator=(weak_immutable const &other) {
#if BUILD_MSDOS
    set_handle(other.handle());
#elif BUILD_POSIX_SIM
    m_seg = other.m_seg;
    m_index = other.m_index;
    m_ofs = other.m_ofs;
#else
#error New platform support needed?
#endif
    return *this;
  }

  weak_immutable &operator=(void const *p) {
    assert(p == NULL);
    (void)p;
#if BUILD_MSDOS
    set_handle(0);
#elif BUILD_POSIX_SIM
    m_seg = 0;
    m_index = 0;
    m_ofs = 0;
#else
#error New platform support needed?
#endif
    return *this;
  }

#if BUILD_DEBUG
  // This is conservative: false means false, true is ambiguous; it's only
  // really safe if you want to overapproximate "were we explicitly set null"
  // and therefore it's only really appropriate inside assert()
  operator bool() const { return (m_index != 0) || m_seg; }
#endif

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_ofs;

#if BUILD_MSDOS
  uint32_t handle() const { return *reinterpret_cast<uint32_t const *>(this); }
  void set_handle(uint32_t n_handle) {
    *reinterpret_cast<uint32_t *>(this) = n_handle;
  }
#endif
};

#endif // __IMMUTABLE_H_INCLUDED
