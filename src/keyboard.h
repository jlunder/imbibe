#ifndef __KEY_MANAGER_H_INCLUDED
#define __KEY_MANAGER_H_INCLUDED


#include "imbibe.h"

#include "task.h"
#include "vector.h"


typedef uint16_t key_code_t;


class key_code {
public:
  enum {
    enter = 0x0d,
    tab = 0x09,
    backspace = 0x08,
    escape = 0x1b,
    up = 0x148,
    down = 0x150,
    left = 0x14b,
    right = 0x14d,
    pgup = 0x149,
    pgdown = 0x151,
    home = 0x147,
    end = 0x14f,
    insert = 0x152,
    del = 0x153,
    f1 = 0x13b,
    f2 = 0x13c,
    f3 = 0x13d,
    f4 = 0x13e,
    f5 = 0x13f,
    f6 = 0x140,
    f7 = 0x141,
    f8 = 0x142,
    f9 = 0x143,
    f10 = 0x144,
    f11 = 0x185,
    f12 = 0x186,
    shift_tab = 0x10f,
    shift_f1 = 0x154,
    shift_f2 = 0x155,
    shift_f3 = 0x156,
    shift_f4 = 0x157,
    shift_f5 = 0x158,
    shift_f6 = 0x159,
    shift_f7 = 0x15a,
    shift_f8 = 0x15b,
    shift_f9 = 0x15c,
    shift_f10 = 0x15d,
    shift_f11 = 0x187,
    shift_f12 = 0x188,
    control_enter = 0x0a,
    control_backspace = 0x7f,
    control_left = 0x173,
    control_right = 0x174,
    control_pgup = 0x184,
    control_pgdown = 0x176,
    control_home = 0x177,
    control_end = 0x175,
    control_f1 = 0x15e,
    control_f2 = 0x15f,
    control_f3 = 0x160,
    control_f4 = 0x161,
    control_f5 = 0x162,
    control_f6 = 0x163,
    control_f7 = 0x164,
    control_f8 = 0x165,
    control_f9 = 0x166,
    control_f10 = 0x167,
    control_f11 = 0x189,
    control_f12 = 0x18a,
    alt_f1 = 0x168,
    alt_f2 = 0x169,
    alt_f3 = 0x16a,
    alt_f4 = 0x16b,
    alt_f5 = 0x16c,
    alt_f6 = 0x16d,
    alt_f7 = 0x16e,
    alt_f8 = 0x16f,
    alt_f9 = 0x170,
    alt_f10 = 0x171,
    alt_f11 = 0x18b,
    alt_f12 = 0x18c
  };
};


class keyboard
{
public:
  static bool key_event_available();
  static key_code_t read_key_event();
};


#endif // __KEY_MANAGER_H_INCLUDED


