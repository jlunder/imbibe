#ifndef __PTR_H_INCLUDED
#define __PTR_H_INCLUDED

// unique
// shared
// weak
// immut
// array
// inplace

// shared -> immut

// *
// segp

#include "imbibe.h"

#define PAGE_SIZE 16

typedef uint16_t segsize_t;
typedef int16_t segdiff_t;

inline void const __far *normalize_segmented(void const __far *p) {
#ifdef SIMULATE
  void const __far *norm_p = p;
#else
  void const __far *norm_p =
      MK_FP(FP_SEG(p) + (FP_OFF(p) >> 4), FP_OFF(p) & 0xF);
#endif
  // logf("normalized %p to %p\n", p, norm_p);
  assert(FP_SEG(norm_p) - FP_SEG(p) < 0x8000);
  return norm_p;
}

inline void __far *normalize_segmented(void __far *p) {
  return (void __far *)normalize_segmented((void const __far *)p);
}

inline void const __far *denormalize_segmented(__segment orig_seg,
                                               void const __far *norm_p) {
#ifdef SIMULATE
  void const __far *p = norm_p;
#else
  void const __far *p =
      MK_FP(orig_seg, FP_OFF(norm_p) + ((FP_SEG(norm_p) - orig_seg) << 4));
#endif
  // logf("denormalized %p to %p\n", norm_p, p);
  assert(FP_SEG(p) == orig_seg);
  assert(FP_SEG(norm_p) - orig_seg < 0x8000);
  return p;
}

inline void __far *denormalize_segmented(__segment orig_seg,
                                         void __far *norm_p) {
  return (void __far *)denormalize_segmented(orig_seg,
                                             (void const __far *)norm_p);
}

template <class T> class ofsp {
public:
  ofsp() {}
  ofsp(ofsp const &other) : m_ofs(other.m_ofs) {}
  explicit ofsp(segsize_t i) : m_ofs(((T __near *)0) + i) {}
  explicit ofsp(T __near *n_ofs) : m_ofs(n_ofs) {}
  template <class U> explicit ofsp(ofsp<U> other) : m_ofs(other.m_ofs) {
    assert(static_cast<U __near *>(m_ofs) == other.m_ofs);
  }
  T __near *ofs() const { return m_ofs; }
  ofsp operator+(segdiff_t n) const { return ofsp(m_ofs + n); }
  ofsp operator-(segdiff_t n) const { return ofsp(m_ofs - n); }
  segdiff_t operator-(ofsp other) const { return m_ofs - other.m_ofs; }
  ofsp &operator+=(segdiff_t n) {
    m_ofs += n;
    return *this;
  }
  ofsp &operator-=(segdiff_t n) {
    m_ofs -= n;
    return *this;
  }
  ofsp &operator++() {
    ++m_ofs;
    return *this;
  }
  ofsp &operator++(int) {
    ofsp result(*this);
    ++m_ofs;
    return result;
  }
  ofsp operator--() {
    --m_ofs;
    return *this;
  }
  ofsp &operator--(int) {
    ofsp result(*this);
    --m_ofs;
    return result;
  }
  bool operator==(ofsp other) { return m_ofs == other.m_ofs; }
  bool operator!=(ofsp other) { return m_ofs != other.m_ofs; }
  bool operator>(ofsp other) { return m_ofs > other.m_ofs; }
  bool operator<=(ofsp other) { return m_ofs <= other.m_ofs; }
  bool operator<(ofsp other) { return m_ofs < other.m_ofs; }
  bool operator>=(ofsp other) { return m_ofs >= other.m_ofs; }

private:
  T __near *m_ofs;
};

template <class T> class segp {
public:
  typedef T __far *pointer_type;
  typedef T __far &reference_type;

  segp() {}
  segp(segp const &other) : m_seg(other.m_seg) {}
  explicit segp(__segment n_seg) : m_seg(n_seg) {}
  template <class U> explicit segp(segp<U> other) : m_seg(other.m_seg) {
    assert(static_cast<U __far *>(get()) == other.get());
  }
  pointer_type get() const { return MK_FP(m_seg, 0); }
  __segment seg() const { return m_seg; }
  operator bool() const { return m_seg != 0; }
  operator pointer_type() const { return get(); }
  bool operator==(segp other) const { return m_seg == other.m_seg; }
  bool operator!=(segp other) const { return m_seg != other.m_seg; }
  pointer_type operator+(segsize_t i) const {
    return (pointer_type)MK_FP(m_seg, i * sizeof(T));
  }
  pointer_type operator+(segdiff_t i) const { return operator+((segsize_t)i); }
  pointer_type operator+(size_t i) const { return operator+((segsize_t)i); }
  pointer_type operator+(ptrdiff_t i) const { return operator+((segsize_t)i); }
  pointer_type operator+(ofsp<T> ofs) const {
    return (pointer_type)MK_FP(m_seg, ofs.ofs());
  }
  reference_type operator[](segsize_t i) const {
    return *(pointer_type)MK_FP(m_seg, i * sizeof(T));
  }
  reference_type operator[](segdiff_t i) const {
    return operator[]((segsize_t)i);
  }
  reference_type operator[](size_t i) const { return operator[]((segsize_t)i); }
  reference_type operator[](ptrdiff_t i) const {
    return operator[]((segsize_t)i);
  }
  reference_type operator[](ofsp<T> ofs) const {
    return *(pointer_type)MK_FP(m_seg, ofs.ofs());
  }
  operator segp<T const>() const { return segp<T const>(m_seg); }

private:
  __segment m_seg;
};

template <class T> bool operator==(segp<T> x, __segment y) {
  return x.seg() == y;
}
template <class T> bool operator!=(segp<T> x, __segment y) {
  return x.seg() != y;
}
template <class T> bool operator==(__segment x, segp<T> y) {
  return x == y.seg();
}
template <class T> bool operator!=(__segment x, segp<T> y) {
  return x != y.seg();
}

#if 0

namespace shared_manager {

segsize_t alloc_shared_handle();
void ref(segsize_t handle);
bool unref(segsize_t handle);

} // namespace shared_manager

template <class T> class default_shared_traits {
public:
  typedef segsize_t handle_type;
  static void ref() {}
  static void unref() {}
};

template <class T> class shared_traits : public default_shared_traits<T> {};

template <class T, class Traits = shared_traits<T> > class shared {};

#if defined(M_I86)

// template <class T, size_t size> class array {
// public:
// };

#else

// template <class T, size_t size> class array {
// public:
//   T const &
// };

#endif

#endif

#endif // __PTR_H_INCLUDED
