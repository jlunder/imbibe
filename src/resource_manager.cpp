#include "imbibe.h"

#include "resource_manager.h"

#include "builtin_data.h"
#include "fletcher16.h"
#include "inplace.h"
#include "map.h"
#include "tbm.h"
#include "unpacker.h"

#define logf_resource_manager(...) enable_logf("RESOURCE_MANAGER: " __VA_ARGS__)

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

// 64 bytes
struct archive_header_t {
  // 'TYAR0\x1A\x00\x00\xD2\x6E\xD5\xCD'
  uint8_t header_magic[12];
  // CRC of the whole shebang from +16 to end
  uint32_t archive_check_value;

  // CRC of this header starting after this value
  uint32_t header_check_value;

  uint32_t data_offset;
  uint32_t data_size;

  // toc_entry_count * 32 == toc_size
  uint16_t toc_entry_count;
  // '66'
  uint8_t pad_0[2];
  // Relative to the beginning of this header
  uint32_t toc_offset;
  // Size in bytes of the TOC entries
  uint32_t toc_size;
  // CRC of full TOC
  uint32_t toc_check_value;

  // Should always be a power of 2
  uint16_t hash_bin_count;
  // How far we might have to search for any given hash
  uint16_t hash_neighbor_count;
  // Relative to this header
  uint32_t hash_bins_offset;
  // Size in bytes of the header and entries
  uint32_t hash_bins_size;
  // CRC of hash table including header and entries
  uint32_t hash_bins_check_value;

  uint8_t pad_1[12]; // '666666666666'
};

// 4 bytes
struct hash_entry_fletcher16_t {
  uint16_t name_hash; // Fletcher-16, 0 seed
  uint16_t toc_index; // 0xFFFF for an unoccupied entry
};

// 16 bytes
struct toc_entry_t {
  uint16_t name_offset; // From TOC begin
  uint16_t name_size;   // Includes null terminator, which must be present and 0
  uint32_t file_offset; // From data area
  uint32_t file_size;   // Only the file, there may be padding between files
  uint32_t file_check_value; // CRC of file data without padding
};

} // namespace tyar

namespace resource_manager {

static unsigned archive_seg = 0;
static uint32_t archive_size = 0;
static tyar::toc_entry_t __far *archive_toc;
static uint16_t archive_toc_entry_count;
static uint32_t archive_toc_offset;
static tyar::hash_entry_fletcher16_t __far *archive_hash;
static uint16_t archive_hash_bin_count;
static uint16_t archive_hash_neighbor_count;
static uint32_t archive_data_offset;
static uint32_t archive_data_size;

bool validate_archive();

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
  archive_seg = 0;

  unsigned long size = UINT32_MAX;
  err = _dos_open(path, O_RDONLY, &handle); // TODO -- near/far problem?
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
    unsigned page_count = (unsigned)((archive_size + 15) / 16);
    if (archive_size / 16 > UINT16_MAX) {
      err = -1;
    } else {
      err = _dos_allocmem(page_count, &archive_seg);
      logf_resource_manager("allocated %u pages into %04X\n", page_count,
                            archive_seg);
    }
  }
  if (err == 0) {
#if BUILD_DEBUG
    op = "reading archive";
#endif
    uint32_t read_total = 0;
    while ((err == 0) && (read_total < archive_size)) {
      unsigned read_actual = 0;
      void __far *dest = MK_FP_O32(archive_seg, read_total);
      uint16_t size =
          (uint16_t)min<uint32_t>(archive_size - read_total, 32768u);
      logf_resource_manager("reading %u bytes into %" PRpF "\n", size, dest);
      err = _dos_read(handle, dest, size, &read_actual);
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
    abortf("unable to read archive '%" PRsF "'\n", path);
#endif
  }
  if (err == 0) {
    if (!validate_archive()) {
      _dos_freemem(archive_seg);
      archive_seg = 0;
      abortf("archive '%" PRsF "' not valid\n", path);
    }
  }
}

