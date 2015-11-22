#include <dos.h>
#include <extools.h>
exfndnxt(fdta)
TDIR *fdta;

{

  union REGS wreg;
  char *svdta;

  svdta = getdta();      /* save old dta address */

  setdta(fdta);           /* set new address */

  wreg.h.ah = 0x4f;		/* find next function */
  intdos(&wreg,&wreg);

  setdta(svdta);        /* conditionalize later for large model */

  if (wreg.x.cflag & 0x0001 == 1) return(wreg.x.ax);
  else return(0);	 /* return 0 if no error, otherwise dos error code */

}
