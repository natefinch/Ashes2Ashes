/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "class.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct cha_app_type cha_app[];
extern struct int_app_type int_app[];
extern struct index_data *obj_index;
extern struct index_data *mob_index;

extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;

extern int mini_mud;

void clearMemory(struct char_data * ch);
void act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);
void damage(struct char_data * ch, struct char_data * victim,
	         int damage, int weapontype);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int mag_savingthrow(struct char_data * ch, int type);
int apply_ac(struct char_data * ch, int eq_pos);
SPECIAL(shop_keeper);


/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
  int water;

  if (ch == NULL || obj == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  if ((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) || (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN)) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
    } else {
      water = level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
      water = MIN(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), water);
      if (water > 0) {
	GET_OBJ_VAL(obj, 2) = LIQ_WATER;
	GET_OBJ_VAL(obj, 1) += water;
	weight_change_object(obj, water);
	act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
  else {
    act("You try, but are unable to fill $p!", FALSE, ch, obj, 0, TO_CHAR);
  }
}


ASPELL(spell_recall)
{
  extern sh_int r_mortal_start_room;
  room_num location;

  if (ch == NULL || victim == NULL || IS_NPC(victim))
    return;

  if(ARENA(victim).kill_mode) {
    send_to_char("You cannot recall someone in arena combat.\r\n", ch);
    return;
  }

  if ((ch != victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)) {
    sprintf(buf, "%s just tried to recall you.\r\n"
            "%s failed because you have summon protection on.\r\n"
            "Type NOSUMMON to allow other players to recall you.\r\n",
            GET_NAME(ch), (ch->player.sex == SEX_MALE) ? "He" : "She");
    send_to_char(buf, victim);

    sprintf(buf, "You failed because %s has summon protection on.\r\n",
            GET_NAME(victim));
    send_to_char(buf, ch);
    return;
  }

  if(ROOM_FLAGGED(victim->in_room, ROOM_NOTELEPORT)) {
    act("A mysterious force prevents $N from recalling back to safety!\n\r",
       FALSE, ch, NULL, victim, TO_CHAR);
    return;
  }

  location=real_room(3072);
  if(location <= 0)
    location=r_mortal_start_room;

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
}


ACAST(spell_teleport)
{
  int to_room, i=0;
  extern int top_of_world;

  if (victim == NULL || IS_NPC(victim))
    return FALSE;

  if (ROOM_FLAGGED(victim->in_room, ROOM_NOTELEPORT)) {
    send_to_char("A mysterious force prevents you from teleporting.\r\n", victim);
    return FALSE;
  }

  if(ARENA(victim).kill_mode) {
    send_to_char("You cannot teleport out of the arena.\r\n", victim);
    return FALSE;
  }

  do {
    to_room = number(0, top_of_world);
    if(++i > 10) {
      to_room = real_room(3072);
      break;
    }
  } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_NORELOCATE) ||
           (zone_table[world[to_room].zone].number == GOD_ZONE) ||
           (world[to_room].zone == 0) ||
           zone_table[world[to_room].zone].closed ||
           *zone_table[world[to_room].zone].locked_by);

  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  if(ROOM_FLAGGED(to_room, ROOM_DEATH)) {
    if(dt_damage(victim)) {
      return FALSE;
    }
  }
  return TRUE;
}

#define SUMMON_FAIL "You failed.\r\n"

