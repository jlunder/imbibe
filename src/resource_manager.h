#ifndef __RESOURCE_MANAGER_H_INCLUDED
#define __RESOURCE_MANAGER_H_INCLUDED

#include "imbibe.h"

#include "immutable.h"
#include "imstring.h"
#include "tbm.h"

#define RESOURCE_NAME_LEN_MAX 63

namespace resource_manager {

void setup();
void teardown();
void teardown_exiting();

tbm fetch_tbm(imstring const &name);
segsize_t fetch_data(imstring const &name, immutable *out_data);

} // namespace resource_manager

#endif // __RESOURCE_MANAGER_H_INCLUDED