bool resource_manager::validate_archive() {
  assert(archive_seg);

  if (archive_size < sizeof(tyar::archive_header_t)) {
    assert(!"archive: smaller than TYAR header");
    return false;
  }

  tyar::archive_header_t __far *archive_header =
      reinterpret_cast<tyar::archive_header_t __far *>(MK_FP(archive_seg, 0));
  if (_fmemcmp(archive_header->header_magic, tyar::header_magic_v0,
               sizeof tyar::header_magic_v0) != 0) {
    assert(!"archive: bad header magic");
    return false;
  }
  if ((archive_size < archive_header->toc_offset) ||
      (archive_header->toc_offset - archive_size < archive_header->toc_size)) {
    assert(!"archive: TOC not within file");
    return false;
  }
  if ((archive_header->toc_size / sizeof(tyar::toc_entry_t) <=
       archive_header->toc_entry_count)) {
    assert(!"archive: TOC not big enough for entries");
    return false;
  }
  if ((archive_size < archive_header->hash_bins_offset) ||
      (archive_header->hash_bins_offset - archive_size <
       archive_header->hash_bins_size)) {
    assert(!"archive: hash not within file");
    return false;
  }
  if ((archive_header->hash_bins_size / sizeof(tyar::hash_entry_fletcher16_t) !=
       archive_header->hash_bin_count)) {
    assert(!"archive: hash size doesn't match bin count");
    return false;
  }
  if ((archive_size < archive_header->data_offset) ||
      (archive_header->toc_offset - archive_size < archive_header->toc_size)) {
    assert(!"archive: data area not within file");
    return false;
  }

  archive_toc_entry_count = archive_header->toc_entry_count;
  archive_toc_offset = archive_header->toc_offset;
  archive_toc = reinterpret_cast<tyar::toc_entry_t __far *>(
      MK_FP_O32(archive_seg, archive_header->toc_offset));
  archive_hash_bin_count = archive_header->hash_bin_count;
  archive_hash_neighbor_count = archive_header->hash_neighbor_count;
  archive_hash = reinterpret_cast<tyar::hash_entry_fletcher16_t __far *>(
      MK_FP_O32(archive_seg, archive_header->hash_bins_offset));
  archive_data_offset = archive_header->data_offset;
  archive_data_size = archive_header->data_size;

  return true;
}

void resource_manager::teardown() {
  if (archive_seg) {
    _dos_freemem(archive_seg);
    archive_seg = 0;
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
  _dos_freemem(archive_seg);
#endif
}

segsize_t resource_manager::fetch_data(imstring const &name,
                                       immutable *out_data) {
  uint16_t hash = fletcher16_str(name.c_str());
  // Swap high and low bytes for reasons
  uint16_t bin_mask = (archive_hash_bin_count - 1);
  uint16_t h = ((hash << 8) | (hash >> 8));
  for (uint16_t i = 0; i < archive_hash_neighbor_count; ++i) {
    uint16_t bin = (h + i) & bin_mask;
    if ((archive_hash[bin].name_hash == hash) &&
        (archive_hash[bin].toc_index != 0xFFFF)) {
      assert(archive_hash[bin].toc_index < archive_toc_entry_count);
      tyar::toc_entry_t const __far *entry =
          &archive_toc[archive_hash[bin].toc_index];
      char const __far *toc_name = reinterpret_cast<char const __far *>(
          MK_FP_O32(archive_seg, archive_toc_offset + entry->name_offset));
      assert((entry + 1)->name_offset > entry->name_offset);
      assert((entry + 1)->name_offset - entry->name_offset > entry->name_size);
      if (_fstrcmp(name.c_str(), toc_name) == 0) {
        assert(entry->file_offset < archive_data_size);
        assert(archive_data_size - entry->file_offset > entry->file_size);
        assert(entry->file_size <= SEGSIZE_INVALID);
        *out_data = immutable(
            immutable::prealloc,
            MK_FP_O32(archive_seg, archive_data_offset + entry->file_offset));
        return (segsize_t)entry->file_size;
      }
    }
  }
  return SEGSIZE_INVALID;
}

