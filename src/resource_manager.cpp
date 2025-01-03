#include "imbibe.h"

#include "resource_manager.h"

#include "builtin_data.h"
#include "fletcher16.h"
#include "inplace.h"
#include "map.h"
#include "tbm.h"
#include "unpacker.h"

#define logf_resource_manager(...)                                             \
  disable_logf("RESOURCE_MANAGER: " __VA_ARGS__)

namespace resource_manager {

static const segsize_t s_hash_capacity = 128;
static const segsize_t s_index_initial_capacity = 8;
static const segsize_t s_empty_index = UINT16_MAX;

// 64 bytes
struct tyar_header {
  // This is just a constant string, but it's somewhat carefully selected.
  // The magic value can't realistically be protected by the header check value
  // because we don't know how long the header is until we've checked the magic
  // to figure out the file version -- so we have a chicken-and-egg problem. To
  // illustrate the magnitude of the problem, the difference between "TYAR0"
  // and "TYAR1" is a single bit flip, not so unlikely at all if we presuppose
  // some data corruption!

  // One way to get around this is to ensure that any valid magic value will be
  // at least some Hamming distance from the ones we're looking for, and that's
  // our plan.

  // So, although to the library user these are just constants, if any future
  // magic value is defined, please ensure that the last 4 bytes are a CRC32 of
  // the first 8, using this very specific CRC:
  //  - polynomial 0x1B49C1C96 (most calculators will want just 0xB49C1C96);
  //  - initial seed 0xFFFFFFFF
  //  - final check value inverted i.e. XOR with 0xFFFFFFFF
  //  - bits are processed and output LSB-to-MSB i.e. right-shift/"reflected"
  // This should match iSCSI CRC32 processing but with a different polynomial,
  // which was selected to optimize HD for the 64 bits we're checking. The CRC
  // is written into the string constant little-endian, and it's wise to
  // confirm before generating a new constant that you can regenerate what was
  // used for the old one -- this is a lot of effort for a constant, but one
  // bad one ruins the HD performance for the whole scheme all!

  // 'TYAR0\x1A\x00\x00\xD2\x6E\xD5\xCD'
  char header_magic[12];
  uint32_t header_check_value; // 0 for now
  uint32_t archive_size;

  // 'Ttc0'
  char toc_magic[4];
  uint32_t toc_offset;
  uint32_t toc_size;
  uint32_t toc_check_value; // 0 for now
  // 'Thf0'
  char hash_magic[4];
  uint32_t hash_offset;
  uint32_t hash_size;
  uint32_t hash_check_value; // 0 for now
};

// 8 bytes
struct tyar_hash_header {
  // 'fh' for Fletcher-16, hopscotch
  uint16_t hash_type;
  uint16_t bin_count;
  uint16_t neighbor_count;
  uint16_t reserved_0;
};

// 4 bytes
struct tyar_hash_entry_fletcher16 {
  uint16_t name_hash; // Fletcher-16, 0 seed
  uint16_t toc_index; // 0xFFFF for an unoccupied entry
};

// 32 bytes
struct tyar_toc_entry {
  // 'Tfn0'
  char name_magic[4];
  uint32_t name_offset;
  uint32_t name_size;
  uint32_t name_check_value;
  // 'Tfd0'
  char file_magic[4];
  uint32_t file_offset;
  uint32_t file_size;
  uint32_t file_check_value;
};

typedef segsize_t hash_t;

struct hash_entry {
  hash_t hash;
  segsize_t index;
};

struct index_entry {
  imstring name;
  union {
    segsize_t size;
    segsize_t next_free;
  };
  weak_immutable data;
};

struct reclaim_header {
  segsize_t index;
};

segsize_t s_hash_mask;
vector<hash_entry> s_name_hash;
vector<index_entry> s_index;
segsize_t s_first_free_index;

segsize_t find_or_load_data(imstring const &name, immutable *out_data);
segsize_t load_data_at_hash(imstring const &name, hash_t hash, segsize_t hint,
                            immutable *out_data);
void __far *load_data_from_file(imstring const &name, segsize_t *out_size);
void reclaim_loaded_data(void __far *data);

reclaim_header __far *reclaim_header_from_data(void __far *data) {
  return reinterpret_cast<reclaim_header __far *>(data) - 1;
}

void __far *data_from_reclaim_header(reclaim_header __far *header) {
  return reinterpret_cast<void __far *>(header + 1);
}

} // namespace resource_manager

