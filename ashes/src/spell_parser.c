/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
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
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

char *spells[] =
{
  "!RESERVED!",			/* 0 - reserved */

  /* SPELLS */

  "armor",			/* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "ice storm",
  "color spray",		/* 10 */
  "control weather",
  "create food",
  "create water",
  "cure blind",
  "cure critical",
  "cure light",
  "curse",
  "detect alignment",
  "detect invisibility",
  "detect magic",		/* 20 */
  "detect poison",
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",		/* 30 */
  "locate object",
  "magic missile",
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",			/* 40 */
  "implosion",
  "word of recall",
  "remove poison",
  "sense life",
  "animate dead",
  "dispel good",
  "group armor",
  "group heal",
  "group recall",
  "infravision",		/* 50 */
  "waterwalk",
  "identify",
  "limited wish",
  "restore",
  "power heal",
  "spell shield",
  "aid",
  "word of death",
  "rejuvenate",
  "cure serious",		/* 60 */
  "flamestrike",
  "fly",
  "magic light",
  "haste",
  "slow",			/* 65 */
  "protection from good",
  "divine protection",
  "holy word",
  "unholy word",
  "group infravision",		/* 70 */
  "group power heal",
  "group rejuvenate",
  "group sanctuary",
  "cone of cold",
  "revitalize",			/* 75 */
  "teleport no error",
  "magic shield",
  "remember",
  "calm",
  "relocate",			/* 80 */
  "lower resistance",
  "battle recall",
  "amnesia",
  "group summon",
  "group invisibility",		/* 85 */
  "cure disease",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",			/* 90 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 95 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 100 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 105 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 110 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 115 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 120 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 125 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 130 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 135 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 140 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 145 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 150 */

  /* SKILLS */

  "backstab",			/* 151 */
  "bash",
  "hide",
  "kick",
  "pick lock",
  "UNUSED1",
  "rescue",
  "sneak",
  "steal",
  "track",			/* 160 */
  "double",
  "triple",
  "quad",
  "circle",
  "grapple",			/* 165 */
  "berserk",
  "swim",
  "healwounds",
  "camouflage",
  "filet",			/* 170 */
  "divine drink",
  "scan",
  "stuntouch",
  "befriend",
  "forage",			/* 175 */
  "silentwalk",
  "skin",
  "redirect",
  "dual backstab",
  "disarm",			/* 180 */
  "hurl",
  "whirlwind",
  "poisonblade",
  "memorize",
  "disrupt",			/* 185 */
  "throw",
  "dive",
  "mindcloud",
  "gouge",
  "dodge",			/* 190 */
  "snare",
  "featherfoot",
  "UNUSED2",
  "clairvoyance",
  "danger sense",		/* 195 */
  "feel light",
  "disintegrate",
  "telekinesis",
  "ballistic attack",
  "molecular agitation",	/* 200 */
  "complete healing",
  "death field",
  "energy containment",
  "life draining",
  "metamorphosis",		/* 205 */
  "adrenalin control",
  "biofeedback",
  "body weaponry",
  "cell adjustment",
  "chameleon power",		/* 210 */
  "displacement",
  "flesh armor",
  "graft weapon",
  "lend health",
  "probability travel",		/* 215 */
  "dimension door",
  "domination",
  "psionic blast",
  "ego whip",
  "life detection",		/* 220 */
  "psychic crush",
  "split personality",
  "cannibalize",
  "magnify",
  "stasis field",		/* 225 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 230 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 235 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 240 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 245 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 250 */

  /* OBJECT SPELLS AND NPC SPELLS/SKILLS */

  "fire breath",		/* 251 */
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath",

  "\n"				/* the end */
};


struct syllable {
  char *org;
  char *new;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"co", "dra"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"ish", "im"},
  {"light", "dies"},
  {"lim", "dor"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"pow", "zul"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};

