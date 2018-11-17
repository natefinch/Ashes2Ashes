/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "class.h"

#define SLOW_CHANCE 50
#define XP_REDUCTION_RATE 65

/* Structures */
struct char_data *combat_list = NULL;	/* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern int auto_save;		/* see config.c */
extern int max_exp_gain;	/* see config.c */

/* External procedures */
void mprog_death_trigger(struct char_data *mob, struct char_data *killer);
char *fread_action(FILE * fl, int nr);
char *fread_string(FILE * fl, char *error);
void stop_follower(struct char_data * ch);
ACMD(do_flee);
ACMD(do_get);
void hit(struct char_data * ch, struct char_data * victim, int type);
void forget(struct char_data * ch, struct char_data * victim);
void remember(struct char_data * ch, struct char_data * victim);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
int mag_savingthrow(struct char_data * ch, int type);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},		/* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},	/* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},	/* 10 */
  {"stab", "stabs"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"pierce", "pierces"}
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SILENT))

/* The Fight related routines */

void appear(struct char_data * ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE);

  if (GET_LEVEL(ch) < LVL_HERO)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}



void load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r"))) {
    sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
    perror(buf2);
    exit(1);
  }
  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }


  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while (*chk == 'M') {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	 (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES) {
      fprintf(stderr, "Too many combat messages.  Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}


void update_pos(struct char_data * victim)
{

  if ((GET_HIT(victim) > 0) && ((GET_POS(victim) > POS_STUNNED) || GET_STUN(victim)))
    return;
  else if ((GET_HIT(victim) > 0) && (GET_CON(victim) >= 3)) {
    send_to_char("You feel that you can move again.\r\n", victim);
    GET_POS(victim) = POS_STANDING;
  }
  else if (GET_HIT(victim) <= -11)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
  if((GET_CON(victim)<3) && (GET_POS(victim) > POS_STUNNED)) {
    send_to_char("You collapse as a result of your low constitution.\r\n", victim);
    GET_POS(victim) = POS_STUNNED;
  }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data * ch, struct char_data * vict)
{
  if (ch == vict)
    return;

  assert(!FIGHTING(ch));

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (IS_AFFECTED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  FIGHTING(ch) = vict;
  if(IS_NPC(ch)&&(!GET_MOB_WAIT(ch)))
    GET_POS(ch) = POS_FIGHTING;

  if(GET_POS(ch) > POS_FIGHTING)
    GET_POS(ch) = POS_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data * ch)
{
  struct char_data *temp;

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;

  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  WAIT_STATE(ch, 0);
  GET_STUN(ch)=0;
  GET_POS(ch) = POS_STANDING;
  update_pos(ch);
  if(IS_NPC(ch) && (GET_HIT(ch) > 0))
    affect_total(ch);
}



void make_corpse(struct char_data * ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;
  int i, j;
  extern int max_npc_corpse_time, max_pc_corpse_time;

  struct obj_data *create_money(int amount);

  if(IS_NPC(ch)&&(GET_CLASS(ch)==CLASS_UNDEAD)) {
    act("$n crumbles to dust as $e dies.", FALSE, ch, NULL, NULL, TO_ROOM);
    while(ch->carrying) {
      o=ch->carrying;
      obj_from_char(o);
      obj_to_room(o, ch->in_room);
      object_list_new_owner(o, NULL);
      if((!IS_NPC(ch))&&(GET_LEVEL(ch) >= LVL_HERO)) {
        o->obj_flags.value[0]=0;
        o->obj_flags.value[1]=0;
        o->obj_flags.value[2]=0;
        o->obj_flags.value[3]=0;
        o->obj_flags.type_flag=13;
        o->obj_flags.cost=0;
        o->obj_flags.bitvector=0;
        o->obj_flags.immune=0;
        o->obj_flags.resist=0;
        o->obj_flags.weak=0;
        for(j=0; j<MAX_OBJ_AFFECT; j++) {
          o->affected[j].location=APPLY_NONE;
          o->affected[j].modifier=0;
        }
      }
    }
    for(i=0; i<NUM_WEARS; i++) {
      if(GET_EQ(ch, i)) {
        obj_to_room((o=unequip_char(ch, i)), ch->in_room);
        if((!IS_NPC(ch))&&(GET_LEVEL(ch) >= LVL_HERO)) {
          o->obj_flags.value[0]=0;
          o->obj_flags.value[1]=0;
          o->obj_flags.value[2]=0;
          o->obj_flags.value[3]=0;
          o->obj_flags.type_flag=13;
          o->obj_flags.cost=0;
          o->obj_flags.bitvector=0;
          o->obj_flags.immune=0;
          o->obj_flags.resist=0;
          o->obj_flags.weak=0;
          for(j=0; j<MAX_OBJ_AFFECT; j++) {
            o->affected[j].location=APPLY_NONE;
            o->affected[j].modifier=0;
          }
        }
      }
    }
    if(GET_GOLD(ch) > 0) {
      if((IS_NPC(ch))||(GET_LEVEL(ch) < LVL_HERO)) {
        money = create_money(GET_GOLD(ch));
        obj_to_room(money, ch->in_room);
      }
    }
    GET_GOLD(ch)=0;
    IS_CARRYING_N(ch) = 0;
    IS_CARRYING_W(ch) = 0;
  }
  else {
    corpse = create_obj();

    corpse->item_number = NOTHING;
    corpse->in_room = NOWHERE;
    if(IS_NPC(ch))
      corpse->name = str_dup("corpse corpseofamob0");
    else
      corpse->name = str_dup("corpse");

    sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
    corpse->description = str_dup(buf2);

    sprintf(buf2, "the corpse of %s", GET_NAME(ch));
    corpse->short_description = str_dup(buf2);

    GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
    GET_OBJ_WEAR(corpse) = ITEM_WEAR_TAKE;
    GET_OBJ_EXTRA(corpse) = ITEM_NORENT | ITEM_NODONATE | ITEM_NOINVIS | ITEM_NOSELL;
    GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
    GET_OBJ_VAL(corpse, 3) = (IS_NPC(ch)? 1 : 2); /* corpse identifier */
    GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
    GET_OBJ_COST(corpse) = 100000;
    GET_OBJ_RENT(corpse) = 100000;


    /* transfer character's inventory to the corpse */
    corpse->contains = ch->carrying;
    for (o = corpse->contains; o != NULL; o = o->next_content) {
      o->in_obj = corpse;
      if((!IS_NPC(ch))&&(GET_LEVEL(ch) >= LVL_HERO)) {
        o->obj_flags.value[0]=0;
        o->obj_flags.value[1]=0;
        o->obj_flags.value[2]=0;
        o->obj_flags.value[3]=0;
        o->obj_flags.type_flag=13;
        o->obj_flags.cost=0;
        o->obj_flags.bitvector=0;
        o->obj_flags.immune=0;
        o->obj_flags.resist=0;
        o->obj_flags.weak=0;
        for(j=0; j<MAX_OBJ_AFFECT; j++) {
          o->affected[j].location=APPLY_NONE;
          o->affected[j].modifier=0;
        }
      }
    }
    object_list_new_owner(corpse, NULL);

    /* transfer character's equipment to the corpse */
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
        obj_to_obj((o=unequip_char(ch, i)), corpse);
        if((!IS_NPC(ch))&&(GET_LEVEL(ch) >= LVL_HERO)) {
          o->obj_flags.value[0]=0;
          o->obj_flags.value[1]=0;
          o->obj_flags.value[2]=0;
          o->obj_flags.value[3]=0;
          o->obj_flags.type_flag=13;
          o->obj_flags.cost=0;
          o->obj_flags.bitvector=0;
          o->obj_flags.immune=0;
          o->obj_flags.resist=0;
          o->obj_flags.weak=0;
          for(j=0; j<MAX_OBJ_AFFECT; j++) {
            o->affected[j].location=APPLY_NONE;
            o->affected[j].modifier=0;
          }
        }
      }

    /* transfer gold */
    if (GET_GOLD(ch) > 0) {
      /* following 'if' clause added to fix gold duplication loophole */
      if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
        if((IS_NPC(ch))||(GET_LEVEL(ch) < LVL_HERO)) {
          money = create_money(GET_GOLD(ch));
          obj_to_obj(money, corpse);
        }
      }
      GET_GOLD(ch) = 0;
    }
    ch->carrying = NULL;
    IS_CARRYING_N(ch) = 0;
    IS_CARRYING_W(ch) = 0;

    if(ch->in_room > 0)
      obj_to_room(corpse, ch->in_room);
    else if(GET_WAS_IN(ch) > 0)
      obj_to_room(corpse, GET_WAS_IN(ch));
    else
      obj_to_room(corpse, 0);

    if (IS_NPC(ch))
      GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
    else
      GET_OBJ_TIMER(corpse) = max_pc_corpse_time;
  }
}


/* When ch kills victim */
void change_alignment(struct char_data * ch, struct char_data * victim)
{
  int temp;
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
  /* Modified to attract towards the poles more, still fast, and more accurate */
  /* Scrapped the alignment thing, lets try something new
  if ((GET_ALIGNMENT(ch) < -350) && (GET_ALIGNMENT(victim) < GET_ALIGNMENT(ch)))
    GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 3;
  else if ((GET_ALIGNMENT(ch) > 350) && (GET_ALIGNMENT(victim) > GET_ALIGNMENT(ch)))
    GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 3;
  else
    GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 4;
  */
  temp=GET_ALIGNMENT(ch)+GET_ALIGNMENT(victim);
  if(temp < 0)
    temp = -temp;
  temp = (temp/200)+6;
  GET_ALIGNMENT(ch) -= (GET_ALIGNMENT(victim)/temp);
  GET_ALIGNMENT(ch) = MIN(1000, MAX(-1000, GET_ALIGNMENT(ch)));
}



void death_cry(struct char_data * ch)
{
  int door, was_in;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
  was_in = ch->in_room;

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      act("Your blood freezes as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}



void raw_kill(struct char_data * ch)
{
  if (FIGHTING(ch))
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected);

  death_cry(ch);

  make_corpse(ch);
  extract_char(ch);
}


void stat_loss(struct char_data * ch)
{
  if(number(0, 99) < (GET_LEVEL(ch)/3)) {
    ch->real_abils.con-=1;
  }
  affect_total(ch);
}


void die(struct char_data * ch, struct char_data *killer)
{
  struct char_data *tch;
  struct spcontinuous *c;

  for(c=ch->char_specials.spcont; c; c=c->next)
    c->sptimer=0;

  if((!IS_NPC(ch))&&(ARENA(ch).kill_mode)) {
    GET_HIT(ch)=ARENA(ch).hit;
    GET_MANA(ch)=ARENA(ch).mana;
    GET_MOVE(ch)=ARENA(ch).move;
    if(GET_HIT(ch)>0)
      GET_POS(ch)=POS_STANDING;
    else
      update_pos(ch);
    char_from_room(ch);
    char_to_room(ch, real_room(3072));
    act("$n appears through a portal.", TRUE, ch, NULL, NULL, TO_ROOM);
    look_at_room(ch, 0);
    sprintf(buf1, "%s has triumphed over %s!", GET_NAME(killer), GET_NAME(ch));
    arenasay(buf1);
    if(ARENA(ch).kill_mode != -1) { /* not challenging all */
      if(ARENA(ch).challenged_by) { /* group challenge */
        for(tch=character_list; tch; tch=tch->next) {
          if((ARENA(tch).kill_mode==ARENA(ch).kill_mode)&&(tch!=killer)&&(tch!=ch)) {
            break;
          }
        }
        if(!tch) {
          sprintf(buf, "%s has won the group contest!", GET_NAME(killer));
          arenasay(buf);
          GET_HIT(killer)=ARENA(killer).hit;
          GET_MANA(killer)=ARENA(killer).mana;
          GET_MOVE(killer)=ARENA(killer).move;
          if(GET_HIT(killer)>0)
            GET_POS(killer)=POS_STANDING;
          else
            update_pos(killer);
          char_from_room(killer);
          char_to_room(killer, real_room(3072));
          act("$n appears through a portal.", TRUE, killer, NULL, NULL, TO_ROOM);
          look_at_room(killer, 0);
          ARENA(killer).kill_mode=0;
          ARENA(killer).challenging=0;
          ARENA(killer).challenged_by=0;
          ARENA(killer).hit=0;
          ARENA(killer).mana=0;
          ARENA(killer).move=0;
        }
      }
      else { /* personal challenge */
        GET_HIT(killer)=ARENA(killer).hit;
        GET_MANA(killer)=ARENA(killer).mana;
        GET_MOVE(killer)=ARENA(killer).move;
        if(GET_HIT(killer)>0)
          GET_POS(killer)=POS_STANDING;
        else
          update_pos(killer);
        char_from_room(killer);
        char_to_room(killer, real_room(3072));
        act("$n appears through a portal.", TRUE, killer, NULL, NULL, TO_ROOM);
        look_at_room(killer, 0);
        ARENA(killer).kill_mode=0;
        ARENA(killer).challenging=0;
        ARENA(killer).challenged_by=0;
        ARENA(killer).hit=0;
        ARENA(killer).mana=0;
        ARENA(killer).move=0;
      }
    }

    ARENA(ch).kill_mode=0;
    ARENA(ch).challenging=0;
    ARENA(ch).challenged_by=0;
    ARENA(ch).hit=0;
    ARENA(ch).mana=0;
    ARENA(ch).move=0;
    if(FIGHTING(ch))
      stop_fighting(ch);
    if(FIGHTING(killer))
      stop_fighting(killer);
    return;
  }

  if(HUNTING(killer)==ch)
    HUNTING(killer)=NULL;

  if (IS_NPC(ch)) {
    if(killer)
      mprog_death_trigger(ch, killer);
    if(ch->in_room==NOWHERE)
      return;
  }

  if((IS_NPC(ch))&&(GET_CLASS(ch)!=CLASS_UNDEAD)&&(killer)&&(!IS_NPC(killer))) {
    raw_kill(ch);
    if(AFF_FLAGGED(killer, AFF_GROUP) && (killer->master) &&
       AFF_FLAGGED(killer->master, AFF_GROUP) && (!IS_NPC(killer->master)) &&
       (killer->in_room==killer->master->in_room)) {
      if(PRF_FLAGGED(killer->master, PRF_AUTOLOOT)) {
        do_get(killer->master, "all corpseofamob0", 0, 0);
      }
      else if(PRF_FLAGGED(killer->master, PRF_AUTOGOLD)) {
        do_get(killer->master, "all.coins corpseofamob0", 0, 0);
        if(PRF_FLAGGED(killer, PRF_AUTOLOOT)) {
          do_get(killer, "all corpseofamob0", 0, 0);
        }
      }
      else {
        if(PRF_FLAGGED(killer, PRF_AUTOLOOT)) {
          do_get(killer, "all corpseofamob0", 0, 0);
        }
        else if(PRF_FLAGGED(killer, PRF_AUTOGOLD)) {
          do_get(killer, "all.coins corpseofamob0", 0, 0);
        }
      }
    }
    else {
      if(PRF_FLAGGED(killer, PRF_AUTOLOOT)) {
        do_get(killer, "all corpseofamob0", 0, 0);
      }
      else if(PRF_FLAGGED(killer, PRF_AUTOGOLD)) {
        do_get(killer, "all.coins corpseofamob0", 0, 0);
      }
    }
  }
  else {
    raw_kill(ch);
  }
}



void perform_group_gain(struct char_data * ch, int base,
			     struct char_data * victim)
{
  int share;

  share = MIN(max_exp_gain, MAX(1, base));
  share = gain_exp(ch, share);

  if (share > 1) {
    sprintf(buf2, "You receive your share of experience -- %d points.\r\n", share);
    send_to_char(buf2, ch);
  } else
    send_to_char("You receive your share of experience -- one measly little point!\r\n", ch);

  change_alignment(ch, victim);

  /* save 25% of the time */
  if(auto_save)
    if(!number(0,3))
      save_char(ch, NOWHERE);
}


void group_gain(struct char_data * ch, struct char_data * victim)
{
  int tot_members=0, base=0, group_level=0, high_level=0, total_levels;
  struct char_data *k;
  struct follow_type *f;

  if (!(k = ch->master))
    k = ch;

  if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
    high_level=group_level=GET_LEVEL(k);
    tot_members = 1;
  }

  for (f = k->followers; f; f = f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room) {
      group_level+=GET_LEVEL(f->follower);
      if(GET_LEVEL(f->follower)>high_level)
        high_level=GET_LEVEL(f->follower);
      tot_members++;
    }
  }

  total_levels = group_level;
  group_level /= tot_members;
  group_level += (group_level*(tot_members-1))/20;
  group_level = MAX(group_level, high_level);

  base = GET_EXP(victim);
  if(GET_LEVEL(victim) < group_level) {
    base -= (base*(group_level-GET_LEVEL(victim)))/XP_REDUCTION_RATE;
  }
  base = MAX(base, 1);

  if (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room)
    perform_group_gain(k, (base*GET_LEVEL(k))/total_levels, victim);

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      perform_group_gain(f->follower, (base*GET_LEVEL(f->follower))/total_levels, victim);
}



char *replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
  static char buf[256];
  char *cp;

  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	break;
      case 'w':
	for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  }				/* For */

  return (buf);
}


