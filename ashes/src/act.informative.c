/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "class.h"
#include "olc.h"

struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;

char m100[] = " is in excellent condition.\r\n";
char m99_90[] = " has a few scratches.\r\n";
char m89_75[] = " has some small wounds and bruises.\r\n";
char m74_50[] = " has quite a few wounds.\r\n";
char m49_30[] = " has some big nasty wounds and scratches.\r\n";
char m29_15[] = " looks pretty hurt.\r\n";
char m14_0[] = " is in awful condition.\r\n";
char m_0[] = " is bleeding awfully from big wounds.\r\n";

char *diag_message[102]= {  m14_0, m14_0, m14_0, m14_0, m14_0,
  m14_0, m14_0, m14_0, m14_0, m14_0, m14_0, m14_0, m14_0, m14_0,
   m14_0, m29_15, m29_15, m29_15, m29_15, m29_15, m29_15, m29_15,
  m29_15, m29_15, m29_15, m29_15, m29_15, m29_15, m29_15, m29_15,
  m49_30, m49_30, m49_30, m49_30, m49_30, m49_30, m49_30, m49_30,
  m49_30, m49_30, m49_30, m49_30, m49_30, m49_30, m49_30, m49_30,
  m49_30, m49_30, m49_30, m49_30, m74_50, m74_50, m74_50, m74_50,
  m74_50, m74_50, m74_50, m74_50, m74_50, m74_50, m74_50, m74_50,
  m74_50, m74_50, m74_50, m74_50, m74_50, m74_50, m74_50, m74_50,
  m74_50, m74_50, m74_50, m74_50, m74_50, m89_75, m89_75, m89_75,
  m89_75, m89_75, m89_75, m89_75, m89_75, m89_75, m89_75, m89_75,
  m89_75, m89_75, m89_75, m89_75, m99_90, m99_90, m99_90, m99_90,
  m99_90, m99_90, m99_90, m99_90, m99_90, m99_90, m100, m_0 };

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct command_info cmd_info[];

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *dirs[];
extern char *abbr_dirs[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *connected_types[];
extern char *class_abbrevs[];
extern char *room_bits[];
extern char *spells[];
extern int max_players;

long find_class_bitvector(char arg);

void show_obj_to_char(struct obj_data * object, struct char_data * ch, int mode)
{
  bool found;

  *buf = '\0';

  if((GET_LEVEL(ch) >= LVL_HERO) && (IS_SET(GET_OBJ_EXTRA(object), ITEM_DUPE)))
    strcat(buf, "<DUPE>");

  if ((mode == 0) && object->description)
    strcat(buf, object->description);
  else if (object->short_description && ((mode == 1) ||
				 (mode == 2) || (mode == 3) || (mode == 4)))
    strcat(buf, object->short_description);
  else if (mode == 5) {
    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
      if (object->action_description && *object->action_description) {
	strcat(buf, "There is something written upon it:\r\n\r\n");
	strcat(buf, object->action_description);
	page_string(ch->desc, buf, 1);
      } else
	act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else if ((GET_OBJ_TYPE(object) != ITEM_DRINKCON)&&(GET_OBJ_TYPE(object) != ITEM_FOUNTAIN)) {
      strcat(buf, "You see nothing special..");
    } else			/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      strcat(buf, "It looks like a drink container.");
  }
  if (mode != 3) {
    found = FALSE;
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buf, " (invisible)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      strcat(buf, " ..It glows blue!");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
      strcat(buf, " ..It glows yellow!");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buf, " ..It glows faintly");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buf, " ..It hums faintly");
      found = TRUE;
    }
  }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}


void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
		           bool show)
{
  struct obj_data *i, *prev_i=NULL;
  int how_many=1;
  bool found;

  found = FALSE;

  if(PRF_FLAGGED(ch, PRF_COMBINE)) {
    for (i = list; i; i = i->next_content) {
      if (CAN_SEE_OBJ(ch, i)) {
        if(prev_i && (GET_OBJ_RNUM(prev_i)==GET_OBJ_RNUM(i)) &&
           ((GET_OBJ_RNUM(prev_i) > -1) ||
            (prev_i->short_description && i->short_description &&
             (!strcmp(prev_i->short_description, i->short_description)))))
          how_many++;
        else {
          if(prev_i) {
            if(how_many > 1) {
              sprintf(buf, "(%d) ", how_many);
              send_to_char(buf, ch);
            }
            show_obj_to_char(prev_i, ch, mode);
          }
          how_many=1;
        }
        prev_i=i;
        found = TRUE;
      }
    }
    if(prev_i) {
      if(how_many > 1) {
        sprintf(buf, "(%d) ", how_many);
        send_to_char(buf, ch);
      }
      show_obj_to_char(prev_i, ch, mode);
    }
  }
  else {
    for (i = list; i; i = i->next_content) {
      if (CAN_SEE_OBJ(ch, i)) {
        show_obj_to_char(i, ch, mode);
        found = TRUE;
      }
    }
  }

  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}


