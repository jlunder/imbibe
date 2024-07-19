#include "cplusplus.hh"

#include "stdlib.h"

#include "key_dispatcher_task.hh"

#include "key_handler.hh"
#include "task.hh"
#include "vector.hh"

#include "key_dispatcher_task.ii"

#include "key_handler.ii"
#include "task.ii"
#include "vector.ii"


#ifdef __386__


/*


struct dpmi_regs
{
  union
  {
    unsigned long edi;
    unsigned short di;
  };
  union
  {
    unsigned long esi;
    unsigned short si;
  };
  union
  {
    unsigned long ebp;
    unsigned short bp;
  };
  unsigned long reserved0;
  union
  {
    unsigned long ebx;
    unsigned short bx;
    struct
    {
      unsigned char bl;
      unsigned char bh;
    };
  };
  union
  {
    unsigned long edx;
    unsigned short dx;
    struct
    {
      unsigned char dl;
      unsigned char dh;
    };
  };
  union
  {
    unsigned long ecx;
    unsigned short cx;
    struct
    {
      unsigned char cl;
      unsigned char ch;
    };
  };
  union
  {
    unsigned long eax;
    unsigned short ax;
    struct
    {
      unsigned char al;
      unsigned char ah;
    };
  };
  union
  {
    unsigned short fl;
    struct
    {
      unsigned cf:1;
      unsigned reserved1:1;
      unsigned pf:1;
      unsigned reserved2:1;
      unsigned af:1;
      unsigned reserved3:1;
      unsigned zf:1;
      unsigned sf:1;
      unsigned tf:1;
      unsigned if_:1;
      unsigned df:1;
      unsigned of:1;
    };
  };
  unsigned short es;
  unsigned short ds;
  unsigned short fs;
  unsigned short gs;
  unsigned short ip;
  unsigned short cs;
  unsigned short sp;
  unsigned short ss;
};


extern void dpmi_rm_interrupt(unsigned char intr, dpmi_regs * r);
#pragma aux dpmi_rm_interrupt="mov bh, 0"\
                              "mov cx, 0"\
                              "pushf"\
                              "pop ax"\
                              "mov [edi + 020h], ax"\
                              "mov ax, 00300h"\
                              "int 31h"\
                              "jnc @1"\
                              "call abort"\
                              "@1:"\
                              parm [bl] [edi]\
                              modify exact [ax bx cx];


inline bool key_avail()
{
  dpmi_regs r;

  r.ah = 0x11;
  r.fl = 0;
  r.ss = 0;
  r.sp = 0;
  dpmi_rm_interrupt(0x16, &r);
  return !r.zf;
}


inline unsigned short read_key()
{
  dpmi_regs r;

  r.ah = 0x10;
  r.fl = 0;
  r.ss = 0;
  r.sp = 0;
  dpmi_rm_interrupt(0x16, &r);
  return r.ax;
}


*/


//faaaahhck PM->RM interfaces are such a bitch!!...


#include <i86.h>
#include <string.h>


#pragma pack(push)
#pragma pack(1)


struct _RMWORDREGS
{
  unsigned short ax, bx, cx, dx, si, di, cflag;
};


struct _RMBYTEREGS
{
  unsigned char al, ah, bl, bh, cl, ch, dl, dh;
};


union RMREGS
{
  struct  _RMWORDREGS x;
  struct  _RMBYTEREGS h;
};


struct _RMREGS
{
  long edi;
  long esi;
  long ebp;
  long reserved;
  long ebx;
  long edx;
  long ecx;
  long eax;
  short flags;
  short es,ds,fs,gs,ip,cs,sp,ss;
};


#pragma pack(pop)


#define IN(reg)     rmregs.e##reg = in->x.reg
#define OUT(reg)    out->x.reg = rmregs.e##reg


int DPMI_int86(int intno, RMREGS *in, RMREGS *out)
{
  _RMREGS         rmregs;
  union REGS      r;
  struct SREGS    sr;

  memset(&rmregs, 0, sizeof(rmregs));
  IN(ax); IN(bx); IN(cx); IN(dx); IN(si); IN(di);

  segread(&sr);
  r.w.ax = 0x300;                 /* DPMI issue real interrupt        */
  r.h.bl = intno;
  r.h.bh = 0;
  r.w.cx = 0;
  sr.es = sr.ds;
  r.x.edi = (unsigned)&rmregs;
  int386x(0x31, &r, &r, &sr);     /* Issue the interrupt              */

  OUT(ax); OUT(bx); OUT(cx); OUT(dx); OUT(si); OUT(di);
  out->x.cflag = rmregs.flags;
  return out->x.ax;
}


inline bool key_avail()
{
  RMREGS regs;

  regs.x.ax = 0x1100;
  DPMI_int86(0x16, &regs, &regs);
  return !(regs.x.cflag & 0x40);
}


inline unsigned short read_key()
{
  RMREGS regs;

  regs.x.ax = 0x1000;
  DPMI_int86(0x16, &regs, &regs);
  return regs.x.ax;
}


#endif //__386__


#ifdef __I86__


extern bool key_avail();
#pragma aux key_avail = "mov ah, 011h"\
                        "int 016h"\
                        "mov al, 0"\
                        "jz @1"\
                        "mov al, 1"\
                        "@1:"\
                        value [al]\
                        modify exact [ax] nomemory;


extern unsigned short read_key();
#pragma aux read_key = "mov ah, 010h"\
                       "int 016h"\
                       value [ax]\
                       modify exact [ax] nomemory;


#endif //__I86__


key_dispatcher_task::key_dispatcher_task(task_manager & n_owner):
  task(n_owner)
{
}


void key_dispatcher_task::cycle()
{
  key_handler_p_list::iterator i;
  unsigned short c;

  while(key_avail())
  {
    c = read_key();
    if(((c & 0xFF) > 0) && ((c & 0xFF) < 128))
    {
      c = c & 0xFF;
    }
    else
    {
      c = (c >> 8) | 0x100;
    }
    for(i = m_key_handlers.begin(); i != m_key_handlers.end(); ++i)
    {
      if((*i)->handle(c)) break;
    }
  }
}


void key_dispatcher_task::add_handler(key_handler & k)
{
  m_key_handlers.insert(m_key_handlers.begin(), &k);
}


void key_dispatcher_task::remove_handler(key_handler & k)
{
  key_handler_p_list::iterator i;

  for(i = m_key_handlers.begin(); (i != m_key_handlers.end()) && (*i != &k); ++i);
  assert(i != m_key_handlers.end());
  m_key_handlers.erase(i);
}


