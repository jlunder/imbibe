#ifndef __ARENA_H_INCLUDED
#define __ARENA_H_INCLUDED

#include "base.h"

#include "memory.h"

#ifdef SIMULATE
#include <vector>
#endif

class arena {
public:
  virtual ~arena() {}
  virtual void __far *alloc(segsize_t sz) = 0;
  virtual void free(void __far *p) = 0;

  static arena *cur() { return s_cur; }
  static arena *temp() { return s_temp; }
  static arena *c() { return s_c; }

  static void __far *cur_alloc(segsize_t size) { return s_cur->alloc(size); }
  static void cur_free(void __far *p) { s_cur->free(p); }
  static void __far *temp_alloc(segsize_t size) { return s_temp->alloc(size); }
  static void temp_free(void __far *p) { s_temp->free(p); }
  static void __far *c_alloc(segsize_t size) { return s_c->alloc(size); }
  static void c_free(void __far *p) { s_c->free(p); }

private:
  static arena *s_cur;
  static arena *s_temp;
  static arena *s_c;

  friend class c_arena;
  friend class with_arena;
  friend class with_temp_arena;
};

inline void *operator new(size_t size, arena *mem) { return mem->alloc(size); }

#if !defined(__WATCOMC__)
// Not generally accessible; called by the compiler to cover up constructor
// exceptions thrown during allocation
inline void operator delete(void *p, arena *mem) { return mem->free(p); }
#endif

class with_arena {
public:
  explicit with_arena(arena *n_cur) : m_saved_cur(arena::s_cur) {
    arena::s_cur = n_cur;
#ifndef NDEBUG
    m_set_cur = n_cur;
#endif
  }
  ~with_arena() {
    assert(arena::s_cur == m_set_cur);
    arena::s_cur = m_saved_cur;
  }

private:
  arena *m_saved_cur;
#ifndef NDEBUG
  arena *m_set_cur;
#endif
};

class with_temp_arena {
public:
  explicit with_temp_arena(arena *n_temp) : m_saved_temp(arena::s_temp) {
    arena::s_temp = n_temp;
#ifndef NDEBUG
    m_set_temp = n_temp;
#endif
  }
  ~with_temp_arena() {
    assert(arena::s_cur == m_set_temp);
    arena::s_temp = m_saved_temp;
  }

private:
  arena *m_saved_temp;
#ifndef NDEBUG
  arena *m_set_temp;
#endif
};

class c_arena : public arena {
public:
  c_arena();
  virtual ~c_arena();
  virtual void __far *alloc(segsize_t sz);
  virtual void free(void __far *p);
};

class stack_arena : public arena {
public:
  struct mark_t {
    segsize_t marked_top;
    segsize_t marked_live;
#ifndef NDEBUG
    segsize_t marked_allocated;
#endif
  };

  stack_arena() : m_seg(0), m_name("<uninit>") {}
  explicit stack_arena(segsize_t n_capacity, char const __far *n_name);
  virtual ~stack_arena();
  virtual void __far *alloc(segsize_t size);
  virtual void free(void __far *p);

  mark_t mark() {
#ifdef SIMULATE
    mark_t result = {m_top, m_live_count, (segsize_t)m_allocated.size()};
#else
    mark_t result = {m_top, m_live_count};
#endif
    return result;
  }
  void reset(mark_t n_mark) {
    assert(n_mark.marked_top <= m_top);
    assert(n_mark.marked_live == m_live_count);
#ifdef SIMULATE
    assert(n_mark.marked_allocated < m_allocated.size());
    for (segsize_t i = n_mark.marked_allocated; i < m_allocated.size(); ++i) {
      assert(!m_allocated[i]);
    }
    m_allocated.resize(n_mark.marked_allocated);
#endif
    m_top = n_mark.marked_top;
    m_live_count = n_mark.marked_live;
  }

  void trim();

private:
  segp<uint8_t> m_seg;
  segsize_t m_top;
  segsize_t m_capacity;
  segsize_t m_live_count;
  void __far *m_allocation;
  char const *m_name;
#ifdef SIMULATE
  std::vector<void *> m_allocated;
#endif
};

#endif // __ARENA_H_INCLUDED
