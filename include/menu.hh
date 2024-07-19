#ifndef __MENU_HH_INCLUDED
#define __MENU_HH_INCLUDED


#include <iostream.h>

#include "bitmap.hh"
#include "string.hh"
#include "vector.hh"


class menu
{
public:
  menu(istream & i);
  ~menu();
  int width() const;
  int height() const;
  int num_links() const;
  string link_action(int which) const;
  string link_target(int which) const;
  int link_x(int which) const;
  int link_y(int which) const;
  int link_width(int which) const;
  int link_height(int which) const;
  bitmap const & link_normal_picture(int which) const;
  bitmap const & link_selected_picture(int which) const;

private:
  struct link
  {
    int x;
    int y;
    int width;
    int height;
    string * action;
    string * target;
    bitmap * picture;
  };

  typedef vector < link > link_list;

  int m_width;
  int m_height;
  link_list m_links;
};


#endif //__MENU_HH_INCLUDED


