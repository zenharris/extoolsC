/*
--------------------------------------------------------------------------------
           " KNOWLEDGE OF MANY THINGS " Database System.
                  BTRIEVE  Scrolling Index System

V2.01           Complete 19 December 1988.   Beta Version.
V2.02           9 January 1989               ALL BUGS FIXED.
V2.03           12 January 1989              Amazingly Recursive.
V2.04           15 January 1989              Ambiguous Searching.
V2.05           20 January 1989              Radical Efficient Mem Management.
V2.06           24 January 1989              Data entry definitions implemented.
V2.07           25 January 1989              All memory bugs ELIMINATED!.
V2.08           11 Feb     1989              Added Next and Previous.
V2.09           01 March   1989              Added Funct. key routines to index.


--------------------------------------------------------------------------------
*/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>

#define MAXTEXTSIZE 10000

#define SCRLTOPL 0
#define FLTOPL 0
#define SCRLBLN ((scrl_consts->type == border) ? (scrl_consts->lnth-3) : (scrl_consts->lnth-1))
#define FILBLN  ((file_consts->type == border) ? (file_consts->lnth-3) : (file_consts->lnth-1))

#define scrl_window_init(wind,scrl); {wind.row = scrl->row;\
                                     wind.col = scrl->col;\
                                     wind.lnth = scrl->lnth;\
                                     wind.width = scrl->width;\
                                     wind.attr = RED;\
                                     wind.svflag = TRUE;\
                                     wind.svscr = NULL;}

#include <extools.h>
#include <texaco.h>

long last_insert_recno = 0;
void (*after_insert)(void) = NULL;
int block_transaction = FALSE;

int border_colour = RED;
int text_colour = BROWN;
int text_hilite = (RED << 4) | LIGHTGREEN;


static struct list_element *lalloc()
{
   struct list_element *memptr;
   if ((memptr =(struct list_element *) malloc(sizeof(struct list_element))) == NULL)
      gen_error("Insufficient Memory for whole Index.");
   else {
      memptr->disp_line[0] = 0;
      memptr->rec_num = 0L;
      memptr->tag = 0;
      memptr->next = NULL;
      memptr->last = NULL;
   }
   return(memptr);
}

dispose_list(list_begin)
struct list_element **list_begin;
{
   register struct list_element *temp;
/*   for (;temp = (*list_begin);free((char *)temp))
                                (*list_begin) = (*list_begin)->next;*/
   while((*list_begin) != NULL) {
      temp = (*list_begin);
      (*list_begin) = (*list_begin)->next;
      free((char *)temp);
   }
}



delete_list(del_record,scrl_consts,crnt_line)
struct list_element **del_record;
struct indexr_params *scrl_consts;
int *crnt_line;
{
    register struct list_element *temp,*temp_list;
    int c;
    temp = (*del_record);
    if ((*del_record) != NULL){
        if ((*del_record)->last != NULL) (*del_record)->last->next = (*del_record)->next;
        else scrl_consts->frst_list = (*del_record)->next;
        if ((*del_record)->next != NULL) (*del_record)->next->last = (*del_record)->last;
        else scrl_consts->last_list = (*del_record)->last;
        if ((*del_record)->last == NULL && (*del_record)->next == NULL) {
           (*del_record) = NULL;
           exclreol((*crnt_line),0,text_colour);
        } else
           if((*del_record)->next != NULL) {        /* not last entry in list*/
              (*del_record) = (*del_record)->next;
              if ((*crnt_line) < SCRLBLN){
/*                 exscrlup((*crnt_line << 8) + 0,(SCRLBLN << 8)+79,1,text_colour);*/
                 exdelln(*crnt_line);
                 temp_list = (*del_record);
                 for (c = (*crnt_line);
                      (temp_list->next != NULL) && (c != SCRLBLN);
                      c++) temp_list = temp_list->next;
                 if (c == SCRLBLN)
                     exscrprn(temp_list->disp_line,SCRLBLN,0,text_colour);
                 exscrprn((*del_record)->disp_line,(*crnt_line),0,text_hilite);
					}
              else{
                 exclreol(SCRLBLN,0,text_colour);
                 exscrprn((*del_record)->disp_line,SCRLBLN,0,text_hilite);
              }
           }
           else {                                 /* is last entry in list */
              (*del_record) = (*del_record)->last;
              exclreol((*crnt_line),0,text_colour);
              (*crnt_line)--;
              if ((*crnt_line) < SCRLTOPL) {
                 exclreol(SCRLTOPL,0,text_colour);
                 (*crnt_line) = SCRLTOPL;
              }
              exscrprn((*del_record)->disp_line,(*crnt_line),0,text_hilite);
           }
        free((char *)temp);
    }
}


static int count_entries(begin_list)
struct list_element *begin_list;
{
    register int count = 0;
    register struct list_element *temp;
    temp = begin_list;
    if (temp != NULL)
       for(count = 1;temp != NULL;count++) temp = temp->next;
    return(count);
}

static put_on_bottom(file_consts,crnt_ptr)
struct indexr_params *file_consts;
struct list_element *crnt_ptr;
{
   if (file_consts->frst_list == NULL) {
      file_consts->frst_list = crnt_ptr;
      crnt_ptr->last = NULL;
   }
   else {
      file_consts->last_list->next = crnt_ptr;
      crnt_ptr->last = file_consts->last_list;
   }
   crnt_ptr->next = NULL;
   file_consts->last_list = crnt_ptr;
}

static put_on_top(file_consts,crnt_ptr)
struct indexr_params *file_consts;
struct list_element *crnt_ptr;
{
   if (file_consts->last_list == NULL) {
      file_consts->last_list = crnt_ptr;
      crnt_ptr->next = NULL;
   }
   else {
      file_consts->frst_list->last = crnt_ptr;
      crnt_ptr->next = file_consts->frst_list;
   }
   crnt_ptr->last = NULL;
   file_consts->frst_list = crnt_ptr;
}


#define LISTSIZE 1
#define MAXLISTSIZE 80

