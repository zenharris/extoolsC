#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <extools.h>


#define CR    13
#define TEXSTATUS "The TEXACO text editor. V3.05                           \
      F5 Full Screen"

void (*after_init_text)(void) = NULL;
int insert_mode = TRUE;
int read_only = FALSE;
int right_margin = 70;

char del_buffer[300];


static insert(str,pos,chr)
char *str;
int pos;
char chr;
{
   char buff[2];
   buff[0] = 0;
   exchrstr(buff,chr);
   exinsstr(str,buff,pos);
}

static delete(str,pos)
char *str;
int pos;
{
   char buff[81];
   buff[0] = 0;
   strcat(buff,exlftstr(str,pos));
   strcat(buff,exrgtstr(str,strlen(str)-pos-1));
   str[0] = 0;
   strcat(str,buff);
}


struct line_rec *balloc()
{
   struct line_rec *memptr;
   if ((memptr =(struct line_rec *) malloc(sizeof(struct line_rec))) == NULL){
      gen_error("Insufficient memory for entire text.");
      CLEARKBD
   }
   else {
      memptr->storage[0] = 0;
      memptr->next = NULL;
      memptr->last = NULL;
   }
   return(memptr);
}



int linedt(edtstr,row,col,lnth,attr,pos,control,cmndline)
char *edtstr;
int col,row,lnth,attr,*pos;
int *control;
struct cmndline_params cmndline[];
{
   int c,exit = TRUE,block_insdel = FALSE;
   int scan,inchr,sv_insert = FALSE;
	struct linedt_msg_rec msg;
	char disp_str[81];
	static char comp_str[81];
	

   if (*control & INIT_ONLY) {
		for (c = strlen(edtstr);c > 0	&& edtstr[c-1] == ' ';c--) edtstr[c-1] = 0;
		disp_str[0] = 0;
      exscrprn(strncat(disp_str,edtstr,lnth),row,col,attr);
      return;
   } else if (*control & ALLOCATE) {
      (*(char **)edtstr) = balloc();
      return;
   } else if (*control & COMP_LOAD) {
		*comp_str = 0;
      strncat(comp_str,edtstr,lnth);
      return;
   } else if (*control & COMP_CHECK) {
      return (strncmp(comp_str,edtstr,lnth) != 0);
   }
	msg.edtstr = edtstr;
	msg.col = col;
	msg.row = row;
	msg.lnth = lnth;
	msg.attr = attr;
	msg.pos = pos;
	msg.control = control;

	del_buffer[0] = 0;

	if (strlen(edtstr) > lnth) {
		strcpy(del_buffer,edtstr+(lnth));
		edtstr[lnth] = 0;
	}
	for (c = strlen(edtstr);c > 0	&& edtstr[c-1] == ' ';c--) edtstr[c-1] = 0;

   if (strlen(edtstr) != 0) exscrprn(edtstr,row,col,attr);
   if (strlen(edtstr) < *pos) *pos = strlen(edtstr);
   if (*pos >= lnth) *pos = lnth - 1;
   else if (*pos < 0) *pos = 0;
   exloccur(row,*pos + col);
   if (*control & SUPRESS_DELETE){
       block_insdel = TRUE;
		 sv_insert = insert_mode;
		 insert_mode = FALSE;
       exsetcur(7,8);
   } else if (*control & INSERT_MODE){

       insert_mode = TRUE;
   } else if (*control & TYPE_OVER) {

       insert_mode = FALSE;
   }
   if (insert_mode) exsetcur(3,8);
   else exsetcur(7,8);
/*	if (cmndline == NULL) {
		exclreol(23,0,BLACK);
		exclreol(24,0,BLACK);
	}*/
	do {
      CLEARKBD
      inchr = command_line(cmndline,&scan,&msg);
      switch (inchr) {
         case 0 : switch (scan) {
                      case RARR  : if (*pos < strlen(edtstr)) (*pos)++;
                                   else *pos = lnth + 1;
                                   break;
                      case LARR  : /*if (*pos > 0)*/ (*pos)--;
                                   break;
			              case END  : *pos = min(lnth-1,strlen(edtstr));
         			                 break;
              			  case HOME : *pos = 0;
                       				  break;
                      case DELETE :if (!read_only && !block_insdel && strlen(edtstr) > *pos) {
                                      delete(edtstr,*pos);
												  if (del_buffer[0] == 0)
	         				                 exchrstr(edtstr,' ');
												  else {
					                          exchrstr(edtstr,del_buffer[0]);
													  delete(del_buffer,0);
												  }
                                      exscrprn(edtstr,row,col,attr);
                                      /* edtstr[strlen(edtstr)-1] = 0;*/
                                   }
                                   break;
                      case INSERT : if (!block_insdel){
                                       insert_mode = !insert_mode;
                                       if (!insert_mode)
                                          exsetcur(7,8);
                                       else
                                          exsetcur(3,8);
                                    }
                                    break;
                      default     : exit = FALSE;
                                     break;
                   }
                   break;
         case TAB : scan = TAB;
         case  CR :exit = FALSE;
                   break;
         case  ESC: exit = FALSE;
                     break;
         case BACKSPACE :
                    if (*pos > 0) {
                       (*pos)--;
                       exloccur(row,*pos + col);
                       if (!read_only && !block_insdel) {
                          delete(edtstr,*pos);
								  if (del_buffer[0] == 0)
	                          exchrstr(edtstr,' ');
								  else {
	                          exchrstr(edtstr,del_buffer[0]);
									  delete(del_buffer,0);
								  }
                          exscrprn(edtstr,row,col,attr);
                          /* edtstr[strlen(edtstr)-1] = 0;*/
                       }
                    }
                    break;
         default : if (!read_only/* && (strlen(edtstr) <= (lnth - 1)) || !insert_mode)*/) {

                      if (*control&CAPITALIZE && (*pos == 0 || isspace(edtstr[*pos-1])))
                         inchr = toupper(inchr);
                      if (block_insdel || !insert_mode) {
                         if (strlen(edtstr) <= *pos)
                            exchrstr(edtstr,inchr);
                         else
                            edtstr[*pos] = inchr;
                         exscrprn(edtstr,row,col,attr);
                      }
                      else {
								 if (strlen(edtstr) > (lnth - 1)){
								 	 insert(del_buffer,0,edtstr[lnth-1]);
								    edtstr[lnth -1] = 0;
								 }
                         insert(edtstr,*pos,inchr);
                         if (*pos <= lnth) exscrprn(edtstr,row,col,attr);
                      }
                      (*pos)++;
                      if ((*control & WORD_WRAP) && *pos >= lnth/*-1*/ ) {
                         exit = FALSE;
                         scan = 0;
                      }
                   }
                   break;
      }
		if (exit) {
			if (*pos >= lnth ) {
   	      exit = FALSE;
      	   scan = ESCEOL ; /*  DARR;*/
	      }else if (*pos < 0) {
   	      exit = FALSE;
      	   scan = ESCBOL;  /* UARR;*/
	      }
		}
      exloccur(row,*pos + col);
   }while (exit);
   if ((*control & SUPRESS_DELETE) && sv_insert/* (*control & INSERT_MODE)*/)
	{
		insert_mode = TRUE;
		exsetcur(3,8);
	}

	for (c = strlen(edtstr);c > 0	&& edtstr[c-1] == ' ';c--) edtstr[c-1] = 0;
	
	return (scan);
}
