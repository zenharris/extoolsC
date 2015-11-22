#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dos.h>
#define CR    13
#define DATIMELNTH 14
#define DATELNTH 8
#include <extools.h>


#define CENTRY 19        /* current century */

static int monthd[] = {31,28,31,30,31,30,31,31,30,31,30,31};

last_date(struct btr_date *date)
{
  if ((date->month >= 1) && (date->month <= 12)) {
  		if (date->year%4 == 0 && date->year !=0) monthd[1] = 29;
		else monthd[1]=28;
	    date->day = monthd[date->month - 1];
	}


}
check_btrv_date(struct btr_date *date)
/*
 This function returns (1) if the date passed (*Date) is a valid
 date in the form M/D/Y.  If it is a vaild date it will return it in the
 form MM/DD/YY always. If not, it will return false (0).
*/
{ /* Check_Date */
  if ((date->day+date->month+date->year) == 0) return(TRUE);
  monthd[1] = 28;
  if ((date->month >= 1) && (date->month <= 12)) 
    return((date->day >= 1) && (date->day <= (monthd[date->month - 1] + ((date->year % 4) == 0 &&
              date->month == 2))) && (date->year >= 0) && (date->year <= 99));
  else
    return(FALSE);
} /* Check_Date */

char check_date(struct date_only *date)
/*
 This function returns (1) if the date passed (*Date) is a valid
 date in the form M/D/Y.  If it is a vaild date it will return it in the
 form MM/DD/YY always. If not, it will return false (0).
*/
{ /* Check_Date */
  if ((date->day+date->month+date->year) == 0) return(TRUE);
	monthd[1] = 28;
  if ((date->month >= 1) && (date->month <= 12)) 
    return((date->day >= 1) && (date->day <= (monthd[date->month - 1] + ((date->year % 4) == 0 &&
              date->month == 2))) && (date->year >= 0) && (date->year <= 99));
  else
    return(FALSE);

} /* Check_Date */

struct date_only *todays_date(struct date_only *Buffer)
{
    struct tm *adjtime;
    struct date_only todays_date;
    long elapstime;
    time (&elapstime);
    adjtime = localtime(&elapstime);
    Buffer->day = adjtime->tm_mday;
    Buffer->month = adjtime->tm_mon+1;
    Buffer->year = adjtime->tm_year;
    return(Buffer);
}

struct btr_date *todays_btrv_date(struct btr_date *buffer)
{
   struct date_only dtbuff;
   return(dotobd(buffer,todays_date(&dtbuff)));

}

long btrv_gtoj(gregdate)
struct btr_date *gregdate;
{
   struct date_only dt_buff;
   return(gtoj(bdtodo(&dt_buff,gregdate)));
}


long gtoj(gregdate)
struct date_only *gregdate;
{

        int i;
        int leapd;
        static long cdays = 36524, ydays = 365;
        int iyr = gregdate->year,imo = gregdate->month,iday = gregdate->day;
        /* adjust month array for leap year/non-leap year */
        if (iyr < 0 || iyr > 99) return(-1);
        if (iyr%4 == 0 && iyr != 0 || CENTRY%4 == 0)
                monthd[1] = 29;
        else
                monthd[1] = 28;

        /* check for invalid month */
        if (imo < 1 || imo > 12) return(-1);

        /* check for invalid day */
        if (iday < 1 || iday > monthd[imo-1]) return(-1);

        /* determine the number of "extra" leap years caused by the  */
        /* %400 criteria and add to it the number of leap years that */
        /* has occured up to the prior year of current century.      */
        leapd = CENTRY/4;
        if (iyr != 0) leapd += (iyr-1)/4;

        /* determine number of days elapsed in current year */
        for (i=0;i<(imo-1);i++)
                iday = iday + monthd[i];

        /* calculate julian date */
        return (CENTRY*cdays + iyr*ydays + leapd + iday);
}

