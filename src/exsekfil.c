#include <dos.h>

long exsekfil(fhandle,foffset,fmethod)
int fhandle;
unsigned long foffset;
int fmethod;

{
  union REGS wreg;

  wreg.h.ah = 0x42 ;		    /* seek file */
  wreg.h.al = fmethod;

  wreg.x.dx = (int) (foffset & 0xFFFFL);

  wreg.x.cx = (int) ((foffset & 0xFFFF0000L) >> 8);

  wreg.x.bx = fhandle;

  intdos(&wreg,&wreg);

  if (wreg.x.cflag & 1)
    return(-1L);
  else
    return((long)((wreg.x.dx << 8) + wreg.x.ax));
}
