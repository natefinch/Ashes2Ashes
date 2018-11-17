/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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
#include "class.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;

extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;

void clearMemory(struct char_data * ch);
void act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);

void damage(struct char_data * ch, struct char_data * victim,
	         int damage, int weapontype);

void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int dice(int number, int size);
extern struct spell_info_type spell_info[];


struct char_data *read_mobile(int, int);


void check_drawn_in(struct char_data *ch, struct char_data *victim)
{
  if(ch && victim && (ch!=victim) && (!FIGHTING(ch)) && FIGHTING(victim)) {
    if(GET_DEX(ch) < number(1, 20)) {
      send_to_char("You are unable to escape the fight!\r\n", ch);
      act("$N assists you!", FALSE, victim, NULL, ch, TO_CHAR);
      act("$n is unable to escape the fight.", FALSE, ch, NULL, victim, TO_ROOM);
      hit(ch, FIGHTING(victim), TYPE_UNDEFINED);
    }
  }
}


int mag_savingthrow(struct char_data * ch, int type)
{
  int save;

  /* negative apply_saving_throw values make saving throws better! */

  save = class_get_save(ch, type);

  save += GET_SAVE(ch, type);

  /* throwing a 0 is always a failure */
  if (MAX(1, save) < number(0, 99))
    return TRUE;
  else
    return FALSE;
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  static struct affected_type *af, *next;
  struct affected_type disease_aff;
  static struct char_data *i, *tch;
  extern char *spell_wear_off_msg[];
  void check_fall(struct char_data *ch, struct obj_data *obj);

  for (i = character_list; i; i = i->next) {
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1) {
	af->duration--;
        if(af->type == SPELL_CURE_DISEASE) {
          if(af->location == APPLY_AC) {
            if(af->duration < 16) {
              if((af->duration % 2) && (af->modifier > 1)) {
                disease_aff.type = SPELL_CURE_DISEASE;
                disease_aff.bitvector = AFF_DISEASE;
                disease_aff.duration = af->duration;
                disease_aff.modifier = -1;
                disease_aff.location = APPLY_AC;
                affect_join(i, &disease_aff, TRUE, TRUE, TRUE, FALSE);
              }
            }
            else {
              disease_aff.type = SPELL_CURE_DISEASE;
              disease_aff.bitvector = AFF_DISEASE;
              disease_aff.duration = af->duration;
              disease_aff.modifier = 1;
              disease_aff.location = APPLY_AC;
              affect_join(i, &disease_aff, TRUE, TRUE, TRUE, FALSE);
            }
            send_to_char("You are wracked by a fit of coughing.\r\n", i);
            damage(i, i, dice(af->modifier, 4)+7, TYPE_SILENT);
          }
          else {
            if(af->duration < 16) {
              if((af->duration % 2) && (af->modifier < -1)) {
                disease_aff.type = SPELL_CURE_DISEASE;
                disease_aff.bitvector = AFF_DISEASE;
                disease_aff.duration = af->duration;
                disease_aff.modifier = 1;
                disease_aff.location = APPLY_HITROLL;
                affect_join(i, &disease_aff, TRUE, TRUE, TRUE, FALSE);
              }
            }
            else {
              disease_aff.type = SPELL_CURE_DISEASE;
              disease_aff.bitvector = AFF_DISEASE;
              disease_aff.duration = af->duration;
              disease_aff.modifier = -1;
              disease_aff.location = APPLY_HITROLL;
              affect_join(i, &disease_aff, TRUE, TRUE, TRUE, FALSE);
            }
          }
        }
      }
      else if (af->duration <= -1)	/* No action */
	af->duration = -1;	/* GODs only! unlimited */
      else {
	if ((af->type > 0) && (af->type <= MAX_SPELLS))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\r\n", i);
	    }
        switch(af->type) {
        case SPELL_MAGIC_LIGHT:
          if(i->in_room != NOWHERE)
            world[i->in_room].light--;
          break;
        case SPELL_SLEEP:
          if(IS_NPC(i)) {
            GET_POS(i)=GET_DEFAULT_POS(i);
            update_pos(i);
          }
          break;
        case SKILL_CLAIRVOYANCE:
          send_to_char("Your vision starts working again.\r\n", i);
          break;
        }
	affect_remove(i, af);
      }
    }
    /* Spread disease */
    if(AFF_FLAGGED(i, AFF_DISEASE)) {
      if(i->in_room != NOWHERE) {
        for(tch=world[i->in_room].people; tch; tch=tch->next_in_room) {
          int prob, found=0;

          if(tch==i)
            continue;
          if(IS_NPC(tch))
            continue;
          if(!found) {
            found=1;
            act("$n doubles over in a coughing fit.", FALSE, i, NULL, NULL, TO_ROOM);
          }
          if(AFF_FLAGGED(tch, AFF_DISEASE))
            continue;
          if(GET_LEVEL(tch) < 20)
            continue;
          if(ROOM_FLAGGED(i->in_room, ROOM_PEACEFUL))
            continue;
          if((GET_LEVEL(tch) >= LVL_HERO) && (!PRF_FLAGGED(tch, PRF_AVTR)))
            continue;
          if(GET_CLASS_BITVECTOR(tch) & (CL_F | PA_F))
            prob=25;
          else
            prob=65;
          if(number(0, 99) < prob) {
            disease_aff.type = SPELL_CURE_DISEASE;
            disease_aff.bitvector = AFF_DISEASE;
            disease_aff.duration = 20+number(1, 10);
            disease_aff.modifier = -1;
            disease_aff.location = APPLY_HITROLL;
            affect_join(tch, &disease_aff, TRUE, TRUE, TRUE, FALSE);
            disease_aff.modifier = 1;
            disease_aff.location = APPLY_AC;
            affect_join(tch, &disease_aff, TRUE, TRUE, TRUE, FALSE);
            send_to_char("You feel feverish.\r\n", tch);
            act("$n looks sickly.", TRUE, tch, NULL, NULL, TO_ROOM);
          }
        }
      }
    }
    check_fall(i, NULL);
  }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (number(0, 2)) {
      case 0:
	send_to_char("A wart sprouts on your nose.\r\n", ch);
	break;
      case 1:
	send_to_char("Your hair falls out in clumps.\r\n", ch);
	break;
      case 2:
	send_to_char("A huge corn develops on your big toe.\r\n", ch);
	break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0) {
      obj_from_char(obj0);
      extract_obj(obj0);
    }
    if (item1 < 0) {
      obj_from_char(obj1);
      extract_obj(obj1);
    }
    if (item2 < 0) {
      obj_from_char(obj2);
      extract_obj(obj2);
    }
  }
  if (verbose) {
    send_to_char("A puff of smoke rises from your pack.\r\n", ch);
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  return (TRUE);
}




