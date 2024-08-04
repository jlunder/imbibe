#ifndef __KEY_HANDLER_HH_INCLUDED
#define __KEY_HANDLER_HH_INCLUDED


#include "imbibe.h"


class key_handler
{
public:
  enum
  {
    key_enter = 0x0d,
    key_tab = 0x09,
    key_backspace = 0x08,
    key_escape = 0x1b,
    key_up = 0x148,
    key_down = 0x150,
    key_left = 0x14b,
    key_right = 0x14d,
    key_pgup = 0x149,
    key_pgdown = 0x151,
    key_home = 0x147,
    key_end = 0x14f,
    key_insert = 0x152,
    key_delete = 0x153,
    key_f1 = 0x13b,
    key_f2 = 0x13c,
    key_f3 = 0x13d,
    key_f4 = 0x13e,
    key_f5 = 0x13f,
    key_f6 = 0x140,
    key_f7 = 0x141,
    key_f8 = 0x142,
    key_f9 = 0x143,
    key_f10 = 0x144,
    key_f11 = 0x185,
    key_f12 = 0x186,
    key_shift_tab = 0x10f,
    key_shift_f1 = 0x154,
    key_shift_f2 = 0x155,
    key_shift_f3 = 0x156,
    key_shift_f4 = 0x157,
    key_shift_f5 = 0x158,
    key_shift_f6 = 0x159,
    key_shift_f7 = 0x15a,
    key_shift_f8 = 0x15b,
    key_shift_f9 = 0x15c,
    key_shift_f10 = 0x15d,
    key_shift_f11 = 0x187,
    key_shift_f12 = 0x188,
    key_control_enter = 0x0a,
    key_control_backspace = 0x7f,
    key_control_left = 0x173,
    key_control_right = 0x174,
    key_control_pgup = 0x184,
    key_control_pgdown = 0x176,
    key_control_home = 0x177,
    key_control_end = 0x175,
    key_control_f1 = 0x15e,
    key_control_f2 = 0x15f,
    key_control_f3 = 0x160,
    key_control_f4 = 0x161,
    key_control_f5 = 0x162,
    key_control_f6 = 0x163,
    key_control_f7 = 0x164,
    key_control_f8 = 0x165,
    key_control_f9 = 0x166,
    key_control_f10 = 0x167,
    key_control_f11 = 0x189,
    key_control_f12 = 0x18a,
    key_alt_f1 = 0x168,
    key_alt_f2 = 0x169,
    key_alt_f3 = 0x16a,
    key_alt_f4 = 0x16b,
    key_alt_f5 = 0x16c,
    key_alt_f6 = 0x16d,
    key_alt_f7 = 0x16e,
    key_alt_f8 = 0x16f,
    key_alt_f9 = 0x170,
    key_alt_f10 = 0x171,
    key_alt_f11 = 0x18b,
    key_alt_f12 = 0x18c
  };
  virtual bool handle(int key) = 0;
};


#endif //__KEY_HANDLER_HH_INCLUDED


