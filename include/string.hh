#ifndef __STRING_HH_INCLUDED
#define __STRING_HH_INCLUDED


#include "imbibe.hh"

#include "vector.hh"


class string
{
public:
  typedef vector < char > ::size_type size_type;
  typedef vector < char > ::difference_type difference_type;
  typedef vector < char > ::reference reference;
  typedef vector < char > ::const_reference const_reference;
  typedef vector < char > ::iterator iterator;
  typedef vector < char > ::const_iterator const_iterator;

  string();
  string(string const & x);
  string(size_type n, char x = ' ');
  string(const_iterator first, const_iterator last);
  string(char const * x);
  void reserve(size_type n);
  size_type capacity() const;
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  char const * c_str() const;
  void resize(size_type n, char x = ' ');
  size_type size() const;
  size_type length() const;
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
  void assign(char x);
  void assign(const_iterator first, const_iterator last);
  void assign(size_type n, char x = ' ');
  void assign(char const * x);
  iterator insert(iterator it, char x = ' ');
  void insert(iterator it, size_type n, char x = ' ');
  void insert(iterator it, const_iterator first, const_iterator last);
  void insert(iterator it, char const * x);
  void append(char x);
  void append(size_type n, char x = ' ');
  void append(const_iterator first, const_iterator last);
  void append(char const * x);
  void erase(iterator it);
  void erase(iterator first, iterator last);
  void clear();
  operator char const *() const;

private:
  vector < char > m_v;
};


bool operator ==(string const & x, string const & y);
bool operator ==(string const & x, char const * y);
bool operator ==(char const * x, string const & y);
bool operator !=(string const & x, string const & y);
bool operator !=(string const & x, char const * y);
bool operator !=(char const * x, string const & y);
bool operator <(string const & x, string const & y);
bool operator <(string const & x, char const * y);
bool operator <(char const * x, string const & y);
bool operator >(string const & x, string const & y);
bool operator >(string const & x, char const * y);
bool operator >(char const * x, string const & y);
bool operator <=(string const & x, string const & y);
bool operator <=(string const & x, char const * y);
bool operator <=(char const * x, string const & y);
bool operator >=(string const & x, string const & y);
bool operator >=(string const & x, char const * y);
bool operator >=(char const * x, string const & y);
string & operator +=(string & x, string const & y);
string & operator +=(string & x, char const * y);
string operator +(string const & x, string const & y);
string operator +(string const & x, char const * y);


#endif //__STRING_HH_INCLUDED


