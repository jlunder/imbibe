#ifndef __HBIN_HH_INCLUDED
#define __HBIN_HH_INCLUDED


#include <iostream.h>

#include "bin_bitmap.hh"
#include "bitmap.hh"
#include "string.hh"
#include "vector.hh"


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

  typedef vector < link > link_list;

  bitmap * m_background;
  link_list m_links;
};


#endif //__HBIN_HH_INCLUDED


