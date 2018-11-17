/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

/* ************************************************************************
*  Mob procs are called every pulse_mobile with cmd 0                     *
*  Room procs are called every pulse_room with cmd 0, and every time the  *
*  zone they are in resets with cmd -1                                    *
*  Item procs are called every pulse_item with cmd 0.                     *
*  Weapons are called with cmd -1 every time they hit                     *
*  Armor is called with cmd -1 every time it is hit with a physical attack*
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


/*   external vars  */
extern int top_of_zone_table;
extern int top_of_world;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];
extern struct char_data *combat_list;
extern struct int_app_type int_app[];
extern struct spell_info_type spell_info[];
extern struct psi_stat_chart psi_stats[];

/* extern functions */
extern int(*spec_proc_table[])(struct char_data *, void *, int, char *);
void add_follower(struct char_data * ch, struct char_data * leader);
void do_start(struct char_data * ch);
void roll_real_abils(struct char_data * ch);
void parse_action(char *to_act, char *act_desc, int type, int self);
ACMD(do_say);
ACMD(do_tell);
ACMD(do_move);
ACMD(do_bash);
ACMD(do_kick);
ACMD(do_berserk);
ACMD(do_hurl);
ACMD(do_drop);
ACMD(do_gen_comm);
ACMD(do_cast);


struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS+1];

extern char *spells[];

void sort_spells(void)
{
  int a, b, tmp;

  /* initialize array */
  for (a = 1; a < MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < MAX_SKILLS - 1; a++)
    for (b = a + 1; b < MAX_SKILLS; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
	tmp = spell_sort_info[a];
	spell_sort_info[a] = spell_sort_info[b];
	spell_sort_info[b] = tmp;
      }
}


char *how_good(int percent, int type)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 65)
    strcpy(buf, " (fair)");
  else if (percent <= 75)
    strcpy(buf, " (good)");
  else
    strcpy(buf, " (very good)");
  if((type==SKILL) && (percent>=SKILL_LEARNED_LEVEL))
    strcpy(buf, " (superb)");
  if((type==SPELL) && (percent>=SPELL_LEARNED_LEVEL))
    strcpy(buf, " (superb)");

  return (buf);
}

#define LEARNED(skill) ((skill>MAX_SPELLS)?SKILL_LEARNED_LEVEL:SPELL_LEARNED_LEVEL)
#define GAIN(ch) ((level_params[PRAC_MUL][(int)GET_CLASS(ch)]*(level_params[PRAC_BONUS][(int)GET_CLASS(ch)]+int_app[GET_INT(ch)].learn))/level_params[PRAC_DIV][(int)GET_CLASS(ch)])
#define SPLSKL(skill) ((skill>MAX_SPELLS)?"skill":"spell")


void list_skills(struct char_data * ch)
{
  extern char *spells[];
  int i, sortpos;
  char buf2[MAX_STRING_LENGTH+200];

  int mag_manacost(struct char_data * ch, int spellnum);

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf, "%sYou know of the following skills:\r\n", buf);

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos < MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if (can_practice(ch, i)) {
      sprintf(buf, "%-20s Level %3d  %12s", spells[i], get_multi_skill_level(ch, i), how_good(GET_SKILL(ch, i), ((i>MAX_SPELLS)?SKILL:SPELL)));
      strcat(buf2, buf);
      if(i>MAX_SPELLS) {
        if((i<PSI_START) || (i>PSI_END)) {
          strcat(buf2, "\r\n");
        }
        else {
          switch(psi_stats[i-PSI_START].abil) {
          case PSI_STR:
            strcpy(buf1, "str");
            break;
          case PSI_INT:
            strcpy(buf1, "int");
            break;
          case PSI_WIS:
            strcpy(buf1, "wis");
            break;
          case PSI_DEX:
            strcpy(buf1, "dex");
            break;
          case PSI_CON:
            strcpy(buf1, "con");
            break;
          case PSI_CHA:
            strcpy(buf1, "cha");
            break;
          }
          sprintf(buf, "  %s%s%d, %3dm %3dh %3dv\r\n", buf1, ((psi_stats[i-PSI_START].mod==0)?"-":""),
                  psi_stats[i-PSI_START].mod, psi_stats[i-PSI_START].mana,
                  psi_stats[i-PSI_START].hp, psi_stats[i-PSI_START].move);
          strcat(buf2, buf);
        }
      }
      else {
        if((i==SPELL_REMEMBER)&&IS_SET(GET_CLASS_BITVECTOR(ch), PS_F))
          strcpy(buf, "\r\n");
        else
          sprintf(buf, "  %d mana\r\n", mag_manacost(ch, i));
        strcat(buf2, buf);
      }
    }
    else if((get_multi_skill_level(ch, i) < LVL_HERO) && (spell_info[i].prac || (GET_SKILL(ch, i)))) {
      sprintf(buf, "%-20s Level %3d\r\n", spells[i], get_multi_skill_level(ch, i));
      strcat(buf2, buf);
    }
  }

  page_string(ch->desc, buf2, 1);
}


SPECIAL(guild)
{
  int skill_num, percent;

  if ((!ch) || IS_NPC(ch) || !CMD_IS("practice"))
    return 0;

  skip_spaces(&argument);

  if (!*argument) {
    list_skills(ch);
    return 1;
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return 1;
  }

  skill_num = find_skill_num(argument);

  if ((skill_num < 1) || (!can_practice(ch, skill_num))) {
    sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(skill_num));
    send_to_char(buf, ch);
    return 1;
  }
  if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num)) {
    send_to_char("You are already learned in that area.\r\n", ch);
    return 1;
  }
  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;
  ch->player_specials->saved.extra_pracs--;

  percent = GET_SKILL(ch, skill_num);
  percent += GAIN(ch);

  SET_SKILL(ch, skill_num, MIN(LEARNED(skill_num), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num))
    send_to_char("You are now learned in that area.\r\n", ch);

  return 1;
}


SPECIAL(retired_guildmaster)
{
  if(CMD_IS("practice")) {
    act("The Guildmaster says, 'I will teach no more. The gods have forsaken us.'", FALSE, (struct char_data *)me, NULL, NULL, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(mayor)
{
  ACMD(do_gen_door);

  static char open_path[] =
  "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
  "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
    do_gen_door(ch, "gate", 0, SCMD_OPEN);
    break;

  case 'C':
    do_gen_door(ch, "gate", 0, SCMD_CLOSE);
    do_gen_door(ch, "gate", 0, SCMD_LOCK);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */


void npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_HERO)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


SPECIAL(snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if(number(0,3))
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 42 - GET_LEVEL(ch)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_ATTACK);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_HERO) && (!number(0, 4))) {
      npc_steal(ch, cons);
      return TRUE;
    }
  return FALSE;
}


struct char_data *random_victim(struct char_data *ch)
{
  struct char_data *targets[25];
  struct char_data *tch;
  int num=0;

  for(tch=combat_list; tch&&(num<25); tch=tch->next_fighting) {
    if((ch==FIGHTING(tch))&&((GET_LEVEL(tch)<LVL_HERO) || PRF_FLAGGED(tch, PRF_AVTR))&&(tch->in_room==ch->in_room)) {
      targets[num++]=tch;
    }
  }

  if(num)
    return(targets[number(0, num-1)]);
  else
    return(NULL);
}


SPECIAL(magic_user)
{
  struct char_data *vict;
  int done=0;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  if ((GET_LEVEL(ch) > 13) && (number(0, 10) == 0)) {
    cast_spell(ch, vict, NULL, SPELL_SLEEP, 0);
    done=1;
  }

  if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0)) {
    cast_spell(ch, vict, NULL, SPELL_BLINDNESS, 0);
    done=1;
  }

  if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
    cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN, 0);
    done=1;
  }
  if (done)
    return TRUE;

  switch (GET_LEVEL(ch)) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
    cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE, 0);
    break;
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
    cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS, 0);
    break;
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH, 0);
    break;
  case 25:
  case 26:
  case 27:
  case 28:
    cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP, 0);
    break;
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
    cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY, 0);
    break;
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
    cast_spell(ch, vict, NULL, SPELL_CONE_OF_COLD, 0);
    break;
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47:
  case 48:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT, 0);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL, 0);
    break;
  }
  return TRUE;

}