ACAST(spell_summon)
{
  if (ch == NULL || victim == NULL)
    return FALSE;

  if (GET_LEVEL(victim) > MIN(LVL_HERO - 1, level + 5)) {
    send_to_char(SUMMON_FAIL, ch);
    return FALSE;
  }

  if(ARENA(victim).kill_mode != ARENA(ch).kill_mode) {
    send_to_char("The arena barrier prevents you from summoning.\r\n", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(ch->in_room, ROOM_NORELOCATE)) {
    send_to_char("A mysterious force prevents you from summoning!", ch);
    return FALSE;
  }
  if(ROOM_FLAGGED(victim->in_room, ROOM_NOTELEPORT)) {
    act("A mysterious force prevents you from summoning $N.", FALSE, ch, NULL, victim, TO_CHAR);
    return FALSE;
  }

  if (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
    act("As the words escape your lips and $N travels\r\n"
        "through time and space towards you, you realize that $E is\r\n"
        "aggressive and might harm you, so you wisely send $M back.",
        FALSE, ch, 0, victim, TO_CHAR);
    return TRUE;
  }
  if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)) {
    sprintf(buf, "%s just tried to summon you to: %s.\r\n"
            "%s failed because you have summon protection on.\r\n"
            "Type NOSUMMON to allow other players to summon you.\r\n",
            GET_NAME(ch), world[ch->in_room].name,
            (ch->player.sex == SEX_MALE) ? "He" : "She");
    send_to_char(buf, victim);

    sprintf(buf, "You failed because %s has summon protection on.\r\n",
            GET_NAME(victim));
    send_to_char(buf, ch);
    return TRUE;
  }

  if (IS_NPC(victim) && (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
      mag_savingthrow(victim, SAVING_SPELL))) {
    send_to_char(SUMMON_FAIL, ch);
    return TRUE;
  }

  if((IS_NPC(victim))&&(world[ch->in_room].zone != world[victim->in_room].zone)) {
    send_to_char("You may not summon creatures outside their home realm.\r\n", ch);
    return FALSE;
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
  return TRUE;
}


ACAST(spell_group_summon)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;
  int retv=0;

  if (ch == NULL)
    return FALSE;

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("You aren't in a group!\r\n", ch);
    return FALSE;
  }

  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room == ch->in_room)
      continue;
    if (!IS_AFFECTED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    retv+=spell_summon(level, ch, tch, NULL);
  }

  if ((k != ch) && IS_AFFECTED(k, AFF_GROUP) && (k->in_room != ch->in_room)) {
    retv+=spell_summon(level, ch, k, NULL);
  }

  return retv;
}


ASPELL(spell_locate_object)
{
  struct obj_data *i, *temp;
  char name[MAX_INPUT_LENGTH];
  int j, room;
  struct char_data *tch;

  if(ch == NULL)
    return;

  strcpy(name, fname(obj->name));
  j = level/5;

  for (i = object_list; i && (j > 0); i = i->next) {
    if (!isexact(name, i->name))
      continue;

    if(IS_OBJ_STAT(i, ITEM_NOLOCATE))
      continue;

    tch=NULL;

    if (i->carried_by) {
      tch=i->carried_by;
      room=i->carried_by->in_room;
      sprintf(buf, "%s is being carried by %s.\n\r",
	      i->short_description, PERS(i->carried_by, ch));
    }
    else if (i->in_room != NOWHERE) {
      room=i->in_room;
      sprintf(buf, "%s is in %s.\n\r", i->short_description,
	      world[i->in_room].name);
    }
    else if (i->in_obj) {
      for(temp=i; temp->in_obj; temp=temp->in_obj);
      if(temp->carried_by) {
        tch=temp->carried_by;
        room=temp->carried_by->in_room;
      }
      else if(temp->in_room != NOWHERE) {
        room=temp->in_room;
      }
      else if(temp->worn_by) {
        tch=temp->worn_by;
        room=temp->worn_by->in_room;
      }
      else
        room=NOWHERE;
      sprintf(buf, "%s is in %s.\n\r", i->short_description,
	      i->in_obj->short_description);
    }
    else if (i->worn_by) {
      tch=i->worn_by;
      room=i->worn_by->in_room;
      sprintf(buf, "%s is being worn by %s.\n\r",
	      i->short_description, PERS(i->worn_by, ch));
    }
    else {
      room=NOWHERE;
      sprintf(buf, "%s's location is uncertain.\n\r",
	      i->short_description);
    }

    if(((room==NOWHERE) || (!zone_table[world[room].zone].closed)) && ((!tch) || IS_NPC(tch) || (GET_LEVEL(tch) < LVL_HERO))) {
      CAP(buf);
      send_to_char(buf, ch);
      j--;
    }
  }

  if (j == 0)
    send_to_char("You are very confused.\r\n", ch);

  if (j == level/5)
    send_to_char("You sense nothing.\n\r", ch);
}



