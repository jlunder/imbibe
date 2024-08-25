#include "imbibe.h"

#include "immutable.h"


struct immutable_tracking_t {
  immutable::reclaim_func_t reclaimer;
  __segment orig_seg;
  union {
    uint16_t live_refs;
    uint16_t next_unrefd;
  };
};


immutable_tracking_t immutable_index[immutable::max_reclaimable + 1];


void immutable::init(reclaim_func_t f, void const * orig_p)
{
  if (!f) {
    m_index = 0;
    return;
  }

  immutable_tracking_t & zero_tracking = immutable_index[0];
  assert(zero_tracking.reclaimer == NULL);
  if (zero_tracking.next_unrefd == 0) {
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
  assert(zero_tracking.next_unrefd != 0);
  assert(zero_tracking.next_unrefd < max_reclaimable);
  m_index = (uint8_t)zero_tracking.next_unrefd;

  immutable_tracking_t & tracking = immutable_index[m_index];
  zero_tracking.next_unrefd = tracking.next_unrefd;
  tracking.live_refs = 1;
  tracking.reclaimer = f;
  tracking.orig_seg = FP_SEG(orig_p);
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

  immutable_tracking_t & tracking = immutable_index[m_index];
  assert(tracking.reclaimer != NULL);
  assert(tracking.live_refs > 0);

  if (tracking.live_refs > 1) {
    --tracking.live_refs;
  } else {
    // Reclaim the data
    void * p = data();
    assert(FP_SEG(p) - tracking.orig_seg < 0x1000);
    void * orig_p = MK_FP(tracking.orig_seg,
      FP_OFF(p) + ((FP_SEG(p) - tracking.orig_seg) << 4));
    logf_immutable("de-normalized %p to %p\n", p, orig_p);
    tracking.reclaimer(orig_p);

    // Add the index entry back into the unrefd chain
    tracking.reclaimer = NULL;
    tracking.next_unrefd = immutable_index[0].next_unrefd;
    immutable_index[0].next_unrefd = m_index;
  }
}
