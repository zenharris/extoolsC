index(s,c)
char *s,c;
{
  unsigned int i = (unsigned int) s + 1;

  while (*s)
    if (*s++ == c)
      return((unsigned int) s - i);

  return(-1);
}
