#include <dos.h>
exredchr ( fchr , fattr )
char *fchr , *fattr;
{
  union REGS wreg;

  wreg.h.ah = 8;
  wreg.h.bh = 0;
  int86(0x10,&wreg,&wreg);
  *fchr =   wreg.h.al;
  *fattr =  wreg.h.ah;
}
