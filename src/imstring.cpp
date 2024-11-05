#include "imbibe.h"

#include <stdarg.h>

#include "imstring.h"
#include "vector.h"

#define logf_imstring(...) disable_logf("IMSTRING: " __VA_ARGS__)
#define IMSTRING_LOG_INTEGRITY 0

namespace aux_imstring {
// anonymous struct
uint32_t IMSTRING_POOL s_null_pad;
uint8_t IMSTRING_POOL s_dynamic_pool[s_dynamic_pool_size];

struct dynamic_header;

#if BUILD_MSDOS
typedef dynamic_header IMSTRING_POOL *dynamic_header_ptr_t;
typedef uint8_t IMSTRING_POOL *pool_ptr_t;
#else
template <class T, uint8_t __far *base, segsize_t range> struct based_ptr {
  typedef T __far *far_pointer_type;
  typedef T __far &far_reference_type;
  static constexpr segsize_t value_size = sizeof(T);
  segsize_t offset;

  based_ptr(based_ptr const __far &n_ptr) : offset(n_ptr.offset) {
    assert(valid());
  }
  based_ptr() { offset = SEGSIZE_INVALID - 1; }
  explicit based_ptr(segsize_t n_offset) {
    assert((n_offset == SEGSIZE_INVALID) || (n_offset <= range));
    offset = n_offset;
  }
  template <class U>
  based_ptr(based_ptr<U, base, range> const __far &n_uptr)
      : offset(n_uptr.offset) {
    assert(valid());
  }
  based_ptr(far_pointer_type p) {
    if (p) {
      assert(reinterpret_cast<uint8_t __far *>(p) >= base);
      assert(reinterpret_cast<uint8_t __far *>(p) <= (base + range));
      offset = reinterpret_cast<uint8_t __far *>(p) - base;
    } else {
      offset = SEGSIZE_INVALID;
    }
  }

  based_ptr &operator=(based_ptr const &n_ptr) {
    offset = n_ptr.offset;
    return *this;
  }
  void operator=(based_ptr const __far &n_ptr) __far { offset = n_ptr.offset; }
  based_ptr &operator=(void *np) {
    // Assign NULL
    assert(!np);
    offset = SEGSIZE_INVALID;
    return *this;
  }
  void operator=(void *np) __far {
    // Assign NULL
    assert(!np);
    offset = SEGSIZE_INVALID;
  }

  far_pointer_type ptr() const {
    assert(nonnull());
    return reinterpret_cast<T __far *>(base + offset);
  }

  bool valid() const {
    return (offset == SEGSIZE_INVALID) || (offset <= range);
  }

  bool nonnull() const {
    return (offset != SEGSIZE_INVALID) && (offset <= range);
  }

  bool derefable() const {
    static_assert(sizeof(T) < SEGDIFF_MAX / 16);
    return (offset != SEGSIZE_INVALID) && (offset + sizeof(T) <= range);
  }

  far_reference_type operator*() const {
    assert(derefable());
    return *ptr();
  }
  far_pointer_type operator->() const {
    assert(derefable());
    return ptr();
  }
  far_reference_type operator[](segsize_t i) const {
    assert(derefable());
    assert(i < ((range - offset) / value_size));
    return ptr()[i];
  }
  far_reference_type operator[](segdiff_t i) const {
    assert(derefable());
    assert(((i >= 0) &&
            (i < static_cast<segdiff_t>((range - offset) / value_size))) ||
           ((-i > 0) && (-i <= static_cast<segdiff_t>(offset / value_size))));
    return reinterpret_cast<T __far *>(base + offset)[i];
  }

  operator far_pointer_type() const { return ptr(); }

  based_ptr operator+(segdiff_t i) const {
    assert(i >= -SEGDIFF_MAX);
    assert(((i >= 0) &&
            (i <= static_cast<segdiff_t>((range - offset) / value_size))) ||
           ((-i > 0) && (-i <= static_cast<segdiff_t>(offset / value_size))));
    return based_ptr(offset + i * sizeof(T));
  }
  based_ptr operator+(segsize_t i) const {
    assert(i <= static_cast<segsize_t>((range - offset) / value_size));
    return based_ptr(offset + i * sizeof(T));
  }

  based_ptr operator-(segdiff_t i) const { return operator+(-i); }
  based_ptr operator-(segsize_t i) const {
    assert(i <= SEGDIFF_MAX);
    return operator+(static_cast<segdiff_t>(-static_cast<segdiff_t>(i)));
  }
  segdiff_t operator-(based_ptr p) const {
    assert(abs(offset - p.offset) % sizeof(T) == 0);
    return static_cast<segdiff_t>((offset - p.offset) / sizeof(T));
  }
  segdiff_t operator-(far_pointer_type p) const {
    assert(abs(offset - based_ptr(p).offset) % sizeof(T) == 0);
    return static_cast<segdiff_t>((offset - based_ptr(p).offset) / sizeof(T));
  }

  bool operator==(based_ptr other) const { return offset == other.offset; }
  bool operator!=(based_ptr other) const { return offset != other.offset; }
  bool operator<(based_ptr other) const { return offset < other.offset; }
  bool operator>=(based_ptr other) const { return offset >= other.offset; }
  bool operator>(based_ptr other) const { return offset > other.offset; }
  bool operator<=(based_ptr other) const { return offset <= other.offset; }

  template <class U> operator based_ptr<U, base, range>() const {
    assert(valid());
    return based_ptr<U, base, range>(offset);
  }

  operator bool() const {
    assert(valid());
    return offset != SEGSIZE_INVALID;
  }
};

typedef based_ptr<dynamic_header, s_dynamic_pool, s_dynamic_pool_size>
    dynamic_header_ptr_t;
typedef based_ptr<uint8_t, s_dynamic_pool, s_dynamic_pool_size> pool_ptr_t;
#endif

struct dynamic_header {
  segsize_t ref_count;
  segsize_t size;
  // If allocated, pref_free and next_free are not used -- they are overwritten
  // by the stored string
  dynamic_header_ptr_t prev_free;
  dynamic_header_ptr_t next_free;
  uint8_t pad[s_page_size - 8];
};

static segsize_t const s_str_offset = offsetof(dynamic_header, prev_free);

static_assert(sizeof(dynamic_header) == s_page_size);

dynamic_header_ptr_t s_first_free;

static inline segsize_t round_to_page(segsize_t x) {
  return (x + (s_page_size - 1)) & (~(s_page_size - 1));
}

static inline bool is_free(dynamic_header_ptr_t header) {
  return header->ref_count == 0;
}

static inline pool_ptr_t as_pool(dynamic_header_ptr_t header) {
  assert(header);
#if BUILD_MSDOS
  return reinterpret_cast<pool_ptr_t>(header);
#else
  return static_cast<pool_ptr_t>(header);
#endif
}

static inline dynamic_header_ptr_t as_header(pool_ptr_t pool_ptr) {
  assert(pool_ptr);
#if BUILD_MSDOS
  assert(pool_ptr >= s_dynamic_pool);
  assert((pool_ptr - s_dynamic_pool) % s_page_size == 0);
  return reinterpret_cast<dynamic_header_ptr_t>(pool_ptr);
#else
  assert((pool_ptr - s_dynamic_pool) % s_page_size == 0);
  return dynamic_header_ptr_t(pool_ptr);
#endif
}

static inline dynamic_header_ptr_t header_from_str(char const __far *str) {
  assert(is_dynamic(str));
  return as_header(pool_ptr_t((uint8_t __far *)str) - s_str_offset);
}

static inline dynamic_header_ptr_t
next_consecutive(dynamic_header_ptr_t header) {
  return as_header(as_pool(header) +
                   round_to_page(header->size + s_str_offset));
}

static inline dynamic_header_ptr_t next_free(dynamic_header_ptr_t header) {
  return header->next_free;
}

static inline dynamic_header_ptr_t prev_free(dynamic_header_ptr_t header) {
  return header->prev_free;
}

static inline void set_next_free(dynamic_header_ptr_t header,
                                 dynamic_header_ptr_t next_header) {
  header->next_free = next_header;
}

static inline void set_prev_free(dynamic_header_ptr_t header,
                                 dynamic_header_ptr_t prev_header) {
  header->prev_free = prev_header;
}

#if BUILD_DEBUG
segsize_t offset_from_header(dynamic_header_ptr_t header) {
  if (!header) {
    return SEGSIZE_INVALID;
  }
  return as_pool(header) - s_dynamic_pool;
}

bool check_integrity() {
  dynamic_header_ptr_t header = as_header(s_dynamic_pool);
  dynamic_header_ptr_t expected_prev_free = NULL;
  dynamic_header_ptr_t expected_next_free = s_first_free;
  dynamic_header_ptr_t pool_end =
      as_header(s_dynamic_pool + s_dynamic_pool_size);
#if IMSTRING_LOG_INTEGRITY
  logf_imstring("check_integrity: first_free=%5u, header[0]=%5u/%u refs\n",
                (unsigned)offset_from_header(s_first_free),
                (unsigned)header->size, (unsigned)header->ref_count);
#endif
  while (header < pool_end) {
#if IMSTRING_LOG_INTEGRITY
    if (header->ref_count == 0) {
      logf_imstring("  header[%5u]=%5u <-%5u/%5u->\n",
                    (unsigned)offset_from_header(header),
                    (unsigned)header->size,
                    (unsigned)offset_from_header(header->prev_free),
                    (unsigned)offset_from_header(header->next_free));
    } else {
      logf_imstring(
          "  header[%5u]=%5u (%u refs) \"%s\"\n",
          (unsigned)offset_from_header(header), (unsigned)header->size,
          (unsigned)header->ref_count,
          (char const *)(static_cast<uint8_t __far *>(
              aux_imstring::as_pool(header) + aux_imstring::s_str_offset)));
    }
#endif
    if (header->ref_count == 0) {
      // this is a free block
      // check that this was the free block linked in proper order
      assert(header == expected_next_free);
      // check that the last free block visited is where we link back to
      assert(prev_free(header) == expected_prev_free);
      // expect this block as the next block's previous
      expected_prev_free = header;
      // expect the linked next as the next free block we encounter
      expected_next_free = next_free(header);
      // consecutive free blocks should be coalesced
      assert(expected_next_free != next_consecutive(header));
    }
    // find the next block, free or allocated
    header = next_consecutive(header);
  }
  // we should reach the end exactly
  assert(header == pool_end);
  // the last block's next free should be null
  assert(!expected_next_free);
  return true;
}

void log_live() {
  dynamic_header_ptr_t header = as_header(s_dynamic_pool);
  dynamic_header_ptr_t pool_end =
      as_header(s_dynamic_pool + s_dynamic_pool_size);
  while (header < pool_end) {
    if (header->ref_count != 0) {
      logf_any("IMSTRING: live string [%u] (x%u): \"%s\"\n",
               (unsigned)offset_from_header(header),
               (unsigned)header->ref_count,
               (char const *)(static_cast<uint8_t __far *>(as_pool(header) +
                                                           s_str_offset)));
    }
    header = next_consecutive(header);
  }
}
#endif

dynamic_header_ptr_t alloc_dynamic(segsize_t raw_size) {
  assert(check_integrity());
  assert(raw_size < s_dynamic_pool_size / 16);
  // Find a free block big enough for our allocation
  dynamic_header_ptr_t prev_header = NULL;
  dynamic_header_ptr_t header = s_first_free;
  while (header && header->size < raw_size) {
    prev_header = header;
    header = next_free(header);
    assert(!header || (header->ref_count == 0));
  }
  if (!header) {
    abortf("imstring out of memory");
  }
  // Gather accounting info -- actual allocation size, next block
  dynamic_header_ptr_t next_header = next_free(header);
  segsize_t alloc_pages = round_to_page(s_str_offset + raw_size);
  segsize_t avail_pages = s_str_offset + header->size;
  assert(alloc_pages <= avail_pages);
  assert(avail_pages == round_to_page(avail_pages));
  // Adjust the block header -- it's allocated now
  header->ref_count = 1;
  header->size = raw_size;
  // Did we allocate the whole block?
  if (alloc_pages < avail_pages) {
    // We allocated less than the whole block -- split it
    // The split_header is the new (smaller) free block we'll create
    dynamic_header_ptr_t split_header = next_consecutive(header);
    assert(split_header);
    assert(!next_header || (split_header < next_header));
    // Set up the new split header
    split_header->ref_count = 0;
    split_header->size = avail_pages - alloc_pages - s_str_offset;
    // Link the split header to the prev and next free
    set_prev_free(split_header, prev_header);
    if (prev_header) {
      set_next_free(prev_header, split_header);
    } else {
      s_first_free = split_header;
    }
    set_next_free(split_header, next_header);
    if (next_header) {
      set_prev_free(next_header, split_header);
    }
  } else {
    // Assert that alloc_pages isn't GREATER than avail_pages
    assert(alloc_pages == avail_pages);
    // Whole block was needed, just link the prev and next blocks to each other
    if (prev_header) {
      set_next_free(prev_header, next_header);
    } else {
      s_first_free = next_header;
    }
    if (next_header) {
      set_prev_free(next_header, prev_header);
    }
  }
  assert(check_integrity());
  logf_imstring("alloc [%u] (len %u)\n", (unsigned)offset_from_header(header),
                (unsigned)raw_size);
  return header;
}

void free_dynamic(dynamic_header_ptr_t header) {
  assert(header);
  assert(header->ref_count == 0);
  logf_imstring("free [%u] (len %u)\n", (unsigned)offset_from_header(header),
                (unsigned)header->size);
  header->size = round_to_page(s_str_offset + header->size) - s_str_offset;
  // Edge case: the pool is 100% FULL
  if (!s_first_free) {
    s_first_free = header;
    header->prev_free = NULL;
    header->next_free = NULL;
    return;
  }
  // Find the free blocks before and after this block, bracketing it
  dynamic_header_ptr_t prev_header;
  dynamic_header_ptr_t next_header;
  if (s_first_free > header) {
    prev_header = NULL;
    next_header = s_first_free;
  } else {
    assert(s_first_free < header);
    prev_header = s_first_free;
    next_header = next_consecutive(header);
    dynamic_header_ptr_t pool_end =
        as_header(s_dynamic_pool + s_dynamic_pool_size);
    do {
      // Advance the free block iterator
      if (prev_header) {
        // We are walking through the free block list from its beginning,
        // looking for a block after the one we're freeing
        dynamic_header_ptr_t prev_next = next_free(prev_header);
        assert(prev_next != header);
        if (prev_next > header) {
          // Found a succeeding free block!
          next_header = prev_next;
          break;
        }
        prev_header = prev_next;
      }
      // Advance the consecutive block iterator
      if (next_header) {
        // We are walking through consecutive blocks from the header we are
        // trying to free, to see if we find a free one, from which we can
        // locate the preceding free block directly
        if (next_header->ref_count == 0) {
          // Found a succeeding free block!
          prev_header = prev_free(next_header);
          break;
        }
        // Not yet -- advance one
        next_header = next_consecutive(next_header);
        if (next_header == pool_end) {
          next_header = NULL;
        }
      }
    } while (prev_header || next_header);
  }
  if (prev_header) {
    if (next_consecutive(prev_header) == header) {
      // adjacent: coalesce with prev_header
      assert(header->size ==
             (round_to_page(s_str_offset + header->size) - s_str_offset));
      prev_header->size += s_str_offset + header->size;
      header = prev_header;
    } else {
      set_next_free(prev_header, header);
      set_prev_free(header, prev_header);
    }
  } else {
    s_first_free = header;
    set_prev_free(header, NULL);
  }
  if (next_header) {
    if (next_consecutive(header) == next_header) {
      // adjacent: coalesce with next_header
      assert(next_header->size ==
             (round_to_page(s_str_offset + next_header->size) - s_str_offset));
      header->size += s_str_offset + next_header->size;
      next_header = next_free(next_header);
    }
    set_next_free(header, next_header);
    // next_header may become null through coalesce
    if (next_header) {
      set_prev_free(next_header, header);
    }
  }
  assert(check_integrity());
}

} // namespace aux_imstring

