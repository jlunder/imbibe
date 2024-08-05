#include "imbibe.h"

#include <dos.h>

#include "timer.h"


extern void ENTER_CRIT();
#pragma aux ENTER_CRIT="pushf"\
                       "cli"\
                       modify exact [] nomemory;


extern void LEAVE_CRIT();
#pragma aux LEAVE_CRIT="popf"\
                       modify exact [] nomemory;


extern void ACKINT();
#pragma aux ACKINT="mov al, 20h"\
                   "out 20h, al"\
                   modify exact [al] nomemory;


extern void OUTB(uint16_t port, uint8_t value);
#pragma aux OUTB="out dx, al"\
                 parm [dx] [al]\
                 modify exact [] nomemory;


extern void OUTWB(uint16_t port, uint16_t value);
#pragma aux OUTWB="out dx, al"\
                  "mov al, ah"\
                  "out dx, al"\
                  parm [dx] [ax]\
                  modify exact [ax] nomemory;

extern uint16_t INB(uint16_t port);
#pragma aux INB="in al, dx"\
                parm [dx]\
                value [al]\
                modify exact [al] nomemory;

extern uint16_t INWB(uint16_t port);
#pragma aux INWB="in al, dx"\
                 "mov ah, al"\
                 "in al, dx"\
                 "xchg al, ah"\
                 parm [dx]\
                 value [ax]\
                 modify exact [ax] nomemory;


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


uint32_t timer::now() {
  uint32_t result;
  ENTER_CRIT();
  result = hw_timer::timer_count;
  LEAVE_CRIT();
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

  ENTER_CRIT();
//  OUTB(PIT_CONTROL, PIT_PERIOD);
//  pit_tick_bios_inc = INWB(PIT_DATA);
  pit_tick_bios_inc = 0;
  if(pit_tick_bios_inc == 0) {
    pit_tick_bios_inc = 0x10000;
  }
  pit_bios_handler = _dos_getvect(PIT_INTERRUPT);
  if(pit_tick_inc < pit_tick_bios_inc) {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_slower);
    OUTB(PIT_CONTROL, PIT_PERIOD);
    OUTWB(PIT_DATA, pit_tick_inc);
  } else {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_faster);
  }
  LEAVE_CRIT();
}


static void hw_timer::stop_timer() {
  ENTER_CRIT();
  if(pit_tick_inc < pit_tick_bios_inc) {
    OUTB(PIT_CONTROL, PIT_PERIOD);
    OUTWB(PIT_DATA, pit_tick_bios_inc);
  }
  _dos_setvect(PIT_INTERRUPT, pit_bios_handler);
  LEAVE_CRIT();
}


static void (__interrupt hw_timer::pit_handler_bios_slower)() {
  ++timer_count;
  pit_tick_count += pit_tick_inc;
  if(pit_tick_count >= PIT_BIOS_PERIOD) {
    pit_tick_count -= PIT_BIOS_PERIOD;
    _chain_intr(pit_bios_handler);
  }
  else ACKINT();
}


static void (__interrupt hw_timer::pit_handler_bios_faster)() {
  pit_tick_count += PIT_BIOS_PERIOD;
  if(pit_tick_count >= pit_tick_inc) {
    ++timer_count;
  }
  pit_tick_count -= pit_tick_inc;
  _chain_intr(pit_bios_handler);
}


void (__interrupt * hw_timer::pit_bios_handler)();
uint32_t hw_timer::pit_tick_inc;
uint32_t hw_timer::pit_tick_bios_inc;
uint32_t hw_timer::pit_tick_count;
volatile uint32_t hw_timer::timer_count;


