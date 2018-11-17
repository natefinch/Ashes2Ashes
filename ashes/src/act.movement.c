/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[];
extern char *abbr_dirs[];
extern char *edirs[];
extern int movement_loss[];

/* external functs */
int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
void mprog_entry_trigger(struct char_data *mob);
void mprog_greet_trigger(struct char_data *ch);
int mag_savingthrow(struct char_data * ch, int type);


/* Checks if a character will fall (no fly in a fly room) and does the falling */
void check_fall(struct char_data *ch, struct obj_data *obj)
{
  int fall_distance, room;
  char buf[MAX_INPUT_LENGTH];

  if(ch) {
    if(ROOM_FLAGGED(ch->in_room, ROOM_FLY) && (!AFF_FLAGGED(ch, AFF_FLY)) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_HERO))) {
      SET_BIT(AFF_FLAGS(ch), AFF_FLY);
      act("$n falls!", TRUE, ch, NULL, NULL, TO_ROOM);
      for(fall_distance=0; CAN_GO(ch, DOWN) && do_simple_move(ch, DOWN, TRUE); fall_distance++) {
        if(fall_distance > 19)
          break;
        act("$n falls by you!", TRUE, ch, NULL, NULL, TO_ROOM);
      }
      REMOVE_BIT(AFF_FLAGS(ch), AFF_FLY);
      if(CAN_GO(ch, DOWN) && (fall_distance > 19))
        send_to_char("A merciful god reaches down and stops your fall... for now.\r\n", ch);
      else {
        send_to_char("You crash into the ground with a heavy THUD!\r\n", ch);
        act("$n crashes into the ground with a heavy THUD!", FALSE, ch, NULL, NULL, TO_ROOM);
      }
      damage(ch, ch, dice(4*fall_distance, 20), TYPE_SILENT);
    }
  }
  if(obj) {
    if(ROOM_FLAGGED(obj->in_room, ROOM_FLY)) {
      sprintf(buf, "%s falls.\r\n", obj->short_description);
      send_to_room(CAP(buf), obj->in_room);
      room=obj->in_room;
      sprintf(buf, "%s falls past you.\r\n", obj->short_description);
      for(fall_distance=0; world[room].dir_option[DOWN] &&
                           (world[room].dir_option[DOWN]->to_room != NOWHERE) &&
                           !IS_SET(world[room].dir_option[DOWN]->exit_info, EX_CLOSED); fall_distance++) {
        room=world[room].dir_option[DOWN]->to_room;
        if(fall_distance > 24)
          break;
        send_to_room(CAP(buf), room);
      }
      if(!ROOM_FLAGGED(room, ROOM_FLY)) {
        obj_from_room(obj);
        sprintf(buf, "%s lands with a THUD at your feet.\r\n", obj->short_description);
        send_to_room(CAP(buf), room);
        obj_to_room(obj, room);
      }
    }
  }
}


/* Damages a char by the DT of the room they're in, returns 1 if they die, 0 otherwise */
int dt_damage(struct char_data *ch)
{
  int dam;
  if(!ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    return 0;

  dam=MAX(0, MAX(((long)world[ch->in_room].dt_percent*GET_MAX_HIT(ch))/100, dice(world[ch->in_room].dt_numdice, world[ch->in_room].dt_sizedice)+world[ch->in_room].dt_add));

  if ((AFF_FLAGGED(ch, AFF_FLY) && (!AFF_FLAGGED(ch, AFF_SANCTUARY))) || mag_savingthrow(ch, SAVING_ROD))
    dam >>= 1;

  dam=MAX(1, dam);

  log_death_trap(ch);

  damage(ch, ch, dam, TYPE_DT);
  if(ch->in_room==NOWHERE)
    return 1;
  return 0;
}


/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data *obj;
  int i;

  if(GET_LEVEL(ch) >= LVL_HERO)
    return 1;

  if (IS_AFFECTED(ch, AFF_WATERWALK))
    return 1;

  if(AFF_FLAGGED(ch, AFF_FLY))
    return 1;

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return 1;

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return 1;

  return 0;
}
  

