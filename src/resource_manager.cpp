#include "imbibe.h"

#include "resource_manager.h"

#include "builtin_data.h"
#include "map.h"
#include "tbm.h"
#include "unpacker.h"

#define logf_resource_manager(...)                                             \
  disable_logf("RESOURCE_MANAGER: " __VA_ARGS__)

namespace resource_manager {

static const uint16_t s_hash_capacity = 128;
static const uint16_t s_index_initial_capacity = 8;
static const uint16_t s_empty_index = UINT16_MAX;

struct hash_entry {
  uint16_t hash;
  uint16_t index;
};

struct index_entry {
  imstring name;
  union {
    uint16_t size;
    uint16_t next_free;
  };
  void const __far *data;
  void __far *resource;
};

struct reclaim_header {
  char const __far *name;
  uint16_t index;
};

typedef map<imstring, im_ptr<bitmap> > loaded_resources_map;

uint16_t s_hash_mask;
vector<hash_entry> s_name_hash;
vector<index_entry> s_index;
uint16_t s_first_free_index;

loaded_resources_map s_loaded_resources;

uint16_t fletcher16_str(char const __far *s);
uint16_t find_or_load_data(imstring const &name);
void load_data_at_hash(imstring const &name, uint16_t hash, uint16_t hint);
void reclaim_loaded_data(void __far *data);

} // namespace resource_manager

#if 0 // !defined(SIMULATE)

extern uint16_t asm_fletcher16_str(char const __far * s);
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

#endif

uint16_t resource_manager::fletcher16_str(char const __far *s) {
#if 1 // !defined(NDEBUG) || defined(SIMULATE)
  uint16_t s1 = 0;
  uint16_t s2 = 0;
  for (char const *p = s; *p; ++p) {
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
  // #if defined(SIMULATE)
  //   return (s2 << 8) | s1;
  // #else
  //   uint16_t result = asm_fletcher16_str(s);
  //   assert(result == ((s2 << 8) | s1));
  //   return result;
  // #endif
}

void resource_manager::setup() {
  hash_entry blank;
  blank.hash = s_empty_index;
  blank.index = s_empty_index;
  assert(s_name_hash.empty());
  s_name_hash.assign(s_hash_capacity, blank);
  s_hash_mask = s_hash_capacity - 1;
  s_first_free_index = s_empty_index;
  assert(s_index.empty());
  assert(s_loaded_resources.empty());
}

void resource_manager::teardown() {
  s_loaded_resources.clear();
  s_name_hash.clear();
  s_index.clear();
  s_first_free_index = s_empty_index;
}

void resource_manager::teardown_exiting() {
  // it's okay to leak everything here
}

bool resource_manager::begin_fetch_tbm(imstring const &name) {
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  (void)name;
  return true;
}

bool resource_manager::fetch_ready(imstring const &name) {
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  (void)name;
  return true;
}

im_ptr<bitmap> resource_manager::fetch_bitmap(imstring const &name) {
  logf_resource_manager("resource_manager::fetch_tbm\n");
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  loaded_resources_map::iterator i = s_loaded_resources.find(name);
  if (i == s_loaded_resources.end()) {
    logf_resource_manager("  i == s_loaded_resources.end()\n");
    uint16_t index = find_or_load_data(name);
    index_entry &entry = s_index[index];
    assert(entry.data);
    if (!entry.resource) {
      logf_resource_manager("  !entry.resource\n");
      reclaim_header *header =
          (reclaim_header *)::malloc(sizeof(reclaim_header) + sizeof(bitmap));
      logf_resource_manager("  header = %p, .name = %s, .index = %u\n", header,
                            name.c_str(), (unsigned)index);
      header->name = name.c_str();
      header->index = index;
      entry.resource = header + 1;
      logf_resource_manager("  entry.resource = %p\n", header);
      new (entry.resource) bitmap;
      logf_resource_manager("  converting data at %p (%u bytes)\n", entry.data,
                            (unsigned)entry.size);
      tbm::to_bitmap(unpacker(entry.data, entry.size),
                     *(bitmap *)entry.resource);
      logf_resource_manager("  conversion complete: %d x %d\n",
                            (int)((bitmap *)entry.resource)->width(),
                            (int)((bitmap *)entry.resource)->height());
      assert(entry.resource);
    }
    i = s_loaded_resources.insert(loaded_resources_map::value_type(
        name, im_ptr<bitmap>(reclaim_loaded_data, (bitmap *)entry.resource)));
    assert(i != s_loaded_resources.end());
  }
  return i->ref;
}

unpacker resource_manager::fetch_tbm(imstring const &name) {
  logf_resource_manager("resource_manager::fetch_tbm\n");
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);

  logf_resource_manager("  i == s_loaded_resources.end()\n");
  uint16_t index = find_or_load_data(name);
  index_entry &entry = s_index[index];
  return unpacker(entry.data, entry.size);
}

void resource_manager::flush(imstring const &name) {
  assert(name.length() <= RESOURCE_NAME_LEN_MAX);
  (void)name;
}

