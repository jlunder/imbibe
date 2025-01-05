#ifndef __RESOURCE_MANAGER_H_INCLUDED
#define __RESOURCE_MANAGER_H_INCLUDED

#include "imbibe.h"

#include "immutable.h"
#include "imstring.h"
#include "tbm.h"

#define RESOURCE_NAME_LEN_MAX 63

namespace resource_manager {

void setup(imstring const &archive_name = imstring());
void teardown();
void teardown_exiting();

segsize_t fetch_data(imstring const &name, immutable *out_data);

inline tbm fetch_tbm(imstring const &name) {
  immutable data;
  segsize_t size = fetch_data(name, &data);
  return tbm(data, size);
}

} // namespace resource_manager

#endif // __RESOURCE_MANAGER_H_INCLUDED