ASPELL(spell_charm)
{
  int charmed_levels=0, num_charmies=0;
  struct affected_type af;
  struct follow_type *f;
  extern int max_charmies;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (!IS_NPC(victim))
    send_to_char("Sorry, you may not charm other players.\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_SANCTUARY))
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char("Your victim resists!\r\n", ch);
  else if(GET_MOB_SPEC(victim) == shop_keeper)
    send_to_char("You fail.\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_CHARM) || ((level >> 1) < GET_LEVEL(victim)))
    send_to_char("You fail.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (mag_savingthrow(victim, SAVING_PARA))
    send_to_char("Your victim resists!\r\n", ch);
  else {
    for(f = ch->followers; f; f = f->next) {
      if(IS_AFFECTED(f->follower, AFF_CHARM)) {
        charmed_levels += GET_LEVEL(f->follower);
        num_charmies++;
      }
    }

    if(((charmed_levels+GET_LEVEL(victim)) <= level) && (num_charmies < max_charmies)) {
      if (victim->master)
        stop_follower(victim);

      add_follower(victim, ch);

      af.type = SPELL_CHARM;

      if (GET_INT(victim))
        af.duration = 24 * 18 / GET_INT(victim);
      else
        af.duration = 24 * 18;

      af.modifier = 0;
      af.location = 0;
      af.bitvector = AFF_CHARM;
      affect_to_char(victim, &af);

      act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
      if (IS_NPC(victim)) {
        REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
        REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
      }
    }
    else {
      send_to_char("You don't have the power to control that victim.\r\n", ch);
    }
  }
}



ASPELL(spell_identify)
{
  int i, thac0;
  int found;
  extern char *damtypes[];
  extern char *wear_bits[];

  struct time_info_data age(struct char_data * ch);
  int class_get_thac0(struct char_data *ch);

  extern struct str_app_type str_app[];

  extern char *spells[];

  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];

  if(ch == NULL)
    return;

  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    send_to_char("Can be worn on  : ", ch);
    sprintbit(obj->obj_flags.wear_flags, wear_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "Weight: %d, Value: %d\r\n",
	    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj));

    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      sprintf(buf, "This %s casts (lvl %d): ", item_types[(int) GET_OBJ_TYPE(obj)], GET_OBJ_VAL(obj, 0));

      if (GET_OBJ_VAL(obj, 1) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 1)]);
      if (GET_OBJ_VAL(obj, 2) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 2)]);
      if (GET_OBJ_VAL(obj, 3) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf, "%s\r\n", buf);
      send_to_char(buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts (lvl %d): ", item_types[(int) GET_OBJ_TYPE(obj)], GET_OBJ_VAL(obj, 0));
      sprintf(buf, "%s %s\r\n", buf, spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf, "%sIt has %d maximum charge%s and %d remaining.\r\n", buf,
	      GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
	      GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
      break;
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'", GET_OBJ_VAL(obj, 1),
	      GET_OBJ_VAL(obj, 2));
      sprintf(buf, "%s for an average per-round damage of %.1f.\r\n", buf,
	      (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    }
    sprintbit(obj->obj_flags.immune, damtypes, buf1);
    sprintf(buf, "Enhanced resistance to: %s\r\n", buf1);
    send_to_char(buf, ch);
    sprintbit(obj->obj_flags.resist, damtypes, buf1);
    sprintf(buf, "Some resistance to: %s\r\n", buf1);
    send_to_char(buf, ch);
    sprintbit(obj->obj_flags.weak, damtypes, buf1);
    sprintf(buf, "Vulnerable to: %s\r\n", buf1);
    send_to_char(buf, ch);
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char("Can affect you as :\r\n", ch);
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2);
	sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
	send_to_char(buf, ch);
      }
    }
  } else if (victim) {		/* victim */
    thac0 = class_get_thac0(victim);
    thac0 -= str_app[STRENGTH_APPLY_INDEX(victim)].tohit;
    thac0 -= 5*GET_HITROLL(victim);
    thac0 -= (int) ((GET_INT(victim) - 13) / 1.5);	/* Intelligence helps! */
    thac0 -= (int) ((GET_WIS(victim) - 13) / 1.5);	/* So does wisdom */
    thac0 *= -1;

    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
	      GET_NAME(victim), age(victim).year, age(victim).month,
	      age(victim).day, age(victim).hours);
      send_to_char(buf, ch);
    }
    buf1[0]=0;
    if((!IS_NPC(victim)) && (GET_CLASS_BITVECTOR(victim) & MK_F))
      sprintf(buf1, ", Barehand: %dd4", 3+((GET_LEVEL(victim)-1)/10));
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n",
	    GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf, "%sLevel: %d, Hits: %d, Mana: %d\r\n", buf,
	    GET_LEVEL(victim), GET_HIT(victim), GET_MANA(victim));
    sprintf(buf, "%sAC: %d, Hitroll: %d, Damroll: %d, THAC0: %d%s\r\n", buf,
	    compute_ac(victim), GET_HITROLL(victim), GET_DAMROLL(victim), thac0, buf1);
    sprintf(buf, "%sStr: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
	    buf, GET_STR(victim), GET_ADD(victim), GET_INT(victim),
	GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim));
    send_to_char(buf, ch);

  }
}



