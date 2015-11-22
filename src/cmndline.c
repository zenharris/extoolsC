#include <stdio.h>
#include <stdlib.h>
#include <sets.h>
#include <extools.h>

#define TOGGLE (1<<15)

int cmndline_colour = (BROWN << 4) | BLACK;
int cmndline_inverse = RED;


static int cmd_menu(tbl,tbl_lnth,rul,cul,rlr,clr,norm,hilite)
struct menu_params tbl[];
int tbl_lnth;
int rul,cul,rlr,clr;
int norm,hilite;
{
    int scan,c;
    int crnt_line = rul,iter;
    for (iter = 0;iter < tbl_lnth && (iter + crnt_line) <= rlr;iter++){
        exscrprn(tbl[iter].prompt,iter + crnt_line,cul,norm);
    }
    iter = 0;
    exscrprn(tbl[iter].prompt,crnt_line,cul,hilite);
    for (;;) {
        c = exinkey(&scan);
        switch(c) {
            case 00 : switch(scan) {
                case DARR : if (iter < tbl_lnth - 1) {
                               exscrprn(tbl[iter++].prompt,crnt_line++,cul,norm);
                               if (crnt_line > rlr) {
                                  exscrlup((rul << 8) + cul,(rlr << 8) + clr,1,norm);
                                  crnt_line = rlr;
                               }
                               exscrprn(tbl[iter].prompt,crnt_line,cul,hilite);
                            }
                            break;
                case UARR : if (iter > 0) {
                               exscrprn(tbl[iter--].prompt,crnt_line--,cul,norm);
                               if (crnt_line < rul) {
                                  exscrldn((rul << 8) + cul,(rlr << 8) + clr,1,norm);
                                  crnt_line = rul;
                               }
                               exscrprn(tbl[iter].prompt,crnt_line,cul,hilite);
                            }
                            break;
                default  : return(scan);
            }
            break;
            case CR : return(iter|TOGGLE);
            case ESC : return (ESC);
        }
    }
}

#define exscrprn(str,row,col,attr) exscrprn(str,row,col,attr)

void title_line(char *title)
{
	char blank[81];
	memset(blank,' ',80);
	blank[79] = 0;
	exscrprn(blank,2,0,WHITE);
 exscrprn(excntstr(title,80),2,0,WHITE);
}

defset(funct_keys,CHARACTER,256,0);

