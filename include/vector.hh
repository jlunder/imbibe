#ifndef __VECTOR_HH_INCLUDED
#define __VECTOR_HH_INCLUDED


#include "imbibe.hh"


template < class T >
class vector
{
public:
  //no reverse iterators, no swap, and pointers used as iterators. oh well.
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T & reference;
  typedef T const & const_reference;
  typedef T * iterator;
  typedef T const * const_iterator;

  vector();
  vector(vector < T > const & n_vector);
  vector(size_type n, T const & x = T());
  vector(const_iterator first, const_iterator last);
  ~vector();
  void reserve(size_type n);
  size_type capacity() const;
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  void resize(size_type n, T const & x = T());
  size_type size() const;
  size_type max_size() const;
  bool empty() const;
  reference at(size_type pos);
  const_reference at(size_type pos) const;
  reference operator [](size_type pos);
  const_reference operator [](size_type pos) const;
  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;
  void push_back(T const & x);
  void pop_back();
  void assign(const_iterator first, const_iterator last);
  void assign(size_type n, T const & x = T());
  iterator insert(iterator it, T const & x = T());
  void insert(iterator it, size_type n, T const & x = T());
  void insert(iterator it, const_iterator first, const_iterator last);
  void erase(iterator it);
  void erase(iterator first, iterator last);
  void clear();
  vector < T > & operator =(vector < T > const & x);

private:
  size_type m_size;
  size_type m_capacity;
  T * m_data;
};


//#include "vector.ii"


#endif //__VECTOR_HH_INCLUDED


