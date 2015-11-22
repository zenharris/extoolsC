#include <dos.h>
exgetdrv()
{
  union REGS wreg;

  wreg.h.ah = 0x19;
  intdos(&wreg,&wreg);

  return(wreg.h.al);
}