/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */
int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in, need_movement;

  int special(struct char_data *ch, int cmd, char *arg);

  /* Check for locked and closed flags */
  if((zone_table[world[ch->in_room].zone].closed || zone_table[world[EXIT(ch, dir)->to_room].zone].closed) &&
     ((!IS_NPC(ch))&&(GET_LEVEL(ch) < LVL_HERO))) {
    send_to_char("Sorry, but that area is closed for construction.\r\n", ch);
    return 0;
  }
  if(((zone_table[world[ch->in_room].zone].locked_by[0]) || (zone_table[world[EXIT(ch, dir)->to_room].zone].locked_by[0])) &&
     (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_HERO))) {
    send_to_char("The power of an immortal prevents you from moving there.\r\n", ch);
    return 0;
  }
  if(IS_NPC(ch) && (zone_table[world[ch->in_room].zone].closed != zone_table[world[EXIT(ch, dir)->to_room].zone].closed)) {
    send_to_char("Silly mob, stay where you belong.\r\n", ch);
    return 0;
  }

  /* Don't let them leave if they're in the arena via challenge */
  if((!IS_NPC(ch)) && ARENA(ch).kill_mode && (zone_table[world[EXIT(ch, dir)->to_room].zone].number!=ARENA_ZONE)) {
    send_to_char("If you want to abandon your challenge, you have to retire.\r\n", ch);
    return 0;
  }

  /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
  if (need_specials_check && special(ch, dir + 1, ""))
    return 0;

  /* charmed? */
  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room) {
    send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }

  /* move points needed is avg. move loss for src and destination sect type */
  need_movement = (movement_loss[SECT(ch->in_room)] +
		   movement_loss[SECT(EXIT(ch, dir)->to_room)]) >> 1;

  if(AFF_FLAGGED(ch, AFF_SNEAK)) {
    need_movement /= 2.0-(((float)LVL_HERO-GET_LEVEL(ch))/LVL_HERO);
  }

  /* if this room or the one we're going to needs a boat, check for one */
  if ((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) {
    if (!has_boat(ch)) {
      send_to_char("You need a boat to go there.\r\n", ch);
      return 0;
    }
    if(AFF_FLAGGED(ch, AFF_WATERWALK))
      need_movement=1;
  }
  if ((SECT(ch->in_room) == SECT_WATER_SWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_SWIM)) {
    if (!has_boat(ch)) {
      if((!IS_NPC(ch)) && get_skill(ch, SKILL_SWIM)) {
        if(number(1, 100)>get_skill(ch, SKILL_SWIM)) {
          send_to_char("You fail to swim there.\r\n", ch);
          return 0;
        }
      }
      else {
        send_to_char("You need a boat to go there.\r\n", ch);
        return 0;
      }
    }
    if(AFF_FLAGGED(ch, AFF_WATERWALK))
      need_movement=1;
  }

  if(AFF_FLAGGED(ch, AFF_FLY)) {
    need_movement=0;
  }

  if(GET_LEVEL(ch) >= LVL_HERO)
    need_movement=0;

  if(ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_FLY) && (!AFF_FLAGGED(ch, AFF_FLY)) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_HERO))) {
    send_to_char("You must be flying to go there.\r\n", ch);
    return 0;
  }

  if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) {
    if (need_specials_check && ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);

    return 0;
  }
  if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 1) {
    send_to_char("There isn't enough room there for more than two people!\r\n", ch);
    return 0;
  }
  if (!IS_NPC(ch))
    GET_MOVE(ch) -= need_movement;

  if (!IS_AFFECTED(ch, AFF_SNEAK)) {
    if(IS_NPC(ch))
      sprintf(buf2, "$n leaves %s.", dirs[dir]);
    else
      sprintf(buf2, "$n %s %s.", WALKOUT(ch), dirs[dir]);
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }
  was_in = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);

  if (!IS_AFFECTED(ch, AFF_SNEAK)) {
    if(IS_NPC(ch))
      sprintf(buf2, "$n enters from %s.", edirs[dir]);
    else
      sprintf(buf2, "$n %s from %s.", WALKIN(ch), edirs[dir]);
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }

  if (ch->desc != NULL)
    look_at_room(ch, 0);

  mprog_entry_trigger(ch);
  /* if they died, return now so we don't do stuff to them dead */
  if(ch->in_room==NOWHERE)
    return 1;

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && (IS_NPC(ch) || PRF_FLAGGED(ch, PRF_AVTR) || (GET_LEVEL(ch) < LVL_HERO)))
  {
    /* if they died, return now so we don't do stuff to them dead */
    if(dt_damage(ch))
      return 1;
  }

  mprog_greet_trigger(ch);
  return 1;
}


int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in;
  struct follow_type *k, *next;

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS)
    return 0;
  else if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
    send_to_char("Alas, you cannot go that way...\r\n", ch);
  else if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
    if(str_cmp(fname(EXIT(ch, dir)->keyword), "secret") && (!IS_SET(EXIT(ch, dir)->exit_info, EX_SECRET))) {
      if (EXIT(ch, dir)->keyword) {
        sprintf(buf2, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));
        send_to_char(buf2, ch);
      } else
        send_to_char("It seems to be closed.\r\n", ch);
    }
    else {
      send_to_char("Alas, you cannot go that way...\r\n", ch);
    }
  } else {
    if (!ch->followers)
      return (do_simple_move(ch, dir, need_specials_check));

    was_in = ch->in_room;
    if (!do_simple_move(ch, dir, need_specials_check))
      return 0;

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((k->follower->in_room == was_in) &&
	  (GET_POS(k->follower) >= POS_STANDING)) {
	act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
	perform_move(k->follower, dir, 1);
      }
    }
    return 1;
  }
  return 0;
}


