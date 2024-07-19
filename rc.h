unsigned * _alloc_count();
void _free_count(unsigned * count);

template <class T>
class rc_ptr {
private:
  unsigned * _count;
  T * _p;

  void release() {
    if(_count != NULL)
    {
      --*_count;
      if(*_count == 0) {
        _free_count(_count);
        delete _p;
      }
    }
  }
public:
  rc_ptr(): _count(NULL) {}
  rc_ptr(T * n_p): _count(_alloc_count()), _p(n_p) {
    ++*_count;
  }
  rc_ptr(rc_ptr<T> const & n_rc_ptr):
    _count(n_rc_ptr._count), _p(n_rc_ptr._p) {
    ++*_count;
  }
  ~rc_ptr() {release();}
  T & operator *() const {return *_p;}
  T * operator ->() const {return _p;}
  operator T * () const {return _p;}
  rc_ptr<T> & operator = (rc_ptr<T> const & n_rc_ptr) {
    release();
    _count = n_rc_ptr._count;
    _p = n_rc_ptr._p;
    ++*_count;
  }
  rc_ptr<T> & operator = (T * n_p) {
    release();
    _count = _alloc_count();
    _p = n_p;
    ++*_count;
  }
};

template <class T>
class rc_arr {
private:
  unsigned * _count;
  T * _p;

  void release() {
    if(_count != NULL)
    {
      --*_count;
      if(*_count == 0) {
        _free_count(_count);
        delete[] _p;
      }
    }
  }
public:
  rc_arr(): _count(NULL) {}
  rc_arr(T * n_p): _count(_alloc_count()), _p(n_p) {
    ++*_count;
  }
  rc_arr(rc_arr<T> const & n_rc_arr):
    _count(n_rc_arr._count), _p(n_rc_arr._p) {
    ++*_count;
  }
  ~rc_arr() {release();}
  T & operator *() const {return *_p;}
  T * operator ->() const {return _p;}
  T * operator + (size_t n) const {return _p + n;}
  T & operator [](size_t n) const {return _p[n];}
  operator T * () const {return _p;}
  rc_arr<T> & operator = (rc_arr<T> const & n_rc_arr) {
    release();
    _count = n_rc_arr._count;
    _p = n_rc_arr._p;
    ++*_count;
  }
  rc_arr<T> & operator = (T * n_p) {
    release();
    _count = _alloc_count();
    _p = n_p;
    ++*_count;
  }
};

