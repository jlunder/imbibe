#include "imbibe.h"

#include "rodata.h"


struct rodata_tracking_t {
  rodata::reclaim_func_t reclaimer;
  union {
    int16_t live_refs;
    int16_t next_unrefd;
  };
};


rodata_tracking_t rodata_index[rodata::max_reclaimable + 1];


void rodata::init(reclaim_func_t f)
{
  assert(rodata_index[0].reclaimer == NULL);
  if (rodata_index[0].next_unrefd == 0) {
    uint16_t last_link = 0;
    // Maybe uninitialized? Rebuild the chain.
    for (uint16_t i = 1; i < LENGTHOF(rodata_index); ++i) {
      if (rodata_index[i].reclaimer == NULL) {
        rodata_index[last_link].next_unrefd = i;
        last_link = i;
        assert(rodata_index[i].next_unrefd == 0);
      }
    }
  }
  // If we still don't have a next_unrefd, we need a bigger index
  assert(rodata_index[0].next_unrefd != 0);
  assert(rodata_index[0].next_unrefd < max_reclaimable);
  m_index = (uint8_t)rodata_index[0].next_unrefd;
  rodata_index[0].next_unrefd = rodata_index[m_index].next_unrefd;
  rodata_index[m_index].live_refs = 1;
  rodata_index[m_index].reclaimer = f;
}


void rodata::ref() {
  assert(m_index > 0);
  assert(m_index < max_reclaimable);
  assert(rodata_index[m_index].reclaimer != NULL);
  assert(rodata_index[m_index].live_refs > 0);

  ++rodata_index[m_index].live_refs;
}


void rodata::unref() {
  assert(m_index > 0);
  assert(m_index < max_reclaimable);
  assert(rodata_index[m_index].reclaimer != NULL);
  assert(rodata_index[m_index].live_refs > 0);

  if (rodata_index[m_index].live_refs > 1) {
    --rodata_index[m_index].live_refs;
  } else {
    // Reclaim the data
    rodata_index[m_index].reclaimer(data());

    // Add the index entry back into the unrefd chain
    rodata_index[m_index].reclaimer = NULL;
    rodata_index[m_index].next_unrefd = rodata_index[0].next_unrefd;
    rodata_index[0].next_unrefd = m_index;
  }
}