void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
  int percent;

  if((!i) || (!ch))
    return;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;		/* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  strcat(buf, diag_message[(percent < 0 ? 101 : (percent > 100 ? 100 : percent))]);

  send_to_char(CCRED(ch, C_CMP), ch);
  send_to_char(buf, ch);
  send_to_char(CCNRM(ch, C_CMP), ch);
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
  int j, found;
  struct obj_data *tmp_obj;

  if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
	send_to_char(where[j], ch);
	show_obj_to_char(GET_EQ(i, j), ch, 1);
      }
  }
  if (ch != i && (IS_SET(GET_CLASS_BITVECTOR(ch), TH_F) || GET_LEVEL(ch) >= LVL_HERO)) {
    found = FALSE;
    act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch))) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
    }

    if (!found)
      send_to_char("You can't see anything.\r\n", ch);
  }
}


void list_one_char(struct char_data * i, struct char_data * ch)
{
  char *positions[] = {
    " is lying here, dead.",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here.",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here.",
    " is falling!"
  };

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    if (IS_AFFECTED(i, AFF_INVISIBLE))
      strcpy(buf, "*");
    else
      *buf = '\0';

    if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      if (IS_EVIL(i))
	strcat(buf, "(Red Aura) ");
      else if (IS_GOOD(i))
	strcat(buf, "(Blue Aura) ");
    }
    strcat(buf, i->player.long_descr);
    send_to_char(buf, ch);

    if (IS_AFFECTED(i, AFF_SANCTUARY))
      act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i, AFF_BLIND))
      act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i, AFF_DISEASE))
      act("...$e looks very sick!", FALSE, i, 0, ch, TO_VICT);

    return;
  }
  if (IS_NPC(i)) {
    strcpy(buf, i->player.short_descr);
    CAP(buf);
  } else
    sprintf(buf, "%s %s", i->player.name, GET_TITLE(i));

  if (IS_AFFECTED(i, AFF_INVISIBLE))
    strcat(buf, " (invisible)");
  if (IS_AFFECTED(i, AFF_HIDE))
    strcat(buf, " (hidden)");
  if (!IS_NPC(i) && !i->desc)
    strcat(buf, " (linkless)");
  if (PLR_FLAGGED(i, PLR_WRITING))
    strcat(buf, " (writing)");
  if (PLR_FLAGGED(i, PLR_BUILDING))
    strcat(buf, " (building)");

  if (GET_POS(i) != POS_FIGHTING)
    strcat(buf, positions[(int) GET_POS(i)]);
  else {
    if (FIGHTING(i)) {
      strcat(buf, " is here, fighting ");
      if (FIGHTING(i) == ch)
	strcat(buf, "YOU!");
      else {
	if (i->in_room == FIGHTING(i)->in_room)
	  strcat(buf, PERS(FIGHTING(i), ch));
	else
	  strcat(buf, "someone who has already left");
	strcat(buf, "!");
      }
    } else			/* NIL fighting pointer */
      strcat(buf, " is here struggling with thin air.");
  }

  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, " (Red Aura)");
    else if (IS_GOOD(i))
      strcat(buf, " (Blue Aura)");
  }
  if (i->char_specials.timer >= 4)
    sprintf(buf, "%s (idle %d)", buf, i->char_specials.timer*SECS_PER_MUD_HOUR/60);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  if (IS_AFFECTED(i, AFF_SANCTUARY))
    act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
  if (IS_AFFECTED(i, AFF_BLIND))
    act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
  if (IS_AFFECTED(i, AFF_DISEASE))
    act("...$e looks very sick!", FALSE, i, 0, ch, TO_VICT);
}



void list_char_to_char(struct char_data * list, struct char_data * ch)
{
  struct char_data *i;

  for (i = list; i; i = i->next_in_room)
    if (ch != i) {
      if (CAN_SEE(ch, i))
	list_one_char(i, ch);
      else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
	       IS_AFFECTED(i, AFF_INFRAVISION))
	send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);
    }
}


void do_auto_exits(struct char_data * ch)
{
  int door;

  *buf = '\0';

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_HERO)
	sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],
		world[EXIT(ch, door)->to_room].number,
		world[EXIT(ch, door)->to_room].name);
      else {
        if(IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
          buf2[0]=0;
        }
        else {
          sprintf(buf2, "%-5s - ", dirs[door]);
          if((IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch)) ||
             (ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_MAGIC_DARK)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))))
            strcat(buf2, "Too dark to tell\r\n");
          else {
            strcat(buf2, world[EXIT(ch, door)->to_room].name);
            strcat(buf2, "\r\n");
          }
        }
      }
      strcat(buf, CAP(buf2));
    }
  send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);

}


ACMD(do_exits)
{
  int door;

  *buf = '\0';

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  if(ROOM_FLAGGED(ch->in_room, ROOM_MAGIC_DARK)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))) {
    send_to_char("There is nothing but blackness all around you...\r\n", ch);
    return;
  }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_HERO)
	sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],
		world[EXIT(ch, door)->to_room].number,
		world[EXIT(ch, door)->to_room].name);
      else {
        if(IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
          buf2[0]=0;
        }
        else {
          sprintf(buf2, "%-5s - ", dirs[door]);
          if((IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch)) ||
             (ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_MAGIC_DARK)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))))
            strcat(buf2, "Too dark to tell\r\n");
          else {
            strcat(buf2, world[EXIT(ch, door)->to_room].name);
            strcat(buf2, "\r\n");
          }
        }
      }
      strcat(buf, CAP(buf2));
    }
  send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);
}



void look_at_room(struct char_data * ch, int ignore_brief)
{
  if(AFF_FLAGGED(ch, AFF_DETECT_MAGIC) && ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
    send_to_char("You sense that magic will not work here.\r\n", ch);
  }
  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    return;
  } else if((IS_AFFECTED(ch, AFF_BLIND))&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))&&(!AFF_FLAGGED(ch, AFF_FEEL_LIGHT))) {
    send_to_char("You see nothing but infinite darkness...\r\n", ch);
    return;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_MAGIC_DARK)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))) {
    if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
        ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
      send_to_char(world[ch->in_room].description, ch);
    send_to_char("There is nothing but blackness all around you...\r\n", ch);
    return;
  }
  send_to_char(CCCYN(ch, C_NRM), ch);
  if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
    sprintbit((long) ROOM_FLAGS(ch->in_room), room_bits, buf);
    sprintf(buf2, "[%5d] %s [ %s]", world[ch->in_room].number,
	    world[ch->in_room].name, buf);
    send_to_char(buf2, ch);
  } else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    send_to_char(world[ch->in_room].description, ch);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  /* autoexits */
  if (!PRF_FLAGGED(ch, PRF_NOEXITS))
    do_auto_exits(ch);

  if(ROOM_FLAGGED(ch->in_room, ROOM_DIM_DOOR))
    send_to_char("There is a black portal before you.\r\n", ch);
  if(ROOM_FLAGGED(ch->in_room, ROOM_STASIS))
    send_to_char("Everything is moving very slow.\r\n", ch);
}



void look_in_direction(struct char_data * ch, int dir)
{
  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      send_to_char(EXIT(ch, dir)->general_description, ch);
    else
      send_to_char("You see nothing special.\r\n", ch);

    if(IS_SET(EXIT(ch, dir)->exit_info, EX_ISDOOR) && EXIT(ch, dir)->keyword) {
      if(IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
        if(str_cmp(fname(EXIT(ch, dir)->keyword), "secret") && (!IS_SET(EXIT(ch, dir)->exit_info, EX_SECRET))) {
          sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
          send_to_char(buf, ch);
        }
      }
      else {
        sprintf(buf, "The %s is open.\r\n", fname(EXIT(ch, dir)->keyword));
        send_to_char(buf, ch);
      }
    }
  } else
    send_to_char("Nothing special there...\r\n", ch);
}