/* function to convert a julian date (1 = 1st day AD) into a         */
/* gregorian date in the format mm/dd/yy.  Returns a 0 if successful */
/* or a -1 if not.                                                   */
/*                                                                   */

int btrv_jtog(julian,outdate)
long julian;
struct btr_date *outdate;
{
   struct date_only dt_buff;
	int temp;

	temp = jtog(julian,&dt_buff);
	dotobd(outdate,&dt_buff);
	return(temp);

}


int jtog(julian,outdate)
long julian;
struct date_only *outdate;
{
    static int monthd[] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int centry,leap,i,iyr,imo,iday;
    static long cdays = 36524, ydays = 365;

    /* reduce julian from 1st day AD to 1st day of CENTRY */
    julian -= CENTRY*cdays + CENTRY/4;

    /* determine year */
    iyr = julian/ydays;
    if ((julian - (iyr*ydays + iyr/4)) < 0)
        iyr -= 1;
    if (iyr < 0)
        return(-1);

    /* determine if this is a leap year or not */
    if (iyr%4 == 0 && iyr != 0 || CENTRY%4 == 0)
        leap = TRUE;
    else
        leap = FALSE;

    /* determine month */
    julian = julian - (iyr*ydays + iyr/4);
    if (leap) julian += 1;
    for (imo=12;imo>0;imo--) {
        i=0;
        if ((leap) && (imo > 2)) i = -1;
        if ((julian - monthd[imo-1] + i) > 0) break;
    }
    if (imo == 0) imo = 1;

    /* determine day */
    iday = julian - monthd[imo-1] + i;
    if (iday == 0) {
        iyr -= 1;
        if (iyr < 0) return(-1);
        imo = 12;
        iday = 31;
    }

    outdate->day = iday;
    outdate->month = imo;
    outdate->year = iyr;

    /* transfer into output string */
/*
    if (iday > 9)
        outdate[0] = (char)(iday/10 + '0');
    else
        outdate[0] = '0';
    outdate[1] = (char)(iday%10 + '0');
    outdate[2] = '/';

    if (imo > 9)
        outdate[3] = '1';
    else
        outdate[3] = '0';
    outdate[4] = (char)(imo%10 + '0');
    outdate[5] = '/';

    if (iyr > 9)
        outdate[6] = (char)(iyr/10 + '0');
    else
        outdate[6] = '0';
    outdate[7] = (char)(iyr%10 + '0');
    outdate[8] = '\0';
*/
    /* done */
    return(0);
}

/* function to determine the day of the week a given gregorian date  */
/* falls on.  Returns a 0 if successful or a -1 if not.              */
/*                                                                   */
/* WARNING - day must be declared to be at least 10 characters or a  */
/*           memory overwrite will occure.                           */


char *weekday(julian)
long julian;
{
    static char *days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    int iday;

    /* calculate day of week */
    iday = (julian-1) % 7;
    return(days[iday]);
}


char *mnthyear(gregdate)
struct date_only *gregdate;
{
   static char *MOY[] = {"jan", "feb", "mar", "apr",
                  "may", "jun", "jul", "aug",
                  "sep", "oct", "nov", "dec"};

   return(MOY[gregdate->month - 1]);
}

char *btrv_mnthyear(struct btr_date *dt)
{
	struct date_only dtbuff;
 	return (mnthyear(bdtodo(&dtbuff,dt)));
}


long today_btrv_comp(date)
struct btr_date *date;
{
   struct date_only dt_buff;
   return (today_comp(bdtodo(&dt_buff,date)));
}


long today_comp(gregdate)
struct date_only *gregdate;
{
    struct tm *adjtime;
    struct date_only todays_date;
    long elapstime;
    time (&elapstime);
    adjtime = localtime(&elapstime);
    todays_date.day = adjtime->tm_mday;
    todays_date.month = adjtime->tm_mon+1;
    todays_date.year = adjtime->tm_year;
      return(gtoj(&todays_date) - gtoj(gregdate));
}


