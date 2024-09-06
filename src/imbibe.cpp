#include "imbibe.h"

// #define RENDER_TEST

#include "inplace.h"

#include "application.h"
#include "imstring.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "timer.h"

#define logf_imbibe(...) disable_logf("IMBIBE: " __VA_ARGS__)

#if !BUILD_POSIX_SIM

int harderr_handler(unsigned deverror, unsigned errcode,
                    unsigned __far *devhdr) {
  (void)deverror;
  (void)errcode;
  (void)devhdr;
  // Fail the OS call that got us here and return the error to the caller,
  // instead of showing a prompt to the user -- we're actually checking call
  // results, and the screen will probabably be full of interactive app, so
  // best not to interrupt with questions the user can't answer anyway.
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
// Notes:	for origins 01h and 02h, the pointer may be positioned before
// the 	  start of the file; no error is returned in that case (except under
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
                              unsigned long __far *where);

#if BUILD_MSDOS_WATCOMC
#pragma aux asm_dos_lseek = "   mov     ah, 042h              "                \
                            "   int     21h                   "                \
                            "   jc      @error                "                \
                            "   mov     es:[di], ax           "                \
                            "   mov     es:[di + 2], dx       "                \
                            "   xor     ax, ax                "                \
                            "@error:                          " parm[bx]       \
    [cx dx][ax][es di] modify[ax bx cx dx] nomemory value[ax]
#else
// TODO
#endif

unsigned _dos_lseek(int handle, long offset, int whence,
                    unsigned long __far *where) {
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

#if BUILD_MSDOS && BUILD_DEBUG

#include <signal.h>

namespace aux_main {

static termel_t screen_temp[80 * 25];
void abort_handler(int sig) {
  (void)sig;
  ::_fmemcpy(screen_temp, MK_FP(0xB800, 0), sizeof screen_temp);
  failsafe_textmode();
  for (segsize_t i = 0; i < LENGTHOF(screen_temp); ++i) {
    screen_temp[i] =
        termel::with_attribute(screen_temp[i], color::white, color::black);
  }
  ::_fmemcpy(MK_FP(0xB800, 0), screen_temp, sizeof screen_temp);
}

} // namespace aux_main

#endif

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

#if !BUILD_POSIX_SIM
  // The DOS default is to show weird error prompts with questions like
  // "Abort, Retry, Ignore, Fail" when there are hardware errors like disk
  // read failures. We're not handling critical data where that prompt could
  // save them from disaster, so this kind of interruption in our UI is not
  // particularly justified.

  // This installs an error handler that fails the DOS function; if we want
  // to handle the error, that can be done in the normal call flow after
  // function return. In practice we will probably just bail, the user can
  // find better hardware to run our program on if it's that marginal (or
  // more like, next time don't eject the disk while we're loading).

  _harderr(harderr_handler);

#endif
  imstring::setup();
  timer::setup();
  resource_manager::setup();

#if BUILD_MSDOS && BUILD_DEBUG
  typedef void (*sig_handler_t)(int);
  sig_handler_t old_abort_handler = signal(SIGABRT, aux_main::abort_handler);
#endif
  application::setup();
  application::run_loop();
  application::teardown();
#if BUILD_MSDOS && BUILD_DEBUG
  signal(SIGABRT, old_abort_handler);
#endif

  resource_manager::teardown_exiting();
  timer::teardown();
  imstring::teardown_exiting();

  return 0;
}

#if BUILD_POSIX_SIM

namespace sim {

uint16_t dummy_screen[16384];

struct key_seq_entry {
  uint32_t ms;
  uint16_t key;
};

struct loop_seq_entry {
  uint32_t ms;
  uint32_t ms_per_loop;
  uint32_t ms_per_idle;
  uint32_t idle_per_loop;
};

key_seq_entry const key_seq[] = {
    {6500, key_code::escape},    {137, key_code::down},
    {272, key_code::down},       {440, key_code::up},
    {84, key_code::up},          {178, key_code::up},
    {2000, key_code::control_q}, {0, 0},
};

loop_seq_entry const loop_seq[] = {
    {1, 10, 1, 0},
    {0, 10, 1, 0},
};

uint32_t now_ms = 0;
key_seq_entry const *key_seq_p = key_seq;
loop_seq_entry const *loop_seq_p = loop_seq;

uint32_t pit_tick_counter = 0;
void pit_tick_counter_int_handler() { ++pit_tick_counter; }
void (*pit_int_handler)() = pit_tick_counter_int_handler;

void advance_time_to(uint32_t to_ms);
} // namespace sim

void sim::step_idle() { advance_time_to(now_ms + loop_seq_p->ms_per_idle); }

