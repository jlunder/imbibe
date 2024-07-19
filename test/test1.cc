#include "cplusplus.hh"

#include <iostream.h>

#include "stdasm.h"

#include "bin_bitmap.hh"
#include "bitmap.hh"
#include "bitmap_element.hh"
#include "cstream.hh"
#include "element.hh"
#include "text_window.hh"

#include "bin_bitmap.ii"
#include "bitmap.ii"
#include "bitmap_element.ii"
#include "cstream.ii"
#include "element.ii"
#include "text_window.ii"


class my_element: public element
{
public:
  my_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner): element(n_x1, n_y1, n_x2, n_y2, n_z, n_owner) {}
  virtual void paint(graphics & g) const
  {
    g.draw_rectangle(0, 0, frame_width(), frame_height(), pixel(' ', color(color::hi_red, color::red)));
    g.draw_text(0, 0, color(color::hi_red, color::red), "testing testing 1 2 3...");
  }
};


void do_it()
{
  int i;
  text_window w;
  my_element e1(0, 0, 80, 25, 0, w);
  bitmap_element e2(4, 4, 24, 24, 10, w, new bitmap(20, 20));
  bitmap_element e3(20, 8, 100, 17, 11, w, new bin_bitmap(80, 9, icstream("hacker3.bin")));

  e2.b().g().draw_rectangle(0, 0, 20, 20, pixel('+', color(color::hi_green, color::green)));
  e2.b().g().draw_text(2, 2, color(color::hi_yellow, color::green), "whee");
  e2.b().g().set_clip(5, 5, 13, 13);
  e2.b().g().draw_rectangle(0, 0, 20, 20, pixel('*', color(color::hi_yellow, color::green)));
  e2.b().g().set_clip(7, 7, 11, 11);
  for(i = 0; i < 20; ++i)
  {
    e2.b().g().draw_text(0, i, color(color::hi_white, color::yellow), "this is just a test. do not panic. this is just a test.");
  }

  w.lock();
  w.add_element(e1);
  w.add_element(e2);
  w.add_element(e3);
  read_key();
  w.unlock();
  read_key();
  e2.set_frame_size(e2.frame_width() - 1, e2.frame_height() - 1);
  read_key();
  e2.set_frame_size(e2.frame_width() - 1, e2.frame_height() - 1);
  read_key();
  e2.set_frame_pos(e2.frame_x1() + 1, e2.frame_y1() + 1);
  read_key();
  w.remove_element(e2);
  read_key();
}


int main(int argc, char * argv[])
{
  cout << "imbibe 1.0" << endl;
  cout << "initializing..." << endl;
  do_it();
  cout << "imbibe 1.0" << endl;
  cout << " code by hacker joe" << endl;
  cout << " i m a h4x0r 31337++*" << endl;
  return 0;
}


