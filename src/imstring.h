#ifndef __IMSTRING_H_INCLUDED
#define __IMSTRING_H_INCLUDED

// Immutable string
class imstring {
public:
  imstring() : m_str(NULL) {}
  imstring(char const __far *n_str) : m_str(n_str) {
    assert(!is_dynamic(n_str));
  }
  imstring(imstring const &other) {
    m_str = other.m_str;
    if (is_dynamic(m_str)) {
      ref_dynamic(*this);
    }
  }
  ~imstring() {
    if (is_dynamic(m_str)) {
      unref_dynamic(*this);
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

  int compare_to(imstring const &other) { return compare(*this, other); }

  //operator char const __far *() const { return c_str(); }

  imstring &operator=(imstring const &other) {
    if (is_dynamic(m_str)) {
      unref_dynamic(*this);
    }
    m_str = other.m_str;
    if (is_dynamic(m_str)) {
      ref_dynamic(*this);
    }
    return *this;
  }

  static int compare(imstring const &x, imstring const &y) {
    return _fstrcmp(x.c_str(), y.c_str());
  }
  static imstring copy(char const __far *str) {
    imstring result;
    copy_dynamic(result, str);
    return result;
  }

  static void setup();
  static void teardown();
  static void teardown_exiting();

private:
#if BUILD_MSDOS
  static __segment s_dynamic_seg;
#endif

  char const __far *m_str;

#if BUILD_MSDOS
  static bool is_dynamic(char const __far *str) {
    // return FP_SEG(str) == s_dynamic_seg;
    (void)str;
    return false;
  }
#elif BUILD_POSIX
  static bool is_dynamic(char const __far *str);
#else
#error New platform support needed?
#endif
  static void copy_dynamic(imstring &ims, char const __far *str);
  static void ref_dynamic(imstring &ims);
  static void unref_dynamic(imstring &ims);
};

inline bool operator==(imstring const &x, imstring const &y) {
  return imstring::compare(x, y) == 0;
}
inline bool operator!=(imstring const &x, imstring const &y) {
  return imstring::compare(x, y) != 0;
}
inline bool operator<(imstring const &x, imstring const &y) {
  return imstring::compare(x, y) < 0;
}
inline bool operator>=(imstring const &x, imstring const &y) {
  return imstring::compare(x, y) >= 0;
}
inline bool operator>(imstring const &x, imstring const &y) {
  return imstring::compare(x, y) > 0;
}
inline bool operator<=(imstring const &x, imstring const &y) {
  return imstring::compare(x, y) <= 0;
}

#endif // __IMSTRING_H_INCLUDED
