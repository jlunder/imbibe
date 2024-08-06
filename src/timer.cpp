#include "imbibe.h"

#include "timer.h"


extern void aux_timer__enter_crit();
extern void aux_timer__leave_crit();
extern void aux_timer__ackint();
extern void aux_timer__outb(uint16_t port, uint8_t value);
extern void aux_timer__outwb(uint16_t port, uint16_t value);
extern uint16_t aux_timer__inb(uint16_t port);
extern uint16_t aux_timer__inwb(uint16_t port);

#if !defined(SIMULATE)

#pragma aux aux_timer__enter_crit=\
  "pushf"\
  "cli"\
  modify exact [] nomemory;

#pragma aux aux_timer__leave_crit=\
  "popf"\
  modify exact [] nomemory;

#pragma aux aux_timer__ackint=\
  "mov al, 20h"\
  "out 20h, al"\
  modify exact [al] nomemory;

#pragma aux aux_timer__outb=\
  "out dx, al"\
  parm [dx] [al]\
  modify exact [] nomemory;

#pragma aux aux_timer__outwb=\
  "out dx, al"\
  "mov al, ah"\
  "out dx, al"\
  parm [dx] [ax]\
  modify exact [ax] nomemory;

#pragma aux aux_timer__inb=\
  "in al, dx"\
  parm [dx]\
  value [al]\
  modify exact [al] nomemory;

#pragma aux aux_timer__inwb=\
  "in al, dx"\
  "mov ah, al"\
  "in al, dx"\
  "xchg al, ah"\
  parm [dx]\
  value [ax]\
  modify exact [ax] nomemory;

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
  aux_timer__enter_crit();
  result = hw_timer::timer_count;
  aux_timer__leave_crit();
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

  aux_timer__enter_crit();
//  aux_timer__outb(PIT_CONTROL, PIT_PERIOD);
//  pit_tick_bios_inc = INWB(PIT_DATA);
  pit_tick_bios_inc = 0;
  if(pit_tick_bios_inc == 0) {
    pit_tick_bios_inc = 0x10000;
  }
  pit_bios_handler = _dos_getvect(PIT_INTERRUPT);
  if(pit_tick_inc < pit_tick_bios_inc) {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_slower);
    aux_timer__outb(PIT_CONTROL, PIT_PERIOD);
    aux_timer__outwb(PIT_DATA, pit_tick_inc);
  } else {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_faster);
  }
  aux_timer__leave_crit();
}


void hw_timer::stop_timer() {
  aux_timer__enter_crit();
  if(pit_tick_inc < pit_tick_bios_inc) {
    aux_timer__outb(PIT_CONTROL, PIT_PERIOD);
    aux_timer__outwb(PIT_DATA, pit_tick_bios_inc);
  }
  _dos_setvect(PIT_INTERRUPT, pit_bios_handler);
  aux_timer__leave_crit();
}


void (__interrupt hw_timer::pit_handler_bios_slower)() {
  ++timer_count;
  pit_tick_count += pit_tick_inc;
  if(pit_tick_count >= PIT_BIOS_PERIOD) {
    pit_tick_count -= PIT_BIOS_PERIOD;
    _chain_intr(pit_bios_handler);
  }
  else aux_timer__ackint();
}


void (__interrupt hw_timer::pit_handler_bios_faster)() {
  pit_tick_count += PIT_BIOS_PERIOD;
  if(pit_tick_count >= pit_tick_inc) {
    ++timer_count;
  }
  pit_tick_count -= pit_tick_inc;
  _chain_intr(pit_bios_handler);
}