unsigned char normyl(val)
unsigned char val;
{
   return((( val>> 4) * 10) + (val & 0x0f));
}

struct date_only *xdtodo(struct date_only *bff,struct x_date_time *xdt)
{
   bff->day = xdt->day;
   bff->month = xdt->month;
   bff->year = xdt->year;
   return(bff);
}


struct x_date_time *sys_date_time(xdt)
struct x_date_time *xdt;
{
   union REGS wreg;
   wreg.h.ah = 0x02 ;
   int86(0x1a,&wreg,&wreg);
   xdt->hour = normyl(wreg.h.ch);
   xdt->min = normyl(wreg.h.cl);
   xdt->sec = normyl(wreg.h.dh);
   wreg.h.ah = 0x04;
   int86(0x1a,&wreg,&wreg);
   xdt->day = normyl(wreg.h.dl);
   xdt->month = normyl(wreg.h.dh);
   xdt->year = normyl(wreg.h.cl);
   xdt->cent = normyl(wreg.h.ch);
   return(xdt);
}

struct x_date_time *dos_date_time(xdt)
struct x_date_time *xdt;
{
    struct date date;
    struct time dtime;

    getdate(&date);


    xdt->day = date.da_day;
    xdt->month = date.da_mon;
    xdt->year = date.da_year - 1900;
    xdt->cent = date.da_year / 100;

    gettime(&dtime);

    xdt->hour = dtime.ti_hour;
    xdt->min = dtime.ti_min;
    xdt->sec = dtime.ti_sec;

    return (xdt);
}

char *asc_x_date_time(bff,xdt)
char *bff;
struct x_date_time *xdt;
{
   struct date_only dtbuff;
   sprintf (bff,"%3s %3s %2.2hu %2.2hu:%2.2hu:%2.2hu%c %2.2hu%2.2hu",
               weekday(gtoj(xdtodo(&dtbuff,xdt))),mnthyear(xdtodo(&dtbuff,xdt)),xdt->day,
               (xdt->hour == 0) ? 12 : (xdt->hour <= 12) ? xdt->hour : xdt->hour - 12,
               xdt->min,xdt->sec,(xdt->hour <= 12) ? 'a':'p',xdt->cent,xdt->year);
   return(bff);
}


char *asc_date(char *buff,struct date_only *prtdate)
{
	int yank_date = (getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0);
	sprintf(buff,"%2.2hu/%2.2hu/%4i",(yank_date)?prtdate->month:prtdate->day,
                  (yank_date)?prtdate->day:prtdate->month,prtdate->year+1900);
    return(buff);
}

char *asc_datetime(char *buff,struct date_time *prtdate)
{
	int yank_date = (getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0);
   sprintf(buff,"%2.2hu/%2.2hu/%4i %2.2hu:%2.2hu",(yank_date)?prtdate->month:prtdate->day,
           (yank_date)?prtdate->day:prtdate->month,prtdate->year+1900,prtdate->hour,prtdate->min);
   return(buff);
}


char *asc_btr_datetime(char *buff,struct btr_datetime *prtdate)
{
	int yank_date = (getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0);
   sprintf(buff,"%2.2hu/%2.2hu/%4i %2.2hu:%2.2hu",(yank_date)?prtdate->month:prtdate->day,
           (yank_date)?prtdate->day:prtdate->month,prtdate->year,prtdate->hour,prtdate->min);
   return(buff);
}

char *asc_btr_date(char *buff,struct btr_date *prtdate)
{
    if (!btrv_datenull(prtdate))
       sprintf(buff,"%2.2hu/%2.2hu/%4i",prtdate->day,prtdate->month,prtdate->year);
    else *buff = 0;
    return(buff);
}


struct date_time *bdttodt (struct date_time *out_date,struct btr_datetime *in_date)
{
   out_date->day = in_date->day;
   out_date->month = in_date->month;
   out_date->year = in_date->year%100;
   out_date->hour = in_date->hour;
   out_date->min = in_date->min;
   return(out_date);
}

