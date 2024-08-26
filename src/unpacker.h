#ifndef __UNPACKER_H_INCLUDED
#define __UNPACKER_H_INCLUDED


#include "imbibe.h"


class unpacker {
public:
  unpacker(): m_base(0), m_end(0), m_cur(0), m_seg(0) { }
  explicit unpacker(void const __far * n_ptr, uint16_t n_size)
    : m_base((uint8_t const __near *)FP_OFF(n_ptr)),
      m_end((uint8_t const __near *)(FP_OFF(n_ptr) + n_size)),
      m_cur((uint8_t const __near *)FP_OFF(n_ptr)),
      m_seg(FP_SEG(n_ptr)) {
    assert(m_end >= m_base);
    assert(m_end - m_base == n_size);
    assert(MK_FP(m_seg, m_base + n_size / 2)
      == (uint8_t __far *)MK_FP(m_seg, m_base) + n_size / 2);
  }

  void const __far * base() { return MK_FP(m_seg, m_base); }

  uint16_t size() const { return m_end - m_base; }

  uint16_t pos() const { return m_cur - m_base; }

  uint16_t remain() const { return m_end - m_cur; }

  template<class T>
  bool fits() const { return remain() >= sizeof (T); }

  template<class T>
  bool fits_array(uint16_t count) const { return remain() >= sizeof (T) * (uint32_t)count; }

  bool fits_untyped(uint16_t sz) const { return remain() >= sz; }

  void seek_to(uint16_t ofs) {
    assert(ofs <= size());
    m_cur = m_base + ofs;
  }

  void reset() {
    m_cur = m_base;
  }

  template<class T>
  T const & peek() const {
    assert(sizeof (T) < remain());
    return *(T const __far *)MK_FP(m_seg, m_cur);
  }

  template<class T>
  T const * peek_array() const {
    return (T const __far *)MK_FP(m_seg, m_cur);
  }

  template<class T>
  T const * peek_array(uint16_t count) const {
    assert(sizeof (T) * (uint32_t)count <= remain());
    return (T const __far *)MK_FP(m_seg, m_cur);
  }

  void const * peek_untyped() const { return MK_FP(m_seg, m_cur); }

  void const * peek_untyped(uint16_t sz) const {
    assert(sz <= remain());
    return MK_FP(m_seg, m_cur);
  }

  uint16_t check_unpad(uint16_t sz) const {
    assert(sz == 2 || sz == 4 || sz == 8 || sz == 16);
    return (m_cur - m_base) & ~(sz - 1);
  }

  template<class T>
  void skip() { skip(sizeof (T)); }

  void skip(uint16_t sz) {
    assert(sz <= remain());
    m_cur += sz;
  }

  uint16_t pad(uint16_t sz) {
    assert(sz == 2 || sz == 4 || sz == 8 || sz == 16);
    assert(((uintptr_t)m_base & ~(sz - 1)) == 0);
    uint16_t dist = ((m_cur - m_base) + sz - 1) & ~(sz - 1);
    skip(dist);
    return dist;
  }

  template<class T>
  uint16_t pad() { return pad(sizeof (T)); }

  template<class T>
  T const __far & unpack() {
    assert(sizeof (T) <= remain());
    T const __near * p = (T const __near *)m_cur;
    m_cur += sizeof (T);
    return *(T const __far *)MK_FP(m_seg, p);
  }

  template<class T>
  T const __far * unpack_array(uint16_t count) {
    assert(sizeof (T) * (uint32_t)count <= remain());
    T const __near * p = (T const __near *)m_cur;
    m_cur += sizeof (T) * count;
    return (T const __far *)MK_FP(m_seg, p);
  }

  void const __far * unpack_untyped(uint16_t sz) {
    assert(sz <= remain());
    void const __near * p = m_cur;
    m_cur += sz;
    return MK_FP(m_seg, p);
  }

private:
  uint8_t const __near * m_base;
  uint8_t const __near * m_end;
  uint8_t const __near * m_cur;
  __segment m_seg;
};


#endif // __UNPACKER_H_INCLUDED