ASPELL(spell_enchant_weapon)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
	return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = (level/30);

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = (level/30);

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}

ASPELL(spell_cure_blind)
{
  void check_drawn_in(struct char_data *ch, struct char_data *victim);

  if((ch == NULL) || (victim == NULL))
    return;

  check_drawn_in(ch, victim);

  if ((!affected_by_spell(victim, SPELL_BLINDNESS)) && (!affected_by_spell(victim, SKILL_GOUGE))) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  if(affected_by_spell(victim, SPELL_BLINDNESS))
    affect_from_char(victim, SPELL_BLINDNESS);
  if(affected_by_spell(victim, SPELL_COLOR_SPRAY))
    affect_from_char(victim, SPELL_COLOR_SPRAY);
  if(affected_by_spell(victim, SKILL_GOUGE))
    affect_from_char(victim, SKILL_GOUGE);

  act("Your vision returns!", FALSE, victim, 0, ch, TO_CHAR);
  act("There's a momentary gleam in $n's eyes.", TRUE, victim, 0, ch, TO_ROOM);
}

ASPELL(spell_heal)
{
  if(victim == NULL)
    return;

  if(affected_by_spell(victim, SPELL_BLINDNESS))
    affect_from_char(victim, SPELL_BLINDNESS);
  if(affected_by_spell(victim, SPELL_COLOR_SPRAY))
    affect_from_char(victim, SPELL_COLOR_SPRAY);
  if(affected_by_spell(victim, SKILL_GOUGE))
    affect_from_char(victim, SKILL_GOUGE);
  if(affected_by_spell(victim, SPELL_POISON))
    affect_from_char(victim, SPELL_POISON);
}


ASPELL(spell_detect_poison)
{
  if(ch == NULL)
    return;

  if (victim) {
    if (victim == ch) {
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\r\n", ch);
      else
        send_to_char("You feel healthy.\r\n", ch);
    } else {
      if (IS_AFFECTED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
	act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
	act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
	    TO_CHAR);
      break;
    default:
      send_to_char("You sense that it should not be consumed.\r\n", ch);
    }
  }
}

