#ifndef __RECLAIM_H_INCLUDED
#define __RECLAIM_H_INCLUDED


template<class T>
class reclaim
{
public:
  virtual void operator()(T * x) = 0;
};


template<class T>
class do_nothing_reclaim : public reclaim<T>
{
public:
  void operator()(T * x) /*override*/ { }
};


template<class T>
class delete_reclaim : public reclaim<T>
{
public:
  void operator()(T * x) /*override*/ { delete x; }
};


#endif // __RECLAIM_H_INCLUDED

