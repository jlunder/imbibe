#include "cplusplus.hh"

#include "fadein_task.hh"

#include <string.h>

#include "bitmap.hh"
#include "bitmap_element.hh"
#include "timed_task.hh"
#include "task_manager.hh"

#include "fadein_task.ii"

#include "bitmap.ii"
#include "bitmap_element.ii"
#include "timed_task.ii"
#include "task_manager.ii"


fadein_task::fadein_task(task_manager & n_owner, unsigned long n_duration, bitmap_element & n_e, bitmap const & n_b):
  timed_task(n_owner, n_duration), m_e(n_e), m_b(n_b), m_last_mask(0)
{
}


void fadein_task::begin()
{
  timed_task::begin();

  m_last_mask = 0;
}


void fadein_task::cycle()
{
  timed_task::cycle();

  int mask_part = 0xF >> (4 - (time() * 4) / duration());
  unsigned short mask = (mask_part << 12) | (mask_part << 8) | 0xFF;
  size_t i;

  if(mask != m_last_mask)
  {
    for(i = 0; i < m_b.height() * m_b.width(); ++i)
    {
      m_e.b().data()[i] = m_b.data()[i] & mask;
    }
    m_e.repaint();
    m_last_mask = mask;
  }
}


void fadein_task::end()
{
  timed_task::end();

  if(m_last_mask != 0xFFFF)
  {
    memcpy(m_e.b().data(), m_b.data(), m_b.width() * m_b.height() * sizeof(unsigned short));
    m_e.repaint();
  }
}


