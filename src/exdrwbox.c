exdrwbox ( frow, fcol , flen , fwidth , fattr )
int frow , fcol , flen , fwidth , fattr;
{
  int w;

  exloccur( frow , fcol );
  exwrtchr(201,fattr,1);
  exloccur( frow , fcol + 1);
  exwrtchr(205,fattr,flen-2);
  exloccur( frow + fwidth - 1 , fcol + 1);
  exwrtchr(205,fattr,flen-2);
  exloccur(frow , fcol + flen - 1);
  exwrtchr(187,fattr,1);
  for (w = frow + 1 ; w < (frow + fwidth - 1) ; w ++ ) {
    exloccur(w , fcol + flen - 1 );
    exwrtchr(186,fattr,1);
    exloccur(w , fcol );
    exwrtchr(186,fattr,1);
  }
  exloccur( frow + fwidth - 1 , fcol + flen - 1);
  exwrtchr(188,fattr,1);
  exloccur( frow + fwidth - 1 , fcol );
  exwrtchr(200,fattr,1);
}
