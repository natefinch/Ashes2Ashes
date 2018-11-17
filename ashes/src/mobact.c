/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
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
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"

int get_dir(struct char_data *ch);

/* external structs */
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern int mprog_disable;

/* external functions */
extern int is_empty (int zone_nr);
extern void mprog_random_trigger (struct char_data *mob);
extern void mprog_wordlist_check (char *arg, struct char_data *mob, struct char_data *actor, struct obj_data *obj, void *vo, int type);

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

struct hit_data {
  struct char_data *ch;
  struct hit_data *next;
};

void ferocity_check(void)
{
  register struct char_data *ch, *next_ch, *vict;
  int found, max, cha_total, temp;
  struct descriptor_data *d;
  struct hit_data *hit_list, *temp_hit;

  for(d=descriptor_list; d; d=d->next) {
    if(d->character && (d->character->in_room > 0)) {
      /* Check for continuous dt's as long as we're going through the list anyway */
      if(world[d->character->in_room].dt_repeat && ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR))) {
        if(dt_damage(d->character))
          continue;
      }
      for(ch=world[d->character->in_room].people; ch; ch=next_ch) {
        next_ch=ch->next_in_room;
        if ((MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN))&&(GET_FEROCITY(ch)>10)&&((GET_FEROCITY(ch)-10)>=number(1, 5))&&(!FIGHTING(ch))) {
          hit_list=NULL;
          cha_total=0;
          temp=2*MAX(13-GET_INT(ch), GET_INT(ch)-13);
          for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
            if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
              continue;
            if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
	      continue;
            if (AFF_FLAGGED(vict, AFF_DIVINE_PROT))
              continue;
            if(IS_EVIL(ch) && AFF_FLAGGED(vict, AFF_PROTECT_EVIL))
              continue;
            if(IS_GOOD(ch) && AFF_FLAGGED(vict, AFF_PROTECT_GOOD))
              continue;
            if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
                (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
                (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
	        (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {
              CREATE(temp_hit, struct hit_data, 1);
              temp_hit->ch=vict;
              temp_hit->next=hit_list;
              hit_list=temp_hit;
              cha_total+=26-GET_CHA(vict)+temp;
            }
          }
          if(hit_list) {
            max=number(0, cha_total-1);
            for(found=0, temp_hit=hit_list; temp_hit; temp_hit=temp_hit->next) {
              found+=26-GET_CHA(temp_hit->ch)+temp;
              if(found>max) {
                hit(ch, temp_hit->ch, TYPE_UNDEFINED);
                break;
              }
            }
            for( ; hit_list; hit_list=temp_hit) {
              temp_hit=hit_list->next;
              free(hit_list);
            }
          }
        }
      }
    }
  }
}