void resource_manager::setup() {
  hash_entry blank;
  blank.hash = s_empty_index;
  blank.index = s_empty_index;
  assert(s_name_hash.empty());
  assert(s_index.empty());
  // s_name_hash.mem(arena::c());
  // s_index.mem(arena::c());
  s_name_hash.assign(s_hash_capacity, blank);
  s_hash_mask = s_hash_capacity - 1;
  s_first_free_index = s_empty_index;
}

void resource_manager::teardown() {
  s_name_hash.clear();
  s_index.clear();
  s_first_free_index = s_empty_index;
}

void resource_manager::teardown_exiting() {
  // it's okay to leak everything here
#if BUILD_DEBUG
  teardown();
#endif
}

tbm resource_manager::fetch_tbm(imstring const &name) {
  logf_resource_manager("resource_manager::fetch_tbm\n");
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  immutable data;
  segsize_t size = find_or_load_data(name, &data);
  return tbm(data, size);
}

segsize_t resource_manager::fetch_data(imstring const &name,
                                       immutable *out_data) {
  logf_resource_manager("resource_manager::fetch_tbm\n");
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  return find_or_load_data(name, out_data);
}

segsize_t resource_manager::find_or_load_data(imstring const &name,
                                              immutable *out_data) {
  assert(s_hash_mask == (s_name_hash.size() - 1));
  assert(s_hash_mask == 15 || s_hash_mask == 31 || s_hash_mask == 63 ||
         s_hash_mask == 127 || s_hash_mask == 255 || s_hash_mask == 511);
  logf_resource_manager("find_or_load_data '%" PRsF "'\n", name.c_str());
  hash_t name_hash = fletcher16_str(name.c_str());
  logf_resource_manager("  fletcher16_str = %04x\n", (unsigned)name_hash);
  segsize_t hint = (name_hash + (name_hash >> 8)) & s_hash_mask;
  for (segsize_t i = hint;; i = (i + 1) & s_hash_mask) {
    // don't search too far
    assert(((i + s_hash_capacity - hint) & s_hash_mask) < 10);
    if (s_name_hash[i].index == s_empty_index) {
      // definitively not found
      logf_resource_manager("  not found at %d\n", (int)i);
      return load_data_at_hash(name, name_hash, i, out_data);
    } else if (s_name_hash[i].hash == name_hash) {
      // maybe our thing?
      logf_resource_manager("  matching hash at %d\n", (int)i);
      segsize_t index = s_name_hash[i].index;
      if (s_index[index].name == name) {
        // that's us!
        logf_resource_manager("  found at %d!\n", (int)i);
        *out_data = s_index[index].data.lock();
        if (!*out_data) {
          // already indexed, but became unloaded -- reload
          segsize_t size = 0;
          void __far *data = load_data_from_file(name, &size);
          assert(data);
          assert(size == s_index[index].size);
          reclaim_header_from_data(data)->index = index;
          out_data->assign(reclaim_loaded_data, data);
          s_index[index].data = *out_data;
        }
        return s_index[index].size;
      } else {
        logf_resource_manager("  clashing names: '%" PRsF "', '%" PRsF "'\n",
                              s_index[index].name.c_str(), name.c_str());
        // otherwise fall through
      }
    }
    logf_resource_manager("  occupied slot at %d\n", (int)i);
    // some other thing, just continue...
  }
}

