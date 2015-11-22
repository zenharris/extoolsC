/*
------------------------------------------------------------------------------
                 "KNOWLEDGE OF MANY THINGS" Database System.
                        The TEXACO Text Editor
                              V3.01

V3.00    This third version at beta stage on 13/9/88
V3.01    Added Word-Wrap on 25/11/88
V3.02    Added Line Counter 20/01/89
V3.03    Added Edited Flag  24/01/89
V3.04    Added a workable command set 26/01/89
V3.05    Improvements to LINEDT 10/03/89

void texaco (list_begin,t_line,b_line,retcode)
struct line_rec *list_begin;
char *retcode;
int t_line,b_line;

------------------------------------------------------------------------------
 */





#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "extools.h"


#define CR    13
#define TEXSTATUS "The TEXACO text editor. V3.05                           \
      F5 Full Screen"

void (*after_init_text)(void) = NULL;
int insert_mode = TRUE;
int read_only = FALSE;
int right_margin = 70;

char del_buffer[300];

static insert(str, pos, chr)
char *str;
int pos;
char chr;
{
    char buff[2];
    buff[0] = 0;
    exchrstr(buff, chr);
    exinsstr(str, buff, pos);
}

static delete(str, pos)
char *str;
int pos;
{
    char buff[81];
    buff[0] = 0;
    strcat(buff, exlftstr(str, pos));
    strcat(buff, exrgtstr(str, strlen(str) - pos - 1));
    str[0] = 0;
    strcat(str, buff);
}

struct line_rec *balloc() {
    struct line_rec *memptr;
    if ((memptr = (struct line_rec *) malloc(sizeof (struct line_rec))) == NULL) {
        gen_error("Insufficient memory for entire text.");
        CLEARKBD
    } else {
        memptr->storage[0] = 0;
        memptr->next = NULL;
        memptr->last = NULL;
    }
    return (memptr);
}

int linedt(edtstr, row, col, lnth, attr, pos, control, cmndline)
char *edtstr;
int col, row, lnth, attr, *pos;
int *control;
struct cmndline_params cmndline[];
{
    int c, exit = TRUE, block_insdel = FALSE;
    int scan, inchr, sv_insert = FALSE;
    struct linedt_msg_rec msg;
    char disp_str[81];
    static char comp_str[81];


    if (*control & INIT_ONLY) {
        for (c = strlen(edtstr); c > 0 && edtstr[c - 1] == ' '; c--) edtstr[c - 1] = 0;
        disp_str[0] = 0;
        exscrprn(strncat(disp_str, edtstr, lnth), row, col, attr);
        return;
    } else if (*control & ALLOCATE) {
        (*(char **) edtstr) = balloc();
        return;
    } else if (*control & COMP_LOAD) {
        *comp_str = 0;
        strncat(comp_str, edtstr, lnth);
        return;
    } else if (*control & COMP_CHECK) {
        return (strncmp(comp_str, edtstr, lnth) != 0);
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
        strcpy(del_buffer, edtstr + (lnth));
        edtstr[lnth] = 0;
    }
    for (c = strlen(edtstr); c > 0 && edtstr[c - 1] == ' '; c--) edtstr[c - 1] = 0;