ASPELL(spell_calm)
{
  struct char_data *fighter;

  if(ch == NULL)
    return;

  for(fighter=world[ch->in_room].people; fighter; fighter=fighter->next_in_room) {
    if(FIGHTING(fighter) && ((!IS_NPC(fighter))||(!mag_savingthrow(fighter, SAVING_SPELL)))) {
      send_to_char("You feel a wave of peace flow through your body.\r\n", fighter);
      stop_fighting(fighter);
    }
  }
}

ACAST(spell_control_weather)
{
  char buf[MAX_STRING_LENGTH];
  char *ptr = buf, *arg = (char *) obj;

  if(ch == NULL)
    return FALSE;

  skip_spaces(&arg);
  one_argument(arg, buf);
  skip_spaces(&ptr);

  if ((!*ptr) || (str_cmp("better", ptr) && str_cmp("worse", ptr))) {
    send_to_char("Do you want it to get better or worse?\n\r", ch);
    return FALSE;
  }

  if (!str_cmp("better", ptr))
    weather_info.change += dice(level/6, 4);
  else
    weather_info.change -= dice(level/6, 4);

  return TRUE;
}

ACAST(spell_relocate)
{
  char buf[MAX_STRING_LENGTH];
  int room=-1;

  if(ch == NULL)
    return FALSE;

  if (victim)
  {
    if (IS_NPC(victim))
    {
      send_to_char("You may not relocate to a mob!\n\r", ch);
      return FALSE;
    }
    if (GET_LEVEL(victim) >= LVL_HERO)
    {
      send_to_char("You may not magicaly travel to an immortal!\n\r", ch);
      return FALSE;
    }
    if(victim->player_specials) {
      if(ARENA(victim).kill_mode != ARENA(ch).kill_mode) {
        send_to_char("The arena barrier stops you.\r\n", ch);
        return FALSE;
      }
    }
    else {
      if(ARENA(ch).kill_mode) {
        send_to_char("You cannot teleport out of the arena.\r\n", ch);
        return FALSE;
      }
    }
    /* NO_TELEPORT NO_RELOCATE FLAGS */
    if(ROOM_FLAGGED(ch->in_room, ROOM_NOTELEPORT)) {
      send_to_char("A mysterious force prevents you from leaving.\n\r",
                    ch);
      return FALSE;
    }
    if(ROOM_FLAGGED(victim->in_room, ROOM_NORELOCATE)) {
      send_to_char("For some unknown reason, you fail.\n\r", ch);
      return FALSE;
    }

    room = victim->in_room;
  }
  if(room < 0) return FALSE;
  if(zone_table[world[room].zone].closed) {
    send_to_char("Sorry, that area is closed for construction.\r\n", ch);
    return FALSE;
  }
  if (ROOM_FLAGGED(room, ROOM_GODROOM | ROOM_PRIVATE) ||
    world[room].zone == 0 || zone_table[world[room].zone].number == GOD_ZONE)
  {
    send_to_char("Hmmmm, your target seems to be in a private room.\n\r", 
                  ch);
    return FALSE;
  }
  else
  {
    strcpy(buf, "$n vanishes.");
    act(buf, TRUE, ch, 0, 0, TO_ROOM);

    char_from_room(ch);
    char_to_room(ch, room);

    strcpy(buf, "$n appears out of thin air.");
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
  }
  return TRUE;
}

