#ifndef __UNPACKER_H_INCLUDED
#define __UNPACKER_H_INCLUDED

#include "imbibe.h"

class imstring;

class unpacker {
public:
  unpacker() : m_seg(0) {}
  explicit unpacker(void const __far *n_ptr, segsize_t n_size)
      : m_base(reinterpret_cast<uint8_t const __near *>(FP_OFF(n_ptr))),
        m_end(reinterpret_cast<uint8_t const __near *>(FP_OFF(n_ptr) + n_size)),
        m_cur(reinterpret_cast<uint8_t const __near *>(FP_OFF(n_ptr))),
        m_seg(FP_SEG(n_ptr)) {
    assert(m_end >= m_base);
    assert(m_end - m_base == n_size);
    assert(&m_seg[m_base + n_size / 2] == &m_seg[m_base] + n_size / 2);
  }

  void const __far *base() {
    return reinterpret_cast<void const __far *>(&m_seg[m_base]);
  }

  segsize_t size() const { return m_end - m_base; }

  segsize_t pos() const { return m_cur - m_base; }

  segsize_t remain() const { return m_end - m_cur; }

  template <class T> bool fits() const { return remain() >= sizeof(T); }

  template <class T> bool fits_array(segsize_t count) const {
    return remain() >= sizeof(T) * (uint32_t)count;
  }

  bool fits_untyped(segsize_t sz) const { return remain() >= sz; }

  bool fits_string(segsize_t *out_len = NULL) const;

  void seek_to(segsize_t ofs) {
    assert(ofs <= size());
    m_cur = m_base + ofs;
  }

  void reset() { m_cur = m_base; }

  template <class T> T const __far &peek() const {
    assert(sizeof(T) < remain());
    return *reinterpret_cast<T const __far *>(&m_seg[m_cur]);
  }

  template <class T> T const __far *peek_array() const {
    return reinterpret_cast<T const __far *>(&m_seg[m_cur]);
  }

  template <class T> T const __far *peek_array(segsize_t count) const {
    assert(sizeof(T) * (uint32_t)count <= remain());
    return reinterpret_cast<T const __far *>(&m_seg[m_cur]);
  }

  char const __far *peek_string() {
    return reinterpret_cast<char const __far *>(m_seg + m_cur);
  }

  void const __far *peek_untyped() const {
    return reinterpret_cast<void const __far *>(&m_seg[m_cur]);
  }

  void const __far *peek_untyped(segsize_t sz) const {
    assert(sz <= remain());
    (void)sz;
    return &m_seg[m_cur];
  }

  segsize_t check_unpad(segsize_t sz) const {
    assert(sz == 2 || sz == 4 || sz == 8 || sz == 16);
    return (m_cur - m_base) & ~(sz - 1);
  }

  template <class T> void skip() { skip_untyped(sizeof(T)); }

  template <class T> void skip_array(segsize_t count) {
    skip_untyped(sizeof(T) * count);
  }

  void skip_untyped(segsize_t sz) {
    assert(sz <= remain());
    m_cur += sz;
  }

  void skip_string(segsize_t *out_len = NULL);

  template <class T> bool try_skip() { return try_skip_untyped(sizeof(T)); }

  template <class T> bool try_skip_array(segsize_t count) {
    return try_skip_untyped(sizeof(T) * count);
  }

  bool try_skip_untyped(segsize_t sz);
  bool try_skip_string(segsize_t *out_len = NULL);

  segsize_t pad(segsize_t sz) {
    assert(sz == 2 || sz == 4 || sz == 8 || sz == 16);
    assert(((uintptr_t)m_base.ofs() & ~(sz - 1)) == 0);
    segsize_t dist = ((m_cur - m_base) + sz - 1) & ~(sz - 1);
    skip_untyped(dist);
    return dist;
  }

  template <class T> segsize_t pad() { return pad(sizeof(T)); }

  template <class T> T const __far &unpack() {
    assert(sizeof(T) <= remain());
    T const __far *p = reinterpret_cast<T const __far *>(&m_seg[m_cur]);
    m_cur += sizeof(T);
    return *p;
  }

  template <class T> T const __far *unpack_array(segsize_t count) {
    assert(sizeof(T) * (uint32_t)count <= remain());
    T const __far *p = reinterpret_cast<T const __far *>(&m_seg[m_cur]);
    m_cur += sizeof(T) * count;
    return p;
  }

  void const __far *unpack_untyped(segsize_t sz) {
    assert(sz <= remain());
    void const __far *p = reinterpret_cast<void const __far *>(&m_seg[m_cur]);
    m_cur += sz;
    return p;
  }

  char const __far *unpack_string(segsize_t *out_len = NULL);

  template <class T> bool try_unpack(T *out_t) {
    assert(out_t);
    if (!fits<T>()) {
      return false;
    }
    *out_t = unpack<T>();
    return true;
  }

  template <class T>
  bool try_unpack_array(segsize_t count, T const __far **out_arr) {
    assert(out_arr);
    if (!fits_array<T>(count)) {
      return false;
    }
    *out_arr = static_cast<T const __far *>(unpack_array<T>(count));
    return true;
  }

  bool try_unpack_untyped(segsize_t sz, void const __far **out_ptr);
  bool try_unpack_string(char const __far **out_str, segsize_t *out_len = NULL);
  bool try_unpack_string(imstring *out_str, segsize_t *out_len = NULL);

  void subrange(segsize_t sz);

  bool try_subrange(segsize_t sz);

private:
  ofsp<uint8_t const> m_base;
  ofsp<uint8_t const> m_end;
  ofsp<uint8_t const> m_cur;
  segp<uint8_t const> m_seg;

  ofsp<uint8_t const> scan_string() const;
};

#endif // __UNPACKER_H_INCLUDED
