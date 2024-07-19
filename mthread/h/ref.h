template<class T>
class ref
{
public:
  ref(T * np)
  {
    refcount = new_refcount();
    ++*refcount;
  }
  ref(ref<T> & nref)
  {
    p = nref.p;
    refcount = nref.refcount;
    ++*refcount;
  }
  ~ref()
  {
    --*refcount;
    if(!*refcount)
    {
      delete p;
      delete_refcount(refcount);
    }
  }
  T & operator *()
  {
    return *p;
  }
  T * operator ->()
  {
    return p;
  }
private:
  T * p;
  unsigned * refcount;
};


