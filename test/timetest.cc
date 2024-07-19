#include <iostream.h>


typedef unsigned char bool;
bool const true = 1;
bool const false = 0;


#include "timer.hh"

#include "timer.ii"

extern char read_key();
#pragma aux read_key="mov ah, 8"\
                     "int 21h"\
                     value [al]\
                     modify exact [ax] nomemory;


int main(int argc, char * argv[])
{
  timer t;

  while(read_key() != 27)
  {
    cout << t.delta_time() << endl;
  }
  return 0;
}


