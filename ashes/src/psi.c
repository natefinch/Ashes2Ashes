/* ************************************************************************
*                                                                         *
*  Copyright (C) 1997 by Jesse Sterr                                      *
*                                                                         *
*  This file contains most of the code for making psionics work.          *
************************************************************************* */

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
#include "class.h"


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
void forget(struct char_data * ch, struct char_data * victim);
SPECIAL(shop_keeper);

int psi_clairvoyance(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_danger_sense(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_feel_light(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_disintegrate(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_telekinesis(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_ballistic_attack(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_molecular_agitation(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_complete_healing(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_death_field(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_energy_containment(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_life_draining(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_metamorphosis(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_adrenalin_control(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_biofeedback(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_body_weaponry(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_cell_adjustment(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_chameleon_power(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_displacement(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_flesh_armor(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_graft_weapon(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_lend_health(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_probability_travel(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_dimension_door(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_domination(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_psionic_blast(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_ego_whip(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_life_detection(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_psychic_crush(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_split_personality(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_cannibalize(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_magnify(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
int psi_stasis_field(struct char_data *ch, char *argument, int psi_power, int power_score, int roll);

struct psi_stat_chart psi_stats[PSI_END-PSI_START+1] = {
/* abil     mod  hp  move mana function */
  {PSI_WIS, -2,   0,   0,  55, psi_clairvoyance},		/* -4 */
  {PSI_WIS, -3,   0,  10,  30, psi_danger_sense},		/* -3 */
  {PSI_WIS, -3,  10,   0,  45, psi_feel_light},			/* -3 */
  {PSI_WIS, -3,  50,  10, 130, psi_disintegrate},		/* -4 */
  {PSI_WIS, -2,   0,  15,  25, psi_telekinesis},		/* -3 */
  {PSI_CON, -1,   0,  10,  35, psi_ballistic_attack},		/* -2 */
  {PSI_INT, -2,   5,   0,  45, psi_molecular_agitation},	/* -3 */
  {PSI_CON,  0,   0,  50, 145, psi_complete_healing},		/*  0 */
  {PSI_CON, -5,   0,  40, 155, psi_death_field},		/* -8 */
  {PSI_CON, -2,  10,   5,  55, psi_energy_containment},		/* -2 */
  {PSI_CON, -3,   0,   5,  60, psi_life_draining},		/* -3 */
  {PSI_CON, -4,  25,  25, 105, psi_metamorphosis},		/* -6 */
  {PSI_CON, -3,   0,   0,  50, psi_adrenalin_control},		/* -3 */
  {PSI_CON, -2,   0,  10,  40, psi_biofeedback},		/* -2 */
  {PSI_CON, -2,  20,   0,  55, psi_body_weaponry},		/* -3 */
  {PSI_CON, -2,   0,   0,  35, psi_cell_adjustment},		/* -3 */
  {PSI_CON, -1,   0,   0,  40, psi_chameleon_power},		/* -1 */
  {PSI_CON, -2,   0,  15,  40, psi_displacement},		/* -3 */
  {PSI_CON, -2,  15,   0,  50, psi_flesh_armor},		/* -3 */
  {PSI_CON, -4,  20,   0,  55, psi_graft_weapon},		/* -5 */
  {PSI_CON, -1,   0,   0,  30, psi_lend_health},		/* -1 */
  {PSI_INT,  0,   0,   0, 100, psi_probability_travel},		/*  0 */
  {PSI_CON, -1,   5,  15, 105, psi_dimension_door},		/* -1 */
  {PSI_WIS, -4,   0,   0,  50, psi_domination},			/* -4 */
  {PSI_WIS, -3,  10,   0,  45, psi_psionic_blast},		/* -5 */
  {PSI_WIS, -2,   0,   0,  30, psi_ego_whip},			/* -3 */
  {PSI_INT, -2,   0,  10,  25, psi_life_detection},		/* -2 */
  {PSI_WIS, -3,   0,   0,  40, psi_psychic_crush},		/* -4 */
  {PSI_WIS, -4,  10,   0, 155, psi_split_personality},		/* -5 */
  {PSI_CON,  0, 100,   0,   0, psi_cannibalize},		/*  0 */
  {PSI_WIS, -4,  30,  30, 175, psi_magnify},			/* -5 */
  {PSI_CON, -3,   0,  20, 100, psi_stasis_field}		/* -3 */
};

ACMD(do_psi)
{
  int i, prob;

  if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
    return;

  if(!(prob=get_skill(ch, subcmd))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  if(!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_TOURING)) {
    send_to_char("You cannot use psionic powers while you are touring.\r\n", ch);
    return;
  }

  subcmd-=PSI_START;

  if((GET_HIT(ch) < psi_stats[subcmd].hp) ||
     (GET_MOVE(ch) < psi_stats[subcmd].move) ||
     (GET_MANA(ch) < psi_stats[subcmd].mana)) {
    send_to_char("You don't have enough energy to do that.\r\n", ch);
    return;
  }

  prob = (prob/5)+1;
  switch(psi_stats[subcmd].abil) {
  case PSI_STR:
    prob += (GET_STR(ch)-18);
    break;
  case PSI_INT:
    prob += (GET_INT(ch)-18);
    break;
  case PSI_WIS:
    prob += (GET_WIS(ch)-18);
    break;
  case PSI_DEX:
    prob += (GET_DEX(ch)-18);
    break;
  case PSI_CON:
    prob += (GET_CON(ch)-18);
    break;
  case PSI_CHA:
    prob += (GET_CHA(ch)-18);
    break;
  }
  prob += psi_stats[subcmd].mod;

  if(AFF_FLAGGED(ch, AFF_SPLIT_PERSONALITY))
    i=PULSE_VIOLENCE/2;
  else
    i=PULSE_VIOLENCE;
  WAIT_STATE(ch, i);

  i=number(1, 20);
  if((i==20)&&(GET_LEVEL(ch)>=LVL_HERO)&&(!IS_NPC(ch)))
    i=19;
  if(i > prob) {
    GET_HIT(ch)-=psi_stats[subcmd].hp/2;
    GET_MOVE(ch)-=psi_stats[subcmd].move/2;
    GET_MANA(ch)-=psi_stats[subcmd].mana/2;
  }
  if((i <= prob) || (i==20)) {
    if(psi_stats[subcmd].func(ch, argument, subcmd, MIN(prob, 19), i)) {
      if((GET_LEVEL(ch)<LVL_HERO)&&(!IS_NPC(ch))) {
        GET_HIT(ch)-=psi_stats[subcmd].hp;
        GET_MOVE(ch)-=psi_stats[subcmd].move;
        GET_MANA(ch)-=psi_stats[subcmd].mana;
      }
    }
  }
  else {
    send_to_char("You fail to activate the power.\r\n", ch);
  }
}


ACMD(do_remember)
{
  int mem_num, prob;

  if(IS_NPC(ch))
    return;

  if(!(prob=get_skill(ch, SPELL_REMEMBER))) {
    send_to_char("You can't do that.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, arg);
  mem_num=atoi(arg);
  if((mem_num > 0) && (mem_num <= GET_INT(ch)/4) && (mem_num <= MAX_REMEMBER)) {
    if(zone_table[world[ch->in_room].zone].number == ARENA_ZONE) {
      send_to_char("You cannot memorize locations in the arena.\r\n", ch);
      return;
    }
    GET_REMEMBER(ch, mem_num-1)=world[ch->in_room].number;
    send_to_char("Location commited to memory.\r\n", ch);
  }
  else {
    send_to_char("You don't have that memory number available.\r\n", ch);
    return;
  }
}

void demagnify(struct char_data *ch)
{
  struct spcontinuous *s;

  for(s=ch->char_specials.spcont; s; s=s->next) {
    if(s->spspell==SKILL_MAGNIFY) {
      if(s->spdata1)
        s->spdata1--;
      else
        s->sptimer=0;
    }
  }
}

void report_danger(struct char_data *ch)
{
  int i, room;
  struct char_data *tch;

  if(ch->in_room==NOWHERE)
    return;

  for(i=0; i<NUM_OF_DIRS; i++) {
    if(EXIT(ch, i) && ((room=EXIT(ch, i)->to_room) != NOWHERE)) {
      if(ROOM_FLAGGED(room, ROOM_DEATH)) {
        sprintf(buf, "You sense danger %s.\r\n", dirs[i]);
        send_to_char(buf, ch);
        continue;
      }
      for(tch=world[room].people; tch; tch=tch->next_in_room) {
        if(IS_NPC(tch) && (MOB_FLAGGED(tch, MOB_AGGRESSIVE) ||
           (MOB_FLAGGED(tch, MOB_AGGR_EVIL) && IS_EVIL(ch)) ||
           (MOB_FLAGGED(tch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(ch)) ||
           (MOB_FLAGGED(tch, MOB_AGGR_GOOD) && IS_GOOD(ch)))) {
          sprintf(buf, "You sense danger %s.\r\n", dirs[i]);
          send_to_char(buf, ch);
          break;
        }
      }
    }
  }
}

int psi_clairvoyance(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int i, room, dir, temp;
  struct affected_type af;
  void look_at_target(struct char_data *ch, char *arg);

  if(roll==20) {
    send_to_char("You accidentaly shut off your vision!\r\n", ch);
    af.type = SKILL_CLAIRVOYANCE;
    af.bitvector = AFF_BLIND;
    af.location = APPLY_HITROLL;
    af.duration = number(1, 4);
    af.modifier = -4;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.location = APPLY_AC;
    af.modifier = 40;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    return FALSE;
  }

  argument=one_argument(argument, buf1);
  one_argument(argument, arg);
  if(!*buf1) {
    send_to_char("You must specify a location.\r\n", ch);
    return FALSE;
  }

  room=ch->in_room;
  for(i=0; buf1[i]; i++) {
    switch(LOWER(buf1[i])) {
    case 'n':
      dir=NORTH;
      break;
    case 'e':
      dir=EAST;
      break;
    case 's':
      dir=SOUTH;
      break;
    case 'w':
      dir=WEST;
      break;
    case 'u':
      dir=UP;
      break;
    case 'd':
      dir=DOWN;
      break;
    case 'h':
      dir=NORTHWEST;
      break;
    case 'i':
      dir=NORTHEAST;
      break;
    case 'k':
      dir=SOUTHWEST;
      break;
    case 'l':
      dir=SOUTHEAST;
      break;
    default:
      sprintf(buf, "Direction %d(%c) is not a direction.\r\n", i+1, buf1[i]);
      send_to_char(buf, ch);
      return FALSE;
      break;
    }
    if((!world[room].dir_option[dir]) || (world[room].dir_option[dir]->to_room==NOWHERE)) {
      sprintf(buf, "Direction %d(%c) does not exist.\r\n", i+1, buf1[i]);
      send_to_char(buf, ch);
      return FALSE;
    }
    room=world[room].dir_option[dir]->to_room;
  }

  if(GET_MANA(ch) < i*10+55) {
    send_to_char("You don't have enough mana for that.\r\n", ch);
    return FALSE;
  }
  GET_MANA(ch)-=i*10;

  temp=ch->in_room;
  char_from_room(ch);
  char_to_room(ch, room);
  if(!*arg)
    look_at_room(ch, 0);
  else
    look_at_target(ch, arg);
  char_from_room(ch);
  char_to_room(ch, temp);
  return TRUE;
}

int psi_danger_sense(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  report_danger(ch);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  return TRUE;
}

int psi_feel_light(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_FEEL_LIGHT;
  af.bitvector = AFF_FEEL_LIGHT;
  af.location = APPLY_NONE;
  af.duration = -1;
  af.modifier = 0;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_disintegrate(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int dam;
  struct char_data *vict;

  if(roll==20) {
    send_to_char("Oops!\r\n", ch);
    if((!mag_savingthrow(ch, SAVING_PETRI)) && (!number(0, 3)))
      damage(ch, ch, number(1, 200)+500, SKILL_DISINTEGRATE);
    else
      damage(ch, ch, number(1, 100)+250, SKILL_DISINTEGRATE);
    return FALSE;
  }

  one_argument(argument, arg);
  if(!*arg) {
    if(FIGHTING(ch))
      vict=FIGHTING(ch);
    else {
      send_to_char("You must specify a victim.\r\n", ch);
      return FALSE;
    }
  }
  else if(!(vict=get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  if(vict==ch) {
    send_to_char("That wouldn't be very bright.\r\n", ch);
    return FALSE;
  }
  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  if(((3*GET_LEVEL(vict)/2) <= GET_LEVEL(ch)) && (GET_MAX_HIT(vict) < GET_MAX_HIT(ch))) {
    if(number(1, 100) <= GET_PR(vict)) {
      damage(ch, vict, 0, SKILL_DISINTEGRATE);
    }
    else {
      GET_HIT(vict)=1;
      damage(ch, vict, 1000, SKILL_DISINTEGRATE);
    }
    return TRUE;
  }

  if(roll==power_score) {
    send_to_char("That worked really well.\r\n", ch);
    dam=number(1, 400)+1000;
  }
  else {
    dam=number(1, 200)+500;
  }
  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    dam*=2;
    demagnify(ch);
  }
  if(number(1, 100) <= GET_PR(vict))
    damage(ch, vict, 0, SKILL_DISINTEGRATE);
  else
    damage(ch, vict, dam, SKILL_DISINTEGRATE);
  return TRUE;
}

int psi_telekinesis(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct obj_data *obj;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  skip_spaces(&argument);
  if(!(obj=get_obj_in_list_vis(ch, argument, world[ch->in_room].contents))) {
    send_to_char("There is no item like that here.\r\n", ch);
    return FALSE;
  }
  if(!(GET_OBJ_WEAR(obj) & ITEM_WEAR_TAKE)) {
    send_to_char("As hard as you try, you still can't lift it.\r\n", ch);
    return FALSE;
  }
  obj_from_room(obj);
  obj_to_char(obj, ch);
  mag_continuous(GET_LEVEL(ch), ch, NULL, obj, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_ballistic_attack(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int type=TYPE_HIT, dam=0;
  struct char_data *vict;
  struct obj_data *obj;

  argument=one_argument(argument, arg);
  one_argument(argument, buf1);

  if((!*arg) || (!(vict=get_char_room_vis(ch, arg)))) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("Attack who?\r\n", ch);
    return FALSE;
  }
  if((!*buf1) || (!(obj=get_obj_in_list_vis(ch, buf1, ch->carrying)))) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("Hurl what item?\r\n", ch);
    return FALSE;
  }

  if(vict == ch) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("Are you mad?!?\r\n", ch);
    return FALSE;
  }

  if(!CAN_KILL(ch, vict)) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  if(GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
    type += GET_OBJ_VAL(obj, 3);
    if((type == TYPE_STING) || (type == TYPE_WHIP) || (type == TYPE_BLAST)) {
      dam=dice(3*GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
    }
  }
  if(!dam)
    dam=dice(5, 6);

  if(roll==20) {
    sprintf(buf, "You hurl %s away, but it turns around and comes back!\r\n", obj->short_description);
    send_to_char(buf, ch);
    vict=ch;
  }
  else {
    sprintf(buf, "You hurl %s at %s.\r\n", obj->short_description, GET_NAME(vict));
    send_to_char(buf, ch);
    act("$n sends $p hurling at you!", FALSE, ch, obj, vict, TO_VICT);
    act("$n sends $p hurling at $N.", FALSE, ch, obj, vict, TO_NOTVICT);
    if(roll==power_score) {
      send_to_char("That worked really well.\r\n", ch);
      dam*=2;
    }
  }

  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    dam*=2;
    demagnify(ch);
  }

  obj_from_char(obj);
  obj_to_char(obj, vict);
  damage(ch, vict, dam, type);

  if(roll==20)
    return FALSE;
  else
    return TRUE;
}

int psi_molecular_agitation(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct char_data *vict;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  one_argument(argument, arg);
  if(!*arg) {
    if(FIGHTING(ch))
      vict=FIGHTING(ch);
    else {
      send_to_char("You must specify a victim.\r\n", ch);
      return FALSE;
    }
  }
  else if(!(vict=get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  if(vict==ch) {
    send_to_char("That wouldn't be very bright.\r\n", ch);
    return FALSE;
  }
  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    demagnify(ch);
  }

  if(number(1, 100) <= GET_PR(vict))
    damage(ch, vict, 0, SKILL_MOLECULAR_AGITATION);
  else {
    mag_continuous(GET_LEVEL(ch), ch, vict, NULL, psi_power+PSI_START, SAVING_PARA);
    send_to_char(OK, ch);
  }
  return TRUE;
}

int psi_complete_healing(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }
  send_to_char("You slip into a deep trance.\r\n", ch);
  mag_continuous(GET_LEVEL(ch), ch, NULL, NULL, psi_power+PSI_START, SAVING_PARA);
  GET_STUN(ch)=3;
  WAIT_STATE(ch, 3*PULSE_VIOLENCE);
  GET_POS(ch)=POS_STUNNED;
  send_to_char(OK, ch);
  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    demagnify(ch);
  }
  return TRUE;
}

int psi_death_field(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int dam, fighting, ch_fighting;
  struct char_data *tch, *next;

  one_argument(argument, arg);
  if(((dam=atoi(arg)) <= 0) || (dam > (GET_HIT(ch)-1))) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("You must specify a number greater than 0 and less than your current hp.\r\n", ch);
    return FALSE;
  }

  if(roll==20) {
    send_to_char("You will away your life, but fail to do anything with it.\r\n", ch);
    damage(ch, ch, dam, TYPE_SILENT);
    return FALSE;
  }

  if(FIGHTING(ch))
    ch_fighting=1;
  else
    ch_fighting=0;
  send_to_char("You will away your life in a wave of negative energy.\r\n", ch);
  act("$n staggers as part of $s life force is destroyed.", TRUE, ch, NULL, NULL, TO_ROOM);
  if(roll==power_score) {
    send_to_char("That worked really well.\r\n", ch);
    damage(ch, ch, dam/2, TYPE_SILENT);
  }
  else
    damage(ch, ch, dam, TYPE_SILENT);
  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    dam*=2;
    demagnify(ch);
  }
  for(tch=world[ch->in_room].people; tch; tch=next) {
    next=tch->next_in_room;
    if(tch==ch)
      continue;
    if(CAN_KILL(ch, tch)) {
      if(FIGHTING(tch))
        fighting=1;
      else
        fighting=0;
      if(number(1, 100) > GET_PR(tch))
        damage(ch, tch, dam, SKILL_DEATH_FIELD);
      if(!fighting && (FIGHTING(tch)==ch)) {
        if(number(0, 4)) {
          stop_fighting(tch);
          if((!ch_fighting) && FIGHTING(ch))
            stop_fighting(ch);
          if(MEMORY(tch) && (MEMORY(tch)->id==GET_IDNUM(ch)))
            forget(tch, ch);
        }
        else
          ch_fighting=1;
      }
    }
  }
  return TRUE;
}

int psi_energy_containment(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_ENERGY_CONTAINMENT;
  af.bitvector = AFF_ENERGY_CONT;
  af.location = APPLY_NONE;
  af.duration = -1;
  af.modifier = 0;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_life_draining(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct char_data *vict;
  struct spcontinuous *s;

  one_argument(argument, arg);
  if(!*arg) {
    if(FIGHTING(ch))
      vict=FIGHTING(ch);
    else {
      if(roll==20) {
        send_to_char("You fail to activate the power.\r\n", ch);
        return FALSE;
      }
      send_to_char("You must specify a victim.\r\n", ch);
      return FALSE;
    }
  }
  else if(!(vict=get_char_room_vis(ch, arg))) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  if(vict==ch) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("That wouldn't be very bright.\r\n", ch);
    return FALSE;
  }
  if(!CAN_KILL(ch, vict)) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  for(s=ch->char_specials.spcont; s; s=s->next)
  {
    if((s->spspell==SKILL_LIFE_DRAINING) && (s->sptarget==(void *)vict)) {
      if(roll==20) {
        send_to_char("You fail to activate the power.\r\n", ch);
        return FALSE;
      }
      sprintf(buf, "You are already draining life from %s.\r\n", GET_NAME(vict));
      send_to_char(buf, ch);
      return FALSE;
    }
  }

  if(roll==20) {
    sprintf(buf, "You screw up and transfer health to %s!\r\n", GET_NAME(vict));
    if(GET_HIT(ch) > 1) {
      GET_HIT(vict)+=GET_HIT(ch)/2;
      GET_HIT(ch)/=2;
    }
    return FALSE;
  }

  if(number(1, 100) <= GET_PR(vict)) {
    sprintf(buf, "%s repells you.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
  }
  else {
    if(roll==power_score)
      send_to_char("That worked really well.\r\n", ch);
    mag_continuous(GET_LEVEL(ch), ch, vict, NULL, psi_power+PSI_START, (roll==power_score?1:0));
    send_to_char(OK, ch);
  }
  return TRUE;
}

int psi_metamorphosis(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  if(AFF_FLAGGED(ch, AFF_METAMORPHOSIS)) {
    send_to_char("You are already transformed.\r\n", ch);
    return FALSE;
  }
  af.type = SKILL_METAMORPHOSIS;
  af.bitvector = AFF_METAMORPHOSIS;
  af.location = APPLY_DAMROLL;
  af.duration = -1;

  one_argument(argument, arg);
  if(!*arg) {
    send_to_char("But what do you want to change in to?\r\n", ch);
    return FALSE;
  }
  if(!strn_cmp("bear", arg, strlen(arg))) {
    af.modifier=25;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.location = APPLY_HITROLL;
    af.modifier=5;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.location = APPLY_AC;
    af.modifier=-10;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  else if(!strn_cmp("cat", arg, strlen(arg))) {
    af.modifier=10;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.location = APPLY_AC;
    af.modifier=-5;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.bitvector = AFF_HASTE;
    af.location = APPLY_HITROLL;
    af.modifier=15;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  else if(!strn_cmp("reptile", arg, strlen(arg))) {
    af.modifier=5;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.location = APPLY_HITROLL;
    af.modifier=10;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.location = APPLY_AC;
    af.modifier=-20;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  else {
    send_to_char("You can't transform into that!\r\n", ch);
    return FALSE;
  }

  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_adrenalin_control(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int i;
  char *ptr, type='c';
  struct affected_type afc, afd, afs;

  if(roll==20) {
    send_to_char("You accidentaly shut off your body's supply of adrenalin!\r\n", ch);
    if(GET_HIT(ch) > 1)
      GET_HIT(ch) >>= 1;
    i=number(1, 8);
    GET_STUN(ch)=i;
    WAIT_STATE(ch, i*PULSE_VIOLENCE);
    GET_POS(ch)=POS_STUNNED;
    return FALSE;
  }

  afc.type = SKILL_ADRENALIN_CONTROL;
  afc.bitvector = 0;
  afc.location = APPLY_CON;
  afc.duration = -1;
  afc.modifier = 0;
  afd.type = SKILL_ADRENALIN_CONTROL;
  afd.bitvector = 0;
  afd.location = APPLY_DEX;
  afd.duration = -1;
  afd.modifier = 0;
  afs.type = SKILL_ADRENALIN_CONTROL;
  afs.bitvector = 0;
  afs.location = APPLY_STR;
  afs.duration = -1;
  afs.modifier = 0;

  i=number(1, 6);
  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    i*=2;
    demagnify(ch);
  }
  if(roll==power_score) {
    afc.modifier=i;
    afd.modifier=i;
    afs.modifier=i;
    send_to_char("That worked really well.", ch);
  }
  else {
    for(ptr=argument; (*ptr) && i; ptr++) {
      switch(*ptr) {
      case 'c':
      case 'C':
        type='c';
        afc.modifier++;
        i--;
        break;
      case 'd':
      case 'D':
        type='d';
        afd.modifier++;
        i--;
        break;
      case 's':
      case 'S':
        type='s';
        afs.modifier++;
        i--;
        break;
      }
    }
    for(; i; i--) {
      switch(type) {
      case 'c':
        afc.modifier++;
        break;
      case 'd':
        afd.modifier++;
        break;
      case 's':
        afs.modifier++;
        break;
      }
    }
  }

  if(afc.modifier)
    affect_join(ch, &afc, FALSE, FALSE, FALSE, FALSE);
  if(afd.modifier)
    affect_join(ch, &afd, FALSE, FALSE, FALSE, FALSE);
  if(afs.modifier)
    affect_join(ch, &afs, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_biofeedback(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You accidentaly cause internal bleeding!\r\n", ch);
    GET_HIT(ch) -= GET_HIT(ch)/10;
    return FALSE;
  }

  af.type = SKILL_BIOFEEDBACK;
  af.bitvector = 0;
  af.location = APPLY_AC;
  af.duration = -1;
  if(roll == power_score) {
    af.modifier = -30;
    send_to_char("That worked really well.", ch);
  }
  else
    af.modifier = -10;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_body_weaponry(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int i;
  struct obj_data *obj;

  if(roll==20) {
    send_to_char("You cause yourself excruciating pain!\r\n", ch);
    i=number(1, 10);
    GET_STUN(ch)=i;
    WAIT_STATE(ch, i*PULSE_VIOLENCE);
    GET_POS(ch)=POS_STUNNED;
    return FALSE;
  }

  if(GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You are already wielding a weapon.\r\n", ch);
    return FALSE;
  }
  if (!(obj = read_object(9, VIRTUAL))) {
    send_to_char("For some strange reason you can't do that.\r\n", ch);
    return FALSE;
  }
  if(roll==power_score) {
    send_to_char("That worked really well.\r\n", ch);
    obj->affected[0].modifier=6;
  }
  equip_char(ch, obj, WEAR_WIELD);
  mag_continuous(GET_LEVEL(ch), ch, ch, obj, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_cell_adjustment(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct char_data *vict;

  if(roll==20) {
    send_to_char("Oops, you slipped.\r\n", ch);
    damage(ch, ch, number(1, 50), TYPE_SILENT);
    return FALSE;
  }

  one_argument(argument, arg);
  if(!(vict=get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  send_to_char(OK, ch);
  if(roll==power_score) {
    send_to_char("That worked really well.\r\n", ch);
    GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict)+50);
  }
  else {
    GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict)+20);
  }
  affect_from_char(vict, SPELL_CURE_DISEASE);
  mag_continuous(GET_LEVEL(ch), ch, vict, NULL, psi_power+PSI_START, SAVING_PARA);
  return TRUE;
}

int psi_chameleon_power(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_CHAMELEON_POWER;
  af.bitvector = AFF_SNEAK;
  af.location = APPLY_NONE;
  af.duration = -1;
  af.modifier = 0;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_displacement(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_DISPLACEMENT;
  af.bitvector = 0;
  af.location = APPLY_AC;
  af.duration = -1;
  af.modifier = -20;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_flesh_armor(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int i;
  struct affected_type af;

  if(GET_CLASS_BITVECTOR(ch) & MK_F) {
    send_to_char("Your flesh is already like armor.\r\n", ch);
    return FALSE;
  }

  if(roll==20) {
    af.type = SKILL_FLESH_ARMOR;
    af.bitvector = 0;
    af.location = APPLY_CHA;
    af.duration = 24;
    af.modifier = -2;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    send_to_char("You grow snarly hair all over your body!\r\n", ch);
    return FALSE;
  }

  switch(roll) {
  case 1:
    i=-5;
    break;
  case 2:
    i=-10;
    break;
  case 3:
  case 4:
    i=-15;
    break;
  case 5:
  case 6:
    i=-20;
    break;
  case 7:
  case 8:
    i=-25;
    break;
  case 9:
  case 10:
    i=-30;
    break;
  default:
    i=-35;
    break;
  }
  if(roll==power_score)
    i-=5;

  af.type = SKILL_FLESH_ARMOR;
  af.bitvector = 0;
  af.location = APPLY_AC;
  af.duration = -1;
  af.modifier = i;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(i, ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_graft_weapon(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct obj_data *obj;
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  if(!(obj=GET_EQ(ch, WEAR_WIELD))) {
    send_to_char("You have to be wielding a weapon to graft it to you hand.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_GRAFT_WEAPON;
  af.bitvector = 0;
  af.location = APPLY_DAMROLL;
  af.duration = -1;
  af.modifier = 2;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  af.location = APPLY_HITROLL;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, obj, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_lend_health(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int i;
  struct char_data *vict;

  skip_spaces(&argument);
  argument=one_argument(argument, arg);
  one_argument(argument, buf1);
  if(!*arg) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("If you want to lend health, you have to specify a target.\r\n", ch);
    return FALSE;
  }
  if((!is_number(buf1)) || ((i=atoi(buf1)) < 1)) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("You must specify a positive number of hitpoints to give.\r\n", ch);
    return FALSE;
  }

  if(!(vict=get_char_room_vis(ch, arg))) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char(NOPERSON, ch);
    return FALSE;
  }
  if(vict==ch) {
    if(roll==20) {
      send_to_char("You fail to activate the power.\r\n", ch);
      return FALSE;
    }
    send_to_char("What would be the point of that?\r\n", ch);
    return FALSE;
  }

  i=MIN(i, GET_HIT(ch)-1);
  i=MIN(MAX(GET_MAX_HIT(vict)-GET_HIT(vict), 0), i);
  if(roll==20) {
    sprintf(buf, "You screw up and take on %s's wounds!\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    if(GET_HIT(ch) > 0)
      GET_HIT(ch) -= MAX(GET_MAX_HIT(vict)-GET_HIT(vict), 0);
    return FALSE;
  }

  sprintf(buf, "You transfer your health to %s.\r\n", GET_NAME(vict));
  send_to_char(buf, ch);
  sprintf(buf, "%s transfers some health to you!\r\n", GET_NAME(ch));
  send_to_char(buf, vict);
  GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict)+i);
  if(roll == power_score) {
    send_to_char("That worked really well.", ch);
    i /= 2;
  }
  GET_HIT(ch) -= i;
  return TRUE;
}

int psi_probability_travel(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int mem_num, room;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  skip_spaces(&argument);
  mem_num=atoi(argument);

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
    room=real_room(GET_REMEMBER(ch, mem_num-1));
  }
  else {
    send_to_char("You don't have that memory number available.\r\n", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(ch->in_room, ROOM_NOTELEPORT)) {
    send_to_char("A mysterious force prevents you from leaving.\n\r", ch);
      return FALSE;
  }
  if(ROOM_FLAGGED(room, ROOM_NORELOCATE)) {
    send_to_char("For some unknown reason, you fail.\n\r", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(room, ROOM_GODROOM | ROOM_PRIVATE) || (world[room].zone == 0) ||
     (zone_table[world[room].zone].number == GOD_ZONE)) {
    send_to_char("Hmmmm, your target seems to be a private room.\n\r", ch);
    return FALSE;
  }

  act("$n vanishes.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, room);
  act("$n appears out of thin air.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);

  return TRUE;
}

int psi_dimension_door(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int mem_num, room;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  skip_spaces(&argument);
  mem_num=atoi(argument);

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
    room=real_room(GET_REMEMBER(ch, mem_num-1));
  }
  else {
    send_to_char("You don't have that memory number available.\r\n", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(ch->in_room, ROOM_NOTELEPORT) || ROOM_FLAGGED(ch->in_room, ROOM_NORELOCATE)) {
    send_to_char("A mysterious force prevents your dimensional link.\n\r", ch);
    return FALSE;
  }
  if(ROOM_FLAGGED(room, ROOM_NOTELEPORT) || ROOM_FLAGGED(room, ROOM_NORELOCATE)) {
    send_to_char("A mysterious force prevents your dimensional link.\n\r", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(room, ROOM_GODROOM | ROOM_PRIVATE) || (world[room].zone == 0) ||
     (zone_table[world[room].zone].number == GOD_ZONE)) {
    send_to_char("Hmmmm, your target seems to be a private room.\n\r", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(ch->in_room, ROOM_DIM_DOOR) || ROOM_FLAGGED(room, ROOM_DIM_DOOR)) {
    send_to_char("Another dimensional rift prevents your power.\r\n", ch);
    return FALSE;
  }

  SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_DIM_DOOR);
  SET_BIT(ROOM_FLAGS(room), ROOM_DIM_DOOR);
  send_to_room("A black portal opens.\r\n", ch->in_room);
  send_to_room("A black portal opens.\r\n", room);
  mag_continuous(ch->in_room, ch, NULL, NULL, psi_power+PSI_START, room);
  return TRUE;
}

int psi_domination(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int charmed_levels=0, num_charmies=0;
  struct char_data *vict;
  struct affected_type af;
  struct follow_type *f;
  extern int max_charmies;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  one_argument(argument, buf);
  if(!(vict=get_char_room_vis(ch, buf))) {
    send_to_char("You don't see that person here.\r\n", ch);
    return FALSE;
  }

  if(!IS_NPC(vict)) {
    send_to_char("You can't dominate a player.\r\n", ch);
    return FALSE;
  }

  if(vict==ch) {
    send_to_char("You can't dominate yourself!\r\n", ch);
    return FALSE;
  }

  if(AFF_FLAGGED(ch, AFF_CHARM)) {
    send_to_char("You can't have any followers of your own.\r\n", ch);
    return FALSE;
  }

  if(AFF_FLAGGED(vict, AFF_SANCTUARY)||MOB_FLAGGED(vict, MOB_NOCHARM)) {
    send_to_char("You fail.\r\n", ch);
    return FALSE;
  }

  if(GET_MOB_SPEC(vict) == shop_keeper) {
    send_to_char("You fail.\r\n", ch);
    return FALSE;
  }

  if(AFF_FLAGGED(vict, AFF_CHARM) || ((GET_LEVEL(ch)>>1)<GET_LEVEL(vict))) {
    send_to_char("You fail.\r\n", ch);
    return FALSE;
  }

  if(circle_follow(vict, ch)) {
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
    return FALSE;
  }

  for(f = ch->followers; f; f = f->next) {
    if(IS_AFFECTED(f->follower, AFF_CHARM)) {
      charmed_levels += GET_LEVEL(f->follower);
      num_charmies++;
    }
  }
  if(((charmed_levels+GET_LEVEL(vict)) > GET_LEVEL(ch)) || (num_charmies >= max_charmies)) {
    send_to_char("You don't have the power to control that victim.\r\n", ch);
    return FALSE;
  }

  if(number(1, 100) <= GET_PR(vict)) {
    sprintf(buf, "%s repells you.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
  }
  else {
    if (vict->master)
      stop_follower(vict);
    add_follower(vict, ch);
    af.type=SKILL_DOMINATION;
    af.duration=-1;
    af.modifier=0;
    af.location=0;
    af.bitvector = AFF_CHARM;
    affect_to_char(vict, &af);
    act("You are compelled to obey $n.", FALSE, ch, NULL, vict, TO_VICT);
    act("You dominate $N.", FALSE, ch, NULL, vict, TO_CHAR);
    if (IS_NPC(vict)) {
      REMOVE_BIT(MOB_FLAGS(vict), MOB_AGGRESSIVE);
      REMOVE_BIT(MOB_FLAGS(vict), MOB_SPEC);
    }
    mag_continuous(GET_LEVEL(ch), ch, vict, NULL, psi_power+PSI_START, SAVING_PARA);
  }

  return TRUE;
}

int psi_psionic_blast(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int i, dam;
  struct char_data *vict;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  one_argument(argument, arg);
  if(!*arg) {
    if(FIGHTING(ch))
      vict=FIGHTING(ch);
    else {
      send_to_char("You must specify a victim.\r\n", ch);
      return FALSE;
    }
  }
  else if(!(vict=get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  if(vict==ch) {
    send_to_char("That wouldn't be very bright.\r\n", ch);
    return FALSE;
  }
  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  if(number(1, 100) <= GET_PR(vict))
    damage(ch, vict, 0, SKILL_PSIONIC_BLAST);
  else {
    if((!mag_savingthrow(vict, SAVING_PARA)) || (roll==power_score)) {
      i=number(1, 2);
      GET_STUN(vict)=i;
      WAIT_STATE(vict, i*PULSE_VIOLENCE);
      GET_POS(vict)=POS_STUNNED;
    }
    dam=dice(GET_LEVEL(ch)/5, 12);
    if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
      dam*=2;
      demagnify(ch);
    }
    damage(ch, vict, dam, SKILL_PSIONIC_BLAST);
  }
  return TRUE;
}

int psi_ego_whip(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int dam;
  struct char_data *vict;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  one_argument(argument, arg);
  if(!*arg) {
    if(FIGHTING(ch))
      vict=FIGHTING(ch);
    else {
      send_to_char("You must specify a victim.\r\n", ch);
      return FALSE;
    }
  }
  else if(!(vict=get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  if(vict==ch) {
    send_to_char("That wouldn't be very bright.\r\n", ch);
    return FALSE;
  }
  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  if(number(1, 100) <= GET_PR(vict))
    damage(ch, vict, 0, SKILL_EGO_WHIP);
  else {
    if(!mag_savingthrow(vict, SAVING_PARA))
      WAIT_STATE(vict, PULSE_VIOLENCE*number(1, 2));
    dam=dice(2, 4)+GET_LEVEL(ch)/5;
    if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
      dam*=2;
      demagnify(ch);
    }
    damage(ch, vict, dam, SKILL_EGO_WHIP);
  }
  return TRUE;
}

int psi_life_detection(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_LIFE_DETECTION;
  af.bitvector = AFF_SENSE_LIFE;
  af.location = APPLY_NONE;
  af.duration = -1;
  af.modifier = 0;
  affect_to_char(ch, &af);
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char(ch, &af);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_psychic_crush(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  int dam;
  struct char_data *vict;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  one_argument(argument, arg);
  if(!*arg) {
    if(FIGHTING(ch))
      vict=FIGHTING(ch);
    else {
      send_to_char("You must specify a victim.\r\n", ch);
      return FALSE;
    }
  }
  else if(!(vict=get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return FALSE;
  }

  if(vict==ch) {
    send_to_char("That wouldn't be very bright.\r\n", ch);
    return FALSE;
  }
  if(!CAN_KILL(ch, vict)) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return FALSE;
  }

  if(number(1, 100) <= GET_PR(vict))
    dam=0;
  else
    dam=dice(GET_LEVEL(ch)/5, 8);
  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    dam*=2;
    demagnify(ch);
  }
  damage(ch, vict, dam, SKILL_PSYCHIC_CRUSH);
  return TRUE;
}

int psi_split_personality(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_SPLIT_PERSONALITY;
  af.bitvector = AFF_SPLIT_PERSONALITY;
  af.location = APPLY_NONE;
  af.duration = -1;
  af.modifier = 0;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_cannibalize(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(GET_HIT(ch) <= 100) {
    if(roll==20)
      send_to_char("You fail to activate the power.\r\n", ch);
    else
      send_to_char("You must have more than 100 hp to use cannibalize.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_CANNIBALIZE;
  af.bitvector = 0;
  af.location = APPLY_HIT;
  af.duration = 24;

  if(roll==20) {
    send_to_char("You lose control as you try to absorb mana.\r\n", ch);
    af.modifier = -50;
  }
  else if(roll==power_score) {
    send_to_char("That worked really well.\r\n", ch);
    GET_HIT(ch)+=50;
    af.modifier = -50;
  }
  else {
    af.modifier = -100;
  }

  affect_to_char(ch, &af);
  if(roll==20)
    return FALSE;
  else
    GET_MANA(ch)+=50;
  send_to_char(OK, ch);
  return TRUE;
}

int psi_magnify(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  struct affected_type af;

  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
    send_to_char("You are already magnifying powers.\r\n", ch);
    return FALSE;
  }

  af.type = SKILL_MAGNIFY;
  af.bitvector = AFF_MAGNIFY;
  af.location = APPLY_NONE;
  af.duration = -1;
  af.modifier = 0;
  affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_char(OK, ch);
  return TRUE;
}

int psi_stasis_field(struct char_data *ch, char *argument, int psi_power, int power_score, int roll)
{
  if(roll==20) {
    send_to_char("You fail to activate the power.\r\n", ch);
    return FALSE;
  }

  if(ROOM_FLAGGED(ch->in_room, ROOM_STASIS)) {
    send_to_char("There is already a stasis field here.\r\n", ch);
    return FALSE;
  }
  SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_STASIS);
  mag_continuous(GET_LEVEL(ch), ch, ch, NULL, psi_power+PSI_START, SAVING_PARA);
  send_to_room("Time slows to a crawl.\r\n", ch->in_room);
  send_to_char(OK, ch);
  return TRUE;
}