struct list_element *read_index (keyln,comp_key,match,dir_code,file_consts,transact_entry,mode,wop_crnt)
int keyln;
char *comp_key;
int (*match)();
int dir_code;
struct indexr_params *file_consts;
void (*transact_entry)();
int mode;
long wop_crnt;
{
   struct list_element *crnt_list,*buff_list;
   int none,status,count = 0,lnth_buff = *file_consts->main_lnth;
	int k_o = (mode & KEY_ONLY);
	long test_rec;
	static char *block;
	static int once = FALSE,max_var = 20000;

	if (!once) { 
		if((block = malloc(max_var)) == NULL) gen_error("Quick Fix Memory Crisis");
		once = TRUE;
	}
	
	
	switch (dir_code) {
   case INSERT :
      dispose_list(&file_consts->frst_list);
      file_consts->frst_list = NULL;
      file_consts->last_list = NULL;


				(*file_consts->main_lnth) = 4;
				status = BTRV(B_GETPOS, file_consts->main_file,
				    &test_rec,
				    file_consts->main_lnth,
				    file_consts->key_buff,
				    file_consts->path_no);


				if (!btrv_ok(status)) {
					(*file_consts->main_lnth) = lnth_buff;
					status = BTRV((k_o)?B_GETPR+50:B_GETPR, file_consts->main_file,
				    file_consts->main_buff,
				    file_consts->main_lnth,
				    file_consts->key_buff,
				    file_consts->path_no);
					 if (!btrv_ok(status)){
						(*file_consts->main_lnth) = lnth_buff;
						status = BTRV((k_o)?B_GETFIRST+50:B_GETFIRST, file_consts->main_file,
				  		  file_consts->main_buff,
				  		  file_consts->main_lnth,
				  		  file_consts->key_buff,
				  		  file_consts->path_no);
						if(!btrv_ok(status)) {
							return(NULL);
						}

					 }	
					(*file_consts->main_lnth) = 4;
					status = BTRV(B_GETPOS, file_consts->main_file,
					    &test_rec,
					    file_consts->main_lnth,
					    file_consts->key_buff,
					    file_consts->path_no);
						 if (!btrv_ok(status)) file_error(status);
				}

				(*file_consts->main_lnth) = lnth_buff;
				memcpy(file_consts->main_buff, &test_rec, 4);
				status = BTRV(B_GETDRC, file_consts->main_file,
				    file_consts->main_buff,
				    file_consts->main_lnth,
				    file_consts->key_buff,
				    file_consts->path_no);

				if (status != 0 && status != 22) {
					file_error(status);
					return(NULL);
				}
		
		
		do {
        (*file_consts->main_lnth) = lnth_buff;
         if ((crnt_list = lalloc()) != NULL) {
				*file_consts->main_lnth = 4;
            status = BTRV(B_GETPOS,file_consts->main_file,
                                   &crnt_list->rec_num,
                                   file_consts->main_lnth,
                                   file_consts->key_buff,
                                   file_consts->path_no);
				if (status != 0 && status != 22) {
					file_error(status);
					free(crnt_list);
					return(NULL);
				}
				(*transact_entry)(crnt_list, file_consts->main_buff);
            put_on_top(file_consts,crnt_list);
            (*file_consts->main_lnth) = lnth_buff;
         }else break;
         (*file_consts->main_lnth) = lnth_buff;
         status = BTRV((k_o)?B_GETPR+50:B_GETPR,file_consts->main_file,
                                       file_consts->main_buff,
                                       file_consts->main_lnth,
                                       file_consts->key_buff,
                                       file_consts->path_no);
      }while(count++ < (FILBLN - FLTOPL) / 2 && (*match)(comp_key) && (status == 22 || status == 0));
      if (status != 0 && status != 22 && status != 9) file_error(status);

      buff_list = file_consts->last_list;
      (*file_consts->main_lnth) = (block != NULL)?max_var:lnth_buff;
      memcpy((block != NULL)?block:file_consts->main_buff,&file_consts->last_list->rec_num,4);
      if((status = BTRV(B_GETDRC,file_consts->main_file,
                                 (block != NULL)?block:file_consts->main_buff,
                                 file_consts->main_lnth,
                                 file_consts->key_buff,
                                 file_consts->path_no))
                                 != 0 && status != 22) file_error(status);
      (*file_consts->main_lnth) = lnth_buff;

      while((status = BTRV((k_o)?B_GETNX+50:B_GETNX,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no))
                                    !=11
                                    && count++ < (FILBLN - FLTOPL) + 1
                                    && (*match)(comp_key)
                                    && (status == 0 || status == 22))    {
         if ((crnt_list = lalloc()) != NULL) {
            (*transact_entry)(crnt_list,file_consts->main_buff);
				*file_consts->main_lnth = 4;
            status = BTRV(B_GETPOS,file_consts->main_file,
                                   &crnt_list->rec_num,
                                   file_consts->main_lnth,
                                   file_consts->key_buff,
                                   file_consts->path_no);
            if (status != 0 && status != 22) file_error(status);
            put_on_bottom(file_consts,crnt_list);
            (*file_consts->main_lnth) = lnth_buff;
         }else break;
      } /*while (getpr)*/
      if (status != 0 && status != 22 && status != 9) file_error(status);
      return (buff_list);
/*      break; */
   case END :           /* End of Index Function  */
      dispose_list(&file_consts->frst_list);
         file_consts->frst_list = NULL;
         file_consts->last_list = NULL;
      if (comp_key == NULL || keyln == 0)  {
           (*file_consts->main_lnth) = lnth_buff;
           status = BTRV((k_o)?B_GETLAST+50:B_GETLAST ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           if (status == 22 || status == 0)
               none = FALSE;
           else
               none = TRUE;
      }
      else {
           memcpy(file_consts->key_buff,comp_key,keyln);
           (*file_consts->main_lnth) = lnth_buff;
           status = BTRV((k_o)?B_GETLT+50:B_GETLT   ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           if(status == 9)	{
              (*file_consts->main_lnth) = lnth_buff;

           status = BTRV((k_o)?B_GETFIRST+50:B_GETFIRST ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           } else {
              (*file_consts->main_lnth) = lnth_buff;
           status = BTRV((k_o)?B_GETNX+50:B_GETNX   ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
			  }
           while ((status == 22 || status == 0) && (*match)(comp_key))
              status = BTRV(B_GETNX + 50,file_consts->main_file,  /* this +50 stays */
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           if(status == 9){
              (*file_consts->main_lnth) = lnth_buff;
              status = BTRV((k_o)?B_GETLAST+50:B_GETLAST ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           }else {
              (*file_consts->main_lnth) = lnth_buff;
              status = BTRV((k_o)?B_GETPR+50:B_GETPR ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           }
           if ((status == 22 || status == 0) && (*match)(comp_key))
                  none = FALSE;
              else
                  none = TRUE;
      }
      if (!none) {
         file_consts->frst_list = NULL;
         file_consts->last_list = NULL;
         do {
            count++;
            if ((crnt_list = lalloc()) != NULL) {
               (*transact_entry)(crnt_list,file_consts->main_buff);
/*               (*file_consts->main_lnth) = lnth_buff;*/
					*file_consts->main_lnth = 4;
               status = BTRV(B_GETPOS,file_consts->main_file,
                                      &crnt_list->rec_num,
                                      file_consts->main_lnth,
                                      file_consts->key_buff,
                                      file_consts->path_no);
               if (status != 0 && status != 22) file_error(status);
               put_on_top(file_consts,crnt_list);
            }else break;
            (*file_consts->main_lnth) = lnth_buff;
            status = BTRV((k_o)?B_GETPR+50:B_GETPR,file_consts->main_file,
                                       file_consts->main_buff,
                                       file_consts->main_lnth,
                                       file_consts->key_buff,
                                       file_consts->path_no);
         }while(count <= (FILBLN - FLTOPL) && (*match)(comp_key) && (status ==0 || status == 22));
         if (status != 0 && status != 22 && status != 9) file_error(status);
      }
      else if (status != 9 && status != 22 && status != 0) file_error(status);
      return(file_consts->last_list);
/*      break;*/
   case HOME : dispose_list(&file_consts->frst_list);
   case 0x00 :           /* No index exists. Read first List. */
         file_consts->frst_list = NULL;
         file_consts->last_list = NULL;
      if (comp_key == NULL || keyln == 0){
           (*file_consts->main_lnth) = lnth_buff;
           status = BTRV((k_o)?B_GETFIRST+50:B_GETFIRST,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           if (status == 22 || status == 0)
               none = FALSE;
           else
               none = TRUE;
      }
      else{
           memcpy(file_consts->key_buff,comp_key,keyln);
           (*file_consts->main_lnth) = lnth_buff;
           status = BTRV((k_o)?B_GETLT+50:B_GETLT   ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);

           if (!btrv_ok(status) && status != 9) file_error(status);

           if (status == 9) {
              (*file_consts->main_lnth) = lnth_buff;

          	 status = BTRV((k_o)?B_GETFIRST+50:B_GETFIRST,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           } else	{
              (*file_consts->main_lnth) = lnth_buff;

           		status = BTRV((k_o)?B_GETNX+50:B_GETNX    ,file_consts->main_file,
                                    file_consts->main_buff,
                                    file_consts->main_lnth,
                                    file_consts->key_buff,
                                    file_consts->path_no);
           }
				if (!btrv_ok(status) && status != 9) file_error(status);

           if (status !=9 && (*match)(comp_key))
                  none = FALSE;
              else
                  none = TRUE;
      }
      if (!none) {
         file_consts->frst_list = NULL;
         file_consts->last_list = NULL;
         do {
            count++;
            if ((crnt_list = lalloc()) != NULL) {
               (*transact_entry)(crnt_list,file_consts->main_buff);
					*(file_consts->main_lnth) = 4;
               status = BTRV(B_GETPOS,file_consts->main_file,
                                      &crnt_list->rec_num,
                                      file_consts->main_lnth,
                                      file_consts->key_buff,
                                      file_consts->path_no);
               if (status != 0 && status != 22) file_error(status);
               put_on_bottom(file_consts,crnt_list);
               (*file_consts->main_lnth) = lnth_buff;
            }else break;
            (*file_consts->main_lnth) = lnth_buff;
            status = BTRV((k_o)?B_GETNX+50:B_GETNX,file_consts->main_file,
                                       file_consts->main_buff,
                                       file_consts->main_lnth,
                                       file_consts->key_buff,
                                       file_consts->path_no);
         }while(count <= (FILBLN - FLTOPL) && (*match)(comp_key) && (status ==0 || status == 22));
         if (status != 0 && status != 22 && status != 9) file_error(status);
      }
      else if (status != 9 && status != 22 && status != 0) file_error(status);
      return(file_consts->frst_list);
/*      break;*/
   case DARR :  /* At end of a list. Read Next List */

	
		*(file_consts->main_lnth) = 4;
      status = BTRV(B_GETPOS,file_consts->main_file,
                                   &test_rec,
                                   file_consts->main_lnth,
                                   file_consts->key_buff,file_consts->path_no);
      if (status != 0 && status != 22 && status != 8) file_error(status);
	
		if (status == 8 || test_rec != file_consts->last_list->rec_num) {
			*file_consts->main_lnth = (block != NULL)?max_var:lnth_buff;

   	   memcpy((block != NULL)?block:file_consts->main_buff,&file_consts->last_list->rec_num,4);
      	status = BTRV(B_GETDRC,file_consts->main_file,
                                 (block != NULL)?block:file_consts->main_buff,
                                 file_consts->main_lnth,
                                 file_consts->key_buff,
   	                           file_consts->path_no);
			if (!btrv_ok(status)) file_error(status);
		}

		*file_consts->main_lnth = lnth_buff;
		status=BTRV((k_o)?B_GETNX+50:B_GETNX,file_consts->main_file,
		                     file_consts->main_buff,
		                     file_consts->main_lnth,
		                     file_consts->key_buff,
		                     file_consts->path_no);
									
		for (count = 0; count<LISTSIZE && (*match)(comp_key) && (status==0||status==22); count++) {
         *(file_consts->main_lnth) = lnth_buff;
         if ((crnt_list = lalloc()) != NULL) {
            if (count_entries(file_consts->frst_list) >= MAXLISTSIZE) {
               buff_list = file_consts->frst_list;
               file_consts->frst_list = file_consts->frst_list->next;
               file_consts->frst_list->last = NULL;
               free((char *)buff_list);
            }
            (*transact_entry)(crnt_list,file_consts->main_buff);
				*(file_consts->main_lnth) = 4;
            status = BTRV(B_GETPOS,file_consts->main_file,
                                   &crnt_list->rec_num,
                                   file_consts->main_lnth,
                                   file_consts->key_buff,file_consts->path_no);
            if (status != 0 && status != 22) file_error(status);
            put_on_bottom(file_consts,crnt_list);
            *file_consts->main_lnth = lnth_buff;
         }else break;
         *(file_consts->main_lnth) = lnth_buff;
			if (count < LISTSIZE-1)
				status=BTRV((k_o)?B_GETNX+50:B_GETNX,file_consts->main_file,
		                     file_consts->main_buff,
		                     file_consts->main_lnth,
		                     file_consts->key_buff,
		                     file_consts->path_no);
      }
      if (status != 0 && status != 22 && status != 9) file_error(status);
      break;
   case UARR :   /* At Begining of list */
		*file_consts->main_lnth = 4;
      status = BTRV(B_GETPOS,file_consts->main_file,
                                   &test_rec,
                                   file_consts->main_lnth,
                                   file_consts->key_buff,file_consts->path_no);
      if (status != 0 && status != 22 && status != 8) file_error(status);

	
		if (status == 8 || test_rec != file_consts->frst_list->rec_num) {

	      *file_consts->main_lnth = (block != NULL)?max_var:lnth_buff;
   	   memcpy((block != NULL)?block:file_consts->main_buff,&(file_consts->frst_list->rec_num),4);
      	status = BTRV(B_GETDRC,file_consts->main_file,
                                 (block != NULL)?block:file_consts->main_buff,				 
                                 file_consts->main_lnth,
                                 file_consts->key_buff,
                                 file_consts->path_no);
			if (!btrv_ok(status)) file_error(status);
		}
		*file_consts->main_lnth = lnth_buff;
		status=BTRV((k_o)?B_GETPR+50:B_GETPR,file_consts->main_file,
		                     file_consts->main_buff,
		                     file_consts->main_lnth,
		                     file_consts->key_buff,
		                     file_consts->path_no);
      for (count = 0;count<LISTSIZE	&& (*match)(comp_key) && (status==0||status==22);count++) {
         *file_consts->main_lnth = lnth_buff;
         if ((crnt_list = lalloc()) != NULL) {
            if (count_entries(file_consts->frst_list) >= MAXLISTSIZE) {
               buff_list = file_consts->last_list;
               file_consts->last_list = file_consts->last_list->last;
               file_consts->last_list->next = NULL;
               free((char *)buff_list);
            }
            (*transact_entry)(crnt_list,file_consts->main_buff);
				*file_consts->main_lnth = 4;
            status = BTRV(B_GETPOS,file_consts->main_file,
                                   &crnt_list->rec_num,
                                   file_consts->main_lnth,
                                   file_consts->key_buff,
                                   file_consts->path_no);
            if (status != 0 && status != 22) file_error(status);
            put_on_top(file_consts,crnt_list);
            *file_consts->main_lnth = lnth_buff;
         }else break;
			*file_consts->main_lnth = lnth_buff;
			if (count < LISTSIZE-1)
 				status=BTRV((k_o)?B_GETPR+50:B_GETPR,file_consts->main_file,
		                     file_consts->main_buff,
		                     file_consts->main_lnth,
		                     file_consts->key_buff,
		                     file_consts->path_no);
      }
      if (status != 0 && status != 22 && status != 9) file_error(status);
      break;
   }
}

static void *lmalloc(size_t amnt)
{
   struct line_rec *temp_ptr = NULL;
   if ((temp_ptr = malloc(amnt + 8)) != NULL) {
      memset (temp_ptr->storage,0,amnt);
      temp_ptr->next = NULL;
      temp_ptr->last = NULL;
   }
   return (temp_ptr);
}


static disassemble(crnt_rec,scrl_consts,v_lnth,v_ptr)
long crnt_rec;
struct indexr_params *scrl_consts;
int v_lnth;
struct line_rec **v_ptr;
{
   struct line_rec *crnt_list = NULL,*temp_list = NULL;
   unsigned int main_lnth,status;
   char *block;
   struct line_rec *balloc();
   for(main_lnth = *scrl_consts->main_lnth + (((unsigned)MAXTEXTSIZE / v_lnth) * v_lnth);
       main_lnth > *scrl_consts->main_lnth
       && (block = malloc(main_lnth)) == NULL;
       main_lnth -= v_lnth);
   if (block != NULL) {
      memcpy(block,&crnt_rec,4);
      status = BTRV(B_GETDRC+200,scrl_consts->main_file,block,
                                  &main_lnth,
                                  scrl_consts->key_buff,
                                  scrl_consts->path_no);
		if (status == 84 || status == 85) {
			gen_error("Record is In Use and Locked");
			free(block);
			return(FALSE);
		}
      if (status == 43){
         free(block);
         return(FALSE);
      }
      if (status == 0 || status == 22) {
         if(status == 22)
             exgenerr(__FILE__,__LINE__,"Failure to allocate memory.",
                                        "(Some Text Not Read)",
                                        "DO  NOT  MODIFY  RECORD !!",NULL);
/*         else
             block = realloc(block,main_lnth);*/
         memcpy(scrl_consts->main_buff,block,(*scrl_consts->main_lnth));
         main_lnth = (main_lnth > v_lnth) ? main_lnth - v_lnth : 0;
         while (main_lnth >= *scrl_consts->main_lnth) {
            if ((crnt_list = lmalloc(v_lnth)) != NULL){
               memcpy(crnt_list->storage,block + main_lnth,v_lnth);
/*               block = realloc(block,main_lnth);*/
               main_lnth = (main_lnth > v_lnth) ? main_lnth - v_lnth : 0;
               if (temp_list == NULL)
                  crnt_list->next = NULL;
               else {
                  temp_list->last = crnt_list;
                  crnt_list->next = temp_list;
               }
               temp_list = crnt_list;
               temp_list->last = NULL;
               (*v_ptr) = crnt_list;
            }
            else{
               exgenerr(__FILE__,__LINE__,"Failure to allocate memory.",
                                          "(Some Text Not Read)",
                                          "DO  NOT  MODIFY  RECORD !!",NULL);
               break;
            }
         }
      } else file_error(status);
      free(block);
   }else {
      exgenerr(__FILE__,__LINE__,"Failure to allocate memory.",
                                 "     (NO Text Read!)",
                                 "DO  NOT  MODIFY  RECORD !!",NULL);
      (*v_ptr) = NULL;
      memcpy(scrl_consts->main_buff,&crnt_rec,4);
      if ((status = BTRV(B_GETDRC,scrl_consts->main_file,
                                  scrl_consts->main_buff,
                                  scrl_consts->main_lnth,
                                  scrl_consts->key_buff,
                                  scrl_consts->path_no))
                                  != 0 && status != 22)
        file_error(status);
   }
   return(TRUE);
}

static dispose_text(text_start)
struct line_rec **text_start;
{
    register struct line_rec *temp;
      if ((*text_start) != NULL) while ((*text_start)->last != NULL)
                                     (*text_start) = (*text_start)->last;
/*      for (;temp = (*text_start);free((char *)temp)) (*text_start) = (*text_start)->next;*/
      while ((*text_start) != NULL){
         temp = (*text_start);
         (*text_start) = (*text_start)->next;
         free((char *)temp);
      }
}


static int count_lines(text_start)
struct line_rec **text_start;
{
   register struct line_rec *temp;
   register int count = 0;
   if ((*text_start) != NULL){
      while((*text_start)->last != NULL) (*text_start) = (*text_start)->last;
      for (temp = (*text_start);temp != NULL;count++) temp = temp->next;
   }
   return(count);
}


assemble(block,scrl_consts,v_lnth,v_ptr)
char **block;
struct indexr_params *scrl_consts;
int v_lnth;
struct line_rec **v_ptr;
{
   register struct line_rec *crnt_line;
   unsigned int mainsz,textsz,offset = 0;
   int  tmp_file;
   mainsz = *scrl_consts->main_lnth;
   textsz = count_lines(v_ptr) ;
   crnt_line = (*v_ptr);
   textsz *= v_lnth;
   if(((*block) = malloc(mainsz + textsz)) != NULL) {
      *scrl_consts->main_lnth = mainsz + textsz;
      memcpy((*block),scrl_consts->main_buff,mainsz);
      while(crnt_line != NULL) {
         memcpy(((*block) + mainsz + offset),crnt_line->storage,v_lnth);
         offset += v_lnth;
         crnt_line = crnt_line->next;
      }
   }
   else {
     if ((tmp_file = open("TEXTSAVE.$$$",O_TRUNC|O_BINARY|O_RDWR)) != -1){
        while(crnt_line != NULL && write(tmp_file,crnt_line->storage,v_lnth) == v_lnth){
           crnt_line = crnt_line->next;
        }
        if(crnt_line == NULL){
           lseek(tmp_file,0L,0);
           *scrl_consts->main_lnth = mainsz + textsz;
           if(((*block) = malloc(*scrl_consts->main_lnth))==NULL){
             if (!verify(2)){
               for(*scrl_consts->main_lnth = mainsz + textsz;
                   *scrl_consts->main_lnth > mainsz
                   && ((*block) = malloc(*scrl_consts->main_lnth)) == NULL;
                   *scrl_consts->main_lnth -= v_lnth);
               if((*block) == NULL) {
                  exgenerr(__FILE__,__LINE__,"","Failure to allocate memory.",
                                                "Save Cancelled !!",NULL);
                  close(tmp_file);
                  remove("TEXTSAVE.$$$");
                  return;
               }
             }else{
                close(tmp_file);
                remove("TEXTSAVE.$$$");
                return;
             }
           }
           memcpy((*block),scrl_consts->main_buff,mainsz);
           read(tmp_file,((*block) + mainsz),*scrl_consts->main_lnth - mainsz);
        }else exgenerr(__FILE__,__LINE__,"","DOS File Handling Error.",
                                            "Save Cancelled !!",NULL);
        close(tmp_file);
        remove("TEXTSAVE.$$$");
     }else exgenerr(__FILE__,__LINE__,"","Failure to allocate memory.",
                                         "Save Cancelled !!",NULL);

   }
}

static init_scrllst(scrl_consts,crnt_line,crnt_list)
struct indexr_params *scrl_consts;
int *crnt_line;
struct list_element *crnt_list;
{
   int temp,temp1,ofset = 0;
   char mactest[10];
   struct list_element *crnt_buff = crnt_list;
   exloccur (24,0);
   cls();
   if (crnt_list != NULL) {
      *crnt_line = SCRLTOPL;
      temp = scrl_consts->lnth - 2;
      temp = temp / 2;
      for (temp1 =1;temp1 < temp;temp1++)
         if (crnt_list->last != NULL){
            crnt_list = crnt_list->last;
            ofset++;
         }
      while ((crnt_list != NULL) && (*crnt_line <= SCRLBLN)) {
         exscrprn(crnt_list->disp_line,*crnt_line,0,text_colour);
         (*crnt_line)++;
         crnt_list = crnt_list->next;
      }
      *crnt_line = SCRLTOPL + ofset;
      exscrprn(crnt_buff->disp_line,*crnt_line,0,text_hilite);
   }
}



unsigned int select_rec(rec_num,keyln,comp_key,match,scrl_consts,transact_entry,heading,index_cmndline,mode,strategy)
unsigned long *rec_num;
int keyln;
char *comp_key;
int (*match)();
struct indexr_params *scrl_consts;
void (*transact_entry)();
char *heading;
struct cmndline_params index_cmndline[];
int mode;
void (*strategy)();
{
   int crnt_line = 0,pos_chng = FALSE,local_keystroke = TRUE,inchr,scan,c,status;
   unsigned int lnth_buff = (*scrl_consts->main_lnth);
   struct list_element *crnt_list= NULL;
   struct scrllst_msg_rec scrllst_msg;
   long rec_buff;

   memset (&scrllst_msg,0,sizeof(scrllst_msg));
   scrllst_msg.crnt_list = &crnt_list;
   scrllst_msg.pos_chngd = &pos_chng;
   scrllst_msg.crnt_line = &crnt_line;
   scrllst_msg.scrl_consts = scrl_consts;


   open_window(scrl_consts->row,scrl_consts->col,scrl_consts->width,
               scrl_consts->lnth,text_colour,scrl_consts->type);

   exscrprn(heading,-1,max(0,(crnt_window->width/2)-(strlen(heading)/2)),border_colour);


   crnt_list = read_index(keyln,comp_key,match,0x00,scrl_consts,transact_entry,mode);
   init_scrllst(scrl_consts,&crnt_line,crnt_list);
   do {
      if (crnt_list != NULL) exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
      if (index_cmndline != NULL)
         inchr = command_line(index_cmndline,&scan,&scrllst_msg);
      else inchr = exinkey(&scan);
      switch (inchr){
         case 00: switch(scan) {
                     case PGDN :for (c = 0;crnt_list != NULL && c < (SCRLBLN-SCRLTOPL);c++ ){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->next == NULL)
                                       read_index(keyln,comp_key,match,DARR,scrl_consts,transact_entry,mode);
                                    if (crnt_list->next != NULL) {
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                        crnt_list = crnt_list->next;
                                        crnt_line++;
                                        if (crnt_line > SCRLBLN) {
                                            exscrlupw();
                                            crnt_line = SCRLBLN;
                                        }
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     case END : if (crnt_list != NULL) {
/*                                    exscrldn((SCRLTOPL << 8) + 0,(SCRLBLN << 8) +79,0,BLACK);*/
                                    cls();
									        *scrl_consts->main_lnth = lnth_buff;
                                    crnt_list = read_index(keyln,comp_key,match,END,scrl_consts,transact_entry,mode);
                                    init_scrllst(scrl_consts,&crnt_line,crnt_list);
                                 }
                                 break;

                     case PGUP : for (c = 0;crnt_list != NULL && c < (SCRLBLN-SCRLTOPL);c++){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->last == NULL)
                                       read_index(keyln,comp_key,match,UARR,scrl_consts,transact_entry,mode);
                                    if (crnt_list->last != NULL) {
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                       crnt_list = crnt_list->last;
                                       crnt_line--;
                                       if (crnt_line < SCRLTOPL) {
                                          exscrldnw();
                                          crnt_line = SCRLTOPL;
                                       }
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     case HOME : if (crnt_list != NULL){
                                    cls();
									        *scrl_consts->main_lnth = lnth_buff;
                                    crnt_list = read_index(keyln,comp_key,match,HOME,scrl_consts,transact_entry,mode);
                                    init_scrllst(scrl_consts,&crnt_line,crnt_list);
                                 }
                                 break;

                     case DARR : if (crnt_list != NULL){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->next == NULL)
                                       read_index(keyln,comp_key,match,DARR,scrl_consts,transact_entry,mode,crnt_list);
                                    if (crnt_list->next != NULL) {
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                        crnt_list = crnt_list->next;
                                        crnt_line++;
                                        if (crnt_line > SCRLBLN) {
                                            exscrlupw();
                                            crnt_line = SCRLBLN;
                                        }
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     case UARR : if (crnt_list != NULL){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->last == NULL)
                                       read_index(keyln,comp_key,match,UARR,scrl_consts,transact_entry,mode,crnt_list);
                                    if (crnt_list->last != NULL) {
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                       crnt_list = crnt_list->last;
                                       crnt_line--;
                                       if (crnt_line < SCRLTOPL) {
                                          exscrldnw();
                                          crnt_line = SCRLTOPL;
                                       }
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     default : if (strategy != NULL)
                                 (*strategy)(inchr,scan,&pos_chng,crnt_list);
											break;
                  }/* end switch scan */
         break;  /*end case 0*/
			case 13 : local_keystroke = FALSE;
						break;
         case ESC : local_keystroke = FALSE;
                    break;
         default : if (strategy != NULL)
	                  (*strategy)(inchr,scan,&pos_chng,crnt_list);
			
			
			/*if (index_cmndline == NULL)*/
                   break;
      } /*end switch inchr*/
      if(pos_chng) {
/*         *scrl_consts->main_lnth = lnth_buff;
			*scrl_consts->main_lnth = 4;
         status = BTRV(B_GETPOS,scrl_consts->main_file,
                                &rec_buff,
                                scrl_consts->main_lnth,
                                scrl_consts->key_buff,
                                scrl_consts->path_no);
         if (status != 0 && status != 22) file_error(status);
         *scrl_consts->main_lnth = lnth_buff;
         memcpy(scrl_consts->main_buff,&rec_buff,4);
         status = BTRV(B_GETDRC,scrl_consts->main_file,
                                      scrl_consts->main_buff,
                                      scrl_consts->main_lnth,
                                      scrl_consts->key_buff,
                                      scrl_consts->path_no);
         if (status != 0 && status != 22)  file_error(status);

*/
         crnt_list = read_index(keyln,comp_key,match,INSERT,scrl_consts,transact_entry,mode);
         init_scrllst(scrl_consts,&crnt_line,crnt_list);
         pos_chng = FALSE;
      }


   } while (local_keystroke);
   *rec_num = crnt_list->rec_num;
   dispose_list(&scrl_consts->frst_list);
   scrl_consts->frst_list = NULL;
   scrl_consts->last_list = NULL;
   close_window();
   return ((inchr<<8)|scan);
}


#define MAX_DEL 65535

display_list (keyln,comp_key,match,scrl_consts,transact_entry,strategy,
              input_scrn,scrn_lnth,v_lnth,v_ptr,savescrn,heading,
              set_defaults,strategy_1,index_cmndline,edit_cmndline,mode)
int keyln;
char *comp_key;
int (*match)();
struct indexr_params *scrl_consts;
void (*transact_entry)();
int (*strategy)();
struct scr_params input_scrn[];
int scrn_lnth;
int v_lnth;
struct line_rec **v_ptr;
int savescrn;
char *heading;
void (*set_defaults)();
void (*strategy_1)();
struct cmndline_params index_cmndline[];
struct cmndline_params edit_cmndline[];
int mode;
{
   static  char *get_out[] = {
      "  Exit To System  ",
      "      Cancel      "
   };

   int dummy_lnth,edited = FALSE,pos_chng = FALSE,scan,inchr,blocked = FALSE;
   int crnt_line = 0,c,status,clr = FALSE,lnth_buff = *scrl_consts->main_lnth;
	char exit1 = FALSE,exit2 = FALSE,*temp_block,*del_block;
	unsigned max_block;
   struct list_element *crnt_list,*temp_list,*dummy_list;
   long rec_num,test_rec;
   struct scrllst_msg_rec scrllst_msg;
	int save_ro = read_only;

	int iter,pglen,cnter;
   struct line_rec *incptr;


	read_only = mode & READ_ONLY;

   memset (&scrllst_msg,0,sizeof(scrllst_msg));
   scrllst_msg.crnt_list = &crnt_list;
   scrllst_msg.pos_chngd = &pos_chng;
   scrllst_msg.crnt_line = &crnt_line;
   scrllst_msg.scrl_consts = scrl_consts;

   open_window(scrl_consts->row,scrl_consts->col,scrl_consts->width,
               scrl_consts->lnth,text_colour,scrl_consts->type);

   exscrprn(heading,-1,max(0,(crnt_window->width/2)-(strlen(heading)/2)),border_colour);

   *scrl_consts->main_lnth = lnth_buff;
   crnt_list = read_index(keyln,comp_key,match,0x00,scrl_consts,transact_entry,mode);
   init_scrllst(scrl_consts,&crnt_line,crnt_list);

   do {
      if (crnt_list != NULL) exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
      switch (inchr = command_line(index_cmndline,&scan,&scrllst_msg))
         {
         case 00: switch(scan) {
                     case PGDN :for (c = 0;crnt_list != NULL && c < (SCRLBLN-SCRLTOPL);c++ ){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->next == NULL)
                                       read_index(keyln,comp_key,match,DARR,scrl_consts,transact_entry,mode);
                                    if (crnt_list->next != NULL) {
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                        crnt_list = crnt_list->next;
                                        crnt_line++;
                                        if (crnt_line > SCRLBLN) {
                                            exscrlupw();
                                            crnt_line = SCRLBLN;
                                        }
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     case END : if (crnt_list != NULL) {
/*                                    exscrldn((SCRLTOPL << 8) + 0,(SCRLBLN << 8) +79,0,BLACK);*/
                                    cls();
									        *scrl_consts->main_lnth = lnth_buff;
                                    crnt_list = read_index(keyln,comp_key,match,END,scrl_consts,transact_entry,mode);
                                    init_scrllst(scrl_consts,&crnt_line,crnt_list);
                                 }
                                 break;

                     case PGUP : for (c = 0;crnt_list != NULL && c < (SCRLBLN-SCRLTOPL);c++){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->last == NULL)
                                       read_index(keyln,comp_key,match,UARR,scrl_consts,transact_entry,mode);
                                    if (crnt_list->last != NULL) {
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                       crnt_list = crnt_list->last;
                                       crnt_line--;
                                       if (crnt_line < SCRLTOPL) {
                                          exscrldnw();
                                          crnt_line = SCRLTOPL;
                                       }
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     case HOME : if (crnt_list != NULL){
                                    cls();
									        *scrl_consts->main_lnth = lnth_buff;
                                    crnt_list = read_index(keyln,comp_key,match,HOME,scrl_consts,transact_entry,mode);
                                    init_scrllst(scrl_consts,&crnt_line,crnt_list);
                                 }
                                 break;

                     case DARR : if (crnt_list != NULL){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->next == NULL) {
													read_index(keyln,comp_key,match,DARR,scrl_consts,transact_entry,mode,crnt_list->rec_num);
												}
                                    if (crnt_list->next != NULL) {
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                        crnt_list = crnt_list->next;
                                        crnt_line++;
                                        if (crnt_line > SCRLBLN) {
                                            exscrlupw();
                                            crnt_line = SCRLBLN;
                                        }
                                        exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                     case UARR : if (crnt_list != NULL){
									        *scrl_consts->main_lnth = lnth_buff;
                                    if (crnt_list->last == NULL) {
													read_index(keyln,comp_key,match,UARR,scrl_consts,transact_entry,mode,crnt_list->rec_num);
												}
                                    if (crnt_list->last != NULL) {
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_colour);
                                       crnt_list = crnt_list->last;
                                       crnt_line--;
                                       if (crnt_line < SCRLTOPL) {
                                          exscrldnw();
                                          crnt_line = SCRLTOPL;
                                       }
                                       exscrprn(crnt_list->disp_line,crnt_line,0,text_hilite);
                                    }
                                 }
                                 break;
                      case DELETE : if(!read_only && verify(3)) {

									for(max_block = MAX_DEL,del_block = NULL ;
									(del_block = malloc(max_block)) == NULL && max_block >= lnth_buff;
								   max_block -= 1024);
								   if (del_block == NULL) { gen_error("No Memory");break; } 
							 
											 memcpy(del_block /*scrl_consts->main_buff */,
                                               &crnt_list->rec_num,4);
                                  *scrl_consts->main_lnth = max_block;
                                  status = BTRV(B_GETDRC+200,
                                                     scrl_consts->main_file,
                                                     del_block /*scrl_consts->main_buff*/,
                                                     scrl_consts->main_lnth,
                                                     scrl_consts->key_buff,
                                                     scrl_consts->path_no);

											if (status == 22) {
												gen_error ("Not Enough Memory");
												free (del_block);
												break;
											}

											if (status == 84 || status == 85) {
												gen_error("Record is In Use and Locked");
												free (del_block);
												break;
											}
                                  if (status == 43) {
                                     gen_error("Could Not Find Record");
												free (del_block);
                                     break;
                                  }
                                  if (status == 0) {
											 	 memcpy(scrl_consts->main_buff,del_block,lnth_buff);
                                     if ((*strategy)(X_DELETE,&pos_chng,crnt_list)){
/*                                        *scrl_consts->main_lnth = lnth_buff;*/
                                        status = BTRV(B_DEL,scrl_consts->main_file,
                                                            del_block /*scrl_consts->main_buff*/,
                                                            scrl_consts->main_lnth,
                                                            scrl_consts->key_buff,
                                                            scrl_consts->path_no);
                                        if (status != 0 ) file_error(status);
                                        else delete_list(&crnt_list,scrl_consts,
                                                                    &crnt_line);
                                     }
                                     *scrl_consts->main_lnth = lnth_buff;
                                  }
                                  else file_error(status);
										  	 free (del_block);
                                }
                                break;
                      case INSERT :	if (!read_only) {
                                open_window(1,0,80,22,text_colour,scrl_consts->type);
                                clr = exit2 = pos_chng = edited = FALSE;
                                exloccur(24,0);
/*                                (*set_defaults)();  */
                                memset(scrl_consts->main_buff,0,lnth_buff);
                                init_screen(input_scrn,scrn_lnth);
                                do {
                                    switch (command_line(edit_cmndline,&scan,&scrllst_msg)) {
                                       case 0 : switch (scan) {
                                          case F1 : if (!block_transaction) {
                                                       status = BTRV(B_BEGIN);
                                                       if (!btrv_ok(status)) file_error(status);
                                                       block_transaction = blocked = TRUE;
                                                    }
                                                    cls();
                                                    (*set_defaults)();
                                                    if (clr){
                                                       clr = FALSE;
                                                       
                                                       if (v_ptr != NULL) dispose_text(v_ptr);
                                                    }
                                                    init_screen(input_scrn,scrn_lnth);
                                                    enter_screen(input_scrn,scrn_lnth,&edited,nobreakout);
                                                    exloccur(24,0);
                                                    if (edited && verify(1)) {
                                                       (*strategy)(F1,&pos_chng,crnt_list);
                                                       if(v_lnth != 0) {
                                                          assemble(&temp_block,scrl_consts,v_lnth,v_ptr);
                                                          if (temp_block != NULL) {
                                                             status = BTRV(B_INS,scrl_consts->main_file,
                                                                              temp_block,
                                                                              scrl_consts->main_lnth,
                                                                              scrl_consts->key_buff,
                                                                              scrl_consts->path_no);
                                                             if (status != 0) file_error(status);
                                                             free(temp_block);
                                                          }
                                                       }else {
                                                          *scrl_consts->main_lnth = lnth_buff;
                                                          status = BTRV(B_INS,scrl_consts->main_file,
                                                                              scrl_consts->main_buff,
                                                                              scrl_consts->main_lnth,
                                                                              scrl_consts->key_buff,
                                                                              scrl_consts->path_no);
                                                          if (status != 0) file_error(status);
                                                       }
                                                       if (status == 0) {
																			*scrl_consts->main_lnth = 4;
                                                          status = BTRV(B_GETPOS,scrl_consts->main_file,
                                                                                 &last_insert_recno,
                                                                                 scrl_consts->main_lnth,
                                                                                 scrl_consts->key_buff,
                                                                                 scrl_consts->path_no);
                                                          if (status != 0) file_error(status);
                                                          if (after_insert != NULL) (*after_insert)();
							                                     (*strategy)(X_INSERT,&pos_chng,crnt_list);
                                                       }
                                                       *scrl_consts->main_lnth = lnth_buff;
                                                       clr = pos_chng = TRUE;
                                                       edited = FALSE;
                                                       if (block_transaction && blocked) {
                                                          status = BTRV(B_END);
                                                          if (!btrv_ok(status)) file_error(status);
                                                          block_transaction = blocked = FALSE;
                                                       }
                                                    } else if (edited) {
                                                       if (block_transaction && blocked) {
                                                          status = BTRV(B_ABORT);
                                                          if (!btrv_ok(status)) file_error(status);
                                                          block_transaction = blocked = FALSE;
                                                       }
                                                    } else {
                                                       if (block_transaction && blocked) {
                                                          status = BTRV(B_ABORT);
                                                          if (!btrv_ok(status)) file_error(status);
                                                          block_transaction = blocked = FALSE;
                                                       }
                                                    }
                                                    break;
                                          default  : (*strategy)(scan,&pos_chng,crnt_list);
                                                     cls();
                                                     init_screen(input_scrn,scrn_lnth);
                                                     break;
                                          }
                                          break;
                                       case ESC : (*strategy)(ESC,&pos_chng,crnt_list);
                                                  if (v_ptr != NULL) dispose_text(v_ptr);
                                                  exit2 = TRUE;
                                                  break;

                                    }
                                } while (!exit2);
/*                                if (!pos_chng) init_scrllst(scrl_consts,&crnt_line,crnt_list);*/
                                close_window();
										  }
                                break;

                      case ALT_X :if (!savescrn){
                                     if (verifyx(get_out,FALSE)) exit1 = TRUE;
                                  }
                                  break;
                      default :
                                (*strategy_1)(inchr,scan,&pos_chng,crnt_list);
/*                                if (!pos_chng)init_scrllst(scrl_consts,&crnt_line,crnt_list);*/
                                break;
                  } /*switch(scan)*/
                  break;

         case ESC: if (savescrn) exit1 = TRUE;
                   break;



         case 13 :
                  *scrl_consts->main_lnth = lnth_buff;
                  if(v_lnth == 0){
                     memcpy(scrl_consts->main_buff,&crnt_list->rec_num,4);
                     status = BTRV(B_GETDRC+200,scrl_consts->main_file,
                                                 scrl_consts->main_buff,
                                                 scrl_consts->main_lnth,
                                                 scrl_consts->key_buff,
                                                 scrl_consts->path_no);
							if (btrv_locked(status)) {
								gen_error("Record is In Use and Locked");
								break;
							}
                     if (status == 43) break;
                     if (status != 0 && status != 22) file_error(status);
                  }
                  else
                     if (!disassemble(crnt_list->rec_num,scrl_consts,v_lnth,v_ptr)){
                        break;
                     }
                  *scrl_consts->main_lnth = lnth_buff;
                  open_window(1,0,80,22,text_colour,scrl_consts->type);
                  init_screen(input_scrn,scrn_lnth);
                  exit2 = pos_chng = edited = FALSE ;
                  exloccur(24,0);
                  do {
                     switch (command_line(edit_cmndline,&scan,&scrllst_msg)) {
                        case 0 : switch (scan) {
									case PGDN :
										for (iter = 0;
											 iter < scrn_lnth && !(input_scrn[iter].format&FREE_TEXT);
											 iter++);
										if (input_scrn[iter].field != NULL && iter < scrn_lnth) {
											pglen =max(0,input_scrn[iter].col - input_scrn[iter].row);
											for (incptr = input_scrn[iter].field,cnter=0;
												incptr->next != NULL	&& cnter < pglen;
												cnter++,incptr = incptr->next);
											
											input_scrn[iter].field = incptr;
                                 cls();
                                 init_screen(input_scrn,scrn_lnth);
										}
										break;
									case PGUP :
										for (iter = 0;
											 iter < scrn_lnth && !(input_scrn[iter].format&FREE_TEXT);
											 iter++);
										if (input_scrn[iter].field != NULL && iter < scrn_lnth) {
											pglen =max(0,input_scrn[iter].col - input_scrn[iter].row);
											for (incptr = input_scrn[iter].field,cnter=0;
												incptr->last != NULL	&& cnter < pglen;
												cnter++,incptr = incptr->last);
											
											input_scrn[iter].field = incptr;
                                 cls();
                                 init_screen(input_scrn,scrn_lnth);
										}
										break;

																
										 case F1 : if(!block_transaction) {
                                            status = BTRV(B_BEGIN);
                                            if (!btrv_ok(status)) file_error(status);
                                            block_transaction = blocked = TRUE;
                                         }
                                        enter_screen(input_scrn,scrn_lnth,&edited,nobreakout);
                                        exloccur(24,0);
                                        if (edited && verify(1)) {
                                            (*strategy)(F1,&pos_chng,crnt_list);
                                            (*transact_entry)(crnt_list,scrl_consts->main_buff);
                                            if(v_lnth != 0) {
                                               assemble(&temp_block,scrl_consts,v_lnth,v_ptr);
                                               if (temp_block != NULL) {
                                                  status = BTRV(B_UPD,scrl_consts->main_file,
                                                                   temp_block,
                                                                   scrl_consts->main_lnth,
                                                                   scrl_consts->key_buff,
                                                                   scrl_consts->path_no);
                                                  if (status != 0 && status != 22) file_error(status);
                                                  free(temp_block);
                                               }
                                            }else {
                                               *scrl_consts->main_lnth = lnth_buff;
                                               status = BTRV(B_UPD,scrl_consts->main_file,
                                                                   scrl_consts->main_buff,
                                                                   scrl_consts->main_lnth,
                                                                   scrl_consts->key_buff,
                                                                   scrl_consts->path_no);
                                               if (status != 0 && status != 22) file_error(status);
                                            }
							                       
														  (*strategy)(X_INSERT,&pos_chng,crnt_list);

                                            *scrl_consts->main_lnth = lnth_buff;
                                            pos_chng = exit2 = TRUE;
                                            if (block_transaction && blocked) {
                                               status = BTRV(B_END);
                                               if (!btrv_ok(status)) file_error(status);
                                               block_transaction = blocked = FALSE;
                                            }
                                        } else if (edited) {
                                           exit2 = TRUE;
                                           if (block_transaction && blocked) {
                                             status = BTRV(B_ABORT);
                                             if (!btrv_ok(status)) file_error(status);
                                             block_transaction = blocked = FALSE;
                                           }
                                        } else {
                                           if (block_transaction && blocked) {
                                              status = BTRV(B_END);
                                              if (!btrv_ok(status)) file_error(status);
                                              block_transaction = blocked = FALSE;
                                           }
                                        }
                                        break;
                               default : (*strategy)(scan,&pos_chng,crnt_list);
                                         if (pos_chng) {
                                            if (v_ptr != NULL)dispose_text(v_ptr);
                                 /*           *scrl_consts->main_lnth = lnth_buff;*/
															*scrl_consts->main_lnth = 4;
                                            status = BTRV(B_GETPOS,scrl_consts->main_file,
                                                                   &rec_num,
                                                                   scrl_consts->main_lnth,
                                                                   scrl_consts->key_buff,
                                                                   scrl_consts->path_no);
                                            if (status != 0 && status != 22) file_error(status);
                                            *scrl_consts->main_lnth = lnth_buff;
                                            if(v_lnth == 0){
                                               memcpy(scrl_consts->main_buff,&rec_num,4);
                                               if ((status = BTRV(B_GETDRC,scrl_consts->main_file,
                                                                           scrl_consts->main_buff,
                                                                           scrl_consts->main_lnth,
                                                                           scrl_consts->key_buff,
                                                                           scrl_consts->path_no))
                                                                           != 0 && status != 22)
                                                  file_error(status);
                                            }
                                            else
                                              disassemble(rec_num,scrl_consts,v_lnth,v_ptr);
                                            *scrl_consts->main_lnth = lnth_buff;
                                         }
                                         cls();
                                         init_screen(input_scrn,scrn_lnth);
                                         break;

                        }
                        break;
                        case ESC :(*strategy)(ESC,&pos_chng,crnt_list);
                                   exit2 = TRUE;
											  break;
                     }
                  } while (!exit2);
                  close_window();
                  
						status = BTRV(B_UNLOCK,scrl_consts->main_file,
                                                 scrl_consts->main_buff,
                                                 scrl_consts->main_lnth,
                                                 scrl_consts->key_buff,
                                                 scrl_consts->path_no);
						if (!btrv_ok(status)) file_error(status);

/*                  if (!pos_chng) init_scrllst(scrl_consts,&crnt_line,crnt_list);*/
                  if (v_ptr != NULL) dispose_text(v_ptr);
                  break;
          default :
                  (*strategy_1)(inchr,scan,&pos_chng,crnt_list);
/*                  if (!pos_chng) init_scrllst(scrl_consts,&crnt_line,crnt_list);*/
                  break;						  
      } /* switch (inkey)*/
      if(pos_chng) {
/*                      *scrl_consts->main_lnth = lnth_buff;
                      status = BTRV(B_GETPOS,scrl_consts->main_file,
                                             &rec_num,
                                             scrl_consts->main_lnth,
                                             scrl_consts->key_buff,
                                             scrl_consts->path_no);
                      if (status != 0 && status != 22) file_error(status);
                      *scrl_consts->main_lnth = lnth_buff;
                      memcpy(scrl_consts->main_buff,&rec_num,4);
                      status = BTRV(B_GETDRC,scrl_consts->main_file,
                                                   scrl_consts->main_buff,
                                                   scrl_consts->main_lnth,
                                                   scrl_consts->key_buff,
                                                   scrl_consts->path_no);
                      if (status != 0 && status != 22)  file_error(status);

*/
         crnt_list = read_index(keyln,comp_key,match,INSERT,scrl_consts,transact_entry,mode);
         init_scrllst(scrl_consts,&crnt_line,crnt_list);
         pos_chng = FALSE;
      }

   }while (!exit1);
   dispose_list(&scrl_consts->frst_list);
   close_window();
	read_only = save_ro;
	return(scan);
}

