#ifndef __MENU_H_INCLUDED
#define __MENU_H_INCLUDED


#include <iostream.h>

#include "imbibe.h"

#include "bitmap.h"
#include "string.h"
#include "vector.h"


class menu
{
public:
  menu(std::istream & i);
  ~menu();
  int width() const { return m_width; }
  int height() const { return m_height; }
  int num_links() const { return m_links.size(); }
  string link_action(int which) const { return *m_links[which].action; }
  string link_target(int which) const { return *m_links[which].target; }
  int link_x(int which) const { return m_links[which].x; }
  int link_y(int which) const { return m_links[which].y; }
  int link_width(int which) const { return m_links[which].width; }
  int link_height(int which) const { return m_links[which].height; }
  bitmap const & link_normal_picture(int which) const
    { return *m_links[which].normal_picture; }
  bitmap const & link_selected_picture(int which) const
    { return *m_links[which].selected_picture; }

private:
  struct link
  {
    int x;
    int y;
    int width;
    int height;
    string * action;
    string * target;
    bitmap * normal_picture;
    bitmap * selected_picture;
  };

  typedef vector<link> link_list;

  int m_width;
  int m_height;
  link_list m_links;
};


#endif //__MENU_H_INCLUDED


