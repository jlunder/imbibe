#ifndef __MAP_HH_INCLUDED
#define __MAP_HH_INCLUDED


#include "imbibe.hh"

#include "functional.hh"
#include "vector.hh"


template < class Key, class T, class Pred >
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
  vector < value_type > m_data;
  key_compare m_key_comp;
  value_compare m_value_comp;
};


#endif //__MAP_HH_INCLUDED


