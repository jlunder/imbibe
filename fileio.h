#ifndef FILEIO_INCLUDED
#define FILEIO_INCLUDED

#include "/source.c/include/types.h"

typedef unsigned short handle;

//Common file I/O routines

#define foRead      0
#define foWrite     1
#define foReadWrite 2

#define faReadOnly  0x01
#define faHidden    0x02
#define faSystem    0x04
#define faVolume    0x08
#define faDirectory 0x10
#define faArchive   0x20

#define faNone      0x00
#define faNormal    faArchive
#define faAll       0x3F

#define dnA 1
#define dnB 2
#define dnC 3
#define dnD 4
#define dnE 5
#define dnF 6
#define dnG 7
#define dnH 8
#define dnI 9
#define dnJ 10
#define dnK 11
#define dnL 12
#define dnM 13
#define dnN 14
#define dnO 15
#define dnP 16
#define dnQ 17
#define dnR 18
#define dnS 19
#define dnT 20
#define dnU 21
#define dnV 22
#define dnW 23
#define dnX 24
#define dnY 25
#define dnZ 26

extern unsigned short Create(handle &, Char const *, unsigned short);
#pragma aux Create="mov ah, 3Ch"\
                   "int 21h"\
                   "jc @End"\
                   "mov [esi], ax"\
                   "xor ax, ax"\
                   "@End:"\
                   parm [esi] [edx] [cx]\
                   value [ax]\
                   modify exact [ax];

extern unsigned short Open(handle &, Char const *, Byte);
#pragma aux Open="mov ah, 3Dh"\
                 "int 21h"\
                 "jc @End"\
                 "mov [esi], ax"\
                 "xor ax, ax"\
                 "@End:"\
                 parm [esi] [edx] [al]\
                 value [ax]\
                 modify exact [ax];

extern unsigned short Close(handle);
#pragma aux Close="mov ah, 3Eh"\
                  "int 21h"\
                  "jc @End"\
                  "xor ax, ax"\
                  "@End:"\
                  parm [bx]\
                  value [ax]\
                  modify exact [ax] nomemory;

extern unsigned short SeekTo(handle, unsigned long);
#pragma aux SeekTo="mov ecx, edx"\
                   "shr ecx, 16"\
                   "and edx, 0FFFFh"\
                   "mov ax, 4200h"\
                   "int 21h"\
                   "jc @End"\
                   "xor ax, ax"\
                   "@End:"\
                   parm [bx] [edx]\
                   value [ax]\
                   modify exact [ax ecx edx] nomemory;

extern unsigned short SeekBy(handle, LongInt);
#pragma aux SeekBy="mov ecx, edx"\
                   "shr ecx, 16"\
                   "and edx, 0FFFFh"\
                   "mov ax, 4201h"\
                   "int 21h"\
                   "jc @End"\
                   "xor ax, ax"\
                   "@End:"\
                   parm [bx] [edx]\
                   value [ax]\
                   modify exact [ax ecx edx] nomemory;

extern unsigned long FilePos(handle);
#pragma aux FilePos="xor ecx, ecx"\
                    "xor edx, edx"\
                    "mov ax, 4201h"\
                    "int 21h"\
                    "jc @End"\
                    "shl edx, 16"\
                    "mov dx, ax"\
                    "@End:"\
                    parm [bx]\
                    value [edx]\
                    modify exact [ax ecx edx] nomemory;

extern unsigned long FileSize(handle);
#pragma aux FileSize="xor ecx, ecx"\
                     "xor edx, edx"\
                     "xor edi, edi"\
                     "mov ax, 4201h"\
                     "int 21h"\
                     "jc @End"\
                     "shl edx, 16"\
                     "mov dx, ax"\
                     "mov esi, edx"\
                     "xor ecx, ecx"\
                     "xor edx, edx"\
                     "mov ax, 4202h"\
                     "int 21h"\
                     "jc @End"\
                     "shl edx, 16"\
                     "mov dx, ax"\
                     "mov edi, edx"\
                     "mov ecx, esi"\
                     "shr ecx, 16"\
                     "mov edx, esi"\
                     "and edx, 0FFFFh"\
                     "mov ax, 4200h"\
                     "int 21h"\
                     "@End:"\
                     parm [bx]\
                     value [edi]\
                     modify exact [ax ecx edx esi edi] nomemory;

