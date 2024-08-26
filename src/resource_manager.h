#ifndef __RESOURCE_MANAGER_H_INCLUDED
#define __RESOURCE_MANAGER_H_INCLUDED


#include "imbibe.h"

#include "bitmap.h"
#include "immutable.h"
#include "imstring.h"
#include "unpacker.h"

#define RESOURCE_NAME_LEN_MAX 63

namespace resource_manager {
  void setup();
  void teardown();
  void teardown_exiting();

  extern bool begin_fetch_tbm(imstring const & name);
  extern bool fetch_ready(imstring const & name);
  extern im_ptr<bitmap> fetch_bitmap(imstring const & name);
  extern unpacker fetch_tbm(imstring const & name);
  void flush(imstring const & name);
  void flush_all();
}


#endif // __RESOURCE_MANAGER_H_INCLUDED


