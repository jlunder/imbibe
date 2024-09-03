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

extern tbm fetch_tbm(imstring const &name);
void flush(imstring const &name);
void flush_all();

} // namespace resource_manager

#endif // __RESOURCE_MANAGER_H_INCLUDED