void look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("Look in what?\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    send_to_char("There's nothing inside that!\r\n", ch);
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
      if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
	send_to_char("It is closed.\r\n", ch);
      else {
	send_to_char(fname(obj->name), ch);
	switch (bits) {
	case FIND_OBJ_INV:
	  send_to_char(" (carried): \r\n", ch);
	  break;
	case FIND_OBJ_ROOM:
	  send_to_char(" (here): \r\n", ch);
	  break;
	case FIND_OBJ_EQUIP:
	  send_to_char(" (used): \r\n", ch);
	  break;
	}

	list_obj_to_char(obj->contains, ch, 2, TRUE);
      }
    } else {		/* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
	send_to_char("It is empty.\r\n", ch);
      else {
	if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0)) {
	  sprintf(buf, "Its contents seem somewhat murky.\r\n"); /* BUG */
	} else {
	  amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
	  sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
	  sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt], buf2);
	}
	send_to_char(buf, ch);
      }
    }
  }
}



char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isexact(word, i->keyword))
      return (i->description);

  return NULL;
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = 0, j, look_type;
  struct char_data *found_char = NULL;
  struct obj_data *obj = NULL, *found_obj = NULL;
  char *desc;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
	act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }
  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL) {
    page_string(ch->desc, desc, 0);
    return;
  }
  /* Cut arg down to just the part after the dot */
  for(j=0; arg[j]; j++) {
    if(arg[j]=='.')
      break;
  }
  if(arg[j])
    arg+=j+1;
  if(found_obj) {
    if (CAN_SEE_OBJ(ch, found_obj))
      if ((desc = find_exdesc(arg, found_obj->ex_description)) != NULL) {
        send_to_char(desc, ch);
        found = 1;
      }
  }
  if(!found) {
    /* Does the argument match an extra desc in the char's equipment? */
    for (j = 0; j < NUM_WEARS && !found; j++)
      if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
        if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL) {
          send_to_char(desc, ch);
          found = 1;
        }
  }
  if(!found) {
    /* Does the argument match an extra desc in the char's inventory? */
    for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
      if (CAN_SEE_OBJ(ch, obj))
        if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
          send_to_char(desc, ch);
          found = 1;
        }
    }
  }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  if (bits) {			/* If an object was found back in
				 * generic_find */
    if (!found)
      show_obj_to_char(found_obj, ch, 5);	/* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);	/* Find hum, glow etc */
  } else if (!found) {
    if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if ((look_type = search_block(arg, abbr_dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else
      send_to_char("You do not see that here.\r\n", ch);
  }
}


ACMD(do_look)
{
  static char arg2[MAX_INPUT_LENGTH];

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if(IS_AFFECTED(ch, AFF_BLIND)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))&&(!AFF_FLAGGED(ch, AFF_FEEL_LIGHT)))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_MAGIC_DARK)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))&&(*argument)) {
    send_to_char("There is nothing but blackness all around you...\r\n", ch);
  } else {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char("Read what?\r\n", ch);
      else
	look_at_target(ch, arg);
      return;
    }
    if (!*arg)			/* "look" alone, without an argument at all */
      look_at_room(ch, 1);
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else
      look_at_target(ch, arg);
  }
}



ACMD(do_examine)
{
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  one_argument(argument, arg);

  if(ROOM_FLAGGED(ch->in_room, ROOM_MAGIC_DARK)&&(!PRF_FLAGGED(ch, PRF_HOLYLIGHT))) {
    send_to_char("There is nothing but blackness all around you...\r\n", ch);
    return;
  }

  if (!*arg) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }
  look_at_target(ch, arg);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\r\n", ch);
      look_in_obj(ch, arg);
    }
  }
}



ACMD(do_gold)
{
  if (GET_GOLD(ch) == 0)
    send_to_char("You're broke!\r\n", ch);
  else if (GET_GOLD(ch) == 1)
    send_to_char("You have one miserable little gold coin.\r\n", ch);
  else {
    sprintf(buf, "You have %ld gold coins.\r\n", GET_GOLD(ch));
    send_to_char(buf, ch);
  }
}