    if (strlen(edtstr) != 0) exscrprn(edtstr, row, col, attr);
    if (strlen(edtstr) < *pos) *pos = strlen(edtstr);
    if (*pos >= lnth) *pos = lnth - 1;
    else if (*pos < 0) *pos = 0;
    exloccur(row, *pos + col);
    if (*control & SUPRESS_DELETE) {
        block_insdel = TRUE;
        sv_insert = insert_mode;
        insert_mode = FALSE;
        exsetcur(7, 8);
    } else if (*control & INSERT_MODE) {

        insert_mode = TRUE;
    } else if (*control & TYPE_OVER) {

        insert_mode = FALSE;
    }
    if (insert_mode) exsetcur(3, 8);
    else exsetcur(7, 8);
    /*	if (cmndline == NULL) {
                    exclreol(23,0,BLACK);
                    exclreol(24,0,BLACK);
            }*/
    do {
        CLEARKBD
        inchr = command_line(cmndline, &scan, &msg);
        switch (inchr) {
            case 0: switch (scan) {
                    case RARR: if (*pos < strlen(edtstr)) (*pos)++;
                        else *pos = lnth + 1;
                        break;
                    case LARR: /*if (*pos > 0)*/ (*pos)--;
                        break;
                    case END: *pos = min(lnth - 1, strlen(edtstr));
                        break;
                    case HOME: *pos = 0;
                        break;
                    case DELETE:if (!read_only && !block_insdel && strlen(edtstr) > *pos) {
                            delete(edtstr, *pos);
                            if (del_buffer[0] == 0)
                                exchrstr(edtstr, ' ');
                            else {
                                exchrstr(edtstr, del_buffer[0]);
                                delete(del_buffer, 0);
                            }
                            exscrprn(edtstr, row, col, attr);
                            /* edtstr[strlen(edtstr)-1] = 0;*/
                        }
                        break;
                    case INSERT: if (!block_insdel) {
                            insert_mode = !insert_mode;
                            if (!insert_mode)
                                exsetcur(7, 8);
                            else
                                exsetcur(3, 8);
                        }
                        break;
                    default: exit = FALSE;
                        break;
                }
                break;
            case TAB: scan = TAB;
            case CR:exit = FALSE;
                break;
            case ESC: exit = FALSE;
                break;
            case BACKSPACE:
                if (*pos > 0) {
                    (*pos)--;
                    exloccur(row, *pos + col);
                    if (!read_only && !block_insdel) {
                        delete(edtstr, *pos);
                        if (del_buffer[0] == 0)
                            exchrstr(edtstr, ' ');
                        else {
                            exchrstr(edtstr, del_buffer[0]);
                            delete(del_buffer, 0);
                        }
                        exscrprn(edtstr, row, col, attr);
                        /* edtstr[strlen(edtstr)-1] = 0;*/
                    }
                }
                break;
            default: if (!read_only/* && (strlen(edtstr) <= (lnth - 1)) || !insert_mode)*/) {

                    if (*control & CAPITALIZE && (*pos == 0 || isspace(edtstr[*pos - 1])))
                        inchr = toupper(inchr);
                    if (block_insdel || !insert_mode) {
                        if (strlen(edtstr) <= *pos)
                            exchrstr(edtstr, inchr);
                        else
                            edtstr[*pos] = inchr;
                        exscrprn(edtstr, row, col, attr);
                    } else {
                        if (strlen(edtstr) > (lnth - 1)) {
                            insert(del_buffer, 0, edtstr[lnth - 1]);
                            edtstr[lnth - 1] = 0;
                        }
                        insert(edtstr, *pos, inchr);
                        if (*pos <= lnth) exscrprn(edtstr, row, col, attr);
                    }
                    (*pos)++;
                    if ((*control & WORD_WRAP) && *pos >= lnth/*-1*/) {
                        exit = FALSE;
                        scan = 0;
                    }
                }
                break;
        }
        if (exit) {
            if (*pos >= lnth) {
                exit = FALSE;
                scan = ESCEOL; /*  DARR;*/
            } else if (*pos < 0) {
                exit = FALSE;
                scan = ESCBOL; /* UARR;*/
            }
        }
        exloccur(row, *pos + col);
    } while (exit);
    if ((*control & SUPRESS_DELETE) && sv_insert/* (*control & INSERT_MODE)*/) {
        insert_mode = TRUE;
        exsetcur(3, 8);
    }

    for (c = strlen(edtstr); c > 0 && edtstr[c - 1] == ' '; c--) edtstr[c - 1] = 0;

    return (scan);
}


static inc_crnt(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter)
int t_line, b_line;

int (*line_ed)();
struct line_rec **crnt_list;
int *crnt_line;
int *line_counter;
{
    int control = INIT_ONLY;
    if ((*crnt_list)->next != NULL) {
        (*line_counter)++;
        (*crnt_list) = (*crnt_list)->next;
        (*crnt_line)++;
        if (*crnt_line > b_line) {
            exscrlup((t_line << 8) + 0, (b_line << 8) + (crnt_window->width - 1), 1, WHITE);
            (*crnt_line) = b_line;
            (*line_ed)((*crnt_list)->storage, *crnt_line, 0, right_margin, WHITE, NULL, &control);
        }
    }
}

static dec_crnt(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter)
int t_line, b_line;

int (*line_ed)();
struct line_rec **crnt_list;
int *crnt_line;
int *line_counter;
{
    int control = INIT_ONLY;
    if ((*crnt_list)->last != NULL) {
        (*line_counter)--;
        (*crnt_list) = (*crnt_list)->last;
        (*crnt_line)--;
        if (*crnt_line < t_line) {
            exscrldn((t_line << 8) + 0, (b_line << 8) + (crnt_window->width - 1), 1, WHITE);
            (*crnt_line) = t_line;
            (*line_ed)((*crnt_list)->storage, *crnt_line, 0, right_margin, WHITE, NULL, &control);
        }
    }
}

