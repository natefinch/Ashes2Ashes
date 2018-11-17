/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
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
#include "class.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern char *dirs[];

/* extern functions */
void die(struct char_data * ch, struct char_data *killer);


ACMD(do_assist)
{
  struct char_data *helpee, *opponent;
  struct follow_type *f;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    if(AFF_FLAGGED(ch, AFF_GROUP)) {
      if(ch->master)
        helpee=ch->master;
      else
        helpee=ch;
      f=helpee->followers;
      while(f && helpee && ((!FIGHTING(helpee)) || (FIGHTING(FIGHTING(helpee))!=helpee))) {
        helpee=f->follower;
        f=f->next;
      }
      if((!helpee) || (!FIGHTING(helpee))) {
        send_to_char("Whom do you wish to assist?\r\n", ch);
        return;
      }
    }
    else {
      send_to_char("Whom do you wish to assist?\r\n", ch);
      return;
    }
  }
  else if (!(helpee = get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }

  if (helpee == ch) {
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  }
  else {
    for (opponent = world[ch->in_room].people;
	 opponent && (FIGHTING(opponent) != helpee);
	 opponent = opponent->next_in_room)
		;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_KILL(ch, opponent))
      act("You can't attack $N!", FALSE, ch, 0, opponent, TO_CHAR);
    else {
      if(AFF_FLAGGED(opponent, AFF_DIVINE_PROT) && number(0, 7))
        send_to_char("A divine force stops you!\r\n", ch);
      else if(AFF_FLAGGED(opponent, AFF_PROTECT_GOOD) && IS_GOOD(ch) && number(0, 7))
        send_to_char("An evil force stops you!\r\n", ch);
      else if(AFF_FLAGGED(opponent, AFF_PROTECT_EVIL) && IS_EVIL(ch) && number(0, 7))
        send_to_char("A good force stops you!\r\n", ch);
      else {
        send_to_char("You join the fight!\r\n", ch);
        act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
        act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
        hit(ch, opponent, TYPE_UNDEFINED);
      }
    }
  }
}


ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);
  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  }
  else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else if (!CAN_KILL(ch, vict))
    send_to_char("That wouldn't be very nice.\r\n", ch);
  else if ((!IS_NPC(ch)) && (GET_LEVEL(ch) >= LVL_HERO) && (GET_LEVEL(ch) < LVL_ASST) && (!GRNT_FLAGGED(ch, GRNT_KILL)))
    send_to_char("Immortals are forbidden to kill!\r\n", ch);
  else {
    if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
      return;		/* you can't order a charmed pet to attack a player */
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      if(AFF_FLAGGED(vict, AFF_DIVINE_PROT) && number(0, 7))
        send_to_char("A divine force stops you!\r\n", ch);
      else if(AFF_FLAGGED(vict, AFF_PROTECT_GOOD) && IS_GOOD(ch) && number(0, 7))
        send_to_char("An evil force stops you!\r\n", ch);
      else if(AFF_FLAGGED(vict, AFF_PROTECT_EVIL) && IS_EVIL(ch) && number(0, 7))
        send_to_char("A good force stops you!\r\n", ch);
      else
        hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}



ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_CIMP) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      if(GET_OLC_MODE(vict) || (vict->player_specials->zone_locked > 0)) {
        send_to_char("Slaying someone who is editting is a bad idea.\r\n", ch);
        return;
      }
      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      die(vict, ch);
      sprintf(buf, "(GC) %s has slain %s", GET_NAME(ch), GET_NAME(vict));
      mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    }
  }
}


