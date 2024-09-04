#include "imbibe.h"

#include "timer.h"

#define PIT_INTERRUPT 8
#define PIT_FREQUENCY 0x1234DD
#define TIMER_FREQUENCY 1000
#define PIT_BIOS_PERIOD 0x10000
#define PIT_CONTROL 0x43
#define PIT_DATA 0x40
#define PIT_PERIOD 0x34

namespace aux_timer {
void(__interrupt *s_pit_bios_handler)();
uint32_t s_pit_tick_inc;
uint32_t s_pit_tick_bios_inc;
uint32_t s_pit_tick_count;
volatile uint32_t s_timer_count;

void start_timer();
void stop_timer();
void(__interrupt pit_handler_bios_slower)();
void(__interrupt pit_handler_bios_faster)();
}; // namespace aux_timer

extern void asm_timer_enter_crit();
extern void asm_timer_leave_crit();
extern void asm_timer_ackint();
extern void asm_timer_outb(uint16_t port, uint8_t value);
extern void asm_timer_outwb(uint16_t port, uint16_t value);
extern uint16_t asm_timer_inb(uint16_t port);
extern uint16_t asm_timer_inwb(uint16_t port);

#if !BUILD_POSIX_SIM

#pragma aux asm_timer_enter_crit = "pushf"                                     \
                                   "cli" modify exact[] nomemory;

#pragma aux asm_timer_leave_crit = "popf" modify exact[] nomemory;

#pragma aux asm_timer_ackint = "mov al, 20h"                                   \
                               "out 20h, al" modify exact[al] nomemory;

#pragma aux asm_timer_outb = "out dx, al" parm[dx][al] modify exact[] nomemory;

#pragma aux asm_timer_outwb =                                                  \
    "out dx, al"                                                               \
    "mov al, ah"                                                               \
    "out dx, al" parm[dx][ax] modify exact[ax] nomemory;

#pragma aux asm_timer_inb =                                                    \
    "in al, dx" parm[dx] value[al] modify exact[al] nomemory;

#pragma aux asm_timer_inwb =                                                   \
    "in al, dx"                                                                \
    "mov ah, al"                                                               \
    "in al, dx"                                                                \
    "xchg al, ah" parm[dx] value[ax] modify exact[ax] nomemory;

#else

inline void asm_timer_enter_crit() {}
inline void asm_timer_leave_crit() {}
inline void asm_timer_ackint() {}

inline void asm_timer_outb(uint16_t port, uint8_t value) {
  (void)port;
  (void)value;
}

inline void asm_timer_outwb(uint16_t port, uint16_t value) {
  (void)port;
  (void)value;
}

inline uint16_t asm_timer_inb(uint16_t port) {
  (void)port;
  return 0;
}

inline uint16_t asm_timer_inwb(uint16_t port) {
  (void)port;
  return 0;
}

#endif

uint32_t timer::now() {
  uint32_t result;
  asm_timer_enter_crit();
  result = aux_timer::s_timer_count;
  asm_timer_leave_crit();
  return result;
}

void timer::setup() { aux_timer::start_timer(); }

void timer::teardown() { aux_timer::stop_timer(); }

void aux_timer::start_timer() {
  s_pit_tick_inc = PIT_FREQUENCY / TIMER_FREQUENCY;
  s_pit_tick_count = 0;
  s_timer_count = 0;

  asm_timer_enter_crit();
  //  asm_timer_outb(PIT_CONTROL, PIT_PERIOD);
  //  s_pit_tick_bios_inc = INWB(PIT_DATA);
  s_pit_tick_bios_inc = 0;
  if (s_pit_tick_bios_inc == 0) {
    s_pit_tick_bios_inc = 0x10000;
  }
  s_pit_bios_handler = _dos_getvect(PIT_INTERRUPT);
  if (s_pit_tick_inc < s_pit_tick_bios_inc) {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_slower);
    asm_timer_outb(PIT_CONTROL, PIT_PERIOD);
    asm_timer_outwb(PIT_DATA, (uint16_t)s_pit_tick_inc);
  } else {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_faster);
  }
  asm_timer_leave_crit();
}

void aux_timer::stop_timer() {
  asm_timer_enter_crit();
  if (s_pit_tick_inc < s_pit_tick_bios_inc) {
    asm_timer_outb(PIT_CONTROL, PIT_PERIOD);
    asm_timer_outwb(PIT_DATA, (uint16_t)s_pit_tick_bios_inc);
  }
  _dos_setvect(PIT_INTERRUPT, s_pit_bios_handler);
  asm_timer_leave_crit();
}

void(__interrupt aux_timer::pit_handler_bios_slower)() {
  ++s_timer_count;
  s_pit_tick_count += s_pit_tick_inc;
  if (s_pit_tick_count >= PIT_BIOS_PERIOD) {
    s_pit_tick_count -= PIT_BIOS_PERIOD;
    _chain_intr(s_pit_bios_handler);
  } else
    asm_timer_ackint();
}

void(__interrupt aux_timer::pit_handler_bios_faster)() {
  s_pit_tick_count += PIT_BIOS_PERIOD;
  if (s_pit_tick_count >= s_pit_tick_inc) {
    ++s_timer_count;
  }
  s_pit_tick_count -= s_pit_tick_inc;
  _chain_intr(s_pit_bios_handler);
}
