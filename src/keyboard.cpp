#include "imbibe.h"

#include "keyboard.h"

/*
From stanislavs.org/helppc/scan_codes.html:

Key    Normal   Shift   w/Ctrl    w/Alt

 A     1E61     1E41     1E01     1E00
 B     3062     3042     3002     3000
 C     2E63     2E42     2E03     2E00
 D     2064     2044     2004     2000
 E     1265     1245     1205     1200
 F     2166     2146     2106     2100
 G     2267     2247     2207     2200
 H     2368     2348     2308     2300
 I     1769     1749     1709     1700
 J     246A     244A     240A     2400
 K     256B     254B     250B     2500
 L     266C     264C     260C     2600
 M     326D     324D     320D     3200
 N     316E     314E     310E     3100
 O     186F     184F     180F     1800
 P     1970     1950     1910     1900
 Q     1071     1051     1011     1000
 R     1372     1352     1312     1300
 S     1F73     1F53     1F13     1F00
 T     1474     1454     1414     1400
 U     1675     1655     1615     1600
 V     2F76     2F56     2F16     2F00
 W     1177     1157     1117     1100
 X     2D78     2D58     2D18     2D00
 Y     1579     1559     1519     1500
 Z     2C7A     2C5A     2C1A     2C00

Key    Normal   Shift   w/Ctrl    w/Alt

 1     0231     0221     7800
 2     0332     0340     0300     7900
 3     0433     0423     7A00
 4     0534     0524     7B00
 5     0635     0625     7C00
 6     0736     075E     071E     7D00
 7     0837     0826     7E00
 8     0938     092A     7F00
 9     0A39     0A28     8000
 0     0B30     0B29     8100

Key    Normal   Shift   w/Ctrl    w/Alt

 -     0C2D     0C5F     0C1F     8200
 =     0D3D     0D2B     8300
 [     1A5B     1A7B     1A1B     1A00
 ]     1B5D     1B7D     1B1D     1B00
 ;     273B     273A     2700
 '     2827     2822
 `     2960     297E
 \     2B5C     2B7C     2B1C     2600 (same as Alt L)
 ,     332C     333C
 .     342E     343E
 /     352F     353F

Key    Normal   Shift   w/Ctrl    w/Alt

 F1    3B00     5400     5E00     6800
 F2    3C00     5500     5F00     6900
 F3    3D00     5600     6000     6A00
 F4    3E00     5700     6100     6B00
 F5    3F00     5800     6200     6C00
 F6    4000     5900     6300     6D00
 F7    4100     5A00     6400     6E00
 F8    4200     5B00     6500     6F00
 F9    4300     5C00     6600     7000
F10    4400     5D00     6700     7100
F11    8500     8700     8900     8B00
F12    8600     8800     8A00     8C00

Key  Normal   Shift   w/Ctrl    w/Alt

BS     0E08     0E08     0E7F     0E00
Del	   5300     532E     9300     A300
DnArr  5000     5032     9100     A000
End	   4F00     4F31     7500     9F00
Enter  1C0D     1C0D     1C0A     A600
Esc	   011B     011B     011B     0100
Home   4700     4737     7700     9700
Ins	   5200     5230     9200     A200
KP 5   4C35     8F00
KP *   372A     9600     3700
KP -   4A2D     4A2D     8E00     4A00
KP +   4E2B     4E2B     4E00
KP /   352F     352F     9500     A400
LtArr  4B00     4B34     7300     9B00
PgDn   5100     5133     7600     A100
PgUp   4900     4939     8400     9900
PrtSc  7200
RtArr  4D00     4D36     7400     9D00
Space  3920     3920     3920     3920
Tab	   0F09     0F00     9400     A500
UpArr  4800     4838     8D00     9800
 */

#define logf_key_manager(...) disable_logf("KEYBOARD: " __VA_ARGS__)

#if BUILD_MSDOS
extern bool asm_bios_key_event_available();
extern uint16_t asm_bios_read_key_event();

#if BUILD_MSDOS_WATCOMC
#pragma aux asm_bios_key_event_available =                                     \
    "   mov     ah, 011h          "                                            \
    "   int     016h              "                                            \
    "   mov     al, 0             "                                            \
    "   jz      @1                "                                            \
    "   mov     al, 1             "                                            \
    "@1:                          " modify nomemory[ax] value[al]

#pragma aux asm_bios_read_key_event =                                          \
    "   mov     ah, 010h          "                                            \
    "   int     016h              " modify nomemory[ax] value[ax];
#else
// TODO
#endif

bool keyboard::key_event_available() {
  bool result = asm_bios_key_event_available();
  logf_sim("('key_avail',%d)\n", result);
  return result;
}

key_code_t keyboard::read_key_event() {
  uint16_t k = asm_bios_read_key_event();
  logf_sim("('read_key',0x%04X)\n", k);
  uint8_t ascii = (uint8_t)(k & 0xFF);
  if ((ascii > 0) && (ascii < 128)) {
    return (key_code_t)ascii;
  } else {
    return (key_code_t)(0x100 | (k >> 8));
  }
}

#elif BUILD_POSIX
// In imbibe.cpp
#else
#error New platform support needed?
#endif
