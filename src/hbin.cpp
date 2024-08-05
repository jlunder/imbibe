#include "imbibe.h"

#include "hbin.h"


// hbin::hbin(istream & i)
// {
//   uint8_t buf[256];
//   int n;
//   int x;
//   int width;
//   int height;
//   link l;

//   i.read(buf, 2);
//   width = buf[0] | (buf[1] << 8);
//   i.read(buf, 2);
//   height = buf[0] | (buf[1] << 8);
//   m_background = new bin_bitmap(width, height, i);
//   i.read(buf, 2);
//   n = buf[0] | (buf[1] << 8);
//   m_links.reserve(n);
//   for(x = 0; x < n; ++x)
//   {
//     i.read(buf, 2);
//     l.x = buf[0] | (buf[1] << 8);
//     i.read(buf, 2);
//     l.y = buf[0] | (buf[1] << 8);

//     i.read(buf, 1);
//     i.read(buf + 1, buf[0]);
//     l.action = new string((char *)(buf + 1), (char *)(buf + 1 + buf[0]));

//     i.read(buf, 1);
//     i.read(buf + 1, buf[0]);
//     l.target = new string((char *)(buf + 1), (char *)(buf + 1 + buf[0]));

//     i.read(buf, 2);
//     width = buf[0] | (buf[1] << 8);
//     i.read(buf, 2);
//     height = buf[0] | (buf[1] << 8);

//     l.picture = new bin_bitmap(width, height, i);
//     m_links.push_back(l);
//   }
// }


hbin::~hbin()
{
  link_list::iterator i;

  delete m_background;
  for(i = m_links.begin(); i != m_links.end(); ++i)
  {
    delete i->action;
    delete i->target;
    delete i->picture;
  }
}


