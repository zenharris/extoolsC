char *exrgtjst(fstr,flen)
char *fstr;
int flen;
{
  static char wbuf[256];
  int len,w;
  char wtemp[256];

  strcpy(wtemp,fstr);
  w = strlen(wtemp);
  len = w-1;

  if (wtemp[len] == ' ') {
    do len --; while(wtemp[len] == ' ');
    wtemp[++len] = 0;
  }

  sprintf(wbuf,"%s%s",expad(flen - strlen(wtemp),' '),wtemp);
  return(wbuf);
}