ACMD(do_score)
{
  struct time_info_data playing_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  struct affected_type *aff;
  extern char *apply_types[];
  extern char *affected_bits[];

  char *print_class_levels(struct char_data *ch);

  sprintf(buf, "You are %d years old.", GET_AGE(ch));

  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf, "  It's your birthday today.\r\n");
  else
    strcat(buf, "\r\n");

  sprintf(buf,
       "%sYou have %d(%d+%d) hit, %d(%d+%d) mana, and %d(%d+%d) move points.\r\n",
	  buf, GET_HIT(ch), GET_MAX_HIT(ch), hit_gain(ch), GET_MANA(ch),
	  GET_MAX_MANA(ch), mana_gain(ch), GET_MOVE(ch), GET_MAX_MOVE(ch),
	  move_gain(ch));

  sprintf(buf, "%sYour abilities are: Str: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
	  buf, GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch),
	  GET_CON(ch), GET_CHA(ch));

  sprintf(buf, "%sYour armor class is %d, and your alignment is %d (%s).\r\n",
	  buf, compute_ac(ch), GET_ALIGNMENT(ch), (IS_EVIL(ch)?"evil":(IS_GOOD(ch)?"good":"neutral")));

  sprintf(buf, "%sYou have scored %ld exp, and have %ld gold coins.\r\n",
	  buf, GET_EXP(ch), GET_GOLD(ch));

  if (!IS_NPC(ch)) {
    if (GET_LEVEL(ch) < LVL_HERO) {
      sprintf(buf, "%sYou need %ld exp to reach your next level.  You have %d quest points.\r\n", buf,
	((GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][GET_LEVEL(ch)+1]) - GET_EXP(ch)), GET_QP(ch));
      sprintf(buf, "%sYour classes and levels are: [%s]\r\n", buf, print_class_levels(ch));
    }

    playing_time = real_time_passed((time(0) - ch->player.time.logon) +
				  ch->player.time.played, 0);
    sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
	  buf, playing_time.day, playing_time.hours);

    sprintf(buf, "%sThis ranks you as %s %s (level %d).\r\n", buf,
	  GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch));
  }

  sprintf(buf, "%sYour resistances are: heat: %d%%, cold: %d%%, energy attacks: %d%%\r\n", buf,
          get_damage_reduction(ch, DAMTYPE_FIRE), get_damage_reduction(ch, DAMTYPE_ICE),
          get_damage_reduction(ch, DAMTYPE_ENERGY));
  sprintf(buf, "%s  blunt weapons: %d%%, slashing weapons %d%%, piercing weapons: %d%%\r\n", buf,
          get_damage_reduction(ch, DAMTYPE_BLUNT), get_damage_reduction(ch, DAMTYPE_SLASH),
          get_damage_reduction(ch, DAMTYPE_PIERCE));
  sprintf(buf, "%sMagic resistance: %d%%   Psionic resistance: %d%%\r\n", buf, GET_MR(ch), GET_PR(ch));

  switch (GET_POS(ch)) {
  case POS_DEAD:
    strcat(buf, "You are DEAD!\r\n");
    break;
  case POS_MORTALLYW:
    strcat(buf, "You are mortally wounded!  You should seek help!\r\n");
    break;
  case POS_INCAP:
    strcat(buf, "You are incapacitated, slowly fading away...\r\n");
    break;
  case POS_STUNNED:
    strcat(buf, "You are stunned!  You can't move!\r\n");
    break;
  case POS_SLEEPING:
    strcat(buf, "You are sleeping.\r\n");
    break;
  case POS_RESTING:
    strcat(buf, "You are resting.\r\n");
    break;
  case POS_SITTING:
    strcat(buf, "You are sitting.\r\n");
    break;
  case POS_FIGHTING:
    if (FIGHTING(ch))
      sprintf(buf, "%sYou are fighting %s.\r\n", buf, PERS(FIGHTING(ch), ch));
    else
      strcat(buf, "You are fighting thin air.\r\n");
    break;
  case POS_STANDING:
    strcat(buf, "You are standing.\r\n");
    break;
  default:
    strcat(buf, "You are floating.\r\n");
    break;
  }

  if (GET_COND(ch, DRUNK) > 10)
    strcat(buf, "You are intoxicated.\r\n");

  if (GET_COND(ch, FULL) == 0)
    strcat(buf, "You are hungry.\r\n");

  if (GET_COND(ch, THIRST) == 0)
    strcat(buf, "You are thirsty.\r\n");

  if (IS_AFFECTED(ch, AFF_BLIND))
    strcat(buf, "You have been blinded!\r\n");

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    strcat(buf, "You are invisible.\r\n");

  if (IS_AFFECTED(ch, AFF_POISON))
    strcat(buf, "You are poisoned!\r\n");

  if (IS_AFFECTED(ch, AFF_CHARM))
    strcat(buf, "You have been charmed!\r\n");

  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
    strcat(buf, "You are summonable by other players.\r\n");

  if (ch->affected) {
    strcat(buf, "You are affected by the following spells:\r\n");
    for (aff = ch->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      sprintf(buf, "%sSPL: (%3dhr) %s%-21s%s ", buf, aff->duration + 1,
	      CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
      if (aff->modifier) {
	sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
	strcat(buf, buf2);
      }
      if (aff->bitvector) {
	if (*buf2)
	  strcat(buf, ", sets ");
	else
	  strcat(buf, "sets ");
	sprintbit(aff->bitvector, affected_bits, buf2);
	strcat(buf, buf2);
      }
      strcat(buf, "\r\n");
    }
  }

  page_string(ch->desc, buf, TRUE);
}


ACMD(do_inventory)
{
  send_to_char("You are carrying:\r\n", ch);
  list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


ACMD(do_equipment)
{
  int i, found = 0;

  send_to_char("You are using:\r\n", ch);
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
	send_to_char(where[i], ch);
	show_obj_to_char(GET_EQ(ch, i), ch, 1);
	found = TRUE;
      } else {
	send_to_char(where[i], ch);
	send_to_char("Something.\r\n", ch);
	found = TRUE;
      }
    }
  }
  if (!found) {
    send_to_char(" Nothing.\r\n", ch);
  }
}


ACMD(do_time)
{
  char *suf;
  int weekday, day;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);
}


ACMD(do_weather)
{
  static char *sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
  "lit by flashes of lightning"};

  if (OUTSIDE(ch)) {
    sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
	    (weather_info.change >= 0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
    send_to_char(buf, ch);
  } else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


ACMD(do_help)
{
  extern int top_of_helpt;
  extern struct help_index_element *help_table;
  extern char *help;

  int chk, bot, top, mid, minlen, i, cmd_num, god_cmd=0;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_table) {
    send_to_char("No help available.\r\n", ch);
    return;
  }

  minlen = strlen(argument);

/* Keeps you from seeing help on god commands you don't have */
  for (cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if ((!strn_cmp(argument, cmd_info[i].command, minlen)) &&
        (cmd_info[i].minimum_level >= LVL_HERO) &&
	(GET_LEVEL(ch) < cmd_info[i].minimum_level) &&
        (!GRNT_FLAGGED(ch, cmd_info[i].grant)))
    {
      god_cmd=1;
      break;
    }
  }

  bot = 0;
  top = top_of_helpt;

  for (;;) {
    mid = (bot + top) / 2;

    if (bot > top) {
      send_to_char("There is no help on that word.\r\n", ch);
      return;
    } else if (!(chk = strn_cmp(argument, help_table[mid].keyword, minlen))) {
      /* trace backwards to find first matching entry. Thanks Jeff Fink! */
      while ((mid > 0) &&
	 (!(chk = strn_cmp(argument, help_table[mid - 1].keyword, minlen))))
	mid--;
      if(god_cmd) {
        for (cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
          i = cmd_sort_info[cmd_num].sort_pos;
          if (!str_cmp(help_table[mid].keyword, cmd_info[i].command)) {
            send_to_char("There is no help on that word.\r\n", ch);
            return;
          }
        }
      }
      page_string(ch->desc, help_table[mid].entry, 0);
      return;
    } else {
      if (chk > 0)
        bot = mid + 1;
      else
        top = mid - 1;
    }
  }
}


