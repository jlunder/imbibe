#include "cplusplus.h"

#include <assert.h>
#include <fstream.h>

#include "stdasm.h"


extern void set_text();
#pragma aux set_text = "mov ax, 00003h" "int 10h" modify exact [eax] nomemory;


uint16_t * video_memory = (uint16_t *)0xB8000;


int main(int argc, char * argv[])
{
  int i, j;
  ifstream f;

  set_text();
  for(i = 1; i < argc; ++i)
  {
    f.open(argv[i], ios::binary | ios::in);
    while(f.good())
    {
      for(j = 0; j < 80 * 25; ++j) video_memory[j] = 0x0700;
      f.read((uint8_t *)video_memory, 80 * 25 * sizeof(uint16_t));
      read_key();
    }
    f.close();
  }
  set_text();

  return 0;
}
