#include <dos.h>
#include "extools.h"

cls(fattr)
int fattr;
{
  union REGS wreg;

  wreg.x.ax = 0x600;
  wreg.h.bh = crnt_window->attr;
  wreg.h.ch = crnt_window->rul;
  wreg.h.cl = crnt_window->cul;
  wreg.h.dh = crnt_window->rlr;
  wreg.h.dl = crnt_window->clr;
  int86(0x10,&wreg,&wreg);
}


cls_all(fattr)
int fattr;
{
  union REGS wreg;

  wreg.x.ax = 0x600;
  wreg.h.bh = fattr;
  wreg.h.ch = 0;
  wreg.h.cl = 0;
  wreg.h.dh = 24;
  wreg.h.dl = 79;
  int86(0x10,&wreg,&wreg);
}