char __far *imstring::alloc_dynamic(segsize_t raw_size) {
  char __far *str = reinterpret_cast<char __far *>(static_cast<uint8_t __far *>(
      aux_imstring::as_pool(aux_imstring::alloc_dynamic(raw_size)) +
      aux_imstring::s_str_offset));
#if BUILD_MSDOS
  logf_imstring("allocated " PRpF ", seg=0x%04X, expect=%04X\n", str,
                (unsigned)FP_SEG(str), (unsigned)__segname(IMSTRING_POOL_NAME));
#endif
  assert(aux_imstring::is_dynamic(str));
  return str;
}

void imstring::ref_dynamic(char const __far *str) {
  assert(aux_imstring::check_integrity());
  aux_imstring::dynamic_header_ptr_t header =
      aux_imstring::header_from_str(str);
  logf_imstring("ref [%u] (len %u, x%u+1)\n",
                (unsigned)aux_imstring::offset_from_header(header),
                (unsigned)header->size, (unsigned)header->ref_count);
  assert(header->ref_count > 0);
  assert(header->ref_count < 10000);
  ++header->ref_count;
}

void imstring::unref_dynamic(char const __far *str) {
  assert(aux_imstring::check_integrity());
  aux_imstring::dynamic_header_ptr_t header =
      aux_imstring::header_from_str(str);
  logf_imstring("unref [%u] (len %u, x%u-1)\n",
                (unsigned)aux_imstring::offset_from_header(header),
                (unsigned)header->size, (unsigned)header->ref_count);
  assert(header->ref_count > 0);
  assert(header->ref_count < 10000);
  --header->ref_count;
  if (header->ref_count == 0) {
    aux_imstring::free_dynamic(header);
  }
}