segsize_t resource_manager::load_data_at_hash(imstring const &name, hash_t hash,
                                              segsize_t hint,
                                              immutable *out_data) {
  assert(hash == fletcher16_str(name.c_str()));
  assert(s_name_hash[hint].index == s_empty_index);
  assert(out_data);

  if (s_first_free_index == s_empty_index) {
    // out of index entries, expand the free list
    assert(s_index.capacity() < s_hash_capacity / 2);
    segsize_t target = min<segsize_t>(
        s_hash_capacity / 2,
        max<segsize_t>(s_index_initial_capacity, s_index.capacity() * 2));
    assert(target > s_index.size());
    s_index.reserve(target);
    s_first_free_index = s_index.size();
    index_entry e;
    e.data = weak_immutable();
    e.name = imstring();
    while (s_index.size() < (target - 1)) {
      e.next_free = s_index.size() + 1;
      s_index.push_back(e);
    }
    e.next_free = s_empty_index;
    s_index.push_back(e);
  }
  assert(s_first_free_index != s_empty_index);

  segsize_t size = 0;
  void __far *data = load_data_from_file(name, &size);

  if (data) {
    assert(size > 0);
    // Success! Put this entry in the database
    segsize_t index = s_first_free_index;
    s_name_hash[hint].hash = hash;
    s_name_hash[hint].index = index;
    s_first_free_index = s_index[s_first_free_index].next_free;
    s_index[index].name = name;
    reclaim_header_from_data(data)->index = index;
    out_data->assign(reclaim_loaded_data, data);
    s_index[index].data = *out_data;
    s_index[index].size = size;

    return size;
  } else {
    return SEGSIZE_INVALID;
  }
}

void __far *resource_manager::load_data_from_file(imstring const &name,
                                                  segsize_t *out_size) {
  int handle = -1;
  char path_buf[RESOURCE_NAME_LEN_MAX + 10];
  void __far *temp_data = NULL;
  segsize_t temp_size;
  unsigned err;

  char const *op = "open";
  (void)op;

  snprintf(path_buf, sizeof path_buf, "testdata/%" PRsF, name.c_str());
  err = _dos_open(path_buf, O_RDONLY, &handle);
  if (err != 0) {
    goto fail_return;
  }

  logf_resource_manager("  opened '%s' as %d (result %u)\n", path_buf, handle,
                        err);
  {
    unsigned long size = UINT32_MAX;
    op = "seek-to-end";
    err = _dos_lseek(handle, 0, SEEK_END, &size);
    logf_resource_manager("  seek to end at %lu (result %u)\n", size, err);
    if (err != 0) {
      goto fail_return;
    }
    op = "allocation (file too big)";
    static_assert(SEGSIZE_INVALID > (INT16_MAX / 2));
    if (size > (INT16_MAX / 2)) {
      goto fail_return;
    }
    if (size == 0) {
      goto fail_return;
    }
    reclaim_header __far *header = reinterpret_cast<reclaim_header __far *>(
        _fmalloc((segsize_t)(sizeof(reclaim_header) + size)));
    if (header == NULL) {
      goto fail_return;
    }
    temp_data = data_from_reclaim_header(header);
    temp_size = (segsize_t)size;
    logf_resource_manager("load data %" PRpF " -> header %" PRpF "\n",
                          temp_data, header);
  }

  {
    unsigned long dummy;
    op = "seek-to-end";
    err = _dos_lseek(handle, 0, SEEK_SET, &dummy);
    if (err != 0) {
      goto fail_return;
    }
  }

  {
    unsigned actual = 0;
    op = "read";
    err = _dos_read(handle, temp_data, temp_size, &actual);
    if (err != 0) {
      goto fail_return;
    }
    if (actual != temp_size) {
      op = "read (wrong size reported)";
      goto fail_return;
    }
  }

  {
    err = _dos_close(handle);
    if (err != 0) {
      logf_resource_manager("ignoring error %u during close, file '%" PRsF
                            "'\n",
                            err, name.c_str());
    }
  }

  *out_size = temp_size;
  return temp_data;

fail_return:
  if (temp_data) {
    _ffree(temp_data);
  }
  *out_size = 0;
  if (handle != -1) {
    _dos_close(handle);
  }
  logf_resource_manager("error during %s (%u), file '%" PRsF "'\n", op, err,
                        name.c_str());
  return NULL;
}

void resource_manager::reclaim_loaded_data(void __far *data) {
  reclaim_header __far *header = reclaim_header_from_data(data);
  logf_resource_manager("reclaim %" PRpF " -> header %" PRpF " = %u\n", data,
                        header, header->index);
  assert(header->index < s_index.size());

  segsize_t index = header->index;
  index_entry &entry = s_index[index];

#if BUILD_DEBUG
  header->index = s_empty_index;
#endif

  _ffree(header);

  assert(entry.data);
  entry.data = NULL;
  // Can't actually reclaim the index entry because that could involve
  // reindexing subsequent entries
  // entry.name = NULL;
  // entry.next_free = s_first_free_index;
  // s_first_free_index = index;
}
