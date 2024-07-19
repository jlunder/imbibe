#include "cplusplus.hh"

#include "fadeout_task.hh"

#include <string.h>

#include "bitmap.hh"
#include "bitmap_element.hh"
#include "timed_task.hh"
#include "task_manager.hh"

#include "fadeout_task.ii"

#include "bitmap.ii"
#include "bitmap_element.ii"
#include "timed_task.ii"
#include "task_manager.ii"


fadeout_task::fadeout_task(task_manager & n_owner, unsigned long n_duration, bitmap_element & n_e, bitmap const & n_b):
  timed_task(n_owner, n_duration), m_e(n_e), m_b(n_b), m_last_mask(0)
{
}


void fadeout_task::begin()
{
  timed_task::begin();

  m_last_mask = 0;
}


void fadeout_task::cycle()
{
  timed_task::cycle();

  int mask_part = 0xF >> ((time() * 4) / duration());
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


void fadeout_task::end()
{
  timed_task::end();

  size_t i;

  if(m_last_mask != 0x00FF)
  {
    for(i = 0; i < m_b.height() * m_b.width(); ++i)
    {
      m_e.b().data()[i] = 0;
    }
    m_e.repaint();
  }
}