struct btr_datetime *dttobdt (struct btr_datetime *out_date,struct date_time *in_date)
{
   out_date->day = in_date->day;
   out_date->month = in_date->month;
   out_date->year = (in_date->year == 0) ? 0 : in_date->year + 1900;
   out_date->hour = in_date->hour;
   out_date->min = in_date->min;
   out_date->hndrdths = out_date->sec = 0;
   return(out_date);
}

struct date_only *bdtodo (struct date_only *out_date,struct btr_date *in_date)
{
   out_date->day = in_date->day;
   out_date->month = in_date->month;
   out_date->year = in_date->year%100;
   return(out_date);
}

struct btr_date *dotobd (struct btr_date *out_date,struct date_only *in_date)
{
   out_date->day = in_date->day;
   out_date->month = in_date->month;
   out_date->year = (in_date->year == 0) ? 0 : in_date->year + 1900;
   return(out_date);
}


int datenull(date)
struct date_only *date;
{
   return(((date)->month+(date)->day+(date)->year == 0)?TRUE:FALSE);
}

int btrv_datenull(date)
struct btr_date *date;
{
   return(((date)->month+(date)->day+(date)->year == 0)?TRUE:FALSE);
}

int datetnull(date)
struct date_time *date;
{
   return (((date)->month+(date)->day+(date)->year+(date)->hour+(date)->min == 0)?TRUE:FALSE);
}

int btrv_datetnull(date)
struct btr_datetime *date;
{
   return(((date)->month+(date)->day+(date)->year+(date)->hour+(date)->min == 0)?TRUE:FALSE);
}




int datimedt(dt,row,col,attr,edtpos,control,cmndline)
struct date_time *dt;
int col,row,attr,*edtpos;
unsigned int *control;
struct cmndline_params cmndline[];
{
   int c,exit;
   int inchr,scan,yank_date = (getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0);
   char edtstr[15];
   int day,month,year,hour,min;
   struct date_only test_date;
	struct linedt_msg_rec msg;

	msg.edtstr = edtstr;
	msg.col = col;
	msg.row = row;
	msg.lnth = 14;
	msg.attr = attr;
	msg.pos = edtpos;
	msg.control = control;
	msg.cmndline = cmndline;


