#ifndef __CSTREAM_HH_INCLUDED
#define __CSTREAM_HH_INCLUDED


#include <strstream.h>

#include "imbibe.h"

//#include "functional.h"
#include "function.h"
#include "map.h"
#include "string.h"


class icstream: public istrstream
{
public:
  icstream(string const & name);

private:
  class directory
  {
  public:
    directory(uint8_t * n_data, uint32_t n_length);
    uint8_t * data(string const & name);
    uint32_t length(string const & name);

  private:
    struct dir_entry
    {
      uint8_t * data;
      uint32_t length;
    };
    typedef map<string, dir_entry, less<string> > dir_entry_list;
    typedef map<string, dir_entry, less<string> >::value_type dir_entry_list_value;
    typedef map<string, dir_entry, less<string> >::iterator dir_entry_list_iterator;

    dir_entry_list m_dir_entries;
  };

  static directory dir;
};


#endif //__CSTREAM_HH_INCLUDED


