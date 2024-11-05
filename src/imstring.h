#ifndef __IMSTRING_H_INCLUDED
#define __IMSTRING_H_INCLUDED

#define IMSTRING_DATA __based(__segname("imstring_DATA"))

namespace aux_imstring {
static segsize_t const s_page_size = 16;
static segsize_t const s_dynamic_pool_size = s_page_size * 2048;

extern uint8_t IMSTRING_DATA s_dynamic_pool[s_dynamic_pool_size];

static inline bool is_dynamic(char const __far *str) {
#if BUILD_MSDOS
  return FP_SEG(str) == __segname("imstring_DATA");
#elif BUILD_POSIX
  return ((uint8_t const __far *)str >= aux_imstring::s_dynamic_pool) &&
         ((uint8_t const __far *)str <
          (aux_imstring::s_dynamic_pool +
           LENGTHOF(aux_imstring::s_dynamic_pool)));
#else
#error New platform support needed?
#endif
}
} // namespace aux_imstring

// Immutable string
class imstring {
public:
  imstring() : m_str(NULL) {}
  explicit imstring(char const __far *n_str) : m_str(n_str) {
    if (aux_imstring::is_dynamic(m_str)) {
      ref_dynamic(m_str);
    }
  }
  imstring(imstring const &other) {
    m_str = other.m_str;
    if (aux_imstring::is_dynamic(m_str)) {
      ref_dynamic(m_str);
    }
  }
  ~imstring() {
    if (aux_imstring::is_dynamic(m_str)) {
      unref_dynamic(m_str);
    }
#if BUILD_DEBUG
    m_str = NULL;
#endif
  }

  bool null() const { return !m_str; }
  bool empty() const {
    assert(m_str);
    return !*m_str;
  }
  bool null_or_empty() const { return null() || empty(); }
  char const __far *c_str() const { return m_str; }

  segsize_t length() const {
    assert(m_str);
    return _fstrlen(m_str);
  }
  char at(segsize_t pos) const {
    assert(m_str);
    assert(pos < length());
    return m_str[pos];
  }
  char operator[](segsize_t pos) const {
    assert(m_str);
    assert(pos < length());
    return m_str[pos];
  }

  int compare(imstring const &other) { return _fstrcmp(m_str, other.m_str); }

  // operator char const __far *() const { return c_str(); }

  imstring &operator=(imstring const &other) {
    if (aux_imstring::is_dynamic(m_str)) {
      unref_dynamic(m_str);
    }
    m_str = other.m_str;
    if (aux_imstring::is_dynamic(m_str)) {
      ref_dynamic(m_str);
    }
    return *this;
  }

  static imstring copy(char const __far *str, segsize_t len = SEGSIZE_MAX);
  static imstring format(char const *fmt, ...);

  static void setup();
  static void teardown();
  static void teardown_exiting();

private:
  struct noref {};

  char const __far *m_str;
  static char __far *alloc_dynamic(segsize_t raw_size);
  static void ref_dynamic(char const __far *str);
  static void unref_dynamic(char const __far *str);

  explicit imstring(noref, char const __far *n_str) : m_str(n_str) {}
};

static_assert(sizeof(imstring) == sizeof(void __far *));

inline bool operator==(imstring const &x, imstring const &y) {
  return _fstrcmp(x.c_str(), y.c_str()) == 0;
}
inline bool operator==(imstring const &x, char const __far *y) {
  return _fstrcmp(x.c_str(), y) == 0;
}
inline bool operator==(char const __far *x, imstring const &y) {
  return _fstrcmp(x, y.c_str()) == 0;
}
inline bool operator!=(imstring const &x, imstring const &y) {
  return _fstrcmp(x.c_str(), y.c_str()) != 0;
}
inline bool operator!=(imstring const &x, char const __far *y) {
  return _fstrcmp(x.c_str(), y) != 0;
}
inline bool operator!=(char const __far *x, imstring const &y) {
  return _fstrcmp(x, y.c_str()) != 0;
}
inline bool operator<(imstring const &x, imstring const &y) {
  return _fstrcmp(x.c_str(), y.c_str()) < 0;
}
inline bool operator<(imstring const &x, char const __far *y) {
  return _fstrcmp(x.c_str(), y) < 0;
}
inline bool operator<(char const __far *x, imstring const &y) {
  return _fstrcmp(x, y.c_str()) < 0;
}
inline bool operator>=(imstring const &x, imstring const &y) {
  return _fstrcmp(x.c_str(), y.c_str()) >= 0;
}
inline bool operator>=(imstring const &x, char const __far *y) {
  return _fstrcmp(x.c_str(), y) >= 0;
}
inline bool operator>=(char const __far *x, imstring const &y) {
  return _fstrcmp(x, y.c_str()) >= 0;
}
inline bool operator>(imstring const &x, imstring const &y) {
  return _fstrcmp(x.c_str(), y.c_str()) > 0;
}
inline bool operator>(imstring const &x, char const __far *y) {
  return _fstrcmp(x.c_str(), y) > 0;
}
inline bool operator>(char const __far *x, imstring const &y) {
  return _fstrcmp(x, y.c_str()) > 0;
}
inline bool operator<=(imstring const &x, imstring const &y) {
  return _fstrcmp(x.c_str(), y.c_str()) <= 0;
}
inline bool operator<=(imstring const &x, char const __far *y) {
  return _fstrcmp(x.c_str(), y) <= 0;
}
inline bool operator<=(char const __far *x, imstring const &y) {
  return _fstrcmp(x, y.c_str()) <= 0;
}

#endif // __IMSTRING_H_INCLUDED
