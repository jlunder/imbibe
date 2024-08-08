#ifndef __IMSTRING_H_INCLUDED
#define __IMSTRING_H_INCLUDED


// Immutable string
class imstring {
public:
  bool operator < (imstring const & other) const { (void)other; return true; }
private:
  static char s_dynamic_pool[];
};





#endif // __IMSTRING_H_INCLUDED

