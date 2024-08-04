#ifndef __HBIN_HH_INCLUDED
#define __HBIN_HH_INCLUDED


#include <iostream.h>

#include "imbibe.h"

// #include "bin_bitmap.h"
#include "bin_bitm.h"
#include "bitmap.h"
#include "string.h"
#include "vector.h"


class hbin
{
public:
  hbin(istream & i);
  ~hbin();
  int width() const;
  int height() const;
  bitmap const & background() const;
  int num_links() const;
  string link_action(int which) const;
  string link_target(int which) const;
  int link_x(int which) const;
  int link_y(int which) const;
  int link_width(int which) const;
  int link_height(int which) const;
  bitmap const & link_picture(int which) const;

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


inline int hbin::width() const
{
  return m_background->width();
}


inline int hbin::height() const
{
  return m_background->height();
}


inline bitmap const & hbin::background() const
{
  return *m_background;
}


inline int hbin::num_links() const
{
  return m_links.size();
}


inline string hbin::link_action(int which) const
{
  return *m_links[which].action;
}


inline string hbin::link_target(int which) const
{
  return *m_links[which].target;
}


inline int hbin::link_x(int which) const
{
  return m_links[which].x;
}


inline int hbin::link_y(int which) const
{
  return m_links[which].y;
}


inline int hbin::link_width(int which) const
{
  return m_links[which].picture->width();
}


inline int hbin::link_height(int which) const
{
  return m_links[which].picture->height();
}


inline bitmap const & hbin::link_picture(int which) const
{
  return *m_links[which].picture;
}


#endif //__HBIN_HH_INCLUDED


