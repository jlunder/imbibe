#include "imbibe.h"

// #include "data_stream.h"
#include "data_str.h"

#include "data.h"


// data_stream::data_stream(string const & name):
//   istrstream(dir.data(name), dir.length(name))
// {
// }


data_stream::directory::directory(uint8_t * data, uint32_t length)
{
  (void)data;
  (void)length;
  // istrstream s(data, length);
  // dir_entry_list_value v;
  // uint32_t dir_begin;
  // uint32_t dir_size;
  // uint32_t i;
  // char buf[57];

  // s.read(buf, 4);
  // dir_begin = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
  // dir_size = length - dir_begin;
  // //assert(dir_size != 0);
  // assert((dir_size % 64) == 0);
  // dir_size /= 64;
  // s.seekg(dir_begin);
  // for(i = 0; i < dir_size; ++i)
  // {
  //   s.read(buf, 56);
  //   buf[56] = 0;
  //   v.key.assign(buf);
  //   s.read(buf, 4);
  //   v.ref.data = data + (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
  //   s.read(buf, 4);
  //   v.ref.length = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
  //   assert(v.ref.data - data <= dir_begin);
  //   assert(v.ref.data - data + v.ref.length <= dir_begin);
  //   m_dir_entries.insert(v);
  // }
}


uint8_t * data_stream::directory::data(imstring const & name)
{
  dir_entry_list_iterator i;

  i = m_dir_entries.find(name);
  assert(i != m_dir_entries.end());
  return i->ref.data;
}


uint32_t data_stream::directory::length(imstring const & name)
{
  dir_entry_list_iterator i;

  i = m_dir_entries.find(name);
  assert(i != m_dir_entries.end());
  return i->ref.length;
}


//data_stream::directory data_stream::dir(test_pkg_data, test_pkg_length);


