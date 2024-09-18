#include "imbibe.h"

#include "unpacker.h"

#include "imstring.h"

bool unpacker::fits_string(segsize_t *out_len) const {
  ofsp<uint8_t const> p = scan_string();
  if (out_len) {
    *out_len = p - m_cur;
  }
  return p < m_end;
}

void unpacker::skip_string(segsize_t *out_len) {
  assert(fits_string());
  ofsp<uint8_t const> p = scan_string();
  if (out_len) {
    *out_len = p - m_cur;
  }
  m_cur = p + 1;
}

bool unpacker::try_skip_untyped(segsize_t sz) {
  if (!fits_untyped(sz)) {
    return false;
  }
  skip_untyped(sz);
  return true;
}

bool unpacker::try_skip_string(segsize_t *out_len) {
  segsize_t len;
  if (!fits_string(&len)) {
    return false;
  }
  skip_array<char>(len + 1);
  if (out_len) {
    *out_len = len;
  }
  return true;
}

char const __far *unpacker::unpack_string(segsize_t *out_len) {
  assert(fits_string());
  char const __far *result =
      reinterpret_cast<char const __far *>(m_seg + m_cur);
  ofsp<uint8_t const> p = scan_string();
  if (out_len) {
    *out_len = p - m_cur;
  }
  m_cur = p + 1;
  return result;
}

bool unpacker::try_unpack_untyped(segsize_t sz, void const __far **out_ptr) {
  assert(out_ptr);
  if (!fits_untyped(sz)) {
    return false;
  }
  *out_ptr = unpack_untyped(sz);
  return true;
}

bool unpacker::try_unpack_string(char const __far **out_str,
                                 segsize_t *out_len) {
  assert(out_str);
  segsize_t len;
  if (!fits_string(&len)) {
    return false;
  }
  *out_str = unpack_array<char>(len + 1);
  if (out_len) {
    *out_len = len;
  }
  return true;
}

bool unpacker::try_unpack_string(imstring *out_str, segsize_t *out_len) {
  char const __far *str;
  if (!try_unpack_string(&str, out_len)) {
    return false;
  }
  *out_str = str;
  return true;
}

void unpacker::subrange(segsize_t sz) {
  assert(fits_untyped(sz));
  m_base = m_cur;
  m_end = m_cur + sz;
}

bool unpacker::try_subrange(segsize_t sz) {
  if (fits_untyped(sz)) {
    subrange(sz);
    return true;
  } else {
    return false;
  }
}

ofsp<uint8_t const> unpacker::scan_string() const {
  ofsp<uint8_t const> p;
  for (p = m_cur; (p < m_end) && m_seg[p]; ++p) {
  }
  return p;
}
