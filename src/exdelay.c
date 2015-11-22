#include <dos.h>

exdelay(ftics)
long ftics;
{
   long wstart; 		/* hold the starting time */
   long welapse;		/* count of elapsed tics  */
   union REGS sreg,dreg;

   sreg.x.ax = 0;		/* return timer count function */
   int86(0x1A,&sreg,&dreg);
   wstart = (long) (dreg.x.cx << 16) | dreg.x.dx;
   welapse = 0;
   while (ftics > welapse) {
     sreg.x.ax = 0;		/* return timer count function */
     int86(0x1A,&sreg,&dreg);
     welapse = ((long) (dreg.x.cx << 16) | dreg.x.dx) - wstart;
   }
}
