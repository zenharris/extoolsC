#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include <math.h>

#define ESCSCAN 1
#define cESC "\001\x3c\x43\x44"

#include <extools.h>
#include <texaco.h>


init_screen(tbl,tbl_len)
struct scr_params tbl[];
int tbl_len;
{
    int iter,pcol;
    char println[81],*temp,fmtbuff[6];
    struct date_time dtbuff;
    struct date_only dbuff;

    for (iter = 0;iter < tbl_len;iter++) {
        if (tbl[iter].format & UNFMT_STRING) {
           exscrprn(tbl[iter].prompt,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
           exscrprn(tbl[iter].field,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
        }else if (tbl[iter].format & DATE_ONLY || tbl[iter].format & BTRV_DATE) {
           exscrprn(tbl[iter].prompt,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
           if (!((tbl[iter].format & DATE_ONLY)?datenull(tbl[iter].field):btrv_datenull(tbl[iter].field))) {
              if (tbl[iter].format & BTRV_DATE) bdtodo(&dbuff,tbl[iter].field);
              else memcpy(&dbuff,tbl[iter].field,sizeof(dbuff));
              if ((getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0))
                 sprintf (println,"%2.2d/%2.2d/%2.2d",dbuff.month,dbuff.day,dbuff.year);
              else
                 sprintf (println,"%2.2d/%2.2d/%2.2d",dbuff.day,dbuff.month,dbuff.year);
              exscrprn(println,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
           }
        }else if (tbl[iter].format & SINGLE_CHAR) {
           sprintf(println,tbl[iter].prompt/*,*(char *)tbl[iter].field*/);
           exscrprn(println,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
           sprintf(println,"%1c",*(char *)tbl[iter].field);
           exscrprn(println,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt) ,tbl[iter].fattr);
        }else if (tbl[iter].format & NUMERIC) {
           if ((temp = strchr(tbl[iter].prompt,'%')) != NULL){
              exgetpce(temp,fmtbuff,' ',1);
              pcol = tbl[iter].col + index(tbl[iter].prompt,'%');
              switch (fmtbuff[strlen(fmtbuff) - 2]) {
                 case 'l':
                    switch (fmtbuff[strlen(fmtbuff)-1]) {
                       case 'd':
                       case 'i':
                       case 'u': /*     UNSIGNED LONG INTEGER   */
                          sprintf(println,tbl[iter].prompt,*(unsigned long *)tbl[iter].field);
                          exscrprn(println,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
                          sprintf(println,fmtbuff,*(unsigned long *)tbl[iter].field);
                          break;
                    }
                    break;
                 case 'h':
                    switch (fmtbuff[strlen(fmtbuff)-1]) {
                       case 'u':  /*   UNSIGNED BYTE    */
                          sprintf(println,tbl[iter].prompt,*(unsigned char *)tbl[iter].field);
                          exscrprn(println,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
                          sprintf(println,fmtbuff,*(unsigned char *)tbl[iter].field);
                          break;
                    }
                    break;
                 default :
                    switch (fmtbuff[strlen(fmtbuff)-1]) {
                       case 'd':
                       case 'i':
                       case 'u': /*        UNSIGNED PLAIN INTEGER    */
                          sprintf(println,tbl[iter].prompt,*(unsigned int *)tbl[iter].field);
                          exscrprn(println,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
                          sprintf(println,fmtbuff,*(unsigned int *)tbl[iter].field);
                          break;
                       case 'f': /*        FLOAT        */
                          sprintf(println,tbl[iter].prompt,*(float *)tbl[iter].field);
                          exscrprn(println,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
                          sprintf(println,fmtbuff,*(float *)tbl[iter].field);
                          break;
                    }
                    break;
              }
              exscrprn(println,tbl[iter].row,pcol,tbl[iter].fattr);
           }
        }else if (tbl[iter].format & DATE_TIME || tbl[iter].format &  BTRV_DATE_TIME) {
           exscrprn(tbl[iter].prompt,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
           if (!((tbl[iter].format & DATE_TIME)?datetnull(tbl[iter].field):btrv_datetnull(tbl[iter].field))) {
              if (tbl[iter].format & BTRV_DATE_TIME) bdttodt(&dtbuff,tbl[iter].field);
              else memcpy(&dtbuff,tbl[iter].field,sizeof(struct date_time));
              if ((getenv("KOMTDT") != NULL) && (strcmp(getenv("KOMTDT"),"USA") == 0))
                 sprintf (println,"%2.2d/%2.2d/%2.2d %2.2d:%2.2d",
                                dtbuff.month,dtbuff.day,dtbuff.year,
                                dtbuff.hour,dtbuff.min);
              else
                 sprintf (println,"%2.2d/%2.2d/%2.2d %2.2d:%2.2d",
                                dtbuff.day,dtbuff.month,dtbuff.year,
                                dtbuff.hour,dtbuff.min);
              exscrprn(println,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
           }
        }else if (tbl[iter].format & FREE_TEXT) {
           exscrprn(excntstr(tbl[iter].prompt,78),tbl[iter].row - 1,0,tbl[iter].prattr);
           init_text(tbl[iter].field,tbl[iter].row,tbl[iter].col,tbl[iter].validate);
        }else  if (tbl[iter].format == 0)
           exscrprn(tbl[iter].prompt,tbl[iter].row,tbl[iter].col,tbl[iter].prattr);
    }
}


static int get_yn(void)
{
    int inchr;
    open_window (10,28,25,5,WHITE,border);
    gotoxy(1,1);
    cprintf ("Exit Enter/Edit Mode\n\r");
    cprintf ("Y/N  ? ");
    while(kbhit()) getch();
    do
       while (!isalpha(inchr = toupper(exinkey())));
    while (inchr != 'Y' && inchr != 'N');
    close_window();
    return (inchr == 'Y');
}



int enter_screen(tbl,tbl_len,edited,en_mode)
struct scr_params tbl[];
int tbl_len;
int *edited;
enum en_mode_type en_mode;
{
   unsigned int crc;
   int iter = 0,wch = CTRL_X,pcol,c,crnt_pos = 0;
   unsigned char chr,att,chr2,att2;
   unsigned int edmode = TYPE_OVER;
   char compstr[81],tmp_fmt[10],blank[81],nulstr[]="";
   char buff[20],fmtbuff[6],*temp;
   struct date_time dtbuff;
   struct date_only dbuff;
   for EVER {
      if (tbl[iter].format  && !(tbl[iter].format & NON_EDIT)){
        if(!(tbl[iter].format & FREE_TEXT)){
          if ((temp = strchr(tbl[iter].prompt,'%')) != NULL)
              pcol = tbl[iter].col + index(tbl[iter].prompt,'%');
          else
              pcol = tbl[iter].col + strlen(tbl[iter].prompt);
          exloccur(tbl[iter].row,pcol- 1);
          exredchr(&chr,&att);
          exwrtchr('[',WHITE/*tbl[iter].prattr*/,1);
          exloccur(tbl[iter].row,pcol +tbl[iter].lnth);
          exredchr(&chr2,&att2);
          exwrtchr(']',WHITE/*tbl[iter].prattr*/,1);
        }
        edmode = (unsigned int)tbl[iter].format & (TYPE_OVER|INSERT_MODE|WORD_WRAP|SUPRESS_DELETE|CAPITALIZE);

        if (crnt_pos < 0) crnt_pos = 81;
        else crnt_pos = 0;

        if ((tbl[iter].format >> 16) != 0) crnt_pos = (int)(tbl[iter].format >> 16);

        if (tbl[iter].format & UNFMT_STRING) {  /* INPUT UNFORMATTED STRING  */
           strcpy(compstr,tbl[iter].field);
           do {
              wch = linedt(tbl[iter].field,tbl[iter].row,
                            tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].lnth,
                            tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);

              if (strcmp(compstr,tbl[iter].field)) *edited = TRUE;
           }while((strcmp(compstr,tbl[iter].field)) && tbl[iter].validate != NULL &&  !(*tbl[iter].validate)(tbl[iter].field,&iter,&wch));
           exscrprn(tbl[iter].field,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
        } else if (tbl[iter].format & DATE_ONLY) {   /* INPUT DATE */
           crc = getcrc(tbl[iter].field,3);
           do {
              wch = datedt(tbl[iter].field,tbl[iter].row,
                           tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
           }while(tbl[iter].validate != NULL && !(*tbl[iter].validate)(tbl[iter].field));
           if (datenull(tbl[iter].field)) {
              sprintf (blank,"%8s",nulstr);
              exscrprn(blank,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
           }
           if (crc != getcrc(tbl[iter].field,3)) *edited = TRUE;
        } else if (tbl[iter].format & BTRV_DATE) {   /* INPUT BTRIEVE DATE */
           crc = getcrc(tbl[iter].field,4);
           do {
              wch = datedt(bdtodo(&dbuff,tbl[iter].field),tbl[iter].row,
                           tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
              dotobd(tbl[iter].field,&dbuff);
           }while(tbl[iter].validate != NULL && !(*tbl[iter].validate)(tbl[iter].field,&iter,&wch));
           if (btrv_datenull(tbl[iter].field)) {
              sprintf (blank,"%8s",nulstr);
              exscrprn(blank,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
           }
           if (crc != getcrc(tbl[iter].field,4)) *edited = TRUE;

        } else if (tbl[iter].format & SINGLE_CHAR) {   /* INPUT CHARACTER */
           crc = *(char *)tbl[iter].field;
           do {
              do {
                 sprintf(buff,"%1c",*((char *)tbl[iter].field));
                 wch = linedt(buff,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),1,tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
              } while (sscanf(buff,"%1c",tbl[iter].field) == 0);
  	           if (crc != *(char*)tbl[iter].field) *edited = TRUE;
           }while(*edited && tbl[iter].validate != NULL && !(*tbl[iter].validate)(tbl[iter].field,&iter,&wch));
           sprintf(buff,"%1c",*((char *)tbl[iter].field));
           exscrprn(buff,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
        } else if (tbl[iter].format & NUMERIC) {   /* INPUT NUMERIC */
           if ((temp = strchr(tbl[iter].prompt,'%')) != NULL){
              exgetpce(temp,fmtbuff,' ',1);
              pcol = tbl[iter].col + index(tbl[iter].prompt,'%');
              switch (fmtbuff[strlen(fmtbuff) - 2]) {
                 case 'l':
                    switch (fmtbuff[strlen(fmtbuff)-1]) {
                       case 'd':
                       case 'i':
                          sprintf(compstr,fmtbuff,*(unsigned long *)tbl[iter].field);
                          do {
                             do {
                                sprintf(buff,fmtbuff,*(unsigned long *)tbl[iter].field);
                                wch = linedt(buff,tbl[iter].row,pcol,tbl[iter].lnth,tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
                             } while (sscanf(buff,"%11li",tbl[iter].field) == 0);
                             sprintf(buff,fmtbuff,*(long *)tbl[iter].field);
                          }while(strcmp(compstr,buff) 
								  			&& tbl[iter].validate != NULL 
											&& !(*tbl[iter].validate)(tbl[iter].field) 
											&& !(crnt_pos=0));
                          break;
                       case 'u': /*     UNSIGNED LONG INTEGER   */
                          sprintf(compstr,fmtbuff,*(unsigned long *)tbl[iter].field);
                          do {
                             do {
                                sprintf(buff,fmtbuff,*(unsigned long *)tbl[iter].field);
                                wch = linedt(buff,tbl[iter].row,pcol,tbl[iter].lnth,tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
                             } while (sscanf(buff,"%11lu",tbl[iter].field) == 0);
                             sprintf(buff,fmtbuff,*(unsigned long *)tbl[iter].field);
                          }while(strcmp(compstr,buff) 
								  			&& tbl[iter].validate != NULL 
											&& !(*tbl[iter].validate)(tbl[iter].field) 
											&& !(crnt_pos=0));
                          break;
                    }
                    break;
                 case 'h':
                    switch (fmtbuff[strlen(fmtbuff)-1]) {
                       case 'u':  /*   UNSIGNED BYTE        */
                          c = *(unsigned char *)tbl[iter].field;
                          sprintf(compstr,fmtbuff,c);
                          do {
                             do {
                                sprintf(buff,fmtbuff,c);
                                wch = linedt(buff,tbl[iter].row,pcol,tbl[iter].lnth,tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
                             } while (sscanf(buff,"%3hu",&c) == 0);
                             sprintf(buff,fmtbuff,c);
                          }while(strcmp(compstr,buff) 
								  			&&tbl[iter].validate != NULL 
											&& !(*tbl[iter].validate)(tbl[iter].field) 
											&& !(crnt_pos=0));
                          *(unsigned char *)tbl[iter].field = c;
                          break;
                    }
                    break;
                 default :
                    switch (fmtbuff[strlen(fmtbuff)-1]) {
                       case 'd':
                       case 'i':
                       case 'u': /*        UNSIGNED PLAIN INTEGER    */
                          sprintf(compstr,fmtbuff,*(unsigned int *)tbl[iter].field);
                          do {
                             do {
                                /* sprintf(tmp_fmt,"%%-%cu",tbl[iter].lnth + '0');*/
                                sprintf(buff,fmtbuff,*(unsigned int *)tbl[iter].field);
                                wch = linedt(buff,tbl[iter].row,pcol,tbl[iter].lnth,tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
                             } while (sscanf(buff,"%5u",tbl[iter].field) == 0);
                             sprintf(buff,fmtbuff,*(unsigned int *)tbl[iter].field);
                          }while(strcmp(compstr,buff) 
								  			&& tbl[iter].validate != NULL 
											&& !(*tbl[iter].validate)(tbl[iter].field) 
											&& !(crnt_pos=0));
                          break;
                       case 'f': /*        FLOAT        */
                          sprintf(compstr,fmtbuff,*(float *)tbl[iter].field);
                          do {
                             sprintf(buff,fmtbuff,*(float *)tbl[iter].field);
                             wch = linedt(buff,tbl[iter].row,pcol,tbl[iter].lnth,tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
                             *(float *)tbl[iter].field = (float)atof(buff);
                             sprintf(buff,fmtbuff,*(float *)tbl[iter].field);
                          }while(strcmp(compstr,buff) 
								  			&&tbl[iter].validate != NULL 
											&& !(*tbl[iter].validate)(tbl[iter].field) 
											&& !(crnt_pos=0));
                          break;
                    }
                    break;
              }
              if (strcmp(compstr,buff)) *edited = TRUE;
              exscrprn(buff,tbl[iter].row,pcol,tbl[iter].fattr);
           }
        } else if (tbl[iter].format & DATE_TIME) {   /* INPUT  DATE/TIME */
           crc = getcrc(tbl[iter].field,5);
           do {
              wch = datimedt(tbl[iter].field,tbl[iter].row,
              tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
           }while(tbl[iter].validate != NULL && !(*tbl[iter].validate)(tbl[iter].field));
           if (datetnull(tbl[iter].field)) {
              sprintf (blank,"%14s",nulstr);
              exscrprn(blank,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
           }
           if (crc != getcrc(tbl[iter].field,5)) *edited = TRUE;
        } else if (tbl[iter].format & BTRV_DATE_TIME) {   /* INPUT Btrieve  DATE/TIME */
           crc = getcrc(tbl[iter].field,8);
           do {
                 wch = datimedt(bdttodt(&dtbuff,tbl[iter].field),tbl[iter].row,
                                tbl[iter].col + strlen(tbl[iter].prompt),
                                tbl[iter].fattr,&crnt_pos,&edmode,tbl[iter].cmndline);
              dttobdt(tbl[iter].field,&dtbuff);
           }while(tbl[iter].validate != NULL && !(*tbl[iter].validate)(tbl[iter].field));
           if (btrv_datetnull(tbl[iter].field)) {
              sprintf (blank,"%14s",nulstr);
              exscrprn(blank,tbl[iter].row,tbl[iter].col + strlen(tbl[iter].prompt),tbl[iter].fattr);
           }
           if (crc != getcrc(tbl[iter].field,8)) *edited = TRUE;
        } else if (tbl[iter].format & FREE_TEXT) { /* INPUT  VARIABLE LENGTH TEXT  */
           texaco(&tbl[iter].field,tbl[iter].row,tbl[iter].col,
                                                     &wch,edited,tbl[iter].lnth,
                                                     tbl[iter].validate,
                                                     edmode,tbl[iter].cmndline);
                                            /* ROW becomes T_LINE. COL , B_LINE */
        }

        if(!(tbl[iter].format & FREE_TEXT)){
            if ((temp = strchr(tbl[iter].prompt,'%')) != NULL)
                pcol = tbl[iter].col + index(tbl[iter].prompt,'%');
            else
                pcol = tbl[iter].col + strlen(tbl[iter].prompt);
            exloccur(tbl[iter].row,pcol - 1);
            exwrtchr(chr,att,1);
            exloccur(tbl[iter].row,pcol + tbl[iter].lnth);
            exwrtchr(chr2,att2,1);
        }
      }else {
         if (iter == 0) wch = ESCEOL;
         if (iter == tbl_len-1) wch = (en_mode == breakout)?wch:ESCBOL;
      }

      if (en_mode == breakout)
         switch (wch){
            case ESCEOL : wch = ENTER;
            case TAB    :
            case ENTER  : if(iter < tbl_len-1) iter++;
                          else return (wch);
                          break;
            case ESCBOL : wch = BACKTAB;
            case BACKTAB: if (iter > 0) iter--;
                          else return (wch);
                          break;
/*            case UARR   :
            case DARR   :
            case ALT_O  :
            case ALT_D  :
            case CTRL_PGUP:
            case CTRL_PGDN:
            case PGUP   :
            case PGDN   :
            case F10    :
            case F5     :
            case F9     :
            case F1     :
            case F2     : return(wch); */
            case ESC    :
            case ESCSCAN: return(ESC);
            default     : return(wch);
         }
      else
         switch (wch){
            case TAB    :
            case DARR   :
            case ENTER  :
            case ESCEOL : if(iter < tbl_len-1) iter++;
                          break;
            case BACKTAB:
            case UARR   :
            case ESCBOL : if (iter > 0) iter--;
                          break;
            case F10    :
            case F9     :
            case F1     :
            case F2     :	if (en_mode == special) return(wch);
									break;
            case ESC    :
            case ESCSCAN:if (*edited ) {
                            if (get_yn()) return(ESC);
                            else break;
                         } else return(ESC);
         }
   }
}


