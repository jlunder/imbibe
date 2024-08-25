#include "imbibe.h"

// #define RENDER_TEST

#include "inplace.h"

#if defined(RENDER_TEST)
#include "render_test_task.h"
#else
#include "main_task.h"
#endif

#include "keyboard.h"
#include "imstring.h"
#include "resource_manager.h"
#include "timer.h"


#define logf_imbibe(...) disable_logf("IMBIBE: " __VA_ARGS__)


#if !defined(SIMULATE)


int harderr_handler(unsigned deverror, unsigned errcode,
    unsigned __far * devhdr) {
  (void)deverror;
  (void)errcode;
  (void)devhdr;
  _hardresume(_HARDERR_FAIL);
  return 0;
}


// --------D-2142-------------------------------
// INT 21 - DOS 2+ - "LSEEK" - SET CURRENT FILE POSITION
// 	AH = 42h
// 	AL = whence of move
// 	    00h start of file
// 	    01h current file position
// 	    02h end of file
// 	BX = file handle
// 	CX:DX = (signed) offset from whence of new file position
// Return: CF clear if successful
// 	    DX:AX = new file position in bytes from start of file
// 	CF set on error
// 	    AX = error code (01h,06h) (see #01680 at AH=59h/BX=0000h)
// Notes:	for origins 01h and 02h, the pointer may be positioned before the
// 	  start of the file; no error is returned in that case (except under
// 	  Windows NT), but subsequent attempts at I/O will produce errors
// 	if the new position is beyond the current end of file, the file will
// 	  be extended by the next write (see AH=40h); for FAT32 drives, the
// 	  file must have been opened with AX=6C00h with the "extended size"
// 	  flag in order to expand the file beyond 2GB
// BUG:	using this method to grow a file from zero bytes to a very large size
// 	  can corrupt the FAT in some versions of DOS; the file should first
// 	  be grown from zero to one byte and then to the desired large size
// SeeAlso: AH=24h,INT 2F/AX=1228h

extern unsigned asm_dos_lseek(int handle, long offset, int whence,
  unsigned long __far * where);

#pragma aux asm_dos_lseek = \
  "   mov     ah, 042h              " \
  "   int     21h                   " \
  "   jc      @error                " \
  "   mov     es:[di], ax           " \
  "   mov     es:[di + 2], dx       " \
  "   xor     ax, ax                " \
  "@error:                          " \
  parm [bx] [cx dx] [ax] [es di] \
  modify [ax bx cx dx] nomemory \
  value [ax]

unsigned _dos_lseek(int handle, long offset, int whence,
    unsigned long __far * where) {
  assert(whence == 0 || whence == 0 || whence == 2);
  assert(handle >= 0);
  assert(where);
  *where = INT32_MIN;
  unsigned result = asm_dos_lseek(handle, offset, whence, where);
  if ((result == 0) && (*where == INT32_MIN)) {
    result = 1;
  }
  return result;
}


#endif


int main(int argc, char * argv[]) {
  (void)argc;
  (void)argv;

#if !defined(SIMULATE)
  _harderr(harderr_handler);
#endif

  imstring::setup();
  timer::setup();
  resource_manager::setup();
#if defined(RENDER_TEST)
  render_task_instance.setup();
  render_task_instance->start();
  render_task_instance->run_loop();
  render_task_instance.teardown();
#else
  main_task_instance.setup();
  main_task_instance->start();
  main_task_instance->run_loop();
  main_task_instance.teardown();
#endif

#ifdef NDEBUG
  resource_manager::teardown_exiting();
#else
  resource_manager::teardown();
#endif
  timer::teardown();
#ifdef NDEBUG
  imstring::teardown_exiting();
#else
  imstring::teardown();
#endif

  return 0;
}


#if defined(SIMULATE)


uint16_t dummy_screen[16384];

struct key_seq_entry {
  uint32_t steps;
  uint32_t ms;
  uint16_t key;
};

key_seq_entry const sim_seq[] = {
  {1000, 20000, 0x001B},
  {  100, 2000,      0},
};
uint32_t sim_seq_i = 0;
uint32_t sim_seq_i_step = 0;
uint32_t sim_seq_i_ms = 0;

