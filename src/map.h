#ifndef __MAP_H_INCLUDED
#define __MAP_H_INCLUDED


#include "imbibe.h"

// #include "functional.h"
#include "function.h"
#include "vector.h"


template<class Key, class T, class Pred>
class map
{
public:
  typedef Key key_type;
  typedef T referent_type;
  typedef Pred key_compare;
  struct value_type
  {
    Key key;
    T ref;
    value_type();
    value_type(value_type const & x);
    value_type(Key const & k, T const & r);
  };
  class value_compare
  {
  public:
    value_compare(key_compare pr);
    bool operator ()(value_type const & x, value_type const & y);
  private:
    key_compare comp;
  };
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef value_type & reference;
  typedef value_type const & const_reference;
  typedef value_type * iterator;
  typedef value_type const * const_iterator;

  map(Pred const & comp = Pred());
  map(map const & x);
  map(const_iterator first, const_iterator last, Pred const & comp = Pred());
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  size_type size() const;
  size_type max_size() const;
  bool empty();
  reference operator [](Key const & key);
  iterator insert(value_type const & x);
  iterator insert(iterator it, value_type const & x);
  void insert(const_iterator first, const_iterator last);
  iterator erase(iterator it);
  iterator erase(iterator first, iterator last);
  size_type erase(Key const & key);
  void clear();
  key_compare key_comp();
  value_compare value_comp();
  iterator find(Key const & key);
  const_iterator find(Key const & key) const;
  size_type count(Key const & key) const;
  iterator lower_bound(Key const & key);
  const_iterator lower_bound(Key const & key) const;
  iterator upper_bound(Key const & key);
  const_iterator upper_bound(Key const & key) const;

private:
  vector<value_type> m_data;
  key_compare m_key_comp;
  value_compare m_value_comp;
};


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::value_type::value_type()
{
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::value_type::value_type(map<Key, T, Pred>::value_type const & x):
  key(x.key), ref(x.ref)
{
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::value_type::value_type(Key const & k, T const & r):
  key(k), ref(r)
{
}


template<class Key, class T, class Pred>
inline bool map<Key, T, Pred>::value_compare::operator ()(value_type const & x, value_type const & y)
{
  return comp(x.key, y.key);
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::value_compare::value_compare(map<Key, T, Pred>::key_compare pr):
  comp(pr)
{
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::map(Pred const & comp):
  m_key_comp(comp), m_value_comp(comp)
{
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::map(map const & x):
  m_data(x.m_data), m_key_comp(x.m_key_comp), m_value_comp(x.m_value_comp)
{
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::map(const_iterator first, const_iterator last, Pred const & comp):
  m_data(last - first), m_key_comp(comp), m_value_comp(value_compare(comp))
{
  insert(first, last);
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::begin()
{
  return m_data.begin();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::const_iterator map<Key, T, Pred>::begin() const
{
  return m_data.begin();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::end()
{
  return m_data.end();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::const_iterator map<Key, T, Pred>::end() const
{
  return m_data.end();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::size_type map<Key, T, Pred>::size() const
{
  return m_data.size();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::size_type map<Key, T, Pred>::max_size() const
{
  return m_data.max_size();
}


template<class Key, class T, class Pred>
inline bool map<Key, T, Pred>::empty()
{
  return m_data.empty();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::reference map<Key, T, Pred>::operator [](Key const & key)
{
  iterator i = find(key);

  if(i == end())
  {
    i = insert(value_type(key, T()));
  }
  return *i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::insert(map<Key, T, Pred>::value_type const & x)
{
  iterator i = lower_bound(x.key);

  i = m_data.insert(i, x);
  return i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::insert(map<Key, T, Pred>::iterator it, map<Key, T, Pred>::value_type const & x)
{
  iterator i = it;

  if(i == m_data.begin())
  {
    if(m_value_comp(*i, x))
    {
      i = lower_bound(x.key);
    }
  }
  else if(i == m_data.end())
  {
    if(m_value_comp(x, *(i - 1)))
    {
      i = lower_bound(x.key);
    }
  }
  else
  {
    if(m_value_comp(*i, x) || m_value_comp(x, *(i - 1)))
    {
      i = lower_bound(x.key);
    }
  }
  i = m_data.insert(i, x);
  return i;
}


template<class Key, class T, class Pred>
inline void map<Key, T, Pred>::insert(const_iterator first, const_iterator last)
{
  const_iterator i;
  iterator j = end();

  for(i = first; i != last; ++i)
  {
    j = insert(j, *i) + 1;
  }
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::erase(iterator it)
{
  m_data.erase(it);
  return it;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::erase(iterator first, iterator last)
{
  m_data.erase(first, last);
  return first;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::size_type map<Key, T, Pred>::erase(Key const & key)
{
  iterator first = lower_bound(key);
  iterator last = upper_bound(key);
  size_t result = last - first;

  if(first != last)
  {
    m_data.erase(first, last);
  }
  return result;
}


template<class Key, class T, class Pred>
inline void map<Key, T, Pred>::clear()
{
  m_data.clear();
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::key_compare map<Key, T, Pred>::key_comp()
{
  return m_key_comp;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::value_compare map<Key, T, Pred>::value_comp()
{
  return m_value_comp;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::find(Key const & key)
{
  iterator i = lower_bound(key);

  if((i != end()) && m_key_comp(key, i->key))
  {
    i = end();
  }
  return i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::const_iterator map<Key, T, Pred>::find(Key const & key) const
{
  const_iterator i = lower_bound(key);

  if((i != end()) && m_key_comp(key, i->key))
  {
    i = end();
  }
  return i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::size_type map<Key, T, Pred>::count(Key const & key) const
{
  return upper_bound(key) - lower_bound(key);
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::lower_bound(Key const & key)
{
  iterator i = m_data.begin();
  size_type size = m_data.size();

  while(size > 1)
  {
    if(m_key_comp((i + size / 2)->key, key))
    {
      i += size / 2;
      size -= size / 2;
    }
    else
    {
      size = size / 2;
    }
  }
  if(i != m_data.end())
  {
    if(m_key_comp(i->key, key))
    {
      ++i;
    }
  }
  return i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::const_iterator map<Key, T, Pred>::lower_bound(Key const & key) const
{
  const_iterator i = m_data.begin();
  size_type size = m_data.size();

  while(size > 1)
  {
    if(m_key_comp((i + size / 2)->key, key))
    {
      i += size / 2;
      size -= size / 2;
    }
    else
    {
      size = size / 2;
    }
  }
  if(i != m_data.end())
  {
    if(m_key_comp(i->key, key))
    {
      ++i;
    }
  }
  return i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::iterator map<Key, T, Pred>::upper_bound(Key const & key)
{
  iterator i = m_data.begin();
  size_type size = m_data.size();

  while(size > 1)
  {
    if(!m_key_comp(key, (i + size / 2)->key))
    {
      i += size / 2;
      size -= size / 2;
    }
    else
    {
      size = size / 2;
    }
  }
  if(i != m_data.end())
  {
    if(!m_key_comp(key, i->key))
    {
      ++i;
    }
  }
  return i;
}


template<class Key, class T, class Pred>
inline map<Key, T, Pred>::const_iterator map<Key, T, Pred>::upper_bound(Key const & key) const
{
  const_iterator i = m_data.begin();
  size_type size = m_data.size();

  while(size > 1)
  {
    if(!m_key_comp(key, (i + size / 2)->key))
    {
      i += size / 2;
      size -= size / 2;
    }
    else
    {
      size = size / 2;
    }
  }
  if(i != m_data.end())
  {
    if(!m_key_comp(key, i->key))
    {
      ++i;
    }
  }
  return i;
}


#endif //__MAP_H_INCLUDED


