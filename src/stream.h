#ifndef __STREAMS_H_INCLUDED
#define __STREAMS_H_INCLUDED


#include "imbibe.h"


class stream {
public:
  enum error_t {
    err_none,
    err_not_supported,
    err_unknown
  };

  typedef int32_t ssize_t;

  stream():
    m_last_error(err_none), m_at_eof(false), m_size(0), m_position(0)
    { }
  virtual ~stream() { }

  ssize_t size() { return m_size; }
  ssize_t position() { return m_position; }
  bool at_eof() { return m_at_eof; }
  error_t last_error() { return m_last_error; }

  virtual void reset() {
    m_last_error = err_none;
    seek_to(0);
  }
  virtual void sync() { }
  virtual ssize_t seek_by(ssize_t dist) {
    assert(m_last_error == err_none);
    assert(position() <= INT32_MAX / 4);
    assert(dist >= -(INT32_MAX / 4));
    assert(dist <= INT32_MAX / 4);
    ssize_t new_pos = position() + dist;
    assert(new_pos >= 0);
    return seek_to(new_pos);
  }
  virtual ssize_t seek_to(ssize_t pos) = 0;
  virtual ssize_t read(void * dest, size_t size) {
    (void)dest;
    (void)size;
    if (m_last_error != err_none) {
      return 0;
    }
    m_last_error = err_not_supported;
    return 0;
  }
  virtual ssize_t write(void * src, size_t size) {
    (void)src;
    (void)size;
    if (m_last_error != err_none) {
      return 0;
    }
    m_last_error = err_not_supported;
    return 0;
  }

private:
  error_t m_last_error;
  bool m_at_eof;
  ssize_t m_size;
  ssize_t m_position;
};


#endif // __STREAMS_H_INCLUDED