/* message for doing damage with a weapon */
void dam_message(int dam, struct char_data * ch, struct char_data * victim,
		      int w_type)
{
  char *buf;
  int msgnum;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {

    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

    {
      "$n tries to #w $N, but misses.",	/* 0: 0     */
      "You try to #w $N, but miss.",
      "$n tries to #w you, but misses."
    },

    {
      "$n scrapes $N with $s #w.",	/* 1: 1..2  */
      "You scrape $N as you #w $M.",
      "$n scrapes you as $e #W you."
    },

    {
      "$n barely #W $N.",		/* 2: 3..4  */
      "You barely #w $N.",
      "$n barely #W you."
    },

    {
      "$n #W $N.",			/* 3: 5..6  */
      "You #w $N.",
      "$n #W you."
    },

    {
      "$n #W $N hard.",			/* 4: 7..10  */
      "You #w $N hard.",
      "$n #W you hard."
    },

    {
      "$n #W $N very hard.",		/* 5: 11..14  */
      "You #w $N very hard.",
      "$n #W you very hard."
    },

    {
      "$n #W $N extremely hard.",	/* 6: 15..19  */
      "You #w $N extremely hard.",
      "$n #W you extremely hard."
    },

    {
      "$n MASSACRES $N to small fragments with $s #w.",		/* 7: 19..23 */
      "You MASSACRE $N to small fragments with your #w.",
      "$n MASSACRES you to small fragments with $s #w."
    },

    {
      "$n OBLITERATES $N with $s deadly #w!!",			/* 8: 24..28 */
      "You OBLITERATE $N with your deadly #w!!",
      "$n OBLITERATES you with $s deadly #w!!"
    },

    {
      "$n DEMOLISHES $N with $s deadly #w!!",			/* 9: 29..33 */
      "You DEMOLISH $N with your deadly #w!!",
      "$n DEMOLISHES you with $s deadly #w!!"
    },

    {
      "$n MUTILATES $N with $s deadly #w!!",			/* 10: 34..39 */
      "You MUTILATE $N with your deadly #w!!",
      "$n MUTILATES you with $s deadly #w!!"
    },

    {
      "$n LIQUIFIES $N with $s deadly #w!!",			/* 11: 40..45 */
      "You LIQUIFY $N with your deadly #w!!",
      "$n LIQUIFIES you with $s deadly #w!!"
    },

    {
      "$n DESTROYS $N with $s deadly #w!!",			/* 12: 46..50 */
      "You DESTROY $N with your deadly #w!!",
      "$n DESTROYS you with $s deadly #w!!"
    },

    {
      "$n DECIMATES $N with $s deadly #w!!",			/* 13: 51..60 */
      "You DECIMATE $N with your deadly #w!!",
      "$n DECIMATES you with $s deadly #w!!"
    },

    {
      "$n PULVERIZES $N with $s deadly #w!!",			/* 14: 61..74 */
      "You PULVERIZE $N with your deadly #w!!",
      "$n PULVERIZES you with $s deadly #w!!"
    },

    {
      "$n ANNIHILATES $N with $s SUPREME #w!!!",		/* 15: 75..89 */
      "You ANNIHILATE $N with your SUPREME #w!!!",
      "$n ANNIHILATES you with $s SUPREME #w!!!"
    },

    {
      "$n BUSTS OPEN A CAN OF WHOOP-ASS on $N with $s #w!!!",	/* 16: 90..109 */
      "You BUST OPEN A CAN OF WHOOP-ASS on $N with your #w!!!",
      "$n BUSTS OPEN A CAN OF WHOOP-ASS on you with $s #w!!!"
    },

    {
      "$n MAIMS $N with $s CRIPPLING #w!!!",			/* 17: 110..129 */
      "You MAIM $N with your CRIPPLING #w!!!",
      "$n MAIMS you with $s CRIPPLING #w!!!"
    },

    {
      "$n SAVAGES $N with $s DEVASTATING #w!!!",		/* 18: 130..149 */
      "You SAVAGE $N with your DEVASTATING #w!!!",
      "$n SAVAGES you with $s DEVASTATING #w!!!"
    },

    {
      "$n DISEMBOWELS $N with $s VICIOUS #w!!!",			/* 19: 150..169 */
      "You DISEMBOWEL $N with your VICIOUS #w!!!",
      "$n DISEMBOWELS you with $s VICIOUS #w!!!"
    },

    {
      "$n BUTCHERS $N with $s BRUTAL #w!!!",			/* 19: 170..189 */
      "You BUTCHER $N with your BRUTAL #w!!!",
      "$n BUTCHERS you with $s BRUTAL #w!!!"
    },

    {
      "$n DISMEMBERS $N with $s UNBELIEVABLE #w!!!",		/* 20: 190..209 */
      "You DISMEMBER $N with your UNBELIEVABLE #w!!!",
      "$n DISMEMBERS you with $s UNBELIEVABLE #w!!!"
    },

    {
      "$n ATOMIZES $N with $s DIVINE #w!!!!",			/* 21: 210+ */
      "You ATOMIZE $N with your DIVINE #w!!!!",
      "$n ATOMIZES you with $s DIVINE #w!!!!"
    }
  };

  static int msg_table[] = { 0,
/* 1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0 */
   1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7,  /* 1-20 */
   7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10,10,10,10,10,10,11, /* 21-40 */
   11,11,11,11,11,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13, /* 41-60 */
   14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15, /* 61-80 */
   15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16, /* 81-100 */
   16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17, /* 101-120 */
   17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,18,18, /* 121-140 */
   18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19, /* 141-160 */
   19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20, /* 161-180 */
   20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21, /* 181-200 */
   21,21,21,21,21,21,21,21,21,22}; /* 201-210 */

  w_type -= TYPE_HIT;		/* Change to base of table with text */

  if(dam>210)
    dam=210;

  msgnum=msg_table[dam];

  /* damage message to onlookers */
  buf = replace_string(dam_weapons[msgnum].to_room,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

  /* damage message to damager */
  send_to_char(CCYEL(ch, C_NRM), ch);
  buf = replace_string(dam_weapons[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);
  send_to_char(CCNRM(ch, C_NRM), ch);

  /* damage message to damagee */
  send_to_char(CCRED(victim, C_NRM), victim);
  buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(victim, C_NRM), victim);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int skill_message(int dam, struct char_data * ch, struct char_data * vict,
		      int attacktype)
{
  int i, j, nr;
  struct message_type *msg;

  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_HERO) && (!PRF_FLAGGED(vict, PRF_AVTR))) {
	act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
	if (GET_POS(vict) == POS_DEAD) {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {	/* Dam == 0 */
	send_to_char(CCYEL(ch, C_CMP), ch);
	act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	send_to_char(CCNRM(ch, C_CMP), ch);

	send_to_char(CCRED(vict, C_CMP), vict);
	act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	send_to_char(CCNRM(vict, C_CMP), vict);

	act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      }
      return 1;
    }
  }
  return 0;
}