ACMD(do_poisonblade)
{
  int prob;
  struct obj_data *weapon=GET_EQ(ch, WEAR_WIELD);

  if(!(prob=get_skill(ch, SKILL_POISONBLADE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }
  if(!weapon) {
    send_to_char("You must be wielding the weapon you want to poison.\r\n", ch);
    return;
  }
  if((GET_MOVE(ch) <= 15) || (GET_HIT(ch) <= 15)) {
    send_to_char("You are too exhausted.\r\n", ch);
    return;
  }
  GET_HIT(ch)-=10;
  GET_MOVE(ch)-=10;
  if(prob < number(1, 100)) {
    send_to_char("You try to poison your weapon, but fail.\r\n", ch);
    GET_HIT(ch)-=5;
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    return;
  }
  send_to_char("You skillfuly poison your weapon.\r\n", ch);
  GET_MOVE(ch)-=5;
  weapon->poisoned=number(GET_LEVEL(ch)>>2, (GET_LEVEL(ch)<<1)/5);
  WAIT_STATE(ch, 4*PULSE_VIOLENCE);
}


ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob;

  if(!get_skill(ch, subcmd==SCMD_DUAL?SKILL_DUAL_BACKSTAB:SKILL_BACKSTAB)) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_STAB - TYPE_HIT) {
    send_to_char("Only stabbing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (subcmd==SCMD_DUAL) {
    if ((!GET_EQ(ch, WEAR_HOLD)) || (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD))!=ITEM_WEAPON)) {
      send_to_char("You need to be holding a second weapon to make it a success.\r\n", ch);
      return;
    }
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3) != TYPE_STAB - TYPE_HIT) {
      send_to_char("Only stabbing weapons can be used for backstabbing.\r\n", ch);
      return;
    }
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }

  if ((MOB_FLAGGED(vict, MOB_AWARE)) || (IS_SET(GET_CLASS_BITVECTOR(vict), MK_F))) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = number(1, 100);
  if (subcmd==SCMD_DUAL)
    prob = get_skill(ch, SKILL_DUAL_BACKSTAB);
  else
    prob = get_skill(ch, SKILL_BACKSTAB);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else if (subcmd==SCMD_DUAL)
    hit(ch, vict, SKILL_DUAL_BACKSTAB);
  else
    hit(ch, vict, SKILL_BACKSTAB);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}