ACMD(do_move)
{
  if(GET_COND(ch, DRUNK) > 0) {
    if(GET_COND(ch, DRUNK) > number(1, 100)) {
      if(GET_POS(ch) >= POS_STANDING) {
        if(GET_COND(ch, DRUNK) < 15)
          send_to_char("You stumble.\r\n", ch);
        else {
          send_to_char("You stumble and fall.\r\n", ch);
          GET_POS(ch)=POS_SITTING;
          return;
        }
      }
    }
  }

  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, cmd - 1, 0);
}


int find_door(struct char_data *ch, char *type, char *dir, char *cmdname)
{
  int door;

  if (*dir) {			/* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
      if ((door = search_block(dir, abbr_dirs, FALSE)) == -1) {	/* Partial Match */
        send_to_char("That's not a direction.\r\n", ch);
        return -1;
      }
    }
    if (EXIT(ch, door))
      if (EXIT(ch, door)->keyword)
	if (isname(type, EXIT(ch, door)->keyword))
	  return door;
	else {
	  sprintf(buf2, "I see no %s there.\r\n", type);
	  send_to_char(buf2, ch);
	  return -1;
      } else
	return door;
    else {
      send_to_char("I really don't see how you can close anything there.\r\n", ch);
      return -1;
    }
  } else {			/* try to locate the keyword */
    if (!*type) {
      sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
      send_to_char(buf2, ch);
      return -1;
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (isname(type, EXIT(ch, door)->keyword))
	    return door;

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return -1;
  }
}


int has_key(struct char_data *ch, int key)
{
  int i;
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return 1;

  for(i=0; i<NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      if (GET_OBJ_VNUM(GET_EQ(ch, i)) == key)
        return 1;

  return 0;
}



#define NEED_OPEN	1
#define NEED_CLOSED	2
#define NEED_UNLOCKED	4
#define NEED_LOCKED	8

char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)		(world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  int other_room = 0;
  struct room_direction_data *back = 0;

  sprintf(buf, "$n %ss ", cmd_door[scmd]);
  if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
    if ((back = world[other_room].dir_option[rev_dir[door]]))
      if (back->to_room != ch->in_room)
	back = 0;

  switch (scmd) {
  case SCMD_OPEN:
  case SCMD_CLOSE:
    OPEN_DOOR(ch->in_room, obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(OK, ch);
    break;
  case SCMD_UNLOCK:
  case SCMD_LOCK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("*Click*\r\n", ch);
    break;
  case SCMD_PICK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("The lock quickly yields to your skills.\r\n", ch);
    strcpy(buf, "$n skillfully picks the lock on ");
    break;
  }

  /* Notify the room */
  sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
	  (EXIT(ch, door)->keyword ? "$F" : "door"));
  if (!(obj) || (obj->in_room != NOWHERE))
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

  /* Notify the other room */
  if ((scmd == SCMD_OPEN || scmd == SCMD_CLOSE) && back) {
    sprintf(buf, "The %s is %s%s from the other side.\r\n",
	 (back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
	    (scmd == SCMD_CLOSE) ? "d" : "ed");
    if (world[EXIT(ch, door)->to_room].people) {
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
    }
  }
}


int ok_pick(struct char_data *ch, int keynum, int pickproof, int scmd)
{
  int percent;

  percent = number(1, 100);

  if (scmd == SCMD_PICK) {
    if (!get_skill(ch, SKILL_PICK_LOCK))
      send_to_char("You can't do that.\r\n", ch);
    else if (keynum < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (pickproof)
      send_to_char("It resists your attempts to pick it.\r\n", ch);
    else if (percent > get_skill(ch, SKILL_PICK_LOCK))
      send_to_char("You failed to pick the lock.\r\n", ch);
    else
      return (1);
    return (0);
  }
  return (1);
}


#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))) :\
			(IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF)) : \
			(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)))

#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
  int door = -1, keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
    send_to_char(CAP(buf), ch);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_OPEN))
      send_to_char("But it's already closed!\r\n", ch);
    else if (!DOOR_IS_CLOSED(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_CLOSED))
      send_to_char("But it's currently open!\r\n", ch);
    else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char("It seems to be locked.\r\n", ch);
    else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_HERO) &&
	     ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
      do_doorcmd(ch, obj, door, subcmd);
  }
  return;
}



