#include <dos.h>

exredfil(fhandle,fbuf,famt)
int fhandle;
char *fbuf;
int famt;
{
  union REGS wreg;
  struct SREGS segregs;
  wreg.h.ah = 0x3F;		/* read file function */

  wreg.x.bx = fhandle;

  segregs.ds = FP_SEG(fbuf);

  wreg.x.dx = FP_OFF(fbuf);

  wreg.x.cx = famt;

  intdosx(&wreg,&wreg,&segregs);

  if (wreg.x.cflag & 1)
    return(-1);
  else
    return(wreg.x.ax);		/* number of bytes actually read */

}

