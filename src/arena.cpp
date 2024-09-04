#include "imbibe.h"

#include "arena.h"

arena *arena::s_cur = NULL;
arena *arena::s_temp = NULL;
arena *arena::s_c = NULL;

namespace aux_arena {

extern c_arena the_c_arena;
extern with_arena with_c;

c_arena the_c_arena;
with_arena with_c(&the_c_arena);

} // namespace aux_arena

c_arena::c_arena() {
  assert(arena::s_c == NULL);
  arena::s_c = this;
}

c_arena::~c_arena() {
  assert(s_c == this);
  s_c = NULL;
}

void __far *c_arena::alloc(segsize_t size) {
  void __far *result = ::_fmalloc(size);
  assert(result != NULL);
  return result;
}

void c_arena::free(void __far *p) {
  assert(p != NULL);
  ::_ffree(p);
}

stack_arena::stack_arena(segsize_t n_capacity, char const *n_name) {
  assert(n_capacity > 0);
  m_allocation = ::_fmalloc(n_capacity + PAGE_SIZE - 1);
  segsize_t overshoot_offset = FP_OFF(m_allocation) + PAGE_SIZE - 1;
  segsize_t adjusted_offset = overshoot_offset - (overshoot_offset % PAGE_SIZE);
  assert(adjusted_offset % PAGE_SIZE == 0);
  assert(adjusted_offset >= FP_OFF(m_allocation));
  assert(adjusted_offset < FP_OFF(m_allocation) + PAGE_SIZE);
  assert(adjusted_offset + n_capacity >= n_capacity); // no rollover
  void __far *aligned = MK_FP(FP_SEG(m_allocation), adjusted_offset);
  void __far *normalized = normalize_segmented(aligned);
  assert(FP_OFF(normalized) == 0);
  m_seg = segp<uint8_t>(FP_SEG(normalized));
  m_top = 0;
  m_capacity = n_capacity;
  m_live_count = 0;
  m_name = n_name;
}

stack_arena::~stack_arena() {
  assert(m_live_count == 0);
  if (m_capacity > 0) {
    ::_ffree(m_allocation);
  }
}

void __far *stack_arena::alloc(segsize_t size) {
  assert(m_capacity >= m_top);
  if (size > m_capacity - m_top) {
    abortf("Out of memory in arena '%s'", m_name);
  }
  segsize_t result = m_top;
  m_top += size;
  ++m_live_count;
#ifdef SIMULATE
  (void)result;
  void *p = ::malloc(size);
  m_allocated.push_back(p);
  return p;
#else
  return reinterpret_cast<void __far *>(m_seg + result);
#endif
}

void stack_arena::free(void __far *p) {
#ifdef SIMULATE
  bool found = false;
  for (std::vector<void *>::iterator i = m_allocated.begin();
       i != m_allocated.end(); ++i) {
    if (*i == p) {
      *i = NULL;
      found = true;
    }
  }
  if (!found) {
    abortf("Free in arena '%s' not allocated there (or already freed)", m_name);
  }
  ::free(const_cast<void *>(p));
#else
  assert((__segment)FP_SEG(p) == m_seg);
  assert((segsize_t)FP_OFF(p) < m_top);
  if ((__segment)FP_SEG(p) != m_seg) {
    abortf("Free in arena '%s' not allocated there", m_name);
  }
#endif
  if (m_live_count == 0) {
    abortf("Too many frees in arena '%s'", m_name);
  }
  --m_live_count;
}

void stack_arena::trim() {
  assert(m_capacity >= m_top);
  if (m_capacity == 0) {
    return;
  }
  if (m_top == 0) {
    ::_ffree(m_allocation);
    m_allocation = NULL;
  } else {
    void __far *result = _fexpand(m_allocation, m_top + PAGE_SIZE - 1);
    (void)result;
    assert(result == m_allocation);
  }
  m_capacity = m_top;
}