SPECIAL(breath_fire)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  return(call_magic(ch, vict, NULL, FIRE_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
}


SPECIAL(breath_acid)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  return(call_magic(ch, vict, NULL, ACID_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
}


SPECIAL(breath_frost)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  return(call_magic(ch, vict, NULL, FROST_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
}


SPECIAL(breath_gas)
{
  if (cmd || (GET_POS(ch) != POS_FIGHTING) || (!FIGHTING(ch)))
    return FALSE;

  if(number(0,3))
    return FALSE;

  return(call_magic(ch, NULL, NULL, GAS_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
}


SPECIAL(breath_lightning)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  return(call_magic(ch, vict, NULL, LIGHTNING_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
}


SPECIAL(breath_any)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING) || (!FIGHTING(ch)))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  switch(number(0, 4)) {
  case 0:
    return(call_magic(ch, vict, NULL, FIRE_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
    break;
  case 1:
    return(call_magic(ch, vict, NULL, ACID_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
    break;
  case 2:
    return(call_magic(ch, vict, NULL, FROST_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
    break;
  case 3:
    return(call_magic(ch, NULL, NULL, GAS_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
    break;
  case 4:
    return(call_magic(ch, vict, NULL, LIGHTNING_BREATH, GET_LEVEL(ch), CAST_UNDEFINED));
    break;
  }
  return FALSE;
}


SPECIAL(cleric)
{
  struct char_data *vict;
  int min_level=1000, spell=SPELL_BLINDNESS;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  while(GET_LEVEL(ch) < min_level) {
    switch(number(1, 10)) {
      case 1:
      case 2: min_level=0; spell=SPELL_BLINDNESS; break;
      case 3:
      case 4:
      case 5: min_level=7; spell=SPELL_EARTHQUAKE; break;
      case 6:
      case 7:
      case 8: min_level=10;
              if(IS_EVIL(ch)) spell=SPELL_DISPEL_GOOD;
              else if(IS_GOOD(ch)) spell=SPELL_DISPEL_EVIL;
              else spell=SPELL_CURSE;
              break;
      case 9: min_level=12; spell=SPELL_CURSE; break;
      case 10: min_level=15; spell=SPELL_HARM; break;
    }
  }

  cast_spell(ch, vict, NULL, spell, 0);

  return TRUE;

}


SPECIAL(undead)
{
  struct char_data *vict;
  int spell=SPELL_CHILL_TOUCH;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=random_victim(ch);

  if(!vict)
    return FALSE;

  switch(number(1, 6)) {
    case 1: spell=SPELL_CURSE; break;
    case 2: spell=SPELL_CHILL_TOUCH; break;
    case 3: spell=SPELL_BLINDNESS; break;
    case 4: spell=SPELL_POISON; break;
    case 5: spell=SPELL_ENERGY_DRAIN; break;
    case 6: spell=SPELL_HARM; break;
  }

  cast_spell(ch, vict, NULL, spell, 0);

  return TRUE;

}


SPECIAL(spec_bash)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=FIGHTING(ch);

  if(!vict)
    return FALSE;

  do_bash(ch, GET_NAME(vict), 0, 0);

  return TRUE;
}


SPECIAL(spec_kick)
{
  struct char_data *vict;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=FIGHTING(ch);

  if(!vict)
    return FALSE;

  do_kick(ch, GET_NAME(vict), 0, 0);

  return TRUE;
}


SPECIAL(spec_berserk)
{
  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  do_berserk(ch, "", 0, 0);

  return TRUE;
}


SPECIAL(spec_warrior)
{
  struct char_data *vict, *tch;
  int num=0;

  if (cmd || (GET_POS(ch) != POS_FIGHTING))
    return FALSE;

  if(number(0,3))
    return FALSE;

  vict=FIGHTING(ch);

  if(!vict)
    return FALSE;

  for(tch=combat_list; tch&&(num < 3); tch=tch->next_fighting) {
    if((ch==FIGHTING(tch))&&(ch->in_room==tch->in_room))
      num++;
  }

  if((GET_LEVEL(ch) >= 15) && (num > 2))
    do_berserk(ch, "", 0, 0);
  else if(number(0, 1))
    do_hurl(ch, GET_NAME(vict), 0, 0);
  else
    do_bash(ch, GET_NAME(vict), 0, 0);

  return TRUE;
}


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

SPECIAL(guild_guard)
{
  int i;
  extern int guild_info[][3];
  struct char_data *guard = (struct char_data *) me;
  char *buf = "The guard humiliates you, and blocks your way.\r\n";
  char *buf2 = "The guard humiliates $n, and blocks $s way.";

  if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND))
    return FALSE;

  if ((GET_LEVEL(ch) >= LVL_HERO) && (!IS_NPC(ch)))
    return FALSE;

  for (i = 0; guild_info[i][0] != -1; i++) {
    if ((IS_NPC(ch) || (!(GET_CLASS_BITVECTOR(ch) & (1 << guild_info[i][0])))) &&
	world[ch->in_room].number == guild_info[i][1] &&
	cmd == guild_info[i][2]) {
      send_to_char(buf, ch);
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }
  }

  return FALSE;
}


SPECIAL(puff)
{
  ACMD(do_say);

  if (cmd)
    return (0);

  switch (number(0, 60)) {
  case 0:
    do_say(ch, "My god!  It's full of stars!", 0, 0);
    return (1);
  case 1:
    do_say(ch, "How'd all those fish get up here?", 0, 0);
    return (1);
  case 2:
    do_say(ch, "I'm a very female dragon.", 0, 0);
    return (1);
  case 3:
    do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
    return (1);
  default:
    return (0);
  }
}


SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}


SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) > 10)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return FALSE;

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}


SPECIAL(statistician)
{
  struct char_data *keeper=(struct char_data *)me;
  struct affected_type *af;
  char buf[128];
  int i, j;

  if((cmd < 1) || IS_NPC(ch))
    return FALSE;

  if(CMD_IS("list")) {
    sprintf(buf, "%s You can buy maxhit, maxmana, and maxmove\r\nat 1 practice per point.", GET_NAME(ch));
    do_tell(keeper, buf, 0, 0);
    sprintf(buf, "%s You can buy str, int, wis, dex, con, and cha\r\nat 20 practices per point.", GET_NAME(ch));
    do_tell(keeper, buf, 0, 0);
    sprintf(buf, "%s You can sell maxhit, maxmana, and maxmove\r\nat 2 points for 1 practice.", GET_NAME(ch));
    do_tell(keeper, buf, 0, 0);
    sprintf(buf, "%s You can sell quest points at 2 quest points\r\nfor 3 practices.", GET_NAME(ch));
    do_tell(keeper, buf, 0, 0);
    return TRUE;
  }
  if(CMD_IS("buy")) {
    skip_spaces(&argument);
    one_argument(argument, buf);

    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
                        GET_EQ(ch, i)->affected[j].modifier,
                        GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
    }
    for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    if(!*buf) {
      sprintf(buf, "%s Buy what?", GET_NAME(ch));
      do_tell(keeper, buf, 0, 0);
    }
    else if(!strn_cmp(buf, "maxhit", strlen(buf))) {
      if(GET_PRACTICES(ch)<1) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else if(GET_MAX_HIT(ch) >= 1500) {
        sprintf(buf, "%s Your maxhit is already as high as it can get!", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)-=1;
        GET_MAX_HIT(ch)+=1;
        ch->player_specials->saved.old_hit++;
        ch->player_specials->saved.new_hit++;
        sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else if(!strn_cmp(buf, "maxmana", strlen(buf))) {
      if(GET_PRACTICES(ch)<1) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else if(GET_MAX_MANA(ch) >= 1500) {
        sprintf(buf, "%s Your maxmana is already as high as it can get!", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)-=1;
        GET_MAX_MANA(ch)+=1;
        ch->player_specials->saved.old_mana++;
        ch->player_specials->saved.new_mana++;
        sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else if(!strn_cmp(buf, "maxmove", strlen(buf))) {
      if(GET_PRACTICES(ch)<1) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else if(GET_MAX_MOVE(ch) >= 1500) {
        sprintf(buf, "%s Your maxmove is already as high as it can get!", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)-=1;
        GET_MAX_MOVE(ch)+=1;
        ch->player_specials->saved.old_move++;
        ch->player_specials->saved.new_move++;
        sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else if(!strn_cmp(buf, "strength", strlen(buf))) {
      if(GET_PRACTICES(ch)<20) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        if(ch->real_abils.str>17) {
          if(ch->real_abils.str_add==100) {
            sprintf(buf, "%s Your strength is already as high as it can get!", GET_NAME(ch));
            do_tell(keeper, buf, 0, 0);
          }
          else {
            ch->real_abils.str_add+=10;
            if(ch->real_abils.str_add>100)
              ch->real_abils.str_add=100;
            GET_PRACTICES(ch)-=20;
            sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
            do_tell(keeper, buf, 0, 0);
          }
        }
        else {
          GET_PRACTICES(ch)-=20;
          ch->real_abils.str+=1;
          sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
      }
    }
    else if(!strn_cmp(buf, "intelligence", strlen(buf))) {
      if(GET_PRACTICES(ch)<20) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        if(ch->real_abils.intel>17) {
          sprintf(buf, "%s Your intelligence is already as high as it can get!", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
        else {
          GET_PRACTICES(ch)-=20;
          ch->real_abils.intel+=1;
          sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
      }
    }
    else if(!strn_cmp(buf, "wisdom", strlen(buf))) {
      if(GET_PRACTICES(ch)<20) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        if(ch->real_abils.wis>17) {
          sprintf(buf, "%s Your wisdom is already as high as it can get!", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
        else {
          GET_PRACTICES(ch)-=20;
          ch->real_abils.wis+=1;
          sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
      }
    }
    else if(!strn_cmp(buf, "dexterity", strlen(buf))) {
      if(GET_PRACTICES(ch)<20) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        if(ch->real_abils.dex>17) {
          sprintf(buf, "%s Your dexterity is already as high as it can get!", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
        else {
          GET_PRACTICES(ch)-=20;
          ch->real_abils.dex+=1;
          sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
      }
    }
    else if(!strn_cmp(buf, "constitution", strlen(buf))) {
      if(GET_PRACTICES(ch)<20) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        if(ch->real_abils.con>17) {
          sprintf(buf, "%s Your constitution is already as high as it can get!", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
        else {
          GET_PRACTICES(ch)-=20;
          ch->real_abils.con+=1;
          sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
      }
    }
    else if(!strn_cmp(buf, "charisma", strlen(buf))) {
      if(GET_PRACTICES(ch)<20) {
        sprintf(buf, "%s You do not have enough practices to buy that.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        if(ch->real_abils.cha>17) {
          sprintf(buf, "%s Your charisma is already as high as it can get!", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
        else {
          GET_PRACTICES(ch)-=20;
          ch->real_abils.cha+=1;
          sprintf(buf, "%s Enjoy your new abilities.", GET_NAME(ch));
          do_tell(keeper, buf, 0, 0);
        }
      }
    }
    else {
      sprintf(buf, "%s That is not a stat you can buy.", GET_NAME(ch));
      do_tell(keeper, buf, 0, 0);
    }

    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
                        GET_EQ(ch, i)->affected[j].modifier,
                        GET_EQ(ch, i)->obj_flags.bitvector, TRUE);
    }
    for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    affect_total(ch);
    save_char(ch, NOWHERE);
    return TRUE;
  }
  if(CMD_IS("sell")) {
    skip_spaces(&argument);
    one_argument(argument, buf);

    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
                        GET_EQ(ch, i)->affected[j].modifier,
                        GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
    }
    for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    if(!*buf) {
      sprintf(buf, "%s Sell what?", GET_NAME(ch));
      do_tell(keeper, buf, 0, 0);
    }
    else if(!strn_cmp(buf, "maxhit", strlen(buf))) {
      if(GET_MAX_HIT(ch)<202) {
        sprintf(buf, "%s Sorry, I can't drain you any lower.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)+=1;
        GET_MAX_HIT(ch)-=2;
        ch->player_specials->saved.old_hit-=2;
        ch->player_specials->saved.new_hit-=2;
        sprintf(buf, "%s Make good use of that practice point.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else if(!strn_cmp(buf, "maxmana", strlen(buf))) {
      if(GET_MAX_MANA(ch)<102) {
        sprintf(buf, "%s Sorry, I can't drain you any lower.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)+=1;
        GET_MAX_MANA(ch)-=2;
        ch->player_specials->saved.old_mana-=2;
        ch->player_specials->saved.new_mana-=2;
        sprintf(buf, "%s Make good use of that practice point.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else if(!strn_cmp(buf, "maxmove", strlen(buf))) {
      if(GET_MAX_MOVE(ch)<82) {
        sprintf(buf, "%s Sorry, I can't drain you any lower.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)+=1;
        GET_MAX_MOVE(ch)-=2;
        ch->player_specials->saved.old_move-=2;
        ch->player_specials->saved.new_move-=2;
        sprintf(buf, "%s Make good use of that practice point.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else if(!strn_cmp(buf, "questpoints", strlen(buf))) {
      if(GET_QP(ch)<2) {
        sprintf(buf, "%s Sorry, you don't have enough quest points.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
      else {
        GET_PRACTICES(ch)+=3;
        GET_QP(ch)-=2;
        sprintf(buf, "%s Make good use of those practice points.", GET_NAME(ch));
        do_tell(keeper, buf, 0, 0);
      }
    }
    else {
      sprintf(buf, "%s That is not a stat you can sell.", GET_NAME(ch));
      do_tell(keeper, buf, 0, 0);
    }

    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
                        GET_EQ(ch, i)->affected[j].modifier,
                        GET_EQ(ch, i)->obj_flags.bitvector, TRUE);
    }
    for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    affect_total(ch);
    save_char(ch, NOWHERE);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(repair_shop)
{
  struct obj_data *o;
  struct char_data *smith=(struct char_data *)me;
  char buf[MAX_INPUT_LENGTH];
  long i, cost;

  if((!CMD_IS("appraise"))&&(!CMD_IS("repair")))
    return FALSE;

  skip_spaces(&argument);
  if(!*argument) {
    do_say(smith, "What item did you have in mind?", 0, 0);
    return TRUE;
  }

  o=get_obj_in_list_vis(ch, argument, ch->carrying);
  if(!o) {
    sprintf(buf, "You don't seem to have %s %s", (strchr("aeiouyAEIOUY", *argument) ? "an" : "a"), argument);
    send_to_char(buf, ch);
    return TRUE;
  }

  if((GET_OBJ_TYPE(o)!=ITEM_ARMOR) && (GET_OBJ_TYPE(o)!=ITEM_DAMAGEABLE)) {
    do_say(smith, "I can't repair that.", 0, 0);
    return TRUE;
  }

  if((GET_OBJ_TYPE(obj_proto+o->item_number)!=ITEM_ARMOR) && (GET_OBJ_TYPE(obj_proto+o->item_number)!=ITEM_DAMAGEABLE)) {
    do_say(smith, "I'm sorry, there is nothing I can do with that item.", 0, 0);
    return TRUE;
  }

  if(GET_OBJ_TYPE(o)==ITEM_ARMOR) {
    i=GET_OBJ_VAL(obj_proto+o->item_number, 0)-GET_OBJ_VAL(o, 0);
    cost=(((i+1)*i)/2)*1000000;
  }
  else {
    i=GET_OBJ_VAL(o, 2);
    cost=((((i*(i-1))/2)*5000)+i*10000)*GET_OBJ_VAL(o, 0);
  }

  if(i<1) {
    do_say(smith, "That item is in top shape.", 0, 0);
    return TRUE;
  }

  if(CMD_IS("appraise")) {
    sprintf(buf, "%s It will cost you %ld gold to repair that item.", GET_NAME(ch), cost);
    do_tell(smith, buf, 0, 0);
    return TRUE;
  }
  else {
    if(GET_GOLD(ch) < cost) {
      sprintf(buf, "%s Sorry, it costs %ld gold to repair that, which you don't have.", GET_NAME(ch), cost);
      do_tell(smith, buf, 0, 0);
    }
    else {
      sprintf(buf, "%s That will cost you %ld gold.", GET_NAME(ch), cost);
      do_tell(smith, buf, 0, 0);
      GET_GOLD(ch)-=cost;
      if(GET_OBJ_TYPE(o)==ITEM_ARMOR) {
        GET_OBJ_VAL(o, 0)=GET_OBJ_VAL(obj_proto+o->item_number, 0);
      }
      else {
        GET_OBJ_VAL(o, 2)=0;
        GET_OBJ_VAL(o, 1)=0;
        for(i=0; i < MAX_OBJ_AFFECT; i++) {
          o->affected[i].modifier = obj_proto[GET_OBJ_RNUM(o)].affected[i].modifier;
        }
      }
      do_say(smith, "There, as good as new.", 0, 0);
    }
    return TRUE;
  }

  return FALSE;
}


SPECIAL(well_of_mystery)
{
  int i;
  long cost;
  struct char_data *mage = (struct char_data *)me;
  struct obj_data *obj;
  char buf[MAX_INPUT_LENGTH];

  if((cmd==0) && me && FIGHTING(mage)) {
    return(magic_user(ch, me, cmd, argument));
  }
  else if(me && CMD_IS("recharge")) {
    skip_spaces(&argument);
    one_argument(argument, buf);
    if((obj=get_obj_in_list_vis(ch, buf, ch->carrying))) {
      if((GET_OBJ_TYPE(obj)==ITEM_WAND) || (GET_OBJ_TYPE(obj)==ITEM_STAFF)) {
        if(GET_OBJ_VAL(obj, 1) < 2) {
          sprintf(buf, "%s Sorry, this can't survive another recharge.", GET_NAME(ch));
          do_tell(mage, buf, 0, 0);
          return TRUE;
        }
        i=MAX(1, GET_OBJ_VAL(obj, 1)/4);
        cost = MAX(1, GET_OBJ_VAL(obj, 1)-i-GET_OBJ_VAL(obj, 2));
        cost *= 1800;
        cost *= GET_OBJ_VAL(obj, 0);
        if(GET_GOLD(ch) < cost) {
          sprintf(buf, "%s Sorry, you don't have the %ld coins that service will cost.", GET_NAME(ch), cost);
          do_tell(mage, buf, 0, 0);
          return TRUE;
        }
        GET_OBJ_VAL(obj, 1) -= i;
        GET_OBJ_VAL(obj, 2) = GET_OBJ_VAL(obj, 1);
        GET_GOLD(ch) -= cost;
        sprintf(buf, "%s That will be %ld coins.", GET_NAME(ch), cost);
        do_tell(mage, buf, 0, 0);
        act("$N takes $p from you and plunges it into\r\n"
            "the Well of Mystery with her bare hands. Blinding light flares as she\r\n"
            "speaks words you can't understand. When your vision clears $N\r\n"
            "is facing you, unscathed, holding $p out to you.",
            FALSE, ch, obj, mage, TO_CHAR);
        act("$N takes $p from $n and plunges it into\r\n"
            "the Well of Mystery with her bare hands. Blinding light flares as she\r\n"
            "speaks words you can't understand. When your vision clears $N\r\n"
            "is facing $n, unscathed, holding $p out to $m.",
            FALSE, ch, obj, mage, TO_ROOM);
      }
      else {
        sprintf(buf, "%s I can't recharge that!", GET_NAME(ch));
        do_tell(mage, buf, 0, 0);
      }
    }
    else {
      sprintf(buf, "%s Sorry, you don't have anything like that.", GET_NAME(ch));
      do_tell(mage, buf, 0, 0);
    }
    return TRUE;
  }
  else if(me && CMD_IS("uninvis")) {
    skip_spaces(&argument);
    one_argument(argument, buf);
    if((obj=get_obj_in_list_vis(ch, buf, ch->carrying))) {
      if(IS_OBJ_STAT(obj, ITEM_INVISIBLE)) {
        if((GET_OBJ_RNUM(obj) >= 0) && IS_OBJ_STAT(&obj_proto[GET_OBJ_RNUM(obj)], ITEM_INVISIBLE))
          cost=1405000L;
        else
          cost=125000L;
        if(GET_GOLD(ch) < cost) {
          sprintf(buf, "%s Sorry, you don't have the %ld coins that service will cost.", GET_NAME(ch), cost);
          do_tell(mage, buf, 0, 0);
          return TRUE;
        }
        REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_INVISIBLE);
        GET_GOLD(ch) -= cost;
        sprintf(buf, "%s That will be %ld coins.", GET_NAME(ch), cost);
        do_tell(mage, buf, 0, 0);
        act("$N takes $p from you and sets it on a cloth\r\n"
            "on the shelf. She takes a black ladle off the shelf, dips it into the\r\n"
            "Well of Mystery, and pours it onto $p. You can't see anything\r\n"
            "in the ladle, but $p shimmers a little. $N\r\n"
            "wraps $p in the cloth and hits it against the side of the well. When\r\n"
            "she unwraps it, it is visible. She picks $p\r\n"
            "up and hands it back to you.",
            FALSE, ch, obj, mage, TO_CHAR);
        act("$N takes $p from $n and sets it on a cloth\r\n"
            "on the shelf. She takes a black ladle off the shelf, dips it into the\r\n"
            "Well of Mystery, and pours it onto $p. You can't see anything\r\n"
            "in the ladle, but $p shimmers a little. $N\r\n"
            "wraps $p in the cloth and hits it against the side of the well. When\r\n"
            "she unwraps it, it is visible. She picks $p\r\n"
            "up and hands it back to $n.",
            FALSE, ch, obj, mage, TO_ROOM);
      }
      else {
        sprintf(buf, "%s It isn't invisible!", GET_NAME(ch));
        do_tell(mage, buf, 0, 0);
      }
    }
    else {
      sprintf(buf, "%s Sorry, you don't have anything like that.", GET_NAME(ch));
      do_tell(mage, buf, 0, 0);
    }
    return TRUE;
  }
  else {
    return FALSE;
  }
}


SPECIAL(recharge_shop)
{
  int i;
  long cost;
  struct char_data *mage = (struct char_data *)me;
  struct obj_data *obj;
  char buf[MAX_INPUT_LENGTH];

  if((cmd==0) && me && FIGHTING(mage)) {
    return(magic_user(ch, me, cmd, argument));
  }
  else if(me && CMD_IS("recharge")) {
    skip_spaces(&argument);
    one_argument(argument, buf);
    if((obj=get_obj_in_list_vis(ch, buf, ch->carrying))) {
      if((GET_OBJ_TYPE(obj)==ITEM_WAND) || (GET_OBJ_TYPE(obj)==ITEM_STAFF)) {
        if(GET_OBJ_VAL(obj, 1) < 2) {
          sprintf(buf, "%s Sorry, this can't survive another recharge.", GET_NAME(ch));
          do_tell(mage, buf, 0, 0);
          return TRUE;
        }
        i=MAX(1, GET_OBJ_VAL(obj, 1)/4);
        cost = MAX(1, GET_OBJ_VAL(obj, 1)-i-GET_OBJ_VAL(obj, 2));
        cost *= 1800;
        cost *= GET_OBJ_VAL(obj, 0);
        if(GET_GOLD(ch) < cost) {
          sprintf(buf, "%s Sorry, you don't have the %ld coins that service will cost.", GET_NAME(ch), cost);
          do_tell(mage, buf, 0, 0);
          return TRUE;
        }
        GET_OBJ_VAL(obj, 1) -= i;
        GET_OBJ_VAL(obj, 2) = GET_OBJ_VAL(obj, 1);
        GET_GOLD(ch) -= cost;
        sprintf(buf, "%s That will be %ld coins.", GET_NAME(ch), cost);
        do_tell(mage, buf, 0, 0);
      }
      else {
        sprintf(buf, "%s I can't recharge that!", GET_NAME(ch));
        do_tell(mage, buf, 0, 0);
      }
    }
    else {
      sprintf(buf, "%s Sorry, you don't have anything like that.", GET_NAME(ch));
      do_tell(mage, buf, 0, 0);
    }
    return TRUE;
  }
  else {
    return FALSE;
  }
}


SPECIAL(quest_shop)
{
  int i, j, k, bottom, top, cost=0, ok;
  struct obj_data *obj=NULL;

  ASPELL(spell_identify);

  if(!CMD_IS("buy") && !CMD_IS("list") && !CMD_IS("identify") && !CMD_IS("sell"))
    return FALSE;

  if((!ch) || (!me))
    return FALSE;

  bottom=zone_table[i=real_zone(QUEST_ZONE)].bottom;
  top=zone_table[i].top;

  if(CMD_IS("buy")) {
    one_argument(argument, arg);
    if((!*arg) || (!is_number(arg))) {
      send_to_char("You must specify an item number.\r\n", ch);
      return TRUE;
    }
    j=atoi(arg);
    if(j < 1) {
      send_to_char("No such item.\r\n", ch);
      return TRUE;
    }
    for(i=bottom, k=1; i <= top; i++) {
      if(real_object(i) > -1) {
        if(j==k) {
          j=real_object(i);
          cost=GET_OBJ_COST(&obj_proto[j]);
          break;
        }
        k++;
      }
    }
    if(i > top) {
      send_to_char("No such item.\r\n", ch);
      return TRUE;
    }
    if(GET_QP(ch) < cost) {
      sprintf(buf, "You can't afford that! It costs %d quest points, you only have %d.\r\n", cost, GET_QP(ch));
      send_to_char(buf, ch);
      return TRUE;
    }
    if(cost > 0) {
      obj=read_object(j, REAL);
      obj_to_char(obj, ch);
      GET_QP(ch) -= cost;
      act("You buy $p.", FALSE, ch, obj, NULL, TO_CHAR);
      act("$n buys $p.", FALSE, ch, obj, NULL, TO_ROOM);
    }
    else {
      send_to_char("Invalid cost.\r\n", ch);
    }
    return TRUE;
  }
  else if(CMD_IS("list")) {
    one_argument(argument, arg);
    if(!*arg) {
      arg[0]='*';
      arg[1]=0;
    }
    if(!strn_cmp("protection", arg, strlen(arg))) {
      buf[0]=0;
      for(i=bottom, k=1; i <= top; i++) {
        if((j=real_object(i)) > -1) {
          obj=&obj_proto[j];
          ok=0;
          for(j=0; j<MAX_OBJ_AFFECT; j++) {
            if((obj->affected[j].location==APPLY_AC) ||
               (obj->affected[j].location==APPLY_MR) ||
               (obj->affected[j].location==APPLY_PR) ||
               (GET_OBJ_TYPE(obj)==ITEM_ARMOR))
              ok=1;
          }
          if(ok) {
            sprintf(buf1, "%3d) %-59s  %4d\r\n", k, obj->short_description, GET_OBJ_COST(obj));
            strcat(buf, buf1);
          }
          k++;
        }
      }
      page_string(ch->desc, buf, 1);
    }
    else if(!strn_cmp("hitter", arg, strlen(arg))) {
      buf[0]=0;
      for(i=bottom, k=1; i <= top; i++) {
        if((j=real_object(i)) > -1) {
          obj=&obj_proto[j];
          ok=0;
          for(j=0; j<MAX_OBJ_AFFECT; j++) {
            if((obj->affected[j].location==APPLY_HITROLL) ||
               (obj->affected[j].location==APPLY_DAMROLL))
              ok=1;
          }
          if(ok) {
            sprintf(buf1, "%3d) %-59s  %4d\r\n", k, obj->short_description, GET_OBJ_COST(obj));
            strcat(buf, buf1);
          }
          k++;
        }
      }
      page_string(ch->desc, buf, 1);
    }
    else if(!strn_cmp("mana", arg, strlen(arg))) {
      buf[0]=0;
      for(i=bottom, k=1; i <= top; i++) {
        if((j=real_object(i)) > -1) {
          obj=&obj_proto[j];
          ok=0;
          for(j=0; j<MAX_OBJ_AFFECT; j++) {
            if((obj->affected[j].location==APPLY_AGE) ||
               (obj->affected[j].location==APPLY_MANA) ||
               (obj->affected[j].location==APPLY_MANA_REGEN))
              ok=1;
          }
          if(ok) {
            sprintf(buf1, "%3d) %-59s  %4d\r\n", k, obj->short_description, GET_OBJ_COST(obj));
            strcat(buf, buf1);
          }
          k++;
        }
      }
      page_string(ch->desc, buf, 1);
    }
    else {
      buf[0]=0;
      for(i=bottom, k=1; i <= top; i++) {
        if((j=real_object(i)) > -1) {
          obj=&obj_proto[j];
          sprintf(buf1, "%3d) %-59s  %4d\r\n", k, obj->short_description, GET_OBJ_COST(obj));
          strcat(buf, buf1);
          k++;
        }
      }
      page_string(ch->desc, buf, 1);
    }
    return TRUE;
  }
  else if(CMD_IS("identify")) {
    one_argument(argument, arg);
    if((!*arg) || (!is_number(arg))) {
      send_to_char("You must specify an item number.\r\n", ch);
      return TRUE;
    }
    j=atoi(arg);
    if(j < 1) {
      send_to_char("No such item.\r\n", ch);
      return TRUE;
    }
    for(i=bottom, k=1; i <= top; i++) {
      if(real_object(i) > -1) {
        if(j==k) {
          obj=&obj_proto[real_object(i)];
          break;
        }
        k++;
      }
    }
    if(!obj) {
      send_to_char("No such item.\r\n", ch);
      return TRUE;
    }
    spell_identify(100, ch, NULL, obj);
    return TRUE;
  }
  else if(CMD_IS("sell")) {
    one_argument(argument, arg);
    if(!*arg) {
      send_to_char("You must specify an item to sell.\r\n", ch);
      return TRUE;
    }
    if(!(obj=get_obj_in_list_vis(ch, arg, ch->carrying))) {
      send_to_char("You aren't carrying anything like that.\r\n", ch);
      return TRUE;
    }
    j=GET_OBJ_VNUM(obj);
    if((j < bottom) || (j > top)) {
      send_to_char("I can't buy that.\r\n", ch);
      return TRUE;
    }
    cost=GET_OBJ_COST(obj);
    if(GET_QP(ch)+cost > 30000) {
      send_to_char("Sorry, you can't hold any more quest points, try buying something instead.\r\n", ch);
      return TRUE;
    }
    if(cost > 1) {
      extract_obj(obj);
      GET_QP(ch) += cost/2;
      act("You sell $p.", FALSE, ch, obj, NULL, TO_CHAR);
      act("$n sells $p.", FALSE, ch, obj, NULL, TO_ROOM);
    }
    else if(cost < 0) {
      send_to_char("Invalid cost on that item!\r\n", ch);
    }
    else {
      send_to_char("That's not worth buying.\r\n", ch);
    }
    return TRUE;
  }
  return FALSE;
}


SPECIAL(ali_rent_a_harem)
{
  struct char_data *ali=(struct char_data *)me;
  char buf[MAX_INPUT_LENGTH];

  if(!CMD_IS("list"))
    return FALSE;

  if(number(0,3)) {
    sprintf(buf, "%s I'm terribly sorry! I'm out of girls, but I do have some sheep right out back if you're interested..", GET_NAME(ch));
    do_tell(ali, buf, 0, 0);
  }
  else {
    sprintf(buf, "%s All of my girls are busy, but there's an old fat lady just outside of town...", GET_NAME(ch));
    do_tell(ali, buf, 0, 0);
  }
  act("Ali gestures with one of his fingers towards some distant location, sneering.", FALSE, ali, NULL, NULL, TO_ROOM);

  return TRUE;
}


SPECIAL(hakico_discount_fertilizer)
{
  struct char_data *hakico=(struct char_data *)me;
  char buf[MAX_INPUT_LENGTH];

  if(!CMD_IS("list"))
    return FALSE;

  sprintf(buf, "%s Fouads camels all have diarrhea this week, but I have a nice joint I will sell you...", GET_NAME(ch));
  do_tell(hakico, buf, 0, 0);
  act("Hakico puffs on his joint happily and says something about free love.", FALSE, hakico, NULL, NULL, TO_ROOM);
  return TRUE;
}


SPECIAL(agraba_rajah)
{
  struct char_data *jasmine, *rajah=(struct char_data *)me;

  if(cmd)
    return FALSE;

  for(jasmine=world[rajah->in_room].people; jasmine; jasmine=jasmine->next_in_room) {
    if(GET_MOB_VNUM(jasmine)==22639)
      break;
  }

  if(jasmine && !rajah->master) add_follower(rajah, jasmine);

  return FALSE;
}


SPECIAL(agraba_iago)
{
  struct char_data *jafar, *iago=(struct char_data *)me;

  if(cmd)
    return FALSE;

  for(jafar=world[iago->in_room].people; jafar; jafar=jafar->next_in_room) {
    if(GET_MOB_VNUM(jafar)==22661)
      break;
  }

  if(jafar && !iago->master) add_follower(iago, jafar);

  return FALSE;
}


SPECIAL(agraba_abu)
{
  struct char_data *aladdin, *abu=(struct char_data *)me;

  if(cmd)
    return FALSE;

  for(aladdin=world[abu->in_room].people; aladdin; aladdin=aladdin->next_in_room) {
    if(GET_MOB_VNUM(aladdin)==22647)
      break;
  }

  if(aladdin && !abu->master) add_follower(abu, aladdin);

  return FALSE;
}


SPECIAL(agraba_jasmine)
{
  struct char_data *rajah, *jasmine=(struct char_data *)me;

  if(cmd)
    return FALSE;

  for(rajah=world[jasmine->in_room].people; rajah; rajah=rajah->next_in_room) {
    if(GET_MOB_VNUM(rajah)==22652)
      break;
  }

  if(rajah && !rajah->master) add_follower(rajah, jasmine);

  return FALSE;
}


SPECIAL(agraba_jafar)
{
  struct char_data *iago, *jafar=(struct char_data *)me;

  if(cmd)
    return FALSE;

  for(iago=world[jafar->in_room].people; iago; iago=iago->next_in_room) {
    if(GET_MOB_VNUM(iago)==22662)
      break;
  }

  if(iago && !iago->master) add_follower(iago, jafar);

  return FALSE;
}


SPECIAL(agraba_aladdin)
{
  struct char_data *abu, *aladdin=(struct char_data *)me;

  if(cmd)
    return FALSE;

  for(abu=world[aladdin->in_room].people; abu; abu=abu->next_in_room) {
    if(GET_MOB_VNUM(abu)==22648)
      break;
  }

  if(abu && !abu->master) add_follower(abu, aladdin);

  return FALSE;
}


void multiclass(struct char_data *ch, int new_class)
{
  GET_CLASS_BITVECTOR(ch) |= (1 << new_class);
  GET_NUM_CLASSES(ch)++;
  GET_CLASS(ch)=new_class;
  do_start(ch);
}


SPECIAL(mage_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a mage, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "mage")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_MAGIC_USER) {
      sprintf(buf, "%s You are already a mage!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), MU_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a mage.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_MAGIC_USER);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a mage.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new mages.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}


SPECIAL(cleric_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a cleric, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "cleric")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_CLERIC) {
      sprintf(buf, "%s You are already a cleric!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), CL_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a cleric.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_CLERIC);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a cleric.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new clerics.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}


SPECIAL(thief_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a thief, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "thief")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_THIEF) {
      sprintf(buf, "%s You are already a thief!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), TH_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a thief.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_THIEF);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a thief.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new thieves.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}


SPECIAL(warrior_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a warrior, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "warrior")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_WARRIOR) {
      sprintf(buf, "%s You are already a warrior!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), WA_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a warrior.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_WARRIOR);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a warrior.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new warriors.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}


SPECIAL(paladin_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a paladin, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "paladin")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_PALADIN) {
      sprintf(buf, "%s You are already a paladin!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), PA_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a paladin.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_PALADIN);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a paladin.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new paladins.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}


SPECIAL(ranger_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a ranger, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "ranger")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_RANGER) {
      sprintf(buf, "%s You are already a ranger!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), RA_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a ranger.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_RANGER);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a ranger.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new rangers.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}


SPECIAL(antipaladin_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));

  if(!CMD_IS("train"))
    return FALSE;

  if(IS_NPC(ch))
    return FALSE;

  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a antipaladin, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "antipaladin")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_ANTIPALADIN) {
      sprintf(buf, "%s You are already a antipaladin!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), AP_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a antipaladin.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_ANTIPALADIN);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a antipaladin.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new antipaladins.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}
SPECIAL(monk_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));
  if(!CMD_IS("train"))
    return FALSE;
  if(IS_NPC(ch))
    return FALSE;
  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a monk, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "monk")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_MONK) {
      sprintf(buf, "%s You are already a monk!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), MK_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a monk.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_MONK);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a monk.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new monks.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}
SPECIAL(psionicist_trainer)
{
  struct char_data *trainer=(struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  if(CMD_IS("practice"))
    return(guild(ch, me, cmd, argument));
  if(!CMD_IS("train"))
    return FALSE;
  if(IS_NPC(ch))
    return FALSE;
  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    sprintf(buf, "%s I can train you to be a psionicist, but it will cost you %d practices.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
    do_tell(trainer, buf, 0, 0);
  }
  else if(!str_cmp(arg, "psionicist")) {
    if(GET_PRACTICES(ch) < ((GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1))) {
      sprintf(buf, "%s You cannot train now, it takes %d pratices with your current status.", GET_NAME(ch), (GET_NUM_CLASSES(ch)*ch->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(ch)+1));
      do_tell(trainer, buf, 0, 0);
    }
    else if(GET_CLASS(ch)==CLASS_PSIONICIST) {
      sprintf(buf, "%s You are already a psionicist!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else if(IS_SET(GET_CLASS_BITVECTOR(ch), PS_F)) {
      sprintf(buf, "%s I'm sorry, you can progress no further as a psionicist.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
    else {
      multiclass(ch, CLASS_PSIONICIST);
      sprintf(buf, "%s Congratulations!", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
      sprintf(buf, "%s With my help you have achieved the basic skills needed progress as a psionicist.", GET_NAME(ch));
      do_tell(trainer, buf, 0, 0);
    }
  }
  else {
    sprintf(buf, "%s I can only train new psionicists.", GET_NAME(ch));
    do_tell(trainer, buf, 0, 0);
  }
  return TRUE;
}
SPECIAL(reroll)
{
  struct char_data *sailor=(struct char_data *)me, *k, *temp;
  char buf[MAX_INPUT_LENGTH];
  if((!CMD_IS("reroll")) && (!CMD_IS("list")))
    return FALSE;
  if(IS_NPC(ch))
    return FALSE;
  if(GET_LEVEL(ch) >= LVL_CIMP) {
    sprintf(buf, "%s Immortals cannot reroll. If you want your stats changed, do it youself.", GET_NAME(ch));
    do_tell(sailor, buf, 0, 0);
    return TRUE;
  }
  if(GET_LEVEL(ch) >= LVL_HERO) {
    sprintf(buf, "%s Immortals cannot reroll. If you want your stats changed, ask an admin.", GET_NAME(ch));
    do_tell(sailor, buf, 0, 0);
    return TRUE;
  }
  if(CMD_IS("list")) {
    sprintf(buf, "%s It will cost you %ld coins to reroll.", GET_NAME(ch), (((GET_REROLLS(ch)+2L)*(GET_REROLLS(ch)+1L))/2L)*1000000L);
    do_tell(sailor, buf, 0, 0);
    return TRUE;
  }
  if(CMD_IS("reroll")) {
    if(ch->char_specials.spcont) {
      sprintf(buf, "%s You cannot reroll with continuous affects.", GET_NAME(ch));
      do_tell(sailor, buf, 0, 0);
    }
    else if(GET_GOLD(ch)<((((GET_REROLLS(ch)+2)*(GET_REROLLS(ch)+1))/2)*1000000)) {
      sprintf(buf, "%s You do not have the %ld coins it costs to reroll.", GET_NAME(ch), (((GET_REROLLS(ch)+2L)*(GET_REROLLS(ch)+1L))/2L)*1000000L);
      do_tell(sailor, buf, 0, 0);
    }
    else {
      sprintf(buf, "%s That will be %ld coins.", GET_NAME(ch), (((GET_REROLLS(ch)+2L)*(GET_REROLLS(ch)+1L))/2L)*1000000L);
      do_tell(sailor, buf, 0, 0);
      GET_GOLD(ch)-=(((GET_REROLLS(ch)+2L)*(GET_REROLLS(ch)+1L))/2L)*1000000L;
      GET_REROLLS(ch)+=1;
      sprintf(buf, "%s has rerolled %s.", GET_NAME(sailor), GET_NAME(ch));
      log(buf);
      act("$n flips a switch on the wall.", FALSE, sailor, NULL, NULL, TO_ROOM);
      send_to_char("You beging tingling from head to foot. The sensation spreads inward\r\n"
                   "through your body and the world fades from view.\r\n", ch);
      act("Ribbons of light beging to swirl around $n. They close in, distorting\r\n"
          "$s flesh so that it becomes harder and harder to distinguish it from\r\n"
          "the ribbons. In a matter of seconds $e has become a rapidly shrinking\r\n"
          "column of colored light which soon dissapears.", TRUE, ch, 0, 0, TO_ROOM);
      ch->was_in_room=world[ch->in_room].number;
      if (FIGHTING(ch))
        stop_fighting(ch);
      for (k = combat_list; k; k = temp) {
        temp = k->next_fighting;
        if (FIGHTING(k) == ch)
          stop_fighting(k);
      }
      ch->desc->prompt_mode = 0;
      ch->player_specials->saved.reroll_level=GET_LEVEL(ch);
      ch->player_specials->saved.rerolling=STATE(ch->desc) = CON_REROLLING;
      save_char(ch, NOWHERE);
      REMOVE_FROM_LIST(ch, character_list, next);
      char_from_room(ch);
      roll_real_abils(ch);
      SEND_TO_Q("\r\nYour abilities are:\r\n", ch->desc);
      sprintf(buf, "       Strength: %2d/%02d\r\n", GET_STR(ch), GET_ADD(ch));
      SEND_TO_Q(buf, ch->desc);
      sprintf(buf, "   Intelligence: %2d\r\n", GET_INT(ch));
      SEND_TO_Q(buf, ch->desc);
      sprintf(buf, "         Wisdom: %2d\r\n", GET_WIS(ch));
      SEND_TO_Q(buf, ch->desc);
      sprintf(buf, "      Dexterity: %2d\r\n", GET_DEX(ch));
      SEND_TO_Q(buf, ch->desc);
      sprintf(buf, "   Constitution: %2d\r\n", GET_CON(ch));
      SEND_TO_Q(buf, ch->desc);
      sprintf(buf, "       Charisma: %2d\r\n", GET_CHA(ch));
      SEND_TO_Q(buf, ch->desc);
      SEND_TO_Q("\nKeep these statistics (y/n)? ", ch->desc);
    }
  }
  return TRUE;
}
/* for the tour guide */
struct tour_info {
  int ok;	/* used to decide if it should be freed */
  long id;	/* id of character */
  int location; /* negative numbers for walking, positive for tour sites */
  struct tour_info *next;
};
int tour_vnums[] = { 3000, 3050, -1 };
SPECIAL(tour)
{
  static struct tour_info *info_start=NULL;
  struct tour_info *info, *tempi;
  int i, j, k, rnum;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guide=(struct char_data *)me, *tch;
  struct follow_type *f;
  int find_first_step(sh_int src, sh_int target);
  ACMD(do_assist);
  ACMD(do_mpcast);
  if(!guide)
    return FALSE;
  for(info=info_start; info; info=info->next)
    info->ok=0;
  if(info_start) {
    for(tch=character_list; tch; tch=tch->next) {
      if(!IS_NPC(tch)) {
        for(info=info_start; info; info=info->next) {
          if(info->id==GET_IDNUM(tch)) {
            for (f = tch->followers; f; f = f->next) {
              if(IS_NPC(f->follower) && (GET_MOB_RNUM(guide)==GET_MOB_RNUM(f->follower))) {
                info->ok=1;
                break;
              }
            }
            if(info->ok==0) {
              char_from_room(tch);
              char_to_room(tch, real_room(3072));
              look_at_room(tch, 1);
              REMOVE_BIT(PLR_FLAGS(ch), PLR_TOURING);
            }
          }
        }
      }
    }
  }
  while(info_start && (info_start->ok==0)) {
    info=info_start->next;
    free(info_start);
    info_start=info;
  }
  if(info_start) {
    for(info=info_start; info->next; info=info->next) {
      if(info->next->ok==0) {
        tempi=info->next->next;
        free(info->next);
        info->next=tempi;
      }
    }
  }
  if((!guide->master) && (world[guide->in_room].number!=4)) {
    act("$n utters the word, 'marissoif', touches $s hat, and disappears.", TRUE, guide, NULL, NULL, TO_ROOM);
    extract_char(guide);
    return FALSE;
  }
  if(IS_AFFECTED(guide, AFF_GROUP) && guide->master && FIGHTING(guide->master) && (!FIGHTING(guide))) {
    do_assist(guide, GET_NAME(guide->master), 0, 0);
  }
  if((cmd==0) && guide->master) {
    for(info=info_start; info; info=info->next) {
      if(info->id==GET_IDNUM(guide->master))
        break;
    }
    if(info) {
      if(info->location < 0) {
        switch(info->location) {
          case -1:
            rnum=real_room(8801);
            break;
          case -2:
            rnum=real_room(4956);
            break;
          case -3:
            rnum=real_room(15800);
            break;
          case -4:
            rnum=real_room(15699);
            break;
          case -5:
            rnum=real_room(11592);
            break;
          default:
            rnum=-1;
            break;
        }
        i=find_first_step(guide->master->in_room, rnum);
        switch(i) {
          case BFS_ERROR:
          case BFS_NO_PATH:
            do_say(guide, "I can't seem to find the way from here.", 0, 0);
            if(info==info_start) {
              info=info_start->next;
              free(info_start);
              info_start=info;
            }
            else {
              for(tempi=info_start; tempi->next!=info; tempi=tempi->next);
              tempi->next=info->next;
              free(info);
            }
            break;
          case BFS_ALREADY_THERE:
            do_say(guide, "Here we are.", 0, 0);
            if(info==info_start) {
              info=info_start->next;
              free(info_start);
              info_start=info;
            }
            else {
              for(tempi=info_start; tempi->next!=info; tempi=tempi->next);
              tempi->next=info->next;
              free(info);
            }
            break;
          default:
            perform_move(guide->master, i, 1);
            break;
        }
      }
    }
  }
  if((!ch) || IS_NPC(ch))
    return FALSE;
  if((cmd > 0) && (cmd < (NUM_OF_DIRS+4)) && PLR_FLAGGED(ch, PLR_TOURING)) {
    for(info=info_start; info; info=info->next) {
      if(GET_IDNUM(ch)==info->id)
        break;
    }
    if(info) {
      for(rnum=0; rnum <= top_of_world; rnum++)
        REMOVE_BIT(ROOM_FLAGS(rnum), ROOM_BFS_MARK);
      if((rnum=real_room(tour_vnums[info->location])) >= 0) {
        SET_BIT(ROOM_FLAGS(rnum), ROOM_BFS_MARK);
        for(i=0; i<NUM_OF_DIRS; i++) {
          if(world[rnum].dir_option[i] && (world[rnum].dir_option[i]->to_room > 0)) {
            SET_BIT(ROOM_FLAGS(world[rnum].dir_option[i]->to_room), ROOM_BFS_MARK);
            for(j=0; j<NUM_OF_DIRS; j++) {
              if(world[world[rnum].dir_option[i]->to_room].dir_option[j] &&
                 (world[world[rnum].dir_option[i]->to_room].dir_option[j]->to_room > 0)) {
                SET_BIT(ROOM_FLAGS(world[world[rnum].dir_option[i]->to_room].dir_option[j]->to_room), ROOM_BFS_MARK);
                for(k=0; k<NUM_OF_DIRS; k++) {
                  if(world[world[world[rnum].dir_option[i]->to_room].dir_option[j]->to_room].dir_option[k] &&
                     (world[world[world[rnum].dir_option[i]->to_room].dir_option[j]->to_room].dir_option[k]->to_room > 0))
                    SET_BIT(ROOM_FLAGS(world[world[world[rnum].dir_option[i]->to_room].dir_option[j]->to_room].dir_option[k]->to_room), ROOM_BFS_MARK);
                }
              }
            }
          }
        }
      }
      if(cmd > NUM_OF_DIRS)
        cmd-=4;
      cmd--;
      if(CAN_GO(ch, cmd) && (!IS_SET(ROOM_FLAGS(world[ch->in_room].dir_option[cmd]->to_room), ROOM_BFS_MARK))) {
        do_say(guide, "Sorry, you can't go any farther while on a tour.", 0, 0);
        return TRUE;
      }
    }
  }
  else if(CMD_IS("say") || CMD_IS("'")) {
    skip_spaces(&argument);
    strcpy(buf, argument);
    for(i=strlen(buf)-1; i>=0; i--) {
      if((!isspace(buf[i])) && (buf[i]!='.') && (buf[i]!='!') && (buf[i]!='?')) {
        buf[i+1]=0;
        break;
      }
    }
    if(guide->master!=ch) {
      if((!guide->master) && (world[guide->in_room].number==4)) {
        if(!str_cmp(argument, "guide me")) {
          do_say(ch, argument, 0, 0);
          for (f = ch->followers; f; f = f->next) {
            if(IS_NPC(f->follower) && (GET_MOB_RNUM(guide)==GET_MOB_RNUM(f->follower))) {
              break;
            }
          }
          if(!f) {
            if (ch->master)
              stop_follower(ch);
            REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
            add_follower(guide, ch);
            guide = read_mobile(GET_MOB_RNUM(guide), REAL);
            char_to_room(guide, ch->in_room);
          }
          return TRUE;
        }
      }
    }
    else {
      do_say(ch, argument, 0, 0);
      if(!str_cmp(buf, "stop")) {
        for(info=info_start; info; info=info->next) {
          if(info->id==GET_IDNUM(guide->master))
            break;
        }
        if(info && (info->location < 0)) {
          if(info==info_start) {
            info=info_start->next;
            free(info_start);
            info_start=info;
          }
          else {
            for(tempi=info_start; tempi->next!=info; tempi=tempi->next);
            tempi->next=info->next;
            free(info);
          }
        }
      }
      else if(!str_cmp(buf, "armor me")) {
        sprintf(buf, "50 'armor' %s", GET_NAME(ch));
        do_mpcast(guide, buf, 0, 0);
      }
      else if(!str_cmp(buf, "aid me")) {
        sprintf(buf, "50 'aid' %s", GET_NAME(ch));
        do_mpcast(guide, buf, 0, 0);
      }
      else if(!str_cmp(buf, "strength me")) {
        sprintf(buf, "50 'str' %s", GET_NAME(ch));
        do_mpcast(guide, buf, 0, 0);
      }
      else if(!str_cmp(buf, "heal me")) {
        sprintf(buf, "50 'heal' %s", GET_NAME(ch));
        do_mpcast(guide, buf, 0, 0);
      }
      else if(!str_cmp(buf, "i can't see")) {
        sprintf(buf, "50 'cure blind' %s", GET_NAME(ch));
        do_mpcast(guide, buf, 0, 0);
        sprintf(buf, "50 'magic light' %s", GET_NAME(ch));
        do_mpcast(guide, buf, 0, 0);
      }
      else if(!str_cmp(buf, "i need food")) {
        if(GET_COND(ch, FULL) > 10) {
          do_say(guide, "No you don't.", 0, 0);
        }
        else {
          act("$n utters the word, 'vfoim'.", TRUE, guide, NULL, NULL, TO_ROOM);
          GET_COND(ch, FULL)=24;
          send_to_char("You feel quite full.\r\n", ch);
        }
      }
      else if(!str_cmp(buf, "i need water")) {
        if(GET_COND(ch, THIRST) > 10) {
          do_say(guide, "No you don't.", 0, 0);
        }
        else {
          act("$n utters the word, 'xihof'.", TRUE, guide, NULL, NULL, TO_ROOM);
          GET_COND(ch, THIRST)=24;
          send_to_char("You feel very refreshed.\r\n", ch);
        }
      }
      else if(!str_cmp(buf, "take me on a tour")) {
        for(info=info_start; info; info=info->next) {
          if(info->id==GET_IDNUM(guide->master))
            break;
        }
        if(info) {
          if((tour_vnums[0] > 0) && (real_room(tour_vnums[0]) >= 0)) {
            info->location=0;
            act("$n utters the word, 'hgrubgsafh'.", TRUE, guide, NULL, NULL, TO_ROOM);
            act("$n vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
            act("$N vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
            char_from_room(ch);
            char_to_room(ch, real_room(tour_vnums[info->location]));
            look_at_room(ch, 1);
            char_from_room(guide);
            char_to_room(guide, real_room(tour_vnums[info->location]));
            SET_BIT(PLR_FLAGS(ch), PLR_TOURING);
          }
        }
        else {
          if((tour_vnums[0] > 0) && (real_room(tour_vnums[0]) >= 0)) {
            CREATE(info, struct tour_info, 1);
            info->id=GET_IDNUM(ch);
            info->location=0;
            info->next=info_start;
            info_start=info;
            act("$n utters the word, 'hgrubgsafh'.", TRUE, guide, NULL, NULL, TO_ROOM);
            act("$n vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
            act("$N vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
            char_from_room(ch);
            char_to_room(ch, real_room(tour_vnums[info->location]));
            look_at_room(ch, 1);
            char_from_room(guide);
            char_to_room(guide, real_room(tour_vnums[info->location]));
            SET_BIT(PLR_FLAGS(ch), PLR_TOURING);
          }
        }
      }
      else if(!str_cmp(buf, "next stop")) {
        if(PLR_FLAGGED(ch, PLR_TOURING)) {
          for(info=info_start; info; info=info->next) {
            if(info->id==GET_IDNUM(guide->master))
              break;
          }
          if(info) {
            info->location++;
            if((tour_vnums[info->location] > 0) && (real_room(tour_vnums[info->location]) >= 0)) {
              act("$n utters the word, 'hgrubgsafh'.", TRUE, guide, NULL, NULL, TO_ROOM);
              act("$n vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
              act("$N vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
              char_from_room(ch);
              char_to_room(ch, real_room(tour_vnums[info->location]));
              look_at_room(ch, 1);
              char_from_room(guide);
              char_to_room(guide, real_room(tour_vnums[info->location]));
            }
            else {
              act("$n utters the word, 'hgrubgsafh'.", TRUE, guide, NULL, NULL, TO_ROOM);
              act("$n vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
              act("$N vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
              char_from_room(ch);
              char_to_room(ch, real_room(3072));
              look_at_room(ch, 1);
              char_from_room(guide);
              char_to_room(guide, real_room(3072));
              REMOVE_BIT(PLR_FLAGS(ch), PLR_TOURING);
              do_say(guide, "The tour is over, I hope you enjoyed it.", 0, 0);
              if(info==info_start) {
                info=info_start->next;
                free(info_start);
                info_start=info;
              }
              else {
                for(tempi=info_start; tempi->next!=info; tempi=tempi->next);
                tempi->next=info->next;
                free(info);
              }
            }
          }
        }
        else {
          do_say(guide, "You aren't on a tour.", 0, 0);
        }
      }
      else if(!str_cmp(buf, "end the tour")) {
        if(PLR_FLAGGED(ch, PLR_TOURING)) {
          for(info=info_start; info; info=info->next) {
            if(info->id==GET_IDNUM(guide->master))
              break;
          }
          if(info) {
            if(info==info_start) {
              info=info_start->next;
              free(info_start);
              info_start=info;
            }
            else {
              for(tempi=info_start; tempi->next!=info; tempi=tempi->next);
              tempi->next=info->next;
              free(info);
            }
          }
          act("$n utters the word, 'hgrubgsafh'.", TRUE, guide, NULL, NULL, TO_ROOM);
          act("$n vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
          act("$N vanishes.", TRUE, guide, NULL, ch, TO_NOTVICT);
          char_from_room(ch);
          char_to_room(ch, real_room(3072));
          look_at_room(ch, 1);
          char_from_room(guide);
          char_to_room(guide, real_room(3072));
          REMOVE_BIT(PLR_FLAGS(ch), PLR_TOURING);
        }
        else {
          do_say(guide, "You aren't on a tour.", 0, 0);
        }
      }
      else if(!str_cmp(buf, "take me to muppets")) {
        CREATE(info, struct tour_info, 1);
        info->id=GET_IDNUM(ch);
        info->location=-1;
        info->next=info_start;
        info_start=info;
      }
      else if(!str_cmp(buf, "take me to toy story")) {
        CREATE(info, struct tour_info, 1);
        info->id=GET_IDNUM(ch);
        info->location=-2;
        info->next=info_start;
        info_start=info;
      }
      else if(!str_cmp(buf, "take me to the newbie forest")) {
        CREATE(info, struct tour_info, 1);
        info->id=GET_IDNUM(ch);
        info->location=-3;
        info->next=info_start;
        info_start=info;
      }
      else if(!str_cmp(buf, "take me to the newbie plateau")) {
        CREATE(info, struct tour_info, 1);
        info->id=GET_IDNUM(ch);
        info->location=-4;
        info->next=info_start;
        info_start=info;
      }
      else if(!str_cmp(buf, "take me to the haunted castle")) {
        CREATE(info, struct tour_info, 1);
        info->id=GET_IDNUM(ch);
        info->location=-5;
        info->next=info_start;
        info_start=info;
      }
      return TRUE;
    }
  }
  return FALSE;
}
/* this will make mobs follow players and move toward them */
SPECIAL(follow)
{
  struct char_data *tch, *mob=(struct char_data *)me;
  int players=0, max_players=0, dir=-1, i, home;
  if((cmd>0)&&(cmd<=NUM_OF_DIRS+4)) {
    if(FIGHTING(mob))
      return FALSE;
    for(home=0; home <= top_of_zone_table; home++) {
      if((GET_MOB_VNUM(mob) >= zone_table[home].bottom) && (GET_MOB_VNUM(mob) <= zone_table[home].top))
        break;
    }
    if(ch && (!IS_NPC(ch)) && (GET_LEVEL(ch) < LVL_HERO)) {
      if(cmd > NUM_OF_DIRS)
        cmd-=4;
      cmd--;
      if(CAN_GO(mob, cmd) && (zone_table[world[EXIT(mob, cmd)->to_room].zone].number==home) &&
         (!ROOM_FLAGGED(EXIT(mob, cmd)->to_room, ROOM_DEATH)))
        if(number(0, 7))
          perform_move(mob, cmd, 0);
    }
  }
  else if((cmd==0) && (!FIGHTING(mob))) {
    if(mob->in_room <= 1)
      return FALSE;
    for(tch=world[mob->in_room].people; tch; tch=tch->next_in_room) {
      if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
        players++;
    }
    if(players==0) {
      for(home=0; home <= top_of_zone_table; home++) {
        if((GET_MOB_VNUM(mob) >= zone_table[home].bottom) && (GET_MOB_VNUM(mob) <= zone_table[home].top))
          break;
      }
      for(i=0; i<NUM_OF_DIRS; i++) {
        players=0;
        if(CAN_GO(mob, i) && (zone_table[world[EXIT(mob, i)->to_room].zone].number==home) &&
           (!ROOM_FLAGGED(EXIT(mob, i)->to_room, ROOM_DEATH))) {
          for(players=0, tch=world[EXIT(mob, i)->to_room].people; tch; tch=tch->next_in_room) {
            if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
              players++;
          }
          if(players > max_players) {
            max_players=players;
            dir=i;
          }
        }
      }
      if(dir >= 0) {
        perform_move(mob, dir, 0);
        return TRUE;
      }
    }
  }
  return FALSE;
}
SPECIAL(teddy)
{
  int i;
  struct char_data *vict = (struct char_data *)me;
  char arg[MAX_INPUT_LENGTH];
  if(ch && me && (CMD_IS("hit") || CMD_IS("murder") || CMD_IS("kill")) && (GET_LEVEL(ch) < LVL_HERO)) {
    one_argument(argument, arg);
    if((*arg) && (vict == get_char_room_vis(ch, arg))) {
      sprintf(arg, " 'sleep' %s", GET_NAME(ch));
      for(i=0; i<35; i++) {
        do_cast(vict, arg, 0, 1);
        if(GET_POS(ch) == POS_SLEEPING) {
          hit(ch, vict, TYPE_UNDEFINED);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
/* Will make a mob walk toward and follow players and steal stuff
   from their inventory, containers, and even worn eq */
SPECIAL(kender)
{
  struct char_data *tch, *mob=(struct char_data *)me;
  struct obj_data *obj, *tobj;
  int players=0, max_players=0, dir=-1, i, j, num_items;
  char arg[MAX_INPUT_LENGTH];
  if((cmd>0)&&(cmd<=NUM_OF_DIRS+4)) {
    if(FIGHTING(mob))
      return FALSE;
    if(ch && (!IS_NPC(ch)) && (GET_LEVEL(ch) < LVL_HERO)) {
      if(cmd > NUM_OF_DIRS)
        cmd-=4;
      cmd--;
      if(CAN_GO(mob, cmd) && ((zone_table[world[EXIT(mob, cmd)->to_room].zone].number==179) ||
          (zone_table[world[EXIT(mob, cmd)->to_room].zone].number==181)) &&
         (!ROOM_FLAGGED(EXIT(mob, cmd)->to_room, ROOM_DEATH)))
        if(number(0, 7))
          perform_move(mob, cmd, 0);
    }
  }
  else if((cmd==0) && (!FIGHTING(mob))) {
    if(mob->in_room <= 1)
      return FALSE;
    for(tch=world[mob->in_room].people; tch; tch=tch->next_in_room) {
      if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
        players++;
    }
    if(players==0) {
      for(i=0; i<NUM_OF_DIRS; i++) {
        players=0;
        if(CAN_GO(mob, i) && ((zone_table[world[EXIT(mob, i)->to_room].zone].number==179) ||
            (zone_table[world[EXIT(mob, i)->to_room].zone].number==181)) &&
           (!ROOM_FLAGGED(EXIT(mob, i)->to_room, ROOM_DEATH))) {
          for(players=0, tch=world[EXIT(mob, i)->to_room].people; tch; tch=tch->next_in_room) {
            if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
              players++;
          }
          if(players > max_players) {
            max_players=players;
            dir=i;
          }
        }
      }
      if(dir >= 0) {
        perform_move(mob, dir, 0);
        return TRUE;
      }
      else {
        for(num_items=0, obj=mob->carrying; obj; obj=obj->next_content) {
          if((GET_OBJ_TYPE(obj)==ITEM_TRASH)||(GET_OBJ_TYPE(obj)==ITEM_DRINKCON)||
             (GET_OBJ_TYPE(obj)==ITEM_FOOD)) {
            for(i=0; obj->name[i]&&(!isalpha(obj->name[i])); i++);
            for(j=0; obj->name[i]&&isalpha(obj->name[i]); j++, i++) {
               arg[j]=obj->name[i];
            }
            arg[j]=0;
            do_drop(mob, arg, 0, SCMD_DROP);
          }
          else
            num_items++;
        }
        if(num_items > 10) {
          for(i=10000000, obj=NULL, tobj=mob->carrying; tobj; tobj=tobj->next_content) {
            if(GET_OBJ_COST(tobj) < i) {
              i=GET_OBJ_COST(tobj);
              obj=tobj;
            }
          }
          if(obj) {
            for(i=0; obj->name[i]&&(!isalpha(obj->name[i])); i++);
            for(j=0; obj->name[i]&&isalpha(obj->name[i]); j++, i++) {
               arg[j]=obj->name[i];
            }
            arg[j]=0;
            do_drop(mob, arg, 0, SCMD_DROP);
          }
        }
        if(number(0, 4) < 3) {
          if((i=get_dir(mob))<NUM_OF_DIRS) {
            if(((zone_table[world[EXIT(mob, i)->to_room].zone].number==179) ||
               (zone_table[world[EXIT(mob, i)->to_room].zone].number==181)) &&
               (!ROOM_FLAGGED(EXIT(mob, i)->to_room, ROOM_DEATH)))
              perform_move(mob, i, 0);
            return TRUE;
          }
        }
      }
    }
    else {
      if(number(1, 4) == 3) {
        j=number(1, players);
        for(i=0, tch=world[mob->in_room].people; tch; tch=tch->next_in_room) {
          if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
            i++;
          if(i==j)
            break;
        }
        if(tch && (number(0, 99) < GET_LEVEL(tch)-30)) {
          for(num_items=0, i=0; i<NUM_WEARS; i++) {
            if(GET_EQ(tch, i) && (GET_OBJ_TYPE(GET_EQ(tch, i))==ITEM_CONTAINER)) {
              for(tobj=GET_EQ(tch, i)->contains; tobj; tobj=tobj->next_content)
                num_items++;
            }
          }
          for(obj=tch->carrying; obj; obj=obj->next_content) {
            if(GET_OBJ_TYPE(obj)==ITEM_CONTAINER) {
              for(tobj=obj->contains; tobj; tobj=tobj->next_content)
                num_items++;
            }
            else
              num_items++;
          }
          switch(i=number(1, num_items+10)) {
          case 1:
            obj=GET_EQ(tch, WEAR_FINGER_R);
            break;
          case 2:
            obj=GET_EQ(tch, WEAR_FINGER_L);
            break;
          case 3:
            obj=GET_EQ(tch, WEAR_NECK_1);
            break;
          case 4:
            obj=GET_EQ(tch, WEAR_NECK_2);
            break;
          case 5:
            obj=GET_EQ(tch, WEAR_HEAD);
            break;
          case 6:
            obj=GET_EQ(tch, WEAR_FEET);
            break;
          case 7:
            obj=GET_EQ(tch, WEAR_HANDS);
            break;
          case 8:
            obj=GET_EQ(tch, WEAR_WAIST);
            break;
          case 9:
            obj=GET_EQ(tch, WEAR_WRIST_R);
            break;
          case 10:
            obj=GET_EQ(tch, WEAR_WRIST_L);
            break;
          default:
            i-=10;
            for(num_items=0, j=0; j<NUM_WEARS; j++) {
              if(GET_EQ(tch, j) && (GET_OBJ_TYPE(GET_EQ(tch, j))==ITEM_CONTAINER)) {
                for(tobj=GET_EQ(tch, j)->contains; tobj; tobj=tobj->next_content) {
                  num_items++;
                  if(i==num_items)
                    break;
                }
                if(i==num_items) {
                  obj=tobj;
                  break;
                }
              }
            }
            if(num_items < i) {
              for(obj=tch->carrying; obj; obj=obj->next_content) {
                if(GET_OBJ_TYPE(obj)==ITEM_CONTAINER) {
                  for(tobj=obj->contains; tobj; tobj=tobj->next_content) {
                    num_items++;
                    if(i==num_items) {
                      obj=tobj;
                      break;
                    }
                  }
                }
                else
                  num_items++;
                if(i==num_items)
                  break;
              }
            }
            break;
          }
          if(obj) {
            switch(number(0, 5)) {
            case 0:
              do_say(mob, "I found it on the floor.", 0, 0);
              break;
            case 1:
              do_say(mob, "It fell in my pouch by accident.", 0, 0);
              break;
            case 2:
              do_say(mob, "Are you sure it's yours?", 0, 0);
              break;
            case 3:
              do_say(mob, "I thought you didn't want it anymore.", 0, 0);
              break;
            case 4:
              do_say(mob, "You just walked off and left it.", 0, 0);
              break;
            case 5:
              do_say(mob, "I was going to wash it and give it back to you.", 0, 0);
              break;
            }
            if (obj->worn_by != NULL)
              if (unequip_char(obj->worn_by, obj->worn_on) != obj)
                log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
            if (obj->carried_by)
              obj_from_char(obj);
            if (obj->in_obj)
              obj_from_obj(obj);
            obj_to_char(obj, mob);
          }
        }
        if(number(0, 1))
          return TRUE;
      }
      else {
        for(num_items=0, obj=mob->carrying; obj; obj=obj->next_content) {
          if((GET_OBJ_TYPE(obj)==ITEM_TRASH)||(GET_OBJ_TYPE(obj)==ITEM_DRINKCON)||
             (GET_OBJ_TYPE(obj)==ITEM_FOOD)) {
            for(i=0; obj->name[i]&&(!isalpha(obj->name[i])); i++);
            for(j=0; obj->name[i]&&isalpha(obj->name[i]); j++, i++) {
               arg[j]=obj->name[i];
            }
            arg[j]=0;
            do_drop(mob, arg, 0, SCMD_DROP);
          }
          else
            num_items++;
        }
        if(num_items > 10) {
          for(i=10000000, obj=NULL, tobj=mob->carrying; tobj; tobj=tobj->next_content) {
            if(GET_OBJ_COST(tobj) < i) {
              i=GET_OBJ_COST(tobj);
              obj=tobj;
            }
          }
          if(obj) {
            for(i=0; obj->name[i]&&(!isalpha(obj->name[i])); i++);
            for(j=0; obj->name[i]&&isalpha(obj->name[i]); j++, i++) {
               arg[j]=obj->name[i];
            }
            arg[j]=0;
            do_drop(mob, arg, 0, SCMD_DROP);
          }
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(tandar)
{
  struct char_data *tch, *tandar=(struct char_data *)me;
  int room_ok=0;
  if((cmd==0) && tandar && (world[tandar->in_room].number != 17948)) {
    for(tch=world[tandar->in_room].people; tch; tch=tch->next) {
      if(IS_NPC(tch) && (GET_MOB_VNUM(tch) == 17929)) {
        room_ok=1;
        break;
      }
    }
    if(!room_ok) {
      char_from_room(tandar);
      char_to_room(tandar, real_room(17948));
      send_to_room("Tandar leaps back into the room!\r\n", real_room(17948));
    }
  }
  return FALSE;
}
SPECIAL(crysania)
{
  struct char_data *tch, *tandar=NULL, *crysania=(struct char_data *)me;
  int room_ok=1, pray=0;
  char buf[MAX_STRING_LENGTH], buf1[MAX_INPUT_LENGTH];
  if((cmd==0) && crysania) {
    if(world[crysania->in_room].number != 17948) {
      room_ok=0;
      for(tch=world[crysania->in_room].people; tch; tch=tch->next_in_room) {
        if(IS_NPC(tch) && (GET_MOB_VNUM(tch) == 17930)) {
          room_ok=1;
          break;
        }
      }
    }
    if(!room_ok) {
      if(GET_POS(crysania) == POS_STANDING) {
        char_from_room(crysania);
        char_to_room(crysania, real_room(17948));
        send_to_room("Crysania walks back into the room.\r\n", real_room(17948));
      }
    }
    else if(GET_POS(crysania)==POS_FIGHTING) {
      strcpy(buf, "Crysania prays to Paladine.\r\n");
      for(tch=world[crysania->in_room].people; tch; tch=tch->next_in_room) {
        if(IS_NPC(tch)) {
          if(GET_MOB_VNUM(tch) == 17930) {
            tandar=tch;
          }
          else if(FIGHTING(tch) == crysania) {
            pray=1;
            sprintf(buf1, "A white light destroys %s!\r\n", GET_NAME(tch));
            strcat(buf, buf1);
            extract_char(tch);
          }
        }
      }
      if(pray) {
        send_to_room(buf, crysania->in_room);
      }
      else {
        if(!tandar) {
          do_gen_comm(crysania, "Somebody help me!", 0, SCMD_SHOUT);
          switch(number(0, 7)) {
          case 0:
          case 1:
          case 2:
            strcpy(buf, " 'restore' ");
            do_cast(crysania, buf, 0, 0);
            break;
          case 3:
          case 4:
            strcpy(buf, " 'implosion' ");
            do_cast(crysania, buf, 0, 0);
            break;
          case 5:
            strcpy(buf, " 'calm' ");
            do_cast(crysania, buf, 0, 0);
            break;
          }
          return TRUE;
        }
        else {
          if(FIGHTING(tandar)==crysania)
            stop_fighting(tandar);
          if(FIGHTING(crysania)==tandar)
            stop_fighting(crysania);
          if(!FIGHTING(tandar)) {
            act("$n jumps to the aid of $N!", FALSE, tandar, 0, crysania, TO_ROOM);
            hit(tandar, FIGHTING(crysania), TYPE_UNDEFINED);
          }
          if(number(0, 2)) {
            if(FIGHTING(tandar) && FIGHTING(crysania)) {
              if((GET_HIT(crysania) < 2000) || ((GET_MAX_HIT(crysania)-GET_HIT(crysania)) > (GET_MAX_HIT(tandar)-GET_HIT(tandar)))) {
                strcpy(buf, " 'restore' ");
                do_cast(crysania, buf, 0, 0);
              }
              else {
                strcpy(buf, " 'restore' tandar");
                do_cast(crysania, buf, 0, 0);
              }
            }
            else if(FIGHTING(tandar)) {
              strcpy(buf, " 'restore' tandar");
              do_cast(crysania, buf, 0, 0);
            }
            else if(FIGHTING(crysania)) {
              strcpy(buf, " 'restore' ");
              do_cast(crysania, buf, 0, 0);
            }
            else
              return FALSE;
          }
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(caramon)
{
  struct char_data *tika, *caramon=(struct char_data *)me;
  if((cmd==0) && caramon && !number(0, 7) && !FIGHTING(caramon)) {
    for(tika=world[caramon->in_room].people; tika; tika=tika->next_in_room) {
      if(IS_NPC(tika) && (GET_MOB_VNUM(tika)==17951))
        break;
    }
    do_say(caramon, "I'm hungry.", 0, 0);
    if(tika && GET_EQ(tika, WEAR_WIELD) && !FIGHTING(tika)) {
      sbyte temp;
      temp=GET_HITROLL(tika);
      GET_HITROLL(tika)=100;
      hit(tika, caramon, -1);
      stop_fighting(tika);
      stop_fighting(caramon);
      GET_HITROLL(tika)=temp;
    }
  }
  return FALSE;
}
SPECIAL(tanis)
{
  struct char_data *porthios, *tanis=(struct char_data *)me;
  if((cmd==0) && tanis && !number(0, 5) && !FIGHTING(tanis)) {
    for(porthios=world[tanis->in_room].people; porthios; porthios=porthios->next_in_room) {
      if(IS_NPC(porthios) && (GET_MOB_VNUM(porthios)==17950))
        break;
    }
    if(porthios) {
      if(number(0, 1))
        act("$n stares coolly at $N.", FALSE, tanis, NULL, porthios, TO_ROOM);
      else
        act("$n stares coolly at $N.", FALSE, porthios, NULL, tanis, TO_ROOM);
    }
  }
  return FALSE;
}
SPECIAL(dragonlance_dragon)
{
  struct char_data *tch, *dragon=(struct char_data *)me;
  struct obj_data *obj;
  int found=0;
  if((cmd==0) && me) {
    for(tch=world[dragon->in_room].people; tch; tch=tch->next_in_room) {
      if(GET_EQ(tch, WEAR_HOLD) && (GET_OBJ_VNUM(GET_EQ(tch, WEAR_HOLD))==17983)) {
        found=1;
        break;
      }
      for(obj=tch->carrying; obj; obj=obj->next_content) {
        if(GET_OBJ_VNUM(obj)==17983) {
          found=1;
          break;
        }
      }
      if(found)
        break;
    }
    if(!found) {
      for(obj=world[dragon->in_room].contents; obj; obj=obj->next_content) {
        if(GET_OBJ_VNUM(obj)==17983) {
          found=1;
          break;
        }
      }
    }
    if(found) {
      GET_AC(dragon)=-220;
      REMOVE_BIT(MOB_FLAGS(dragon), MOB_AGGR_NEUTRAL);
      if(GET_MOB_VNUM(dragon) < 17979)
        REMOVE_BIT(MOB_FLAGS(dragon), MOB_AGGR_GOOD);
      else
        REMOVE_BIT(MOB_FLAGS(dragon), MOB_AGGR_EVIL);
    }
    else {
      GET_AC(dragon)=-370;
      SET_BIT(MOB_FLAGS(dragon), MOB_AGGR_NEUTRAL);
      if(GET_MOB_VNUM(dragon) < 17979)
        SET_BIT(MOB_FLAGS(dragon), MOB_AGGR_GOOD);
      else
        SET_BIT(MOB_FLAGS(dragon), MOB_AGGR_EVIL);
      if(FIGHTING(dragon)) {
        if(dragon->mob_specials.spec_proc>0)
          return(spec_proc_table[dragon->mob_specials.spec_proc](ch, me, cmd, argument));
      }
    }
  }
  return FALSE;
}
SPECIAL(tasslehoff)
{
  struct char_data *tch, *mob=(struct char_data *)me;
  struct obj_data *obj, *tobj;
  int players=0, max_players=0, dir=-1, i, j, num_items;
  if((cmd>0)&&(cmd<=NUM_OF_DIRS+4)) {
    if(FIGHTING(mob))
      return FALSE;
    if(ch && (!IS_NPC(ch)) && (GET_LEVEL(ch) < LVL_HERO)) {
      if(cmd > NUM_OF_DIRS)
        cmd-=4;
      cmd--;
      if(CAN_GO(mob, cmd) && ((zone_table[world[EXIT(mob, cmd)->to_room].zone].number==179) ||
          (zone_table[world[EXIT(mob, cmd)->to_room].zone].number==181)) &&
         (!ROOM_FLAGGED(EXIT(mob, cmd)->to_room, ROOM_DEATH)))
        perform_move(mob, cmd, 0);
    }
  }
  else if(cmd==0) {
    if(!FIGHTING(mob)) {
      if(mob->in_room <= 1)
        return FALSE;
      for(tch=world[mob->in_room].people; tch; tch=tch->next_in_room) {
        if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
          players++;
      }
      if(players==0) {
        for(i=0; i<NUM_OF_DIRS; i++) {
          players=0;
          if(CAN_GO(mob, i) && ((zone_table[world[EXIT(mob, i)->to_room].zone].number==179) ||
              (zone_table[world[EXIT(mob, i)->to_room].zone].number==181)) &&
             (!ROOM_FLAGGED(EXIT(mob, i)->to_room, ROOM_DEATH))) {
            for(players=0, tch=world[EXIT(mob, i)->to_room].people; tch; tch=tch->next_in_room) {
              if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
                players++;
            }
            if(players > max_players) {
              max_players=players;
              dir=i;
            }
          }
        }
        if(dir >= 0) {
          perform_move(mob, dir, 0);
          return TRUE;
        }
        else {
          if(number(0, 3) < 2) {
            if((i=get_dir(mob))<NUM_OF_DIRS) {
              if(((zone_table[world[EXIT(mob, i)->to_room].zone].number==179) ||
                 (zone_table[world[EXIT(mob, i)->to_room].zone].number==181)) &&
                 (!ROOM_FLAGGED(EXIT(mob, i)->to_room, ROOM_DEATH)))
                perform_move(mob, i, 0);
              return TRUE;
            }
          }
        }
      }
      else {
        if(number(1, 2) == 2) {
          j=number(1, players);
          for(i=0, tch=world[mob->in_room].people; tch; tch=tch->next_in_room) {
            if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_HERO))
              i++;
            if(i==j)
              break;
          }
          if(tch && (number(0, 99) < GET_LEVEL(tch)-30)) {
            for(num_items=0, i=0; i<NUM_WEARS; i++) {
              if(GET_EQ(tch, i) && (GET_OBJ_TYPE(GET_EQ(tch, i))==ITEM_CONTAINER)) {
                for(tobj=GET_EQ(tch, i)->contains; tobj; tobj=tobj->next_content)
                  num_items++;
              }
            }
            for(obj=tch->carrying; obj; obj=obj->next_content) {
              if(GET_OBJ_TYPE(obj)==ITEM_CONTAINER) {
                for(tobj=obj->contains; tobj; tobj=tobj->next_content)
                  num_items++;
              }
              else
                num_items++;
            }
            switch(i=number(1, num_items+10)) {
            case 1:
              obj=GET_EQ(tch, WEAR_FINGER_R);
              break;
            case 2:
              obj=GET_EQ(tch, WEAR_FINGER_L);
              break;
            case 3:
              obj=GET_EQ(tch, WEAR_NECK_1);
              break;
            case 4:
              obj=GET_EQ(tch, WEAR_NECK_2);
              break;
            case 5:
              obj=GET_EQ(tch, WEAR_HEAD);
              break;
            case 6:
              obj=GET_EQ(tch, WEAR_FEET);
              break;
            case 7:
              obj=GET_EQ(tch, WEAR_HANDS);
              break;
            case 8:
              obj=GET_EQ(tch, WEAR_WAIST);
              break;
            case 9:
              obj=GET_EQ(tch, WEAR_WRIST_R);
              break;
            case 10:
              obj=GET_EQ(tch, WEAR_WRIST_L);
              break;
            default:
              i-=10;
              for(num_items=0, j=0; j<NUM_WEARS; j++) {
                if(GET_EQ(tch, j) && (GET_OBJ_TYPE(GET_EQ(tch, j))==ITEM_CONTAINER)) {
                  for(tobj=GET_EQ(tch, j)->contains; tobj; tobj=tobj->next_content) {
                    num_items++;
                    if(i==num_items)
                      break;
                  }
                  if(i==num_items) {
                    obj=tobj;
                    break;
                  }
                }
              }
              if(num_items < i) {
                for(obj=tch->carrying; obj; obj=obj->next_content) {
                  if(GET_OBJ_TYPE(obj)==ITEM_CONTAINER) {
                    for(tobj=obj->contains; tobj; tobj=tobj->next_content) {
                      num_items++;
                      if(i==num_items) {
                        obj=tobj;
                        break;
                      }
                    }
                  }
                  else
                    num_items++;
                  if(i==num_items)
                    break;
                }
              }
              break;
            }
            if(obj) {
              switch(number(0, 5)) {
              case 0:
                do_say(mob, "I found it on the floor.", 0, 0);
                break;
              case 1:
                do_say(mob, "It fell in my pouch by accident.", 0, 0);
                break;
              case 2:
                do_say(mob, "Are you sure it's yours?", 0, 0);
                break;
              case 3:
                do_say(mob, "I thought you didn't want it anymore.", 0, 0);
                break;
              case 4:
                do_say(mob, "You just walked off and left it.", 0, 0);
                break;
              case 5:
                do_say(mob, "I was going to wash it and give it back to you.", 0, 0);
                break;
              }
              if (obj->worn_by != NULL)
                if (unequip_char(obj->worn_by, obj->worn_on) != obj)
                  log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
              if (obj->carried_by)
                obj_from_char(obj);
              if (obj->in_obj)
                obj_from_obj(obj);
              obj_to_char(obj, mob);
            }
          }
          if(number(0, 1))
            return TRUE;
        }
      }
    }
    else {
      if(!number(0, 4)) {
        switch(number(1, 5)) {
        case 1:
          do_say(mob, "Your father was a gully dwarf!", 0, 0);
          break;
        case 2:
          do_say(mob, "My grandmother can swing a sword better than you!", 0, 0);
          break;
        case 3:
          do_say(mob, "Why don't you take that nasty weapon and stick it where the sun don't shine!", 0, 0);
          break;
        case 4:
          do_say(mob, "Hehe, your underpants are showing. You wear pink polkadots?!?", 0, 0);
          break;
        case 5:
          do_say(mob, "I hear your mother is a 'special friend' of the ogres.", 0, 0);
          break;
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(chaos)
{
  struct char_data *tch, *chaos=(struct char_data *)me;
  struct obj_data *obj;
  struct affected_type af;
  int graygem;
  static int dead=0;
  if((cmd==0) && chaos && FIGHTING(chaos)) {
    if(GET_HIT(chaos) < 4000) {
      dead=MIN(dead+4000-GET_HIT(chaos), 5000);
      GET_HIT(chaos)=4000;
    }
    else
      dead=0;
    for(tch=world[chaos->in_room].people; tch; tch=tch->next_in_room) {
      if(GET_EQ(tch, WEAR_WIELD) && (GET_OBJ_VNUM(GET_EQ(tch, WEAR_WIELD))==17953)) {
        af.type=SPELL_ENCHANT_WEAPON;
        af.bitvector=0;
        af.location=APPLY_HITROLL;
        af.duration=1;
        af.modifier=5;
        affect_join(tch, &af, FALSE, FALSE, FALSE, FALSE);
      }
    }
    if(!spec_proc_table[chaos->mob_specials.spec_proc](ch, me, cmd, argument)) {
      if(number(0, 1))
        return(spec_proc_table[chaos->mob_specials.spec_proc](ch, me, cmd, argument));
    }
    else
      return TRUE;
  }
  else if(cmd==-1) {
    if(number(0, 99) < ((dead-4000)/10)) {
      act("$n plunges $p deep into $N and blood spurts forth.", FALSE,
          ((struct obj_data *)me)->worn_by, (struct obj_data *)me, ch, TO_ROOM);
      act("You plunge $p deep into $N and blood spurts forth.", FALSE,
          ((struct obj_data *)me)->worn_by, (struct obj_data *)me, ch, TO_CHAR);
      for(tch=world[ch->in_room].people; tch; tch=tch->next_in_room) {
        if(ch==tch)
          continue;
        graygem=0;
        for(obj=tch->carrying; obj; obj=obj->next_content) {
          if(GET_OBJ_VNUM(obj)==17970)
            graygem|=1;
          if(GET_OBJ_VNUM(obj)==17971)
            graygem|=2;
        }
        if(graygem==3) {
          act("$N catches a drop of $n's blood in the halves of the graygem\r\n"
              "and closes it. $n screams in rage as $e realizes he has been defeated.",
              FALSE, ch, NULL, tch, TO_ROOM);
          damage(tch, ch, 10000, TYPE_SILENT);
          dead=0;
          break;
        }
      }
    }
  }
  else if(CMD_IS("use") || CMD_IS("recite")) {
    if((obj=GET_EQ(ch, WEAR_HOLD)) && GET_EQ(ch, WEAR_LIGHT) &&
       (GET_OBJ_VNUM(obj)==17966) && (GET_OBJ_VNUM(GET_EQ(ch, WEAR_LIGHT))==17990)) {
      two_arguments(argument, arg, buf);
      if(isname(arg, obj->name)) {
        if((chaos==get_char_room_vis(ch, buf)) && (!AFF_FLAGGED(chaos, AFF_BLIND))) {
          act("$N holds Magius' spellbook in one hand and his staff in the\r\n"
              "other. $E begins reading aloud the words of a powerful spell.\r\n"
              "Soon a white glow consumes the book and spreads to the staff,\r\n"
              "bursting forth from the crystal on top in a beam so bright that\r\n"
              "it makes $n seem dim. The beam hits $n full in the eyes and\r\n"
              "follows $s every move, weakening and blinding $m.", FALSE,
              chaos, NULL, ch, TO_ROOM);
          GET_AC(chaos)+=50;
          GET_MR(chaos)-=100;
          GET_HITROLL(chaos)-=4;
          GET_SAVE(chaos, 0)=5;
          GET_SAVE(chaos, 1)=5;
          GET_SAVE(chaos, 2)=5;
          GET_SAVE(chaos, 3)=5;
          GET_SAVE(chaos, 4)=5;
          AFF_FLAGS(chaos)|=AFF_BLIND;
          chaos->char_specials.saved.affected_by|=AFF_BLIND;
          damage(chaos, ch, 5, TYPE_SILENT);
          extract_obj(obj);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */
SPECIAL(bank)
{
  long amount;
  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %ld coins.\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if ((amount = atol(argument)) <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %ld coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %ld coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else
    return 0;
}
SPECIAL(damage_eq)
{
  struct char_data *to_who, *found_char = NULL;
  struct obj_data *obj=(struct obj_data *)me, *found_obj = NULL;
  int dam, i, pos=0;
  if((cmd == -1) && (me)) {
    if(GET_OBJ_TYPE(obj) != ITEM_DAMAGEABLE)
      return FALSE;
    dam=atoi(argument);
    if(dam > 70000)
      dam = 70000;
    if(GET_OBJ_VAL(obj, 0) < 1)
      GET_OBJ_VAL(obj, 0)=1;
    if(dam < GET_OBJ_VAL(obj, 0)) {
      GET_OBJ_VAL(obj, 1) += dam>>1;
      GET_OBJ_VAL(obj, 1) = MIN(GET_OBJ_VAL(obj, 1), 70000);
    }
    else {
      if(number(1, 100) <= (GET_OBJ_VAL(obj, 1)/(7*GET_OBJ_VAL(obj, 0)))) {
        GET_OBJ_VAL(obj, 2) += 1;
        GET_OBJ_VAL(obj, 1) = 0;
        to_who=obj->worn_by;
        if(GET_OBJ_VAL(obj, 2) >= 10) {
          if(to_who)
            act("$p is damaged beyond repair.", FALSE, to_who, obj, NULL, TO_CHAR);
          extract_obj(obj);
        }
        else {
          if(to_who) {
            act("$p is damaged.", FALSE, to_who, obj, NULL, TO_CHAR);
            pos=obj->worn_on;
            unequip_char(to_who, pos);
          }
          if((GET_OBJ_RNUM(obj) >= 0) && (GET_OBJ_TYPE(obj_proto+GET_OBJ_RNUM(obj))==ITEM_DAMAGEABLE)) {
            for(i=0; i < MAX_OBJ_AFFECT; i++) {
              obj->affected[i].modifier = ((obj_proto[GET_OBJ_RNUM(obj)].affected[i].modifier+0.5) * (10.0-GET_OBJ_VAL(obj, 2)))/10.0;
            }
          }
          if(to_who) {
            equip_char(to_who, obj, pos);
          }
        }
      }
      GET_OBJ_VAL(obj, 1) += dam;
      GET_OBJ_VAL(obj, 1) = MIN(GET_OBJ_VAL(obj, 1), 70000);
    }
    return TRUE;
  }
  if((CMD_IS("look") || CMD_IS("examine")) && obj) {
    one_argument(argument, arg);
    if(*arg) {
      generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
                   FIND_CHAR_ROOM, ch, &found_char, &found_obj);
      if(found_obj==obj) {
        switch(GET_OBJ_VAL(obj, 2)) {
        case 0:
          act("$p is in excellent shape.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 1:
          act("$p is in good shape.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 2:
          act("$p is in fair shape.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 3:
          act("$p has some nicks and scratches.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 4:
          act("$p is a little damaged.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 5:
          act("$p is damaged.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 6:
          act("$p is in poor shape.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 7:
          act("$p is in bad shape.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 8:
          act("$p is in very bad shape.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        case 9:
          act("$p is falling to pieces.", FALSE, ch, obj, NULL, TO_CHAR);
          break;
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(evaporating_potions)
{
  if(me && ch && (!IS_NPC(ch)) && (GET_LEVEL(ch) > 59) && (GET_LEVEL(ch) < LVL_HERO)) {
    extract_obj((struct obj_data *)me);
    act("$p crumbles in your hands.", FALSE, ch, (struct obj_data *)me, NULL, TO_CHAR);
  }
  return FALSE;
}
SPECIAL(mana_potions_low)
{
  struct obj_data *obj;
  if(me && ch && (!IS_NPC(ch)) && (GET_LEVEL(ch) > 59) && (GET_LEVEL(ch) < LVL_HERO)) {
    extract_obj((struct obj_data *)me);
    act("$p crumbles in your hands.", FALSE, ch, (struct obj_data *)me, NULL, TO_CHAR);
  }
  if(me && ch && CMD_IS("quaff")) {
    one_argument(argument, arg);
    obj = GET_EQ(ch, WEAR_HOLD);
    if(!obj || !isname(arg, obj->name))
      obj = get_obj_in_list_vis(ch, arg, ch->carrying);
    if(obj==me) {
      act("$n takes a blue potion from $s belt and drinks it.", TRUE, ch, NULL, NULL, TO_ROOM);
      send_to_char("You take a blue potion from your belt and drink it.\r\n", ch);
      GET_MANA(ch)=MIN(GET_MAX_MANA(ch), GET_MANA(ch)+dice(3, 8)+3);
      send_to_char("You feel energy course through your body.\r\n", ch);
      extract_obj(obj);
      return TRUE;
    }
  }
  return FALSE;
}
SPECIAL(mana_potions_high)
{
  struct obj_data *obj;
  if(me && ch && (!IS_NPC(ch)) && (GET_LEVEL(ch) > 59) && (GET_LEVEL(ch) < LVL_HERO)) {
    extract_obj((struct obj_data *)me);
    act("$p crumbles in your hands.", FALSE, ch, (struct obj_data *)me, NULL, TO_CHAR);
  }
  if(me && ch && CMD_IS("quaff")) {
    one_argument(argument, arg);
    obj = GET_EQ(ch, WEAR_HOLD);
    if(!obj || !isname(arg, obj->name))
      obj = get_obj_in_list_vis(ch, arg, ch->carrying);
    if(obj==me) {
      act("$n takes a blue potion from $s belt and drinks it.", TRUE, ch, NULL, NULL, TO_ROOM);
      send_to_char("You take a blue potion from your belt and drink it.\r\n", ch);
      GET_MANA(ch)=MIN(GET_MAX_MANA(ch), GET_MANA(ch)+dice(5, 8)+8);
      send_to_char("You feel energy course through your body.\r\n", ch);
      extract_obj(obj);
      return TRUE;
    }
  }
  return FALSE;
}
SPECIAL(diablo_recall)
{
  extern sh_int r_mortal_start_room;
  struct char_data *tch;
  struct obj_data *tobj;
  int location;
  char action[MAX_STRING_LENGTH];
  if(me && ch && CMD_IS("recite")) {
    argument=one_argument(argument, arg);
    tobj = GET_EQ(ch, WEAR_HOLD);
    if(!tobj || !isname(arg, tobj->name))
      tobj = get_obj_in_list_vis(ch, arg, ch->carrying);
    if(tobj==me) {
      if((location=real_room(2930)) < 0)
        location=r_mortal_start_room;
      one_argument(argument, arg);
      generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                   FIND_OBJ_EQUIP, ch, &tch, &tobj);
      tobj=(struct obj_data *)me;
      if((!*arg) || (!tch))
        tch=ch;
      if (tch == ch) {
	if (tobj->action_description && *tobj->action_description) {
          parse_action(action, tobj->action_description, TO_CHAR, TRUE);
          act(action, FALSE, ch, tobj, tch, TO_CHAR);
          parse_action(action, tobj->action_description, TO_ROOM, TRUE);
          act(action, TRUE, ch, tobj, tch, TO_NOTVICT);
        }
        else {
          act("You recite $p which dissolves.", TRUE, ch, tobj, 0, TO_CHAR);
          act("$n recites $p.", TRUE, ch, tobj, NULL, TO_ROOM);
        }
      } else {
	if (tobj->action_description && *tobj->action_description) {
          parse_action(action, tobj->action_description, TO_CHAR, FALSE);
          act(action, FALSE, ch, tobj, tch, TO_CHAR);
          parse_action(action, tobj->action_description, TO_ROOM, FALSE);
          act(action, TRUE, ch, tobj, tch, TO_NOTVICT);
          parse_action(action, tobj->action_description, TO_VICT, FALSE);
          act(action, FALSE, ch, tobj, tch, TO_VICT);
        }
	else {
          act("You recite $p which dissolves.", FALSE, ch, tobj, 0, TO_CHAR);
          act("$n recites $p.", TRUE, ch, tobj, NULL, TO_ROOM);
        }
      }
      if(ROOM_FLAGGED(tch->in_room, ROOM_NOTELEPORT)) {
        act("A mysterious force prevents $N from recalling back to safety!",
            FALSE, ch, NULL, tch, TO_CHAR);
      }
      else {
        act("$n disappears.", TRUE, tch, 0, 0, TO_ROOM);
        char_from_room(tch);
        char_to_room(tch, location);
        act("$n appears in the middle of the room.", TRUE, tch, 0, 0, TO_ROOM);
        look_at_room(tch, 0);
      }
      extract_obj(tobj);
      return TRUE;
    }
  }
  return FALSE;
}
SPECIAL(evaporating_newbie_eq)
{
  struct obj_data *obj=(struct obj_data *)me;
  if((cmd >= 0) && (obj) && (obj->worn_by)) {
    if((GET_LEVEL(obj->worn_by)>25) && (!FIGHTING(obj->worn_by))) {
      act("$p grows hazy, then evaporates into mist and blows away.", FALSE, obj->worn_by, obj, NULL, TO_CHAR);
      act("Something on $n starts giving off mist.", TRUE, obj->worn_by, obj, NULL, TO_ROOM);
      extract_obj(obj);
    }
  }
  return FALSE;
}
SPECIAL(crysania_medallion)
{
  struct obj_data *obj=(struct obj_data *)me;
  if((cmd > 0) && (cmd <= NUM_OF_DIRS+4)) {
    if(cmd > NUM_OF_DIRS)
      cmd-=4;
    cmd--;
    if(ch && me && CAN_GO(ch, cmd)) {
      if((!ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) && ROOM_FLAGGED(EXIT(ch, cmd)->to_room, ROOM_DEATH)) {
        send_to_char("Crysania's medallion pulses and begins to glow.\r\n", ch);
        GET_OBJ_EXTRA(obj) |= ITEM_GLOW;
        obj->obj_flags.bitvector = (AFF_SANCTUARY | AFF_DIVINE_PROT);
        affect_total(ch);
      }
      else if((!ROOM_FLAGGED(EXIT(ch, cmd)->to_room, ROOM_DEATH)) && IS_OBJ_STAT(obj, ITEM_GLOW)) {
        send_to_char("Crysania's medallion grows dim.\r\n", ch);
        GET_OBJ_EXTRA(obj) ^= ITEM_GLOW;
        obj->obj_flags.bitvector = 0;
        affect_total(ch);
      }
    }
  }
  else if(ch && me && IS_OBJ_STAT(obj, ITEM_GLOW) && (!ROOM_FLAGGED(ch->in_room, ROOM_DEATH))) {
    send_to_char("Crysania's medallion grows dim.\r\n", ch);
    GET_OBJ_EXTRA(obj) ^= ITEM_GLOW;
    obj->obj_flags.bitvector = 0;
    affect_total(ch);
  }
  return FALSE;
}
SPECIAL(pure_chaos)
{
  struct obj_data *obj=(struct obj_data *)me;
  int i, points=6;
  if((cmd==0) && me) {
    if(obj->worn_by) {
      for (i=0; i<MAX_OBJ_AFFECT; i++)
        affect_modify(obj->worn_by, obj->affected[i].location,
               obj->affected[i].modifier, obj->obj_flags.bitvector, FALSE);
    }
    i=number(-1, 1);
    points-=i;
    obj->affected[2].modifier=10*i;
    i=number(-1, 2);
    points-=i;
    obj->affected[3].modifier=5*i;
    i=number(-1, 1);
    points-=i;
    obj->affected[4].modifier=15*i;
    i=number(-1, 2);
    points-=i;
    obj->affected[5].modifier=10*i;
    i=number(0, points);
    obj->affected[0].modifier=20+5*i;
    obj->affected[1].modifier=3+points-i;
    if(obj->worn_by) {
      for (i=0; i<MAX_OBJ_AFFECT; i++)
        affect_modify(obj->worn_by, obj->affected[i].location,
               obj->affected[i].modifier, obj->obj_flags.bitvector, TRUE);
      affect_total(obj->worn_by);
    }
  }
  return FALSE;
}
SPECIAL(staff_of_magius)
{
  struct obj_data *obj=(struct obj_data *)me;
  struct affected_type af;
  int i;
  if((cmd==0) && me) {
    if(GET_OBJ_VAL(obj, 3) > 0)
      GET_OBJ_VAL(obj, 3)--;
    if(obj->worn_by) {
      if(!number(0, 99)) {
        switch(number(1, 5)) {
        case 1:
          strcpy(buf2, "shirak");
          break;
        case 2:
          strcpy(buf2, "dulak");
          break;
        case 3:
          strcpy(buf2, "kilat");
          break;
        case 4:
          strcpy(buf2, "birsih");
          break;
        case 5:
          strcpy(buf2, "nomar");
          break;
        }
        sprintf(buf, "$p whispers '%s' in your head.", buf2);
        act(buf, FALSE, obj->worn_by, obj, NULL, TO_CHAR | TO_SLEEP);
      }
    }
  }
  else if((CMD_IS("say") || CMD_IS("'")) && me && ch && (obj->worn_by==ch) && (GET_POS(ch) > POS_SITTING) && (ch->in_room != NOWHERE) && (GET_OBJ_TYPE(obj)==ITEM_LIGHT)) {
    if(GET_OBJ_VAL(obj, 3) < 1) {
      skip_spaces(&argument);
      if(!str_cmp(argument, "shirak")) {
        do_say(ch, argument, 0, 0);
        if(GET_OBJ_VAL(obj, 2)) {
          send_to_char("Nothing happens.\r\n", ch);
        }
        else {
          i=obj->worn_on;
          unequip_char(ch, i);
          GET_OBJ_VAL(obj, 2)=-1;
          obj->affected[0].modifier=10;
          obj->affected[1].modifier=5;
          equip_char(ch, obj, i);
          act("The crystal atop $p bursts into radiant light.", FALSE, ch, obj, NULL, TO_CHAR);
          act("The crystal atop $p bursts into radiant light.", FALSE, ch, obj, NULL, TO_ROOM);
        }
        return TRUE;
      }
      else if(!str_cmp(argument, "dulak")) {
        do_say(ch, argument, 0, 0);
        if(!GET_OBJ_VAL(obj, 2)) {
          send_to_char("Nothing happens.\r\n", ch);
        }
        else {
          act("The crystal atop $p goes dark.", FALSE, ch, obj, NULL, TO_CHAR);
          act("The crystal atop $p goes dark.", FALSE, ch, obj, NULL, TO_ROOM);
          i=obj->worn_on;
          unequip_char(ch, i);
          GET_OBJ_VAL(obj, 2)=0;
          obj->affected[0].modifier=25;
          obj->affected[1].modifier=2;
          equip_char(ch, obj, i);
        }
        return TRUE;
      }
      else if(!str_cmp(argument, "kilat")) {
        do_say(ch, argument, 0, 0);
        if(!FIGHTING(ch)) {
          send_to_char("Nothing happens.\r\n", ch);
        }
        else {
          for(i=0; i<7; i++) {
            if(!FIGHTING(ch) || (FIGHTING(ch)->in_room==NOWHERE) ||
               !call_magic(ch, FIGHTING(ch), NULL, SPELL_MAGIC_MISSILE, GET_LEVEL(ch), CAST_SPELL))
              break;
          }
          GET_OBJ_VAL(obj, 3)=1;
        }
        return TRUE;
      }
      else if(!str_cmp(argument, "birsih")) {
        do_say(ch, argument, 0, 0);
        act("$p protects you.\r\n", FALSE, ch, obj, NULL, TO_CHAR);
        af.type=SPELL_MAGIC_SHIELD;
        af.bitvector=AFF_MAGICSHIELD;
        af.location=APPLY_AC;
        af.duration=1;
        af.modifier=-100;
        affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
        GET_OBJ_VAL(obj, 3)=5;
        return TRUE;
      }
      else if(!str_cmp(argument, "nomar")) {
        do_say(ch, argument, 0, 0);
        if(GET_REMEMBER(ch, 0)<=0) {
          send_to_char("Nothing happens.\r\n", ch);
        }
        else if((i=real_room(GET_REMEMBER(ch, 0))) < 0) {
          send_to_char("That room no longer exists.\r\n", ch);
        }
        else if(zone_table[world[i].zone].closed || *zone_table[world[i].zone].locked_by) {
          send_to_char("Some immortal power prevents you from teleporting.\r\n", ch);
        }
        else if(ROOM_FLAGGED(ch->in_room, ROOM_NOTELEPORT)) {
          send_to_char("A mysterious force prevents you from leaving.\n\r", ch);
        }
        else if(ROOM_FLAGGED(i, ROOM_NORELOCATE)) {
          send_to_char("For some unknown reason, you fail.\n\r", ch);
        }
        else if(ROOM_FLAGGED(i, ROOM_GODROOM | ROOM_PRIVATE) || (world[i].zone == 0) ||
           (zone_table[world[i].zone].number == GOD_ZONE)) {
          send_to_char("Hmmmm, your target seems to be a private room.\n\r", ch);
        }
        else {
          act("$n vanishes.", TRUE, ch, 0, 0, TO_ROOM);
          char_from_room(ch);
          char_to_room(ch, i);
          act("$n appears out of thin air.", TRUE, ch, 0, 0, TO_ROOM);
          look_at_room(ch, 0);
          GET_OBJ_VAL(obj, 3)=3;
        }
        return TRUE;
      }
    }
  }
  return FALSE;
}
SPECIAL(kitiara_sword)
{
  struct obj_data *obj=(struct obj_data *)me;
  struct char_data *tch;
  struct affected_type af;
  int i;
  if((cmd==-1) && (!number(0, 20))) {
    i=atoi(argument);
    act("$p's negative energy bites deep into $n.", FALSE, ch, me, NULL, TO_ROOM);
    act("$p's negative energy bites deep into you!", FALSE, ch, me, NULL, TO_CHAR);
    if(GET_STR(ch) > 3) {
      af.type=SPELL_CURSE;
      af.bitvector=0;
      af.location=APPLY_STR;
      af.duration=24;
      af.modifier=-1;
      affect_to_char(ch, &af);
    }
    if(GET_MAX_HIT(ch) > i) {
      af.type=SPELL_CURSE;
      af.bitvector=0;
      af.location=APPLY_HIT;
      af.duration=24;
      af.modifier=-i;
      affect_to_char(ch, &af);
    }
  }
  else {
    if(obj->in_room!=NOWHERE)
      i=world[obj->in_room].number;
    else if(obj->worn_by)
      i=world[obj->worn_by->in_room].number;
    else if(obj->carried_by)
      i=world[obj->carried_by->in_room].number;
    else
      return FALSE;
    if(((i<17900)||(i>18107)||((i>17918)&&(i<18100)))&&(i!=18005)&&(i!=18006)&&(i!=18098)) {
      if(obj->worn_by) {
        tch=obj->worn_by;
        extract_obj(obj);
        if(real_object(17991)>=0) {
          obj=read_object(17991, VIRTUAL);
          equip_char(tch, obj, WEAR_WIELD);
        }
      }
      else if(obj->carried_by) {
        tch=obj->carried_by;
        extract_obj(obj);
        if(real_object(17991)>=0) {
          obj=read_object(17991, VIRTUAL);
          obj_to_char(obj, tch);
        }
      }
      else {
        i=obj->in_room;
        extract_obj(obj);
        if(real_object(17991)>=0) {
          obj=read_object(17991, VIRTUAL);
          obj_to_room(obj, i);
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(kitiara_stick)
{
  struct obj_data *obj=(struct obj_data *)me;
  struct char_data *tch;
  int i;
  if(cmd>=0) {
    if(obj->in_room!=NOWHERE)
      i=world[obj->in_room].number;
    else if(obj->worn_by)
      i=world[obj->worn_by->in_room].number;
    else if(obj->carried_by)
      i=world[obj->carried_by->in_room].number;
    else
      return FALSE;
    if(((i>=17900)&&(i<=17918))||((i>=18100)&&(i<=18107))||(i==18005)||(i==18006)||(i==18098)) {
      if(obj->worn_by) {
        tch=obj->worn_by;
        extract_obj(obj);
        if(real_object(17989)>=0) {
          obj=read_object(17989, VIRTUAL);
          equip_char(tch, obj, WEAR_WIELD);
        }
      }
      else if(obj->carried_by) {
        tch=obj->carried_by;
        extract_obj(obj);
        if(real_object(17989)>=0) {
          obj=read_object(17989, VIRTUAL);
          obj_to_char(obj, tch);
        }
      }
      else {
        i=obj->in_room;
        extract_obj(obj);
        if(real_object(17989)>=0) {
          obj=read_object(17989, VIRTUAL);
          obj_to_room(obj, i);
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(rabbitslayer)
{
  struct obj_data *obj=(struct obj_data *)me;
  if((cmd==-1) && (world[obj->worn_by->in_room].number==18098))
    return(chaos(ch, me, cmd, argument));
  else
    return FALSE;
}
/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */
SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;
  struct room_data *here=(struct room_data *)me;
  char *fname(char *namelist);
  for (k = here->contents; k; k = here->contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }
  if (!CMD_IS("drop"))
    return 0;
  do_drop(ch, argument, cmd, 0);
  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }
  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);
    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return 1;
}
#define PET_PRICE(pet) (GET_EXP(pet) * 3)
SPECIAL(pet_shops)
{
  int charmed_levels=0, num_charmies=0;
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;
  struct follow_type *f;
  extern int max_charmies;
  if(!ch)
    return FALSE;
  if(IS_NPC(ch))
    return FALSE;
  if((pet_room = real_room(((struct room_data *)me)->number+1)) < 0) {
    send_to_char("The pet shop is broken.\r\n", ch);
    return FALSE;
  }
  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      if(!IS_NPC(pet))
        continue;
      sprintf(buf, "%8ld - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {
    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);
    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if(!IS_NPC(pet)) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    for(f = ch->followers; f; f = f->next) {
      if(IS_AFFECTED(f->follower, AFF_CHARM)) {
        charmed_levels += GET_LEVEL(f->follower);
        num_charmies++;
      }
    }
    if(((charmed_levels+GET_LEVEL(pet)) > GET_LEVEL(ch)) || (num_charmies >= max_charmies)) {
      send_to_char("You can't have any more pets!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= PET_PRICE(pet);
    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);
    SET_BIT(pet->char_specials.saved.affected_by, AFF_CHARM);
    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);
      sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);
    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;
    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);
    return 1;
  }
  /* All commands except list and buy */
  return 0;
}
struct lag {
  long id;		/* id num of char		*/
  long timer;		/* time before command goes	*/
  struct lag *next;	/* next in list			*/
};
SPECIAL(lag_trap)
{
  static struct lag *lag_list=NULL;
  static long prev_time=0;
  struct lag *index, *index2, *nexti;
  struct char_data *tch;
  struct room_data *here=(struct room_data *)me;
  if(cmd<1)
    return FALSE;
  if((GET_LEVEL(ch) >= LVL_HERO) || (IS_NPC(ch)))
    return FALSE;
  if(prev_time!=time(0)) {
    for(index=lag_list; index; index=nexti) {
      nexti=index->next;
      index->timer-=time(0)-prev_time;
      if(index->timer <= 0) {
        index->timer=0;
        for(tch=here->people; tch&&(GET_IDNUM(tch)!=index->id); tch=tch->next_in_room);
        if(!tch) {
          if(index==lag_list) {
            lag_list=index->next;
            free(index);
          }
          else {
            for(index2=lag_list; index2&&(index2->next!=index); index2=index2->next);
            if(index2) {
              index2->next=index->next;
            }
            free(index);
          }
        }
      }
    }
  }
  for(index=lag_list; index&&(GET_IDNUM(ch)!=index->id); index=index->next);
  if(index) {
    if(index->timer)
      send_to_char("Your screen is frozen, you are unable to do anything, but will be fine soon!\r\n", ch);
    else {
      if(index==lag_list) {
        lag_list=index->next;
        free(index);
      }
      else {
        for(index2=lag_list; index2&&(index2->next!=index); index2=index2->next);
        if(index2) {
          index2->next=index->next;
        }
        free(index);
      }
      prev_time=time(0);
      return FALSE;
    }
  }
  else {
    send_to_char("Your screen is frozen, you are unable to do anything, but you will be fine soon!\r\n", ch);
    CREATE(index, struct lag, 1);
    index->id=GET_IDNUM(ch);
    index->timer=20;
    index->next=lag_list;
    lag_list=index;
  }
  prev_time=time(0);
  return TRUE;
}
SPECIAL(poppy_field)
{
  if(!IS_MOVE(cmd))
    return FALSE;
  call_magic(ch, ch, NULL, SPELL_SLEEP, 1, CAST_SPELL);
  do_move(ch, arg, cmd, 0);
  return TRUE;
}
void hit_bottom(struct char_data *ch)
{
  int dam;
  if(IS_NPC(ch))
    return;
  if(GET_POS(ch)==POS_FALLING) {
    send_to_char("WOW! You finally stopped falling.\r\n", ch);
    send_to_char("Unfortunately....You've hit the ground!\r\n", ch);
    dam=(9*GET_MAX_HIT(ch))/10;
    damage(ch, ch, dam, TYPE_SUFFERING);
    if(GET_POS(ch)>POS_STUNNED)
      GET_POS(ch)=POS_STUNNED;
  }
  return;
}
SPECIAL(falling_down)
{
  int special(struct char_data *ch, int cmd, char *arg);
  if(cmd<1)
    return FALSE;
  if(IS_NPC(ch))
    return FALSE;
  if((CMD_IS("sleep")||CMD_IS("rest")||CMD_IS("sit")) && (GET_POS(ch)!=POS_FALLING)) {
    send_to_char("You can't do that here!\r\n", ch);
    return TRUE;
  }
  if(!IS_MOVE(cmd) && (GET_POS(ch)!=POS_FALLING))
    return FALSE;
  if(GET_POS(ch)==POS_FALLING) {
    if(GET_DEX(ch) > number(8, 16)) {
      send_to_char("You barely manage to grab a handhold!\r\n", ch);
      send_to_char("I'd be a little more careful if I were you.\r\n", ch);
      GET_POS(ch)=POS_STANDING;
      return TRUE;
    }
    else {
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("V\r\n",ch);
      send_to_char("You couldn't recover. You're still falling!\r\n",ch);
      if(!world[ch->in_room].dir_option[SCMD_DOWN]) {
        hit_bottom(ch);
      }
      else {
        do_move(ch, argument, SCMD_DOWN, 0);
        special(ch, SCMD_DOWN, argument);
      }
      return TRUE;
    }
  }
  else if(GET_POS(ch)==POS_STANDING) {
    if(GET_DEX(ch) < number(6, 14)) {
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("|\r\n",ch);
      send_to_char("V\r\n",ch);
      send_to_char("As you reach for a new handhold, your left foot slips and you begin to fall!\r\n", ch);
      GET_POS(ch)=POS_FALLING;
      do_move(ch, argument, SCMD_DOWN, 0);
      special(ch, SCMD_DOWN, argument);
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  return FALSE;
}
SPECIAL(museum_tour1)
{
  if(CMD_IS("down") && (!IS_NPC(ch)) && (GET_LEVEL(ch) < LVL_HERO)) {
    REMOVE_BIT(PLR_FLAGS(ch), PLR_TOURING);
  }
  return FALSE;
}
SPECIAL(museum_tour2)
{
  if(CMD_IS("west") && (!IS_NPC(ch)) && (GET_LEVEL(ch) < LVL_HERO)) {
    SET_BIT(PLR_FLAGS(ch), PLR_TOURING);
  }
  return FALSE;
}
SPECIAL(museum_tour3)
{
  if(CMD_IS("east") && (!IS_NPC(ch)) && (GET_LEVEL(ch) < LVL_HERO)) {
    REMOVE_BIT(PLR_FLAGS(ch), PLR_TOURING);
  }
  return FALSE;
}
SPECIAL(raistlin_ghost)
{
  struct char_data *mob;
  struct obj_data *obj;
  int dotmode, found=0, i;
  if((CMD_IS("get") || CMD_IS("take")) && ch) {
    two_arguments(argument, buf1, buf2);
    if(!*buf2) {
      dotmode=find_all_dots(buf1);
      switch(dotmode) {
      case FIND_INDIV:
        if(get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents))
          found=1;
        break;
      case FIND_ALLDOT:
        for(obj=world[ch->in_room].contents; obj; obj=obj->next_content) {
          if(CAN_SEE_OBJ(ch, obj) && isname(buf1, obj->name)) {
            found=1;
            break;
          }
        }
        break;
      case FIND_ALL:
        if(world[ch->in_room].contents)
          found=1;
        break;
      }
      if(found) {
        if((i=real_mobile(17953))>=0) {
          mob=read_mobile(i, REAL);
          char_to_room(mob, ch->in_room);
          act("$n appears out of nowhere!", FALSE, mob, NULL, NULL, TO_ROOM);
          hit(mob, ch, -1);
          if(ch->in_room==NOWHERE)
            return TRUE;
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(waywreth_forest)
{
  int to, dir1, dir2;
  if((cmd==0) && (!number(0, 5))) {
    dir1=number(0, 3);
    dir2=number(0, 3);
    to=((struct room_data *)me)->dir_option[dir1]->to_room;
    ((struct room_data *)me)->dir_option[dir1]->to_room=((struct room_data *)me)->dir_option[dir2]->to_room;
    ((struct room_data *)me)->dir_option[dir2]->to_room=to;
  }
  return FALSE;
}
SPECIAL(wight_and_beard)
{
  struct char_data *mob;
  struct obj_data *obj;
  int dotmode, found=0;
  if((CMD_IS("get") || CMD_IS("take")) && ch) {
    two_arguments(argument, buf1, buf2);
    if(!*buf2) {
      dotmode=find_all_dots(buf1);
      switch(dotmode) {
      case FIND_INDIV:
        if((obj=get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents))) {
          if((GET_OBJ_VNUM(obj) >= 17973) && (GET_OBJ_VNUM(obj) <= 17982))
            found=1;
        }
        break;
      case FIND_ALLDOT:
        for(obj=world[ch->in_room].contents; obj; obj=obj->next_content) {
          if(CAN_SEE_OBJ(ch, obj) && isname(buf1, obj->name)) {
            if((GET_OBJ_VNUM(obj) >= 17973) && (GET_OBJ_VNUM(obj) <= 17982)) {
              found=1;
              break;
            }
          }
        }
        break;
      case FIND_ALL:
        for(obj=world[ch->in_room].contents; obj; obj=obj->next_content) {
          if(CAN_SEE_OBJ(ch, obj)) {
            if((GET_OBJ_VNUM(obj) >= 17973) && (GET_OBJ_VNUM(obj) <= 17982)) {
              found=1;
              break;
            }
          }
        }
        break;
      }
      if(found) {
        for(mob=world[ch->in_room].people; mob; mob=mob->next_in_room) {
          if(IS_NPC(mob) && (GET_MOB_VNUM(mob)==17968)) {
            hit(mob, ch, -1);
            if(ch->in_room==NOWHERE)
              return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}
SPECIAL(slow_decay)
{
  static int inc=0;
  struct room_data *room=(struct room_data *)me;
  struct obj_data *obj;
  if((cmd==0) && me) {
    inc++;
    if(inc==3) {
      inc=0;
    }
    else {
      for(obj=room->contents; obj; obj=obj->next_content) {
        GET_OBJ_TIMER(obj)++;
      }
    }
  }
  return FALSE;
}
#define SHUTTLE_TO_TATOOINE		0
#define SHUTTLE_TO_DESERT		1
#define SHUTTLE_AT_TATOOINE		2
#define SHUTTLE_AT_DESERT		3
#define SHUTTLE_APPROACHING_DESERT	4
#define SHUTTLE_APPROACHING_TATOOINE	5
#define SHUTTLE_DERAILED		6
#define SHUTTLE_GATE     24630   /* This is where the gate of the shuttle is*/
#define DESERT_STATION   24508  /* The location of the Desert station */
#define STARW_STATION    24511  /* The location of the Star Wars station */
#define SHUTTLE_FARE    2000
#define SHUTTLE_TOKEN   24566    /* The vnum of the token needed */
#define OPEN_DOOR(x, y) (REMOVE_BIT(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED | EX_LOCKED))
#define CLOSE_DOOR(x, y) (SET_BIT(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED | EX_LOCKED))
#define IS_OPEN(x, y) (!IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED))
#define SET_PATH(x, y, z) (world[(x)].dir_option[(y)]->to_room = (z))
#define DIR_NAME(x) (dirs[x])
struct shuttle_data {
   int destination;
   int state;
};
struct shuttle_station_data {
   char *name;
   int room;
};
SPECIAL(shuttle_station)
{
  static struct shuttle_station_data shuttle_stations[2]=
    {{"The Eastern Desert", 24508}, {"Tatooine", 24511}};
  static int shuttle_rooms[]=
    {24630, 24692, 24693, 24694, 24695, 24696, 24697, 24698, 24699, -1};
  static struct shuttle_data shuttle;
  static int initialized=0;
  char arg[MAX_INPUT_LENGTH];
  struct char_data tch;
  struct obj_data *token;
  int i;
  if(!initialized) {
    SET_PATH(real_room(DESERT_STATION), 4, NOWHERE);
    SET_PATH(real_room(SHUTTLE_GATE), 5, NOWHERE);
    CLOSE_DOOR(real_room(DESERT_STATION), 4);
    CLOSE_DOOR(real_room(SHUTTLE_GATE), 5);
    SET_PATH(real_room(STARW_STATION), 4, NOWHERE);
    CLOSE_DOOR(real_room(STARW_STATION), 4);
    shuttle.state=SHUTTLE_TO_TATOOINE;
    shuttle.destination=STARW_STATION;
    initialized=1;
    return FALSE;
  }
  if(CMD_IS("look")) {
    skip_spaces(&argument);
    one_argument(argument, arg);
    if(!*arg)
      return FALSE;    if((!str_cmp(arg, "machine")) || (!str_cmp(arg, "token"))) {
      sprintf(buf, "This is a small token vending machine. There is a small sign on it:\r\n"
                   "   Tokens -- %d coins each.\r\n"
                   "   Type 'buy token' to buy one.\r\n"
                   "   You need a token to board the shuttle.\r\n", SHUTTLE_FARE);
      send_to_char(buf, ch);
      return TRUE;
    }
  }
  else if(CMD_IS("buy")) {
    if(GET_GOLD(ch) < SHUTTLE_FARE) {      sprintf(buf, "You need %d coins to buy a token!\r\n", SHUTTLE_FARE);
      send_to_char(buf, ch);
      return TRUE;
    }
    if(!(token=read_object(SHUTTLE_TOKEN, VIRTUAL))) {
      mudlog("Shuttle token obj does not exist!", NRM, LVL_CIMP, TRUE);
      send_to_char("Sorry, the token machine is out of order.\r\n", ch);
    }
    else {
      GET_GOLD(ch)-=SHUTTLE_FARE;
      obj_to_char(token, ch);
      sprintf(buf, "You deposit %d coins and a token comes out of the machine.\r\n"
                   "You take the token.\r\n", SHUTTLE_FARE);
      send_to_char(buf, ch);
      act("$n buys a shuttle token.", TRUE, ch, NULL, NULL, TO_ROOM);
    }
    return TRUE;
  }
  else if(CMD_IS("up")) {
    if(!CAN_GO(ch, UP)) {
      send_to_char("The shuttle isn't here, how can you board?\r\n", ch);
      return TRUE;
    }
    if(!(token=get_obj_in_list_num(real_object(SHUTTLE_TOKEN), ch->carrying))) {
      send_to_char("The captain tells you, 'You'll need a token to get on the shuttle!'\r\n", ch);
      return TRUE;
    }
    send_to_char("The captain takes a token from you.\r\n", ch);
    extract_obj(token);
    act("$n steps through the shuttle gates and enters the shuttle.", FALSE, ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(SHUTTLE_GATE));
    act("$n enters through the shuttle gates.\r\n", FALSE, ch, NULL, NULL, TO_ROOM);
    look_at_room(ch, 0);
    return TRUE;
  }
  else if((!cmd)&&(((struct room_data *)me)->number==DESERT_STATION)) {
    switch(shuttle.state) {
    case SHUTTLE_TO_TATOOINE:
    case SHUTTLE_TO_DESERT:
      for (i=0; shuttle_rooms[i] != -1; i++) {
        tch.in_room=real_room(shuttle_rooms[i]);
        act("Yahoo!! We made it out of hyperspace!! Get ready to land, we just entered the\n\r"
            "planets atmosphere!", FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      }
      i=((shuttle.destination==DESERT_STATION)?0:1);
      if(i)
        shuttle.state=SHUTTLE_APPROACHING_TATOOINE;
      else
        shuttle.state=SHUTTLE_APPROACHING_DESERT;
      sprintf(buf, "The shuttle heading toward %s approaches in the distance.\r\n", shuttle_stations[i].name);
      tch.in_room=real_room(shuttle.destination);
      act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      break;
    case SHUTTLE_APPROACHING_DESERT:
      shuttle.state=SHUTTLE_AT_DESERT;
      sprintf(buf, "A loudspeaker screams, 'The shuttle to %s is now boarding!'", shuttle_stations[1].name);
      tch.in_room=real_room(shuttle.destination);
      act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      act("The gates of the shuttle open.", FALSE, &tch, NULL, NULL, TO_ROOM);
      tch.in_room=real_room(SHUTTLE_GATE);
      act("The gates of the shuttle open.", FALSE, &tch, NULL, NULL, TO_ROOM);
      sprintf(buf, "Welcome back to %s! Please exit the shuttle promptly, and visit us again!", shuttle_stations[0].name);
      for(i=0; shuttle_rooms[i] != -1; i++) {
        tch.in_room=real_room(shuttle_rooms[i]);
        act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      }
      SET_PATH(real_room(SHUTTLE_GATE), 5, real_room(shuttle.destination));
      SET_PATH(real_room(shuttle.destination), 4, real_room(SHUTTLE_GATE));
      OPEN_DOOR(real_room(SHUTTLE_GATE), 5);
      OPEN_DOOR(real_room(shuttle.destination), 4);
      shuttle.destination=STARW_STATION;
      break;
    case SHUTTLE_APPROACHING_TATOOINE:
      shuttle.state=SHUTTLE_AT_TATOOINE;
      sprintf(buf, "A loudspeaker screams, 'The shuttle to %s is now boarding!'", shuttle_stations[0].name);
      tch.in_room=real_room(shuttle.destination);
      act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      act("The gates of the shuttle open.", FALSE, &tch, NULL, NULL, TO_ROOM);
      tch.in_room=real_room(SHUTTLE_GATE);
      act("The gates of the shuttle open.", FALSE, &tch, NULL, NULL, TO_ROOM);
      sprintf(buf, "Well we made it! Please exit the shuttle promptly, and enjoy your adventures in\r\n%s!", shuttle_stations[1].name);
      for(i=0; shuttle_rooms[i] != -1; i++) {
        tch.in_room=real_room(shuttle_rooms[i]);
        act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      }
      SET_PATH(real_room(SHUTTLE_GATE), 5, real_room(shuttle.destination));
      SET_PATH(real_room(shuttle.destination), 4, real_room(SHUTTLE_GATE));
      OPEN_DOOR(real_room(SHUTTLE_GATE), 5);
      OPEN_DOOR(real_room(shuttle.destination), 4);
      shuttle.destination=DESERT_STATION;
      break;
    case SHUTTLE_AT_TATOOINE:
      shuttle.state=SHUTTLE_APPROACHING_DESERT;
      sprintf(buf, "Strap in gang! We're hijackin this baby to %s!\r\nEntering hyperspace now!", shuttle_stations[0].name);
      for(i=0; shuttle_rooms[i] != -1; i++) {
        tch.in_room=real_room(shuttle_rooms[i]);
        act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      }
      SET_PATH(real_room(STARW_STATION), 4, NOWHERE);
      SET_PATH(real_room(SHUTTLE_GATE), 5, NOWHERE);
      CLOSE_DOOR(real_room(STARW_STATION), 4);
      CLOSE_DOOR(real_room(SHUTTLE_GATE), 5);
      tch.in_room=real_room(STARW_STATION);
      act("The shuttle doors close as it vanishes into space.", FALSE, &tch, NULL, NULL, TO_ROOM);
      tch.in_room=real_room(SHUTTLE_GATE);
      act("The shuttle doors close.", FALSE, &tch, NULL, NULL, TO_ROOM);
      i=((shuttle.destination==DESERT_STATION)?0:1);
      sprintf(buf, "The shuttle heading toward %s approaches in the distance.\r\n", shuttle_stations[i].name);
      tch.in_room=real_room(shuttle.destination);
      act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      break;
    case SHUTTLE_AT_DESERT:
      shuttle.state=SHUTTLE_APPROACHING_TATOOINE;
      sprintf(buf, "Strap in gang! We're hijackin this baby to %s!\r\nEntering hyperspace now!", shuttle_stations[1].name);
      for(i=0; shuttle_rooms[i] != -1; i++) {
        tch.in_room=real_room(shuttle_rooms[i]);
        act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      }
      SET_PATH(real_room(DESERT_STATION), 4, NOWHERE);
      SET_PATH(real_room(SHUTTLE_GATE), 5, NOWHERE);
      CLOSE_DOOR(real_room(DESERT_STATION), 4);
      CLOSE_DOOR(real_room(SHUTTLE_GATE), 5);
      tch.in_room=real_room(DESERT_STATION);
      act("The shuttle doors close as it vanishes into space.", FALSE, &tch, NULL, NULL, TO_ROOM);
      tch.in_room=real_room(SHUTTLE_GATE);
      act("The shuttle doors close.", FALSE, &tch, NULL, NULL, TO_ROOM);
      i=((shuttle.destination==DESERT_STATION)?0:1);
      sprintf(buf, "The shuttle heading toward %s approaches in the distance.\r\n", shuttle_stations[i].name);
      tch.in_room=real_room(shuttle.destination);
      act(buf, FALSE, &tch, NULL, NULL, TO_ROOM | TO_SLEEP);
      break;
    }
  }
  return FALSE;
}