void sim::step_poll() {
  assert(loop_seq_p < loop_seq + LENGTHOF(loop_seq));
  advance_time_to(loop_seq_p->ms);
  while ((loop_seq_p->ms > 0) && (now_ms >= (loop_seq_p + 1)->ms)) {
    ++loop_seq_p;
  }
  advance_time_to(now_ms + loop_seq_p->ms_per_loop);
}

void sim::advance_time_to(uint32_t to_ms) {
  while (now_ms < to_ms) {
    if (pit_int_handler != pit_tick_counter_int_handler) {
      pit_int_handler();
    }
    ++now_ms;
  }
}

void sim::step_animate(uint32_t anim_ms) {
  (void)anim_ms;
  // assert desired anim_ms achieved
}

bool keyboard::key_event_available() {
  assert(sim::key_seq_p < (sim::key_seq + LENGTHOF(sim::key_seq)));
  return (sim::key_seq_p->ms > 0) && (sim::key_seq_p->ms <= sim::now_ms);
}

key_code_t keyboard::read_key_event() {
  assert(sim::key_seq_p < (sim::key_seq + LENGTHOF(sim::key_seq)));
  if (sim::key_seq_p->ms == 0) {
    assert(!"read_key_event no key available!");
    return 0;
  }
  if (sim::key_seq_p->ms > sim::now_ms) {
    logf_imbibe("read_key_event blocking\n");
    sim::advance_time_to(sim::key_seq_p->ms);
  }
  assert(sim::key_seq_p->ms <= sim::now_ms);
  return (sim::key_seq_p++)->key;
}

void _dos_setvect(int int_no, void (*int_handler)()) {
  (void)int_no;
  assert(int_no == 8);
  logf_imbibe("setvect 0x%02X, %p (overwrites %p)\n", int_no, int_handler,
              sim::pit_int_handler);
  sim::pit_int_handler = int_handler;
}

void (*_dos_getvect(int int_no))() {
  (void)int_no;
  assert(int_no == 8);
  logf_imbibe("getvect 0x%02X: %p\n", int_no, sim::pit_int_handler);
  return sim::pit_int_handler;
}

void _chain_intr(void (*int_handler)()) {
  assert(int_handler);
  logf_imbibe("chain_intr %p\n", int_handler);
  int_handler();
}

unsigned _dos_open(char const __far *path, unsigned mode, int __far *handle) {
  assert(handle);
  *handle = open(const_cast<char const *>(path), mode);
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

unsigned _dos_read(int handle, void __far *buf, unsigned count,
                   unsigned __far *bytes) {
  assert(handle >= 0);
  assert(buf);
  ssize_t amount = read(handle, const_cast<void *>(buf), count);
  if (amount < 0) {
    return 1;
  }
  *bytes = (unsigned)count;
  return 0;
}

unsigned _dos_lseek(int handle, long offset, int whence,
                    unsigned long __far *where) {
  ssize_t actual = lseek(handle, offset, whence);
  if (actual < 0) {
    *where = UINT32_MAX;
    return 1;
  }
  *where = actual;
  return 0;
}

void __far *_fmalloc(size_t size) {
  assert(size > 0);
  size_t *header_p =
      reinterpret_cast<size_t *>(::malloc(size + sizeof(size_t)));
  *header_p = size;
  return reinterpret_cast<void __far *>(header_p + 1);
}

void __far *_fexpand(void __far *p, size_t size) {
  assert(p);
  size_t *header_p = reinterpret_cast<size_t *>(const_cast<void *>(p)) - 1;
  assert(size <= *header_p);
  if (size > *header_p) {
    return NULL;
  }
  *header_p = size;
  return p;
}

void _ffree(void __far *p) {
  assert(p);
  size_t *header_p = reinterpret_cast<size_t *>(const_cast<void *>(p)) - 1;
  ::free(header_p);
}

int _fstrcmp(char const __far *x, char const __far *y) {
  return strcmp(const_cast<char const *>(x), const_cast<char const *>(y));
}

size_t _fstrlen(char const __far *s) {
  return strlen(const_cast<char const *>(s));
}

void _fmemcpy(void __far *dest, void const __far *src, size_t size) {
  memcpy(const_cast<void *>(dest), const_cast<void const *>(src), size);
}

#else

namespace aux_simulation {
uint32_t s_idle_count = 0;
}

void sim::step_idle() { ++aux_simulation::s_idle_count; }

void sim::step_poll() {
  logf_sim("('poll',{'t':%lu,'idle'=%lu})\n", (unsigned long)timer::now(),
           (unsigned long)aux_simulation::s_idle_count);
}

void sim::step_animate(uint32_t anim_ms) {
  logf_sim("('animate',{'t':%lu,'anim_ms'=%lu})\n", (unsigned long)timer::now(),
           (unsigned long)anim_ms);
}

#endif