char *imm_abbr(struct char_data *ch)
{
  switch(GET_LEVEL(ch)) {
    case LVL_HERO:
      return(TITLE_HERO);
    case LVL_IMMORT:
      return(TITLE_IMMORT);
    case LVL_ASST:
      return(TITLE_ASST);
    case LVL_CIMP:
      return(TITLE_CIMP);
    case LVL_IMPL:
      return(TITLE_IMPL);
  }
  return("      ");
}


char *print_class_levels(struct char_data *ch)
{
  static char buf[80];
  int i;

  sprintf(buf, "%-3d%s", GET_LEVEL(ch), CLASS_ABBR(ch));
  for(i=0; i<NUM_CLASSES; i++) {
    if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i)) && (i != GET_CLASS(ch))) {
      sprintf(buf, "%s | %-3d%s", buf, GET_CLASS_LEVEL(ch, i), class_abbrevs[i]);
    }
  }
  return(buf);
}

#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-q] [-r] [-z] [-a]\r\n"

ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char name_search[MAX_INPUT_LENGTH];
  char whobuf[(max_players+1)*250];
  char mode;
  size_t i;
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
  int showclass = 0, short_list = 0, assassinwho = 0, num_can_see = 0;
  int who_room = 0;

  whobuf[0]=whobuf[1]=0;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);	/* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'a':
        assassinwho = 1;
        strcpy(buf, buf1);
        break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);
	break;
      case 's':
	short_list = 1;
	strcpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(WHO_FORMAT, ch);
	return;
	break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */

  if(short_list)
    send_to_char("Players\r\n-------\r\n", ch);
  else
    strcat(whobuf, "Players\r\n-------\r\n");

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected)
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (assassinwho && !PLR_FLAGGED(tch, PLR_ASSASSIN) && !PRF_FLAGGED(tch, PRF_REQ_ASS))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (showclass && !(showclass & GET_CLASS_BITVECTOR(tch)))
      continue;
    if (short_list) {
      if(GET_LEVEL(tch) >= LVL_HERO)
        sprintf(buf, "%s[%s] %-11.11s%s%s",
	      CCYEL(ch, C_SPR), imm_abbr(tch), GET_NAME(tch),
	      CCNRM(ch, C_SPR), ((!(++num_can_see % 4)) ? "\r\n" : ""));
      else
        sprintf(buf, "%s%c%-3d%s%c %-11.11s%s%s",
	      (PLR_FLAGGED(tch, PLR_ASSASSIN) ? CCRED(ch, C_SPR) : ""),
              ((GET_NUM_CLASSES(tch)>1)?'(':'['), GET_LEVEL(tch), CLASS_ABBR(tch),
              ((GET_NUM_CLASSES(tch)>1)?')':']'), GET_NAME(tch),
	      (PLR_FLAGGED(tch, PLR_ASSASSIN) ? CCNRM(ch, C_SPR) : ""),
	      ((!(++num_can_see % 4)) ? "\r\n" : ""));
      send_to_char(buf, ch);
    } else {
      num_can_see++;
      if(GET_LEVEL(tch) >= LVL_HERO)
        sprintf(buf, "%s[%s] %s %s", CCYEL(ch, C_SPR), imm_abbr(tch), GET_NAME(tch), GET_TITLE(tch));
      else
        sprintf(buf, "%s[%s] %s %s",
	      (PLR_FLAGGED(tch, PLR_ASSASSIN) ? CCRED(ch, C_SPR) : ""),
	      print_class_levels(tch), GET_NAME(tch),
	      GET_TITLE(tch));

      if (GET_INVIS_LEV(tch))
	sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
      else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	strcat(buf, " (invis)");

      if (PLR_FLAGGED(tch, PLR_MAILING))
	strcat(buf, " (mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	strcat(buf, " (writing)");

      if (PRF_FLAGGED(tch, PRF_DEAF))
	strcat(buf, " (deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	strcat(buf, " (notell)");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	strcat(buf, " (quest)");
      if (PRF_FLAGGED(tch, PRF_AFK))
	strcat(buf, " (AFK)");
      if (PLR_FLAGGED(tch, PLR_ASSASSIN))
	strcat(buf, " (assassin)");
      if (PRF_FLAGGED(tch, PRF_REQ_ASS))
	strcat(buf, " (req assassin)");
      if (GET_OLC_MODE(tch)) {
        if((GET_LEVEL(ch) < LVL_HERO) && (PLR_FLAGGED(tch, PLR_BUILDING)))
          strcat(buf, " (building)");
        else if(GET_LEVEL(ch) >= LVL_HERO) {
          if(PLR_FLAGGED(tch, PLR_BUILDING))
            strcat(buf, " (building)");
          switch(GET_OLC_MODE(tch)) {
          case OLC_MEDIT:
            sprintf(buf, "%s (med:%ld)", buf, GET_OLC_NUM(tch));
            break;
          case OLC_OEDIT:
            sprintf(buf, "%s (oed:%ld)", buf, GET_OLC_NUM(tch));
            break;
          case OLC_REDIT:
            sprintf(buf, "%s (red:%ld)", buf, GET_OLC_NUM(tch));
            break;
          case OLC_IEDIT:
            sprintf(buf, "%s (ied:%ld)", buf, GET_OLC_NUM(tch));
            break;
          case OLC_PEDIT:
            sprintf(buf, "%s (ped:%s)", buf, GET_NAME((struct char_data *)GET_OLC_PTR(tch)));
            break;
          case OLC_ZEDIT:
            sprintf(buf, "%s (zed:%ld)", buf, GET_OLC_NUM(tch));
            break;
          default:
            strcat(buf, " (unknown edit)");
            break;
          }
        }
      }
      if((tch->player_specials->zone_locked > 0) && (GET_LEVEL(ch) >= LVL_HERO))
        sprintf(buf, "%s (zlock:%d)", buf, tch->player_specials->zone_locked);
      if (tch->char_specials.timer >= 4)
	sprintf(buf, "%s (idle %d)", buf, tch->char_specials.timer*SECS_PER_MUD_HOUR/60);
      strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "\r\n");
      strcat(whobuf, buf);
    }				/* endif shortlist */
  }				/* end of for */
  if (short_list && (num_can_see % 4)) {
    send_to_char("\r\n", ch);
    if (num_can_see == 0)
      sprintf(buf, "\r\nNo-one at all!\r\n");
    else if (num_can_see == 1)
      sprintf(buf, "\r\nOne lonely character displayed.\r\n");
    else
      sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
    send_to_char(buf, ch);
  }
  else {
    if (num_can_see == 0)
      strcat(whobuf, "\r\nNo-one at all!\r\n");
    else if (num_can_see == 1)
      strcat(whobuf, "\r\nOne lonely character displayed.\r\n");
    else {
      sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
      strcat(whobuf, buf);
    }
    page_string(ch->desc, whobuf, 1);
  }
}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-d] [-p] [-s]\r\n"