ACMD(do_cont)
{
  struct spcontinuous *s, *temp, *next_s;
  struct char_data *tch;
  struct obj_data *tobj;
  int i;

  send_to_char("Spells with continuous affects:\r\n     Name                  Cost  Duration\r\n", ch);

  for(i=1, s=ch->char_specials.spcont; s; s=next_s, i++)
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
          i--;
          continue;
        }
      }
    }
    sprintf(buf, "%3d  %20s  %4d  %4s\r\n", i, spells[s->spspell], s->spcost, (s->sptimer >= 0 ? itoa(s->sptimer) : "-"));
    send_to_char(buf, ch);
  }

  return;
}

ACMD(do_discont)
{
  struct spcontinuous *c;
  int i, n;

  skip_spaces(&argument);
  if(!*argument) {
    send_to_char("Discontinue what spell?\r\n", ch);
    return;
  }

  any_one_arg(argument, argument);
  if((*argument >= '0')&&(*argument <= '9'))
  {
    n=atoi(argument);
    for(i=1, c=ch->char_specials.spcont; i<n && c; c=c->next, i++);
    if(!n || !c)
      send_to_char("That is not a spell number.\r\n", ch);
    else {
      c->sptimer=0;
      send_to_char(OK, ch);
    }
  }
  else if(!strcmp(argument, "all")) {
    for(c=ch->char_specials.spcont; c; c=c->next)
      c->sptimer=0;
    send_to_char(OK, ch);
  }
  else {
    for(c=ch->char_specials.spcont; c; c=c->next) {
      if(!strn_cmp(spells[c->spspell], argument, strlen(argument)))
        break;
    }
    if(c) {
      c->sptimer=0;
      send_to_char(OK, ch);
    }
    else {
      send_to_char("You aren't maintaining anything like that.\r\n", ch);
    }
  }

  return;
}

int mag_manacost(struct char_data * ch, int spellnum)
{
  int mana;

  mana = MAX(SINFO.mana_max - (SINFO.mana_change *
		    (GET_LEVEL(ch) - get_multi_skill_level(ch, spellnum))),
	     SINFO.mana_min);

  return mana;
}


/* say_spell erodes buf, buf1, buf2 */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
	            struct obj_data * tobj)
{
  char lbuf[256];
  long spell_class_bitv;
  struct char_data *i;
  int j, ofs = 0;

