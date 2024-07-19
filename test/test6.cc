#include "cplusplus.hh"

#include "cstream.hh"
#include "hbin.hh"
#include "hbin_element.hh"
#include "hbin_view_handler.hh"
#include "key_dispatcher_task.hh"
#include "rectangle_element.hh"
#include "stop_handler.hh"
#include "task_manager.hh"
#include "text_window.hh"

#include "cstream.ii"
#include "hbin.ii"
#include "hbin_element.ii"
#include "hbin_view_handler.ii"
#include "key_dispatcher_task.ii"
#include "rectangle_element.ii"
#include "stop_handler.ii"
#include "task_manager.ii"
#include "text_window.ii"


#ifndef __BROWSER_HH_INCLUDED
#define __BROWSER_HH_INCLUDED


class browser;


#include "function.hh"
#include "map.hh"
//#include "plugin.hh"
//#include "plugin_factory.hh"
#include "string.hh"
#include "task.hh"
#include "task_manager.hh"


class hbin_browser: public element, public key_handler
{
public:
  browser();
  ~browser();
  void browse_link(string const & action, string const & target);
  void back();
  void forward();

private:
  struct hbin_view_info
  {
    string filename;

  };
  typedef vector < hbin_view_info * > hbin_view_info_p_list;

  hbin_view_info_p_list m_hbin_view_info_stack;
  hbin_view_info_p_list::size_type m_current_hbin;
};


#endif //__BROWSER_HH_INCLUDED


#ifndef __BROWSER_II_INCLUDED
#define __BROWSER_II_INCLUDED


#include <assert.h>

#include "function.ii"
#include "string.ii"
#include "task.ii"
#include "task_manager.ii"
#include "vector.ii"


#endif //__BROWSER_II_INCLUDED


#include "cplusplus.hh"

#include "function.hh"
#include "map.hh"
//#include "plugin.hh"
//#include "plugin_factory.hh"
#include "string.hh"
#include "task.hh"
#include "task_manager.hh"

#include "function.ii"
#include "map.ii"
//#include "plugin.ii"
//#include "plugin_factory.ii"
#include "string.ii"
#include "task.ii"
#include "task_manager.ii"


browser::browser(task_manager & n_owner):
  key_handler(), task(n_owner), m_cur_plugin(0)
{
}


browser::~browser()
{
  plugin_p_list::iterator i;

  for(i = m_plugins.begin(); i != m_plugins.end(); ++i)
  {
    delete *i;
  }
}


void browser::browse(string const & action, string const & target)
{
  assert(m_plugin_factories.find(key) != m_plugin_factories.end());
  if(m_cur_plugin + 1 < m_plugins.size())
  {
    for(i = m_plugins.begin() + m_cur_plugin + 1; i != m_plugins.end(); ++i)
    {
      delete *i;
    }
    m_plugins.erase(m_plugins.begin() + m_cur_plugin + 1, m_plugins.end());
  }
  m_plugins.push_back(m_plugin_factories.find(action)->create_plugin(action, target));
  if(m_plugins.size() > m_max_backup_depth)
  {
    m_cur_plugin -= m_plugins.size() - m_max_backup_depth;
    for(i = m_plugins.begin(); i != m_plugins.begin() + (m_plugins.size() - m_max_backup_depth); ++i)
    {
      delete *i;
    }
    m_plugins.erase(m_plugins.begin(), m_plugins.begin() + (m_plugins.size() - m_max_backup_depth));
  }
  ++m_cur_plugin;
}


void browser::add_plugin(plugin_factory & pf)
{
  assert(m_plugin_factories.find(pf.action()) == m_plugin_factories.end());
  m_plugin_factories.insert(string_plugin_factory_p_map::value_type(pf.action(), &pf));
}


