#include <dos.h>

exalloc(fbytes , fseg)
unsigned int fbytes;
char **fseg;
{
  union REGS wreg;

  wreg.h.ah = 0x48;			/* dos allocate memory	 */
  fbytes += (fbytes % 16);
  wreg.x.bx = fbytes;
  intdos(&wreg,&wreg);

  if (wreg.x.cflag & 1) {       /* can't allocate memory */
    return(-1);
  }
  else {
     (*fseg) = MK_FP(wreg.x.ax,0x00);
     return(0);
  }
}