uint32_t pit_tick_counter = 0;
void pit_tick_counter_int_handler() { ++pit_tick_counter; }
void (*pit_int_handler)() = pit_tick_counter_int_handler;


void step_simulator() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  ++sim_seq_i_step;
  if ((sim_seq_i_step >= sim_seq[sim_seq_i].key)
      && (sim_seq[sim_seq_i].key == 0)) {
    assert(sim_seq_i + 1 < LENGTHOF(sim_seq));
    ++sim_seq_i;
    sim_seq_i_step = 0;
    sim_seq_i_ms = 0;
  }
  uint32_t target_ms = (uint64_t)sim_seq_i_step * sim_seq[sim_seq_i].ms
    / sim_seq[sim_seq_i].steps;
  while (sim_seq_i_ms < target_ms) {
    if (pit_int_handler != pit_tick_counter_int_handler) {
      pit_int_handler();
    }
    ++sim_seq_i_ms;
  }
  logf_imbibe("sim_step [%u:%u, %u ms]\n",
    sim_seq_i, sim_seq_i_step, sim_seq_i_ms);
}


bool keyboard::key_event_available() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  bool avail = (sim_seq[sim_seq_i].key != 0)
    && (sim_seq_i_step >= sim_seq[sim_seq_i].steps);
  logf_imbibe("key_avail: %d\n", (int)avail);
  return avail;
}


key_code_t keyboard::read_key_event() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  assert(key_event_available());
  if (sim_seq_i >= LENGTHOF(sim_seq)) {
    return 0;
  }
  if (sim_seq_i_step >= sim_seq[sim_seq_i].steps) {
    assert(sim_seq_i_ms >= sim_seq[sim_seq_i].ms);
    uint16_t key = sim_seq[sim_seq_i].key;
    assert(sim_seq_i + 1 < LENGTHOF(sim_seq));
    ++sim_seq_i;
    sim_seq_i_step = 0;
    sim_seq_i_ms = 0;
    logf_imbibe("read_key: 0x%04X\n", key);
    return key;
  } else {
    assert(!"read_key no key available!");
    return 0;
  }
}


void _dos_setvect(int int_no, void (*int_handler)()) {
  (void)int_no;
  assert(int_no == 8);
  logf_imbibe("setvect 0x%02X, %p (overwrites %p)\n", int_no,
    int_handler, pit_int_handler);
  pit_int_handler = int_handler;
}


void (*_dos_getvect(int int_no))() {
  (void)int_no;
  assert(int_no == 8);
  logf_imbibe("getvect 0x%02X: %p\n", int_no, pit_int_handler);
  return pit_int_handler;
}


void _chain_intr(void (*int_handler)()) {
  assert(int_handler);
  logf_imbibe("chain_intr %p\n", int_handler);
  int_handler();
}


unsigned _dos_open(const char * path, unsigned mode, int * handle) {
  assert(handle);
  *handle = open(path, mode);
  if (*handle < 0) {
    return 1;
  }
  return 0;
}


unsigned _dos_close(int handle) {
  assert(handle >= 0);
  close(handle);
  return 0;
}


unsigned _dos_read(int handle, void * buf, unsigned count,
    unsigned * bytes) {
  assert(handle >= 0);
  assert(buf);
  ssize_t amount = read(handle, buf, count);
  if (amount < 0) {
    return 1;
  }
  *bytes = (unsigned)count;
  return 0;
}


unsigned _dos_lseek(int handle, long offset, int whence,
    unsigned long __far * where) {
  ssize_t actual = lseek(handle, offset, whence);
  if (actual < 0) {
    *where = UINT32_MAX;
    return 1;
  }
  *where = actual;
  return 0;
}


unsigned _dos_allocmem(unsigned size, unsigned * seg) {
  (void)size;
  (void)seg;
  assert(!"TODO");
  return 1;
}


unsigned _dos_freemem(unsigned seg) {
  (void)seg;
  assert(!"TODO");
  return 1;
}


#endif