uint16_t resource_manager::find_or_load_data(imstring const &name) {
  assert(s_hash_mask == (s_name_hash.size() - 1));
  assert(s_hash_mask == 15 || s_hash_mask == 31 || s_hash_mask == 63 ||
         s_hash_mask == 127 || s_hash_mask == 255 || s_hash_mask == 511);
  logf_resource_manager("find_or_load_data %s\n", name.c_str());
  uint16_t name_hash = fletcher16_str(name.c_str());
  logf_resource_manager("  fletcher16_str = %04x\n", (unsigned)name_hash);
  for (uint16_t i = name_hash & s_hash_mask;; i = (i + 1) & s_hash_mask) {
    // don't search too far
    assert(((i - name_hash) & s_hash_mask) < 10);
    if (s_name_hash[i].index == s_empty_index) {
      // definitively not found
      logf_resource_manager("  not found at %d\n", (int)i);
      load_data_at_hash(name, name_hash, i);
      assert(s_name_hash[i].index != s_empty_index);
      return s_name_hash[i].index;
    } else if (s_name_hash[i].hash == name_hash) {
      // maybe our thing?
      uint16_t index = s_name_hash[i].index;
      if (s_index[index].name == name) {
        // that's us!
        return index;
      }
      // otherwise fall through
    }
    // some other thing, just continue...
  }
}

void resource_manager::load_data_at_hash(imstring const &name, uint16_t hash,
                                         uint16_t hint) {
  assert(hash == fletcher16_str(name.c_str()));
  assert(s_name_hash[hint].index == s_empty_index);

  if (s_first_free_index == s_empty_index) {
    // out of index entries, expand the free list
    assert(s_index.capacity() < s_hash_capacity / 2);
    uint16_t target = min<uint16_t>(
        s_hash_capacity / 2,
        max<uint16_t>(s_index_initial_capacity, s_index.capacity() * 2));
    assert(target > s_index.size());
    s_index.reserve(target);
    s_first_free_index = s_index.size();
    while (s_index.size() < (target - 1)) {
      index_entry e;
      e.next_free = s_index.size() + 1;
      s_index.push_back(e);
    }
    index_entry e;
    e.next_free = s_empty_index;
    s_index.push_back(e);
  }
  assert(s_first_free_index != s_empty_index);
  uint16_t index = s_first_free_index;
  s_name_hash[hint].hash = hash;
  s_name_hash[hint].index = index;
  s_first_free_index = s_index[s_first_free_index].next_free;

  index_entry &entry = s_index[index];
  entry.name = name;
  entry.data = NULL;
  entry.resource = NULL;
  entry.size = 0;

  int handle = -1;
  char const *op = "open";
  (void)op;
  char path_buf[RESOURCE_NAME_LEN_MAX + 10];
  snprintf(path_buf, sizeof path_buf, "testdata/%s", name.c_str());
  unsigned err = _dos_open(path_buf, O_RDONLY, &handle);
  if (err != 0)
    goto fail_return;

  logf_resource_manager("  opened '%s' as %d (result %u)\n", path_buf, handle,
                        err);
  {
    unsigned long size = UINT32_MAX;
    op = "seek-to-end";
    err = _dos_lseek(handle, 0, SEEK_END, &size);
    logf_resource_manager("  seek to end at %lu (result %u)\n", size, err);
    if (err != 0)
      goto fail_return;
    op = "allocation (file too big)";
    if (size > (INT16_MAX / 2))
      goto fail_return;
    entry.size = (uint16_t)size;
    entry.data = ::malloc(entry.size);
    assert(entry.data != NULL);
    if (entry.data == NULL)
      goto fail_return;
  }

  {
    unsigned long dummy;
    op = "seek-to-end";
    err = _dos_lseek(handle, 0, SEEK_SET, &dummy);
    if (err != 0)
      goto fail_return;
  }

  {
    unsigned actual = 0;
    op = "read";
    err = _dos_read(handle, (void *)entry.data, entry.size, &actual);
    if (err != 0)
      goto fail_return;
    if (actual != entry.size) {
      op = "read (wrong size reported)";
      goto fail_return;
    }
  }

  {
    op = "close";
    err = _dos_close(handle);
    if (err != 0)
      goto fail_return;
  }
  return;

fail_return:
  if (entry.data != NULL) {
    ::free(((reclaim_header *)entry.data) - 1);
    _dos_close(handle);
  }
  logf_resource_manager("error during %s, file '%s'\n", op, name.c_str());
}

void resource_manager::reclaim_loaded_data(void __far *data) {
  reclaim_header *header = ((reclaim_header __far *)data) - 1;
  assert(header->index < s_index.size());

  char const *name = header->name;
  assert(name);
  uint16_t index = header->index;
  index_entry &entry = s_index[index];
  logf_resource_manager(
      "reclaim_loaded_data %p: header=%p, entry.resource=%p, .name=%s\n", data,
      header, entry.resource, entry.name.c_str());
  assert((void *)entry.resource == data);
  assert(entry.name == imstring(name));

  ((bitmap *)(header + 1))->~bitmap();

#ifndef NDEBUG
  header->index = s_empty_index;
  header->name = NULL;
#endif

  ::free(header);
  ::free((void *)entry.data);

  entry.resource = NULL;
  entry.data = NULL;
  entry.name = NULL;

  entry.next_free = s_first_free_index;
  s_first_free_index = index;
}