ACMD(do_users)
{
  char line[220], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, *format, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  char usersbuf[(max_players+20)*220+20];
  long buf_size=(max_players+20)*220;
  int overflow=0;
  struct char_data *tch;
  struct descriptor_data *d;
  int low = 0, high = LVL_IMPL, num_can_see = 0, num_linkless=0;
  int showclass = 0, sort = 0, playing = 0, deadweight = 0;
  struct switch_list_type {
    struct char_data *ch;
    struct switch_list_type *next;
  } *switch_list=NULL, *sltemp;
  int ns1, ns2, ns3, ns4, i, j, k, s=0;
  char cs[6][HOST_LENGTH+1];
  struct {
    unsigned long ns;
    char cs[HOST_LENGTH+1];
    long offset;
    int length;
  } slist[max_players];

  usersbuf[0]=usersbuf[1]=0;

  host_search[0] = name_search[0] = '\0';

  for(i=0; i<max_players; i++) {
    slist[i].ns=0;
    slist[i].cs[0]=0;
    slist[i].offset=-1;
    slist[i].length=0;
  }
printf("users 1\n");
fflush(stdout);
  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 's':
	sort = 1;
	strcpy(buf, buf1);
	break;
      case 'p':
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'd':
	deadweight = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      case 'c':
	playing = 1;
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(USERS_FORMAT, ch);
	return;
	break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(USERS_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */
printf("users 2\n");
fflush(stdout);
  strcpy(line,
	 "Num Class    Name         State          Idl Login@   Site\r\n");
  strcat(line,
	 "--- -------- ------------ -------------- --- -------- ------------------------\r\n");
  strcat(usersbuf, line);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected && playing)
      continue;
    if (!d->connected && deadweight)
      continue;
    if (!d->connected) {
      if (d->original) {
	tch = d->original;
        CREATE(sltemp, struct switch_list_type, 1);
        sltemp->ch=tch;
        sltemp->next=switch_list;
        switch_list=sltemp;
      }
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (showclass && !(showclass & GET_CLASS_BITVECTOR(tch)))
	continue;
      if (GET_INVIS_LEV(tch) > GET_LEVEL(ch))
	continue;

      if (GET_LEVEL(tch)>=LVL_HERO)
	sprintf(classname, "[%s]", imm_abbr(tch));
      else
	sprintf(classname, "%c%-3d%s%c", ((GET_NUM_CLASSES(tch)>1) ? '(' : '['),
                GET_LEVEL(tch), CLASS_ABBR(tch), ((GET_NUM_CLASSES(tch)>1) ? ')' : ']'));
    } else
      strcpy(classname, "   -    ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (!d->connected && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[d->connected]);

    if (d->character && !d->connected)
      sprintf(idletime, "%3d", d->character->char_specials.timer *
	      SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      strcpy(idletime, "");

    format = "%3d %-7s %-12s %-14s %-3s %-8s ";

    if (d->character && d->character->player.name) {
      if (d->original)
	sprintf(line, format, d->desc_num, classname,
		d->original->player.name, state, idletime, timeptr);
      else
	sprintf(line, format, d->desc_num, classname,
		d->character->player.name, state, idletime, timeptr);
    } else
      sprintf(line, format, d->desc_num, "   -    ", "UNDEFINED",
	      state, idletime, timeptr);

    if(d->original)
      tch=d->original;
    else
      tch=d->character;
    if (d->host && *d->host && (!tch || (GET_LEVEL(ch) >= GET_LEVEL(tch))))
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
    else
      strcat(line, "[Hostname unknown]\r\n");

    if (d->connected) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }
    if (d->connected || (!d->connected && CAN_SEE(ch, tch))) {
      if(overflow||((strlen(usersbuf)+strlen(line)+1) > buf_size)) {
        if(!overflow) {
          overflow=1;
          strcat(usersbuf, "***OVERFLOW***");
        }
      }
      else {
        if(sort) {
          slist[s].offset=strlen(usersbuf);
          slist[s].length=strlen(line);
          if (d->host && *d->host && (!tch || (GET_LEVEL(ch) >= GET_LEVEL(tch)))) {
            ns1=ns2=ns3=ns4=-1;
            sscanf(d->host, "%d.%d.%d.%d", &ns4, &ns3, &ns2, &ns1);
            if((ns1==-1)||(ns2==-1)||(ns3==-1)||(ns4==-1))
              strcpy(slist[s].cs, d->host);
            else
              slist[s].ns=(((unsigned long)ns4)<<24)+(((unsigned long)ns3)<<16)+(((unsigned long)ns2)<<8)+(unsigned long)ns1;
          }
          else {
            slist[s].ns=4294967295LU;
            sscanf(d->host, "%s.%s.%s.%s.%s.%s", cs[0], cs[1], cs[2], cs[3], cs[4], cs[5]);
            for(j=5; j>=0; j--) {
              if(cs[j][0]) {
                if(slist[s].cs[0])
                  strcat(slist[s].cs, ".");
                strcat(slist[s].cs, cs[j]);
              }
            }
          }
          s++;
        }
        strcat(usersbuf, line);
      }
      num_can_see++;
    }
  }
printf("users 3\n");
fflush(stdout);
  if(sort&&(!overflow)) {
    unsigned long nstemp;
    long offtemp;
    int ltemp;
    char cstemp[HOST_LENGTH+1];
    char sort_buf[(max_players+20)*220+20];
printf("users 3.1\n");
fflush(stdout);
    for(i=0; i<s; i++) {
      for(j=i; (j>0)&&(slist[j].ns<slist[j-1].ns); j--) {
        nstemp=slist[j-1].ns;
        offtemp=slist[j-1].offset;
        ltemp=slist[j-1].length;
        strcpy(cstemp, slist[j-1].cs);
        slist[j-1].ns=slist[j].ns;
        slist[j-1].offset=slist[j].offset;
        slist[j-1].length=slist[j].length;
        strcpy(slist[j-1].cs, slist[j].cs);
        slist[j].ns=nstemp;
        slist[j].offset=offtemp;
        slist[j].length=ltemp;
        strcpy(slist[j].cs, cstemp);
      }
    }
printf("users 3.2\n");
fflush(stdout);
    for(i=0; (i<s)&&(slist[i].cs[0]); i++) {
      for(j=i; (j>0)&&(str_cmp(slist[j].cs, slist[j-1].cs)<0); j--) {
        nstemp=slist[j-1].ns;
        offtemp=slist[j-1].offset;
        ltemp=slist[j-1].length;
        strcpy(cstemp, slist[j-1].cs);
        slist[j-1].ns=slist[j].ns;
        slist[j-1].offset=slist[j].offset;
        slist[j-1].length=slist[j].length;
        strcpy(slist[j-1].cs, slist[j].cs);
        slist[j].ns=nstemp;
        slist[j].offset=offtemp;
        slist[j].length=ltemp;
        strcpy(slist[j].cs, cstemp);
      }
    }
printf("users 3.3\n");
fflush(stdout);
    strcpy(sort_buf, usersbuf);
    k=0;
printf("users 3.4\n");
fflush(stdout);
    for(i=0; i<s; i++) {
      for(j=0; j<slist[i].length; j++) {
        usersbuf[k++]=sort_buf[slist[i].offset+j];
      }
    }
printf("users 3.5\n");
fflush(stdout);
    usersbuf[k]=0;
  }
printf("users 4\n");
fflush(stdout);
  for(tch=character_list; tch; tch=tch->next) {
    if((!IS_NPC(tch))&&(!tch->desc)) {
      for(sltemp=switch_list; sltemp; sltemp=sltemp->next) {
        if(tch==sltemp->ch)
          break;
      }
      if(!sltemp) {
        if (GET_LEVEL(tch)>=LVL_HERO)
          sprintf(classname, "[%s]", imm_abbr(tch));
        else
          sprintf(classname, "%c%-3d%s%c", ((GET_NUM_CLASSES(tch)>1) ? '(' : '['),
                  GET_LEVEL(tch), CLASS_ABBR(tch), ((GET_NUM_CLASSES(tch)>1) ? ')' : ']'));
        strcpy(state, "Linkless");
        sprintf(idletime, "%3d", tch->char_specials.timer *
                SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);

        format = "%3s %-7s %-12s %-14s %-3s %-8s ";

        sprintf(line2, format, " - ", classname, GET_NAME(tch), state, idletime, "   -    ");
        sprintf(line, "%s%s%s", CCGRN(ch, C_SPR), line2, CCNRM(ch, C_SPR));
        if((tch->in_room==NOWHERE) || CAN_SEE(ch, tch)) {
          if(overflow||((strlen(usersbuf)+strlen(line)+3) > buf_size)) {
            if(!overflow) {
              overflow=1;
              strcat(usersbuf, "***OVERFLOW***");
            }
          }
          else {
            strcat(usersbuf, "\r\n");
            strcat(usersbuf, line);
          }
          num_linkless++;
        }
      }
    }
  }
printf("users 5\n");
fflush(stdout);
  sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
  if(overflow||((strlen(usersbuf)+strlen(line)+1) > buf_size)) {
    if(!overflow) {
      overflow=1;
      strcat(usersbuf, "***OVERFLOW***");
    }
  }
  else {
    strcat(usersbuf, line);
  }
printf("users 6\n");
fflush(stdout);
  sprintf(line, "\r\n%d linkless characters in the game.\r\n", num_linkless);
  if(overflow||((strlen(usersbuf)+strlen(line)+1) > buf_size)) {
    if(!overflow) {
      overflow=1;
      strcat(usersbuf, "***OVERFLOW***");
    }
  }
  else {
    strcat(usersbuf, line);
  }
printf("users 7\n");
fflush(stdout);
  page_string(ch->desc, usersbuf, 1);

  while(switch_list) {
    sltemp=switch_list->next;
    free(switch_list);
    switch_list=sltemp;
  }
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  extern char circlemud_version[];

  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    page_string(ch->desc, wizlist, 0);
    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    page_string(ch->desc, motd, 0);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_CLEAR:
    send_to_char("\033[H\033[J", ch);
    break;
  case SCMD_VERSION:
    send_to_char(circlemud_version, ch);
    break;
  case SCMD_WHOAMI:
    send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
    break;
  default:
    return;
    break;
  }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct descriptor_data *d;
  char wherebuf[110*(max_players+1)];

  wherebuf[0]=wherebuf[1]=0;

  if (!*arg) {
    strcat(wherebuf, "Players in your Zone\r\n--------------------\r\n");
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
	    (world[ch->in_room].zone == world[i->in_room].zone)) {
	  sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
          strcat(wherebuf, buf);
	}
      }
      page_string(ch->desc, wherebuf, 1);    
  } else {			/* print only FIRST char, not all. */
    for (i = character_list; i; i = i->next)
      if (world[i->in_room].zone == world[ch->in_room].zone && CAN_SEE(ch, i) &&
	  (i->in_room != NOWHERE) && isname(arg, i->player.name)) {
	sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	send_to_char(buf, ch);
	return;
      }
    send_to_char("No-one around by that name.\r\n", ch);
  }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
			        int recur, char **wbuf)
{
  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%33s", " - ");

  if (obj->in_room > NOWHERE) {
    sprintf(buf + strlen(buf), "[%5d] %s\n\r",
	    world[obj->in_room].number, world[obj->in_room].name);
    RECREATE(*wbuf, char, strlen(*wbuf)+strlen(buf)+1);
    strcat(*wbuf, buf);
  } else if (obj->carried_by) {
    sprintf(buf + strlen(buf), "carried by %s\n\r",
	    PERS(obj->carried_by, ch));
    RECREATE(*wbuf, char, strlen(*wbuf)+strlen(buf)+1);
    strcat(*wbuf, buf);
  } else if (obj->worn_by) {
    sprintf(buf + strlen(buf), "worn by %s\n\r",
	    PERS(obj->worn_by, ch));
    RECREATE(*wbuf, char, strlen(*wbuf)+strlen(buf)+1);
    strcat(*wbuf, buf);
  } else if (obj->in_obj) {
    sprintf(buf + strlen(buf), "inside %s%s\n\r",
	    obj->in_obj->short_description, (recur ? ", which is" : " "));
    RECREATE(*wbuf, char, strlen(*wbuf)+strlen(buf)+1);
    strcat(*wbuf, buf);
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur, wbuf);
  } else {
    sprintf(buf + strlen(buf), "in an unknown location\n\r");
    RECREATE(*wbuf, char, strlen(*wbuf)+strlen(buf)+1);
    strcat(*wbuf, buf);
  }
}



