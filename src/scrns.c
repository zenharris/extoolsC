#include <extools.h>

savscrn(buff,t_col,t_line,b_col,b_line)
char *buff;
int t_col,t_line,b_col,b_line;
{
   int wscrseg;
   if (MEMB(0x40,0x49)==3)
      wscrseg = 0xb800;
   else
      wscrseg = 0xb000;
   cmoveb(wscrseg,t_line * 160 + (t_col << 1),
          buff,b_line - t_line + 1,b_col - t_col + 1);
}

rstscrn(buff,t_col,t_line,b_col,b_line)
char *buff;
int t_col,t_line,b_col,b_line;
{
   int wscrseg;
   if (MEMB(0x40,0x49)==3)
      wscrseg = 0xb800;
   else
      wscrseg = 0xb000;
   cmoves(buff,wscrseg,t_line * 160 + (t_col << 1),
         b_line - t_line + 1,b_col - t_col + 1);    
}