ACMD(do_circle)
{
  int percent, prob;

  if(!(prob=get_skill(ch, SKILL_CIRCLE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }
  if(!FIGHTING(ch)) {
    send_to_char("You must be fighting in order to circle.\r\n", ch);
    return;
  }
  if(FIGHTING(FIGHTING(ch))==ch) {
    send_to_char("You can't circle if you are the tank!\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_STAB - TYPE_HIT) {
    send_to_char("Only stabbing weapons can be used for circling.\r\n", ch);
    return;
  }

  percent = number(1, 100);

  if (AWAKE(FIGHTING(ch)) && (percent > prob))
    damage(ch, FIGHTING(ch), 0, SKILL_CIRCLE);
  else
    hit(ch, FIGHTING(ch), SKILL_CIRCLE);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}


bool check_order(char *order)
{
  int cmd, length;

  static char *bad_orders[] = {
    "adrenalincontrol",
    "backstab",
    "ballisticattack",
    "bash",
    "befriend",
    "berserk",
    "biofeedback",
    "bodyweaponry",
    "cannibalize",
    "cast",
    "celladjustment",
    "chameleonpower",
    "circle",
    "clairvoyance",
    "completehealing",
    "dangersense",
    "deathfield",
    "dimensiondoor",
    "disarm",
    "disintegrate",
    "displacement",
    "disrupt",
    "dive",
    "divine",
    "domination",
    "dualbackstab",
    "featherfoot",
    "filet",
    "forage",
    "egowhip",
    "energycontainment",
    "feellight",
    "flesharmor",
    "gouge",
    "grab",
    "graftweapon",
    "grapple",
    "hide",
    "hold",
    "hurl",
    "kick",
    "lendhealth",
    "lifedetection",
    "lifedraining",
    "magnify",
    "metamorphosis",
    "mindcloud",
    "molecularagitation",
    "pick",
    "poisonblade",
    "probabilitytravel",
    "psionicblast",
    "psychiccrush",
    "redirect",
    "request",
    "skin",
    "snare",
    "sneak",
    "splitpersonality",
    "stasisfield",
    "steal",
    "stuntouch",
    "telekinesis",
    "tell",
    "throw",
    "wear",
    "whirlwind",
    "wield",
    "\n"
  };



  length=strlen(order);
  for(cmd=0; *bad_orders[cmd]!='\n'; cmd++) {
    if(!strn_cmp(bad_orders[cmd], order, length)) {
      return FALSE;
    }
  }
  return TRUE;
}


ACMD(do_order)
{
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  bool found = FALSE;
  bool cmd_ok;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  one_argument(message, buf);
  cmd_ok=check_order(buf);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
	send_to_char(OK, ch);
        if(cmd_ok && (!GET_MOB_WAIT(vict)))
          command_interpreter(vict, message);
      }
    } else {			/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
            if(cmd_ok && (!GET_MOB_WAIT(k->follower)))
              command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char(OK, ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}



ACMD(do_flee)
{
  int i, attempt, loss;
  extern int max_exp_loss;
  struct char_data *fighting;

  if(((ch->desc) || IS_NPC(ch)) && (GET_POS(ch) < POS_FIGHTING)) {
    send_to_char("You have to be standing to flee!\r\n", ch);
    return;
  }

  if(IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_SNARE)) {
    if(number(1, 100) <= 5) {
      act("You try to flee and are caught in a snare, but break it!", TRUE, ch, NULL, NULL, TO_CHAR);
      act("$n tries to flee and is caught in a snare, but breaks it!", TRUE, ch, NULL, NULL, TO_ROOM);
      REMOVE_BIT(ROOM_FLAGS(ch->in_room), ROOM_SNARE);
    }
    else {
      act("You try to flee and are caught in a snare!", TRUE, ch, NULL, NULL, TO_CHAR);
      act("$n tries to flee and is caught in a snare.", TRUE, ch, NULL, NULL, TO_ROOM);
    }
    return;
  }

  for (i = 0; i < 8; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_NOFLEE)) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      fighting=FIGHTING(ch);
      if (do_simple_move(ch, attempt, TRUE)) {
	send_to_char("You flee head over heels.\r\n", ch);
	if (fighting) {
	  if (!IS_NPC(ch)) {
	    loss = GET_MAX_HIT(fighting) - GET_HIT(fighting);
	    loss *= 0.75*MIN(GET_LEVEL(fighting), GET_EXP(fighting)/MAX(1, GET_MAX_HIT(fighting)));
            loss /= MAX(1, 6-(GET_LEVEL(ch)/20));
            if(GET_LEVEL(ch) < 15)
              loss=0;
            loss=MIN(loss, max_exp_loss);
	    gain_exp(ch, -loss);
	  }
/*
	  if (FIGHTING(FIGHTING(ch)) == ch)
	    stop_fighting(FIGHTING(ch));
	  stop_fighting(ch);
*/
	}
      } else {
	act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_grapple)
{
  int percent, prob, cycles;

  if(!(prob=get_skill(ch, SKILL_GRAPPLE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }
  if(!FIGHTING(ch)) {
    send_to_char("You must be fighting in order to grapple.\r\n", ch);
    return;
  }
  if(FIGHTING(FIGHTING(ch))!=ch) {
    sprintf(buf, "But %s isn't attacking you!", GET_NAME(FIGHTING(ch)));
    send_to_char(buf, ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }

  percent = number(1, 100);
  prob += (GET_DEX(ch) - GET_DEX(FIGHTING(ch)))*5;
  prob += (GET_LEVEL(ch) - GET_LEVEL(FIGHTING(ch)))*2;
  prob = MIN(prob, 100);
  prob = MAX(prob, MIN(GET_SKILL(ch, SKILL_GRAPPLE), 5));

  if(percent > prob) {
    send_to_char("You fail to grapple your opponent.\r\n", ch);
    act("$n fails to grapple $N.", TRUE, ch, NULL, FIGHTING(ch), TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
  }
  else {
    if(GET_LEVEL(ch) > 40)
      cycles=2;
    else
      cycles=1;
    GET_STUN(FIGHTING(ch))=cycles;
    GET_STUN(ch)=1;
    send_to_char("You grapple your opponent, holding them motionless.\r\n", ch);
    act("$n grapples $N, holding $M motionless.", TRUE, ch, NULL, FIGHTING(ch), TO_ROOM);
    WAIT_STATE(FIGHTING(ch), PULSE_VIOLENCE*(cycles+1));
    WAIT_STATE(ch, PULSE_VIOLENCE*5);
  }
}


ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;

  if(!(prob=get_skill(ch, SKILL_BASH))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if ((!GET_EQ(ch, WEAR_WIELD)) && (!IS_NPC(ch))) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }

  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }

  percent = number(1, 100);
  prob += (GET_DEX(ch) - GET_DEX(vict))*3;
  prob += (GET_LEVEL(ch) - GET_LEVEL(vict))*2;
  prob = MIN(prob, 100);
  prob = MAX(prob, MIN(GET_SKILL(ch, SKILL_BASH), 5));

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BASH);
    GET_POS(ch) = POS_SITTING;
  } else {
    if(vict->in_room!=NOWHERE) {
      GET_POS(vict) = POS_SITTING;
      WAIT_STATE(vict, PULSE_VIOLENCE);
    }
    damage(ch, vict, 1, SKILL_BASH);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  if(!get_skill(ch, SKILL_RESCUE)) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }

  if(!CAN_KILL(ch, tmp_ch)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }

  percent = number(1, 100);
  prob = get_skill(ch, SKILL_RESCUE);

  if (percent > prob) {
    send_to_char("You fail the rescue!\r\n", ch);
    return;
  }
  send_to_char("Banzai!  To the rescue...\r\n", ch);
  act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
  act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

  if (FIGHTING(vict) == tmp_ch)
    stop_fighting(vict);
  if (FIGHTING(tmp_ch))
    stop_fighting(tmp_ch);
  if (FIGHTING(ch))
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
}



ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;

  if(!(prob=get_skill(ch, SKILL_KICK))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }

  percent = ((10 - (GET_AC(vict) / 10)) << 1) + number(1, 100);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KICK);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 2, SKILL_KICK);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_stuntouch)
{
  int i;

  if(!(i=get_skill(ch, SKILL_STUNTOUCH))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!FIGHTING(ch)) {
    send_to_char("You must be fighting in order to stuntouch.\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 10) {
    send_to_char("You don't have enough energy to do that right now.\r\n", ch);
    return;
  }
  GET_MOVE(ch)-=10;

  i+=(GET_DEX(ch)-GET_DEX(FIGHTING(ch)))*5;
  i+=MIN((GET_LEVEL(ch)-GET_LEVEL(FIGHTING(ch)))*2, 0);

  if(i<number(1, 100)) {
    send_to_char("You failed to stun your opponent.\r\n", ch);
    act("$n fails to stun $N", TRUE, ch, NULL, FIGHTING(ch), TO_NOTVICT);
    act("$n tries to stun you, but fails.", FALSE, ch, NULL, FIGHTING(ch), TO_VICT);
    WAIT_STATE(ch, PULSE_VIOLENCE*4);
    return;
  }

  if(GET_LEVEL(ch) < 40)
    i=1;
  else if(GET_LEVEL(ch) < 80)
    i=number(1, 2);
  else
    i=number(1, 3);
  GET_POS(FIGHTING(ch))=POS_STUNNED;
  GET_STUN(FIGHTING(ch))=i;
  send_to_char("You stun your opponent.\r\n", ch);
  act("$n stuns $N.", TRUE, ch, NULL, FIGHTING(ch), TO_NOTVICT);
  act("$n stuns you!", FALSE, ch, NULL, FIGHTING(ch), TO_VICT);
  WAIT_STATE(ch, PULSE_VIOLENCE*4);
  WAIT_STATE(FIGHTING(ch), PULSE_VIOLENCE*i);
}


ACMD(do_berserk)
{
  int percent, prob, found=0;
  struct char_data *tch, *next_ch;

  if(!(prob=get_skill(ch, SKILL_BERSERK))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }
  if((GET_MOVE(ch) < 10) && (!IS_NPC(ch))) {
    send_to_char("You are too exhausted.\r\n", ch);
    return;
  }

  percent = number(1, 100);

  if(percent > prob) {
    send_to_char("You try to go berserk, but trip.\r\n", ch);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
  }
  else {
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    for(tch=world[ch->in_room].people; tch; tch=next_ch) {
      next_ch=tch->next_in_room;
      if(ch==tch)
        continue;
      if(IS_NPC(ch)) {
        if(AFF_FLAGGED(ch, AFF_CHARM)) {
          if(IS_NPC(tch)) {
            if(!found) {
              act("$n GOES BERSERK AND ATTACKS EVERYTHING IN SIGHT!", FALSE, ch, NULL, NULL, TO_ROOM);
              act("YOU GO BERSERK AND ATTACK EVERYTHING IN SIGHT!", FALSE, ch, NULL, NULL, TO_CHAR);
            }
            hit(ch, tch, TYPE_UNDEFINED);
            found++;
          }
        }
        else {
          if(!IS_NPC(tch) && ((GET_LEVEL(tch) < LVL_HERO) || PRF_FLAGGED(tch, PRF_AVTR))) {
            if(!found) {
              act("$n GOES BERSERK AND ATTACKS EVERYTHING IN SIGHT!", FALSE, ch, NULL, NULL, TO_ROOM);
              act("YOU GO BERSERK AND ATTACK EVERYTHING IN SIGHT!", FALSE, ch, NULL, NULL, TO_CHAR);
            }
            hit(ch, tch, TYPE_UNDEFINED);
            found++;
          }
        }
      }
      else if(IS_NPC(tch) && CAN_SEE(ch, tch)) {
        if(!found) {
          act("$n GOES BERSERK AND ATTACKS EVERYTHING IN SIGHT!", FALSE, ch, NULL, NULL, TO_ROOM);
          act("YOU GO BERSERK AND ATTACK EVERYTHING IN SIGHT!", FALSE, ch, NULL, NULL, TO_CHAR);
        }
        hit(ch, tch, TYPE_UNDEFINED);
        found++;
      }
    }
    if(!found) {
      act("$n swings wildly at the air.", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You swing wildly at the air.", FALSE, ch, NULL, NULL, TO_CHAR);
    }
  }
  if(!IS_NPC(ch))
    GET_MOVE(ch) -= 10;
}


ACMD(do_whirlwind)
{
  int percent, prob, found=0;
  struct char_data *tch, *next_ch, *master;

  if(!(prob=get_skill(ch, SKILL_WHIRLWIND))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }
  if(GET_MOVE(ch) < 20) {
    send_to_char("You are too exhausted.\r\n", ch);
    return;
  }

  GET_MOVE(ch) -= 10;

  percent = number(1, 100);

  if(percent > prob) {
    send_to_char("You spin wildly, hitting nothing but air.\r\n", ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
  else {
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    if(ch->master)
      master=ch->master;
    else
      master=ch;
    for(tch=world[ch->in_room].people; tch; tch=next_ch) {
      next_ch=tch->next_in_room;
      if((tch==master) || (tch->master==master))
        continue;
      if(!CAN_KILL(ch, tch))
        continue;
      if(!IS_NPC(tch) && (GET_LEVEL(tch) >= LVL_HERO) && (!PRF_FLAGGED(tch, PRF_AVTR)))
        continue;
      if(MOB_FLAGGED(tch, MOB_DOORSTOP))
        continue;
      if(!found) {
        act("$n spins out of control, hitting everyone near $m!", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You spin out of control, hitting everyone near you!", FALSE, ch, NULL, NULL, TO_CHAR);
      }
      hit(ch, tch, TYPE_UNDEFINED);
      found++;
    }
    if(!found)
      send_to_char("You spin wildly, hitting nothing but air.\r\n", ch);
    else
      GET_MOVE(ch) -= 10;
  }
}


ACMD(do_redirect)
{
  int prob;
  struct char_data *vict;

  if(!(prob=get_skill(ch, SKILL_REDIRECT))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if(!FIGHTING(ch)) {
    send_to_char("You need to be fighting to do that.\r\n", ch);
    return;
  }
  if(!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Who would you like to redirect your strikes at?\r\n", ch);
    return;
  }
  if(vict==ch) {
    send_to_char("Aren't we funny today?\r\n", ch);
    return;
  }
  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }
  if(FIGHTING(ch)==vict) {
    act("But you are already fighting $M.", FALSE, ch, NULL, vict, TO_CHAR);
  }
  if(FIGHTING(vict)!=ch) {
    act("But $N is not attacking you!", FALSE, ch, NULL, vict, TO_CHAR);
    return;
  }

  if(prob < number(1, 100)) {
    act("You fail to control your anger and continue fighting $N.", FALSE, ch, NULL, FIGHTING(ch), TO_CHAR);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }

  FIGHTING(ch)=vict;
  act("You quickly evade your opponent and start fighting $N.", FALSE, ch, NULL, FIGHTING(ch), TO_CHAR);
  act("$n quickly evades $s opponent and starts fighting $N.", FALSE, ch, NULL, FIGHTING(ch), TO_NOTVICT);
  act("$n quickly evades $s opponent and starts fighting YOU!", FALSE, ch, NULL, FIGHTING(ch), TO_VICT);
  WAIT_STATE(ch, 2*PULSE_VIOLENCE);
  return;
}


ACMD(do_hurl)
{
  int dir, was_in, prob;
  struct char_data *vict, *tch;

  if(!(prob=get_skill(ch, SKILL_HURL))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }
  if(!FIGHTING(ch)) {
    send_to_char("You need to be fighting in order to hurl!\r\n", ch);
    return;
  }
  vict=FIGHTING(ch);

  if(vict->in_room != ch->in_room)
    return;

  if((prob < number(1, 100)) || ((2*GET_WEIGHT(ch))<GET_WEIGHT(vict))) {
    damage(ch, vict, 0, SKILL_HURL);
    WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    return;
  }
  if(((dir=get_dir(vict))==NUM_OF_DIRS) || ((number(1, 100) + ((GET_WEIGHT(vict)-GET_WEIGHT(ch))/6)) > 25)) {
    if(GET_POS(vict)>POS_SITTING)
      GET_POS(vict)=POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    damage(ch, vict, GET_LEVEL(ch)>>2, SKILL_HURL);
  }
  else {
    for(tch=world[ch->in_room].people; tch; tch=tch->next_in_room) {
      if(FIGHTING(tch)==vict)
        stop_fighting(tch);
    }
    stop_fighting(vict);
    act("You hurl $N out of the room!", FALSE, ch, NULL, vict, TO_CHAR);
    act("$n hurls $N out of the room!", FALSE, ch, NULL, vict, TO_NOTVICT);
    act("$n hurls you out of the room!", FALSE, ch, NULL, vict, TO_VICT);
    was_in=ch->in_room;
    char_from_room(vict);
    char_to_room(vict, world[was_in].dir_option[dir]->to_room);
    act("$n arrives hurling into the room!", FALSE, vict, NULL, NULL, TO_ROOM);
    if(GET_POS(vict)>POS_SITTING)
      GET_POS(vict)=POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    damage(ch, vict, GET_LEVEL(ch)>>2, TYPE_SILENT);
  }
}


ACMD(do_disarm)
{
  int i, level_diff;
  struct char_data *vict;

  if(!(i=get_skill(ch, SKILL_DISARM))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!(vict=FIGHTING(ch))) {
    send_to_char("You can only disarm if you're fighting.\r\n", ch);
    return;
  }

  if(!GET_EQ(vict, WEAR_WIELD)) {
    send_to_char("Your opponent is already bare-handed.\r\n", ch);
    return;
  }

  level_diff=GET_LEVEL(vict)-GET_LEVEL(ch);
  level_diff=2*MAX(level_diff, 0);

  WAIT_STATE(ch, 3*PULSE_VIOLENCE);

  if(i < (number(1, 100)+level_diff)) {
    send_to_char("You fail to disarm your opponent.\r\n", ch);
    sprintf(buf, "%s tries to disarm you but fails.\r\n", GET_NAME(ch));
    send_to_char(buf, vict);
    act("$n tries to disarm $N but fails.", TRUE, ch, NULL, vict, TO_NOTVICT);
    return;
  }

  if(IS_NPC(vict))
    SET_BIT(MOB_FLAGS(vict), MOB_AWARE);
  obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
  act("You disarm $N!", FALSE, ch, NULL, vict, TO_CHAR);
  act("$n disarms you!", FALSE, ch, NULL, vict, TO_VICT);
  act("$n disarms $N!", FALSE, ch, NULL, vict, TO_NOTVICT);
}


ACMD(do_disrupt)
{
  int i, dex, level;

  if(!(i=get_skill(ch, SKILL_DISRUPT))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!FIGHTING(ch)) {
    send_to_char("You can only disrupt someone you're fighting!\r\n", ch);
    return;
  }

  level=GET_LEVEL(FIGHTING(ch))-GET_LEVEL(ch);
  dex=3*(GET_DEX(FIGHTING(ch))-GET_DEX(ch));

  if((i-dex-level) < number(1, 100)) {
    act("You try to disrupt $m with your quick moves, but only entangle yourself.", FALSE, FIGHTING(ch), NULL, ch, TO_VICT);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    return;
  }

  i=number(1, 3);
  if(GET_LEVEL(ch) >= 80)
    i++;

  act("You begin hitting $N with lightning quick attacks,\r\n disrupting $S concentration.", FALSE, ch, NULL, FIGHTING(ch), TO_CHAR);
  act("$n begins hitting $N with lightning quick attacks,\r\n disrupting $S concentration.", TRUE, ch, NULL, FIGHTING(ch), TO_NOTVICT);
  act("$n begins hitting you with lightning quick attacks,\r\n disrupting your concentration.", FALSE, ch, NULL, FIGHTING(ch), TO_VICT);
  GET_STUN(ch) += i-1;
  WAIT_STATE(FIGHTING(ch), i*PULSE_VIOLENCE);
  WAIT_STATE(ch, 6*PULSE_VIOLENCE);
}


ACMD(do_dive)
{
  int i, dex, dir, room;

  if(!(i=get_skill(ch, SKILL_DIVE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!FIGHTING(ch)) {
    send_to_char("You can only dive out of a fight if you're fighting!\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 15) {
    send_to_char("You're too exhausted!\r\n", ch);
    return;
  }

  dex=2*(GET_DEX(FIGHTING(ch))-GET_DEX(ch));

  if(((i-dex) < number(1, 100)) || ((dir=get_dir(ch)) == NUM_OF_DIRS)) {
    send_to_char("You try to dive out of the fight, but only trip and fall.\r\n", ch);
    WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    GET_MOVE(ch) -= 5;
    return;
  }

  if (FIGHTING(FIGHTING(ch)) == ch)
    stop_fighting(FIGHTING(ch));
  stop_fighting(ch);

  GET_MOVE(ch) -= 15;

  act("$n dives out of the fight!\r\n", TRUE, ch, NULL, NULL, TO_ROOM);
  room=ch->in_room;
  char_from_room(ch);
  char_to_room(ch, world[room].dir_option[dir]->to_room);

  sprintf(buf, "You dive %s out of the fight.\r\n", dirs[dir]);
  send_to_char(buf, ch);

  if(IS_NPC(ch) || (GET_LEVEL(ch) < LVL_HERO))
    dt_damage(ch);
}


ACMD(do_gouge)
{
  int i, level, type, dam;
  struct obj_data *weapon;
  struct affected_type af;

  extern int dam_type_attack[];

  if(!(i=get_skill(ch, SKILL_GOUGE))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!FIGHTING(ch)) {
    send_to_char("You can only gouge if you're fighting!\r\n", ch);
    return;
  }

  if(GET_MOVE(ch) < 10) {
    send_to_char("You're too exhausted!\r\n", ch);
    return;
  }

  if(!(weapon=GET_EQ(ch, WEAR_WIELD))) {
    send_to_char("You must be wielding a weapon to gouge.\r\n", ch);
    return;
  }

  if(GET_OBJ_TYPE(weapon) != ITEM_WEAPON) {
    send_to_char("That isn't a weapon!\r\n", ch);
    return;
  }

  type = GET_OBJ_VAL(weapon, 3);

  if(!(dam_type_attack[type] & (DAMTYPE_SLASH | DAMTYPE_PIERCE))) {
    send_to_char("You need a sharp weapon to gouge with.\r\n", ch);
    return;
  }

  level=MAX(2*(GET_LEVEL(FIGHTING(ch))-GET_LEVEL(ch)), 0);

  GET_MOVE(ch) -= 10;

  if(((i-level) < number(1, 100)) || MOB_FLAGGED(FIGHTING(ch), MOB_NOBLIND)) {
    sprintf(buf, "You try to gouge %s's eyes, but fail.\r\n", GET_NAME(FIGHTING(ch)));
    send_to_char(buf, ch);
    act("$n takes a swipe at $N's eyes.", FALSE, ch, NULL, FIGHTING(ch), TO_NOTVICT);
    act("$n takes a swipe at your eyes!", TRUE, ch, NULL, FIGHTING(ch), TO_VICT);
    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    return;
  }

  WAIT_STATE(ch, 5*PULSE_VIOLENCE);

  if((!IS_NPC(ch))&&(GET_DAMROLL(ch) > MIN(50, GET_LEVEL(ch))))
    dam = MIN(50, GET_LEVEL(ch));
  else
    dam = GET_DAMROLL(ch);

  dam += dice(GET_OBJ_VAL(weapon, 1), GET_OBJ_VAL(weapon, 2));

  dam *= 3;

  act("You gouge $N's eyes!", FALSE, ch, NULL, FIGHTING(ch), TO_CHAR);
  act("$n gouges $N's eyes!", TRUE, ch, NULL, FIGHTING(ch), TO_NOTVICT);
  act("$n gouges your eyes!", FALSE, ch, NULL, FIGHTING(ch), TO_VICT);

  if((!affected_by_spell(FIGHTING(ch), SPELL_BLINDNESS)) && (!affected_by_spell(FIGHTING(ch), SKILL_GOUGE))) {
    af.type = SKILL_GOUGE;
    af.bitvector = AFF_BLIND;
    af.modifier = 30;
    af.location = APPLY_AC;
    af.duration = 1;
    affect_join(FIGHTING(ch), &af, FALSE, FALSE, FALSE, FALSE);

    af.modifier = -3;
    af.location = APPLY_HITROLL;
    af.duration = 1;
    affect_join(FIGHTING(ch), &af, FALSE, FALSE, FALSE, FALSE);
  }

  damage(ch, FIGHTING(ch), dam, type+TYPE_HIT);

}