/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 */

void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int dam = 0;

  if (victim == NULL || ch == NULL)
    return;

  switch (spellnum) {
    /* Mostly mages */
  case SPELL_MAGIC_MISSILE:
    dam = dice(MIN(level, 25), 4) + MIN(level, 25);
    break;
  case SPELL_BURNING_HANDS:
    dam = dice((level+1)>>1, 4) + ((level+1)>>1);
    break;
  case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
    dam = dice((level+1)>>1, 4) + 25;
    break;
  case SPELL_SHOCKING_GRASP:
    dam = dice((level+1)/2, 6) + 25;
    break;
  case SPELL_CONE_OF_COLD:
    if((IS_NPC(ch)) || (level!=GET_LEVEL(ch)) || (savetype!=SAVING_SPELL))
      dam = dice(level, 6);
    else
      dam = dice(level-get_skill_level(ch, spellnum)+1, 6);
    break;
  case SPELL_LIGHTNING_BOLT:
    if((IS_NPC(ch)) || (level!=GET_LEVEL(ch)) || (savetype!=SAVING_SPELL))
      dam = dice(level, 8);
    else
      dam = dice(level-get_skill_level(ch, spellnum)+1, 8);
    break;
  case SPELL_FIREBALL:
    if((IS_NPC(ch)) || (level!=GET_LEVEL(ch)) || (savetype!=SAVING_SPELL))
      dam = dice(level, 8) + level;
    else
      dam = dice(level-get_skill_level(ch, spellnum)+1, 8) + level-get_skill_level(ch, spellnum)+1;
    break;
  case SPELL_COLOR_SPRAY:	/* also has an effect */
    dam = dice(level, 3);
    break;
  case SPELL_ICE_STORM:
    dam = dice(5, 8) + level/2;
    break;
    /* Mostly clerics */
  case SPELL_DISPEL_EVIL:
    if (IS_EVIL(ch)) {
      victim = ch;
    } else if (IS_GOOD(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return;
    }
    dam = dice(level >> 1, 4);
    if(GET_LEVEL(victim) <= level)
      dam = MAX(100, dam);
    break;
  case SPELL_DISPEL_GOOD:
    if (IS_GOOD(ch)) {
      victim = ch;
    } else if (IS_EVIL(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return;
    }
    dam = dice(level >> 1, 4);
    if(GET_LEVEL(victim) <= level)
      dam = MAX(100, dam);
    break;
  case SPELL_CALL_LIGHTNING:
    if(OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
      dam = MIN(dice((level >> 1), 8), 120);
    }
    else {
      dam = 0;
      send_to_char("Nothing happens.\r\n", ch);
      return;
    }
    break;
  case SPELL_FLAMESTRIKE:
    dam = MIN(dice((level >> 1), 6), 180);
    break;
  case SPELL_HARM:
    dam = MIN(MAX(GET_HIT(victim)-dice(1,4), 0), 100);
    break;
  case SPELL_ENERGY_DRAIN:
    if (GET_LEVEL(victim) <= 2)
      dam = 100;
    else
      dam = dice(1, 10);
    break;
  case SPELL_WORD_OF_DEATH:
    dam = dice(34, 30) + level;
    break;

    /* Area spells */
  case SPELL_EARTHQUAKE:
    dam = dice(1, 8) + (level >> 1);
    break;
  case SPELL_IMPLOSION:
    dam = 3*(dice(1, 8) + (level >> 1));
    break;

    /* NPC spells */
  case GAS_BREATH:
    dam = MIN(dice(level, 10) + 5*level, 500);
    break;
  case FIRE_BREATH:
    dam = MIN(dice(level, 10) + 5*level, 500);
    break;
  case FROST_BREATH:
    dam = MIN(dice(level, 10) + 5*level, 500);
    break;
  case LIGHTNING_BREATH:
    dam = MIN(dice(level, 10) + 5*level, 500);
    break;
  case ACID_BREATH:
    dam = MIN(dice(level, 10) + 5*level, 500);
    break;
  } /* switch(spellnum) */


  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype))
    dam >>= 1;

  /* and finally, inflict the damage */
  damage(ch, victim, dam, spellnum);
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

#define MAX_SPELL_AFFECTS 5	/* change if more needed */

