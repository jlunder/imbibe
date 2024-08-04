#ifndef __VECTOR_HH_INCLUDED
#define __VECTOR_HH_INCLUDED


#include "imbibe.hh"


template<class T>
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
  vector(vector<T> const & n_vector);
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
  vector<T> & operator =(vector<T> const & x);

private:
  size_type m_size;
  size_type m_capacity;
  T * m_data;
};


template<class T>
inline vector<T>::vector():
  m_size(0), m_capacity(16)
{
  m_data = new T[m_capacity];
  assert(m_data != NULL);
}


template<class T>
inline vector<T>::vector(vector const & x):
  m_size(x.m_size), m_capacity(x.m_capacity)
{
  iterator i;
  const_iterator j;

  m_data = new T[x.m_capacity];
  assert(m_data != NULL);
  for(i = begin(), j = x.begin(); j != x.end(); ++i, ++j)
  {
    *i = *j;
  }
}


template<class T>
inline vector<T>::vector(size_type n, T const & x):
  m_size(n), m_capacity((n * 3) / 2 + 16)
{
  iterator i;

  m_data = new T[m_capacity];
  assert(m_data != NULL);
  for(i = begin(); i != end(); ++i)
  {
    *i = x;
  }
}


template<class T>
inline vector<T>::vector(const_iterator first, const_iterator last):
  m_size(last - first), m_capacity(((last - first) * 3) / 2 + 16)
{
  iterator i;
  const_iterator j;

  m_data = new T[m_capacity];
  assert(m_data != NULL);
  for(i = begin(), j = first; j != last; ++i, ++j)
  {
    *i = *j;
  }
}


template<class T>
inline vector<T>::~vector()
{
  assert(m_data != NULL);
  delete[] m_data;
}


template<class T>
inline void vector<T>::reserve(size_type n)
{
  iterator i;
  const_iterator j;
  T * old_data = m_data;

  if(n > m_capacity)
  {
    m_capacity = n;
    m_data = new T[m_capacity];
    assert(m_data != NULL);
    for(i = m_data, j = old_data; j != old_data + m_size; ++i, ++j)
    {
      *i = *j;
    }
    delete[] old_data;
  }
}


template<class T>
inline vector<T>::size_type vector<T>::capacity() const
{
  return m_capacity;
}


template<class T>
inline vector<T>::iterator vector<T>::begin()
{
  return m_data;
}


template<class T>
inline vector<T>::const_iterator vector<T>::begin() const
{
  return m_data;
}


template<class T>
inline vector<T>::iterator vector<T>::end()
{
  return m_data + m_size;
}


template<class T>
inline vector<T>::const_iterator vector<T>::end() const
{
  return m_data + m_size;
}


template<class T>
inline void vector<T>::resize(size_type n, T const & x)
{
  iterator i;

  reserve(n);
  if(n > m_size)
  {
    for(i = m_data + m_size; i != m_data + n; ++i)
    {
      *i = x;
    }
  }
  m_size = n;
}


template<class T>
inline vector<T>::size_type vector<T>::size() const
{
  return m_size;
}


template<class T>
inline vector<T>::size_type vector<T>::max_size() const
{
  return -1;
}


template<class T>
inline bool vector<T>::empty() const
{
  return m_size == 0;
}


template<class T>
inline vector<T>::reference vector<T>::at(size_type pos)
{
  assert(pos < m_size);
  return m_data[pos];
}


template<class T>
inline vector<T>::const_reference vector<T>::at(size_type pos) const
{
  assert(pos < m_size);
  return m_data[pos];
}


template<class T>
inline vector<T>::reference vector<T>::operator [](size_type pos)
{
  assert(pos < m_size);
  return m_data[pos];
}


template<class T>
inline vector<T>::const_reference vector<T>::operator [](size_type pos) const
{
  assert(pos < m_size);
  return m_data[pos];
}


template<class T>
inline vector<T>::reference vector<T>::front()
{
  assert(m_size > 0);
  return m_data[0];
}


template<class T>
inline vector<T>::const_reference vector<T>::front() const
{
  assert(m_size > 0);
  return m_data[0];
}


template<class T>
inline vector<T>::reference vector<T>::back()
{
  assert(m_size > 0);
  return m_data[m_size - 1];
}


template<class T>
inline vector<T>::const_reference vector<T>::back() const
{
  assert(m_size > 0);
  return m_data[m_size - 1];
}


template<class T>
inline void vector<T>::push_back(T const & x)
{
  insert(end(), x);
}


template<class T>
inline void vector<T>::pop_back()
{
  assert(m_size > 0);
  erase(end() - 1);
}


template<class T>
inline void vector<T>::assign(const_iterator first, const_iterator last)
{
  iterator i;
  const_iterator j;

  assert(last >= first);
  if(last - first > m_capacity)
  {
    delete[] m_data;
    m_capacity = ((last - first) * 3) / 2 + 1;
    m_data = new T[m_capacity];
    assert(m_data != NULL);
  }
  m_size = last - first;
  for(i = begin(), j = first; j != last; ++i, ++j)
  {
    *i = *j;
  }
}