int command_line (cmd_pack,scan,message)
struct cmndline_params cmd_pack[];
int *scan;
void *message;
{
   char cmn_line1[85],cmn_line2[85],blank[15] = "          ",out_buff[20];
   int fkey_index[10],x,y,menu_width,box_pos,ret,ret2,ret3,ret4,buff_row,buff_col;
   int funct = FALSE,local_keystroke = TRUE;
    int cmd_lnth = (cmd_pack != NULL) ? cmd_pack[0].fkey : 0;
	struct cmndline_params **cmd_buff;
	static int once = FALSE;
   static void *save_ptr = NULL;

	if (cmd_lnth == 0xff) {
		save_ptr = NULL;
      return (exinkey(scan));
   }

	cmd_pack++;
   if (!once)
      {
      if (set_of(&funct_keys,F1 __ F10,_E)==FAILURE)
         {
         gen_error("Failure to populate Set");
         exit(99);
         }
      once = TRUE;
      }
   memset(fkey_index,0,sizeof(fkey_index));
   for (x = 0;x < cmd_lnth;x++) fkey_index[cmd_pack[x].fkey - F1] = x+1;
   sprintf(cmn_line1,"[F1]%-11s [F2]%-11s [F3]%-11s [F4]%-11s [F5]%-12s\0",
                  (!cmd_lnth || fkey_index[0] == 0) ? blank : cmd_pack[fkey_index[0]-1].prompt,
                  (!cmd_lnth || fkey_index[1] == 0) ? blank : cmd_pack[fkey_index[1]-1].prompt,
                  (!cmd_lnth || fkey_index[2] == 0) ? blank : cmd_pack[fkey_index[2]-1].prompt,
                  (!cmd_lnth || fkey_index[3] == 0) ? blank : cmd_pack[fkey_index[3]-1].prompt,
                  (!cmd_lnth || fkey_index[4] == 0) ? blank : cmd_pack[fkey_index[4]-1].prompt);
   sprintf(cmn_line2,"[F6]%-11s [F7]%-11s [F8]%-11s [F9]%-11s [F10]%-11s\0",
                  (!cmd_lnth || fkey_index[5] == 0) ? blank : cmd_pack[fkey_index[5]-1].prompt,
                  (!cmd_lnth || fkey_index[6] == 0) ? blank : cmd_pack[fkey_index[6]-1].prompt,
                  (!cmd_lnth || fkey_index[7] == 0) ? blank : cmd_pack[fkey_index[7]-1].prompt,
                  (!cmd_lnth || fkey_index[8] == 0) ? blank : cmd_pack[fkey_index[8]-1].prompt,
                  (!cmd_lnth || fkey_index[9] == 0) ? blank : cmd_pack[fkey_index[9]-1].prompt);

	if (cmd_lnth == 0) {
		exscrprn(cmn_line1,23,0,cmndline_inverse);
  		exscrprn(cmn_line2,24,0,cmndline_inverse);
		save_ptr = cmd_pack;
      return (exinkey(scan));
   }


	if (save_ptr != cmd_pack) {
		exscrprn(cmn_line1,23,0,cmndline_colour);
   	exscrprn(cmn_line2,24,0,cmndline_colour);
		save_ptr = cmd_pack;
	}
   do {
/*      CLEARKBD*/
      if (!funct) ret = exinkey(&ret2);
      funct = FALSE;
      if (ret == 0) {
         if (in_set(&funct_keys,ret2)) {
            sprintf (out_buff,"[F%d]%-11s\0",(ret2 - F1)+1,
                     (fkey_index[ret2-F1]==0)? blank : cmd_pack[fkey_index[ret2 - F1]-1].prompt);
            buff_row = (ret2 > F5) ? 24 : 23;
            buff_col = ((ret2 > F5) ? ret2-F6 : ret2-F1) * 16;
            exscrprn(out_buff,buff_row,buff_col,cmndline_inverse);

                       /* Execute Keystroke Routine */
            if (!((x = fkey_index[ret2 - F1] - 1) < 0)) {
               if (cmd_pack[x].fn_ptr != NULL){
						  cmd_buff = message;
						  (*cmd_buff) = &cmd_pack[x];
						  (*cmd_pack[x].fn_ptr)(message);
                  local_keystroke = FALSE;
               }
               else
               if (cmd_pack[x].menu != NULL) {
                  for (y=0,menu_width=0;y<cmd_pack[x].menu_lnth;y++)
                     if (strlen(cmd_pack[x].menu[y]) > menu_width)
                        menu_width = strlen(cmd_pack[x].menu[y]);
                  box_pos = max(0,(((((ret2 > F5)?ret2-F6:ret2-F1)*16)+8)-(menu_width/2)));
                  open_window(max(0,23 - (cmd_pack[x].menu_lnth+2)),
                              max(0,box_pos-max(0,(box_pos+menu_width+2)-80)),
                              min(80,menu_width+2),
                              min(23,cmd_pack[x].menu_lnth+2),text_colour,border);
                  exscrprn(cmd_pack[x].prompt,crnt_window->rul-1,crnt_window->cul,cmndline_inverse);
                  ret3 = cmd_menu(cmd_pack[x].menu,cmd_pack[x].menu_lnth,
                                  0,0,min(20,cmd_pack[x].menu_lnth-1),min(77,menu_width-1),
                                  text_colour,text_hilite);
                  close_window();
                  if (ret3 < 0 && (ret3&(~TOGGLE)) < cmd_pack[x].menu_lnth){
                     if (cmd_pack[x].menu[ret3&(~TOGGLE)].fn_ptr != NULL)
/* Execute Menu Opt*/(*cmd_pack[x].menu[ret3&(~TOGGLE)].fn_ptr)(message);
                     if (((struct scrllst_msg_rec *)message)->breaker) local_keystroke = FALSE;

                  }/* else if (ret3 == ESC){
                     counter = F1 - 1;
                  }*/ else if (in_set(&funct_keys,ret3)) {
                     funct = TRUE;
                     ret2 = ret3;
                  }

               } else local_keystroke = FALSE;
            }
            exscrprn(out_buff,buff_row,buff_col,cmndline_colour);
         } else local_keystroke = FALSE;
      } else local_keystroke = FALSE;
   } while (local_keystroke);
   *scan = ret2;
   return(ret);
}



