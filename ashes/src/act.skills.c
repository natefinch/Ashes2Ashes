/* ************************************************************************
*   File: act.skills.c                                  Part of CircleMUD *
*  Usage: Non-combat skill commands                                       *
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

#include <sys/stat.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char *dirs[];
extern char *edirs[];

/* extern procedures */
void add_follower(struct char_data * ch, struct char_data * leader);
int mag_savingthrow(struct char_data * ch, int type);
void weight_change_object(struct obj_data * obj, int weight);
SPECIAL(shop_keeper);


ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    send_to_char("You're already sneaking.\r\n", ch);
    return;
  }
  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);

  percent = number(1, 100);

  if (percent > get_skill(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = -1;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, SKILL_SNEAK, SAVING_PARA);
}


ACMD(do_hide)
{
  byte percent;

  send_to_char("You attempt to hide yourself.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  percent = number(1, 100);

  if (percent > get_skill(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
    return;

  SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
}


ACMD(do_steal)
{
  struct char_data *vict;
  struct obj_data *obj;
  char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  int percent, gold, eq_pos, prob, ohoh = 0;
  extern int pt_allowed;


  ACMD(do_gen_comm);

  if(!(prob=get_skill(ch, SKILL_STEAL))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  argument = one_argument(argument, obj_name);
  one_argument(argument, vict_name);

  if (!(vict = get_char_room_vis(ch, vict_name))) {
    send_to_char("Steal what from who?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("Come on now, that's rather stupid!\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N stealing from you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you stealing from $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N stealing from $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = number(1, 100) - dex_app_skill[GET_DEX(ch)].p_pocket;
  percent -= (GET_LEVEL(ch)-GET_LEVEL(vict))*3;

  if (GET_POS(vict) < POS_SLEEPING)
    percent = -1;		/* ALWAYS SUCCESS */

  if (!pt_allowed && !IS_NPC(vict) && !IS_NPC(ch)) {
    send_to_char("No stealing from other players!\r\n", ch);
    return;
  }

  /* NO NO With Imp's and Shopkeepers, and if player thieving is not allowed */
  if (((GET_LEVEL(vict) >= LVL_HERO)&&(!IS_NPC(vict))) || GET_MOB_SPEC(vict) == shop_keeper)
    percent = 101;		/* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(ch, obj_name, vict->carrying))) {

      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (GET_EQ(vict, eq_pos) &&
	    (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
	    CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
	  obj = GET_EQ(vict, eq_pos);
	  break;
	}
      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
	return;
      } else {			/* It is equipment */
	if ((GET_POS(vict) >= POS_STUNNED) || GET_STUN(vict)) {
	  send_to_char("Steal the equipment now?  Impossible!\r\n", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
	  obj_to_char(unequip_char(vict, eq_pos), ch);
	}
      }
    } else {			/* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */

      if (AWAKE(vict) && (percent > prob)) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
      } else {			/* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\r\n", ch);
	  }
	} else
	  send_to_char("You cannot carry that much.\r\n", ch);
      }
    }
  } else {			/* Steal some coins */
    if (AWAKE(vict) && (percent > prob)) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(vict) -= gold;
        if (gold > 1) {
	  sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
	  send_to_char(buf, ch);
	} else {
	  send_to_char("You manage to swipe a solitary gold coin.\r\n", ch);
	}
      } else {
	send_to_char("You couldn't get any gold...\r\n", ch);
      }
    }
  }

  if (ohoh && IS_NPC(vict) && AWAKE(vict))
    hit(vict, ch, TYPE_UNDEFINED);
}


ACMD(do_forage)
{
  int i;
  struct obj_data *tobj;

  if(!(i=get_skill(ch, SKILL_FORAGE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!WILDERNESS(world[ch->in_room].sector_type)) {
    send_to_char("You can only forage in the wilderness.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 5) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=5;

  if(i<number(1, 100)) {
    send_to_char("You fail to find any berries right now.\r\n", ch);
    return;
  }

  act("$n picks some berries.", TRUE, ch, NULL, NULL, TO_ROOM);
  send_to_char("You find some berries. Yummy.\r\n", ch);

  if(!(tobj=read_object(11, VIRTUAL))) {
    send_to_char("There seems to be a problem with berries.\r\n", ch);
    return;
  }
  GET_OBJ_VAL(tobj, 0)=5+(GET_LEVEL(ch)>>1);
  obj_to_char(tobj, ch);
}


ACMD(do_healwounds)
{
  int i;
  struct char_data *vict;

  one_argument(argument, buf);

  if(!(i=get_skill(ch, SKILL_HEALWOUNDS))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 15) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }

  if(!*buf) {
    vict=ch;
  }
  else if(!(vict=get_char_room_vis(ch, buf))) {
    send_to_char("Who's wounds do you want to heal?\r\n", ch);
    return;
  }

  GET_MOVE(ch)-=15;

  if(i<number(1, 100)) {
    act("You fail to cure $s wounds.", FALSE, vict, NULL, ch, TO_VICT);
    GET_MOVE(ch)+=5;
    return;
  }

  GET_HIT(vict) += dice(2, 8) + (GET_LEVEL(ch)>>3);
  if(GET_HIT(vict)>GET_MAX_HIT(vict))
    GET_HIT(vict)=GET_MAX_HIT(vict);

  update_pos(vict);

  if(affected_by_spell(vict, SPELL_POISON)) {
    affect_from_char(vict, SPELL_POISON);
  }

  act("$n applies a smelly poultice to $N's wounds.", FALSE, ch, NULL, vict, TO_ROOM);
  act("You apply a smelly poultice to $N's wounds.", FALSE, ch, NULL, vict, TO_CHAR);
  act("Your wounds are healed.", FALSE, vict, NULL, NULL, TO_CHAR);
}


ACMD(do_divine)
{
  int i;
  struct obj_data *tobj;

  one_argument(argument, buf);

  if(!(i=get_skill(ch, SKILL_DIVINE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 5) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=5;

  if(!WILDERNESS(world[ch->in_room].sector_type)) {
    send_to_char("You can only divine in the wilderness.\r\n", ch);
    return;
  }

  if(!(tobj=get_obj_in_list_vis(ch, buf, ch->carrying))) {
    send_to_char("You don't have that item.\r\n", ch);
    return;
  }

  if(GET_OBJ_TYPE(tobj) != ITEM_DRINKCON) {
    send_to_char("You can only fill drink containers.\r\n", ch);
    return;
  }

  if(i<number(1, 100)) {
    send_to_char("You couldn't find a stream.\r\n", ch);
    return;
  }

  if((GET_OBJ_VAL(tobj, 2)!=LIQ_WATER)&&(GET_OBJ_VAL(tobj, 1)!=0)) {
    GET_OBJ_VAL(tobj, 2)=LIQ_SLIME;
  }
  else {
    i = GET_LEVEL(ch) * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
    i = MIN(GET_OBJ_VAL(tobj, 0)-GET_OBJ_VAL(tobj, 1), i);
    GET_OBJ_VAL(tobj, 2) = LIQ_WATER;
    GET_OBJ_VAL(tobj, 1) += i;
    weight_change_object(tobj, i);
    act("You fill a $p from a hidden stream.", FALSE, ch, tobj, NULL, TO_CHAR);
  }
}


ACMD(do_skin)
{
  int i;
  struct obj_data *tobj, *obj;

  one_argument(argument, buf);

  if(!(i=get_skill(ch, SKILL_SKIN))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 15) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=15;

  if(!(tobj=get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
    send_to_char("You can't find a corpse to skin.\r\n", ch);
    GET_MOVE(ch)+=15;
    return;
  }

  if((GET_OBJ_TYPE(tobj)!=ITEM_CONTAINER)||(!GET_OBJ_VAL(tobj, 3))) {
    send_to_char("You can only take skins from corpses.\r\n", ch);
    GET_MOVE(ch)+=15;
    return;
  }

  if(GET_OBJ_VAL(tobj, 3)!=1) {
    send_to_char("You may not skin the corpse of a player.\r\n", ch);
    GET_MOVE(ch)+=15;
    return;
  }

  if(i<number(1, 100)) {
    send_to_char("This corpse's skin is too tough for you.\r\n", ch);
    GET_MOVE(ch)+=10;
    return;
  }

  act("You expertly skin a piece of armor from $p.", FALSE, ch, tobj, NULL, TO_CHAR);
  act("$n expertly skins a piece of armor from $p.", FALSE, ch, tobj, NULL, TO_ROOM);

  obj=read_object(12+number(0, 10), VIRTUAL);
  if(!obj) {
    send_to_char("Something seems to be wrong.\r\n", ch);
    return;
  }

  GET_OBJ_VAL(obj, 0) = 2+(GET_LEVEL(ch)/20);
  obj_to_room(obj, ch->in_room);

  while(tobj->contains) {
    obj=tobj->contains;
    obj_from_obj(obj);
    obj_to_room(obj, ch->in_room);
  }
  extract_obj(tobj);
}


ACMD(do_filet)
{
  int i;
  struct obj_data *tobj, *obj;

  one_argument(argument, buf);

  if(!(i=get_skill(ch, SKILL_FILET))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 5) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }

  if(!(tobj=get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
    send_to_char("You can't find a corpse to filet.\r\n", ch);
    return;
  }

  if((GET_OBJ_TYPE(tobj)!=ITEM_CONTAINER)||(!GET_OBJ_VAL(tobj, 3))) {
    send_to_char("You can only filet corpses.\r\n", ch);
    return;
  }

  if(GET_OBJ_VAL(tobj, 3)!=1) {
    send_to_char("You may not filet the corpse of a player.\r\n", ch);
    return;
  }

  GET_MOVE(ch)-=5;

  if(i<number(1, 100)) {
    send_to_char("You failed to filet the corpse.\r\n", ch);
    return;
  }

  act("You carve a nice big juicy steak from $p.", FALSE, ch, tobj, NULL, TO_CHAR);
  act("$n carves a nice big juicy steak from $p.", FALSE, ch, tobj, NULL, TO_ROOM);

  obj=read_object(23, VIRTUAL);
  GET_OBJ_VAL(obj, 0) = 5+(GET_LEVEL(ch)>>1);
  obj_to_room(obj, ch->in_room);

  while(tobj->contains) {
    obj=tobj->contains;
    obj_from_obj(obj);
    obj_to_room(obj, ch->in_room);
  }
  extract_obj(tobj);
}


ACMD(do_befriend)
{
  int i, charmed_levels=0, num_charmies=0;
  struct char_data *vict;
  struct affected_type af;
  struct follow_type *f;
  extern int max_charmies;

  one_argument(argument, buf);

  if(!(i=get_skill(ch, SKILL_BEFRIEND))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 5) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }

  if(!(vict=get_char_room_vis(ch, buf))) {
    send_to_char("You don't see that person here.\r\n", ch);
    return;
  }

  if(!IS_NPC(vict)) {
    send_to_char("You can't befriend a player.\r\n", ch);
    return;
  }

  if(vict==ch) {
    send_to_char("You can't befriend yourself.\r\n", ch);
    return;
  }

  if(AFF_FLAGGED(ch, AFF_CHARM)) {
    send_to_char("You can't have any followers of your own.\r\n", ch);
    return;
  }

  GET_MOVE(ch)-=5;

  if(i<number(1, 100)) {
     act("You failed to befriend $m", FALSE, vict, NULL, ch, TO_VICT);
     return;
  }

  if(AFF_FLAGGED(vict, AFF_SANCTUARY)||MOB_FLAGGED(vict, MOB_NOCHARM)) {
    send_to_char("Your victim resists.\r\n", ch);
    return;
  }

  if(GET_MOB_SPEC(vict) == shop_keeper) {
    send_to_char("You fail.\r\n", ch);
    return;
  }

  if(AFF_FLAGGED(vict, AFF_CHARM) || ((GET_LEVEL(ch)>>1)<GET_LEVEL(vict))) {
    send_to_char("You fail.\r\n", ch);
    return;
  }

  if(circle_follow(vict, ch)) {
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
    return;
  }

  if(mag_savingthrow(vict, SAVING_PARA)) {
    send_to_char("Your victim resists.\r\n", ch);
    return;
  }

  for(f = ch->followers; f; f = f->next) {
    if(IS_AFFECTED(f->follower, AFF_CHARM)) {
      charmed_levels += GET_LEVEL(f->follower);
      num_charmies++;
    }
  }
  if(((charmed_levels+GET_LEVEL(vict)) > GET_LEVEL(ch)) || (num_charmies >= max_charmies)) {
    send_to_char("You don't have the power to control that victim.\r\n", ch);
    return;
  }

  if (vict->master)
    stop_follower(vict);
  add_follower(vict, ch);

  af.type = SPELL_CHARM;

  if (GET_INT(vict))
    af.duration = 24 * 18 / GET_INT(vict);
  else
    af.duration = 24 * 18;

  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(vict, &af);

  act("Isn't $n just such a nice fellow?", FALSE, ch, NULL, vict, TO_VICT);
  act("$n befriends $N.", TRUE, ch, NULL, vict, TO_ROOM);
  act("You befriend $N.", FALSE, ch, NULL, vict, TO_CHAR);

  if (IS_NPC(vict)) {
    REMOVE_BIT(MOB_FLAGS(vict), MOB_AGGRESSIVE);
    REMOVE_BIT(MOB_FLAGS(vict), MOB_SPEC);
  }
}


ACMD(do_camouflage)
{
  int i;

  if(!(i=get_skill(ch, SKILL_CAMOUFLAGE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!WILDERNESS(world[ch->in_room].sector_type)) {
    send_to_char("You can only camouflage yourself in the wilderness.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 10) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=10;

  send_to_char("You attempt to camouflage yourself.\r\n", ch);

  if((i+dex_app_skill[GET_DEX(ch)].hide)<number(1, 100)) {
    return;
  }

  SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
}


ACMD(do_scan)
{
  int i, scan_room;
  struct char_data *see;

  if(!(i=get_skill(ch, SKILL_SCAN))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 10) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=10;

  send_to_char("You carefully examine your surroundings.\r\n", ch);

  if(i<number(1, 100)) {
    return;
  }

  for(i=0; i<NUM_OF_DIRS; i++) {
    if(world[ch->in_room].dir_option[i] && (world[ch->in_room].dir_option[i]->to_room != NOWHERE)) {
      scan_room=world[ch->in_room].dir_option[i]->to_room;
      if(world[scan_room].people) {
        for(see=world[scan_room].people; see; see=see->next_in_room) {
          sprintf(buf, "You see $n %s from here.", dirs[i]);
          act(buf, TRUE, see, NULL, ch, TO_VICT);
        }
        send_to_char("\r\n", ch);
      }
    }
  }
}


ACMD(do_silentwalk)
{
  int i;
  struct affected_type af;

  if(!(i=get_skill(ch, SKILL_SILENTWALK))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 5) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=5;

  send_to_char("Ok, you'll try to move silently for a while.\r\n", ch);

  if((i+dex_app_skill[GET_DEX(ch)].sneak)<number(1, 100)) {
    return;
  }

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch)>>1;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}


ACMD(do_memorize)
{
  int i, spell, prob;
  struct obj_data *scroll;
  extern char *spells[];

  if(!(prob=get_skill(ch, SKILL_MEMORIZE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, arg);

  if(!*arg) {
    send_to_char("Memorize what?\r\n", ch);
    return;
  }

  if(!(scroll=get_obj_in_list_vis(ch, arg, ch->carrying))) {
    if(!(scroll=GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("You do not have that item.\r\n", ch);
      return;
    }
    if(!isname(arg, scroll->name)) {
      send_to_char("You do not have that item.\r\n", ch);
      return;
    }
    if(GET_OBJ_TYPE(scroll) != ITEM_SCROLL) {
      send_to_char("You can only memorize scrolls.\r\n", ch);
      return;
    }
    obj_to_char(unequip_char(ch, WEAR_HOLD), ch);
  }
  else if(GET_OBJ_TYPE(scroll) != ITEM_SCROLL) {
    send_to_char("You can only memorize scrolls.\r\n", ch);
    return;
  }

  for(i=1; i<4; i++) {
    spell=GET_OBJ_VAL(scroll, i);
    if(spell < 1)
      continue;
    if(get_skill_level(ch, spell) >= LVL_HERO) {
      sprintf(buf, "You are not the right class for %s.\r\n", spells[spell]);
      send_to_char(buf, ch);
    }
    else if((spell > MAX_SPELLS) || (GET_LEVEL(ch) < get_multi_skill_level(ch, spell))) {
      sprintf(buf, "%s is beyond your level.\r\n", spells[spell]);
      send_to_char(CAP(buf), ch);
    }
    else if(GET_SKILL(ch, spell)) {
      sprintf(buf, "You already know %s.\r\n", spells[spell]);
      send_to_char(buf, ch);
    }
    else if(prob < number(1, 100)) {
      sprintf(buf, "You fail to memorize %s.\r\n", spells[spell]);
      send_to_char(buf, ch);
    }
    else {
      SET_SKILL(ch, spell, MAX(1, prob>>2));
      sprintf(buf, "You memorize %s.\r\n", spells[spell]);
      send_to_char(buf, ch);
    }
  }

  act("$p crumbles to dust.", FALSE, ch, scroll, NULL, TO_CHAR);
  extract_obj(scroll);
}

ACMD(do_throw)
{
  int i, dam, room, type;
  struct obj_data *weapon;
  struct char_data *vict;

  if(!(i=get_skill(ch, SKILL_THROW))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  two_arguments(argument, arg, buf);

  if((!*arg) || (!*buf)) {
    send_to_char("Throw what at who?\r\n", ch);
    return;
  }

  if(!(weapon=get_obj_in_list_vis(ch, arg, ch->carrying))) {
    send_to_char("You aren't carrying anything like that.\r\n", ch);
    return;
  }

  if(!(vict=get_char_room_vis(ch, buf))) {
    send_to_char("You can't find your victim.\r\n", ch);
    return;
  }

  if(vict == ch) {
    send_to_char("Are you mad?!?\r\n", ch);
    return;
  }

  if(GET_OBJ_TYPE(weapon) != ITEM_WEAPON) {
    send_to_char("That isn't a weapon!\r\n", ch);
    return;
  }

  type = GET_OBJ_VAL(weapon, 3) + TYPE_HIT;
  if((type == TYPE_STING) || (type == TYPE_WHIP) || (type == TYPE_BLAST)) {
    send_to_char("Throwing that wouldn't do anything.\r\n", ch);
    return;
  }

  if(GET_OBJ_WEIGHT(weapon) > 5) {
    send_to_char("That is too heavy to throw.\r\n", ch);
    return;
  }

  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }

  act("You throw $p at $N.", FALSE, ch, weapon, vict, TO_CHAR);
  act("$n throws $p at $N.", TRUE, ch, weapon, vict, TO_NOTVICT);
  act("$n throws $p at you!", FALSE, ch, weapon, vict, TO_VICT);

  WAIT_STATE(ch, 3*PULSE_VIOLENCE);

  if(i < number(1, 100)) {
    obj_from_char(weapon);
    if((i=get_dir(ch)) < NUM_OF_DIRS) {
      room=world[ch->in_room].dir_option[i]->to_room;
      obj_to_room(weapon, room);
      if(world[room].people) {
        sprintf(buf, "$p flies in from %s.", edirs[i]);
        CAP(buf);
        act(buf, TRUE, world[room].people, weapon, NULL, TO_ROOM);
        act(buf, TRUE, world[room].people, weapon, NULL, TO_CHAR);
      }
    }
    else
      obj_to_room(weapon, ch->in_room);
    return;
  }

  if((!IS_NPC(ch))&&(GET_DAMROLL(ch) > MIN(50, GET_LEVEL(ch))))
    dam = MIN(50, GET_LEVEL(ch));
  else
    dam = GET_DAMROLL(ch);

  dam += dice(GET_OBJ_VAL(weapon, 1), GET_OBJ_VAL(weapon, 2));

  damage(ch, vict, dam, GET_OBJ_VAL(weapon, 3) + TYPE_HIT);
  extract_obj(weapon);
}


ACMD(do_mindcloud)
{
  int i;
  struct affected_type af;

  if(!(i=get_skill(ch, SKILL_MINDCLOUD))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  send_to_char("You start concentrating, trying to cloud the minds of those around you.\r\n", ch);

  if(i >= number(1, 100)) {
    af.type = SKILL_MINDCLOUD;
    af.bitvector = AFF_INVISIBLE;
    af.modifier = -50;
    af.location = APPLY_AC;
    af.duration = GET_LEVEL(ch);
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
}


ACMD(do_snare)
{
  int i;

  if(!(i=get_skill(ch, SKILL_SNARE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 15) {
    send_to_char("You're too exhausted!\r\n", ch);
    return;
  }

  GET_MOVE(ch) -= 15;

  send_to_char("You set a trap in the room.\r\n", ch);

  if(i >= number(1, 100)) {
    SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_SNARE);
  }
}


ACMD(do_featherfoot)
{
  int i;
  struct affected_type af;

  if(!(i=get_skill(ch, SKILL_FEATHERFOOT))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 20) {
    send_to_char("You're too exhausted.\r\n", ch);
  }

  GET_MOVE(ch) -= 20;

  if(i >= number(1, 100)) {
    send_to_char("You feel like you could walk on water.\r\n", ch);
    af.type = SKILL_FEATHERFOOT;
    af.bitvector = AFF_WATERWALK;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.duration = GET_LEVEL(ch) >> 2;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  else {
    send_to_char("Your feet aren't cooperating.\r\n", ch);
  }
}
