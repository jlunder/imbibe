#ifndef __KEY_MANAGER_H_INCLUDED
#define __KEY_MANAGER_H_INCLUDED

#include "imbibe.h"

typedef uint16_t key_code_t;

class key_code {
public:
  enum {
    enter = 0x0d, // == control_l
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
    control_enter = 0x0a, // == control_j
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
    control_a = 1 + 'a' - 'a',
    control_b = 1 + 'b' - 'a',
    control_c = 1 + 'c' - 'a',
    control_d = 1 + 'd' - 'a',
    control_e = 1 + 'e' - 'a',
    control_f = 1 + 'f' - 'a',
    control_g = 1 + 'g' - 'a',
    control_h = 1 + 'h' - 'a',
    control_i = 1 + 'i' - 'a',
    control_j = 1 + 'j' - 'a',
    control_k = 1 + 'k' - 'a',
    control_l = 1 + 'l' - 'a',
    control_m = 1 + 'm' - 'a',
    control_n = 1 + 'n' - 'a',
    control_o = 1 + 'o' - 'a',
    control_p = 1 + 'p' - 'a',
    control_q = 1 + 'q' - 'a',
    control_r = 1 + 'r' - 'a',
    control_s = 1 + 's' - 'a',
    control_t = 1 + 't' - 'a',
    control_u = 1 + 'u' - 'a',
    control_v = 1 + 'v' - 'a',
    control_w = 1 + 'w' - 'a',
    control_x = 1 + 'x' - 'a',
    control_y = 1 + 'y' - 'a',
    control_z = 1 + 'z' - 'a',
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
    alt_f12 = 0x18c,
    alt_a = 0x011E,
    alt_b = 0x0130,
    alt_c = 0x012E,
    alt_d = 0x0120,
    alt_e = 0x0112,
    alt_f = 0x0121,
    alt_g = 0x0122,
    alt_h = 0x0123,
    alt_i = 0x0117,
    alt_j = 0x0124,
    alt_k = 0x0125,
    alt_l = 0x0126,
    alt_m = 0x0132,
    alt_n = 0x0131,
    alt_o = 0x0118,
    alt_p = 0x0119,
    alt_q = 0x0110,
    alt_r = 0x0113,
    alt_s = 0x011F,
    alt_t = 0x0114,
    alt_u = 0x0116,
    alt_v = 0x012F,
    alt_w = 0x0111,
    alt_x = 0x012D,
    alt_y = 0x0115,
    alt_z = 0x012C,
  };
};

namespace keyboard {
extern bool key_event_available();
extern key_code_t read_key_event();
} // namespace keyboard

#endif // __KEY_MANAGER_H_INCLUDED