	if (yank_date) {
      day = dt->month;
      month = dt->day;
   } else {
      day = dt->day;
      month = dt->month;
   }
   year = dt->year;
   hour = dt->hour;
   min = dt->min;
   sprintf(edtstr,"%2.2u/%2.2u/%2.2u %2.2u:%2.2u",day,month,year,hour,min);
   if (*edtpos >= DATIMELNTH) *edtpos = DATIMELNTH - 1;
   else if (*edtpos < 0) *edtpos = 0;
   exloccur(row,*edtpos + col);
   exscrprn(edtstr,row,col,attr);
   exsetcur(6,8);
   exit = TRUE;
   do {
      exscrprn(edtstr,row,col,attr);
      inchr = command_line(cmndline,&scan,&msg);
      switch (inchr) {
         case 0 : switch (scan) {
                      case RARR  : if (*edtpos < strlen(edtstr)) (*edtpos)++;
                                   else *edtpos = DATIMELNTH + 1;
                                   switch (*edtpos) {
                                      case 2 :
                                      case 5 :
                                      case 8 :
                                      case 11: (*edtpos)++;
                                               break;
                                    }
                                   break;
                      case LARR  : (*edtpos)--;
                                   switch (*edtpos) {
                                      case 2 :
                                      case 5 :
                                      case 8 :
                                      case 11: (*edtpos)--;
                                               break;
                                   }
                                   break;

                      case HOME  : if (!read_only) {
								 				  exdltstr(edtstr,0,strlen(edtstr));
      	                             *edtpos = 0;
   	                                exloccur(row,*edtpos + col);
	                                   sprintf(edtstr,"%2.2u/%2.2u/%2.2u %2.2u:%2.2u",day,month,
                                                    year,hour,min);
											  }
                                   break;
                      default    :exit = FALSE;
                                  break;

                   }
                   break;
         case TAB : scan = TAB;
         case  CR :exit = FALSE;
                   break;
         case  ESC:  exit = FALSE;
                     break;
         case BACKSPACE :
                    if (*edtpos > 0 ) {
                       (*edtpos)--;
                       switch (*edtpos) {
                           case 2 :
                           case 5 :
                           case 8 :
                           case 11: (*edtpos)--;
                                    break;
                       }
/*                       edtstr[*edtpos] = ' ';
                       exscrprn(edtstr,row,col,attr);*/
                       exloccur(row,*edtpos + col);
                    }
                    break;
         default : if (!read_only && isdigit(inchr)) {
                         edtstr[*edtpos] = inchr;
                         exscrprn(edtstr,row,col,attr);
                         (*edtpos)++;
                         switch (*edtpos) {
                           case 2 :
                           case 5 :
                           case 8 :
                           case 11: (*edtpos)++;
                                    break;
                        }
                   }
                   break;
     }
     if (*edtpos >= DATIMELNTH) {
        exit = FALSE;
        scan = ESCEOL ; /* DARR;*/
     }else if (*edtpos < 0) {
        exit = FALSE;
        scan = ESCBOL ; /* UARR;*/
     }

     if (!exit) {
        if (sscanf(edtstr,"%2hu/%2hu/%2hu%2hu:%2hu",&day,&month,&year,&hour,&min) == 5){
           test_date.day = (unsigned char) day;
           test_date.month = (unsigned char) month;
           test_date.year = (unsigned char) year;
           if (!check_date(&test_date)) {
              exscrprn("    Invalid   ",row,col,RED);
              fprintf (stderr,"%c",7);
              exit = TRUE;
           }
        } else exit = TRUE;
        if (exit)
           if (*edtpos >= DATIMELNTH) *edtpos = DATIMELNTH - 1;
           else if (*edtpos < 0) *edtpos = 0;
     }
     exloccur(row,*edtpos + col);
   }while (exit);
   sprintf(edtstr,"%2.2u/%2.2u/%2.2u %2.2u:%2.2u",day,month,year,hour,min);
   exscrprn(edtstr,row,col,attr);
   if (yank_date) {
      dt->day = (unsigned char) month;
      dt->month = (unsigned char) day;
   } else {
      dt->day =(unsigned char) day;
      dt->month =(unsigned char) month;
   }
   dt->year =(unsigned char) year;
   dt->hour =(unsigned char) hour;
   dt->min = (unsigned char)min;
   if (*control & INSERT_MODE) exsetcur(3,8);
   return(scan);
}

int btrv_datedt(dat,row,col,attr,edtpos,control,cmndline)
struct btr_date *dat;
int col,row,*edtpos;
unsigned int *control;
struct cmndline_params cmndline[];
{
   int ret;
   struct date_only dt_buff;
   ret = datedt(bdtodo(&dt_buff,dat),row,col,attr,edtpos,control,cmndline);
   dotobd(dat,&dt_buff);
   return(ret);
}




int datedt(dat,row,col,attr,edtpos,control,cmndline)
struct date_only *dat;
int col,row,*edtpos;
unsigned int *control;
struct cmndline_params cmndline[];
{
   int c,exit,day,month,year;
   int inchr,scan,yank_date = (getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0);
   char edtstr[10];
   struct date_only test_date;
	struct linedt_msg_rec msg;

	msg.edtstr = edtstr;
	msg.col = col;
	msg.row = row;
	msg.lnth = 8;
	msg.attr = attr;
	msg.pos = edtpos;
	msg.control = control;
	msg.cmndline = cmndline;


