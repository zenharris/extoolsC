#include <dos.h>
exopnfil(fname , fmode)
char *fname;
int fmode;
{
  union REGS sreg,dreg;
  struct SREGS seg;

  sreg.x.dx = (int) fname;
  segread(&seg);
  sreg.x.ax = 0x3d00 | fmode;
  intdosx(&sreg,&dreg,&seg);
  if (dreg.x.cflag & 1 == 1) return(-1);
  else return(dreg.x.ax);
}