  long get_spell_classes(struct char_data *ch, int spell);

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  spell_class_bitv=get_spell_classes(ch, spellnum);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }

  if (tch != NULL && tch->in_room == ch->in_room) {
    if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
    else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
  } else if (tobj != NULL &&
	     ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
  else
    sprintf(lbuf, "$n utters the words, '%%s'.");

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if ((spell_class_bitv&GET_CLASS_BITVECTOR(i)) || (GET_LEVEL(i) >= LVL_HERO) || IS_NPC(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
    sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    ((spell_class_bitv&GET_CLASS_BITVECTOR(tch)) || (GET_LEVEL(tch) >= LVL_HERO) || IS_NPC(tch)) ? spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}


char *skill_name(int num)
{
  int i = 0;

  if (num <= 0) {
    if (num == -1)
      return "UNUSED";
    else
      return "UNDEFINED";
  }

  while (num && *spells[i] != '\n') {
    num--;
    i++;
  }

  if (*spells[i] != '\n')
    return spells[i];
  else
    return "UNDEFINED";
}

	 
int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*spells[++index] != '\n') {
    if (is_abbrev(name, spells[index]))
      return index;

    ok = 1;
    temp = any_one_arg(spells[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}



/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
	     struct obj_data * ovict, int spellnum, int level, int casttype)
{
  int savetype, drain=0;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return 0;

  if(!IS_NPC(caster)) {
    if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC)) {
      send_to_char("Your magic fizzles out and dies.\r\n", caster);
      act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
      return 0;
    }
    if (IS_SET(ROOM_FLAGS(caster->in_room), ROOM_PEACEFUL) &&
        (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE)) &&
        (cvict&&(!IS_NPC(cvict)))) {
      send_to_char("A flash of white light fills the room, dispelling your "
                   "violent magic!\r\n", caster);
      act("White light from no particular source suddenly fills the room, "
          "then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
      return 0;
    }
    /* For powerful spells that drain the caster */
    switch(spellnum) {
    case SPELL_REVITALIZE:
      drain=3;
      break;
    }
    if(drain&&caster->char_specials.spdrained) {
      send_to_char("You can't summon the energy to cast that spell.\r\n", caster);
      return 0;
    }
    if (SINFO.violent && (GET_LEVEL(caster) >= LVL_HERO) && (GET_LEVEL(caster) < LVL_ASST) && (!GRNT_FLAGGED(caster, GRNT_KILL))) {
      send_to_char("Immortals are forbidden to kill!\r\n", caster);
      return 0;
    }
  }
  if(SINFO.violent && cvict && (!IS_NPC(cvict)) && (caster!=cvict) && (!CAN_KILL(caster, cvict))) {
    send_to_char("That wouldn't be very nice.\r\n", caster);
    return 0;
  }

  caster->char_specials.spdrained+=drain;

  if(SINFO.violent && cvict && (spellnum <= MAX_SPELLS) && (((AFF_FLAGGED(cvict, AFF_LOWER_MR)||(spellnum==SPELL_LOWER_RESISTANCE))?(GET_MR(cvict)>>1):GET_MR(cvict)) >= number(1, 100))) {
    sprintf(buf, "Your magic is dissipated as it hits %s.\r\n", GET_NAME(cvict));
    send_to_char(buf, caster);
    return 1;
  }

  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  case CAST_ATTACK:
    savetype = SAVING_PARA;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }

  if (IS_SET(SINFO.routines, MAG_PASS_ARG)) {
    switch(spellnum) {
    case SPELL_LIMITED_WISH:		RETURN_SPELL(spell_limited_wish);
    case SPELL_TELEPORT_NO_ERROR:	RETURN_SPELL(spell_teleport_no_error);
    case SPELL_REMEMBER:		RETURN_SPELL(spell_remember);
    case SPELL_RELOCATE:		RETURN_SPELL(spell_relocate);
    case SPELL_TELEPORT:		RETURN_SPELL(spell_teleport);
    case SPELL_CONTROL_WEATHER:		RETURN_SPELL(spell_control_weather);
    case SPELL_SUMMON:			RETURN_SPELL(spell_summon);
    case SPELL_GROUP_SUMMON:		RETURN_SPELL(spell_group_summon);
    }
  }

  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    mag_damage(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_CONTINUOUS))
    mag_continuous(level, caster, cvict, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_CHARM:		MANUAL_SPELL(spell_charm); break;
    case SPELL_CREATE_WATER:	MANUAL_SPELL(spell_create_water); break;
    case SPELL_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
    case SPELL_ENCHANT_WEAPON:	MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_IDENTIFY:	MANUAL_SPELL(spell_identify); break;
    case SPELL_LOCATE_OBJECT:	MANUAL_SPELL(spell_locate_object); break;
    case SPELL_WORD_OF_RECALL:	MANUAL_SPELL(spell_recall); break;
    case SPELL_CALM:		MANUAL_SPELL(spell_calm); break;
    case SPELL_CURE_BLIND:	MANUAL_SPELL(spell_cure_blind); break;
    case SPELL_HEAL:		MANUAL_SPELL(spell_heal); break;
    case SPELL_POWER_HEAL:	MANUAL_SPELL(spell_heal); break;
    case SPELL_RESTORE:		MANUAL_SPELL(spell_heal); break;
    case SPELL_BATTLE_RECALL:	MANUAL_SPELL(spell_recall); break;
    case SPELL_AMNESIA:		MANUAL_SPELL(spell_amnesia); break;
    case FIRE_BREATH:		MANUAL_SPELL(fire_breath); break;
    case FROST_BREATH:		MANUAL_SPELL(frost_breath); break;
    case ACID_BREATH:		MANUAL_SPELL(acid_breath); break;
    }

  return 1;
}


/* Generates a string to send to act based on an action description
   and a type that specifies perspective */
