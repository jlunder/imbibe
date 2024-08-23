#include "imbibe.h"

#include "timer.h"


extern void asm_timer_enter_crit();
extern void asm_timer_leave_crit();
extern void asm_timer_ackint();
extern void asm_timer_outb(uint16_t port, uint8_t value);
extern void asm_timer_outwb(uint16_t port, uint16_t value);
extern uint16_t asm_timer_inb(uint16_t port);
extern uint16_t asm_timer_inwb(uint16_t port);

#if !defined(SIMULATE)

#pragma aux asm_timer_enter_crit=\
  "pushf"\
  "cli"\
  modify exact [] nomemory;

#pragma aux asm_timer_leave_crit=\
  "popf"\
  modify exact [] nomemory;

#pragma aux asm_timer_ackint=\
  "mov al, 20h"\
  "out 20h, al"\
  modify exact [al] nomemory;

#pragma aux asm_timer_outb=\
  "out dx, al"\
  parm [dx] [al]\
  modify exact [] nomemory;

#pragma aux asm_timer_outwb=\
  "out dx, al"\
  "mov al, ah"\
  "out dx, al"\
  parm [dx] [ax]\
  modify exact [ax] nomemory;

#pragma aux asm_timer_inb=\
  "in al, dx"\
  parm [dx]\
  value [al]\
  modify exact [al] nomemory;

#pragma aux asm_timer_inwb=\
  "in al, dx"\
  "mov ah, al"\
  "in al, dx"\
  "xchg al, ah"\
  parm [dx]\
  value [ax]\
  modify exact [ax] nomemory;

#else

inline void asm_timer_enter_crit() { }
inline void asm_timer_leave_crit() { }
inline void asm_timer_ackint() { }

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


#define PIT_INTERRUPT 8
#define PIT_FREQUENCY 0x1234DD
#define TIMER_FREQUENCY 1000
#define PIT_BIOS_PERIOD 0x10000
#define PIT_CONTROL 0x43
#define PIT_DATA 0x40
#define PIT_PERIOD 0x34


class hw_timer {
public:
  static void start_timer();
  static void stop_timer();
  static void (__interrupt pit_handler_bios_slower)();
  static void (__interrupt pit_handler_bios_faster)();
  static void (__interrupt * pit_bios_handler)();
  static uint32_t pit_tick_inc;
  static uint32_t pit_tick_bios_inc;
  static uint32_t pit_tick_count;
  static volatile uint32_t timer_count;
};


void (__interrupt * hw_timer::pit_bios_handler)();
uint32_t hw_timer::pit_tick_inc;
uint32_t hw_timer::pit_tick_bios_inc;
uint32_t hw_timer::pit_tick_count;
volatile uint32_t hw_timer::timer_count;


uint32_t timer::now() {
  uint32_t result;
  asm_timer_enter_crit();
  result = hw_timer::timer_count;
  asm_timer_leave_crit();
  return result;
}


void timer::setup() {
  hw_timer::start_timer();
}


void timer::teardown() {
  hw_timer::stop_timer();
}


void hw_timer::start_timer() {
  pit_tick_inc = PIT_FREQUENCY / TIMER_FREQUENCY;
  pit_tick_count = 0;
  timer_count = 0;

  asm_timer_enter_crit();
//  asm_timer_outb(PIT_CONTROL, PIT_PERIOD);
//  pit_tick_bios_inc = INWB(PIT_DATA);
  pit_tick_bios_inc = 0;
  if(pit_tick_bios_inc == 0) {
    pit_tick_bios_inc = 0x10000;
  }
  pit_bios_handler = _dos_getvect(PIT_INTERRUPT);
  if(pit_tick_inc < pit_tick_bios_inc) {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_slower);
    asm_timer_outb(PIT_CONTROL, PIT_PERIOD);
    asm_timer_outwb(PIT_DATA, (uint16_t)pit_tick_inc);
  } else {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_faster);
  }
  asm_timer_leave_crit();
}


void hw_timer::stop_timer() {
  asm_timer_enter_crit();
  if(pit_tick_inc < pit_tick_bios_inc) {
    asm_timer_outb(PIT_CONTROL, PIT_PERIOD);
    asm_timer_outwb(PIT_DATA, (uint16_t)pit_tick_bios_inc);
  }
  _dos_setvect(PIT_INTERRUPT, pit_bios_handler);
  asm_timer_leave_crit();
}


void (__interrupt hw_timer::pit_handler_bios_slower)() {
  ++timer_count;
  pit_tick_count += pit_tick_inc;
  if(pit_tick_count >= PIT_BIOS_PERIOD) {
    pit_tick_count -= PIT_BIOS_PERIOD;
    _chain_intr(pit_bios_handler);
  }
  else asm_timer_ackint();
}


void (__interrupt hw_timer::pit_handler_bios_faster)() {
  pit_tick_count += PIT_BIOS_PERIOD;
  if(pit_tick_count >= pit_tick_inc) {
    ++timer_count;
  }
  pit_tick_count -= pit_tick_inc;
  _chain_intr(pit_bios_handler);
}