void mobile_activity(void)
{
  register struct char_data *ch, *next_ch, *vict;
  struct obj_data *obj, *best_obj, *obj2;
  int door, found, max, cha_total, temp;
  struct descriptor_data *d;
  memory_rec *names;
  struct hit_data *hit_list, *temp_hit;
  char buf[MAX_INPUT_LENGTH];

  extern int no_specials;

  int find_first_step(sh_int src, sh_int target);
  ACMD(do_get);

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if ((!IS_MOB(ch)) || FIGHTING(ch))
      continue;

    /* Examine call for special procedure */
    if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
      if (mob_index[GET_MOB_RNUM(ch)].func == NULL) {
	sprintf(buf, "%s (#%d): Attempting to call non-existing mob func",
		GET_NAME(ch), GET_MOB_VNUM(ch));
	log(buf);
	REMOVE_BIT(MOB_FLAGS(ch), MOB_SPEC);
      } else {
	if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
	  continue;
      }
    }

    /* actions string */
    if(ACTIONS(ch) && (*ACTIONS(ch))) {
      mprog_disable=1;
      while(1) {
        for(temp=0; (ACTIONS(ch)[GET_ACTION(ch)] && (ACTIONS(ch)[GET_ACTION(ch)]!='\r') && (ACTIONS(ch)[GET_ACTION(ch)]!='\n') && (temp < MAX_INPUT_LENGTH)); GET_ACTION(ch)++)
          buf[temp++]=ACTIONS(ch)[GET_ACTION(ch)];
        buf[temp]=0;
        while((ACTIONS(ch)[GET_ACTION(ch)]=='\r') || (ACTIONS(ch)[GET_ACTION(ch)]=='\n'))
          GET_ACTION(ch)++;
        if(!strn_cmp(buf, "delay", 5)) {
          if(ACTIONS(ch)[GET_ACTION(ch)]==0)
            GET_ACTION(ch)=0;
          break;
        }
        else
          command_interpreter(ch, buf);
        if(ACTIONS(ch)[GET_ACTION(ch)]==0) {
          GET_ACTION(ch)=0;
          break;
        }
      }
      mprog_disable=0;
    }

    if(!AWAKE(ch))
      continue;

    /* Mob prog rand trigger */
    if(IS_SET(mob_index[GET_MOB_RNUM(ch)].progtypes, RAND_PROG) && (!is_empty(world[ch->in_room].zone)))
      mprog_random_trigger(ch);

    /* Make sure they're still alright after rand prog */
    if((ch->in_room==NOWHERE) || FIGHTING(ch) || !AWAKE(ch))
      continue;

    /* Scavenger (picking up objects) */
    if (MOB_FLAGGED(ch, MOB_SCAVENGER))
      if (world[ch->in_room].contents && !number(0, 3)) {
	max = 1;
	best_obj = NULL;
	for (obj = world[ch->in_room].contents; obj; obj = obj->next_content) {
          if(GET_OBJ_TYPE(obj)==ITEM_MONEY)
            continue;
          if((GET_OBJ_TYPE(obj)==ITEM_CONTAINER)&&(GET_OBJ_VAL(obj, 3))) {
            for(obj2=obj->contains; obj2; obj2=obj2->next_content) {
              if(GET_OBJ_TYPE(obj2)==ITEM_MONEY)
                continue;
              if(CAN_GET_OBJ(ch, obj2) && (GET_OBJ_COST(obj2)>max)) {
                best_obj=obj2;
                max=GET_OBJ_COST(obj2);
              }
            }
          }
	  else if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
	    best_obj = obj;
	    max = GET_OBJ_COST(obj);
	  }
        }
	if (best_obj != NULL) {
          if(best_obj->in_obj)
            obj_from_obj(best_obj);
          else
            obj_from_room(best_obj);
	  obj_to_char(best_obj, ch);
	  act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
	}
      }

    /* QHeal */
    if (MOB_FLAGGED(ch, MOB_QHEAL) && (GET_HIT(ch)<GET_MAX_HIT(ch))) {
      if(number(1, 3) == 1) {
        act("$n's wounds rapidly disappear.", FALSE, ch, 0, 0, TO_ROOM);
        GET_HIT(ch)+=MIN(500, GET_MAX_HIT(ch)/4);
        GET_HIT(ch)=MIN(GET_HIT(ch), GET_MAX_HIT(ch));
      }
    }

    /* Mob Movement */
    if (!MOB_FLAGGED(ch, MOB_SENTINEL) && (GET_POS(ch) == POS_STANDING) && (number(1, 10) <= GET_MOVE_RATE(ch))) {
      if(MOB_FLAGGED(ch, MOB_HUNTER) && MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
        for(d=descriptor_list; d; d=d->next) {
          if((d->character) && (HUNTING(ch)==d->character))
            break;
        }
        if(!d)
          HUNTING(ch)=NULL;
        if((!HUNTING(ch))||(world[HUNTING(ch)->in_room].zone!=world[ch->in_room].zone)) {
          found=0;
          for (names = MEMORY(ch); names && (!found); names = names->next) {
            for(d=descriptor_list; d; d=d->next) {
              if((d->character)&&(names->id == GET_IDNUM(d->character))&&(d->character->in_room>0)&&(world[d->character->in_room].zone==world[ch->in_room].zone)) {
                HUNTING(ch)=d->character;
                found=1;
                break;
              }
            }
          }
        }
        if(HUNTING(ch)) {
          if((door=find_first_step(ch->in_room, HUNTING(ch)->in_room))>=0) {
            if(!ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB | ROOM_DEATH) &&
               (!MOB_FLAGGED(ch, MOB_STAY_ZONE) || (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone)))
              perform_move(ch, door, 1);
          }
        }
        else if(((door = get_dir(ch)) < NUM_OF_DIRS) && !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB | ROOM_DEATH) &&
                (!MOB_FLAGGED(ch, MOB_STAY_ZONE) || (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))) {
          if(ch->mob_specials.last_direction == door) {
            ch->mob_specials.last_direction = -1;
          }
          else {
            ch->mob_specials.last_direction = door;
            perform_move(ch, door, 1);
          }
        }
      }
      else if(((door = get_dir(ch)) < NUM_OF_DIRS) && !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB | ROOM_DEATH) &&
              (!MOB_FLAGGED(ch, MOB_STAY_ZONE) || (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))) {
        if(ch->mob_specials.last_direction == door) {
          ch->mob_specials.last_direction = -1;
        }
        else {
          ch->mob_specials.last_direction = door;
          perform_move(ch, door, 1);
        }
      }
    }

    /* Stop if the mob died via greet or entry prog */
    if(ch->in_room==NOWHERE)
      continue;

    /* Mob act progs */
    if(ch->mob_specials.mpactnum > 0) {
      MPROG_ACT_LIST *tmp_act, *tmp2_act;
      for(tmp_act = ch->mob_specials.mpact; tmp_act != NULL; tmp_act = tmp_act->next) {
        mprog_wordlist_check(tmp_act->buf, ch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG);
        free(tmp_act->buf);
      }
      for(tmp_act = ch->mob_specials.mpact; tmp_act != NULL; tmp_act = tmp2_act) {
        tmp2_act = tmp_act->next;
        free(tmp_act);
      }
      ch->mob_specials.mpactnum=0;
      ch->mob_specials.mpact=NULL;
    }

    /* Stop if the mob died via act prog */
    if(ch->in_room==NOWHERE)
      continue;

    /* Aggressive Mobs */
    /* Agro now hits a random player, weighted by the player's charisma,
       which is weighted by the mob's inteligence */
    if ((MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN))&&(GET_FEROCITY(ch)<11)&&(number(1, 10) <= GET_FEROCITY(ch))) {
      hit_list=NULL;
      cha_total=0;
      temp=2*MAX(13-GET_INT(ch), GET_INT(ch)-13);
      for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
	if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
	  continue;
	if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
	  continue;
        if (AFF_FLAGGED(vict, AFF_DIVINE_PROT))
          continue;
        if(IS_EVIL(ch) && AFF_FLAGGED(vict, AFF_PROTECT_EVIL))
          continue;
        if(IS_GOOD(ch) && AFF_FLAGGED(vict, AFF_PROTECT_GOOD))
          continue;
	if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {
          CREATE(temp_hit, struct hit_data, 1);
          temp_hit->ch=vict;
          temp_hit->next=hit_list;
          hit_list=temp_hit;
          cha_total+=26-GET_CHA(vict)+temp;
	}
      }
      if(hit_list) {
        max=number(0, cha_total-1);
        for(found=0, temp_hit=hit_list; temp_hit; temp_hit=temp_hit->next) {
          found+=26-GET_CHA(temp_hit->ch)+temp;
          if(found>max) {
            hit(ch, temp_hit->ch, TYPE_UNDEFINED);
            break;
          }
        }
        for( ; hit_list; hit_list=temp_hit) {
          temp_hit=hit_list->next;
          free(hit_list);
        }
      }
    }

    /* Mob Memory */
    if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
	  continue;
	for (names = MEMORY(ch); names && !found; names = names->next)
	  if (names->id == GET_IDNUM(vict)) {
	    found = TRUE;
	    act("'Hey!  You're the fiend that attacked me!!!', exclaims $n.",
		FALSE, ch, 0, 0, TO_ROOM);
	    hit(ch, vict, TYPE_UNDEFINED);
	  }
      }
    }

    /* Helper Mobs */
    if (MOB_FLAGGED(ch, MOB_HELPER)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room)
	if (ch != vict && IS_NPC(vict) && FIGHTING(vict) &&
            !IS_NPC(FIGHTING(vict)) && ch != FIGHTING(vict)) {
	  act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);
	  hit(ch, FIGHTING(vict), TYPE_UNDEFINED);
	  found = TRUE;
	}
    }
    /* Add new mobile actions here */

  }				/* end for() */
}



/* Mob Memory Routines */

/* make ch remember victim */
void remember(struct char_data * ch, struct char_data * victim)
{
  memory_rec *tmp;
  bool present = FALSE;

  if (!IS_NPC(ch) || IS_NPC(victim))
    return;

  for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
    if (tmp->id == GET_IDNUM(victim))
      present = TRUE;

  if (!present) {
    CREATE(tmp, memory_rec, 1);
    tmp->next = MEMORY(ch);
    tmp->id = GET_IDNUM(victim);
    MEMORY(ch) = tmp;
  }
}


/* make ch forget victim */
void forget(struct char_data * ch, struct char_data * victim)
{
  memory_rec *curr, *prev = NULL;

  if (!(curr = MEMORY(ch)))
    return;

  while (curr && curr->id != GET_IDNUM(victim)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    return;			/* person wasn't there at all. */

  if (curr == MEMORY(ch))
    MEMORY(ch) = curr->next;
  else
    prev->next = curr->next;

  free(curr);
}


/* erase ch's memory */
void clearMemory(struct char_data * ch)
{
  memory_rec *curr, *next;

  curr = MEMORY(ch);

  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }

  MEMORY(ch) = NULL;
}
