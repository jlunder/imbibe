#ifndef __VECTOR_H_INCLUDED
#define __VECTOR_H_INCLUDED

#include "imbibe.h"

#define logf_vector(...) disable_logf("VECTOR: " __VA_ARGS__)

namespace aux_vector {
template <class T> inline T *allocate(uint_fast16_t n) {
  assert(n > 0);
  T *p = (T *)arena::cur_alloc(n * sizeof(T));
  assert(p);
  return p;
}

template <class T> inline void free(T *p) {
  assert(p);
  arena::cur_free(p);
}

template <class T> inline void construct(T *p, uint_fast16_t n) {
  for (uint_fast16_t i = 0; i < n; ++i) {
    new (&p[i]) T;
  }
}

template <class T> inline void destroy(T *p, uint_fast16_t n) {
  for (uint_fast16_t i = 0; i < n; ++i) {
    p[i].~T();
  }
}

template <class T> inline void copy(T const *src, T *dest, uint_fast16_t n) {
  for (uint_fast16_t i = 0; i < n; ++i) {
    dest[i] = src[i];
  }
}

template <class T>
inline void copy_construct(T const *src, T *dest, uint_fast16_t n) {
  for (uint_fast16_t i = 0; i < n; ++i) {
    new (&dest[i]) T(src[i]);
  }
}

template <class T>
inline void copy_construct(T const &src, T *dest, uint_fast16_t n) {
  for (uint_fast16_t i = 0; i < n; ++i) {
    new (&dest[i]) T(src);
  }
}

template <class T> inline void move(T *src, T *dest) {
  new (dest) T(*src);
  src->~T();
}

template <class T> inline void move(T *src, T *dest, uint_fast16_t n) {
  if ((src == dest) || (n == 0)) {
    // do nothing
  } else if (n == 1) {
    move<T>(src, dest);
  } else if ((dest < src) || (src + n <= dest)) {
    for (uint_fast16_t i = 0; i < n; ++i) {
      new (&dest[i]) T(src[i]);
      src[i].~T();
    }
  } else {
    // Overlapping range
    for (uint_fast16_t i = n; i > 0;) {
      --i;
      new (&dest[i]) T(src[i]);
      src[i].~T();
    }
  }
}
} // namespace aux_vector