void parse_action(char *to_act, char *act_desc, int type, int self)
{
  int i, j;

  if(type==TO_CHAR) {
    for(i=0, j=0; act_desc[i]; i++) {
      if(act_desc[i]=='$') {
        if((act_desc[i+1]=='n') || (act_desc[i+1]=='e') || (act_desc[i+1]=='m')) {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          i++;
        }
        else if(self && (act_desc[i+1]=='N')) {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          to_act[j++]='r';
          to_act[j++]='s';
          to_act[j++]='e';
          to_act[j++]='l';
          to_act[j++]='f';
          i++;
        }
        else if(self && ((act_desc[i+1]=='E') || (act_desc[i+1]=='M'))) {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          i++;
        }
        else if(act_desc[i+1]=='s') {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          to_act[j++]='r';
          i++;
        }
        else if(self && (act_desc[i+1]=='S')) {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          to_act[j++]='r';
          i++;
        }
        else if(act_desc[i+1]=='i') {
          to_act[j++]='a';
          to_act[j++]='r';
          to_act[j++]='e';
          i++;
        }
        else if(act_desc[i+1]=='I') {
          if(self) {
            to_act[j++]='a';
            to_act[j++]='r';
            to_act[j++]='e';
          }
          else {
            to_act[j++]='i';
            to_act[j++]='s';
          }
          i++;
        }
        else if((act_desc[i+1]=='c') || (act_desc[i+1]=='k')) {
          i++;
        }
        else if(act_desc[i+1]=='C') {
          if(!self)
            to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='K') {
          if(!self) {
            to_act[j++]='e';
            to_act[j++]='s';
          }
          i++;
        }
        else if(act_desc[i+1]=='$') {
          to_act[j++]='$';
          to_act[j++]='$';
          i++;
        }
        else
          to_act[j++]=act_desc[i];
      }
      else
        to_act[j++]=act_desc[i];
    }
    to_act[j]=0;
  }
  if(type==TO_ROOM) {
    for(i=0, j=0; act_desc[i]; i++) {
      if(act_desc[i]=='$') {
        if(self && (act_desc[i+1]=='N')) {
          to_act[j++]='$';
          to_act[j++]='M';
          to_act[j++]='s';
          to_act[j++]='e';
          to_act[j++]='l';
          to_act[j++]='f';
          i++;
        }
        else if(act_desc[i+1]=='i') {
          to_act[j++]='i';
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='I') {
          to_act[j++]='i';
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='c') {
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='k') {
          to_act[j++]='e';
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='C') {
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='K') {
          to_act[j++]='e';
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='$') {
          to_act[j++]='$';
          to_act[j++]='$';
          i++;
        }
        else
          to_act[j++]=act_desc[i];
      }
      else
        to_act[j++]=act_desc[i];
    }
    to_act[j]=0;
  }
  if(type==TO_VICT) {
    for(i=0, j=0; act_desc[i]; i++) {
      if(act_desc[i]=='$') {
        if((act_desc[i+1]=='N') || (act_desc[i+1]=='E') || (act_desc[i+1]=='M')) {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          i++;
        }
        else if(act_desc[i+1]=='S') {
          if(i==0)
            to_act[j++]='Y';
          else
            to_act[j++]='y';
          to_act[j++]='o';
          to_act[j++]='u';
          to_act[j++]='r';
          i++;
        }
        else if(act_desc[i+1]=='i') {
          if(self) {
            to_act[j++]='a';
            to_act[j++]='r';
            to_act[j++]='e';
          }
          else {
            to_act[j++]='i';
            to_act[j++]='s';
          }
          i++;
        }
        else if(act_desc[i+1]=='I') {
          to_act[j++]='a';
          to_act[j++]='r';
          to_act[j++]='e';
          i++;
        }
        else if(act_desc[i+1]=='c') {
          to_act[j++]='s';
          i++;
        }
        else if(act_desc[i+1]=='k') {
          to_act[j++]='e';
          to_act[j++]='s';
          i++;
        }
        else if((act_desc[i+1]=='C') || (act_desc[i+1]=='K')) {
          i++;
        }
        else if(act_desc[i+1]=='$') {
          to_act[j++]='$';
          to_act[j++]='$';
          i++;
        }
        else
          to_act[j++]=act_desc[i];
      }
      else
        to_act[j++]=act_desc[i];
    }
    to_act[j]=0;
  }
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
		          char *argument)
{
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;
  char action[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    if(spell_info[GET_OBJ_VAL(obj, 3)].routines&MAG_PASS_ARG)
      break;
    if (obj->action_description && *obj->action_description) {
      parse_action(action, obj->action_description, TO_CHAR, FALSE);
      act(action, FALSE, ch, obj, 0, TO_CHAR);
      parse_action(action, obj->action_description, TO_ROOM, FALSE);
      act(action, TRUE, ch, obj, 0, TO_ROOM);
    }
    else {
      act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n taps $p three times on the ground.", TRUE, ch, obj, 0, TO_ROOM);
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", TRUE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      if((!IS_NPC(ch)) || (FIGHTING(ch)))
        WAIT_STATE(ch, PULSE_VIOLENCE);
      for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	if (ch == tch)
	  continue;
	if (GET_OBJ_VAL(obj, 0))
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		     GET_OBJ_VAL(obj, 0), CAST_STAFF);
	else
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		     DEFAULT_STAFF_LVL, CAST_STAFF);
      }
    }
    break;
  case ITEM_WAND:
    if(spell_info[GET_OBJ_VAL(obj, 3)].routines&MAG_PASS_ARG)
      break;
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	if (obj->action_description && *obj->action_description) {
          parse_action(action, obj->action_description, TO_CHAR, TRUE);
          act(action, FALSE, ch, obj, tch, TO_CHAR);
          parse_action(action, obj->action_description, TO_ROOM, TRUE);
          act(action, TRUE, ch, obj, tch, TO_NOTVICT);
        }
        else {
          act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
          act("$n points $p at $mself.", TRUE, ch, obj, 0, TO_ROOM);
        }
      } else {
	if (obj->action_description && *obj->action_description) {
          parse_action(action, obj->action_description, TO_CHAR, FALSE);
          act(action, FALSE, ch, obj, tch, TO_CHAR);
          parse_action(action, obj->action_description, TO_ROOM, FALSE);
          act(action, TRUE, ch, obj, tch, TO_NOTVICT);
          parse_action(action, obj->action_description, TO_VICT, FALSE);
          act(action, FALSE, ch, obj, tch, TO_VICT);
        }
	else {
          act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
          act("$n points $p at $N.", TRUE, ch, obj, tch, TO_NOTVICT);
          act("$n points $p at you.", FALSE, ch, obj, tch, TO_VICT);
        }
      }
    } else if (tobj != NULL) {
      if (obj->action_description && *obj->action_description) {
        parse_action(action, obj->action_description, TO_CHAR, FALSE);
        act(action, FALSE, ch, obj, tobj, TO_CHAR);
        parse_action(action, obj->action_description, TO_ROOM, FALSE);
        act(action, TRUE, ch, obj, tobj, TO_ROOM);
      }
      else {
        act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
        act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
      }
    } else {
      act("On what should $p be used?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", TRUE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    if((!IS_NPC(ch)) || (FIGHTING(ch)))
      WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else {
      tch = ch;
      k = FIND_CHAR_ROOM;
    }

    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	if (obj->action_description && *obj->action_description) {
          parse_action(action, obj->action_description, TO_CHAR, TRUE);
          act(action, FALSE, ch, obj, tch, TO_CHAR);
          parse_action(action, obj->action_description, TO_ROOM, TRUE);
          act(action, TRUE, ch, obj, tch, TO_NOTVICT);
        }
        else {
          act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
          act("$n recites $p.", TRUE, ch, obj, NULL, TO_ROOM);
        }
      } else {
	if (obj->action_description && *obj->action_description) {
          parse_action(action, obj->action_description, TO_CHAR, FALSE);
          act(action, FALSE, ch, obj, tch, TO_CHAR);
          parse_action(action, obj->action_description, TO_ROOM, FALSE);
          act(action, TRUE, ch, obj, tch, TO_NOTVICT);
          parse_action(action, obj->action_description, TO_VICT, FALSE);
          act(action, FALSE, ch, obj, tch, TO_VICT);
        }
	else {
          act("You recite $p which dissolves.", FALSE, ch, obj, 0, TO_CHAR);
          act("$n recites $p.", TRUE, ch, obj, NULL, TO_ROOM);
        }
      }
    } else if (tobj != NULL) {
      if (obj->action_description && *obj->action_description) {
        parse_action(action, obj->action_description, TO_CHAR, FALSE);
        act(action, FALSE, ch, obj, tobj, TO_CHAR);
        parse_action(action, obj->action_description, TO_ROOM, FALSE);
        act(action, TRUE, ch, obj, tobj, TO_ROOM);
      }
      else {
        act("You recite $p which dissolves.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n recites $p.", TRUE, ch, obj, NULL, TO_ROOM);
      }
    } else {
      if (obj->action_description && *obj->action_description) {
        parse_action(action, obj->action_description, TO_CHAR, FALSE);
        act(action, FALSE, ch, obj, NULL, TO_CHAR);
        parse_action(action, obj->action_description, TO_ROOM, FALSE);
        act(action, TRUE, ch, obj, NULL, TO_ROOM);
      }
      else {
        act("You recite $p which dissolves.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n recites $p.", TRUE, ch, obj, NULL, TO_ROOM);
      }
    }

    if((!IS_NPC(ch)) || (FIGHTING(ch)))
      WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++) {
      if(spell_info[GET_OBJ_VAL(obj, i)].routines&MAG_PASS_ARG)
        continue;
      if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL)))
	break;
    }

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;
    if (obj->action_description && *obj->action_description) {
      parse_action(action, obj->action_description, TO_CHAR, TRUE);
      act(action, FALSE, ch, obj, tch, TO_CHAR);
      parse_action(action, obj->action_description, TO_ROOM, TRUE);
      act(action, TRUE, ch, obj, tch, TO_NOTVICT);
    }
    else {
      act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);
    }

    if((!IS_NPC(ch)) || (FIGHTING(ch)))
      WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++) {
      if(spell_info[GET_OBJ_VAL(obj, i)].routines&MAG_PASS_ARG)
        continue;
      if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_POTION)))
	break;
    }

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type in mag_objectmagic");
    break;
  }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
	           struct obj_data * tobj, int spellnum, int silent)
{
  char buf[256];

  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE) {
    sprintf(buf, "SYSERR: cast_spell trying to call spellnum %d\n", spellnum);
    log(buf);
    return 0;
  }

  if ((GET_POS(ch) < SINFO.min_position) && (!IS_NPC(ch))) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char("You dream about great magical powers.\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("You cannot concentrate while resting.\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("You can't do this sitting!\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      break;
    default:
      send_to_char("You can't do much of anything like this!\r\n", ch);
      break;
    }
    return 0;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("You are afraid you might hurt your master!\r\n", ch);
    return 0;
  }

  send_to_char(OK, ch);

  if(!silent)
    say_spell(ch, spellnum, tch, tobj);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */

ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0;

  if(!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_TOURING)) {
    send_to_char("You cannot cast while you are touring.\r\n", ch);
    return;
  }

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }
  if (!IS_NPC(ch)) {
    if (GET_LEVEL(ch) < get_multi_skill_level(ch, spellnum)) {
      send_to_char("You can't do that.\r\n", ch);
      return;
    }
    if (get_skill(ch, spellnum) == 0) {
      send_to_char("You are unfamiliar with that spell.\r\n", ch);
      return;
    }
  }

  if(ch->char_specials.spnocast) {
    send_to_char("You can't summon the energy to cast.\r\n", ch);
    return;
  }
  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD)) {
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV)) {
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && !str_cmp(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM)) {
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD)) {
      if ((tobj = get_open_obj_vis(ch, t)))
	target = TRUE;
    }
    if (target && (tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
      send_to_char("You can only cast this spell upon yourself!\r\n", ch);
      return;
    }
  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
	 IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	      "what" : "whom");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char("You cannot cast this spell upon yourself!\r\n", ch);
    return;
  }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("You can't cast this spell if you're not in a group!\r\n",ch);
    return;
  }
  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && ((GET_LEVEL(ch) < LVL_HERO) || PRF_FLAGGED(ch, PRF_AVTR)) && (!IS_NPC(ch))) {
    send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    return;
  }

  /* You throws the dice and you takes your chances.. */
  if ((number(1, 100) > get_skill(ch, spellnum)) && (!IS_NPC(ch))) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (((!(SINFO.routines&MAG_PASS_ARG))&&cast_spell(ch, tch, tobj, spellnum, subcmd)) ||
        ((SINFO.routines&MAG_PASS_ARG)&&cast_spell(ch, tch, (struct obj_data *) arg, spellnum, subcmd))) {
      if(!IS_NPC(ch)) {
        WAIT_STATE(ch, PULSE_VIOLENCE);
        if (mana > 0)
          GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
      }
    }
  }
}



