#ifndef __CSTREAM_HH_INCLUDED
#define __CSTREAM_HH_INCLUDED


#include <strstream.h>

#include "imbibe.hh"

#include "functional.hh"
#include "map.hh"
#include "string.hh"


class icstream: public istrstream
{
public:
  icstream(string const & name);

private:
  class directory
  {
  public:
    directory(unsigned char * n_data, unsigned long n_length);
    unsigned char * data(string const & name);
    unsigned long length(string const & name);

  private:
    struct dir_entry
    {
      unsigned char * data;
      unsigned long length;
    };
    typedef map < string, dir_entry, less < string > > dir_entry_list;
    typedef map < string, dir_entry, less < string > > ::value_type dir_entry_list_value;
    typedef map < string, dir_entry, less < string > > ::iterator dir_entry_list_iterator;

    dir_entry_list m_dir_entries;
  };

  static directory dir;
};


#endif //__CSTREAM_HH_INCLUDED


