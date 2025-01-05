#include "imbibe.h"

#include "resource_manager.h"

#include "builtin_data.h"
#include "fletcher16.h"
#include "inplace.h"
#include "map.h"
#include "tbm.h"
#include "unpacker.h"

#define logf_resource_manager(...) logf_any("RESOURCE_MANAGER: " __VA_ARGS__)

#define BUILD_TYAR_LOADER 1

#if BUILD_TYAR_LOADER

namespace tyar {

// This is just a constant string, but it's somewhat carefully selected.
// The magic value can't realistically be protected by the header check value
// because we don't know how long the header is until we've checked the magic
// to figure out the file format -- so we have a chicken-and-egg problem. To
// illustrate the magnitude of the problem, the difference between "TYAR0"
// and "TYAR1" is a single bit flip, not so unlikely at all if we presuppose
// some data corruption! We could get around this by CRC'ing the whole file
// with a check value tacked on at the end, but of course then you have to
// read the entire file before you can process ANY of it, which is s-l-o-w on
// old min spec computers. We would like to be able to check stuff just-in-
// time right before we process it.

// One way to get around this is to ensure that any valid magic value will be
// at least some Hamming distance from the ones we're looking for, and that's
// the plan here.

// So, although to the module user these are just constants, if any future
// magic value is defined, please ensure it's at least HD 20 from any
// previously defined constants.
static uint8_t const header_magic_v0[12] = {'T',  'Y',  'A',  'R',  '0',  0x1A,
                                            0x00, 0x00, 0xD2, 0x6E, 0xD5, 0xCD};

static uint8_t const archive_header_toc_magic_v0[4] = {'T', 't', 'c', '0'};
static uint8_t const archive_header_hash_magic_v0[4] = {'T', 'h', 't', '0'};

static uint16_t const hash_type_fletcher16_hopscotch = 'f' + ('h' << 8);

static uint8_t const toc_entry_name_magic_v0[4] = {'T', 'f', 'n', '0'};
static uint8_t const toc_entry_file_magic_v0[4] = {'T', 'f', 'd', '0'};

// 64 bytes
struct archive_header {
  // 'TYAR0\x1A\x00\x00\xD2\x6E\xD5\xCD'
  uint8_t header_magic[12];
  uint32_t header_check_value; // 0 for now
  uint32_t archive_size;

  // 'Ttc0'
  uint8_t toc_magic[4];
  uint32_t toc_offset;
  uint32_t toc_size;
  uint32_t toc_check_value; // 0 for now
  // 'Tht0'
  uint8_t hash_magic[4];
  uint32_t hash_offset;
  uint32_t hash_size;
  uint32_t hash_check_value; // 0 for now
};

// 8 bytes
struct hash_header {
  // 'fh' for Fletcher-16, hopscotch hashing
  uint16_t hash_type;
  // Should always be a power of 2
  uint16_t bin_count;
  // The neighbor_count is how far we might have to search for any given hash
  // value. Farther than that, we can give up knowing it's not in the table.
  // This is a property guaranteed by hopscotch hashing; for reads, it's
  // otherwise just like a linear open table.
  uint16_t neighbor_count;
  uint16_t reserved_0;
};

// 4 bytes
struct hash_entry_fletcher16 {
  uint16_t name_hash; // Fletcher-16, 0 seed
  uint16_t toc_index; // 0xFFFF for an unoccupied entry
};

// 32 bytes
struct toc_entry {
  // 'Tfn0'
  uint8_t name_magic[4];
  uint32_t name_offset;
  uint32_t name_size;
  uint32_t name_check_value;
  // 'Tfd0'
  uint8_t file_magic[4];
  uint32_t file_offset;
  uint32_t file_size;
  uint32_t file_check_value;
};

} // namespace tyar

namespace resource_manager {

static __segment archive_data_seg = 0;
static uint32_t archive_size = 0;
static tyar::hash_entry_fletcher16 __far *archive_hash;
static tyar::toc_entry __far *archive_toc;

} // namespace resource_manager

void resource_manager::setup(imstring const &archive_name) {
  char const __far *path = "data.tya";
  if (!archive_name.null()) {
    path = archive_name.c_str();
  }

  int handle = -1;
  unsigned err;

#if BUILD_DEBUG
  char const *op = "opening archive";
#endif
  archive_data_seg = 0;

  unsigned long size = UINT32_MAX;
  err = _dos_open(path, O_RDONLY, &handle);
  if (err == 0) {
#if BUILD_DEBUG
    op = "getting archive file size";
#endif
    err = _dos_lseek(handle, 0, SEEK_END, &size);
    if (err == 0) {
      archive_size = size;
      err = _dos_lseek(handle, 0, SEEK_SET, &size);
    }
  }
  if (err == 0) {
#if BUILD_DEBUG
    op = "allocating archive data";
#endif
    if (archive_size / 16 > UINT16_MAX) {
      err = -1;
    } else {
      err = _dos_allocmem(archive_size / 16, &archive_data_seg);
    }
  }
  if (err == 0) {
#if BUILD_DEBUG
    op = "reading archive";
#endif
    uint32_t read_total = 0;
    while ((err == 0) && (read_total < archive_size)) {
      unsigned read_actual = 0;
      err = _dos_read(handle, MK_FP_O32(archive_data_seg, read_total),
                      min<uint32_t>(archive_size - read_total, 32768u),
                      &read_actual);
      if (err == 0) {
        read_total += read_actual;
      }
    }
  }
  if (err == 0) {
    err = _dos_close(handle);
    if (err != 0) {
      logf_resource_manager(
          "ignoring error %u during close, archive '%" PRsF "'\n", err, path);
    }
  } else {
    if (handle != -1) {
      _dos_close(handle);
    }
#if BUILD_DEBUG
    abortf("error during %s (%u), archive '%" PRsF "'\n", op, err, path);
#else
    abortf("unable to read archive '%" PRsF "'\n", op, err, path);
#endif
  }
}

void resource_manager::teardown() {
  if (archive_data_seg) {
    _dos_freemem(archive_data_seg);
    archive_size = 0;
    archive_hash = NULL;
    archive_toc = NULL;
  }
}

void resource_manager::teardown_exiting() {
#if BUILD_DEBUG
  teardown();
#else
  // not actually sure it's okay to leak DOS-allocated resources here
  _dos_freemem(FP_SEG(archive_data));
#endif
}

segsize_t resource_manager::fetch_data(imstring const &name,
                                       immutable *out_data) {
  (void)name;
  (void)out_data;
  assert(!"NOT IMPLEMENTED");
  // logf_resource_manager("resource_manager::fetch_data\n");
  // assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  // return find_or_load_data(name, out_data);
}

#else

namespace resource_manager {

static const segsize_t s_hash_capacity = 128;
static const segsize_t s_index_initial_capacity = 8;
static const segsize_t s_empty_index = UINT16_MAX;

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

imstring s_folder_name;

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

void resource_manager::setup(imstring const &archive_name) {
  if (archive_name) {
    s_folder_name = archive_name;
  } else {
    s_folder_name = "testdata";
  }

  hash_entry blank;
  blank.hash = s_empty_index;
  blank.index = s_empty_index;
  assert(s_name_hash.empty());
  assert(s_index.empty());
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

  snprintf(path_buf, sizeof path_buf, "%" PRsF "/%" PRsF, s_folder_name.c_str(),
           name.c_str());
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

#endif
