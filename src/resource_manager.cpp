#include "imbibe.h"

#include "resource_manager.h"

#include "builtin_data.h"
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

hash_t fletcher16_str(char const __far *s);
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

// #if !BUILD_POSIX
#if 0

extern hash_t asm_fletcher16_str(char const __far * s);
#if BUILD_MSDOS_WATCOMC
#pragma aux asm_fletcher16_str =                                               \
    "   dec     di              "                                              \
    "   xor     ax, ax          "                                              \
    "   xor     bx, bx          "                                              \
    "@loop:                     "                                              \
    "   inc     di              "                                              \
    "   mov     bl, es:[di]     "                                              \
    "   or      bx, bx          "                                              \
    "   je      @done           "                                              \
    "   add     al, bl          "                                              \
    "   jc      @roll_l         "                                              \
    "   cmp     al, 255         "                                              \
    "   jne     @noroll_l       "                                              \
    "@roll_l:                   "                                              \
    "   inc     al              "                                              \
    "@noroll_l:                 "                                              \
    "   jc      @roll_h         "                                              \
    "   cmp     al, 255         "                                              \
    "   jne     @loop           "                                              \
    "@roll_h:                   "                                              \
    "   inc     al              "                                              \
    "   jmp     @loop           "                                              \
    "@done:                     " modify[ax bl di] parm[es di] value[ax]
#else
// TODO
#endif

#endif

resource_manager::hash_t resource_manager::fletcher16_str(char const __far *s) {
#if 1 // BUILD_DEBUG || BUILD_POSIX
  uint16_t s1 = 0;
  uint16_t s2 = 0;
  for (char const __far *p = s; *p; ++p) {
    s1 += (uint8_t)*p;
    if (s1 >= 255) {
      s1 -= 255;
      assert(s1 < 255);
    }
    s2 += s1;
    if (s2 >= 255) {
      s2 -= 255;
      assert(s2 < 255);
    }
  }
  return (s2 << 8) | s1;
#endif
  // #if BUILD_MSDOS
  //   uint16_t result = asm_fletcher16_str(s);
  //   assert(result == ((s2 << 8) | s1));
  //   return result;
  // #elif BUILD_POSIX
  //   return (s2 << 8) | s1;
  // #else
  // #error New platform support needed?
  // #endif
}

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
        arena::c_alloc((segsize_t)(sizeof(reclaim_header) + size)));
    if (header == NULL) {
      goto fail_return;
    }
    temp_data = data_from_reclaim_header(header);
    temp_size = (segsize_t)size;
    logf_resource_manager("load data %" PRpF " -> header %" PRpF "\n", temp_data,
                          header);
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
      logf_resource_manager("ignoring error %u during close, file '%" PRsF "'\n",
                            err, name.c_str());
    }
  }

  *out_size = temp_size;
  return temp_data;

fail_return:
  if (temp_data) {
    arena::c_free(temp_data);
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

  arena::c_free(header);

  assert(entry.data);
  entry.data = NULL;
  // Can't actually reclaim the index entry because that could involve
  // reindexing subsequent entries
  // entry.name = NULL;
  // entry.next_free = s_first_free_index;
  // s_first_free_index = index;
}

#if 0

namespace resource_manager {

typedef map<imstring, im_ptr<tbm> > loaded_resources_map;

loaded_resources_map s_loaded_resources;

extern im_ptr<tbm> fetch_tbm(imstring const &name);
extern void reclaim_loaded_data(void __far *data);

} // namespace resource_manager

im_ptr<tbm> resource_manager::fetch_tbm(imstring const &name) {
  logf_resource_manager("resource_manager::fetch_tbm\n");
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  loaded_resources_map::iterator i = s_loaded_resources.find(name);
  if (i == s_loaded_resources.end()) {
    logf_resource_manager("  i == s_loaded_resources.end()\n");
    segsize_t index = find_or_load_data(name);
    index_entry &entry = s_index[index];
    assert(entry.data);
    if (!entry.resource) {
      logf_resource_manager("  !entry.resource\n");
      reclaim_header __far *header =
          reinterpret_cast<reclaim_header __far *>(::_fmalloc(sizeof(reclaim_header) + sizeof(bitmap)));
      logf_resource_manager("  header = %" PRpF ", .name = %" PRsF ", .index = %u\n", header,
                            name.c_str(), (unsigned)index);
      header->name = name.c_str();
      header->index = index;
      entry.resource = header + 1;
      logf_resource_manager("  entry.resource = %" PRpF "\n", header);
      new (entry.resource) bitmap;
      logf_resource_manager("  converting data at %" PRpF " (%u bytes)\n", entry.data,
                            (unsigned)entry.size);
      tbm::to_bitmap(unpacker(entry.data, entry.size),
                     *reinterpret_cast<bitmap *>(entry.resource));
      logf_resource_manager("  conversion complete: %d x %d\n",
                            (int)(reinterpret_cast<bitmap *>(entry.resource))->width(),
                            (int)(reinterpret_cast<bitmap *>(entry.resource))->height());
      assert(entry.resource);
    }
    i = s_loaded_resources.insert(loaded_resources_map::value_type(
        name, im_ptr<bitmap>(reclaim_loaded_data, reinterpret_cast<bitmap *>(entry.resource))));
    assert(i != s_loaded_resources.end());
  }
  return i->ref;
}

void resource_manager::reclaim_loaded_data(void __far *data) {
  reclaim_header __far *header = (reinterpret_cast<reclaim_header __far *>(data)) - 1;
  assert(header->index < s_index.size());

  char const *name = header->name;
  assert(name);
  segsize_t index = header->index;
  index_entry &entry = s_index[index];
  logf_resource_manager(
      "reclaim_loaded_data %" PRpF ": header=%" PRpF ", entry.resource=%" PRpF ", .name=%" PRsF "\n", data,
      header, entry.resource, entry.name.c_str());
  assert(reinterpret_cast<void *>(entry.resource) == data);
  assert(entry.name == imstring(name));

  (reinterpret_cast<bitmap *>(header + 1))->~bitmap();

#if BUILD_DEBUG
  header->index = s_empty_index;
  header->name = NULL;
#endif

  ::ffree(header);
  ::ffree(reinterpret_cast<void __far *>(entry.data));

  entry.resource = NULL;
  entry.data = NULL;
  entry.name = NULL;

  entry.next_free = s_first_free_index;
  s_first_free_index = index;
}

#endif
