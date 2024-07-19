#include <conio.h>
#include <iostream.h>

extern "C" void timer_handler();
extern "C" void timer_init();
extern "C" void timer_done();
extern "C" unsigned long timer_count;
extern "C" unsigned long overflow;

int main()
{
  unsigned long temp;

  timer_init();
  while(getch() != 27)
  {
    temp = timer_count;
    cout << temp << " " << (temp & 0xFFFF) << " " << (temp >> 16) << " " << overflow << endl;
  }
  timer_done();
  return 0;
}
