#include <stddef.h>


class in_stream
{
public:
  void read(unsigned char * x, size_t n);
};


void in_stream::read(unsigned char * x, size_t n)
{
}


template < class T >
inline in_stream & operator >> (in_stream & i, T & x)
{
  i.read((unsigned char *)&x, sizeof(T));
  return i;
}