ACMD(do_enter)
{
  int door;
  struct char_data *tch;
  struct spcontinuous *s;

  one_argument(argument, buf);

  if (*buf) {			/* an argument was supplied, search for door
				 * keyword */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    sprintf(buf2, "There is no %s here.\r\n", buf);
    send_to_char(buf2, ch);
  }
  else {
    if (ROOM_FLAGGED(ch->in_room, ROOM_DIM_DOOR)) {
      for(tch=character_list; tch; tch=tch->next) {
        if(!IS_NPC(tch)) {
          for(s=tch->char_specials.spcont; s; s=s->next) {
            if(s->spspell==SKILL_DIMENSION_DOOR) {
              if(s->spdata1==ch->in_room) {
                act("$n steps through the portal and vanishes.", TRUE, ch, NULL, NULL, TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, s->spdata2);
                act("$n steps out of the portal.", TRUE, ch, NULL, NULL, TO_ROOM);
                look_at_room(ch, 0);
                break;
              }
              if(s->spdata2==ch->in_room) {
                act("$n steps through the portal and vanishes.", TRUE, ch, NULL, NULL, TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, s->spdata1);
                act("$n steps out of the portal.", TRUE, ch, NULL, NULL, TO_ROOM);
                look_at_room(ch, 0);
                break;
              }
            }
          }
          if(s)
            break;
        }
      }
      return;
    }
    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
      send_to_char("You are already indoors.\r\n", ch);
    else {
      /* try to locate an entrance */
      for (door = 0; door < NUM_OF_DIRS; door++)
        if (EXIT(ch, door))
          if (EXIT(ch, door)->to_room != NOWHERE)
            if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
                IS_SET(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
              perform_move(ch, door, 1);
              return;
            }
      send_to_char("You can't seem to find anything to enter.\r\n", ch);
    }
  }
}


ACMD(do_leave)
{
  int door;

  if (!IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	    !IS_SET(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SITTING:
    act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_RESTING:
    act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_SLEEPING:
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
      send_to_char("You can't wake up!\r\n", ch);
    }
    else {
      act("You wake up, and stand up.", FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      act("$n wakes up, and stands up.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_STANDING;
    }
    break;
  case POS_FIGHTING:
    act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and put your feet on the ground.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and puts $s feet on the ground.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}


ACMD(do_sit)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
      send_to_char("You can't wake up!\r\n", ch);
    }
    else {
      act("You wake up, and sit up.", FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      act("$n wakes up, and sits up.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
    }
    break;
  case POS_FIGHTING:
    act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SLEEPING:
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
      send_to_char("You can't wake up!\r\n", ch);
    }
    else {
      act("You wake up.", FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      act("$n wakes up.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_RESTING;
    }
    break;
  case POS_FIGHTING:
    act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and stop to rest your tired bones.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_sleep)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    act("You stop floating around, and lie down to sleep.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  struct char_data *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("Maybe you should wake yourself up first.\r\n", ch);
    else if ((vict = get_char_room_vis(ch, arg)) == NULL)
      send_to_char(NOPERSON, ch);
    else if (vict == ch)
      self = 1;
    else if (GET_POS(vict) > POS_SLEEPING)
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (IS_AFFECTED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (IS_AFFECTED(ch, AFF_SLEEP))
    send_to_char("You can't wake up!\r\n", ch);
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else {
    send_to_char("You awaken.\r\n", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
  }
}


ACMD(do_follow)
{
  struct char_data *leader;

  void stop_follower(struct char_data *ch);
  void add_follower(struct char_data *ch, struct char_data *leader);

  one_argument(argument, buf);

  if (*buf) {
    if (!(leader = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Whom do you wish to follow?\r\n", ch);
    return;
  }

  if (ch->master == leader) {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  } else {			/* Not Charmed follow person */
    if (leader == ch) {
      if (!ch->master) {
	send_to_char("You are already following yourself.\r\n", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
	act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
      if (ch->master)
	stop_follower(ch);
      if(ARENA(ch).challenged_by == -1)
        ARENA(ch).challenged_by=0;
      if((ch->master&&(ARENA(ch).challenging==(GET_IDNUM(ch->master)+1)))||(ARENA(ch).challenging==(GET_IDNUM(ch)+1)))
        ARENA(ch).challenging=0;
      REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
      add_follower(ch, leader);
    }
  }
}

ACMD(do_walkset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_WALKIN:    msg = &(WALKIN(ch));    break;
  case SCMD_WALKOUT:   msg = &(WALKOUT(ch));   break;
  default:    return;    break;
  }

  skip_spaces(&argument);

  if (!*argument)
  {
    send_to_char(*msg, ch);
    send_to_char("\r\n", ch);
  }
  else
  {
    if(strlen(argument) > MAX_WALK_LENGTH) {
      sprintf(buf, "Your %s can be no longer than %d characters.\r\n",
              (subcmd==SCMD_WALKIN ? "walkin" : "walkout"), MAX_WALK_LENGTH);
      send_to_char(buf, ch);
      return;
    }
    free(*msg);
    *msg = str_dup(argument);
    send_to_char(OK, ch);
  }

}
