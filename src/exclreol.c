#include<extools.h>

exclreol(frow,fcol,fattr)
int frow,fcol,fattr;
{
  char fill[81];

  memset(fill,' ',crnt_window->width-fcol);

  fill[crnt_window->width-fcol] = 0;    /* mark end of string */

  exscrprn(fill,frow,fcol,crnt_window->attr);
}