void perform_immort_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;
  char *wbuf;

  if (!*arg) {
    wbuf=str_dup("Players\r\n-------\r\n");
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
	  if (d->original)
	    sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
		    GET_NAME(i), world[d->character->in_room].number,
		 world[d->character->in_room].name, GET_NAME(d->character));
	  else
	    sprintf(buf, "%-20s - [%5d] %s\r\n", GET_NAME(i),
		    world[i->in_room].number, world[i->in_room].name);
          RECREATE(wbuf, char, strlen(wbuf)+strlen(buf)+1);
          strcat(wbuf, buf);
	}
      }
    page_string(ch->desc, wbuf, 1);
  } else {
    wbuf=str_dup(" ");
    wbuf[0]=0;
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
	sprintf(buf, "M%3d. %-25s - [%5d] %s\r\n", ++num, GET_NAME(i),
		world[i->in_room].number, world[i->in_room].name);
        RECREATE(wbuf, char, strlen(wbuf)+strlen(buf)+1);
        strcat(wbuf, buf);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
	found = 1;
	print_object_location(++num, k, ch, TRUE, &wbuf);
      }
    if (!found)
      send_to_char("Couldn't find any such thing.\r\n", ch);
    else
      page_string(ch->desc, wbuf, 1);
  }
  free(wbuf);
}



