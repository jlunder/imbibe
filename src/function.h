#ifndef __FUNCTIONAL_H_INCLUDED
#define __FUNCTIONAL_H_INCLUDED


#include "imbibe.h"


template<class T>
class less
{
public:
  bool operator ()(T const & a, T const & b) const;
};


template<class T>
inline bool less<T>::operator ()(T const & a, T const & b) const
{
  return a < b;
}


#endif //__FUNCTIONAL_H_INCLUDED


