#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <dos.h>
#include <stdarg.h>
#include <signal.h>

#include <extools.h>


jmp_buf crit_error_context;
int ctrlc_flag = FALSE;

exfilerr(int ecode,char *source_file,int source_linenum)
{

   char tempstr[60];
   printf("%c",7);
   open_window(8,18,43,9,text_colour,border);
   exscrprn("***  BTRIEVE File Operation Error  ***",0,1,text_colour);
   sprintf(tempstr,"Number  : %d",ecode);
   exscrprn(tempstr,2,12,text_colour);
   sprintf (tempstr,"In Source File  : %s",source_file);
   exscrprn(tempstr,4,2,text_colour);
   sprintf (tempstr,"At Line Number  : %d",source_linenum);
   exscrprn(tempstr,5,2,text_colour);
   exscrprn("Hit Any Key to Continue",crnt_window->lnth,3,text_colour);
   CLEARKBD
   getch();
   close_window();
}



exgenerr(char *source_file,int source_linenum,char *fst_ln,...)
{
   char *out_ptr = NULL;
   va_list argp;
   va_start(argp,fst_ln);
   out_ptr = fst_ln;
   fputc(7,stderr);
   open_window(8,18,43,9,text_colour,border);
   exscrprn("***  General  Operation  Error  ***",0,2,text_colour);
   gotoxy(1,2);
   while (out_ptr != NULL){
      gotoxy(max(0,20 -  (strlen(out_ptr) / 2)),wherey());
      textattr(text_colour);
      cprintf("%s\n\r",out_ptr);
      out_ptr = va_arg(argp,char *);
   }
   cprintf ("\n\rSource : %s  Line Number : %d",source_file,source_linenum);
   exscrprn("Hit Any Key to Continue",crnt_window->lnth,3,text_colour);
   CLEARKBD
   getch();
   close_window();
}

int verifyx(source_array,no_mac)
char *source_array[];
int no_mac;
{
   char mcol;
   fputc(7,stderr);
   open_window(8,18,43,9,text_colour,border);
   exscrprn("Verify Window",0,13,text_colour);
   mcol = max(0,20 -  (max (strlen(source_array[0]),strlen(source_array[1])) / 2));
   CLEARKBD
   for EVER switch (menu(source_array,2,3,mcol,6,40,text_colour,text_hilite,no_mac)) {
      case 1 : close_window();
              return(TRUE);
      case ESC : break;
      case 2 :close_window();
              return(FALSE);
   }
}

int verify(vcode)
int vcode;
{
   static char *vedits[] = {
       "Exit Saving Changes to this Record.",
       "   Exit Without Saving Changes.    "
   };
   static char *vexits[] = {
       " Exit  To  System. ",
       "   Do NOT Exit.    "
   };
   static char *vdels[] = {
       "Delete this Record.",
       " Cancel Deletion.  "
   };
   static char *vdels2[] = {
       "    Cancel Deletion   ",
       "   Confirm  Deletion  "
   };
   static char *vsavpart[] = {
       "  Cancel  Save.   ",
       "Save Partial Text."
   };
   static char *vglobup[] = {
       "  Global Update   ",
       "  Cancel Update   "
   };
   int ret;
   switch (vcode) {
         case 1 : ret = verifyx(vedits,FALSE);
                  break;
         case 2 :/* exscrprn("** Failure to allocate memory. **",0,3,text_colour);*/
                  ret = verifyx(vsavpart,FALSE);
                  break;
         case 3 : ret = verifyx(vdels,FALSE);
						if (ret) ret = !verifyx(vdels2,FALSE);
						break;
         case 4 : ret = verifyx(vexits,FALSE);
                  break;
         case 5 : ret = verifyx(vglobup,FALSE);
                  break;
   }
   return(ret);
}

#define NOT_DISK_ERROR (1<<15)
#define WRITE_ERROR (1<<8)
#define ABORT (1<<11)
#define RETRY (1<<12)
#define IGNORE (1<<13)




static void error_analasys(int errorcode,int deverror,int bpval,int sival)
{
    char *err_mess[] = {"Diskette is write-protected:",
                       "Unknown unit:",
                       "Drive not ready:",
                       "Unknown command:",
                       "Data Error (CRC):",
                       "Bad request structure length:",
                       "Seek error:",
                       "Unknown media type:",
                       "Sector not found:",
                       "Printer out of Paper",
                       "Write fault:",
                       "Read fault:",
                       "General failure:",
                       "In MSDOS",
                       "In File Allocation Table",
                       "In The Directory",
                       "In the Data Area"};
   char *devhdr,buffer[9];
   devhdr = MK_FP((unsigned)bpval,(unsigned)sival);
   if (deverror&NOT_DISK_ERROR) {
      cprintf("%s",err_mess[errorcode & 0xff]);
      strncpy(buffer,devhdr+0x0a,8);
      buffer[8] = 0;
      cprintf("In Device %s\n\n\r",buffer);
   } else {
      if (deverror & WRITE_ERROR) cprintf ("Write Error ");
      else cprintf ("Read Error ");
      cprintf("On drive %c  ",'A'+ (deverror & 0xff));
      cprintf("%s\n\r",err_mess[13 +((deverror >> 9) & 0x03)]);
      cprintf("%s\n\r",err_mess[errorcode & 0xff]);
   }
}

int critical_error_handler(int errorcode,int deverror,int bpval,int sival)
{
   int c;
   open_window(8,18,43,9,text_colour,border);
   gotoxy(1,1);
   fprintf (stderr,"%c%c",7,7);
   cprintf ("            Critical Error\n\n\r");
   error_analasys(errorcode,deverror,bpval,sival);
   cprintf ("[A]bort Gracefully ");
   if (deverror&NOT_DISK_ERROR || deverror & RETRY) cprintf ("[R]etry ");
/*   if (deverror&NOT_DISK_ERROR || deverror & IGNORE) cprintf ("[I]gnore ");*/
   cprintf("\n\r");
   if (deverror&NOT_DISK_ERROR || deverror & ABORT) cprintf ("[H]ard Abort (Emergency use only)\n\r");
   cprintf(" ?");
   for EVER {
      CLEARKBD
      switch (toupper(getch())) {
         case 'A' : cprintf ("Aborting Gracefully\n\r");
                    close_window();
                    longjmp(crit_error_context,1);
                    break;
         case 'H' : if (deverror&NOT_DISK_ERROR || deverror & ABORT) {
                       cprintf ("Aborting Hard !!!\n\r");
                       close_window();
                       hardresume(2);
                    }
                    break;
         case 'R' : if (deverror&NOT_DISK_ERROR || deverror & RETRY) {
                       cprintf ("Retry\n\r");
                       close_window();
                       hardretn(1);
                    }
                    break;
/*         case 'I' : if (deverror&NOT_DISK_ERROR || deverror & IGNORE) {
                       cprintf ("Ignore\n\r");
                       close_window();
                       hardresume(0);
                    }
                    break;*/
      }
   }
}


void ctrlc_handler()
{
   int c;
   signal(SIGINT,SIG_IGN);
   open_window(9,22,37,5,text_colour,border);
   gotoxy(1,1);
   fprintf (stderr,"%c%c",7,7);
   cprintf ("******    Control C Break    ******");
   cprintf ("   [A]bort Gracefully  [C]ontinue  ");
   for EVER {
      CLEARKBD
      c = toupper(getch());
      switch (c) {
         case 'A' : ctrlc_flag = TRUE;
         case 'C' : close_window();
                    signal(SIGINT,ctrlc_handler);
                    return;
      }
   }
}


