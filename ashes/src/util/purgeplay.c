/* ************************************************************************
*  file: purgeplay.c                                    Part of CircleMUD * 
*  Usage: purge useless chars from playerfile                             *
*  All Rights Reserved                                                    *
*  Copyright (C) 1992, 1993 The Trustees of The Johns Hopkins University  *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************* */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"


void purge(char *filename)
{
  FILE *fl;
  FILE *outfile;
  struct char_file_u player;
  int okay, num = 0, i, level, invalid;
  long timeout;
  char *ptr, reason[80];

  if (!(fl = fopen(filename, "r+"))) {
    printf("Can't open %s.", filename);
    exit(0);
  }
  outfile = fopen("players.new", "w");
  printf("Deleting: \n");

  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (feof(fl)) {
      fclose(fl);
      fclose(outfile);
      puts("Done.");
      exit(0);
    }
    okay = 1;
    invalid = 0;
    *reason = '\0';

    for (ptr = player.name; *ptr; ptr++)
      if (!isalpha(*ptr) || *ptr == ' ') {
	okay = 0;
        invalid = 1;
	strcpy(reason, "Invalid name");
      }

    level=0;
    for(i=0; i<32; i++)
      if(level < player.player_specials_saved.level[i])
        level = player.player_specials_saved.level[i];

    if ((player.level == 0) && (level == 0)) {
      okay = 0;
      strcpy(reason, "Never entered game");
    }
    if (player.level < 0 || player.level > LVL_IMPL) {
      okay = 0;
      invalid = 1;
      strcpy(reason, "Invalid level");
    }
    /* now, check for timeouts.  They only apply if the char is not
       cryo-rented.   Lev 32-34 do not time out.  */

    timeout = 1000;

    if (okay && (level < LVL_HERO)) {

      if (!(player.char_specials_saved.act & PLR_CRYO)) {
	if (level <= 1)		timeout = 7;		/* Lev   1 : 7 days */
	else if (level <= 8)	timeout = 14;		/* Lev 2-8 : 14 days */
	else if (level <= 20)	timeout = 30;		/* Lev 9-20: 30 days */
	else if (level <= 70)	timeout = 60;		/* Lev 21-70: 60 days */
	else if (level < LVL_HERO) timeout = 90;	/* Lev 71-100: 90 days */
      } else
	timeout = 90;

      timeout *= SECS_PER_REAL_DAY;

      if ((time(0) - player.last_logon) > timeout) {
	okay = 0;
	sprintf(reason, "Level %2d idle for %3ld days", level,
		((time(0) - player.last_logon) / SECS_PER_REAL_DAY));
      }
    }
    if (player.char_specials_saved.act & PLR_DELETED) {
      okay = 0;
      sprintf(reason, "Deleted flag set");
    }

    /* Don't delete if they are valid and have NODELETE or are frozen less than half a year */
    if(!okay && !invalid) {
      if (player.char_specials_saved.act & PLR_NODELETE) {
        okay = 2;
        strcat(reason, "; NOT deleted.");
      }
      else if((player.char_specials_saved.act & PLR_FROZEN) && (((time(0) - player.last_logon) / SECS_PER_REAL_DAY) < 180)) {
        okay = 2;
        strcat(reason, "; NOT deleted, frozen.");
      }
    }

    if (okay)
      fwrite(&player, sizeof(struct char_file_u), 1, outfile);
    else
      printf("%4d. %-20s %s\n", ++num, player.name, reason);

    if (okay == 2)
      fprintf(stdout, "%-20s %s\n", player.name, reason);
  }
}



int main(int argc, char *argv[])
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    purge(argv[1]);

  return 0;
}