static insert_line(new_ptr, b_line, crnt_list, crnt_line, total_lines)
struct line_rec *new_ptr;
int b_line;
struct line_rec **crnt_list;
int *crnt_line;
int *total_lines;
{
    struct line_rec *last_list = NULL, *next_list = NULL;
    last_list = (*crnt_list)->last;
    next_list = (*crnt_list);
    (*crnt_list) = new_ptr;
    (*crnt_list)->last = last_list;
    (*crnt_list)->next = next_list;
    if (next_list != NULL) next_list->last = (*crnt_list);
    if (last_list != NULL) last_list->next = (*crnt_list);
    if (*crnt_line < b_line)
        exscrldn(((*crnt_line) << 8) + 0, (b_line << 8) + (crnt_window->width - 1), 1, WHITE);
    else
        exclreol(b_line, 0, WHITE);
    (*total_lines)++;
}

static delete_line(t_line, b_line, line_ed, crnt_list, crnt_line,
        line_counter, total_lines)
int t_line;
int b_line;

int (*line_ed)();
struct line_rec **crnt_list;
int *crnt_line;
int *line_counter;
int *total_lines;
{
    struct line_rec *del_list = (*crnt_list), *temp = NULL;
    int c, control = INIT_ONLY;
    if (!((*crnt_list)->next == NULL && (*crnt_list)->last == NULL)) {
        if ((*crnt_list)->last != NULL) (*crnt_list)->last->next = (*crnt_list)->next;
        if ((*crnt_list)->next != NULL) (*crnt_list)->next->last = (*crnt_list)->last;
        if ((*crnt_list)->next != NULL) { /* not last entry in list*/
            (*crnt_list) = (*crnt_list)->next;
            if (*crnt_line < b_line) {
                exscrlup(((*crnt_line) << 8) + 0, (b_line << 8) + (crnt_window->width - 1), 1, WHITE);
                temp = (*crnt_list);
                c = *crnt_line;
                while ((temp->next != NULL) && (c != b_line)) {
                    temp = temp->next;
                    c++;
                }
                if (c == b_line) (*line_ed)(temp->storage, b_line, 0, right_margin, WHITE, NULL, &control);
            } else {
                exclreol(b_line, 0, WHITE);
                (*line_ed)((*crnt_list)->storage, b_line, 0, right_margin, WHITE, NULL, &control);
            }
        } else {
            exclreol(*crnt_line, 0, LIGHTGRAY);
            dec_crnt(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter);
        }

        free(del_list);
        (*total_lines)--;
    }
}


static carridge_return(t_line, b_line, line_ed, crnt_list, crnt_line,
        line_counter, crnt_pos, word_warp_flag, total_lines)
int t_line, b_line;

int (*line_ed)();
struct line_rec **crnt_list;
int *crnt_line;
int *line_counter;
int *crnt_pos;
int *word_warp_flag;
int *total_lines;
{
    struct line_rec *temp_list;
    char tempstr[81];
    int control = ALLOCATE;
    if (*word_warp_flag) {
        tempstr[0] = 0;
        strcat(tempstr, exrgtstr((*crnt_list)->storage, strlen((*crnt_list)->storage) - *crnt_pos));
        exdltstr((*crnt_list)->storage, *crnt_pos, strlen((*crnt_list)->storage));
        exclreol(*crnt_line, *crnt_pos, WHITE);
    }
    if ((*crnt_list)->next == NULL) {
        (*line_ed)(&temp_list, 0, 0, 0, 0, NULL, &control);
        if (temp_list != NULL) {
            temp_list->last = (*crnt_list);
            (*crnt_list)->next = temp_list;
            inc_crnt(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter);
            (*total_lines)++;
        }
    } else {
        (*line_ed)(&temp_list, 0, 0, 0, 0, NULL, &control);
        if (temp_list != NULL) {
            inc_crnt(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter);
            insert_line(temp_list, b_line, crnt_list, crnt_line, total_lines);
        }
    }
    *crnt_pos = 0;
    if (*word_warp_flag) {
        strcat((*crnt_list)->storage, tempstr);
        *crnt_pos = strlen((*crnt_list)->storage);
        strncat((*crnt_list)->storage, del_buffer, right_margin - *crnt_pos);
    }
}

init_text(list_begin, t_line, b_line, line_ed)
struct line_rec *list_begin;
int t_line, b_line;

