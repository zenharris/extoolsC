#include <dos.h>
exclsfil(fhandle)
int fhandle;
{
  union REGS sreg,dreg;

  sreg.x.bx = fhandle;
  sreg.h.ah = 0x3e;
  intdos(&sreg,&dreg);
  if (dreg.x.cflag & 1 == 1) return(-1);
  else return(0);
}
