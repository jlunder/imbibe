#include "imbibe.h"

#include "immutable.h"


struct immutable_tracking_t {
  immutable::reclaim_func_t reclaimer;
  union {
    uint16_t live_refs;
    uint16_t next_unrefd;
  };
};


immutable_tracking_t immutable_index[immutable::max_reclaimable + 1];


void immutable::init(reclaim_func_t f)
{
  if (!f) {
    m_index = 0;
    return;
  }

  assert(immutable_index[0].reclaimer == NULL);
  if (immutable_index[0].next_unrefd == 0) {
    uint16_t last_link = 0;
    // Maybe uninitialized? Rebuild the chain.
    for (uint16_t i = 1; i < LENGTHOF(immutable_index); ++i) {
      if (immutable_index[i].reclaimer == NULL) {
        immutable_index[last_link].next_unrefd = i;
        last_link = i;
        assert(immutable_index[i].next_unrefd == 0);
      }
    }
  }
  // If we still don't have a next_unrefd, we need a bigger index
  assert(immutable_index[0].next_unrefd != 0);
  assert(immutable_index[0].next_unrefd < max_reclaimable);
  m_index = (uint8_t)immutable_index[0].next_unrefd;
  immutable_index[0].next_unrefd = immutable_index[m_index].next_unrefd;
  immutable_index[m_index].live_refs = 1;
  immutable_index[m_index].reclaimer = f;
}


void immutable::ref() {
  assert(m_index > 0);
  assert(m_index < max_reclaimable);
  assert(immutable_index[m_index].reclaimer != NULL);
  assert(immutable_index[m_index].live_refs > 0);

  ++immutable_index[m_index].live_refs;
}


void immutable::unref() {
  assert(m_index > 0);
  assert(m_index < max_reclaimable);
  assert(immutable_index[m_index].reclaimer != NULL);
  assert(immutable_index[m_index].live_refs > 0);

  if (immutable_index[m_index].live_refs > 1) {
    --immutable_index[m_index].live_refs;
  } else {
    // Reclaim the data
    immutable_index[m_index].reclaimer(data());

    // Add the index entry back into the unrefd chain
    immutable_index[m_index].reclaimer = NULL;
    immutable_index[m_index].next_unrefd = immutable_index[0].next_unrefd;
    immutable_index[0].next_unrefd = m_index;
  }
}
