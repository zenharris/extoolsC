#include<dos.h>
#include<extools.h>

exfndfst (fmask,fattr,fdta)
char *fmask;
int  fattr;
TDIR *fdta;
{

  union REGS wreg;
  struct SREGS segregs;
  char *svdta;

  svdta = getdta();      /* get old dta */

  setdta(fdta);           /* set new dta */

  segregs.ds = FP_SEG(fmask);
  wreg.x.dx  = FP_OFF(fmask);
  wreg.h.ah  = 0x4e;		/* find first matching file	 */
  wreg.x.cx  = fattr;		/* set file attribute for search */
  intdosx(&wreg,&wreg,&segregs);

  setdta(svdta);        /* conditionalize later for large model */

  if (wreg.x.cflag & 0x0001) return(wreg.x.ax);
  else return(0);	/* return 0 if no error, otherwise dos error code */
}
