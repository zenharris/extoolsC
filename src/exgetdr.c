#include <dos.h>

exgetdr(fdir)
char *fdir;
{
   union REGS wreg;
   struct SREGS segregs;
   wreg.h.ah = 0x47;
   segregs.ds = FP_SEG(fdir);
   wreg.x.si = FP_OFF(fdir);
   wreg.h.dl = 0;

   intdosx(&wreg,&wreg,&segregs);

   if (wreg.x.cflag & 1)
     *fdir = 0;
}
