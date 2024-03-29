#include <stdlib.h>

#include <stdio.h>
#include <conio.h>
#include <extools.h>

struct crnt_wind_rec *crnt_window = NULL;

void destroy_windows(void)
{
   register void *tempw;
   while (crnt_window != NULL) {
      tempw = crnt_window;
      crnt_window = crnt_window->last;
      free(tempw);
   }
}
#define MSGLEN 11
char *message_bit[MSGLEN] = {
	"EXTOOLS Libraries\n",
	"Copyright Michael Brown\n",
	"(049) 621783\n\n",
	"Lisenced to\n",
	"Mega Technology\n",
	"For Their Customer\n",
	"Rhino Truck Bodies\n", 
	"31 Munibung Rd\n",
	"CARDIFF 2285\n",
	"A.C.N. 002 090 495\n",
	"Phone (049) 566322\n"};

void init_windows(void)
{
   static unsigned long share_crc = 407776,calc_crc=0;
	int iter;
/*	for(iter=0;iter < MSGLEN;iter++) calc_crc += getcrc(message_bit[iter],strlen(message_bit[iter]));
   if (calc_crc != share_crc) {
      fprintf (stderr,"\nPlease Don't try Hacking \n No-matter how easy it may be.\n\n");
      exit (99);

   }
	fprintf(stderr,"\n");
	for (iter=0;iter < MSGLEN;iter++) fprintf(stderr,"%s",excntstr(message_bit[iter],80));
	exdelay(36);
  */

   if (crnt_window == NULL){
      if ((crnt_window = malloc(sizeof(struct crnt_wind_rec))) != NULL){
/*         set_window (NULL,0,0,79,24,BLACK,80,25);*/
         crnt_window->svscr = NULL;
         crnt_window->cul   = 0;
         crnt_window->rul   = 0;
         crnt_window->clr   = 79;
         crnt_window->rlr   = 24;
         crnt_window->attr  = WHITE;
         crnt_window->width = 80;
         crnt_window->lnth  = 25;

         crnt_window->last = NULL;
/*         atexit (destroy_windows);*/
      } else gen_error("Not enough memory for windows");
   }
}

void *open_window(row,col,width,lnth,attr,type)
int row,col,width,lnth,attr;
enum win_type type;
{
   void *svscr;
   struct crnt_wind_rec *tempw;
   if (crnt_window == NULL) init_windows();
   if ((svscr = malloc((width*lnth)*2)) == NULL ) return(NULL);
   if ((tempw = malloc(sizeof(struct crnt_wind_rec))) == NULL){
      free(svscr);
      return(NULL);
   }
   savscrn(svscr,col,row,col+width-1,row+lnth-1);
   tempw->last = crnt_window;
   crnt_window = tempw;
   if (type == border) {
      window(col+2,row+2,col+width-1,row+lnth-1);
/*   set_window(svscr,col+1,row+1,col+width-2,row+lnth-2,attr,width-2,lnth-2);*/
      {
          crnt_window->svscr = svscr;
          crnt_window->cul   = col+1;
          crnt_window->rul   = row+1;
          crnt_window->clr   = col+width-2;
          crnt_window->rlr   = row+lnth-2;
          crnt_window->attr  = attr;
          crnt_window->width = width - 2;
          crnt_window->lnth  = lnth - 2;
          crnt_window->type  = type;
       }
       exdrwbox(row,col,width,lnth,border_colour);
   } else if (type == borderless){
      window(col+1,row+1,col+width,row+lnth);
/*   set_window(svscr,col+1,row+1,col+width-2,row+lnth-2,attr,width-2,lnth-2);*/
      {
          crnt_window->svscr = svscr;
          crnt_window->cul   = col;
          crnt_window->rul   = row;
          crnt_window->clr   = col+width-1;
          crnt_window->rlr   = row+lnth-1;
          crnt_window->attr  = attr;
          crnt_window->width = width;
          crnt_window->lnth  = lnth;
          crnt_window->type  = type;
       }
   }
   textattr(attr);
   cls(attr);
   return(crnt_window);
}

void close_window(void)
{
   struct crnt_wind_rec *tempw;
   if(crnt_window->last != NULL){
      if (crnt_window->type == border)
         rstscrn(crnt_window->svscr,crnt_window->cul-1,crnt_window->rul-1,
                                 crnt_window->clr+1,crnt_window->rlr+1);
      else if (crnt_window->type == borderless)
         rstscrn(crnt_window->svscr,crnt_window->cul,crnt_window->rul,
                                 crnt_window->clr,crnt_window->rlr);

      free(crnt_window->svscr);
      tempw = crnt_window;
      crnt_window = crnt_window->last;
      free(tempw);
      window(crnt_window->cul+1,crnt_window->rul+1,
             crnt_window->clr+1,crnt_window->rlr+1);
      textattr(crnt_window->attr);
   }
}
