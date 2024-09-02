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

class arena {
public:
  virtual ~arena() {}
  virtual void __far *alloc(segsize_t sz) = 0;
  virtual void free(void __far *p) = 0;

  static arena *cur() { return s_cur; }
  static arena *temp() { return s_temp; }
  static arena *c() { return s_c; }

  static void __far *cur_alloc(segsize_t size) { return s_cur->alloc(size); }
  static void cur_free(void __far *p) { s_cur->free(p); }
  static void __far *temp_alloc(segsize_t size) { return s_temp->alloc(size); }
  static void temp_free(void __far *p) { s_temp->free(p); }

private:
  static arena *s_cur;
  static arena *s_temp;
  static arena *s_c;

  friend class c_arena;
  friend class with_arena;
  friend class with_temp_arena;
};

class with_arena {
public:
  explicit with_arena(arena *n_cur) : m_saved_cur(arena::s_cur) {
    arena::s_cur = n_cur;
#ifndef NDEBUG
    m_set_cur = n_cur;
#endif
  }
  ~with_arena() {
    assert(arena::s_cur == m_set_cur);
    arena::s_cur = m_saved_cur;
  }

private:
  arena *m_saved_cur;
#ifndef NDEBUG
  arena *m_set_cur;
#endif
};

class with_temp_arena {
public:
  explicit with_temp_arena(arena *n_temp) : m_saved_temp(arena::s_temp) {
    arena::s_temp = n_temp;
#ifndef NDEBUG
    m_set_temp = n_temp;
#endif
  }
  ~with_temp_arena() {
    assert(arena::s_cur == m_set_temp);
    arena::s_temp = m_saved_temp;
  }

private:
  arena *m_saved_temp;
#ifndef NDEBUG
  arena *m_set_temp;
#endif
};

class c_arena : public arena {
public:
  c_arena();
  virtual ~c_arena();
  virtual void __far *alloc(segsize_t sz);
  virtual void free(void __far *p);
};

class stack_arena : public arena {
public:
  struct mark_t {
    segsize_t marked_top;
    segsize_t marked_live;
  };

  stack_arena() : m_seg(0) {}
  explicit stack_arena(segsize_t n_capacity);
  virtual ~stack_arena();
  virtual void __far *alloc(segsize_t size);
  virtual void free(void __far *p);

  mark_t mark() {
    mark_t result = {m_top, m_live_count};
    return result;
  }
  void reset(mark_t n_mark) {
    assert(n_mark.marked_top <= m_top);
    assert(n_mark.marked_live == m_live_count);
    m_top = n_mark.marked_top;
    m_live_count = n_mark.marked_live;
  }

private:
  segp<uint8_t> m_seg;
  segsize_t m_top;
  segsize_t m_capacity;
  segsize_t m_live_count;
  char const *m_name;
};

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
