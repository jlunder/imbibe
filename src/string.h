#ifndef __STRING_HH_INCLUDED
#define __STRING_HH_INCLUDED


#include "imbibe.h"

#include "vector.h"


class string
{
public:
  typedef vector<char>::size_type size_type;
  typedef vector<char>::difference_type difference_type;
  typedef vector<char>::reference reference;
  typedef vector<char>::const_reference const_reference;
  typedef vector<char>::iterator iterator;
  typedef vector<char>::const_iterator const_iterator;

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
  vector<char> m_v;
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


inline string::string()
{
  m_v.push_back('\000');
}


inline string::string(string const & x):
  m_v(x.m_v)
{
}


inline string::string(size_type n, char x):
  m_v(n, x)
{
  m_v.push_back('\000');
}


inline string::string(const_iterator first, const_iterator last):
  m_v(first, last)
{
  m_v.push_back('\000');
}


inline string::string(char const * x)
{
  m_v.assign(x, x + strlen(x) + 1);
}


inline void string::reserve(size_type n)
{
  m_v.reserve(n + 1);
}


inline string::size_type string::capacity() const
{
  return m_v.capacity();
}


inline string::iterator string::begin()
{
  return m_v.begin();
}


inline string::const_iterator string::begin() const
{
  return m_v.begin();
}


inline string::iterator string::end()
{
  return m_v.end() - 1;
}


inline string::const_iterator string::end() const
{
  return m_v.end() - 1;
}


inline char const * string::c_str() const
{
  return m_v.begin();
}


inline void string::resize(size_type n, char x)
{
  m_v.resize(n + 1, x);
  m_v[n] = '\000';
}


inline string::size_type string::size() const
{
  return m_v.size();
}


inline string::size_type string::length() const
{
  return m_v.size() - 1;
}


inline string::size_type string::max_size() const
{
  return m_v.max_size();
}


inline bool string::empty() const
{
  return m_v.size() == 1;
}


inline string::reference string::at(size_type pos)
{
  return m_v.at(pos);
}


inline string::const_reference string::at(size_type pos) const
{
  return m_v.at(pos);
}


inline string::reference string::operator [](size_type pos)
{
  return m_v.at(pos);
}


inline string::const_reference string::operator [](size_type pos) const
{
  return m_v.at(pos);
}


inline string::reference string::front()
{
  return m_v.front();
}


inline string::const_reference string::front() const
{
  return m_v.front();
}


inline string::reference string::back()
{
  return *(m_v.end() - 2);
}


inline string::const_reference string::back() const
{
  return *(m_v.end() - 2);
}


inline void string::assign(char x)
{
  m_v.assign(2, x);
  m_v[1] = '\000';
}


inline void string::assign(const_iterator first, const_iterator last)
{
  m_v.assign(first, last);
  m_v.push_back('\000');
}


inline void string::assign(size_type n, char x)
{
  m_v.assign(n, x);
  m_v.push_back('\000');
}


inline void string::assign(char const * x)
{
  m_v.assign(x, x + strlen(x) + 1);
}


inline string::iterator string::insert(iterator it, char x)
{
  return m_v.insert(it, x);
}


inline void string::insert(iterator it, size_type n, char x)
{
  m_v.insert(it, n, x);
}


inline void string::insert(iterator it, const_iterator first, const_iterator last)
{
  m_v.insert(it, first, last);
}


inline void string::insert(iterator it, char const * x)
{
  m_v.insert(it, x, x + strlen(x) + 1);
}


inline void string::append(char x)
{
  m_v.push_back('\000');
  m_v[m_v.size() - 2] = x;
}


inline void string::append(size_type n, char x)
{
  m_v.insert(m_v.end() - 1, n, x);
}


inline void string::append(const_iterator first, const_iterator last)
{
  m_v.insert(m_v.end() - 1, first, last);
}


inline void string::append(char const * x)
{
  m_v.insert(m_v.end() - 1, x, x + strlen(x));
}


inline void string::erase(iterator it)
{
  m_v.erase(it);
}


inline void string::erase(iterator first, iterator last)
{
  m_v.erase(first, last);
}


inline void string::clear()
{
  m_v.clear();
  m_v.push_back('\000');
}


inline string::operator char const *() const
{
  return c_str();
}


inline bool operator ==(string const & x, string const & y)
{
  return strcmp(x.c_str(), y.c_str()) == 0;
}


inline bool operator ==(string const & x, char const * y)
{
  return strcmp(x.c_str(), y) == 0;
}


inline bool operator ==(char const * x, string const & y)
{
  return strcmp(x, y.c_str()) == 0;
}


inline bool operator !=(string const & x, string const & y)
{
  return strcmp(x.c_str(), y.c_str()) != 0;
}


inline bool operator !=(string const & x, char const * y)
{
  return strcmp(x.c_str(), y) != 0;
}


inline bool operator !=(char const * x, string const & y)
{
  return strcmp(x, y.c_str()) != 0;
}


inline bool operator <(string const & x, string const & y)
{
  return strcmp(x.c_str(), y.c_str()) < 0;
}


inline bool operator <(string const & x, char const * y)
{
  return strcmp(x.c_str(), y) < 0;
}


inline bool operator <(char const * x, string const & y)
{
  return strcmp(x, y.c_str()) < 0;
}


inline bool operator >(string const & x, string const & y)
{
  return strcmp(x.c_str(), y.c_str()) > 0;
}


inline bool operator >(string const & x, char const * y)
{
  return strcmp(x.c_str(), y) > 0;
}


inline bool operator >(char const * x, string const & y)
{
  return strcmp(x, y.c_str()) > 0;
}


inline bool operator <=(string const & x, string const & y)
{
  return strcmp(x.c_str(), y.c_str()) <= 0;
}


inline bool operator <=(string const & x, char const * y)
{
  return strcmp(x.c_str(), y) <= 0;
}


inline bool operator <=(char const * x, string const & y)
{
  return strcmp(x, y.c_str()) <= 0;
}


inline bool operator >=(string const & x, string const & y)
{
  return strcmp(x.c_str(), y.c_str()) >= 0;
}


inline bool operator >=(string const & x, char const * y)
{
  return strcmp(x.c_str(), y) >= 0;
}


inline bool operator >=(char const * x, string const & y)
{
  return strcmp(x, y.c_str()) >= 0;
}


inline string & operator +=(string & x, string const & y)
{
  x.append(y);
  return x;
}


inline string & operator +=(string & x, char const * y)
{
  x.append(y);
  return x;
}


inline string operator +(string const & x, string const & y)
{
  string s(x);
  s.append(y);
  return s;
}


inline string operator +(string const & x, char const * y)
{
  string s(x);
  s.append(y);
  return s;
}


#endif //__STRING_HH_INCLUDED


