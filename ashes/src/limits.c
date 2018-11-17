/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "class.h"
#include "interpreter.h"


extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern int max_exp_gain;
extern int max_exp_loss;
extern struct index_data *obj_index;	/* in db.c */
extern char *pc_class_types[];

void die(struct char_data * ch, struct char_data *killer);

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);		/* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
  else
    return (p6);		/* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain, tmpgain=0, stat_bonus=0;
  int class_add=0, class_mul=0, i;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = graf(age(ch).year, 4, 8, 12, 16, 20, 24, 28);

    /* Affectation calculations */
    gain += GET_MANA_REGEN_ADD(ch);

    /* Stat calculations */
    if(GET_INT(ch) > 14)
      stat_bonus+=GET_INT(ch)-14;
    if(GET_INT(ch) < 8)
      stat_bonus+=GET_INT(ch)-8;
    if(GET_WIS(ch) > 14)
      stat_bonus+=GET_WIS(ch)-14;
    if(GET_WIS(ch) < 8)
      stat_bonus+=GET_WIS(ch)-8;
    gain += stat_bonus/2;

    /* Class calculations */
    for(i=0; i < NUM_CLASSES; i++) {
      if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i))) {
        class_add = level_params[MANA_REGEN_BASE][i] + GET_CLASS_LEVEL(ch, i)/level_params[MANA_REGEN_DIV][i];
        class_mul = level_params[MANA_REGEN_MUL][i];
        tmpgain += (gain+class_add)*class_mul;
      }
    }
    gain = tmpgain/GET_NUM_CLASSES(ch);

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain <<= 1;
      break;
    case POS_RESTING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_SITTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    }
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  return (gain);
}


int hit_gain(struct char_data * ch)
/* Hitpoint gain pr. game hour */
{
  int gain, i, class_add=0;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
    gain += ch->char_specials.hp_regen_add;
    /* Neat and fast */
  } else {

    gain = graf(age(ch).year, 8, 12, 20, 32, 16, 10, 4);

    /* Affectation calculations */
    gain += GET_HP_REGEN_ADD(ch);

    /* Class/Level calculations */
    for(i=0; i < NUM_CLASSES; i++) {
      if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i))) {
        class_add += level_params[HP_REGEN_BASE][i] + GET_CLASS_LEVEL(ch, i)/level_params[HP_REGEN_DIV][i];
      }
    }
    class_add /= GET_NUM_CLASSES(ch);
    gain += class_add;

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain <<= 1;
      break;
    case POS_RESTING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_SITTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    }
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  return (gain);
}



int move_gain(struct char_data * ch)
/* move gain pr. game hour */
{
  int gain, i, class_add=0;

  if (IS_NPC(ch)) {
    return (GET_LEVEL(ch));
    /* Neat and fast */
  } else {
    gain = graf(age(ch).year, 16, 20, 24, 20, 16, 12, 10);

    /* Affectation calculations */
    gain += GET_MOVE_REGEN_ADD(ch);

    /* Class/Level calculations */
    for(i=0; i < NUM_CLASSES; i++) {
      if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i))) {
        class_add += level_params[MOVE_REGEN_BASE][i] + GET_CLASS_LEVEL(ch, i)/level_params[MOVE_REGEN_DIV][i];
      }
    }
    class_add /= GET_NUM_CLASSES(ch);
    gain += class_add;

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);	/* Divide by 8 */
      break;
    }
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  return (gain);
}



void set_title(struct char_data * ch, char *title)
{
  if (title == NULL)
  {
    if (GET_TITLE(ch) != NULL)
      free(GET_TITLE(ch));
    CREATE(GET_TITLE(ch), char, MAX_TITLE_LENGTH+1);
    sprintf(GET_TITLE(ch), "the newbie %s", pc_class_types[(int)GET_CLASS(ch)]);
  }
  else {
    if (GET_TITLE(ch) != NULL)
      free(GET_TITLE(ch));
    GET_TITLE(ch)=str_dup(title);
    return;
  }
}


void check_autowiz(struct char_data * ch)
{
  char buf[100];
  extern int use_autowiz;
  extern int min_wizlist_lev;
  pid_t getpid(void);

  if (use_autowiz && GET_LEVEL(ch) >= LVL_HERO) {
    save_char(ch, NOWHERE);
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_HERO, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}



int gain_exp(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;
  char buf[128];

  if (IS_NPC(ch))
    return 0;

  if ((GET_LEVEL(ch) < 1) || (GET_LEVEL(ch) >= LVL_HERO))
    return 0;

  if (gain > 0) {
    /* put a cap on the max gain per kill */
    gain = MIN((exp_table[(int)GET_CLASS(ch)][GET_LEVEL(ch)+1]-exp_table[(int)GET_CLASS(ch)][(int)GET_LEVEL(ch)])/4, gain);
    GET_EXP(ch) += gain;
    while (GET_LEVEL(ch) < (LVL_HERO-1) &&
	GET_EXP(ch) >= (GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][GET_LEVEL(ch) + 1])) {
      GET_LEVEL(ch) += 1;
      GET_CLASS_LEVEL(ch, (int)GET_CLASS(ch)) = GET_LEVEL(ch);
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if(GET_EXP(ch) > (GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][LVL_HERO])) {
      GET_EXP(ch) = (GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][LVL_HERO]);
    }

    if (is_altered) {
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else {
	sprintf(buf, "You rise %d levels!\r\n", num_levels);
	send_to_char(buf, ch);
      }
      check_autowiz(ch);
    }
  } else if (gain < 0) {
    gain = MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
    GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
  return(gain);
}


void gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;

  if (!IS_NPC(ch)) {
    GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
    while (GET_LEVEL(ch) < LVL_IMPL &&
	GET_EXP(ch) >= (GET_NUM_CLASSES(ch)*exp_table[(int)GET_CLASS(ch)][GET_LEVEL(ch) + 1])) {
      GET_LEVEL(ch) += 1;
      GET_CLASS_LEVEL(ch, (int)GET_CLASS(ch)) = GET_LEVEL(ch);
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else {
	sprintf(buf, "You rise %d levels!\r\n", num_levels);
	send_to_char(buf, ch);
      }
      check_autowiz(ch);
    }
  }
}


void gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (GET_COND(ch, condition) == -1)	/* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING | PLR_BUILDING))
    return;

  switch (condition) {
  case FULL:
    send_to_char("You are hungry.\r\n", ch);
    return;
  case THIRST:
    send_to_char("You are thirsty.\r\n", ch);
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  default:
    break;
  }

}


void check_idling(struct char_data * ch)
{
  extern int free_rent;
  void Crash_forcerentsave(struct char_data *ch, int cost);
  ACMD(do_vcdepart);

  if ((++(ch->char_specials.timer) > 8) && (GET_LEVEL(ch) < LVL_ASST))
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
      GET_WAS_IN(ch) = ch->in_room;
      if (FIGHTING(ch)) {
	stop_fighting(FIGHTING(ch));
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
    } else if (ch->char_specials.timer > 48) {
      if (ch->in_room != NOWHERE)
	char_from_room(ch);
      char_to_room(ch, 3);
      if (ch->desc)
	close_socket(ch->desc);
      if(ch->in_room != NOWHERE) {
        ch->desc = NULL;
        do_vcdepart(ch, "", 0, 0);
        if (free_rent)
          Crash_forcerentsave(ch, 0);
        else
          Crash_idlesave(ch);
        sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
        mudlog(buf, CMP, LVL_ASST, TRUE);
        extract_char(ch);
      }
    }
}



/* Update PCs, NPCs, and objects */
void point_update(void)
{
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  extern int obj_decay_time;

  void update_char_objects(struct char_data * ch);	/* handler.c */
  void extract_obj(struct obj_data * obj);	/* handler.c */
  void update_pos(struct char_data * victim);	/* fight.c */

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    update_pos(i);
    if (GET_POS(i) >= POS_STUNNED) {
      GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      if (IS_AFFECTED(i, AFF_POISON))
	damage(i, i, 2, SPELL_POISON);
      if (GET_POS(i) <= POS_STUNNED)
	update_pos(i);
    } else if (GET_POS(i) == POS_INCAP)
      damage(i, i, 1, TYPE_SUFFERING);
    else if (GET_POS(i) <= POS_MORTALLYW) {
      if((GET_HIT(i) <= -10) && (!IS_NPC(i))) {
        sprintf(buf2, "%s (lvl %d) killed by %s at %s [%d]", GET_NAME(i), GET_LEVEL(i),
                GET_NAME(i), world[i->in_room].name, world[i->in_room].number);
        mudlog(buf2, BRF, LVL_HERO, TRUE);
        sprintf(buf2, "%s (lvl %d) killed by %s at %s", GET_NAME(i), GET_LEVEL(i),
                GET_NAME(i), world[i->in_room].name);
        mortlog(buf2, BRF, LVL_HERO, FALSE);
        die(i, i);
        continue;
      }
      else
        damage(i, i, 2, TYPE_SUFFERING);
    }

    if (!IS_NPC(i)) {
      update_char_objects(i);
      check_idling(i);
    }

    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;	/* Next in object list */

    /* If this is a corpse */
    if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER)&&(GET_OBJ_VAL(j, 3))) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
	GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {

	if (j->carried_by)
	  act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
	}
	for (jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content;	/* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj, j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj, j->carried_by->in_room);
	  else if (j->in_room != NOWHERE)
	    obj_to_room(jj, j->in_room);
	  else {
            char wierdbuf[MAX_INPUT_LENGTH];
            sprintf(wierdbuf, "corpse decaying nowhere: %s", j->short_description);
            log(buf);
	    extract_obj(jj);
          }
	}
	extract_obj(j);
      }
    }
    else if(GET_OBJ_TIMER(j) > 0) {
      if(j->in_room!=NOWHERE)
        if(*zone_table[world[j->in_room].zone].locked_by)
          GET_OBJ_TIMER(j) = obj_decay_time;
        else
          GET_OBJ_TIMER(j)--;
      else
        GET_OBJ_TIMER(j) = obj_decay_time;

      if(GET_OBJ_TIMER(j)==0) {
        if(GET_OBJ_TYPE(j)==ITEM_CONTAINER) {
          for (jj = j->contains; jj; jj = next_thing2) {
            next_thing2 = jj->next_content;	/* Next in inventory */
            obj_from_obj(jj);
            obj_to_room(jj, j->in_room);
          }
        }
        if ((world[j->in_room].people)) {
	  act("$p disintegrates.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
	  act("$p disintegrates.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
        }
        extract_obj(j);
      }
    }
  }
}
