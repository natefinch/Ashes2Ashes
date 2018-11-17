/* ************************************************************************
*  autocheck.b.c: Scans a file containing output from ps for instances of *
*               buildrun. If none exist, removes the file .building       *
*                                                                         *
*  Copyright (C) 1998 by Jesse Sterr                                      *
*                                                                         *
************************************************************************* */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

void main()
{
  int i;
  FILE *fp;
  char buf[256];

  if((fp=fopen("ps_output", "r"))) {
    while(!feof(fp)) {
      fgets(buf, 255, fp);
      if(!feof(fp)) {
        for(i=0; buf[i]; i++)
          buf[i]=LOWER(buf[i]);
        if(strstr(buf, "buildrun"))
          break;
      }
    }
    if(feof(fp))
      unlink(".building");
    fclose(fp);
  }
  return;
}
