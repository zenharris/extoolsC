#include <dos.h>

exrealloc(fbytes , fseg)
unsigned int fbytes;
char **fseg;
{
  union REGS wreg;
  struct SREGS segregs;
  wreg.h.ah = 0x4A;         /* dos reallocate memory   */
  fbytes += (fbytes % 16);
  wreg.x.bx = fbytes;
  segregs.es = FP_SEG((*fseg));
  intdosx(&wreg,&wreg,&segregs);

  if (wreg.x.cflag & 1) {       /* can't allocate memory */
    return(-1);
  }
  else {
     return(0);
  }
}