ACAST(spell_teleport_no_error)
{
  int mem_num, retv, alloced=FALSE;
  char buf[MAX_STRING_LENGTH];
  char *ptr = buf, *arg = (char *) obj;
  struct char_data *tar_char=NULL;

  if(ch == NULL)
    return FALSE;

  skip_spaces(&arg);
  one_argument(arg, buf);
  skip_spaces(&ptr);
  if(isdigit(*ptr)) {
     mem_num=atoi(ptr);
    if((mem_num > 0) && (mem_num <= GET_INT(ch)/4) && (mem_num <= MAX_REMEMBER)) {
      if(GET_REMEMBER(ch, mem_num-1)<=0) {
        send_to_char("You don't have a location remembered there.\r\n", ch);
        return FALSE;
      }
      if(real_room(GET_REMEMBER(ch, mem_num-1)) < 0) {
        send_to_char("That room no longer exists.\r\n", ch);
        return FALSE;
      }
      if(zone_table[world[real_room(GET_REMEMBER(ch, mem_num-1))].zone].closed || *zone_table[world[real_room(GET_REMEMBER(ch, mem_num-1))].zone].locked_by) {
        send_to_char("Some immortal power prevents you from teleporting.\r\n", ch);
        return FALSE;
      }
      CREATE(tar_char, struct char_data, 1);
      tar_char->player_specials=NULL;
      GET_LEVEL(tar_char)=1;
      alloced=TRUE;
      tar_char->in_room=real_room(GET_REMEMBER(ch, mem_num-1));
    }
    else {
      send_to_char("You don't have that memory number available.\r\n", ch);
      return FALSE;
    }
  }
  else {
    if(!(tar_char=get_player_vis(ch, ptr))) {
      send_to_char("There is no one by that name playing.\r\n", ch);
      return FALSE;
    }
  }
  retv=spell_relocate(level, ch, tar_char, NULL);
  if(alloced)
    free(tar_char);
  return(retv);
}

ACAST(spell_remember)
{
  int mem_num;
  char num[MAX_STRING_LENGTH];
  char *ptr = num, *arg = (char *) obj;

  if(ch == NULL)
    return FALSE;

  skip_spaces(&arg);
  one_argument(arg, num);
  skip_spaces(&ptr);
  mem_num=atoi(ptr);
  if((mem_num > 0) && (mem_num <= GET_INT(ch)/4) && (mem_num <= MAX_REMEMBER)) {
    if(zone_table[world[ch->in_room].zone].number == ARENA_ZONE) {
      send_to_char("You cannot memorize locations in the arena.\r\n", ch);
      return FALSE;
    }
    GET_REMEMBER(ch, mem_num-1)=world[ch->in_room].number;
    send_to_char("Location commited to memory.\r\n", ch);
  }
  else {
    send_to_char("You don't have that memory number available.\r\n", ch);
    return FALSE;
  }
  return TRUE;
}