imstring imstring::copy(char const __far *str, segsize_t len) {
  if (str == NULL) {
    assert((len == 0) || (len == SEGSIZE_INVALID));
    return imstring(NULL);
  }
  assert(_fstrlen(str) < SEGSIZE_MAX);
  assert((len == SEGSIZE_INVALID) || (len == _fstrlen(str)));
  if (*str == '\0') {
    return imstring("");
  }
  segsize_t raw_size;
  if (len == SEGSIZE_INVALID) {
    raw_size = _fstrlen(str) + 1;
  } else {
    raw_size = len + 1;
  }
  char __far *dest_str = alloc_dynamic(raw_size);
  _fmemcpy(dest_str, str, raw_size);
  return imstring(noref(), dest_str);
}

imstring imstring::format(char const *fmt, ...) {
  char temp_str[1024];
  va_list va;
  va_start(va, fmt);
  vsnprintf(temp_str, LENGTHOF(temp_str), fmt, va);
  va_end(va);
  return copy(temp_str);
}

void imstring::setup() {
#if BUILD_MSDOS
  assert(FP_OFF(aux_imstring::s_dynamic_pool) != 0);
#endif

  aux_imstring::s_first_free =
      aux_imstring::as_header(aux_imstring::s_dynamic_pool);
  aux_imstring::s_first_free->ref_count = 0;
  aux_imstring::s_first_free->size =
      aux_imstring::s_dynamic_pool_size - aux_imstring::s_str_offset;
  aux_imstring::s_first_free->prev_free = NULL;
  aux_imstring::s_first_free->next_free = NULL;
}

void imstring::teardown() {
#if BUILD_DEBUG
  aux_imstring::log_live();
#endif
  assert(aux_imstring::s_first_free ==
         aux_imstring::as_header(aux_imstring::s_dynamic_pool));
  assert(aux_imstring::s_first_free->ref_count == 0);
  assert(aux_imstring::s_first_free->size ==
         aux_imstring::s_dynamic_pool_size - aux_imstring::s_str_offset);
}

void imstring::teardown_exiting() {
  // do nothing, drop allocated memory on the floor
#if BUILD_DEBUG
  teardown();
#endif
}
