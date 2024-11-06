#include "imbibe.h"

#include "immutable.h"

namespace aux_immutable {

static segsize_t const max_reclaimable = UINT8_MAX;

struct immutable_tracking_t {
  immutable::reclaim_func_t reclaimer;
#if BUILD_DEBUG
  void const __far *orig_ptr;
#endif
  __segment orig_seg;
  union {
    segsize_t live_refs;
    segsize_t next_unrefd;
  };
};

immutable_tracking_t immutable_index[max_reclaimable + 1];

} // namespace aux_immutable

void immutable::assign(prealloc_t policy, void const __far *p) {
  (void)policy;
  if (m_index) {
    unref();
  }
  if (p) {
    void const __far *norm_p = p;
    if (FP_OFF(p) > UINT8_MAX) {
      norm_p = normalize_segmented(p);
      assert(FP_OFF(norm_p) < 0x10);
    }
    m_seg = FP_SEG(norm_p);
    assert(m_seg != 0);
    m_index = 0;
    m_ofs = (uint8_t)FP_OFF(norm_p);
    assert(denormalize_segmented(FP_SEG(p), MK_FP(m_seg, m_ofs)) == p);
    logf_immutable("Acquiring untracked %" PRpF " normalized to %" PRpF "\n", p,
                   norm_p);
  } else {
    m_seg = 0;
    m_index = 0;
    m_ofs = 0;
  }
}

void immutable::assign(reclaim_func_t f, void const __far *p) {
  if (m_index) {
    unref();
  }
  if (p) {
    void const __far *norm_p = p;
    if (FP_OFF(p) > UINT8_MAX) {
      norm_p = normalize_segmented(p);
      assert(FP_OFF(norm_p) < 0x10);
    }
    m_seg = FP_SEG(norm_p);
    assert(m_seg != 0);
    m_ofs = (uint8_t)FP_OFF(norm_p);
    init(f, p);
    assert(denormalize_segmented(FP_SEG(p), MK_FP(m_seg, m_ofs)) == p);
    logf_immutable("Acquiring tracked %" PRpF " normalized to %" PRpF
                   ", index %u\n",
                   p, norm_p, m_index);
  } else {
    m_seg = 0;
    m_index = 0;
    m_ofs = 0;
  }
}

immutable &immutable::operator=(immutable const &other) {
  if (m_index != 0) {
    if (m_index == other.m_index) {
      return *this;
    }
    unref();
  }
  m_seg = other.m_seg;
  m_index = other.m_index;
  m_ofs = other.m_ofs;
  if (m_index != 0) {
    ref();
  }
  return *this;
}

void immutable::init(reclaim_func_t f, void const __far *orig_p) {
  if (!f) {
    m_index = 0;
    return;
  }

  aux_immutable::immutable_tracking_t &zero_tracking =
      aux_immutable::immutable_index[0];
  assert(zero_tracking.reclaimer == NULL);
  if (zero_tracking.next_unrefd == 0) {
    segsize_t last_link = 0;
    // Maybe uninitialized? Rebuild the chain.
    for (segsize_t i = 1; i < LENGTHOF(aux_immutable::immutable_index); ++i) {
      if (aux_immutable::immutable_index[i].reclaimer == NULL) {
        aux_immutable::immutable_index[last_link].next_unrefd = i;
        last_link = i;
        assert(aux_immutable::immutable_index[i].next_unrefd == 0);
      }
    }
  }
  // If we still don't have a next_unrefd, we need a bigger index
  assert(zero_tracking.next_unrefd != 0);
  assert(zero_tracking.next_unrefd < aux_immutable::max_reclaimable);
  segsize_t index = zero_tracking.next_unrefd;

  aux_immutable::immutable_tracking_t &tracking =
      aux_immutable::immutable_index[index];
  zero_tracking.next_unrefd = tracking.next_unrefd;
  tracking.live_refs = 1;
  tracking.reclaimer = f;
#if BUILD_DEBUG
  tracking.orig_ptr = orig_p;
#endif
  tracking.orig_seg = FP_SEG(orig_p);

  m_index = (uint8_t)index;
}

void immutable::ref() {
  assert(m_index > 0);
  assert(m_index < aux_immutable::max_reclaimable);
  assert(aux_immutable::immutable_index[m_index].reclaimer != NULL);
  assert(aux_immutable::immutable_index[m_index].live_refs > 0);

  logf_immutable("Ref %" PRpF ", index %u\n", data(), m_index);
  ++aux_immutable::immutable_index[m_index].live_refs;
}

void immutable::unref() {
  assert(m_index > 0);
  assert(m_index < aux_immutable::max_reclaimable);

  aux_immutable::immutable_tracking_t &tracking =
      aux_immutable::immutable_index[m_index];
  assert(tracking.reclaimer != NULL);
  assert(tracking.live_refs > 0);

  logf_immutable("Unref %" PRpF ", index %u\n", data(), m_index);
  if (tracking.live_refs > 1) {
    --tracking.live_refs;
  } else {
    // Reclaim the data
    void const __far *norm_p = data();
    void const __far *p = denormalize_segmented(tracking.orig_seg, norm_p);
#if BUILD_DEBUG
    assert(p == tracking.orig_ptr);
    tracking.orig_ptr = NULL;
#endif
    logf_immutable("Reclaim %" PRpF " denorm to %" PRpF ", index %u\n", norm_p, p,
                   m_index);
    tracking.reclaimer(const_cast<void __far *>(p));

    // Add the index entry back into the unrefd chain
    tracking.reclaimer = NULL;
    tracking.next_unrefd = aux_immutable::immutable_index[0].next_unrefd;
    aux_immutable::immutable_index[0].next_unrefd = m_index;
  }
}

immutable weak_immutable::lock() {
  if (m_index == 0) {
    return immutable();
  }

  assert(m_index > 0);
  assert(m_index < aux_immutable::max_reclaimable);
  aux_immutable::immutable_tracking_t &tracking =
      aux_immutable::immutable_index[m_index];
  assert(tracking.reclaimer);
  assert(tracking.live_refs > 0);
#if BUILD_DEBUG
  assert(denormalize_segmented(tracking.orig_seg, MK_FP(m_seg, m_ofs)) ==
         tracking.orig_ptr);
#endif

  if (!tracking.reclaimer || (tracking.live_refs == 0)) {
    *this = weak_immutable();
  }

  return immutable(m_seg, m_index, m_ofs);
}
