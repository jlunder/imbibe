#include "imbibe.hh"

#include <dos.h>

#include "timer.hh"

#include "timer.ii"


#ifdef __386__


extern void ENTER_CRIT();
#pragma aux ENTER_CRIT="pushfd"\
                       "cli"\
                       modify exact [] nomemory;


extern void LEAVE_CRIT();
#pragma aux LEAVE_CRIT="popfd"\
                       modify exact [] nomemory;


#endif //__386__


#ifdef __I86__


extern void ENTER_CRIT();
#pragma aux ENTER_CRIT="pushf"\
                       "cli"\
                       modify exact [] nomemory;


extern void LEAVE_CRIT();
#pragma aux LEAVE_CRIT="popf"\
                       modify exact [] nomemory;


#endif //__I86__


extern void ACKINT();
#pragma aux ACKINT="mov al, 20h"\
                   "out 20h, al"\
                   modify exact [al] nomemory;


extern void OUTB(unsigned short port, unsigned char value);
#pragma aux OUTB="out dx, al"\
                 parm [dx] [al]\
                 modify exact [] nomemory;


extern void OUTWB(unsigned short port, unsigned short value);
#pragma aux OUTWB="out dx, al"\
                  "mov al, ah"\
                  "out dx, al"\
                  parm [dx] [ax]\
                  modify exact [ax] nomemory;

extern unsigned short INB(unsigned short port);
#pragma aux INB="in al, dx"\
                parm [dx]\
                value [al]\
                modify exact [al] nomemory;

extern unsigned short INWB(unsigned short port);
#pragma aux INWB="in al, dx"\
                 "mov ah, al"\
                 "in al, dx"\
                 "xchg al, ah"\
                 parm [dx]\
                 value [ax]\
                 modify exact [ax] nomemory;

timer::timer():
  m_current_time(hw_t.time())
{
}


timer::hw_timer::hw_timer()
{
  start_timer();
}


timer::hw_timer::~hw_timer()
{
  stop_timer();
}


#define PIT_INTERRUPT 8
#define PIT_FREQUENCY 0x1234DD
#define TIMER_FREQUENCY 1000
#define PIT_BIOS_PERIOD 0x10000
#define PIT_CONTROL 0x43
#define PIT_DATA 0x40
#define PIT_PERIOD 0x34


#include <iostream.h>
#include <stdlib.h>


static void timer::hw_timer::start_timer()
{
  ENTER_CRIT();
  pit_tick_inc = PIT_FREQUENCY / TIMER_FREQUENCY;
  pit_tick_count = 0;
  timer_count = 0;

//  OUTB(PIT_CONTROL, PIT_PERIOD);
//  pit_tick_bios_inc = INWB(PIT_DATA);
  pit_tick_bios_inc = 0;
  if(pit_tick_bios_inc == 0)
  {
    pit_tick_bios_inc = 0x10000;
  }
  pit_bios_handler = _dos_getvect(PIT_INTERRUPT);
  if(pit_tick_inc < pit_tick_bios_inc)
  {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_slower);
    OUTB(PIT_CONTROL, PIT_PERIOD);
    OUTWB(PIT_DATA, pit_tick_inc);
  }
  else
  {
    _dos_setvect(PIT_INTERRUPT, pit_handler_bios_faster);
  }
  LEAVE_CRIT();
}


static void timer::hw_timer::stop_timer()
{
  ENTER_CRIT();
  if(pit_tick_inc < pit_tick_bios_inc)
  {
    OUTB(PIT_CONTROL, PIT_PERIOD);
    OUTWB(PIT_DATA, pit_tick_bios_inc);
  }
  _dos_setvect(PIT_INTERRUPT, pit_bios_handler);
  LEAVE_CRIT();
}


static void (__interrupt timer::hw_timer::pit_handler_bios_slower)()
{
  ++timer_count;
  pit_tick_count += pit_tick_inc;
  if(pit_tick_count >= PIT_BIOS_PERIOD)
  {
    pit_tick_count -= PIT_BIOS_PERIOD;
    _chain_intr(pit_bios_handler);
  }
  else ACKINT();
}


static void (__interrupt timer::hw_timer::pit_handler_bios_faster)()
{
  pit_tick_count += PIT_BIOS_PERIOD;
  if(pit_tick_count >= pit_tick_inc)
  {
    ++timer_count;
  }
  pit_tick_count -= pit_tick_inc;
  _chain_intr(pit_bios_handler);
}


static void (__interrupt * timer::hw_timer::pit_bios_handler)();


static unsigned long timer::hw_timer::pit_tick_inc;


static unsigned long timer::hw_timer::pit_tick_bios_inc;


static unsigned long timer::hw_timer::pit_tick_count;


static volatile unsigned long timer::hw_timer::timer_count;


static timer::hw_timer timer::hw_t;