int (*line_ed)();
{
    int c, control = INIT_ONLY;
    struct line_rec *temp;
    temp = list_begin;
    if (temp != NULL)
        for (c = t_line; c <= b_line; c++) {
            if (temp != NULL) {
                (*line_ed)(temp->storage, c, 0, right_margin, WHITE, NULL, &control);
                temp = temp->next;
            }
        }
    if (after_init_text != NULL) (*after_init_text)();
}

static initialize(t_line, b_line, line_ed, crnt_list, crnt_line)
int t_line, b_line;

int (*line_ed)();
struct line_rec **crnt_list;
int *crnt_line;
{
    struct line_rec *temp_list;
    int temp, temp1, offset = 0, control = INIT_ONLY, pos;
    (*crnt_line) = t_line;
    temp_list = (*crnt_list);
    exscrldn((t_line << 8) + 0, (b_line << 8) +(crnt_window->width - 1), 0, WHITE);
    temp = b_line - t_line;
    temp /= 2;
    for (temp1 = 1; temp1 < temp; temp1++)
        if (temp_list->last != NULL) {
            temp_list = temp_list->last;
            offset++;
        }
    while ((temp_list != NULL) && (*crnt_line <= b_line)) {
        (*line_ed)(temp_list->storage, (*crnt_line)++, 0, right_margin, WHITE, &pos, &control);
        temp_list = temp_list->next;
    }
    (*crnt_line) = t_line + offset;
}


word_warp(t_line, b_line, line_ed, crnt_list, crnt_line, crnt_pos,
        right_margin, line_counter, word_warp_flag, total_lines)
int t_line, b_line;

int (*line_ed)();
struct line_rec **crnt_list;
int *crnt_line;
int *crnt_pos;
int *right_margin;
int *line_counter;
int *word_warp_flag;
int *total_lines;
{
    int c;
    for (c = (*right_margin) - 1; c > 0 && (*crnt_list)->storage[c] != ' '; c--);
    if (strlen((*crnt_list)->storage) >= (*right_margin) && c > 0) {
        (*crnt_pos) = c + 1;
        carridge_return(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter, crnt_pos, word_warp_flag, total_lines);
        /*        (*crnt_pos) = strlen((*crnt_list)->storage);*/
    } else {
        carridge_return(t_line, b_line, line_ed, crnt_list, crnt_line, line_counter, crnt_pos, word_warp_flag, total_lines);
        (*crnt_pos) = 0;
    }
}


void texaco(crnt_list, t_line, b_line, retcode, edited, lines_limit, line_ed, cont, cmndline)
struct line_rec **crnt_list;
int t_line, b_line, *retcode, *edited, lines_limit;

int (*line_ed)();
unsigned int cont;
struct cmndline_params cmndline[];
{
    int total_lines, line_counter, crnt_line = 0, crnt_pos = 0;
    int word_warp_flag = TRUE;


    int c, st_line, sb_line, scan, control = ALLOCATE;
    char convstr[10];
    unsigned crc;
    unsigned char inchr, exit = TRUE, scrn_toggle = TRUE, savestr[81], *savescr = NULL;
    /*   unsigned int cont_byte = WORD_WRAP;*/
    struct line_rec *temp_list;

    if (cont & WORD_WRAP) word_warp_flag = TRUE;
    else word_warp_flag = FALSE;

