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

#define SEGSIZE_MAX UINT16_MAX
#define SEGSIZE_INVALID UINT16_MAX
#define SEGDIFF_MAX INT16_MAX
#define SEGDIFF_MIN INT16_MIN

inline void const __far *normalize_segmented(void const __far *p) {
#if BUILD_MSDOS
  void const __far *norm_p =
      MK_FP(FP_SEG(p) + (FP_OFF(p) >> 4), FP_OFF(p) & 0xF);
#elif BUILD_POSIX
  void const __far *norm_p = p;
#else
#error New platform support needed?
#endif
  // enable_logf("normalized "PRpF" to "PRpF"\n", p, norm_p);
  assert(FP_SEG(norm_p) - FP_SEG(p) < 0x8000);
  return norm_p;
}

inline void __far *normalize_segmented(void __far *p) {
  return const_cast<void __far *>(
      normalize_segmented(reinterpret_cast<void const __far *>(p)));
}

inline void const __far *denormalize_segmented(__segment orig_seg,
                                               void const __far *norm_p) {
#if BUILD_MSDOS
  void const __far *p =
      MK_FP(orig_seg, FP_OFF(norm_p) + ((FP_SEG(norm_p) - orig_seg) << 4));
#elif BUILD_POSIX
  void const __far *p = norm_p;
#else
#error New platform support needed?
#endif
  // enable_logf("denormalized "PRpF" to "PRpF"\n", norm_p, p);
  assert(FP_SEG(p) == orig_seg);
  assert(FP_SEG(norm_p) - orig_seg < 0x8000);
  return p;
}

inline void __far *denormalize_segmented(__segment orig_seg,
                                         void __far *norm_p) {
  return const_cast<void __far *>(denormalize_segmented(
      orig_seg, reinterpret_cast<void const __far *>(norm_p)));
}

template <class T> class ofsp {
public:
  ofsp() {}
#if defined(__WATCOMC__)
  ofsp(ofsp const &other) : m_ofs(other.m_ofs) {}
#else
  ofsp(ofsp const &other) = default;
#endif
  explicit ofsp(segsize_t i) : m_ofs((static_cast<T __near *>(0)) + i) {}
  explicit ofsp(T __near *n_ofs) : m_ofs(n_ofs) {}
  template <class U> explicit ofsp(ofsp<U> other) : m_ofs(other.m_ofs) {
    assert(static_cast<U __near *>(m_ofs) == other.m_ofs);
  }
  T __near *ofs() const { return m_ofs; }
  ofsp operator+(segdiff_t n) const { return ofsp(m_ofs + n); }
  ofsp operator-(segdiff_t n) const { return ofsp(m_ofs - n); }
  segdiff_t operator-(ofsp other) const { return m_ofs - other.m_ofs; }
#if !defined(__WATCOMC__)
  ofsp &operator=(ofsp const &other) = default;
#endif
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
#if defined(__WATCOMC__)
  segp(segp const &other) : m_seg(other.m_seg) {}
#else
  segp(segp const &other) = default;
#endif
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
#if !defined(__WATCOMC__)
  segp &operator=(segp const &other) = default;
#endif

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

#endif // __PTR_H_INCLUDED
