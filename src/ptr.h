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

typedef uint16_t segsize_t;
typedef int16_t segdiff_t;

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

#endif // __PTR_H_INCLUDED
