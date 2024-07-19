#include "cplusplus.hh"

#include <iostream.h>
#include <stdlib.h>

#include "bin_bitmap.hh"
#include "bitmap.hh"
#include "bitmap_element.hh"
#include "cstream.hh"
#include "timed_task.hh"
#include "task_manager.hh"
#include "text_window.hh"

#include "stdasm.h"

#include "bin_bitmap.ii"
#include "bitmap.ii"
#include "bitmap_element.ii"
#include "cstream.ii"
#include "timed_task.ii"
#include "task_manager.ii"
#include "text_window.ii"


class fadein_task: public timed_task
{
public:
  fadein_task(task_manager & n_owner, unsigned long n_duration, bitmap_element & n_e, bitmap const & n_b);
  virtual void begin();
  virtual void cycle();
  virtual void end();

private:
  bitmap_element & m_e;
  bitmap const & m_b;
  unsigned short m_last_mask;
};


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
  }
}


void run()
{
  text_window w;
  task_manager t;
  bin_bitmap b(80, 9, icstream("hacker3.bin"));
  bitmap_element e(0, 0, 80, 25, 0, w, new bitmap(80, 9));
  fadein_task m(t, 250, e, b);

  w.add_element(e);
  m.start();
  t.run();
  read_key();
  e.b().g().draw_bitmap(0, 0, bin_bitmap(80, 25, icstream("about.bin")));
  e.repaint();
  read_key();
}


int main(int argc, char * argv[])
{
  cout << "imbibe 1.0 loaded" << endl;
  run();
  cout << "imbibe 1.0 done" << endl;
  cout << "  code courtesy of hacker joe" << endl;
  return 0;
}


