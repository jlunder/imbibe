#include "imbibe.hh"

#include <iostream.h>

#include "menu.hh"

#include "bin_bitmap.hh"
#include "bitmap.hh"
#include "string.hh"
#include "vector.hh"


menu::menu(istream & i)
{
  uint8_t buf[256];
  int w;
  int h;
  link l;

  i.read(buf, 2);
  m_width = buf[0] + (buf[1] << 8);
  i.read(buf, 2);
  m_height = buf[0] + (buf[1] << 8);

  i.read(buf, 2);
  assert(i.good());
  uint_fast16_t num_links = buf[0] + (buf[1] << 8);
  m_links.reserve(num_links);

  for(uint_fast16_t x = 0; x < num_links; ++x)
  {
    i.read(buf, 1);
    w = buf[0];
    assert(w < 256);
    i.read(buf, w);
    l.action = new string((char *)buf, (char *)buf + w);

    i.read(buf, 1);
    w = buf[0];
    assert(w < 256);
    i.read(buf, w);
    l.target = new string((char *)buf, (char *)buf + w);

    i.read(buf, 2);
    l.x = buf[0] + ((buf[1] & 0x7F) << 8) + ((buf[1] & 0x80) ? -(1 << 15) : 0);
    i.read(buf, 2);
    l.y = buf[0] + ((buf[1] & 0x7F) << 8) + ((buf[1] & 0x80) ? -(1 << 15) : 0);

    i.read(buf, 2);
    l.width = buf[0] + (buf[1] << 8);
    i.read(buf, 2);
    l.height = buf[0] + (buf[1] << 8);
    assert(i.good());

    l.normal_picture = new bin_bitmap(l.width, l.height, i);
    l.selected_picture = new bin_bitmap(l.width, l.height, i);

    m_links.push_back(l);
  }
}


menu::~menu()
{
  link_list::iterator i;

  for(i = m_links.begin(); i != m_links.end(); ++i)
  {
    delete i->action;
    delete i->target;
    delete i->normal_picture;
    delete i->selected_picture;
  }
}


