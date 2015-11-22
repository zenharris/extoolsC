#include <extools.h>

char *exgetvol(fdrive,fdest)
char fdrive;
char *fdest;
{

   TDIR wdir;
   char fspec[65];

   fspec[0] = fdrive+65;		/* convert to uppercase letter */
   fspec[1] = ':';
   fspec[2] = 0;

   strcat(fspec,"\\*.*");

   if (exfndfst(fspec,8,&wdir)) {
     *fdest = 0;
     return(0);
   }

   strcpy(fdest,wdir.name);
   return(fdest);
}

