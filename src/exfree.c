#include <dos.h>

exfree(fseg)
char **fseg;
{
  union REGS wreg;
  struct SREGS segregs;
  wreg.h.ah = 0x49;         /* dos release memory   */
  segregs.es = FP_SEG((*fseg));
  intdosx(&wreg,&wreg,&segregs);

  if (wreg.x.cflag & 1)
    return(-1);
  else
     return(0);
}


