#ifndef __FUNCTIONAL_HH_INCLUDED
#define __FUNCTIONAL_HH_INCLUDED


#include "imbibe.hh"


template < class T >
class less
{
public:
  bool operator ()(T const & a, T const & b) const;
};


#endif //__FUNCTIONAL_HH_INCLUDED


