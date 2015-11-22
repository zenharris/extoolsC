#include <extools.h>
#include <stdlib.h>

exinstr(fstr,flen,frow,fcol,fattr,ftime,fkeys)
char *fstr;
int flen,frow,fcol,fattr;
long ftime;
char *fkeys;
{


  static int WINSERT = 0;	/* insert mode flag	 */
  int WDONE = 0;		/* init loop condition	 */
  int w1,w2,wstart,wend;	/* work variables for cursor */
  char wtemp[256];		/* temporary work string */
  int WNULLFOUND;		/* flag variable	 */
  int wcol;			/* work column variable  */
  long wtime,wtics;		/* time accumulator	 */
  int wch,wscan;		/* char and scan code	 */
  char *expad();

  wcol	= fcol; 		/* work column indicator */


  if (MEMB(0x40,0x49) == 3) {	/* color mode */
    wstart = 6;
    wend   = 7;
  }
  else {
    wstart = 12;
    wend   = CTRL_M;
  }

  exsetcur(WINSERT ? wstart-4 : wstart , wend); /* set cursor size */

  sprintf(wtemp,"%s%s",fstr,expad(flen-strlen(fstr),' '));
  exscrprn(wtemp,frow,fcol,fattr);

  while (! WDONE) {		     /* endless loop */

    exloccur(frow,wcol);    /* position cursor correctly */

    wtime = time(&wtics);	  /* initialize elapsed time counter */

    if (wcol - fcol == flen) {
      wch = CTRL_M;
      break;
    }

    while ( ! exkeyprs()) {
      if (time(&wtics) - wtime >= ftime) {
	wch = CTRL_M;
	break;
      }
    }

    if (WDONE) break;	/* must have timed out */

    /* at this point there is a key sitting in the buffer for us */

    wch = exinkey(&wscan);    /* read char in buffer */

    /* look for a command key and exit if hit */
    if (strchr(fkeys,wscan)) {
      wch = wscan;
      WDONE = 1;
    }
    else if (wscan == PLUS) {  /* fake like it's a special key */
      exredscr(wtemp,frow,fcol,flen);
      strcpy(fstr,exrgtjst(wtemp,flen));
      exscrprn(fstr,frow,fcol,fattr);
      WDONE = 1;
      wch = CTRL_M;
    }

    if (! WDONE)
      do {
	WNULLFOUND = 0; 	   /* init boolean	    */
	switch (wch) {
	  case 0 :		   /* some kind of command key */
	    WNULLFOUND = 1;	   /* got a command key        */
	    switch(wscan) {	   /* translate arrow keys to wordstar */
	      case HOME :
		wch = CTRL_A;
		break;
	      case END	:
		wch = CTRL_F;
		break;
	      case UARR :
		wch = CTRL_E;
		break;
	      case DARR :
		wch = CTRL_X;
		break;
	      case LARR :
		wch = CTRL_S;
		break;
	      case RARR :
		wch = CTRL_D;
		break;
	      case DELETE :
		wch = CTRL_G;
		break;
	      case INSERT :
		wch = CTRL_V;
		break;
	      case PGUP   :
		wch = CTRL_R;
		break;
	      case PGDN   :
		wch = CTRL_C;
		break;
	      case BACKTAB :
		wch = BACKTAB;
		break;

	      default :
		WNULLFOUND = 0; 	   /* reset to false */
		break;
	    }	   /* end to inner case statement */
	    break;
	  case CTRL_E :
	  case CTRL_X :
	  case CTRL_C :
	  case CTRL_R :
	    WDONE = 1;
	    break;
	  case CTRL_G :
	    if (flen-(wcol-fcol)-1)
          exredscr(wtemp,frow,wcol+1,flen-(wcol-fcol)-1);
	    else
	      wtemp[0] = 0;
	    strcat(wtemp," ");
        exscrprn(wtemp,frow,wcol,fattr);
	    break;
	  case BACKSPACE :
	    if (wcol > fcol) {
          exredscr(wtemp,frow,wcol,flen-(wcol-fcol));
	      strcat(wtemp," ");
	      wcol = max(fcol,wcol-1);
          exscrprn(wtemp,frow,wcol,fattr);
	    }
	    break;
	  case CTRL_A :
	    wcol = fcol;	  /* back to column 1 of field */
	    break;
	  case CTRL_F :
	    wcol = fcol + flen - 1;   /* end of field */
	    break;
	  case CTRL_S :
	    wcol = max(fcol,wcol-1);
	    break;
	  case CTRL_D :
	    wcol = min(fcol+flen,wcol+1);
	    break;
	  case CTRL_V :
	    WINSERT = ! WINSERT;
        if (WINSERT) exsetcur(wstart-4,wend);
        else exsetcur(wstart,wend);
	    break;
	  case CTRL_M :
	    WDONE = 1;
	    break;
	  case TAB :
	    wcol = min(fcol+flen-1,wcol+8);
	    break;
	  case BACKTAB :
	    wcol = max(fcol,wcol-8);
	    break;
	  default :
	    if (wch >= ' ' && wch <= '~')
	      if (! WINSERT) {
        exwrtchr(wch,fattr,1);
		wcol ++;
	      }
	      else if (flen-(wcol-fcol)-1) {	  /* insert mode */
        exredscr(wtemp,frow,wcol,flen-(wcol-fcol)-1);
        exwrtchr(wch,fattr,1);
        exscrprn(wtemp,frow,++wcol,fattr);
	      }
	      else {
        exwrtchr(wch,fattr,1);
		++wcol;
	      }
	    break;
	}
      } while (WNULLFOUND);
  }

  exsetcur(wstart,wend);     /* restore cursor size */
  exredscr(fstr,frow,fcol,flen);

  -- flen;
  if (fstr[flen] == ' ') {  /* get rid of trailing spaces */
    do
      flen --;
    while (fstr[flen] == ' ');
    fstr[flen + 1] = 0;
  }
  return(wch);		     /* maximum length */
}