void damage(struct char_data * ch, struct char_data * victim, int dam,
	    int attacktype)
{
  int exp, damagetype;
  struct affected_type af;
  extern int dam_type_spell[];
  extern int dam_type_skill[];
  extern int dam_type_npcspecial[];
  extern int dam_type_attack[];

  if(GET_HIT(victim) < -10)	/* To catch mob prog loops */
    return;

  if((GET_POS(victim) <= POS_DEAD) || (victim->in_room == NOWHERE)) {
    log("SYSERR: Attempt to damage a corpse.");
    return;			/* -je, 7/7/92 */
  }

  if((ch!=victim) && (!CAN_KILL(ch, victim))) {
    send_to_char("That wouldn't be very nice.\r\n", ch);
    return;
  }

  /* peaceful rooms */
  if ((ch != victim) && (!IS_NPC(ch)) && (!IS_NPC(victim)) && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  /* shopkeeper protection */
  if (!ok_damage_shopkeeper(ch, victim))
    return;

  /* You can't damage an immortal! */
  if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_HERO) && (!PRF_FLAGGED(victim, PRF_AVTR)))
    dam = 0;

  /* IMMs can't kill */
  if((!IS_NPC(ch)) && (GET_LEVEL(ch)>=LVL_HERO) && (GET_LEVEL(ch)<LVL_ASST) && (!GRNT_FLAGGED(ch, GRNT_KILL))) {
    send_to_char("Immortals are forbidden to kill!\r\n", ch);
    if(FIGHTING(ch))
      stop_fighting(ch);
    return;
  }

  if (victim != ch) {
    if (GET_POS(ch) > POS_STUNNED) {
      if (!(FIGHTING(ch)))
	set_fighting(ch, victim);

      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	  !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	if (FIGHTING(ch))
	  stop_fighting(ch);
	hit(ch, victim->master, TYPE_UNDEFINED);
	return;
      }
    }
    if (((GET_POS(victim) > POS_STUNNED)||((GET_POS(victim)==POS_STUNNED)&&GET_STUN(victim))) && !FIGHTING(victim)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
	  ((GET_LEVEL(ch) < LVL_HERO) || (!PRF_FLAGGED(ch, PRF_AVTR))))
	remember(victim, ch);
    }
  }

  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE))
    appear(ch);

  /* immunities/resistances/weaknesses */
  if(attacktype>=TYPE_SILENT) {
    switch(attacktype) {
    case TYPE_SFIRE:
      damagetype=DAMTYPE_FIRE;
      break;
    case TYPE_SICE:
      damagetype=DAMTYPE_ICE;
      break;
    case TYPE_SENERGY:
      damagetype=DAMTYPE_ENERGY;
      break;
    case TYPE_SBLUNT:
      damagetype=DAMTYPE_BLUNT;
      break;
    case TYPE_SSLASH:
      damagetype=DAMTYPE_SLASH;
      break;
    case TYPE_SPIERCE:
      damagetype=DAMTYPE_PIERCE;
      break;
    default:
      damagetype=DAMTYPE_NONE;
      break;
    }
  }
  else if(attacktype>0) {
    if(attacktype<=MAX_SPELLS) {
      damagetype=dam_type_spell[attacktype-1];
    }
    else if(attacktype<=MAX_SKILLS) {
      damagetype=dam_type_skill[attacktype-MAX_SPELLS-1];
    }
    else if(attacktype<=TOP_SPELL_DEFINE) {
      damagetype=dam_type_npcspecial[attacktype-MAX_SKILLS-1];
    }
    else {
      damagetype=dam_type_attack[attacktype-TOP_SPELL_DEFINE-1];
    }
  }
  else {
    damagetype=DAMTYPE_NONE;
  }

  if(ROOM_FLAGGED(victim->in_room, ROOM_STASIS)) {
    if(damagetype & (DAMTYPE_FIRE | DAMTYPE_ICE | DAMTYPE_ENERGY)) {
      dam=0;
    }
    else if(damagetype & DAMTYPE_BLUNT) {
      dam/=4;
    }
    else if(damagetype & DAMTYPE_SLASH) {
      dam/=3;
    }
    else if(damagetype & DAMTYPE_PIERCE) {
      dam/=2;
    }
  }

  if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
    if(dam>1)
      dam >>= 1;		/* 1/2 damage when sanctuary */
  }
  else if (IS_AFFECTED(victim, AFF_MAGICSHIELD) && (attacktype<=TOP_SPELL_DEFINE) && (attacktype>0) && ((attacktype>MAX_SKILLS) || (attacktype<=MAX_SPELLS))) {
    if(dam>1)
      dam >>= 1;
  }
  else if (IS_AFFECTED(victim, AFF_SPELLSHIELD) && (attacktype<=MAX_SPELLS) && (attacktype>0)) {
    if(dam>1)
      dam >>= 1;
  }
  else if (IS_AFFECTED(victim, AFF_ENERGY_CONT) && (damagetype & (DAMTYPE_FIRE | DAMTYPE_ICE | DAMTYPE_ENERGY))) {
    if((dam>1) && mag_savingthrow(victim, SAVING_PETRI)) {
      dam >>=1;
      af.type = SKILL_ENERGY_CONTAINMENT;
      af.bitvector = AFF_MAGIC_LIGHT;
      af.location = APPLY_NONE;
      af.duration = MAX(1, dam/60);
      af.modifier = 0;
      affect_join(victim, &af, TRUE, FALSE, FALSE, FALSE);
    }
  }

  /* poisonblade */
  if((ch!=victim) && (attacktype >= TYPE_HIT) && (GET_EQ(ch, WEAR_WIELD)&&(GET_EQ(ch, WEAR_WIELD)->poisoned>0))) {
    GET_EQ(ch, WEAR_WIELD)->poisoned--;
    if((number(1, 100) <= 15) && (!affected_by_spell(victim, SPELL_POISON)) && (!MOB_FLAGGED(victim, MOB_NOPOISON))) {
      af.type = SPELL_POISON;
      af.bitvector = AFF_POISON;
      af.modifier = -2;
      af.location = APPLY_STR;
      af.duration = 4;
      affect_join(victim, &af, FALSE, FALSE, FALSE, FALSE);
      act("Your blade poisons $N!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$N screams in pain as $n's wicked blade poisons $M!", FALSE, ch, NULL, victim, TO_NOTVICT);
      act("You scream in pain as $n's wicked blade poisons you!", FALSE, ch, NULL, victim, TO_VICT);
      dam *= 1.3;
    }
    else if(number(1, 100) <= 15) {
      act("Your blade cuts deeply into $N's flesh!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n's blade cuts deeply into $N's flesh!", FALSE, ch, NULL, victim, TO_NOTVICT);
      act("$n's blade cuts deeply into your flesh!", FALSE, ch, NULL, victim, TO_VICT);
      dam *= 1.15;
    }
  }

  if(damagetype)
    dam -= (dam*get_damage_reduction(victim, damagetype))/100;

  dam = MAX(dam, 0);

  GET_HIT(victim) -= dam;

  if ((ch != victim) && (!IS_NPC(ch)) && (IS_NPC(victim))) {
    exp = dam*MIN(GET_LEVEL(victim), GET_EXP(victim)/MAX(1, GET_MAX_HIT(victim)));
    gain_exp(ch, exp);
  }

  update_pos(victim);

  /*
   * skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   * 
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message.
   */
  if (!IS_WEAPON(attacktype)) {
    if(attacktype!=TYPE_SILENT)
      skill_message(dam, ch, victim, attacktype);
  }
  else {
    if (GET_POS(victim) == POS_DEAD || dam == 0) {
      if (!skill_message(dam, ch, victim, attacktype))
	dam_message(dam, ch, victim, attacktype);
    } else
      dam_message(dam, ch, victim, attacktype);
  }

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
    break;
  case POS_STUNNED:
    if(GET_STUN(victim) < 1) {
      act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
    }
    break;
  case POS_DEAD:
    act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You are dead!  Sorry...\r\n", victim);
    break;

  default:			/* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) >> 2))
      act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 2)) {
      sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
	      CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      send_to_char(buf2, victim);
      if (MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim) && (GET_STUN(victim) <= 0))
	do_flee(victim, "", 0, 0);
      if(victim->in_room == NOWHERE) /* died from fleeing */
        return;
    }
    if ((!IS_NPC(victim)) && GET_WIMP_LEV(victim) && (victim != ch) &&
	(GET_HIT(victim) < GET_WIMP_LEV(victim)) && (GET_STUN(victim) <= 0) &&
        victim->desc && (victim->desc->wait <= 0)) {
      send_to_char("You wimp out, and attempt to flee!\r\n", victim);
      do_flee(victim, "", 0, 0);
      if(victim->in_room == NOWHERE) /* died from fleeing */
        return;
    }
    break;
  }

  if (!IS_NPC(victim) && !(victim->desc) && (GET_HIT(victim)>0)) {
    do_flee(victim, "", 0, 0);
    if(victim->in_room == NOWHERE) /* died from fleeing */
      return;
    if (!FIGHTING(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = victim->in_room;
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }

  if ((MOB_FLAGGED(ch, MOB_NICE)) && (GET_POS(victim)<POS_STUNNED))
    if (FIGHTING(ch)==victim)
      stop_fighting(ch);

  if ((!AWAKE(victim)) && (!((GET_POS(victim) == POS_STUNNED) && (GET_STUN(victim) > 0))))
    if (FIGHTING(victim))
      stop_fighting(victim);

  if (GET_POS(victim) == POS_DEAD) {
    if((victim->in_room == 0) && (GET_WAS_IN(victim) > 0)) {
      char_from_room(victim);
      char_to_room(victim, GET_WAS_IN(victim));
      GET_WAS_IN(victim)=NOWHERE;
    }
    if((!IS_NPC(ch)) && (IS_NPC(victim))) {
      if (IS_AFFECTED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
      }
      else {
	exp = MIN(max_exp_gain, GET_EXP(victim));

	/* Calculate level-difference bonus */
        if(GET_LEVEL(victim) < GET_LEVEL(ch))
          exp -= (exp*(GET_LEVEL(ch)-GET_LEVEL(victim)))/XP_REDUCTION_RATE;

	exp = MAX(exp, 1);
	exp=gain_exp(ch, exp);
	if (exp > 1) {
	  sprintf(buf2, "You receive %d experience points.\r\n", exp);
	  send_to_char(buf2, ch);
	} else
	  send_to_char("You receive one lousy experience point.\r\n", ch);
	change_alignment(ch, victim);
        if(auto_save)
          if(!number(0, 3))
            save_char(ch, NOWHERE);
      }
    }
    if (!IS_NPC(victim)) {
      if(ARENA(victim).kill_mode) {
        sprintf(buf2, "%s (lvl %d) arena-killed by %s at %s [%d]", GET_NAME(victim), GET_LEVEL(victim),
                GET_NAME(ch), world[victim->in_room].name, world[victim->in_room].number);
        mudlog(buf2, CMP, LVL_HERO, TRUE);
      }
      else {
        sprintf(buf2, "%s (lvl %d) killed by %s at %s [%d]", GET_NAME(victim), GET_LEVEL(victim),
                GET_NAME(ch), world[victim->in_room].name, world[victim->in_room].number);
        mudlog(buf2, BRF, LVL_HERO, TRUE);
        sprintf(buf2, "%s (lvl %d) killed by %s at %s", GET_NAME(victim), GET_LEVEL(victim),
                GET_NAME(ch), world[victim->in_room].name);
        mortlog(buf2, BRF, LVL_HERO, FALSE);
      }
      if (MOB_FLAGGED(ch, MOB_MEMORY))
	forget(ch, victim);
    }
    if(IS_NPC(ch)) {
      stat_loss(victim); /* You don't lose a stat if killed by a player */
      save_char(victim, NOWHERE);
    }
    die(victim, ch);
  }
}



void hit(struct char_data * ch, struct char_data * victim, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int w_type, victim_ac, calc_thaco, dam, diceroll, i, dodge, bless;
  int roll_to_hit;

  extern struct str_app_type str_app[];

  int backstab_mult(int level);

  if((!ch)||(!victim))
    return;

  if (ch->in_room != victim->in_room) {
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

  if ((ch != victim) && (!IS_NPC(ch)) && (!IS_NPC(victim)) && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  if(type==TYPE_SECOND_WEAPON)
    wielded=ch->equipment[WEAR_HOLD];

  if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else {
    if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  }

  /* Calculate the raw armor including magic armor.  Lower AC is better. */

  calc_thaco = class_get_thac0(ch);

  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= 5*GET_HITROLL(ch);
  calc_thaco -= (int) ((GET_INT(ch) - 13) / 1.5);	/* Intelligence helps! */
  calc_thaco -= (int) ((GET_WIS(ch) - 13) / 1.5);	/* So does wisdom */
  diceroll = number(1, 100);

  victim_ac = compute_ac(victim);
  if(MOB_FLAGGED(ch, MOB_FLESH_EATER))
    victim_ac += victim->player_specials->saved.inherent_ac_apply/3;

  if((!IS_NPC(victim)) && (GET_SKILL(victim, SKILL_DODGE)) && ((get_skill(victim, SKILL_DODGE)/5) >= number(1, 100)))
    dodge=1;
  else
    dodge=0;

  roll_to_hit=victim_ac-calc_thaco;
  if((!IS_NPC(ch)) && (GET_NUM_CLASSES(ch) > 1)) {
    roll_to_hit/=GET_NUM_CLASSES(ch);
  }
  /* decide whether this is a hit or a miss */
  if (((((diceroll < 96) && AWAKE(victim)) &&
       ((diceroll < 6) || (diceroll > (victim_ac-calc_thaco))))) || (dodge)) {
    if(dodge) {
      act("You skillfully dodge an attack from $N.", FALSE, victim, 0, ch, TO_CHAR);
      act("$n skillfully dodges an attack from $N.", FALSE, victim, 0, ch, TO_NOTVICT);
      act("$n skillfully dodges your attack.", FALSE, victim, 0, ch, TO_VICT);
    }
    else {
      if (type == SKILL_BACKSTAB)
        damage(ch, victim, 0, SKILL_BACKSTAB);
      else if (type == SKILL_CIRCLE)
        damage(ch, victim, 0, SKILL_CIRCLE);
      else
        damage(ch, victim, 0, w_type);
    }
  } else {
    /* okay, we know the guy has been hit.  now calculate damage. */
    dam=0;
    if((!IS_NPC(ch))&&(GET_DAMROLL(ch) > MIN(50, GET_LEVEL(ch))))
      dam += MIN(50, GET_LEVEL(ch));
    else
      dam += GET_DAMROLL(ch);

    if(AFF_FLAGGED(ch, AFF_UN_HOLY))
      dam += (GET_LEVEL(ch)/20)+1;

    if (wielded) {
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
      /* Call weapon spec proc on a hit */
      if(GET_OBJ_SPEC(wielded))
        GET_OBJ_SPEC(wielded)(victim, wielded, -1, itoa(dam));
    }
    else {
      if (IS_NPC(ch)) {
	dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
      }
      else if (IS_SET(GET_CLASS_BITVECTOR(ch), MK_F)) {
        dam += dice(3+((GET_LEVEL(ch)-1)/10), 4);
      }
      else {
	dam += number(0, 2);	/* Max. 2 dam with bare hands */
      }
    }

    if(IS_NPC(ch) || (!(GET_CLASS_BITVECTOR(ch) & MK_F)))
      dam += str_app[STRENGTH_APPLY_INDEX(ch)].todam;

    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1.0 + ((POS_FIGHTING - GET_POS(victim)) / 3.0);
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    if(IS_NPC(ch) && (GET_CLASS(ch)==CLASS_UNDEAD)) {
      bless=0;
      for(i=0; i<NUM_WEARS; i++) {
        if(GET_EQ(victim, i)&&IS_OBJ_STAT(GET_EQ(victim, i), ITEM_BLESS)) {
          bless++;
        }
      }
      dam -= (bless*dam)/100;
    }

    dam = MAX(1, dam);		/* at least 1 hp damage min per hit */

    if(type == SKILL_DUAL_BACKSTAB) {
      if(GET_EQ(ch, WEAR_HOLD)&&(GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD))==ITEM_WEAPON)) {
        dam += dice(GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 1), GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2));
        if((!IS_NPC(ch))&&(GET_DAMROLL(ch) > MIN(50, GET_LEVEL(ch))))
          dam += MIN(50, GET_LEVEL(ch));
        else
          dam += GET_DAMROLL(ch);
      }
    }

    if ((type == SKILL_BACKSTAB)||(type == SKILL_DUAL_BACKSTAB)) {
      dam *= backstab_mult(GET_LEVEL(ch));
      damage(ch, victim, dam, SKILL_BACKSTAB);
    } else if (type == SKILL_CIRCLE) {
      dam *= 0.75*backstab_mult(GET_LEVEL(ch));
      damage(ch, victim, dam, SKILL_CIRCLE);
    } else
      damage(ch, victim, dam, w_type);
    /* call armor spec procs when hit */
    for(i=0; i<NUM_WEARS; i++) {
      if(i==WEAR_WIELD)
        continue;
      if(GET_EQ(victim, i)&&GET_OBJ_SPEC(GET_EQ(victim, i))) {
        GET_OBJ_SPEC(GET_EQ(victim, i))(ch, GET_EQ(victim, i), -1, itoa(dam));
      }
    }
  }
}



