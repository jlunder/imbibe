#ifndef __STDASM_H_INCLUDED
#define __STDASM_H_INCLUDED


extern bool key_avail();
#pragma aux key_avail="mov ah, 1"\
                      "int 16h"\
                      "mov al, 0"\
                      "setnz al"\
                      value [al]\
                      modify exact [ax] nomemory;


extern char read_key();
#pragma aux read_key="mov ah, 8"\
                     "int 21h"\
                     value [al]\
                     modify exact [ax] nomemory;


#endif //__STDASM_H_INCLUDED