ACMD(do_where)
{
  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_HERO)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
    return;
  }
  *buf = '\0';

  for (i = 1; i < LVL_HERO; i++) {
    sprintf(buf + strlen(buf), "[%3d] %9ld-%-9ld\r\n", i,
	    GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][i]+1, GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][i + 1]);
  }
  page_string(ch->desc, buf, 1);
}



ACMD(do_consider)
{
  struct char_data *victim;
  int diff;

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("Consider killing who?\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("Easy!  Very easy indeed!\r\n", ch);
    return;
  }
  if (!IS_NPC(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\r\n", ch);
    return;
  }
  diff = ((GET_LEVEL(victim) - GET_LEVEL(ch))/(2.0-(((float)LVL_HERO-GET_LEVEL(ch))/LVL_HERO)));

  if (diff <= -10)
    send_to_char("Now where did that chicken go?\r\n", ch);
  else if (diff <= -5)
    send_to_char("You could do it with a needle!\r\n", ch);
  else if (diff <= -2)
    send_to_char("Easy.\r\n", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\r\n", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\r\n", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\r\n", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\r\n", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\r\n", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\r\n", ch);
  else if (diff <= 10)
    send_to_char("Are you mad!?\r\n", ch);
  else
    send_to_char("You ARE mad!\r\n", ch);

}



ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    } else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("Diagnose who?\r\n", ch);
  }
}


static char *ctypes[] = {
"off", "sparse", "normal", "complete", "\n"};

ACMD(do_color)
{
  int tp;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
  SET_BIT(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

  sprintf(buf, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR),
	  CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "OFF");
  else
    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

  sprintf(buf,
	  "        Mortlog: %-3s    "
	  "     Brief Mode: %-3s    "
	  " Summon Protect: %-3s\r\n"

	  "  Arena Channel: %-3s    "
	  "   Compact Mode: %-3s    "
	  "       On Quest: %-3s\r\n"

	  "  Music Channel: %-3s    "
	  "         NoTell: %-3s    "
	  "   Repeat Comm.: %-3s\r\n"

	  " Auto Show Exit: %-3s    "
	  "           Deaf: %-3s    "
	  "     Wimp Level: %-3s\r\n"

	  " Gossip Channel: %-3s    "
	  "Auction Channel: %-3s    "
	  "  Grats Channel: %-3s\r\n"

	  "Spanish Channel: %-3s    "
	  " French Channel: %-3s    "
	  "            AFK: %-3s\r\n"

	  "       Autoloot: %-3s    "
	  "       Autogold: %-3s    "
	  "      Autosplit: %-3s\r\n"

	  "  Combine items: %-3s\r\n"

          "    Page Length: %-3d    "
	  "    Color Level: %s",

	  ONOFF(PRF_FLAGGED(ch, PRF_MORTLOG)),
	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOARENA)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOMUS)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOEXITS)),
	  YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
	  buf2,

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOESP)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOFRAN)),
	  YESNO(PRF_FLAGGED(ch, PRF_AFK)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOGOLD)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOSPLIT)),

          YESNO(PRF_FLAGGED(ch, PRF_COMBINE)),

          PAGE_LEN(ch),
	  ctypes[COLOR_LEV(ch)]);

  send_to_char(buf, ch);
}



void sort_commands(void)
{
  int a, b, tmp;

  ACMD(do_action);

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  (socials ? "socials" : "commands"),
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if ((cmd_info[i].minimum_level >= 0) &&
	((GET_LEVEL(vict) >= cmd_info[i].minimum_level) ||
         GRNT_FLAGGED(vict, cmd_info[i].grant)) &&
	(cmd_info[i].minimum_level >= LVL_HERO) == wizhelp &&
	(wizhelp || socials == cmd_sort_info[i].is_social)) {
      sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
      if (!(no % 7))
	strcat(buf, "\r\n");
      no++;
    }
  }

  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}

ACMD(do_memory)
{
  int i, found=0;

  for(i=0; i<MAX_REMEMBER; i++) {
    if((GET_REMEMBER(ch, i)>0)&&(real_room(GET_REMEMBER(ch, i))>0)) {
      if(!found) {
        send_to_char("Locations memorized:\r\n", ch);
        found=1;
      }
      sprintf(buf, " %d.  %s\r\n", i+1, world[real_room(GET_REMEMBER(ch, i))].name);
      send_to_char(buf, ch);
    }
  }

  if(!found)
    send_to_char("You haven't memorized any places, yet.\r\n", ch);
}