void spell_level(int spell, int class, int level, char prac)
{
  char buf[256];
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    sprintf(buf, "SYSERR: attempting assign to illegal spellnum %d", spell);
    log(buf);
    return;
  }

  if (class < 0 || class >= NUM_CLASSES) {
    sprintf(buf, "SYSERR: assigning '%s' to illegal class %d",
	    skill_name(spell), class);
    log(buf);
    bad = 1;
  }

  if (level < 1 || level > (LVL_IMPL+1)) {
    sprintf(buf, "SYSERR: assigning '%s' to illegal level %d",
	    skill_name(spell), level);
    log(buf);
    bad = 1;
  }

  if (!bad) {
    spell_info[spell].min_level[class] = level;
    spell_info[spell].prac = prac;
  }
}


/* Assign the spells on boot up */
void spello(int spl, int max_mana, int min_mana, int mana_change, int minpos,
	         int targets, int violent, int routines)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_HERO;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
  spell_info[spl].prac = FALSE;
}


void unused_spell(int spl)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMPL + 1;
  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
  spell_info[spl].prac = FALSE;
}

#define skillo(skill) spello(skill, 0, 0, 0, 0, 0, 0, 0);


/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell).
 *
 * minmana :  The minimum mana this spell will take, no matter how high
 * level the caster is.
 *
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