void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		      int spellnum, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  int is_mage = FALSE, is_cleric = FALSE;
  bool accum_affect = FALSE, accum_duration = FALSE;
  bool avg_affect = FALSE, avg_duration = FALSE, affect_anyway = FALSE;
  char *to_vict = NULL, *to_room = NULL;
  int i;


  if (victim == NULL || ch == NULL)
    return;

  is_mage = (GET_CLASS(ch) == CLASS_MAGIC_USER);
  is_cleric = (GET_CLASS(ch) == CLASS_CLERIC);

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    af[i].type = spellnum;
    af[i].bitvector = 0;
    af[i].modifier = 0;
    af[i].location = APPLY_NONE;
  }

  switch (spellnum) {

  case SPELL_CHILL_TOUCH:
    if (!mag_savingthrow(victim, savetype)) {
      af[0].location = APPLY_STR;
      af[0].duration = 6;
      af[0].modifier = -1;
      accum_affect = TRUE;
      to_vict = "You feel your strength wither!";
    }
    break;

  case SPELL_ARMOR:
    af[0].location = APPLY_AC;
    af[0].modifier = -(MAX(20, level/2));
    af[0].duration = 24;
    affect_anyway = TRUE;
    to_vict = "You feel someone protecting you.";
    break;

  case SPELL_BLESS:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 1;
    af[0].duration = 6;

    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -1;
    af[1].duration = 6;

    accum_duration = TRUE;
    to_vict = "You feel righteous.";
    break;

  case SPELL_COLOR_SPRAY:
  case SPELL_BLINDNESS:
    if (MOB_FLAGGED(victim,MOB_NOBLIND) || mag_savingthrow(victim, savetype)) {
      if(spellnum==SPELL_BLINDNESS)
        send_to_char("You fail.\r\n", ch);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4;
    af[0].duration = level/20;
    af[0].bitvector = AFF_BLIND;

    af[1].location = APPLY_AC;
    af[1].modifier = 40;
    af[1].duration = level/20;
    af[1].bitvector = AFF_BLIND;

    to_room = "$n seems to be blinded!";
    to_vict = "You have been blinded!";
    break;

  case SPELL_CURSE:
    if (MOB_FLAGGED(victim,MOB_NOCURSE) || mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].duration = (level >> 1);
    af[0].modifier = -(level/20);
    af[0].bitvector = AFF_CURSE;

    af[1].location = APPLY_SAVING_PARA;
    af[1].duration = (level >> 1);
    af[1].modifier = level/20;
    af[1].bitvector = AFF_CURSE;

    to_room = "$n briefly glows red!";
    to_vict = "You feel very uncomfortable.";
    break;

  case SPELL_DETECT_ALIGN:
    af[0].duration = (2.5*level);
    af[0].bitvector = AFF_DETECT_ALIGN;
    accum_duration = TRUE;
    avg_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_INVIS:
    af[0].duration = (2.5*level);
    af[0].bitvector = AFF_DETECT_INVIS;
    accum_duration = TRUE;
    avg_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_MAGIC:
    af[0].duration = (2.5*level);
    af[0].bitvector = AFF_DETECT_MAGIC;
    accum_duration = TRUE;
    avg_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_INFRAVISION:
    af[0].duration = (level >> 2);
    af[0].bitvector = AFF_INFRAVISION;
    accum_duration = TRUE;
    avg_duration = TRUE;
    to_vict = "Your eyes glow red.";
    to_room = "$n's eyes glow red.";
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;

    af[0].duration = 24;
    af[0].modifier = -40;
    af[0].location = APPLY_AC;
    af[0].bitvector = AFF_INVISIBLE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;

  case SPELL_POISON:
    if (MOB_FLAGGED(victim,MOB_NOPOISON) || mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    af[0].location = APPLY_STR;
    af[0].duration = level;
    af[0].modifier = -2;
    af[0].bitvector = AFF_POISON;
    accum_duration = TRUE;
    accum_affect = TRUE;
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;

  case SPELL_PROT_FROM_EVIL:
    af[0].duration = 24;
    af[0].bitvector = AFF_PROTECT_EVIL;
    to_vict = "You have a righteous feeling!";
    break;

  case SPELL_PROT_FROM_GOOD:
    af[0].duration = 24;
    af[0].bitvector = AFF_PROTECT_GOOD;
    to_vict = "You have an evil feeling!";
    break;

  case SPELL_DIVINE_PROTECTION:
    af[0].duration = 3;
    af[0].bitvector = AFF_DIVINE_PROT;
    to_vict = "You feel totally protected!";
    break;

  case SPELL_SANCTUARY:
    af[0].duration = ((level < LVL_HERO) ? 4 : level);
    af[0].bitvector = AFF_SANCTUARY;

    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    check_drawn_in(ch, victim);
    break;

  case SPELL_SLEEP:
    if (MOB_FLAGGED(victim, MOB_NOSLEEP)) {
      act("'Hey, I did not appreciate that!' - exclaims $n.", FALSE, victim, 0, 0, TO_ROOM);
      return;
    }
    if (mag_savingthrow(victim, savetype))
      return;

    af[0].duration = 4 + (level >> 2);
    af[0].bitvector = AFF_SLEEP;

    accum_affect = TRUE;

    if (GET_POS(victim) > POS_SLEEPING) {
      act("You feel very sleepy...  Zzzz......", FALSE, victim, 0, 0, TO_CHAR);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    af[0].location = APPLY_STR;
    af[0].duration = (level >> 1);
    af[0].modifier = (level+60)/50;
    accum_duration = TRUE;
    avg_duration = TRUE;
    accum_affect = TRUE;
    to_vict = "You feel stronger!";
    break;

  case SPELL_SENSE_LIFE:
    to_vict = "Your feel your awareness improve.";
    af[0].duration = (2.5*level);
    af[0].bitvector = AFF_SENSE_LIFE;
    break;

  case SPELL_WATERWALK:
    af[0].duration = level;
    af[0].bitvector = AFF_WATERWALK;
    accum_duration = TRUE;
    avg_duration = TRUE;
    to_vict = "You feel webbing between your toes.";
    break;

  case SPELL_HOLY_WORD:
    af[0].duration = 3;
    af[0].bitvector = AFF_UN_HOLY;
    to_vict = "You look very holy.";
    to_room = "$n looks very holy.";
    break;

  case SPELL_UNHOLY_WORD:
    af[0].duration = 3;
    af[0].bitvector = AFF_UN_HOLY;
    to_vict = "You look very unholy.";
    to_room = "$n looks very unholy.";
    break;

  case SPELL_HASTE:
    if ((!IS_NPC(victim)) && (GET_CLASS_BITVECTOR(victim) & MK_F)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    af[0].duration = 3;
    af[0].location = APPLY_DEX;
    af[0].modifier = 2;
    af[0].bitvector = AFF_HASTE;
    to_vict = "You start moving much faster.";
    to_room = "$n starts moving much faster.";
    break;

  case SPELL_SLOW:
    if (MOB_FLAGGED(victim, MOB_NOSLOW)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if ((!IS_NPC(victim)) && (GET_CLASS_BITVECTOR(victim) & MK_F)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(number(1, 4)==1) {
      if(IS_NPC(victim)) {
        act("\"Hey, I did not appreciate that!\" - exclaims $n!", FALSE, victim, 0, 0, TO_ROOM);
        hit(victim, ch, TYPE_UNDEFINED);
      }
      return;
    }
    af[0].duration = 3;
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4;
    af[0].bitvector = AFF_SLOW;
    to_vict = "You suddenly start to move slower...";
    to_room = "$n suddenly seems very sluggish.";
    break;

  case SPELL_AID:
    af[0].duration = 3;
    af[0].location = APPLY_HIT;
    af[0].modifier = MAX(20, level >> 1);

    af[1].duration = 3;
    af[1].location = APPLY_HITROLL;
    af[1].modifier = 1;

    accum_affect = TRUE;
    avg_affect = TRUE;
    to_vict = "You feel aided for battle.";
    break;

  case SPELL_SPELL_SHIELD:
    af[0].duration = ((level < LVL_HERO) ? 6 : level);
    af[0].bitvector = AFF_SPELLSHIELD;

    accum_affect = TRUE;
    avg_affect = TRUE;
    to_vict = "You feel resistant to magic attacks.";
    break;

  case SPELL_MAGIC_SHIELD:
    af[0].duration = ((level < LVL_HERO) ? 6 : level);
    af[0].bitvector = AFF_MAGICSHIELD;

    accum_affect = TRUE;
    avg_affect = TRUE;
    to_vict = "You feel resistant to magic attacks.";
    break;

  case SPELL_FLY:
    af[0].duration = 2*level;
    af[0].bitvector = AFF_FLY;

    accum_affect = TRUE;
    avg_affect = TRUE;
    to_vict = "Your feet leave the ground.";
    break;

  case SPELL_MAGIC_LIGHT:
    af[0].duration = level;
    af[0].bitvector = AFF_MAGIC_LIGHT;
    world[victim->in_room].light++;
    to_vict = "A magic flame appears between your hands and floats above your head.";
    to_room = "A magic flame appears between $n's hands and floats above $s head.";
    break;

  case SPELL_LOWER_RESISTANCE:
    if (!GET_MR(victim)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    af[0].duration = level/20;
    af[0].bitvector = AFF_LOWER_MR;
    to_vict = "You become more vulnerable to spells!";
    to_room = "$n winces and appears more vulnerable.";
    break;
  }


  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect||affect_anyway)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || (af[i].location != APPLY_NONE))
      affect_join(victim, af+i, accum_duration, avg_duration, accum_affect, avg_affect);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */

void perform_mag_groups(int level, struct char_data * ch,
			struct char_data * tch, int spellnum, int savetype)
{
  switch (spellnum) {
  case SPELL_GROUP_HEAL:
    mag_points(level, ch, tch, SPELL_HEAL, savetype);
    spell_heal(level, ch, tch, NULL);
    break;
  case SPELL_GROUP_POWER_HEAL:
    mag_points(level, ch, tch, SPELL_POWER_HEAL, savetype);
    spell_heal(level, ch, tch, NULL);
    break;
  case SPELL_GROUP_ARMOR:
    mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
    break;
  case SPELL_GROUP_RECALL:
    spell_recall(level, ch, tch, NULL);
    break;
  case SPELL_GROUP_INFRAVISION:
    mag_affects(level, ch, tch, SPELL_INFRAVISION, savetype);
    break;
  case SPELL_GROUP_SANCTUARY:
    mag_affects(level, ch, tch, SPELL_SANCTUARY, savetype);
    break;
  case SPELL_GROUP_REJUVENATE:
    mag_points(level, ch, tch, SPELL_REJUVENATE, savetype);
    break;
  case SPELL_GROUP_INVISIBLE:
    mag_affects(level, ch, tch, SPELL_INVISIBLE, savetype);
  }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!IS_AFFECTED(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!IS_AFFECTED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }

  if ((k != ch) && IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
    perform_mag_groups(level, ch, k, spellnum, savetype);
  }
  perform_mag_groups(level, ch, ch, spellnum, savetype);
}


/*
 * mass spells affect every creature in the room except the caster and immortals
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_HERO)
      continue;
    if (MOB_FLAGGED(tch, MOB_DOORSTOP))
      continue;

    switch (spellnum) {
    }
  }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void mag_areas(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  char *to_char = NULL;
  char *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum) {
  case SPELL_EARTHQUAKE:
    to_char = "You gesture and the earth begins to shake all around you!";
    to_room ="$n gracefully gestures and the earth begins to shake violently!";
    break;
  case SPELL_ICE_STORM:
    to_char = "You make huge blocks of ice drop on everyone in the room!";
    to_room = "$n makes huge blocks of ice fall from above!";
    break;
  case SPELL_IMPLOSION:
    to_char = "You detonate dynamite!";
    to_room = "$n detonates some dynamite!";
    break;
  case GAS_BREATH:
    to_char = "You exhail a cloud of gas!";
    to_room = "$n breathes gas!";
    break;
  }

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);
  

  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     * players can only hit players in CRIMEOK rooms 4) players can only hit
     * charmed mobs in CRIMEOK rooms
     */

    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_HERO)
      continue;
    if (!CAN_KILL(ch, tch))
      continue;
/* make it only skip charmies you control
    if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
*/
    if ((tch->master==ch) && IS_AFFECTED(tch, AFF_CHARM))
      continue;
    if (MOB_FLAGGED(tch, MOB_DOORSTOP))
      continue;

    mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
  }
}


/*
 *  Every spell which summons/gates/conjours a mob comes through here.
 *
 *  None of these spells are currently implemented in Circle 3.0; these
 *  were taken as examples from the JediMUD code.  Summons can be used
 *  for spells like clone, ariel servant, etc.
 */

static char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!\r\n",
  "$n animates a corpse!\r\n",
  "$N appears from a cloud of thick blue smoke!\r\n",
  "$N appears from a cloud of thick green smoke!\r\n",
  "$N appears from a cloud of thick red smoke!\r\n",
  "$N disappears in a thick black cloud!\r\n"
  "As $n makes a strange magical gesture, you feel a strong breeze.\r\n",
  "As $n makes a strange magical gesture, you feel a searing heat.\r\n",
  "As $n makes a strange magical gesture, you feel a sudden chill.\r\n",
  "As $n makes a strange magical gesture, you feel the dust swirl.\r\n",
  "$n magically divides!\r\n",
  "$n animates a corpse!\r\n"
};

static char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Oh shit!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};

