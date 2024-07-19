#include "cplusplus.hh"

#include <assert.h>
#include <fstream.h>

#include "stdasm.h"


extern void set_text();
#pragma aux set_text = "mov ax, 00003h" "int 10h" modify exact [eax] nomemory;


unsigned short * video_memory = (unsigned short *)0xB8000;


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
      f.read((unsigned char *)video_memory, 80 * 25 * sizeof(unsigned short));
      read_key();
    }
    f.close();
  }
  set_text();

  return 0;
}
