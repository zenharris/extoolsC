#include <dos.h>
#include <conio.h>
#include <extools.h>


exscrlup ( fuleft , flrite , fnum , fattr )
int fuleft,flrite,fnum,fattr;
{

  union REGS wreg;
  struct text_info info;
  gettextinfo(&info);

  wreg.h.ah = 6;
  wreg.h.al = fnum;
  wreg.x.cx = fuleft;
  wreg.h.ch += crnt_window->rul;
  wreg.h.cl += crnt_window->cul;
  wreg.x.dx = flrite;
  wreg.h.dh += crnt_window->rul;
  wreg.h.dl += crnt_window->cul;
  wreg.h.bh = fattr;
  int86(0x10,&wreg,&wreg);
}