void browser::remove_plugin(plugin_factory & pf)
{
  assert(m_plugin_factories.find(pf.action()) != m_plugin_factories.end());
  assert(m_plugin_factories.find(pf.action())->ref == &pf);
  m_plugin_factories.erase(pf.action());
}


bool browser::handle(int c)
{
  switch(c)
  {
  case control_left:
    back();
    return true;
    break;
  case control_right:
    forward();
    return true;
    break;
  case escape:
    stop();
    return true;
    break;
  default:
    break;
  }
  return false;
}


void browser::back()
{
  if(m_cur_plugin > 0)
  {
    m_plugins[m_cur_plugin].hide();
    --m_cur_plugin;
    m_plugins[m_cur_plugin].show();
  }
}


void browser::forward()
{
  if(m_cur_plugin + 1 < m_plugins.size())
  {
    m_plugins[m_cur_plugin].hide();
    ++m_cur_plugin;
    m_plugins[m_cur_plugin].show();
  }
}


  browser & m_owner;
  string m_filename;
  int m_x1;
  int m_y1;
  int m_x2;
  int m_y2;
  window & m_w;
  int m_scroll_x;
  int m_scroll_y;
  bool m_running;
  hbin * m_hb;
  hbin_element * m_hbe;
  hbin_view_handler * m_hbvh;
};


inline browser & hbin_view_plugin::owner()
{
  return m_owner;
}


hbin_view_plugin::hbin_view_plugin(browser & n_owner, string const & n_filename, int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_w, int n_scroll_x, int n_scroll_y):
  plugin(n_owner), m_filename(n_filename), m_x1(n_x1), m_y1(n_y1), m_x2(n_x2), m_y2(n_y2), m_z(n_z), m_w(n_w), m_scroll_x(n_scroll_x), m_scroll_y(n_scroll_y)
{
}


void hbin_view_plugin::start()
{
  assert(!m_running);
  m_hb = new hbin(cstream(m_filename));
  m_hbe = new hbin_element(m_x1, m_y1, m_x2, m_y2, m_z, m_w, *m_hb);
  m_hbe->set_scroll_pos(m_scroll_x, m_scroll_y);
  m_hbvh = new hbin_view_handler(*m_hb, *m_hbe);
  m_hbe->show();
  owner().add_handler(*m_hbvh);
  m_running = true;
}


void hbin_view_plugin::stop()
{
  assert(m_running);
  owner().remove_handler(*m_hbvh);
  m_hbe->hide();
  delete m_hb;
  delete m_hbe;
  delete m_hbvh;
  m_running = false;
}


class imbibe_plugin_factory
{
  imbibe_plugin_factory(browser & n_owner);
  virtual plugin * create_plugin(string const & action, string const & target);
  browser & owner();

private:
  browser & m_owner;
};


inline browser & imbibe_plugin_factory::owner()
{
  return m_owner;
}


imbibe_plugin_factory::imbibe_plugin_factory(browser & n_owner):
  m_owner(n_owner)
{
}


plugin * imbibe_plugin_factory::create_plugin(string const & action, string const & target)
{
  return new hbin_view_plugin(, m_owner);
}


void run()
{
  task_manager tm;
  key_dispatcher_task kdt(tm);
  text_window tw;
  stop_handler sh(kdt);
  rectangle_element r(0, 0, 80, 25, 0, tw, pixel('#', color(color::hi_red, color::red)));
  hbin hb(icstream("about.hbi"));
  hbin_element hbe(3, 1, 3 + 60, 1 + 23, 10, tw, hb);
  hbin_view_handler mh(hb, hbe);

  tw.add_element(r);
  tw.add_element(hbe);
  kdt.add_handler(sh);
  kdt.add_handler(mh);
  kdt.start();
  tm.run();
}


int main(int argc, char * argv[])
{
  cout << "imbibe 1.0 loaded" << endl;
  run();
  cout << "imbibe 1.0 done" << endl;
  cout << "  code courtesy of hacker joe" << endl;
  return 0;
}