//Binary file I/O routines

extern unsigned short Read(handle, void *, unsigned long);
#pragma aux Read="mov ah, 3Fh"\
                 "int 21h"\
                 "jc @End"\
                 "xor ax, ax"\
                 "@End:"\
                 parm [bx] [edx] [ecx]\
                 value [ax]\
                 modify exact [ax];

extern unsigned short Write(handle, void const *, unsigned long);
#pragma aux Write="mov ah, 40h"\
                  "int 21h"\
                  "jc @End"\
                  "xor ax, ax"\
                  "@End:"\
                  parm [bx] [edx] [ecx]\
                  value [ax]\
                  modify exact [ax];

//Text file I/O routines

//DOS file management functions

extern unsigned short Delete(Char const *);
#pragma aux Delete="mov ah, 41h"\
                   "int 21h"\
                   "jc @End"\
                   "xor ax, ax"\
                   "@End:"\
                   parm [edx]\
                   value [ax]\
                   modify exact [ax] nomemory;

extern unsigned short Rename(Char const *);
#pragma aux Rename="mov ah, 56h"\
                   "int 21h"\
                   "jc @End"\
                   "xor ax, ax"\
                   "@End:"\
                   parm [edx]\
                   value [ax]\
                   modify exact [ax] nomemory;

extern unsigned short GetAttr(Char const *, unsigned short &);
#pragma aux GetAttr="mov ax, 4300h"\
                    "int 21h"\
                    "jc @End"\
                    "mov [esi], ax"\
                    "xor ax, ax"\
                    "@End:"\
                    parm [edx] [esi]\
                    value [ax]\
                    modify exact [ax];

extern unsigned short SetAttr(Char const *, unsigned short);
#pragma aux SetAttr="mov ax, 4301h"\
                    "int 21h"\
                    "jc @End"\
                    "xor ax, ax"\
                    "@End:"\
                    parm [edx] [cx]\
                    value [ax]\
                    modify exact [ax] nomemory;

extern unsigned short MkDir(Char const *);
#pragma aux MkDir="mov ah, 39h"\
                  "int 21h"\
                  "jc @End"\
                  "xor ax, ax"\
                  "@End:"\
                  parm [edx]\
                  value [ax]\
                  modify exact [ax] nomemory;

extern unsigned short RmDir(Char const *);
#pragma aux RmDir="mov ah, 3Ah"\
                  "int 21h"\
                  "jc @End"\
                  "xor ax, ax"\
                  "@End:"\
                  parm [edx]\
                  value [ax]\
                  modify exact [ax] nomemory;

extern unsigned short ChDir(Char const *);
#pragma aux ChDir="mov ah, 3Bh"\
                  "int 21h"\
                  "jc @End"\
                  "xor ax, ax"\
                  "@End:"\
                  parm [edx]\
                  value [ax]\
                  modify exact [ax] nomemory;

extern unsigned long DiskSize(Byte);
#pragma aux DiskSize="xor eax, eax"\
                     "xor ecx, ecx"\
                     "xor edx, edx"\
                     "mov ah, 36h"\
                     "int 21h"\
                     "mul edx"\
                     "mul ecx"\
                     parm [dl]\
                     value [eax]\
                     modify exact [eax ebx ecx edx] nomemory;

extern unsigned long DiskFree(Byte);
#pragma aux DiskFree="xor eax, eax"\
                     "xor ebx, ebx"\
                     "xor ecx, ecx"\
                     "mov ah, 36h"\
                     "int 21h"\
                     "mul ebx"\
                     "mul ecx"\
                     parm [dl]\
                     value [eax]\
                     modify exact [eax ebx ecx edx] nomemory;

//Useful file functions 

//Beware of trying to Exist an open file, it could cause an "unexpected error"! 
Bool Exist(Char const * filename) {
  handle F;
  if(!Open(F, filename, foRead)) {
    Close(F);
    return 1;
  }
  else return 0;
}

#endif