template<class T>
inline void vector<T>::assign(size_type n, T const & x)
{
  iterator i;

  if(n > m_capacity)
  {
    delete[] m_data;
    m_capacity = (n * 3) / 2 + 1;
    m_data = new T[m_capacity];
    assert(m_data != NULL);
  }
  m_size = n;
  for(i = begin(); i != end(); ++i)
  {
    *i = x;
  }
}


template<class T>
inline vector<T>::iterator vector<T>::insert(iterator it, T const & x)
{
  difference_type pos = it - m_data;
  iterator i;
  const_iterator j;
  T * old_data;

  assert(it >= m_data);
  assert(it - m_data <= m_size);
  if(m_size + 1 > m_capacity)
  {
    m_capacity = (m_capacity * 3) / 2 + 1;
    old_data = m_data;
    m_data = new T[m_capacity];
    assert(m_data != NULL);
    for(i = m_data, j = old_data; j != old_data + pos; ++i, ++j)
    {
      *i = *j;
    }
    for(i = m_data + pos + 1, j = old_data + pos; j != old_data + m_size; ++i, ++j)
    {
      *i = *j;
    }
    delete[] old_data;
    ++m_size;
  }
  else
  {
    for(i = m_data + m_size, j = m_data + m_size - 1; j + 1 != m_data + pos; --i, --j)
    {
      *i = *j;
    }
    ++m_size;
  }
  m_data[pos] = x;
  return m_data + pos;
}


template<class T>
inline void vector<T>::insert(iterator it, size_type n, T const & x)
{
  difference_type pos = it - m_data;
  iterator i;
  const_iterator j;
  T * old_data;

  assert(it >= m_data);
  assert(it - m_data <= m_size);
  if(m_size + n > m_capacity)
  {
    m_capacity = ((m_size + n) * 3) / 2 + 1;
    old_data = m_data;
    m_data = new T[m_capacity];
    assert(m_data != NULL);
    for(i = m_data, j = old_data; j != old_data + pos; ++i, ++j)
    {
      *i = *j;
    }
    for(i = m_data + pos + n, j = old_data + pos; j != old_data + m_size; ++i, ++j)
    {
      *i = *j;
    }
    delete[] old_data;
    ++m_size;
  }
  else
  {
    for(i = m_data + m_size + n - 1, j = m_data + m_size - 1; j != m_data + pos - 1; --i, --j)
    {
      *i = *j;
    }
    ++m_size;
  }
  for(i = m_data + pos; i != m_data + pos + n; ++i)
  {
    *i = x;
  }
}


template<class T>
inline void vector<T>::insert(iterator it, const_iterator first, const_iterator last)
{
  difference_type pos = it - m_data;
  iterator i;
  const_iterator j;
  T * old_data;
  difference_type n = last - first;

  assert(it >= m_data);
  assert(it - m_data <= m_size);
  assert(last >= first);
  if(m_size + n > m_capacity)
  {
    m_capacity = ((m_size + n) * 3) / 2 + 1;
    old_data = m_data;
    m_data = new T[m_capacity];
    assert(m_data != NULL);
    for(i = m_data, j = old_data; j != old_data + pos; ++i, ++j)
    {
      *i = *j;
    }
    for(i = m_data + pos + n, j = old_data + pos; j != old_data + m_size; ++i, ++j)
    {
      *i = *j;
    }
    delete[] old_data;
    m_size += n;
  }
  else
  {
    for(i = m_data + m_size + n - 1, j = m_data + m_size - 1; j != m_data + pos - 1; --i, --j)
    {
      *i = *j;
    }
    m_size += n;
  }
  for(i = m_data + pos, j = first; j != last; ++i, ++j)
  {
    *i = *j;
  }
}


template<class T>
inline void vector<T>::erase(iterator it)
{
  iterator i;
  const_iterator j;

  assert(it >= m_data);
  assert(it - m_data < m_size);
  for(i = it, j = it + 1; j != end(); ++i, ++j)
  {
    *i = *j;
  }
  --m_size;
}


template<class T>
inline void vector<T>::erase(iterator first, iterator last)
{
  iterator i;
  const_iterator j;

  assert(first >= m_data);
  assert(first - m_data < m_size);
  assert(last >= m_data);
  assert(last - m_data <= m_size);
  for(i = first, j = last; j != end(); ++i, ++j)
  {
    *i = *j;
  }
  m_size -= last - first;
}


template<class T>
inline void vector<T>::clear()
{
  m_size = 0;
}


template<class T>
inline vector<T> & vector<T>::operator =(vector<T> const & x)
{
  assign(x.begin(), x.end());
  return *this;
}


#endif //__VECTOR_HH_INCLUDED