    crnt_line = t_line;
    line_counter = 1;
    total_lines = 1;
    if ((*crnt_list) != NULL) {
        initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
        for (temp_list = (*crnt_list);
                temp_list->last != NULL;
                temp_list = temp_list->last, line_counter++);
        for (; temp_list->next != NULL;
                temp_list = temp_list->next, total_lines++);
    } else
        (*line_ed)(crnt_list, 0, 0, 0, 0, NULL, &control);
    /*   exredscr(savestr,24,0,80);
       exclreol(24,0,RED);
       exscrprn(TEXSTATUS,24,0,RED);*/
    do {
        /*      exclreol(t_line,70,LIGHTGRAY);
              sprintf(convstr,"L %d",line_counter);
              exscrprn(convstr,t_line,70,LIGHTGRAY);*/
        control = COMP_LOAD;
        (*line_ed)((*crnt_list)->storage, 0, 0, right_margin, 0, NULL, &control);
        scan = (*line_ed)((*crnt_list)->storage, crnt_line, 0, right_margin, WHITE, &crnt_pos, &cont, cmndline);
        control = COMP_CHECK;
        if ((*line_ed)((*crnt_list)->storage, 0, 0, right_margin, 0, NULL, &control)) *edited = TRUE;
        control = ALLOCATE;

        if (!insert_mode && (*crnt_list)->next != NULL) if (scan == ENTER) {
                crnt_pos = 0;
                scan = DARR;
            }

        switch (scan) {
            case 00:if (total_lines < lines_limit) {
                    if (word_warp_flag) word_warp(t_line, b_line, line_ed, crnt_list, &crnt_line, &crnt_pos, &right_margin, &line_counter, &word_warp_flag, &total_lines);
                    else carridge_return(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter, &crnt_pos, &word_warp_flag, &total_lines);
                    *edited = TRUE;
                }
                break;
            case END: crnt_pos = strlen((*crnt_list)->storage);
                break;
            case HOME: crnt_pos = 0;
                break;
            case CTRL_END: for (c = crnt_line; c < b_line; c++) inc_crnt(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter);
                break;
            case CTRL_HOME: for (c = crnt_line; c > t_line; c--) dec_crnt(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter);
                break;
            case F5: if (scrn_toggle) {
                    if ((savescr = malloc((22 - 6 + 1) * 160)) != NULL) {
                        st_line = t_line;
                        sb_line = b_line;
                        t_line = 6;
                        /*b_line = 22;*/
                        savscrn(savescr, 0, t_line, 79, b_line);
                        initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
                        scrn_toggle = !scrn_toggle;
                    } else gen_error("Not Enough Memory to Save Screen");
                } else {
                    if (savescr != NULL) {
                        exscrldn((t_line << 8) + 0, (b_line << 8) +(crnt_window->width - 1), 0, WHITE);
                        rstscrn(savescr, 0, t_line, 79, b_line);
                        t_line = st_line;
                        b_line = sb_line;
                        initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
                        free(savescr);
                        savescr = NULL;
                        scrn_toggle = !scrn_toggle;
                    }
                }
                break;
            case ESCEOL:
            case DARR: if ((*crnt_list)->next != NULL) {
                    inc_crnt(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter);
                    if (crnt_pos > right_margin) crnt_pos = 0;
                }
                break;
            case ESCBOL:
            case UARR: if ((*crnt_list)->last != NULL) {
                    dec_crnt(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter);
                    if (crnt_pos < 0) crnt_pos = right_margin;
                } else {
                    exit = FALSE;
                    crnt_pos = 0;
                }
                break;
            case PGDN: for (c = 0; c < (b_line - t_line)
                        && (*crnt_list)->next != NULL;
                        c++, (*crnt_list) = (*crnt_list)->next,
                        line_counter++);
                initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
                break;
            case PGUP: for (c = 0; c < (b_line - t_line)
                        && (*crnt_list)->last != NULL;
                        c++, (*crnt_list) = (*crnt_list)->last,
                        line_counter--);
                initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
                break;
            case CTRL_PGUP: for (; (*crnt_list)->last != NULL; line_counter--,
                        (*crnt_list) = (*crnt_list)->last);
                initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
                break;
            case CTRL_PGDN: for (; (*crnt_list)->next != NULL; line_counter++,
                        (*crnt_list) = (*crnt_list)->next);
                initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
                break;
            case ALT_O: if (!read_only && total_lines < lines_limit) {
                    (*line_ed)(&temp_list, 0, 0, 0, 0, NULL, &control);
                    if (temp_list != NULL) insert_line(temp_list, b_line, crnt_list, &crnt_line, &total_lines);
                    *edited = TRUE;
                }
                break;
            case ALT_D: if (!read_only) {
                    delete_line(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter, &total_lines);
                    *edited = TRUE;
                }
                break;
            case ENTER:if ((!read_only) && total_lines < lines_limit) {
                    carridge_return(t_line, b_line, line_ed, crnt_list, &crnt_line, &line_counter, &crnt_pos, &word_warp_flag, &total_lines);
                    crnt_pos = 0;
                    *edited = TRUE;
                }
                break;
            case ESC:
            case 01: exit = FALSE;
                break;
        }

    } while (exit);
    if (savescr != NULL) {
        exscrldn((t_line << 8) + 0, (b_line << 8) +(crnt_window->width - 1), 0, WHITE);
        rstscrn(savescr, 0, t_line, 79, b_line);
        t_line = st_line;
        b_line = sb_line;
        initialize(t_line, b_line, line_ed, crnt_list, &crnt_line);
        free(savescr);
        savescr = NULL;
        scrn_toggle = !scrn_toggle;
    }
    /*   exclreol(24,0,RED);
       exscrprn(savestr,24,0,RED);*/
    *retcode = scan;
}

