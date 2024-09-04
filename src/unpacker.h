#ifndef __UNPACKER_H_INCLUDED
#define __UNPACKER_H_INCLUDED

#include "imbibe.h"

class unpacker {
public:
  unpacker() : m_seg(0) {}
  explicit unpacker(void const __far *n_ptr, segsize_t n_size)
      : m_base((uint8_t const __near *)FP_OFF(n_ptr)),
        m_end((uint8_t const __near *)(FP_OFF(n_ptr) + n_size)),
        m_cur((uint8_t const __near *)FP_OFF(n_ptr)), m_seg(FP_SEG(n_ptr)) {
    assert(m_end >= m_base);
    assert(m_end - m_base == n_size);
    assert(&m_seg[m_base + n_size / 2] == &m_seg[m_base] + n_size / 2);
  }

  void const __far *base() { return (void const __far *)&m_seg[m_base]; }

  segsize_t size() const { return m_end - m_base; }

  segsize_t pos() const { return m_cur - m_base; }

  segsize_t remain() const { return m_end - m_cur; }

  template <class T> bool fits() const { return remain() >= sizeof(T); }

  template <class T> bool fits_array(segsize_t count) const {
    return remain() >= sizeof(T) * (uint32_t)count;
  }

  bool fits_untyped(segsize_t sz) const { return remain() >= sz; }

  void seek_to(segsize_t ofs) {
    assert(ofs <= size());
    m_cur = m_base + ofs;
  }

  void reset() { m_cur = m_base; }

  template <class T> T const __far &peek() const {
    assert(sizeof(T) < remain());
    return *(T const __far *)&m_seg[m_cur];
  }

  template <class T> T const __far *peek_array() const {
    return (T const __far *)&m_seg[m_cur];
  }

  template <class T> T const __far *peek_array(segsize_t count) const {
    assert(sizeof(T) * (uint32_t)count <= remain());
    return (T const __far *)&m_seg[m_cur];
  }

  void const __far *peek_untyped() const {
    return (void const __far *)&m_seg[m_cur];
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

  template <class T> void skip() { skip(sizeof(T)); }

  void skip(segsize_t sz) {
    assert(sz <= remain());
    m_cur += sz;
  }

  segsize_t pad(segsize_t sz) {
    assert(sz == 2 || sz == 4 || sz == 8 || sz == 16);
    assert(((uintptr_t)m_base.ofs() & ~(sz - 1)) == 0);
    segsize_t dist = ((m_cur - m_base) + sz - 1) & ~(sz - 1);
    skip(dist);
    return dist;
  }

  template <class T> segsize_t pad() { return pad(sizeof(T)); }

  template <class T> T const __far &unpack() {
    assert(sizeof(T) <= remain());
    T const __far *p = (T const __far *)&m_seg[m_cur];
    m_cur += sizeof(T);
    return *p;
  }

  template <class T> T const __far *unpack_array(segsize_t count) {
    assert(sizeof(T) * (uint32_t)count <= remain());
    T const __far *p = (T const __far *)&m_seg[m_cur];
    m_cur += sizeof(T) * count;
    return p;
  }

  void const __far *unpack_untyped(segsize_t sz) {
    assert(sz <= remain());
    void const __far *p = (void const __far *)&m_seg[m_cur];
    m_cur += sz;
    return p;
  }

private:
  ofsp<uint8_t const> m_base;
  ofsp<uint8_t const> m_end;
  ofsp<uint8_t const> m_cur;
  segp<uint8_t const> m_seg;
};

#endif // __UNPACKER_H_INCLUDED
