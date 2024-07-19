#include <string.h>

class string {
private:
  char * _data;
  size_t _capacity;
  size_t _size;
public:
  typedef char & reference;
  typedef char const & const_reference;
  typedef char * iterator;
  typedef char const * const_iterator;

  string(size_t n = 4): _data(new char[n + 1]), _capacity(n), _size(0) {
    _data[0] = 0;
  }
  string(string const & x, size_t n = 4): _data(new char[x._size + n + 1]),
    _capacity(x._size + n), _size(x._size) {
    memcpy(_data, x._data, x._size + 1);
  }
  string(char const * x, size_t n = 4) {
    _size = strlen(x);
    _capacity = _size + n;
    _data = new char[_capacity + 1];
    memcpy(_data, x, _size + 1);
  }
  string(const_iterator first, const_iterator last, size_t n = 4):
    _data(new char[last - first + n + 1]), _capacity(last - first + n),
    _size(last - first) {
    memcpy(_data, first, _size);
    _data[_size] = 0;
  }
  ~string() {delete[] _data;}
  void reserve(size_t n) {
    char * new_data;
    if(_capacity < n + 1)
    {
      new_data = new char[n + 1];
      memcpy(new_data, _data, _size + 1);
      delete[] _data;
      _data = new_data;
    }
  }
  iterator begin() {return _data;}
  iterator end() {return _data + _size;}
  char & front() {return _data[0];}
  char & back() {return _data[_size - 1];}
  void assign(const_iterator first, const_iterator last) {
    _size = 0;
    reserve(last - first);
    _size = last - first;
    memcpy(_data, first, last - first + 1);
  }
  void assign(string const & x) {assign(x._data, x._data + x._size);}
  void assign(char const * x) {assign(x, x + strlen(x));}
  void assign(char c) {assign(&c, &c + 1);}
  void insert(iterator it, const_iterator first, const_iterator last) {
    char * new_data;
    if(_capacity >= (_size + (last - first)))
    {
      memmove(it + (last - first), it, _size - (it - _data) + 1);
      memcpy(it, first, last - first);
      _size += last - first;
    }
    else
    {
      new_data = new char[_size + (last - first) + 1];
      memcpy(new_data, _data, it - _data);
      memcpy(new_data + (it - _data), first, last - first);
      memcpy(new_data + (it - _data) + (last - first), it,
        _size - (it - _data) + 1);
      _size += last - first;
      delete[] _data;
      _data = new_data;
    }
  }
  void insert(iterator it, string const & x) {
    insert(it, x._data, x._data + x._size);
  }
  void insert(iterator it, char const * x) {insert(it, x, x + strlen(x));}
  void insert(iterator it, char c) {insert(it, &c, &c + 1);}
  void append(string const & x) {insert(end(), x);}
  void append(char const * x) {insert(end(), x);}
  void append(const_iterator first, const_iterator last) {
    insert(end(), first, last);
  }
  void append(char c) {insert(end(), c);}
  void erase(iterator it) {erase(it, it + 1);}
  void erase(iterator first, iterator last) {
    memmove(first, last, _size - (last - _data) + 1);
  }
  void clear() {_size = 0; _data[0] = 0;}
  char const * cstr() const {return _data;}
  size_t capacity() {return _capacity;}
  char & operator [](size_t n) {return _data[n];}
  char operator [](size_t n) const {return _data[n];}
};

