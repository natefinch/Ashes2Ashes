/* ************************************************************************
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "class.h"

extern struct time_data time_info;
extern struct room_data *world;
extern struct spell_info_type spell_info[];


/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((random() % (to - from + 1)) + from);
}


/* simulates dice roll */
int dice(int number, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  while (number-- > 0)
    sum += ((random() % size) + 1);

  return sum;
}


int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}



/* Create a duplicate of a string */
char *str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}



/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
      if (chk < 0)
	return (-1);
      else
	return (1);
  return (0);
}


/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk, i;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
      if (chk < 0)
	return (-1);
      else
	return (1);

  return (0);
}


/* log a death trap hit */
void log_death_trap(struct char_data * ch)
{
  char buf[150];
  extern struct room_data *world;

  sprintf(buf, "%s hit death trap at %s [%d].", GET_NAME(ch),
	  world[ch->in_room].name, world[ch->in_room].number);
  mudlog(buf, CMP, LVL_HERO, TRUE);
}


/* writes a string to the log */
void log(char *str)
{
  time_t ct;
  char *tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%-19.19s :: %s\n", tmstr, str);
}


/* the "touch" command, essentially. */
int touch(char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    perror(path);
    return -1;
  } else {
    fclose(fl);
    return 0;
  }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void mudlog(char *str, char type, int level, byte file)
{
  char buf[MAX_STRING_LENGTH];
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i;
  char *tmp, tp;
  time_t ct;

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (file)
    fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING | PLR_BUILDING)) {
      tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
	    (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

      if ((GET_LEVEL(i->character) >= level) && (tp >= type)) {
	send_to_char(CCGRN(i->character, C_NRM), i->character);
	send_to_char(buf, i->character);
	send_to_char(CCNRM(i->character, C_NRM), i->character);
      }
    }
}


void mortlog(char *str, char type, int level, byte file)
{
  char buf[MAX_STRING_LENGTH];
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i;
  char *tmp, tp;
  time_t ct;

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (file)
    fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING | PLR_BUILDING)) {
      tp = (PRF_FLAGGED(i->character, PRF_MORTLOG) ? 1 : 0);

      if ((GET_LEVEL(i->character) < level) && (tp >= type)) {
	send_to_char(CCGRN(i->character, C_NRM), i->character);
	send_to_char(buf, i->character);
	send_to_char(CCNRM(i->character, C_NRM), i->character);
      }
    }
}


void sprintbit(long long bitvector, char *names[], char *result)
{
  long nr;

  *result = '\0';

  if (bitvector < 0) {
    strcpy(result, "<INVALID BITVECTOR>");
    return;
  }
  for (nr = 0; bitvector; bitvector >>= 1) {
    if (IS_SET(bitvector, 1)) {
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else
	strcat(result, "UNDEFINED ");
    }
    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcpy(result, "NOBITS ");
}



void sprinttype(int type, char *names[], char *result)
{
  int nr = 0;

  while (type && *names[nr] != '\n') {
    type--;
    nr++;
  }

  if (*names[nr] != '\n')
    strcpy(result, names[nr]);
  else
    strcpy(result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17;	/* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

  return now;
}



struct time_info_data age(struct char_data * ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;	/* All players start at 17 */

  player_age.year += ch->player_specials->saved.age_add;

  return player_age;
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  assert(ch->master);

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM))
      affect_from_char(ch, SPELL_CHARM);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    if(GET_LEVEL(ch->master)>=GET_INVIS_LEV(ch))
      act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) {	/* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else {			/* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    free(j);
  }

  ch->master = NULL;
  REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data * ch, struct char_data * leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl)) {
    buf[0]=0;
    return 0;
  }
  else {
    strcpy(buf, temp);
    return lines;
  }
}


int get_filename(char *orig_name, char *filename, int mode)
{
  char *prefix, *middle, *suffix, *ptr, name[64];

  switch (mode) {
  case CRASH_FILE:
    prefix = "plrobjs";
    suffix = "objs";
    break;
  case ALIAS_FILE:
    prefix = "plrtext";
    suffix = "alias";
    break;
  case REIMB_FILE:
    prefix = "plrobjs";
    suffix = "reimb";
    break;
  case SAVEMAIL_FILE:
    prefix = "plrtext";
    suffix = "mail";
    break;
  default:
    return 0;
    break;
  }

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
  return 1;
}


int num_pc_in_room(struct room_data *room)
{
  int i = 0;
  struct char_data *ch;

  for (ch = room->people; ch != NULL; ch = ch->next_in_room)
    if (!IS_NPC(ch))
      i++;

  return i;
}

char *itoa(int n)
{
  char a[7];
  static char b[7];
  int c, i=0;

  if(n) {
    while(n) {
      a[i++]=(n%10)+48;
      n=n/10;
    }
    for(c=0, i--; i; c++, i--) {
      b[c]=a[i];
    }
    b[c++]=a[0];
    b[c]=0;
  }
  else {
    b[0]='0';
    b[1]=0;
  }
  return b;
}

/* Returns the level you get a skill at, uses best class */
int get_skill_level(struct char_data *ch, int skill)
{
  int i, min_lev=LVL_IMPL+1;

  if(IS_NPC(ch))
    return 1;

  for(i=0; i < NUM_CLASSES; i++) {
    if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i)))
      if(GET_CLASS_LEVEL(ch, i) >= spell_info[skill].min_level[i])
        if(spell_info[skill].min_level[i] < min_lev)
          min_lev = spell_info[skill].min_level[i];
  }
  if(spell_info[skill].min_level[(int)GET_CLASS(ch)] < min_lev)
    min_lev=spell_info[skill].min_level[(int)GET_CLASS(ch)];
  return min_lev;
}

