#ifndef __MAP_H_INCLUDED
#define __MAP_H_INCLUDED


#include "imbibe.h"

#include "vector.h"


template<class T>
class less
{
public:
  bool operator ()(T const & a, T const & b) const { return a < b; }
};


template<class Key, class T, class Pred = less<Key> >
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
    value_type() { }
    value_type(value_type const & x): key(x.key), ref(x.ref) { }
    value_type(Key const & k, T const & r): key(k), ref(r) { }
  };
  class value_compare
  {
  public:
    value_compare(): comp() { }
    value_compare(key_compare pr): comp(pr) { }
    bool operator ()(value_type const & x, value_type const & y)
      { return comp(x.key, y.key); }
  private:
    key_compare comp;
  };
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef value_type & reference;
  typedef value_type const & const_reference;
  typedef value_type * iterator;
  typedef value_type const * const_iterator;

  map(): m_key_comp(), m_value_comp() { }
  explicit map(Pred const & comp): m_key_comp(comp), m_value_comp(comp) { }
  map(map const & x)
    : m_data(x.m_data), m_key_comp(x.m_key_comp),
      m_value_comp(x.m_value_comp)
    { }
  map(const_iterator first, const_iterator last, Pred const & comp)
    : m_data(last - first), m_key_comp(comp),
      m_value_comp(value_compare(comp))
    { insert(first, last); }

  iterator begin() { return m_data.begin(); }
  const_iterator begin() const { return m_data.begin(); }
  iterator end() { return m_data.end(); }
  const_iterator end() const { return m_data.end(); }
  size_type size() const { return m_data.size(); }
  size_type max_size() const { return m_data.max_size(); }
  bool empty() { return m_data.empty(); }

  reference operator [](Key const & key) {
    iterator i = find(key);

    if(i == end()) {
      i = insert(value_type(key, T()));
    }
    return *i;
  }

  iterator insert(value_type const & x) {
    iterator i = lower_bound(x.key);
    i = m_data.insert(i, x);
    return i;
  }

  iterator insert(iterator it, value_type const & x) {
    iterator i = it;

    if(i == m_data.begin()) {
      if(m_value_comp(*i, x)) {
        i = lower_bound(x.key);
      }
    } else if(i == m_data.end()) {
      if(m_value_comp(x, *(i - 1))) {
        i = lower_bound(x.key);
      }
    } else {
      if(m_value_comp(*i, x) || m_value_comp(x, *(i - 1))) {
        i = lower_bound(x.key);
      }
    }
    i = m_data.insert(i, x);
    return i;
  }

  void insert(const_iterator first, const_iterator last) {
    iterator j = end();

    for(const_iterator i = first; i != last; ++i) {
      j = insert(j, *i) + 1;
    }
  }

  iterator erase(iterator it) {
    m_data.erase(it);
    return it;
  }

  iterator erase(iterator first, iterator last) {
    m_data.erase(first, last);
    return first;
  }

  size_type erase(Key const & key) {
    iterator first = lower_bound(key);
    iterator last = upper_bound(key);
    size_t result = last - first;

    if(first != last) {
      m_data.erase(first, last);
    }
    return result;
  }

  void clear() { m_data.clear(); }
  key_compare key_comp() { return m_key_comp; }
  value_compare value_comp() { return m_value_comp; }

  iterator find(Key const & key) {
    iterator i = lower_bound(key);

    if((i != end()) && m_key_comp(key, i->key)) {
      i = end();
    }
    return i;
  }

  const_iterator find(Key const & key) const {
    const_iterator i = lower_bound(key);

    if((i != end()) && m_key_comp(key, i->key)) {
      i = end();
    }
    return i;
  }

  size_type count(Key const & key) const
    { return upper_bound(key) - lower_bound(key); }

  iterator lower_bound(Key const & key) {
    iterator i = m_data.begin();
    size_type size = m_data.size();

    while(size > 1) {
      if(m_key_comp((i + size / 2)->key, key)) {
        i += size / 2;
        size -= size / 2;
      } else {
        size = size / 2;
      }
    }
    if(i != m_data.end()) {
      if(m_key_comp(i->key, key)) {
        ++i;
      }
    }
    return i;
  }

  const_iterator lower_bound(Key const & key) const {
    const_iterator i = m_data.begin();
    size_type size = m_data.size();

    while(size > 1) {
      if(m_key_comp((i + size / 2)->key, key)) {
        i += size / 2;
        size -= size / 2;
      }
      else {
        size = size / 2;
      }
    }
    if(i != m_data.end()) {
      if(m_key_comp(i->key, key)) {
        ++i;
      }
    }
    return i;
  }

  iterator upper_bound(Key const & key) {
    iterator i = m_data.begin();
    size_type size = m_data.size();

    while(size > 1) {
      if(!m_key_comp(key, (i + size / 2)->key)) {
        i += size / 2;
        size -= size / 2;
      }
      else {
        size = size / 2;
      }
    }
    if(i != m_data.end()) {
      if(!m_key_comp(key, i->key)) {
        ++i;
      }
    }
    return i;
  }

  const_iterator upper_bound(Key const & key) const {
    const_iterator i = m_data.begin();
    size_type size = m_data.size();

    while(size > 1) {
      if(!m_key_comp(key, (i + size / 2)->key)) {
        i += size / 2;
        size -= size / 2;
      } else {
        size = size / 2;
      }
    }
    if(i != m_data.end()) {
      if(!m_key_comp(key, i->key)) {
        ++i;
      }
    }
    return i;
  }

private:
  vector<value_type> m_data;
  key_compare m_key_comp;
  value_compare m_value_comp;
};


#endif //__MAP_H_INCLUDED