template <class T> class vector {
public:
  // no reverse iterators, no swap, and pointers used as iterators. oh well.
  typedef uint16_t size_type;
  typedef int16_t difference_type;
  typedef T &reference;
  typedef T const &const_reference;
  typedef T *iterator;
  typedef T const *const_iterator;

  static size_type const c_size_max = (UINT16_MAX - 31) / sizeof(T);

  vector() : m_mem(NULL), m_size(0), m_capacity(0), m_data(NULL) {}

  vector(vector const &x)
      : m_mem(arena::cur()), m_size(x.m_size),
        m_capacity(round_up(x.m_capacity)) {
    m_data = aux_vector::allocate<T>(m_capacity);
    aux_vector::copy_construct<T>(x.begin(), m_data, m_size);
  }

  explicit vector(arena *n_mem)
      : m_mem(n_mem), m_size(0), m_capacity(0), m_data(NULL) {}

  vector(arena *n_mem, vector const &x)
      : m_mem(n_mem), m_mem(arena::cur()), m_size(x.m_size),
        m_capacity(round_up(x.m_capacity)) {
    m_data = aux_vector::allocate<T>(m_capacity);
    aux_vector::copy_construct<T>(x.begin(), m_data, m_size);
  }

  ~vector() {
    assert((m_size == 0) || m_data);
    if (m_data) {
      assert(m_mem);
      aux_vector::destroy<T>(m_data, m_size);
      aux_vector::free<T>(m_data);
    }
#ifndef NDEBUG
    m_data = NULL;
#endif
  }

  void mem(arena *n_mem) {
    assert(!m_mem || (n_mem == m_mem));
    m_mem = n_mem;
  }

  void reserve(size_type n) {
    assert(n <= c_size_max);
    if (n <= m_capacity) {
      return;
    }
    with_arena with(m_mem);
    T *n_data = aux_vector::allocate<T>(n);
    assert(m_data || ((m_size == 0) && (m_capacity == 0)));
    if (m_data) {
      aux_vector::copy_construct<T>(m_data, n_data, m_size);
      aux_vector::destroy<T>(m_data, m_size);
      aux_vector::free<T>(m_data);
    }
    m_data = n_data;
    m_capacity = n;
  }

  arena *mem() const { return m_mem; }

  size_type capacity() const { return m_capacity; }
  iterator begin() { return m_data; }
  const_iterator begin() const { return m_data; }
  iterator end() { return m_data + m_size; }
  const_iterator end() const { return m_data + m_size; }

  void resize(size_type n, T const &x) {
    assert(n <= c_size_max);
    reserve(n);
    with_arena with(m_mem);
    if (n > m_size) {
      aux_vector::copy_construct<T>(x, m_data + m_size, n - m_size);
    } else if (n < m_size) {
      aux_vector::destroy<T>(m_data + n, m_size - n);
    }
    m_size = n;
  }

  size_type size() const { return m_size; }
  size_type max_size() const { return -1; }
  bool empty() const { return m_size == 0; }

  reference at(size_type pos) {
    assert(pos < m_size);
    return m_data[pos];
  }
  const_reference at(size_type pos) const {
    assert(pos < m_size);
    return m_data[pos];
  }
  reference operator[](size_type pos) {
    assert(pos < m_size);
    return m_data[pos];
  }
  const_reference operator[](size_type pos) const {
    assert(pos < m_size);
    return m_data[pos];
  }
  reference front() {
    assert(m_size > 0);
    return m_data[0];
  }
  const_reference front() const {
    assert(m_size > 0);
    return m_data[0];
  }
  reference back() {
    assert(m_size > 0);
    return m_data[m_size - 1];
  }
  const_reference back() const {
    assert(m_size > 0);
    return m_data[m_size - 1];
  }

  void push_back(T const &x) {
    assert(m_size < c_size_max);
    insert(end(), x);
  }

  void pop_back() {
    assert(m_size > 0);
    erase(end() - 1);
  }

  void assign(const_iterator first, const_iterator last) {
    assert(last >= first);
    assert(last - first < c_size_max);
    size_type n = last - first;
    assert(m_size <= c_size_max - n);
    clear();
    if(n == 0) {
      return;
    }
    reserve(n);
    with_arena with(m_mem);
    aux_vector::copy_construct<T>(first, m_data, n);
    m_size = n;
  }

  void assign(size_type n, T const &x) {
    assert(n < c_size_max);
    clear();
    if(n == 0) {
      return;
    }
    reserve(n);
    with_arena with(m_mem);
    aux_vector::copy_construct<T>(x, m_data, n);
    m_size = n;
  }

  iterator insert(iterator it, T const &x) {
    assert(m_size < c_size_max);
    return insert(it, 1, x);
  }

  iterator insert(iterator it, size_type n, T const &x) {
    assert(it >= m_data);
    assert(it - m_data <= m_size);
    assert(m_size <= c_size_max - n);
    size_type i = it - m_data;
    assert(i <= m_size);
    size_type m = m_size - i;
    with_arena with(m_mem);
    if (m_capacity >= m_size + n) {
      aux_vector::move<T>(it, it + n, m);
      aux_vector::copy_construct(x, it, n);
    } else {
      size_type n_capacity = round_up(m_size + n);
      T *n_data = aux_vector::allocate<T>(n_capacity);
      if (m_data) {
        aux_vector::move<T>(m_data, n_data, i);
        aux_vector::copy_construct(x, n_data + i, n);
        aux_vector::move<T>(it, n_data + i + n, m);
        aux_vector::free<T>(m_data);
      } else {
        aux_vector::copy_construct(x, n_data + i, n);
      }
      m_data = n_data;
      m_capacity = n_capacity;
    }
    m_size += n;
    return begin() + i;
  }

  iterator insert(iterator it, const_iterator first, const_iterator last) {
    assert(it >= m_data);
    assert(it - m_data <= m_size);
    assert(last - first >= 0);
    size_type n = last - first;
    assert(m_size <= c_size_max - n);
    size_type i = it - m_data;
    assert(i <= m_size);
    size_type m = m_size - i;
    with_arena with(m_mem);
    if (m_capacity >= m_size + n) {
      aux_vector::move<T>(it, it + n, m);
      aux_vector::copy_construct(first, it, n);
    } else {
      size_type n_capacity = round_up(m_size + n);
      T *n_data = aux_vector::allocate<T>(n_capacity);
      if (m_data) {
        aux_vector::move<T>(m_data, n_data, i);
        aux_vector::copy_construct(first, n_data + i, n);
        aux_vector::move<T>(m_data + i, n_data + i + n, m);
        aux_vector::free<T>(m_data);
      } else {
        aux_vector::copy_construct(first, n_data + i, n);
      }
      m_data = n_data;
      m_capacity = n_capacity;
    }
    m_size += n;
    return begin() + i;
  }

  void erase(iterator it) {
    assert(it >= m_data);
    assert(it - m_data < m_size);
    erase(it, it + 1);
  }

  void erase(iterator first, iterator last) {
    assert(first >= m_data);
    assert(first - m_data <= m_size);
    assert(last >= m_data);
    assert(last - m_data <= m_size);
    assert(first <= last);
    if (first == last) {
      return;
    }
    size_type n = last - first;
    size_type m = m_size - (last - m_data);
    T *p = first;
    T *q = last;
    for (size_type i = 0; i < m; ++i, ++p, ++q) {
      *p = *q;
    }
    m_size -= n;
    assert(m_mem);
    aux_vector::destroy<T>(last, n);
  }

  void clear() {
    if (m_size == 0) {
      return;
    }
    assert(m_mem);
    aux_vector::destroy<T>(m_data, m_size);
    m_size = 0;
  }

  vector<T> &operator=(vector<T> const &x) {
    assign(x.begin(), x.end());
    return *this;
  }

private:
  arena *m_mem;
  size_type m_size;
  size_type m_capacity;
  T *m_data;

  static size_type round_up(size_type n) {
    assert(n < (c_size_max / 3 * 2));
    assert(sizeof(T) <= c_size_max / 4);
    size_type r = (n * 3 + 1) / 2;
    r = max<size_type>(r, (16 + sizeof(T) - 1) / sizeof(T));
    assert(c_size_max > 1);
    assert(r < c_size_max);
    assert(r >= n);
    return r;
  }
};

#endif // __VECTOR_H_INCLUDED