/* Returns skill level or 0 if you're not high enough to use it (multiclass) */
int get_skill(struct char_data *ch, int skill)
{
  int i;

  if(IS_NPC(ch)) {
    if(AFF_FLAGGED(ch, AFF_CURSE))
      return 75;
    else
      return 100;
  }

  for(i=0; i < NUM_CLASSES; i++) {
    if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i))) {
      if(GET_LEVEL(ch) < ((100*spell_info[skill].min_level[i])/MULTICLASS_SKILL_LEVELS[GET_NUM_CLASSES(ch)]))
        continue;
      if((i == CLASS_PALADIN) && (GET_ALIGNMENT(ch) < 0))
        continue;
      if((i == CLASS_ANTIPALADIN) && (GET_ALIGNMENT(ch) > 0))
        continue;

      if(AFF_FLAGGED(ch, AFF_CURSE))
        i=MAX((GET_SKILL(ch, skill)-25), 1);
      else
        i=GET_SKILL(ch, skill);
      i-=MAX(GET_COND(ch, DRUNK), 0);
      return i;
    }
  }
  return 0;
}

int can_practice(struct char_data *ch, int skill)
{
  if(GET_LEVEL(ch) < ((100*get_skill_level(ch, skill))/MULTICLASS_SKILL_LEVELS[GET_NUM_CLASSES(ch)])) {
    return 0;
  }
  if(spell_info[skill].prac || GET_SKILL(ch, skill)) {
    return 1;
  }
  return 0;
}

int get_multi_skill_level(struct char_data *ch, int skill)
{
  return((100*get_skill_level(ch, skill))/MULTICLASS_SKILL_LEVELS[GET_NUM_CLASSES(ch)]);
}

int get_damage_reduction(struct char_data *ch, int type)
{
  int i, percent=0;
  extern int resist_apply[];

  if(IS_NPC(ch)) {
    if(IS_SET(IMMUNE_FLAGS(ch), type))
      percent+=100;
    if(IS_SET(RESIST_FLAGS(ch), type))
      percent+=50;
    if(IS_SET(WEAK_FLAGS(ch), type))
      percent-=50;
  }
  else {
    for(i=0; i<NUM_WEARS; i++) {
      if(GET_EQ(ch, i)) {
        if(IS_SET(GET_EQ(ch, i)->obj_flags.immune, type))
          percent+=2*resist_apply[i];
        if(IS_SET(GET_EQ(ch, i)->obj_flags.resist, type))
          percent+=resist_apply[i];
        if(IS_SET(GET_EQ(ch, i)->obj_flags.weak, type))
          percent-=resist_apply[i];
      }
    }
    percent>>=1;
    if(GET_COND(ch, DRUNK) >= 10) {
      if(type==DAMTYPE_ICE)
        percent+=(GET_COND(ch, DRUNK)-3)/5;
      else
        percent+=GET_COND(ch, DRUNK)/10;
    }
  }

  return percent;
}

int compute_ac(struct char_data *ch)
{
  int ac=0;
  extern struct dex_app_type dex_app[];

  ac = GET_AC(ch) - ch->player_specials->saved.inherent_ac_apply;
  if(AWAKE(ch) && (!(GET_CLASS_BITVECTOR(ch) & MK_F)))
    ac += 10*dex_app[GET_DEX(ch)].defensive;

  return(ac);
}

int get_dir(struct char_data *ch)
{
  int i, j, num=0, moveable[NUM_OF_DIRS];

  for(i=0; i<NUM_OF_DIRS; i++) {
    if(CAN_GO(ch, i)) {
      moveable[i]=1;
      num++;
    }
    else {
      moveable[i]=0;
    }
  }

  if(num) {
    j=number(1, num);
    for(i=0, num=0; i<NUM_OF_DIRS; i++) {
      if(moveable[i]) {
        if(j == ++num)
          return i;
      }
    }
  }

  return NUM_OF_DIRS;
}


long long atoll(char *str)
{
  register char *ptr=str;
  register long long retv=0;
  while(*ptr)
    retv=(retv*10)+((*ptr++)-'0');
  return retv;
}


long get_spell_classes(struct char_data *ch, int spell)
{
  int i;
  long retv=0;

  for(i=0; i<NUM_CLASSES; i++) {
    if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i)) &&
      (GET_CLASS_LEVEL(ch, i) >= ((100*spell_info[spell].min_level[i])/MULTICLASS_SKILL_LEVELS[GET_NUM_CLASSES(ch)])))
      retv |= (1 << i);
  }

  return(retv);
}


char *fix_string(char *str)
{
  static char new_str[2*MAX_STRING_LENGTH];
  int i, j;

  for(i=0, j=0; str[i]; i++) {
    if(str[i]=='\r')
      continue;
    else if(str[i]=='\n') {
      new_str[j++]='\r';
      new_str[j++]='\n';
    }
    else
      new_str[j++]=str[i];
  }
  new_str[j]=0;
  return(new_str);
}
