char *exgetpce(fsource , fdestin , fdel , fnum )
char *fsource , *fdestin , fdel;
int fnum;
{
  char *p;
  int w;
  int wcount;

  wcount = 1;				/* search for specified piece */
  while (*fsource && (wcount < fnum))
    if (*fsource++ == fdel)
      wcount ++;

  if ((w = index(fsource,fdel)) != -1) {  /* this is not the last piece */
    strncpy(fdestin,fsource,w);
    *(fdestin+w) = 0;
  }
  else
    strcpy(fdestin,fsource);

  return(fdestin);

}
