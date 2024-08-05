#ifndef __CSTREAM_H_INCLUDED
#define __CSTREAM_H_INCLUDED


#include "imbibe.h"

#include "stream.h"


class data_stream: public stream
{
public:
  // static data_stream open_data(char const * name);
  // static void read_into(char const * name, size_t size);

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


#endif //__CSTREAM_H_INCLUDED