/*
 * NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
 * In order to make this cleaner, as well as to make adding new classes
 * much easier, spell levels are now assigned in class.c.  You only need
 * a spello() call to define a new spell; to decide who gets to use a spell
 * or skill, look in class.c.  -JE 5 Feb 1996
 */

void mag_assign_spells(void)
{
  int i;

  /* Do not change the loop below */
  for (i = 1; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above */

  spello(SPELL_ARMOR, 30, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLESS, 35, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLINDNESS, 50, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_BURNING_HANDS, 50, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CALL_LIGHTNING, 75, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CHARM, 50, 5, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_CHILL_TOUCH, 60, 15, 6, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_ANIMATE_DEAD, 60, 10, 5, POS_STANDING,
	TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);

  spello(SPELL_COLOR_SPRAY, 60, 15, 6, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CONTROL_WEATHER, 75, 25, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_PASS_ARG);

  spello(SPELL_CREATE_FOOD, 40, 5, 3, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS);

  spello(SPELL_CREATE_WATER, 40, 5, 3, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_CURE_BLIND, 30, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_CURE_CRITIC, 50, 20, 4, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_REVITALIZE, 90, 60, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_LIGHT, 30, 2, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURSE, 60, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP,
	TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_DETECT_ALIGN, 36, 4, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_INVIS, 40, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_MAGIC, 40, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_POISON, 35, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_DISPEL_EVIL, 40, 15, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DISPEL_GOOD, 40, 15, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EARTHQUAKE, 50, 15, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ENCHANT_WEAPON, 150, 100, 2, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_ENERGY_DRAIN, 50, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_POINTS);

  spello(SPELL_GROUP_ARMOR, 50, 30, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_FIREBALL, 50, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GROUP_HEAL, 130, 90, 5, POS_FIGHTING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_HARM, 65, 35, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HEAL, 75, 50, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_MANUAL);

  spello(SPELL_INFRAVISION, 35, 10, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_INVISIBLE, 35, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,
	FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_LIGHTNING_BOLT, 50, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LOCATE_OBJECT, 50, 20, 2, POS_STANDING,
	TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_MAGIC_MISSILE, 35, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_POISON, 50, 10, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV | TAR_OBJ_EQUIP,
	TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_PROT_FROM_EVIL, 45, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_REMOVE_CURSE, 45, 5, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM,
	FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SANCTUARY, 125, 75, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SHOCKING_GRASP, 50, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SLEEP, 45, 15, 3, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_STRENGTH, 35, 20, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SUMMON, 75, 50, 3, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_PASS_ARG);

  spello(SPELL_WORD_OF_RECALL, 50, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_GROUP_RECALL, 150, 50, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_REMOVE_POISON, 45, 5, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,
	FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SENSE_LIFE, 50, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_IDENTIFY, 50, 15, 7, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM,
	FALSE, MAG_MANUAL);

  spello(SPELL_WATERWALK, 65, 15, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_PROT_FROM_GOOD, 45, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_WORD_OF_DEATH, 150, 50, 10, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_TELEPORT, 50, 35, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_PASS_ARG);

  spello(SPELL_TELEPORT_NO_ERROR, 110, 75, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_PASS_ARG);

  spello(SPELL_REMEMBER, 50, 15, 1, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_PASS_ARG);

  spello(SPELL_LIMITED_WISH, 150, 65, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_PASS_ARG);

  spello(SPELL_GROUP_INFRAVISION, 60, 20, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_RESTORE, 250, 170, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_MANUAL);

  spello(SPELL_POWER_HEAL, 150, 90, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_MANUAL);

  spello(SPELL_SPELL_SHIELD, 100, 65, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_AID, 50, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_GROUP_POWER_HEAL, 230, 170, 5, POS_FIGHTING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_REJUVENATE, 60, 30, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_GROUP_REJUVENATE, 100, 75, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_MAGIC_SHIELD, 125, 75, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CALM, 100, 50, 10, POS_FIGHTING,
	TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_RELOCATE, 110, 75, 5, POS_STANDING,
	TAR_CHAR_WORLD, FALSE, MAG_PASS_ARG);

  spello(SPELL_CURE_SERIOUS, 48, 12, 4, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_FLAMESTRIKE, 50, 35, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FLY, 75, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MAGIC_LIGHT, 50, 15, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_HASTE, 100, 50, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ICE_STORM, 60, 40, 2, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_GROUP_SANCTUARY, 300, 250, 3, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_SLOW, 100, 45, 8, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_DIVINE_PROTECTION, 150, 80, 10, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_HOLY_WORD, 50, 35, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_UNHOLY_WORD, 50, 35, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_IMPLOSION, 100, 40, 5, POS_STANDING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_CONE_OF_COLD, 50, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LOWER_RESISTANCE, 100, 30, 2, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_BATTLE_RECALL, 148, 50, 8, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_AMNESIA, 200, 80, 10, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_MANUAL);

  spello(SPELL_GROUP_SUMMON, 300, 200, 10, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_PASS_ARG);

  spello(SPELL_GROUP_INVISIBLE, 100, 35, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_CURE_DISEASE, 75, 15, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(FIRE_BREATH, 0, 0, 0, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_DAMAGE | MAG_MANUAL);

  spello(GAS_BREATH, 0, 0, 0, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(FROST_BREATH, 0, 0, 0, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_DAMAGE | MAG_MANUAL);

  spello(ACID_BREATH, 0, 0, 0, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_DAMAGE | MAG_MANUAL);

  spello(LIGHTNING_BREATH, 0, 0, 0, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_DAMAGE);



  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  for(i=MAX_SPELLS+1; ((spells[i][0]!='!')&&(i<=MAX_SKILLS)); i++)
    skillo(i);
  /* Take out any unused skills here with unused_spell */
  unused_spell(SKILL_UNUSED1);
  unused_spell(SKILL_UNUSED2);
}

