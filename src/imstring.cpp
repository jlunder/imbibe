#include "imbibe.h"

#include "imstring.h"
#include "vector.h"


// namespace aux_imstring {
//   struct available_pool {
//     uint16_t size;
//     uint16_t next;
//   };

//   struct dynamic_ref {
//     imstring * holder;
//     uint16_t ims_index;
//     uint16_t pool_index;
//     uint16_t next;
//   };

//   char __based(imstring::s_dynamic_seg) * s_dynamic_pool;
//   vector<dynamic_ref> s_dynamic_refs;
//   vector<dynamic_ref> s_imstrings;
//   uint16_t s_first_live_ref;
//   uint16_t s_first_free_ref;
// }


#if !defined(SIMULATE)
__segment imstring::s_dynamic_seg;
#endif


#if defined(SIMULATE)
bool imstring::is_dynamic(char const __far * str) {
  // TODO
  (void)str;
  return false;
}
#endif


void imstring::copy_dynamic(imstring & ims, char const __far * str) {
  (void)ims;
  (void)str;
  assert(!"TODO");
  // if (is_dynamic(str)) {
  //   ims.m_str = str;
  //   ref_dynamic(ims);
  // } else {
  //   // TODO
  // }
}


void imstring::ref_dynamic(imstring & ims) {
  (void)ims;
  assert(!"TODO");
    // TODO
}


void imstring::unref_dynamic(imstring & ims) {
  (void)ims;
  assert(!"TODO");
    // TODO
}


void imstring::setup() {
    // assert(s_dynamic_refs.empty());
    // aux_imstring::s_first_free_ref = UINT16_MAX;
    // aux_imstring::s_first_live_ref = UINT16_MAX;
}


void imstring::teardown() {
    // assert(aux_imstring::s_first_live_ref == UINT16_MAX);
    // ::free(aux_imstring::s_dynamic_pool);
}


void imstring::teardown_exiting() {
  // do nothing, drop allocated memory on the floor
}




