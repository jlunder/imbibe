#ifndef __HBIN_H_INCLUDED
#define __HBIN_H_INCLUDED


#include "imbibe.h"

// #include "bin_bitmap.h"
#include "bitmap.h"
#include "string.h"
#include "vector.h"


class hbin
{
public:
  ~hbin();
  int width() const { return m_background->width(); }
  int height() const { return m_background->height(); }
  bitmap const & background() const { return *m_background; }
  int num_links() const { return m_links.size(); }
  string link_action(int which) const { return *m_links[which].action; }
  string link_target(int which) const { return *m_links[which].target; }
  int link_x(int which) const { return m_links[which].x; }
  int link_y(int which) const { return m_links[which].y; }
  int link_width(int which) const { return m_links[which].picture->width(); }
  int link_height(int which) const { return m_links[which].picture->height(); }
  bitmap const & link_picture(int which) const { return *m_links[which].picture; }

private:
  struct link
  {
    int x;
    int y;
    string * action;
    string * target;
    bitmap * picture;
  };

  typedef vector<link> link_list;

  bitmap * m_background;
  link_list m_links;
};


#endif //__HBIN_H_INCLUDED