/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
  static char round_up=0;
  int i, num_attacks;
  struct char_data *ch;
  extern struct index_data *mob_index;

  void diag_char_to_char(struct char_data * i, struct char_data * ch);
  void mprog_hitprcnt_trigger(struct char_data *mob, struct char_data *ch);
  void mprog_fight_trigger(struct char_data *mob, struct char_data *ch);

  if(round_up)
    round_up=0;
  else
    round_up=1;

  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;

    if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
      stop_fighting(ch);
      continue;
    }

    if(GET_STUN(ch)) {
      GET_STUN(ch)--;
      continue;
    }

    if(GET_POS(ch) > POS_FIGHTING)
      GET_POS(ch) = POS_FIGHTING;

    if (IS_NPC(ch)) {
      num_attacks=(ch->mob_specials.attacks>>1)+(round_up*(ch->mob_specials.attacks%2));
    }
    else {
      num_attacks=1;
      if(number(1, 100) <= get_skill(ch, SKILL_DOUBLE))
        num_attacks++;
      if(number(1, 100) <= get_skill(ch, SKILL_TRIPLE))
        num_attacks++;
      if(number(1, 100) <= get_skill(ch, SKILL_QUAD))
        num_attacks++;
    }
    if(AFF_FLAGGED(ch, AFF_HASTE)&&(number(1, 100)<=85))
      num_attacks++;
    if(AFF_FLAGGED(ch, AFF_METAMORPHOSIS))
      num_attacks++;
    for(i=0; i<num_attacks; i++) {
      if(!(AFF_FLAGGED(ch, AFF_SLOW)&&(number(1, 100)<=SLOW_CHANCE)))
        hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }

    if(IS_SET(GET_CLASS_BITVECTOR(ch), RA_F)&&GET_EQ(ch, WEAR_HOLD)&&(GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD))==ITEM_WEAPON)) {
      if(!(AFF_FLAGGED(ch, AFF_SLOW)&&(number(1, 100)<=SLOW_CHANCE)))
        hit(ch, FIGHTING(ch), TYPE_SECOND_WEAPON);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), TH_F) && GET_EQ(ch, WEAR_HOLD) &&
            (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD))==ITEM_WEAPON) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3)==(TYPE_STAB-TYPE_HIT))) {
      if(!(AFF_FLAGGED(ch, AFF_SLOW)&&(number(1, 100)<=SLOW_CHANCE)))
        hit(ch, FIGHTING(ch), TYPE_SECOND_WEAPON);
    }

    if(IS_NPC(ch)&&(GET_MOB_WAIT(ch) > 0)) {
      GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
      if(GET_MOB_WAIT(ch) <= 0) {
        GET_MOB_WAIT(ch) = 0;
        if (GET_POS(ch) < POS_FIGHTING) {
          GET_POS(ch) = POS_FIGHTING;
          act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
        }
      }
      continue;
    }

    if(IS_NPC(ch) && (FIGHTING(ch))) {
      mprog_hitprcnt_trigger(ch, FIGHTING(ch));
      if(ch->in_room!=NOWHERE && (FIGHTING(ch)))
        mprog_fight_trigger(ch, FIGHTING(ch));
      if(ch->in_room!=NOWHERE && (FIGHTING(ch)))
        if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
          (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
    }
  }
  
  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;
    if(!IS_NPC(ch))
      diag_char_to_char(FIGHTING(ch), ch);
  }
}