#define MOB_MONSUM_I		130
#define MOB_MONSUM_II		140
#define MOB_MONSUM_III		150
#define MOB_GATE_I		160
#define MOB_GATE_II		170
#define MOB_GATE_III		180
#define MOB_ELEMENTAL_BASE	110
#define MOB_CLONE		69
#define MOB_ZOMBIE		101
#define MOB_AERIALSERVANT	109


void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		      int spellnum, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data *tobj, *next_obj;
  struct follow_type *f;
  int pfail = 0;
  int msg = 0, fmsg = 0;
  int num = 1;
  int i;
  int mob_num = 0;
  int handle_corpse = 0;
  int charmed_levels = 0, num_fol=0;
  extern int max_charmies;

  if (ch == NULL)
    return;

  switch (spellnum) {
  case SPELL_ANIMATE_DEAD:
    if ((obj == NULL) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) ||
	(!GET_OBJ_VAL(obj, 3))) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if(GET_OBJ_VAL(obj, 3) > 1) {
      send_to_char("You may not animate the corpse of a player.\r\n", ch);
      return;
    }
    handle_corpse = 1;
    msg = 12;
    mob_num = MOB_ZOMBIE;
    pfail = 0;
    send_to_char("You shiver as you call on powers of darkness.\r\n", ch);
    break;

  default:
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("You are too giddy to have any followers!\r\n", ch);
    return;
  }

  for (f = ch->followers; f; f = f->next) {
    if (IS_AFFECTED(f->follower, AFF_CHARM)) {
      charmed_levels += GET_LEVEL(f->follower);
      num_fol++;
    }
  }
  if (((charmed_levels+(level/10)) > level) || (num_fol >= max_charmies)) {
    send_to_char("You can't have that many followers.\n\r", ch);
    return;
  }

  if (number(0, 101) < pfail) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    return;
  }

  if(real_mobile(mob_num) <0) {
    send_to_char("There is an error with this spell.\r\n", ch);
    return;
  }

  for (i = 0; i < num; i++) {
    mob = read_mobile(mob_num, VIRTUAL);
    char_to_room(mob, ch->in_room);
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
    add_follower(mob, ch);
    act(mag_summon_msgs[fmsg], FALSE, ch, 0, mob, TO_ROOM);
    switch(spellnum) {
    case SPELL_ANIMATE_DEAD:
      GET_LEVEL(mob) = level/10;
      GET_HIT(mob) = GET_MAX_HIT(mob) = 5*level;
      GET_HITROLL(mob) = level/2;
      GET_DAMROLL(mob) = level/10;
      mob->mob_specials.damnodice = level/10;
      mob->mob_specials.damsizedice = level/10;
      break;
    }
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


void mag_points(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int hit = 0;
  int mana = 0;
  int move = 0;

  if((ch == NULL) || (victim == NULL))
    return;

  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    hit = dice(2, 4);
    send_to_char("You feel better.\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_CURE_SERIOUS:
    hit = dice(3, 8) + 3;
    send_to_char("You feel better!\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_CURE_CRITIC:
    hit = dice(5, 8) + 3;
    send_to_char("You feel a lot better!\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_REVITALIZE:
    hit = MIN(dice(level >> 1, 4), 80);
    send_to_char("Your body tingles for a few seconds.\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_HEAL:
    hit = 100;
    send_to_char("A warm feeling floods your body.\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_POWER_HEAL:
    hit = 200;
    send_to_char("You feel a warming go over your body as you are power healed.\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_RESTORE:
    hit = 400;
    send_to_char("You feel restored.\r\n", victim);
    check_drawn_in(ch, victim);
    break;
  case SPELL_ENERGY_DRAIN:
    if((!mag_savingthrow(victim, savetype)) && (GET_LEVEL(victim) > 2)) {
      mana = GET_MANA(victim) >> 1;
      GET_MOVE(victim) >>= 1;
      GET_MANA(ch) = MAX(GET_MAX_MANA(ch), GET_MANA(ch) + (mana >> 1));
      mana *= -1;
      send_to_char("Your life energy is drained!\r\n", victim);
    }
    break;
  case SPELL_REJUVENATE:
    move = 50;
    send_to_char("You feel ready to move on.\r\n", victim);
    break;
  }
  GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MANA(victim) = MAX(MIN(GET_MAX_MANA(victim), GET_MANA(victim) + mana), 0);
  GET_MOVE(victim) = MAX(MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move), 0);
  update_pos(victim);
}


void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		        int spellnum, int type)
{
  int spell = 0;
  char *to_vict = NULL, *to_room = NULL;

  if((ch == NULL) || (victim == NULL))
    return;

  switch (spellnum) {
  case SPELL_REMOVE_POISON:
    spell = SPELL_POISON;
    to_vict = "A warm feeling runs through your body!";
    to_room = "$n looks better.";
    break;
  case SPELL_REMOVE_CURSE:
    spell = SPELL_CURSE;
    to_vict = "You don't feel so unlucky.";
    break;
  case SPELL_CURE_DISEASE:
    spell = SPELL_CURE_DISEASE;
    to_vict = "Your fever breaks.";
    to_room = "$n looks healthier.";
    break;
  default:
    sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects", spellnum);
    log(buf);
    return;
    break;
  }

  if (!affected_by_spell(victim, spell)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  affect_from_char(victim, spell);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
		         int spellnum, int savetype)
{
  char *to_char = NULL;
  char *to_room = NULL;

  if (obj == NULL)
    return;

  switch (spellnum) {
    case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
	  GET_OBJ_VAL(obj, 2)--;
	to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
      GET_OBJ_VAL(obj, 3) = 1;
      to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2) = GET_OBJ_VAL(obj_proto+obj->item_number, 2);
        to_char = "$p briefly glows blue.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "$p steams briefly.";
      }
      break;
  }

  if (to_char == NULL)
    send_to_char(NOEFFECT, ch);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}



void mag_creations(int level, struct char_data * ch, int spellnum)
{
  struct obj_data *tobj;
  int z;

  if (ch == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  switch (spellnum) {
  case SPELL_CREATE_FOOD:
    z = 10;
    break;
  default:
    send_to_char("Spell unimplemented, it would seem.\r\n", ch);
    return;
    break;
  }

  if (!(tobj = read_object(z, VIRTUAL))) {
    send_to_char("I seem to have goofed.\r\n", ch);
    sprintf(buf, "SYSERR: spell_creations, spell %d, obj %d: obj not found",
	    spellnum, z);
    log(buf);
    return;
  }
  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
}


void mag_continuous(int level, struct char_data *ch, struct char_data *victim,
                    struct obj_data *obj, int spellnum, int savetype)
{
  int multiple_ok=-1;
  struct spcontinuous *s, *temp;

  if(ch == NULL)
    return;

  CREATE(s, struct spcontinuous, 1);

  s->spspell=spellnum;
  s->spdata1=0;
  s->spdata2=0;
  s->spdata3=0;
  s->spdata4=0;
  s->sptarget=NULL;

  switch(spellnum) {
  case SKILL_SNEAK:
    s->sptimer=-1;
    s->spcost=0;
    break;
  case SKILL_BIOFEEDBACK:
    s->sptimer=-1;
    s->spcost=3;
    s->spdata1=GET_HIT(ch);
    break;
  case SKILL_ADRENALIN_CONTROL:
    s->sptimer=-1;
    if(AFF_FLAGGED(ch, AFF_MAGNIFY))
      s->spcost=8;
    else
      s->spcost=4;
    break;
  case SKILL_LIFE_DETECTION:
    s->sptimer=-1;
    s->spcost=3;
    break;
  case SKILL_TELEKINESIS:
    s->sptimer=-1;
    s->spcost=MAX(GET_OBJ_WEIGHT(obj)/20, 1);
    s->sptarget=obj;
    break;
  case SKILL_CHAMELEON_POWER:
    s->sptimer=-1;
    s->spcost=4;
    break;
  case SKILL_BODY_WEAPONRY:
    s->sptimer=-1;
    s->spcost=3;
    s->sptarget=obj;
    break;
  case SKILL_DOMINATION:
    s->sptimer=-1;
    s->spcost=4;
    s->sptarget=victim;
    break;
  case SKILL_CELL_ADJUSTMENT:
    s->sptimer=-1;
    s->spcost=10;
    s->sptarget=victim;
    break;
  case SKILL_GRAFT_WEAPON:
    s->sptimer=-1;
    s->spcost=1;
    s->sptarget=obj;
    break;
  case SKILL_FLESH_ARMOR:
    s->sptimer=-1;
    s->spcost=4;
    s->spdata1=level;
    break;
  case SKILL_DISPLACEMENT:
    s->sptimer=-1;
    s->spcost=3;
    break;
  case SKILL_DANGER_SENSE:
    s->sptimer=-1;
    s->spcost=2;
    break;
  case SKILL_MOLECULAR_AGITATION:
    s->sptimer=-1;
    s->spcost=5;
    s->sptarget=victim;
    if(AFF_FLAGGED(ch, AFF_MAGNIFY))
      s->spdata1=5;
    else
      s->spdata1=1;
    multiple_ok=1;
    break;
  case SKILL_DIMENSION_DOOR:
    s->sptimer=-1;
    s->spcost=2;
    s->spdata1=level;
    s->spdata2=savetype;
    multiple_ok=1;
    break;
  case SKILL_LIFE_DRAINING:
   s->sptimer=-1;
   s->spcost=5;
   s->sptarget=victim;
   s->spdata1=savetype;
   break;
  case SKILL_FEEL_LIGHT:
    s->sptimer=-1;
    s->spcost=1;
    break;
  case SKILL_ENERGY_CONTAINMENT:
    s->sptimer=-1;
    s->spcost=5;
    break;
  case SKILL_METAMORPHOSIS:
    s->sptimer=-1;
    s->spcost=3;
    break;
  case SKILL_COMPLETE_HEALING:
    s->spcost=0;
    if(AFF_FLAGGED(ch, AFF_MAGNIFY)) {
      s->sptimer=15;
      s->spdata1=16;
    }
    else {
      s->sptimer=30;
      s->spdata1=1;
    }
    s->spdata2=MAX(1, (GET_MAX_HIT(ch)-GET_HIT(ch))/60);
    break;
  case SKILL_SPLIT_PERSONALITY:
    s->sptimer=-1;
    s->spcost=6;
    break;
  case SKILL_STASIS_FIELD:
    s->sptimer=-1;
    s->spcost=20;
    s->spdata1=ch->in_room;
    multiple_ok=1;
    break;
  case SKILL_MAGNIFY:
    s->sptimer=-1;
    s->spcost=5;
    s->spdata1=number(0, 2);
    break;
  default:
    s->sptimer=2;
    s->spcost=0;
    break;
  }

  if((GET_LEVEL(ch)>=LVL_HERO)||IS_NPC(ch)) {
    s->spcost=0;
  }

  if(multiple_ok <= 0) {
    for(temp=ch->char_specials.spcont; temp; temp=temp->next)
      if(temp->spspell==spellnum)
        break;
    if(temp) {
      if(!multiple_ok) {
        send_to_char(NOEFFECT, ch);
      }
      free(s);
      return;
    }
  }
  s->next=ch->char_specials.spcont;
  ch->char_specials.spcont=s;
}

/*
* Handles spells with a continuous affect and updates drainedness
* char_specials.spcont
*/

void update_spells(void)
{
  int i, ac, tempac;
  struct char_data *ch, *next_char, *tch;
  struct obj_data *tobj;
  struct spcontinuous *s, *temp, *next_s;
  struct affected_type *af;

  void report_danger(struct char_data *ch);

  for (ch = character_list; ch; ch = next_char) {
    next_char = ch->next;
    /* handle stun counter here if they aren't fighting */
    if((!FIGHTING(ch)) && (GET_STUN(ch) > 0)) {
      GET_STUN(ch)--;
      update_pos(ch);
    }
    if(ch->char_specials.spdrained > 0)
      ch->char_specials.spdrained--;
    if(ch->char_specials.spnocast > 0)
      ch->char_specials.spnocast--;
    for(s=ch->char_specials.spcont; s; s=next_s)
    {
      next_s=s->next;
      if(s->sptarget) {
        for(tobj=object_list; tobj; tobj=tobj->next) {
          if(s->sptarget==tobj)
            break;
        }
        if(!tobj) {
          for(tch=character_list; tch; tch=tch->next) {
            if(s->sptarget==tch)
              break;
          }
          if(!tch) {
            REMOVE_FROM_LIST(s, ch->char_specials.spcont, next);
            free(s);
            continue;
          }
        }
      }
      if(s->sptimer > 0)
        s->sptimer--;
      if(s->sptimer)
        if(GET_MANA(ch) >= s->spcost)
          GET_MANA(ch)-=s->spcost;
        else {
          GET_MANA(ch)=0;
          s->sptimer=0;
        }
      if(s->sptimer)
      {
        /* Put effects that happen each tic here */
        switch (s->spspell) {
        case SKILL_BIOFEEDBACK:
          if(GET_HIT(ch) < s->spdata1)
          GET_HIT(ch) += MIN(10, s->spdata1-GET_HIT(ch));
          s->spdata1=GET_HIT(ch);
          break;
        case SKILL_TELEKINESIS:
          tobj=(struct obj_data *)s->sptarget;
          if(tobj->carried_by!=ch)
            s->sptimer=0;
          break;
        case SKILL_CHAMELEON_POWER:
          SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
          break;
        case SKILL_BODY_WEAPONRY:
          if(((struct obj_data *)s->sptarget)->worn_by!=ch)
            extract_obj((struct obj_data *)s->sptarget);
          break;
        case SKILL_DOMINATION:
          tch=(struct char_data *)s->sptarget;
          if(tch->master != ch)
            s->sptimer=0;
          break;
        case SKILL_CELL_ADJUSTMENT:
          tch=(struct char_data *)s->sptarget;
          GET_HIT(tch) = MIN(GET_MAX_HIT(tch), GET_HIT(tch)+20);
          if(GET_HIT(tch) == GET_MAX_HIT(tch))
            s->sptimer=0;
          break;
        case SKILL_GRAFT_WEAPON:
          if(((struct obj_data *)s->sptarget)->worn_by!=ch)
            s->sptimer=0;
          break;
        case SKILL_FLESH_ARMOR:
          ac=0;
          if((tobj=GET_EQ(ch, WEAR_ARMS))) {
            tempac=0;
            if(GET_OBJ_TYPE(tobj) == ITEM_ARMOR)
              tempac-=GET_OBJ_VAL(tobj, 0);
            for(i=0; i < MAX_OBJ_AFFECT; i++) {
              if(tobj->affected[i].location==APPLY_AC)
                tempac+=tobj->affected[i].modifier;
            }
            if(tempac < 0)
              ac+=tempac;
          }
          if((tobj=GET_EQ(ch, WEAR_LEGS))) {
            tempac=0;
            if(GET_OBJ_TYPE(tobj) == ITEM_ARMOR)
              tempac-=2*GET_OBJ_VAL(tobj, 0);
            for(i=0; i < MAX_OBJ_AFFECT; i++) {
              if(tobj->affected[i].location==APPLY_AC)
                tempac+=tobj->affected[i].modifier;
            }
            if(tempac < 0)
              ac+=tempac;
          }
          for(af=ch->affected; af; af=af->next) {
            if(af->type==SKILL_FLESH_ARMOR) {
              affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
              af->modifier=MIN(s->spdata1-ac, 0);
              affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
            }
          }
          affect_total(ch);
          break;
        case SKILL_DANGER_SENSE:
          report_danger(ch);
          break;
        case SKILL_MOLECULAR_AGITATION:
          tch=(struct char_data *)s->sptarget;
          if(tch->in_room != ch->in_room) {
            sprintf(buf, "%s is out of range of molecular agitation.\r\n", GET_NAME(tch));
            send_to_char(buf, ch);
            s->sptimer=0;
          }
          else {
            damage(ch, tch, dice(s->spdata1, 4), SKILL_MOLECULAR_AGITATION);
            if(s->spdata1 < 10)
              s->spdata1++;
          }
          break;
        case SKILL_LIFE_DRAINING:
          tch=(struct char_data *)s->sptarget;
          if(tch->in_room != ch->in_room) {
            sprintf(buf, "%s is out of range of life draining.\r\n", GET_NAME(tch));
            send_to_char(buf, ch);
            s->sptimer=0;
          }
          else {
            send_to_char("You feel your life flowing away.\r\n", tch);
            send_to_char("You feel life flowing into you.\r\n", ch);
            i=dice(5, s->spdata1?6:20);
            damage(ch, tch, i, TYPE_SILENT);
            GET_HIT(ch)+=i;
            if(GET_HIT(ch) > GET_MAX_HIT(ch))
              s->sptimer=0;
          }
          break;
        case SKILL_COMPLETE_HEALING:
          if(s->spdata1 == 15) {
            affect_from_char(ch, SPELL_CURE_DISEASE);
            affect_from_char(ch, SPELL_POISON);
          }
          GET_HIT(ch)+=s->spdata2;
          s->spdata1++;
          GET_STUN(ch)=3;
          WAIT_STATE(ch, 3*PULSE_VIOLENCE);
          GET_POS(ch)=POS_STUNNED;
          if(FIGHTING(ch))
            s->sptimer=0;
          break;
        case SKILL_STASIS_FIELD:
          if(s->spdata1!=ch->in_room)
            s->spcost=60;
          else
            s->spcost=20;
          break;
        default:
          break;
        }
      }
      else
      {
        /* Put wear-off effects here */
        switch (s->spspell) {
        case SKILL_SNEAK:
          send_to_char("You stop sneaking.\r\n", ch);
          affect_from_char(ch, SKILL_SNEAK);
          break;
        case SKILL_BIOFEEDBACK:
          send_to_char("Your blood flow returns to normal.\r\n", ch);
          affect_from_char(ch, SKILL_BIOFEEDBACK);
          break;
        case SKILL_ADRENALIN_CONTROL:
          send_to_char("Your adrenalin flow returns to normal.\r\n", ch);
          affect_from_char(ch, SKILL_ADRENALIN_CONTROL);
          break;
        case SKILL_LIFE_DETECTION:
          send_to_char("Your sense of minds fades.\r\n", ch);
          affect_from_char(ch, SKILL_LIFE_DETECTION);
          break;
        case SKILL_TELEKINESIS:
          tobj=(struct obj_data *)s->sptarget;
          sprintf(buf, "You lose control of %s.\r\n", tobj->short_description);
          send_to_char(buf, ch);
          if(tobj->carried_by==ch) {
            obj_from_char(tobj);
            obj_to_room(tobj, ch->in_room);
          }
          break;
        case SKILL_CHAMELEON_POWER:
          send_to_char("Your coloring returns to normal.\r\n", ch);
          REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
          affect_from_char(ch, SKILL_CHAMELEON_POWER);
          break;
        case SKILL_BODY_WEAPONRY:
          send_to_char("You hand returns to normal.\r\n", ch);
          extract_obj((struct obj_data *)s->sptarget);
          break;
        case SKILL_DOMINATION:
          tch=(struct char_data *)s->sptarget;
          affect_from_char(tch, SKILL_DOMINATION);
          if(tch->master == ch)
            stop_follower(tch);
          send_to_char("Your free will returns.\r\n", tch);
          sprintf(buf, "You lose control of %s.\r\n", GET_NAME(tch));
          send_to_char(buf, ch);
          break;
        case SKILL_CELL_ADJUSTMENT:
          tch=(struct char_data *)s->sptarget;
          sprintf(buf, "You stop adjusting %s's cells.\r\n", GET_NAME(tch));
          send_to_char(buf, ch);
          break;
        case SKILL_GRAFT_WEAPON:
          tobj=(struct obj_data *)s->sptarget;
          affect_from_char(ch, SKILL_GRAFT_WEAPON);
          sprintf(buf, "%s separates from your hand.\r\n", tobj->short_description);
          send_to_char(buf, ch);
          break;
        case SKILL_FLESH_ARMOR:
          send_to_char("Your skin returns to normal.\r\n", ch);
          affect_from_char(ch, SKILL_FLESH_ARMOR);
          break;
        case SKILL_DISPLACEMENT:
          send_to_char("Your image returns to your real location.\r\n", ch);
          affect_from_char(ch, SKILL_DISPLACEMENT);
          break;
        case SKILL_DANGER_SENSE:
          send_to_char("You can no longer sense danger.\r\n", ch);
          break;
        case SKILL_MOLECULAR_AGITATION:
          sprintf(buf, "You stop burning %s.\r\n", GET_NAME((struct char_data *)s->sptarget));
          send_to_char(buf, ch);
          break;
        case SKILL_DIMENSION_DOOR:
          send_to_char("Your dimension door closes.\r\n", ch);
          send_to_room("The portal closes.\r\n", s->spdata1);
          send_to_room("The portal closes.\r\n", s->spdata2);
          REMOVE_BIT(ROOM_FLAGS(s->spdata1), ROOM_DIM_DOOR);
          REMOVE_BIT(ROOM_FLAGS(s->spdata2), ROOM_DIM_DOOR);
          break;
        case SKILL_LIFE_DRAINING:
          sprintf(buf, "You stop draining life from %s.\r\n", GET_NAME((struct char_data *)s->sptarget));
          send_to_char(buf, ch);
          break;
        case SKILL_FEEL_LIGHT:
          send_to_char("You can no longer feel light.\r\n", ch);
          affect_from_char(ch, SKILL_FEEL_LIGHT);
          break;
        case SKILL_ENERGY_CONTAINMENT:
          send_to_char("You will no longer absorb energy.\r\n", ch);
          affect_from_char(ch, SKILL_ENERGY_CONTAINMENT);
          break;
        case SKILL_METAMORPHOSIS:
          send_to_char("You change back to your normal form.\r\n", ch);
          affect_from_char(ch, SKILL_METAMORPHOSIS);
          break;
        case SKILL_COMPLETE_HEALING:
          send_to_char("You awaken from your trance.\r\n", ch);
          if(s->spdata1 >= 30)
            GET_HIT(ch)=GET_MAX_HIT(ch);
          GET_POS(ch)=POS_STANDING;
          GET_STUN(ch)=0;
          WAIT_STATE(ch, 0);
          update_pos(ch);
          break;
        case SKILL_SPLIT_PERSONALITY:
          send_to_char("Your mind becomes one again.\r\n", ch);
          affect_from_char(ch, SKILL_SPLIT_PERSONALITY);
          break;
        case SKILL_STASIS_FIELD:
          send_to_char("You drop the stasis field.\r\n", ch);
          send_to_room("Time returns to normal.\r\n", s->spdata1);
          REMOVE_BIT(ROOM_FLAGS(s->spdata1), ROOM_STASIS);
          break;
        case SKILL_MAGNIFY:
          send_to_char("You will no longer magnify powers.\r\n", ch);
          affect_from_char(ch, SKILL_MAGNIFY);
          break;
        default:
          break;
        }
        REMOVE_FROM_LIST(s, ch->char_specials.spcont, next);
        free(s);
      }
    }
  }
}
