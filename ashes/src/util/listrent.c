/* ************************************************************************
*  file: listrent.c                                  Part of CircleMUD *
*  Usage: list player rent files                                          *
*  Written by Jeremy Elson                                                *
*  All Rights Reserved                                                    *
*  Copyright (C) 1993 The Trustees of The Johns Hopkins University        *
************************************************************************* */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"

void Crash_listrent(char *fname);

void main(int argc, char **argv)
{
  int x;

  for (x = 1; x < argc; x++)
    Crash_listrent(argv[x]);
}

void Crash_listrent(char *fname)
{
  FILE *fl;
  char buf[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct rent_info rent;

  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s: no such file.\r\n", fname);
    printf("%s", buf);
    return;
  }
  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  switch (rent.rentcode) {
  case RENT_RENTED:
    strcat(buf, "Rent\r\n");
    break;
  case RENT_CRASH:
    strcat(buf, "Crash\r\n");
    break;
  case RENT_CRYO:
    strcat(buf, "Cryo\r\n");
    break;
  case RENT_TIMEDOUT:
  case RENT_FORCED:
    strcat(buf, "TimedOut\r\n");
    break;
  default:
    strcat(buf, "Undef\r\n");
    break;
  }
  strcat(buf, ctime(&rent.time));
  strcat(buf, "\r");
  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      return;
    }
    if (!feof(fl)) {
      sprintf(buf, "%s [%5d] id:%ld%s %s\r\n", buf, object.item_number,
              object.id, ((object.extra_flags&ITEM_DUPE)?" <DUPE>":""), fname);
    }
  }

  printf("%s", buf);
  fclose(fl);
}
