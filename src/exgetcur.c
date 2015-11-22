#include <dos.h>
exgetcur(frow,fcol,fstart,fend)
int *frow,*fcol,*fstart,*fend;
{

  union REGS winreg,woutreg;
  winreg.h.bh = 0;			/* page 0 */
  winreg.h.ah = 3;
  int86(0x10,&winreg,&woutreg);
  *fstart = woutreg.h.ch;
  *fend   = woutreg.h.cl;
  *frow   = woutreg.h.dh;
  *fcol   = woutreg.h.dl;
}
