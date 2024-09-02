#include "imbibe.h"

#include "memory.h"

arena __far *arena::s_cur = NULL;
arena __far *arena::s_temp = NULL;
arena __far *arena::s_c = NULL;

namespace aux_memory {

c_arena the_c_arena;
with_arena with_c(&the_c_arena);

}

c_arena::c_arena() {
  assert(arena::s_c == NULL);
  arena::s_c = this;
}

c_arena::~c_arena() {
  assert(s_c == this);
  s_c = NULL;
}

void __far *c_arena::alloc(segsize_t size) {
  void __far *result = ::malloc(size);
  assert(result != NULL);
  return result;
}

void c_arena::free(void __far *p) {
  assert(p != NULL);
  ::free(p);
}

void __far *stack_arena::alloc(segsize_t size) {
  assert(m_capacity >= m_top);
  if (size > m_capacity - m_top) {
    abortf("Out of memory in arena '%s'", m_name);
  }
  segsize_t result = m_top;
  m_top += size;
  return (void __far *)(m_seg + result);
}

void stack_arena::free(void __far *p) {
  assert((__segment)FP_SEG(p) == m_seg);
  assert((segsize_t)FP_OFF(p) < m_top);
  if ((__segment)FP_SEG(p) != m_seg) {
    abortf("Free in arena '%s' not allocated there", m_name);
  } else if (m_live_count == 0) {
    abortf("Too many frees in arena '%s'", m_name);
  } else {
    --m_live_count;
  }
}