ACAST(spell_limited_wish)
{
  struct affected_type af;
  char desire[MAX_STRING_LENGTH];
  char target_name[MAX_STRING_LENGTH];
  char *arg = (char *) obj;

  if(ch == NULL)
    return FALSE;

  arg = one_argument(arg, desire);
  arg = one_argument(arg, target_name);

  if (*target_name)
  {
    victim = get_char_room_vis(ch, target_name);
    if (!victim)
    {
      send_to_char("Your victim is not here.\n\r", ch);
      return FALSE;
    }
  }
  else
    victim = ch;

  af.type      = SPELL_LIMITED_WISH;
  af.duration  = 24;
  af.bitvector = 0;

  if (strncmp("moves", desire, strlen(desire)) == 0)
  {
    GET_MOVE(victim) += 100;

    if (GET_MOVE(victim) >= GET_MAX_MOVE(victim))
      GET_MOVE(victim) = GET_MAX_MOVE(victim) - dice(1, 4);
  }
  else if (strncmp("hits", desire, strlen(desire)) == 0)
  {
    GET_HIT(victim) += 100;

    if (GET_HIT(victim) >= GET_MAX_HIT(victim))
      GET_HIT(victim) = GET_MAX_HIT(victim) - dice(1, 4);
  }
  else if (strncmp("maxmana", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_MANA;
    af.modifier = 10;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("maxhits", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_HIT;
    af.modifier = 10;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("maxmoves", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_MOVE;
    af.modifier = 10;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("strength", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_STR;
    af.modifier = 1;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("constitution", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_CON;
    af.modifier = 1;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("intelligence", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_INT;
    af.modifier = 1;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("wisdom", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_WIS;
    af.modifier = 1;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("dexterity", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_DEX;
    af.modifier = 1;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("hitregeneration", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_HP_REGEN;
    af.modifier = 2;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("manaregeneration", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_MANA_REGEN;
    af.modifier = 2;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  else if (strncmp("moveregeneration", desire, strlen(desire)) == 0)
  {
    af.location = APPLY_CON;
    af.modifier = -1;
    affect_join(ch, &af, FALSE, FALSE, TRUE, FALSE);

    af.location = APPLY_MOVE_REGEN;
    af.modifier = 2;
    affect_join(victim, &af, TRUE, TRUE, TRUE, FALSE);
  }
  return TRUE;
}

ASPELL(fire_breath)
{
  struct obj_data *to_destroy;

  if((ch == NULL) || (victim == NULL))
    return;

  if(number(0, 30) < (GET_LEVEL(ch) - (GET_LEVEL(victim)/4))) {
    if(!mag_savingthrow(victim, SAVING_BREATH)) {
      for(to_destroy=victim->carrying;
          (to_destroy && (GET_OBJ_TYPE(to_destroy) != ITEM_SCROLL) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_WAND) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_STAFF) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_POTION) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_NOTE) && number(0, 2));
          to_destroy=to_destroy->next_content);
      if(to_destroy) {
        act("$o burns.", FALSE, victim, to_destroy, NULL, TO_CHAR);
        extract_obj(to_destroy);
      }
    }
  }
}

ASPELL(frost_breath)
{
  struct obj_data *to_destroy;

  if((ch == NULL) || (victim == NULL))
    return;

  if(number(0, 30) < (GET_LEVEL(ch) - (GET_LEVEL(victim)/4))) {
    if(!mag_savingthrow(victim, SAVING_BREATH)) {
      for(to_destroy=victim->carrying;
          (to_destroy && (GET_OBJ_TYPE(to_destroy) != ITEM_FOOD) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_DRINKCON) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_FOUNTAIN) &&
           (GET_OBJ_TYPE(to_destroy) != ITEM_POTION) && number(0, 2));
          to_destroy=to_destroy->next_content);
      if(to_destroy) {
        act("$o breaks.", FALSE, victim, to_destroy, NULL, TO_CHAR);
        extract_obj(to_destroy);
      }
    }
  }
}

ASPELL(acid_breath)
{
  int i;

  if((ch == NULL) || (victim == NULL))
    return;

  if(number(0, 30) < (GET_LEVEL(ch) - (GET_LEVEL(victim)/4))) {
    if(!mag_savingthrow(victim, SAVING_BREATH)) {
      for(i=0; i<NUM_WEARS; i++) {
        if(GET_EQ(victim, i) && (GET_OBJ_TYPE(GET_EQ(victim, i)) == ITEM_ARMOR) &&
           (GET_OBJ_VAL(GET_EQ(victim, i), 0) > 0) && (!number(0, 2))) {
          act("$o is damaged!", FALSE, victim, GET_EQ(victim, i), NULL, TO_CHAR);
          GET_AC(victim) += apply_ac(victim, i);
          GET_OBJ_VAL(GET_EQ(victim, i), 0) -= MIN(GET_OBJ_VAL(GET_EQ(victim, i), 0), number(1, 7));
          GET_OBJ_COST(GET_EQ(victim, i)) /= 4;
          GET_AC(victim) -= apply_ac(victim, i);
          break;
        }
      }
    }
  }
}


ASPELL(spell_amnesia)
{
  if((ch == NULL) || (victim == NULL))
    return;

  if ((!MOB_FLAGGED(victim, MOB_MEMORY)) || mag_savingthrow(victim, SAVING_SPELL)) {
    send_to_char(NOEFFECT, ch);
    return;
  }
  clearMemory(victim);
  act("A blank look washes over $n's face.\r\n", TRUE, victim, NULL, NULL, TO_ROOM);
}