	if (yank_date) {
      day = dat->month;
      month = dat->day;
   } else {
      day = dat->day;
      month = dat->month;
   }
   year = dat->year;
   sprintf(edtstr,"%2.2u/%2.2u/%2.2u",day,month,year);
   if (*edtpos >= DATELNTH) *edtpos = DATELNTH - 1;
   else if (*edtpos < 0) *edtpos = 0;
   exloccur(row,*edtpos + col);
   exscrprn(edtstr,row,col,attr);
   exsetcur(6,8);
   exit = TRUE;
   while (exit) {
      exscrprn(edtstr,row,col,attr);
      inchr = command_line(cmndline,&scan,&msg);
      switch (inchr) {
         case 0 : switch (scan) {
                      case RARR  : if (*edtpos < strlen(edtstr)) (*edtpos)++;
                                   else *edtpos = DATELNTH + 1;
                                   switch (*edtpos)  {
                                      case 2 :
                                      case 5 : (*edtpos)++;
                                               break;
                                    }
                                   break;
                      case LARR  : (*edtpos)--;
                                   switch (*edtpos) {
                                      case 5 :
                                      case 2 : (*edtpos)--;
                                               break;
                                   }
                                   break;

                      case HOME  : if (!read_only) {
								 					exdltstr(edtstr,0,strlen(edtstr));
      	                             *edtpos = 0;
   	                                exloccur(row,*edtpos + col);
	                                   sprintf(edtstr,"%2.2d/%2.2d/%2.2d",day,month,year);
											  }
                                   break;
                      default    :exit = FALSE;
                                  break;

                   }
                   break;
         case TAB : scan = TAB;
         case  CR :exit = FALSE;
                   break;
         case  ESC:  exit = FALSE;
                     break;
         case BACKSPACE :
                    if (*edtpos > 0) {
                       (*edtpos)--;
                       switch (*edtpos) {
                          case 5 :
                          case 2 : (*edtpos)--;
                                   break;
                       }
/*                       edtstr[*edtpos] = ' ';
                       exscrprn(edtstr,row,col,attr);*/
                       exloccur(row,*edtpos + col);
                    }
                    break;
         default : if (!read_only && isdigit(inchr)) {
                         edtstr[*edtpos] = inchr;
                         exscrprn(edtstr,row,col,attr);
                         (*edtpos)++;
                         switch (*edtpos) {
                           case 2 :
                           case 5 : (*edtpos)++;
                                    break;
                        }
                   }
                   break;
     }
     if (*edtpos >= DATELNTH) {
        exit = FALSE;
        scan = ESCEOL ; /* DARR; */
     }else if (*edtpos < 0) {
        exit = FALSE;
        scan = ESCBOL ; /* UARR; */
     }


     if (!exit) {
        if (sscanf(edtstr,"%2hu/ %2hu/ %2hu",&day,&month,&year) == 3){
           test_date.day = (unsigned char) day;
           test_date.month = (unsigned char) month;
           test_date.year = (unsigned char) year;
           if (!check_date(&test_date)) {
              exscrprn("Invalid ",row,col,RED);
              fprintf (stderr,"%c",7);
              exit = TRUE;
           }
        } else exit = TRUE;
        if (exit)
           if (*edtpos >= DATELNTH) *edtpos = DATELNTH - 1;
           else if (*edtpos < 0) *edtpos = 0;
     }
     exloccur(row,*edtpos + col);
   }
   sprintf(edtstr,"%2.2u/%2.2u/%2.2u",day,month,year);
   exscrprn(edtstr,row,col,attr);
   if (yank_date) {
      dat->day = (unsigned char) month;
      dat->month = (unsigned char) day;
   } else {
      dat->day =(unsigned char) day;
      dat->month =(unsigned char) month;
   }
   dat->year =(unsigned char) year;
   if (*control & INSERT_MODE) exsetcur(3,8);
   return(scan);
}
