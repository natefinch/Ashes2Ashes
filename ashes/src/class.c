/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */

#define __CLASS_C_

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "class.h"
#include "handler.h"

#undef __CLASS_C_

#define MIL	* 1000000

/* Names first */

const char *class_abbrevs[] = {
  "Mag",
  "Cle",
  "Thi",
  "War",
  "Pal",
  "Ran",
  "Apa",
  "Mnk",
  "Psi",
  "\n"
};


const char *pc_class_types[] = {
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "Paladin",
  "Ranger",
  "Antipaladin",
  "Monk",
  "Psionicist",
  "\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
"\r\n"
"Select a class:\r\n"
"  [M]age\r\n"
"  [C]leric\r\n"
"  [T]hief\r\n"
"  [W]arrior\r\n"
"  [P]aladin\r\n"
"  [R]anger\r\n"
"  [A]ntipaladin\r\n"
"  Mon[k]\r\n"
"  Ps[i]onicist\r\n"
"\r\n"
"Class: ";


/* Determines what level of skills you get by number of classes */
const int MULTICLASS_SKILL_LEVELS[NUM_CLASSES+1] =
          {100, 100, 65, 44, 31, 23, 18, 15, 13, 12};


/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'm':
    return CLASS_MAGIC_USER;
    break;
  case 'c':
    return CLASS_CLERIC;
    break;
  case 'w':
    return CLASS_WARRIOR;
    break;
  case 't':
    return CLASS_THIEF;
    break;
  case 'p':
    return CLASS_PALADIN;
    break;
  case 'r':
    return CLASS_RANGER;
    break;
  case 'a':
    return CLASS_ANTIPALADIN;
    break;
  case 'k':
    return CLASS_MONK;
    break;
  case 'i':
    return CLASS_PSIONICIST;
    break;
  default:
    return CLASS_UNDEFINED;
    break;
  }
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long find_class_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'm':
      return (MU_F);
      break;
    case 'c':
      return (CL_F);
      break;
    case 't':
      return (TH_F);
      break;
    case 'w':
      return (WA_F);
      break;
    case 'p':
      return (PA_F);
      break;
    case 'r':
      return (RA_F);
      break;
    case 'a':
      return (AP_F);
      break;
    case 'k':
      return (MK_F);
      break;
    case 'i':
      return (PS_F);
      break;
    default:
      return 0;
      break;
  }
}


long exp_table[NUM_CLASSES][LVL_IMPL + 1];

/* This controls all aspects of level gaining for the classes */

int level_params[NUM_LEVEL_PARAMS][NUM_CLASSES] = {
/* MAG    CLE    THE    WAR    PAL    RAN    APAL  MONK    PSI */
  {20,    20,    45,    50,    30,    40,    30,    40,    10},   /* prac bonus (added to int bonus) */
  {5,     5,     5,     1,     5,     5,     5,     5,     5},    /* prac multiplier */
  {14,    14,    19,    4,     16,    18,    16,    18,    12},   /* prac divisor (when prac, get mul*(prac_bonus+int bonus)/div percent) */
  {1,     1,     1,     0,     1,     1,     1,     0,     0},    /* min prac per level */
  {3,     3,     1,     1,     2,     1,     2,     1,     2},    /* max prac per level */
  {1200,  1200,  1000,  1600,  1600,  1600,  1600,  2400,  2000}, /* level 2 xp */
  {104 MIL, 96 MIL, 93 MIL, 118 MIL, 164 MIL, 164 MIL, 164 MIL, 195 MIL, 150 MIL}, /* level 101 xp */
  {28,    29,    27,    30,    26,    26,    26,    29,    30},   /* xp curve (the lower the number, the more xp is piled up at the end) */
  {2,     3,     11,    5,     5,     5,     5,     5,     4},    /* thac0 numerator per level   4 5 */
  {2,     2,     5,     2,     2,     2,     2,     2,     2},    /* thac0 denominator per level 2 3 */
  {2,     2,     3,     5,     4,     4,     4,     3,     4},    /* min hp per level */
  {4,     5,     7,     8,     7,     7,     7,     5,     6},    /* max hp per level */
  {1,     1,     0,     0,     0,     0,     0,     0,     1},    /* min mana per level */
  {5,     5,     0,     0,     3,     0,     3,     0,     3},    /* max mana per level */
  {0,     0,     0,     0,     0,     1,     0,     0,     0},    /* min move per level */
  {1,     1,     1,     1,     1,     2,     1,     2,     1},    /* max move per level */
  {-4,    -4,    0,     0,     0,     0,     0,     -2,    10},   /* hp regen base */
  {200,   200,   10,    10,    14,    14,    14,    16,    10},   /* hp regen level divisor */
  {2,     2,     1,     1,     1,     1,     1,     1,     1},    /* mana regen class multiplier */
  {0,     0,     0,     0,     0,     0,     0,     0,     25},   /* mana regen base */
  {10,    10,    200,   200,   14,    200,   14,    200,   1},    /* mana regen level divisor */
  {0,     0,     0,     0,     0,     0,     0,     0,     -2},   /* move regen base */
  {200,   200,   200,   200,   200,   14,    200,   24,    12},   /* move regen level divisor */
  {95,    95,    95,    95,    95,    95,    95,    95,    95},   /* save para level 1 */
  {28,    7,     26,    12,    12,    12,    12,    19,    10},   /* save para level 100 */
  {95,    95,    95,    95,    95,    95,    95,    95,    95},   /* save rod level 1 */
  {9,     24,    13,    22,    22,    22,    22,    14,    18},   /* save rod level 100 */
  {95,    95,    95,    95,    95,    95,    95,    95,    95},   /* save petri level 1 */
  {17,    12,    21,    27,    27,    27,    27,    24,    23},   /* save petri level 100 */
  {95,    95,    95,    95,    95,    95,    95,    95,    95},   /* save breath level 1 */
  {23,    27,    31,    17,    17,    17,    17,    14,    27},   /* save breath level 100 */
  {95,    95,    95,    95,    95,    95,    95,    95,    95},   /* save spell level 1 */
  {12,    22,    17,    27,    27,    27,    27,    19,    17}    /* save spell level 100 */
};


/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only MAGIC_USERS are allowed
 * to go south.
 */
int guild_info[][3] = {

/* Midgaard */
  {CLASS_RANGER,	2996,	SCMD_SOUTH},
  {CLASS_MAGIC_USER,	3017,	SCMD_SOUTH},
  {CLASS_CLERIC,	3004,	SCMD_NORTH},
  {CLASS_THIEF,		3027,	SCMD_EAST},
  {CLASS_WARRIOR,	3021,	SCMD_EAST},
  {CLASS_PALADIN,	3057,	SCMD_UP},
  {CLASS_ANTIPALADIN,	3068,	SCMD_DOWN},
  {CLASS_MONK,		3097,	SCMD_WEST},
  {CLASS_PSIONICIST,	3147,	SCMD_NORTH},

/* Brass Dragon */
  {-999 /* all */ ,	5065,	SCMD_WEST},

/* Land of lag */
  {-999 /* all */ ,	17281,	SCMD_SOUTH},

/* Dragonlance */
  {-999 /* all */ ,	18066,	SCMD_UP},
  {-999 /* all */ ,	18068,	SCMD_UP},
  {-999 /* all */ ,	18108,	SCMD_NORTH},

/* New Sparta */
  {CLASS_MAGIC_USER,	21075,	SCMD_NORTH},
  {CLASS_CLERIC,	21019,	SCMD_WEST},
  {CLASS_THIEF,		21014,	SCMD_SOUTH},
  {CLASS_WARRIOR,	21023,	SCMD_SOUTH},

/* this must go last -- add new guards above! */
{-1, -1, -1}};


/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 */
void roll_real_abils(struct char_data * ch)
{
  int i, j, k, temp, high, num;
  ubyte table[6];
  ubyte rolls[6];
  static ubyte num_dice[6] = {6, 5, 4, 4, 3, 3};

  for (i = 0; i < 6; i++) {

    for (j = 0; j < num_dice[i]; j++)
      rolls[j] = number(1, 6);

    for(; j<6; j++)
      rolls[j]=0;

    for(temp=0, k=0; k<3; k++) {
      for(num=0, high=0, j=0; j<num_dice[i]; j++) {
        if(rolls[j]>high) {
          high=rolls[j];
          num=j;
        }
      }
      temp+=high;
      rolls[num]=0;
    }

    table[i]=temp;
  }


  ch->real_abils.str_add = 0;

  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    ch->real_abils.intel = table[0];
    ch->real_abils.wis = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_CLERIC:
    ch->real_abils.wis = table[0];
    ch->real_abils.intel = table[1];
    ch->real_abils.str = table[2];
    ch->real_abils.dex = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_THIEF:
    ch->real_abils.dex = table[0];
    ch->real_abils.str = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_WARRIOR:
    ch->real_abils.str = table[0];
    ch->real_abils.dex = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.wis = table[3];
    ch->real_abils.intel = table[4];
    ch->real_abils.cha = table[5];
    if (table[0] < 16)
      ch->real_abils.str += number(0, (18-table[0]));
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = number(0, 100);
    break;
  case CLASS_PALADIN:
    ch->real_abils.str = table[0];
    ch->real_abils.wis = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.dex = table[3];
    ch->real_abils.intel = table[4];
    ch->real_abils.cha = table[5];
    if (table[0] < 16)
      ch->real_abils.str += number(0, (18-table[0]));
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = number(0, 100);
    break;
  case CLASS_RANGER:
    ch->real_abils.dex = table[0];
    ch->real_abils.str = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    if (table[1] < 16)
      ch->real_abils.str += number(0, (18-table[1]));
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = number(0, 100);
    break;
  case CLASS_ANTIPALADIN:
    ch->real_abils.str = table[0];
    ch->real_abils.intel = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.dex = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    if (table[0] < 16)
      ch->real_abils.str += number(0, (18-table[0]));
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = number(0, 100);
    break;
  case CLASS_MONK:
    ch->real_abils.dex = table[0];
    ch->real_abils.con = table[1];
    ch->real_abils.wis = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.intel = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_PSIONICIST:
    ch->real_abils.wis = table[0];
    ch->real_abils.con = table[1];
    ch->real_abils.intel = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.dex = table[4];
    ch->real_abils.cha = table[5];
    break;
  }
  ch->aff_abils = ch->real_abils;

}


/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
  struct affected_type *af;
  int i, j;

  void advance_level(struct char_data * ch);

  GET_CLASS_LEVEL(ch, (int)GET_CLASS(ch)) = GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;
  ch->player_specials->saved.extra_pracs=GET_PRACTICES(ch)=0;
  if(ch->player_specials->saved.rerolling) {
    if(GET_CLASS(ch)==CLASS_MONK)
      ch->player_specials->saved.inherent_ac_apply=0;
    ch->player_specials->saved.rerolling=0;
  }
  else {
    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
          affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
    }

    for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    ch->player_specials->saved.reroll_level=0;
    ch->player_specials->saved.old_hit = GET_MAX_HIT(ch);
    ch->player_specials->saved.old_mana = GET_MAX_MANA(ch);
    ch->player_specials->saved.old_move = GET_MAX_MOVE(ch);

    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
          affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, TRUE);
    }

    for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
        affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
  }

  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

  ch->points.max_hit = 10;
  ch->points.max_move = 82;
  ch->points.max_mana = 100;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    break;

  case CLASS_CLERIC:
    break;

  case CLASS_THIEF:
    GET_MAX_MANA(ch) = 0;
    break;

  case CLASS_WARRIOR:
    GET_MAX_MANA(ch) = 0;
    break;

  case CLASS_PALADIN:
    SET_BIT(ch->char_specials.saved.affected_by, AFF_DETECT_ALIGN);
    break;

  case CLASS_RANGER:
    GET_MAX_MOVE(ch) = 100;
    GET_MAX_MANA(ch) = 0;
    break;
  case CLASS_ANTIPALADIN:
    SET_BIT(ch->char_specials.saved.affected_by, AFF_DETECT_ALIGN);
    break;
  case CLASS_MONK:
    GET_MAX_MANA(ch) = 0;
    break;
  case CLASS_PSIONICIST:
    GET_MAX_MANA(ch) = 200;
    break;
  }

  ch->player_specials->saved.new_hit = GET_MAX_HIT(ch);
  ch->player_specials->saved.new_mana = GET_MAX_MANA(ch);
  ch->player_specials->saved.new_move = GET_MAX_MOVE(ch);

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

  advance_level(ch);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  if(!((ch->player_specials->saved.rerolling || (GET_NUM_CLASSES(ch)>1)) && (GET_COND(ch, THIRST) == -1)))
    GET_COND(ch, THIRST) = 24;
  if(!((ch->player_specials->saved.rerolling || (GET_NUM_CLASSES(ch)>1)) && (GET_COND(ch, FULL) == -1)))
    GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;
  affect_total(ch);
}


void class_init_exp_tables(void)
{
  int class, i, j;
  double e10_c, ugf, xmax_x10;
  long xp;

  double exp(double);

  for(class=0; class < NUM_CLASSES; class++) {

    if(!level_params[XP_CURVE][class])
      continue;

    exp_table[class][0]=0;
    exp_table[class][1]=1;
    exp_table[class][2]=level_params[XP_2][class];
    for(i=3; i<=10; i++)
      exp_table[class][i]=2*exp_table[class][i-1];

    e10_c = exp(10.0 / (double) level_params[XP_CURVE][class]);
    ugf = exp((double) (LVL_HERO) / (double) level_params[XP_CURVE][class]) - e10_c;
    xmax_x10 = ((double) level_params[XP_MAX][class] - (double) exp_table[class][10]);

    for(i=11; i < LVL_HERO; i++) {
      xp = (xmax_x10 * (exp((double) i / (double) level_params[XP_CURVE][class]) - e10_c) / ugf) + exp_table[class][10];
      xp /= 1000;
      xp *= 1000;
      exp_table[class][i]=xp;
    }

    for(j=LVL_IMPL; j>=i; j--) {
      exp_table[class][j] = 250000000L-(10000000L*(LVL_IMPL-j));
    }
  }
}

int single_class_get_thac0(struct char_data *ch, int class)
{
  float thac0mul=0;

  /* Bonus thac0 if you're a fighter type at low level */
  if(((WA_F|PA_F|RA_F|AP_F|MK_F)&(1 << class))&&(GET_LEVEL(ch)<=5)) {
    return(-24);
  }
  else {
    thac0mul=(float)level_params[THAC0_NUM][class]/level_params[THAC0_DEN][class];
    return(-4 - ((GET_LEVEL(ch)-1)*thac0mul));
  }
}

/* Multiclass use best average thac0 and best saves */
int class_get_thac0(struct char_data *ch)
{
  int i;
  int total_thac0=0;

  if(IS_NPC(ch)) {
    return(0);
  }
  else {
    for(i=0; i < NUM_CLASSES; i++) {
      if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i)))
        total_thac0 += single_class_get_thac0(ch, i);
    }
    return(total_thac0 / GET_NUM_CLASSES(ch));
  }
}


int class_get_save(struct char_data *ch, int save_type)
{
  int s1, s2, retv=0, i, temp;

  if(IS_NPC(ch)) {
    return(0);
  }
  else {
    for(i=0; i < NUM_CLASSES; i++) {
      if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i))) {
        s1=level_params[SAVE_PARA_MIN+(2*save_type)][i];
        s2=level_params[SAVE_PARA_MAX+(2*save_type)][i];
        if((temp=(((s2-s1)*(GET_LEVEL(ch)-1)) / (LVL_HERO-2))+s1) > retv)
          retv=temp;
      }
    }
    return(retv);
  }
}


/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void advance_level(struct char_data * ch)
{
  int add_hp = 0, add_mana = 0, add_move = 0, add_prac = 0, tmp_mana, tmp_move;
  float more_prac;
  struct affected_type *af;
  int i, j, tmp;
  struct char_ability_data temp1, temp2;
  

  extern struct dex_app_type dex_app[];
  extern struct con_app_type con_app[];

  temp1 = ch->aff_abils;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
  }


  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

  temp2 = ch->aff_abils;
  ch->aff_abils = temp1;

  tmp_mana = MAX(GET_INT(ch)-10, 0) + MAX(GET_WIS(ch)-10, 0);
  for(i=0; i < NUM_CLASSES; i++) {
    if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i))) {
      add_hp += number(level_params[MIN_HP_LEVEL][i], level_params[MAX_HP_LEVEL][i]);
      add_mana += ((tmp_mana*(level_params[MAX_MANA_LEVEL][i] - level_params[MIN_MANA_LEVEL][i]))/16)+level_params[MIN_MANA_LEVEL][i];
      tmp_move = number(level_params[MIN_MOVE_LEVEL][i], level_params[MAX_MOVE_LEVEL][i]);
      if(i==CLASS_RANGER) {
        tmp_move += (GET_CON(ch) + GET_DEX(ch))/18;
        tmp_move = MIN(tmp_move, level_params[MAX_MOVE_LEVEL][i]);
      }
      add_move += tmp_move;
    }
  }
  add_hp /= GET_NUM_CLASSES(ch);
  tmp = add_mana % GET_NUM_CLASSES(ch);
  add_mana /= GET_NUM_CLASSES(ch);
  if(tmp > number(0, GET_NUM_CLASSES(ch)-1))
    add_mana++;
  tmp = add_move % GET_NUM_CLASSES(ch);
  add_move /= GET_NUM_CLASSES(ch);
  if(tmp > number(0, GET_NUM_CLASSES(ch)-1))
    add_move++;
  if(GET_LEVEL(ch)>ch->player_specials->saved.reroll_level) {
    add_prac = MAX(level_params[MIN_PRAC][(int)GET_CLASS(ch)], (((GET_WIS(ch)-3)*(level_params[MAX_PRAC][(int)GET_CLASS(ch)]-level_params[MIN_PRAC][(int)GET_CLASS(ch)]))/15)+level_params[MIN_PRAC][(int)GET_CLASS(ch)]);
    more_prac = ((((GET_WIS(ch)-3.0)*(level_params[MAX_PRAC][(int)GET_CLASS(ch)]-level_params[MIN_PRAC][(int)GET_CLASS(ch)]))/15.0)+level_params[MIN_PRAC][(int)GET_CLASS(ch)]) - add_prac;
    if(more_prac > 0)
      add_prac += ((number(1, 100) <= (100*more_prac)) ? 1 : 0);
    add_prac = MIN(add_prac, level_params[MAX_PRAC][(int)GET_CLASS(ch)]);
    add_prac += ((number(1, 100) <= GET_WIS(ch)) ? 1 : 0);
  }
  add_hp += con_app[GET_CON(ch)].hitp >> 1;
  if(con_app[GET_CON(ch)].hitp % 2)
    add_hp += number(0, 1);

  if(GET_CLASS(ch)==CLASS_MONK) {
    ch->player_specials->saved.inherent_ac_apply += MIN(MAX(1, -dex_app[GET_DEX(ch)].defensive), 3);
  }

  ch->player_specials->saved.new_hit += MAX(1, add_hp);
  ch->player_specials->saved.new_move += MAX(0, add_move);

  if (GET_LEVEL(ch) > 1)
    ch->player_specials->saved.new_mana += MAX(0, add_mana);

  ch->points.max_hit = MAX(ch->player_specials->saved.new_hit, ch->player_specials->saved.old_hit);
  ch->points.max_mana = MAX(ch->player_specials->saved.new_mana, ch->player_specials->saved.old_mana);
  ch->points.max_move = MAX(ch->player_specials->saved.new_move, ch->player_specials->saved.old_move);

  GET_PRACTICES(ch) += add_prac;
  ch->player_specials->saved.extra_pracs+=add_prac;

  if (GET_LEVEL(ch) >= LVL_HERO) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT | PRF_NOHASSLE);
  }

  GET_MAX_MANA(ch)=MIN(MAX_MANA, GET_MAX_MANA(ch));
  GET_MAX_HIT(ch)=MIN(MAX_HIT, GET_MAX_HIT(ch));
  GET_MAX_MOVE(ch)=MIN(MAX_MOVE, GET_MAX_MOVE(ch));

  ch->aff_abils = temp2;

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

  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlog(buf, BRF, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
  mortlog(buf, BRF, LVL_HERO, FALSE);
}


/*
 * This simply calculates the backstab multiplier based on a character's
 * level.  This used to be an array, but was changed to be a function so
 * that it would be easier to add more levels to your MUD.  This doesn't
 * really create a big performance hit because it's not used very often.
 */
int backstab_mult(int level)
{
  if (level <= 0)
    return 1;	  /* level 0 */
  else if (level <= 23)
    return 2;	  /* level 1 - 7 */
  else if (level <= 46)
    return 3;	  /* level 8 - 13 */
  else if (level <= 69)
    return 4;	  /* level 14 - 20 */
  else if (level <= 100)
    return 5;	  /* level 21 - 28 */
  else
    return 20;	  /* immortals */
}


/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_class(struct char_data *ch, struct obj_data *obj)
{
  int i;
  long long bitv, strict_bitv=0;

  if(IS_NPC(ch))
    return 0;

  if(IS_SET(GET_CLASS_BITVECTOR(ch), MK_F)) {
    int i, ac=0;
    if(GET_OBJ_TYPE(obj) == ITEM_ARMOR)
      return 1;
    for(i=0; i < MAX_OBJ_AFFECT; i++) {
      if(obj->affected[i].location==APPLY_AC)
        ac+=obj->affected[i].modifier;
    }
    if(ac < -4)
      return 1;
  }

  if(IS_OBJ_STAT(obj, ITEM_STRICT_CLASSES)) {
    for(i=0; i < NUM_CLASSES; i++) {
      switch(i) {
      case CLASS_MAGIC_USER:
        bitv = ITEM_ANTI_MAGIC_USER;
        break;
      case CLASS_CLERIC:
        bitv = ITEM_ANTI_CLERIC;
        break;
      case CLASS_THIEF:
        bitv = ITEM_ANTI_THIEF;
        break;
      case CLASS_WARRIOR:
        bitv = ITEM_ANTI_WARRIOR;
        break;
      case CLASS_PALADIN:
        bitv = ITEM_ANTI_PALADIN;
        break;
      case CLASS_RANGER:
        bitv = ITEM_ANTI_RANGER;
        break;
      case CLASS_ANTIPALADIN:
        bitv = ITEM_ANTI_ANTIPALADIN;
        break;
      case CLASS_MONK:
        bitv = ITEM_ANTI_MONK;
        break;
      case CLASS_PSIONICIST:
        bitv = ITEM_ANTI_PSIONICIST;
        break;
      default:
        bitv = 0;
        break;
      }
      if(!IS_OBJ_STAT(obj, bitv))
        SET_BIT(strict_bitv, (1 << i));
    }
    if(GET_CLASS_BITVECTOR(ch)==strict_bitv)
      return 0;
    else
      return 1;
  }
  else {
    for(i=0; i < NUM_CLASSES; i++) {
      switch(i) {
      case CLASS_MAGIC_USER:
        bitv = ITEM_ANTI_MAGIC_USER;
        break;
      case CLASS_CLERIC:
        bitv = ITEM_ANTI_CLERIC;
        break;
      case CLASS_THIEF:
        bitv = ITEM_ANTI_THIEF;
        break;
      case CLASS_WARRIOR:
        bitv = ITEM_ANTI_WARRIOR;
        break;
      case CLASS_PALADIN:
        bitv = ITEM_ANTI_PALADIN;
        break;
      case CLASS_RANGER:
        bitv = ITEM_ANTI_RANGER;
        break;
      case CLASS_ANTIPALADIN:
        bitv = ITEM_ANTI_ANTIPALADIN;
        break;
      case CLASS_MONK:
        bitv = ITEM_ANTI_MONK;
        break;
      case CLASS_PSIONICIST:
        bitv = ITEM_ANTI_PSIONICIST;
        break;
      default:
        bitv = 0;
        break;
      }
      if(IS_OBJ_STAT(obj, ITEM_STRONG_RESTRICT)) {
        if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i)) && IS_OBJ_STAT(obj, bitv))
          return 1;
      }
      else {
        if(IS_SET(GET_CLASS_BITVECTOR(ch), (1 << i)) && (!IS_OBJ_STAT(obj, bitv)))
          return 0;
      }
    }
    if(IS_OBJ_STAT(obj, ITEM_STRONG_RESTRICT))
      return 0;
    else
      return 1;
  }
  return 1;
}




/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */
void init_spell_levels(void)
{
  /* MAGE */
  spell_level(SPELL_MAGIC_MISSILE, CLASS_MAGIC_USER, 1, TRUE);
  spell_level(SPELL_DETECT_MAGIC, CLASS_MAGIC_USER, 3, TRUE);
  spell_level(SPELL_DETECT_INVIS, CLASS_MAGIC_USER, 5, TRUE);
  spell_level(SPELL_INVISIBLE, CLASS_MAGIC_USER, 6, TRUE);
  spell_level(SPELL_ARMOR, CLASS_MAGIC_USER, 8, TRUE);
  spell_level(SPELL_BURNING_HANDS, CLASS_MAGIC_USER, 9, TRUE);
  spell_level(SPELL_LOCATE_OBJECT, CLASS_MAGIC_USER, 11, TRUE);
  spell_level(SPELL_STRENGTH, CLASS_MAGIC_USER, 13, TRUE);
  spell_level(SPELL_TELEPORT, CLASS_MAGIC_USER, 14, TRUE);
  spell_level(SPELL_MAGIC_LIGHT, CLASS_MAGIC_USER, 15, TRUE);
  spell_level(SPELL_BLINDNESS, CLASS_MAGIC_USER, 16, TRUE);
  spell_level(SPELL_CHILL_TOUCH, CLASS_MAGIC_USER, 17, TRUE);
  spell_level(SPELL_CONTROL_WEATHER, CLASS_MAGIC_USER, 18, TRUE);
  spell_level(SPELL_SLEEP, CLASS_MAGIC_USER, 19, TRUE);
  spell_level(SPELL_INFRAVISION, CLASS_MAGIC_USER, 20, TRUE);
  spell_level(SPELL_ENERGY_DRAIN, CLASS_MAGIC_USER, 21, TRUE);
  spell_level(SPELL_IDENTIFY, CLASS_MAGIC_USER, 23, TRUE);
  spell_level(SKILL_MEMORIZE, CLASS_MAGIC_USER, 24, TRUE);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_MAGIC_USER, 25, TRUE);
  spell_level(SPELL_CURSE, CLASS_MAGIC_USER, 26, TRUE);
  spell_level(SPELL_CHARM, CLASS_MAGIC_USER, 27, TRUE);
  spell_level(SPELL_COLOR_SPRAY, CLASS_MAGIC_USER, 29, TRUE);
  spell_level(SPELL_SPELL_SHIELD, CLASS_MAGIC_USER, 30, TRUE);
  spell_level(SPELL_ENCHANT_WEAPON, CLASS_MAGIC_USER, 32, TRUE);
  spell_level(SPELL_CONE_OF_COLD, CLASS_MAGIC_USER, 33, TRUE);
  spell_level(SPELL_REJUVENATE, CLASS_MAGIC_USER, 35, TRUE);
  spell_level(SPELL_GROUP_INFRAVISION, CLASS_MAGIC_USER, 38, TRUE);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_MAGIC_USER, 41, TRUE);
  spell_level(SPELL_GROUP_ARMOR, CLASS_MAGIC_USER, 42, TRUE);
  spell_level(SPELL_TELEPORT_NO_ERROR, CLASS_MAGIC_USER, 43, TRUE);
  spell_level(SPELL_ICE_STORM, CLASS_MAGIC_USER, 45, TRUE);
  spell_level(SPELL_GROUP_INVISIBLE, CLASS_MAGIC_USER, 47, TRUE);
  spell_level(SPELL_FIREBALL, CLASS_MAGIC_USER, 49, TRUE);
  spell_level(SPELL_MAGIC_SHIELD, CLASS_MAGIC_USER, 52, TRUE);
  spell_level(SPELL_LOWER_RESISTANCE, CLASS_MAGIC_USER, 54, TRUE);
  spell_level(SPELL_WATERWALK, CLASS_MAGIC_USER, 57, TRUE);
  spell_level(SPELL_GROUP_REJUVENATE, CLASS_MAGIC_USER, 59, TRUE);
  spell_level(SPELL_REMEMBER, CLASS_MAGIC_USER, 62, TRUE);
  spell_level(SPELL_HASTE, CLASS_MAGIC_USER, 64, TRUE);
  spell_level(SPELL_LIMITED_WISH, CLASS_MAGIC_USER, 67, TRUE);
  spell_level(SPELL_AMNESIA, CLASS_MAGIC_USER, 72, TRUE);
  spell_level(SPELL_WORD_OF_DEATH, CLASS_MAGIC_USER, 75, TRUE);
  spell_level(SPELL_FLY, CLASS_MAGIC_USER, 80, FALSE);

  /* CLERIC */
  spell_level(SPELL_CURE_LIGHT, CLASS_CLERIC, 1, TRUE);
  spell_level(SPELL_ARMOR, CLASS_CLERIC, 2, TRUE);
  spell_level(SPELL_DETECT_POISON, CLASS_CLERIC, 3, TRUE);
  spell_level(SPELL_DETECT_MAGIC, CLASS_CLERIC, 4, TRUE);
  spell_level(SPELL_CURE_BLIND, CLASS_CLERIC, 5, TRUE);
  spell_level(SPELL_DETECT_ALIGN, CLASS_CLERIC, 6, TRUE);
  spell_level(SPELL_BLESS, CLASS_CLERIC, 7, TRUE);
  spell_level(SPELL_CURE_SERIOUS, CLASS_CLERIC, 8, TRUE);
  spell_level(SPELL_DETECT_INVIS, CLASS_CLERIC, 9, TRUE);
  spell_level(SPELL_CREATE_WATER, CLASS_CLERIC, 10, TRUE);
  spell_level(SPELL_BLINDNESS, CLASS_CLERIC, 11, TRUE);
  spell_level(SPELL_EARTHQUAKE, CLASS_CLERIC, 12, TRUE);
  spell_level(SPELL_SENSE_LIFE, CLASS_CLERIC, 13, TRUE);
  spell_level(SPELL_CREATE_FOOD, CLASS_CLERIC, 14, TRUE);
  spell_level(SPELL_SUMMON, CLASS_CLERIC, 15, TRUE);
  spell_level(SPELL_CURE_CRITIC, CLASS_CLERIC, 16, TRUE);
  spell_level(SPELL_REMOVE_POISON, CLASS_CLERIC, 17, TRUE);
  spell_level(SPELL_DISPEL_GOOD, CLASS_CLERIC, 18, TRUE);
  spell_level(SPELL_DISPEL_EVIL, CLASS_CLERIC, 18, TRUE);
  spell_level(SPELL_POISON, CLASS_CLERIC, 19, TRUE);
  spell_level(SPELL_CONTROL_WEATHER, CLASS_CLERIC, 20, TRUE);
  spell_level(SPELL_AID, CLASS_CLERIC, 21, TRUE);
  spell_level(SPELL_LOCATE_OBJECT, CLASS_CLERIC, 22, TRUE);
  spell_level(SPELL_REVITALIZE, CLASS_CLERIC, 23, TRUE);
  spell_level(SPELL_CALL_LIGHTNING, CLASS_CLERIC, 24, TRUE);
  spell_level(SPELL_WORD_OF_RECALL, CLASS_CLERIC, 25, TRUE);
  spell_level(SPELL_REMOVE_CURSE, CLASS_CLERIC, 26, TRUE);
  spell_level(SPELL_REJUVENATE, CLASS_CLERIC, 27, TRUE);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_CLERIC, 28, TRUE);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_CLERIC, 28, TRUE);
  spell_level(SPELL_GROUP_ARMOR, CLASS_CLERIC, 29, TRUE);
  spell_level(SPELL_HEAL, CLASS_CLERIC, 31, TRUE);
  spell_level(SPELL_HARM, CLASS_CLERIC, 32, TRUE);
  spell_level(SPELL_SANCTUARY, CLASS_CLERIC, 33, TRUE);
  spell_level(SPELL_ANIMATE_DEAD, CLASS_CLERIC, 35, TRUE);
  spell_level(SPELL_INFRAVISION, CLASS_CLERIC, 37, TRUE);
  spell_level(SPELL_CALM, CLASS_CLERIC, 40, TRUE);
  spell_level(SPELL_CURE_DISEASE, CLASS_CLERIC, 42, TRUE);
  spell_level(SPELL_POWER_HEAL, CLASS_CLERIC, 45, TRUE);
  spell_level(SPELL_FLAMESTRIKE, CLASS_CLERIC, 46, TRUE);
  spell_level(SPELL_GROUP_REJUVENATE, CLASS_CLERIC, 48, TRUE);
  spell_level(SPELL_GROUP_HEAL, CLASS_CLERIC, 52, TRUE);
  spell_level(SPELL_RELOCATE, CLASS_CLERIC, 56, TRUE);
  spell_level(SPELL_GROUP_RECALL, CLASS_CLERIC, 64, TRUE);
  spell_level(SPELL_RESTORE, CLASS_CLERIC, 66, TRUE);
  spell_level(SPELL_IMPLOSION, CLASS_CLERIC, 67, TRUE);
  spell_level(SPELL_GROUP_POWER_HEAL, CLASS_CLERIC, 72, TRUE);
  spell_level(SPELL_DIVINE_PROTECTION, CLASS_CLERIC, 75, TRUE);
  spell_level(SPELL_BATTLE_RECALL, CLASS_CLERIC, 77, TRUE);
  spell_level(SPELL_GROUP_SANCTUARY, CLASS_CLERIC, 80, TRUE);
  spell_level(SPELL_GROUP_SUMMON, CLASS_CLERIC, 85, TRUE);

  /* ANTIPALADIN */
  spell_level(SKILL_KICK, CLASS_ANTIPALADIN, 1, TRUE);
  spell_level(SPELL_MAGIC_MISSILE, CLASS_ANTIPALADIN, 5, TRUE);
  spell_level(SPELL_ARMOR, CLASS_ANTIPALADIN, 6, TRUE);
  spell_level(SKILL_BASH, CLASS_ANTIPALADIN, 8, TRUE);
  spell_level(SPELL_DETECT_INVIS, CLASS_ANTIPALADIN, 9, TRUE);
  spell_level(SPELL_DETECT_MAGIC, CLASS_ANTIPALADIN, 10, TRUE);
  spell_level(SPELL_INVISIBLE, CLASS_ANTIPALADIN, 11, TRUE);
  spell_level(SPELL_BURNING_HANDS, CLASS_ANTIPALADIN, 15, TRUE);
  spell_level(SKILL_HEALWOUNDS, CLASS_ANTIPALADIN, 16, TRUE);
  spell_level(SPELL_BLINDNESS, CLASS_ANTIPALADIN, 17, TRUE);
  spell_level(SPELL_EARTHQUAKE, CLASS_ANTIPALADIN, 18, TRUE);
  spell_level(SPELL_LOCATE_OBJECT, CLASS_ANTIPALADIN, 19, TRUE);
  spell_level(SPELL_STRENGTH, CLASS_ANTIPALADIN, 20, TRUE);
  spell_level(SKILL_RESCUE, CLASS_ANTIPALADIN, 21, TRUE);
  spell_level(SPELL_COLOR_SPRAY, CLASS_ANTIPALADIN, 22, TRUE);
  spell_level(SPELL_POISON, CLASS_ANTIPALADIN, 23, TRUE);
  spell_level(SPELL_CHILL_TOUCH, CLASS_ANTIPALADIN, 24, TRUE);
  spell_level(SPELL_INFRAVISION, CLASS_ANTIPALADIN, 25, TRUE);
  spell_level(SKILL_DOUBLE, CLASS_ANTIPALADIN, 26, TRUE);
  spell_level(SPELL_SLEEP, CLASS_ANTIPALADIN, 27, TRUE);
  spell_level(SPELL_DISPEL_GOOD, CLASS_ANTIPALADIN, 28, TRUE);
  spell_level(SPELL_ENERGY_DRAIN, CLASS_ANTIPALADIN, 29, TRUE);
  spell_level(SPELL_CURSE, CLASS_ANTIPALADIN, 30, TRUE);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_ANTIPALADIN, 32, TRUE);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_ANTIPALADIN, 33, TRUE);
  spell_level(SPELL_REJUVENATE, CLASS_ANTIPALADIN, 36, TRUE);
  spell_level(SPELL_UNHOLY_WORD, CLASS_ANTIPALADIN, 40, TRUE);
  spell_level(SPELL_CONE_OF_COLD, CLASS_ANTIPALADIN, 45, TRUE);
  spell_level(SPELL_HARM, CLASS_ANTIPALADIN, 47, TRUE);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_ANTIPALADIN, 54, TRUE);
  spell_level(SPELL_FIREBALL, CLASS_ANTIPALADIN, 61, TRUE);
  spell_level(SPELL_GROUP_REJUVENATE, CLASS_ANTIPALADIN, 63, TRUE);
  spell_level(SPELL_FLAMESTRIKE, CLASS_ANTIPALADIN, 66, TRUE);
  spell_level(SPELL_SLOW, CLASS_ANTIPALADIN, 75, TRUE);

  /* PALADIN */
  spell_level(SKILL_KICK, CLASS_PALADIN, 1, TRUE);
  spell_level(SPELL_REMOVE_POISON, CLASS_PALADIN, 2, TRUE);
  spell_level(SPELL_CURE_LIGHT, CLASS_PALADIN, 5, TRUE);
  spell_level(SPELL_ARMOR, CLASS_PALADIN, 6, TRUE);
  spell_level(SPELL_DETECT_POISON, CLASS_PALADIN, 7, TRUE);
  spell_level(SKILL_BASH, CLASS_PALADIN, 8, TRUE);
  spell_level(SPELL_DETECT_MAGIC, CLASS_PALADIN, 10, TRUE);
  spell_level(SPELL_CREATE_WATER, CLASS_PALADIN, 11, TRUE);
  spell_level(SPELL_CURE_BLIND, CLASS_PALADIN, 12, TRUE);
  spell_level(SPELL_BLESS, CLASS_PALADIN, 13, TRUE);
  spell_level(SKILL_RESCUE, CLASS_PALADIN, 14, TRUE);
  spell_level(SPELL_CURE_SERIOUS, CLASS_PALADIN, 15, TRUE);
  spell_level(SPELL_CREATE_FOOD, CLASS_PALADIN, 16, TRUE);
  spell_level(SPELL_DETECT_INVIS, CLASS_PALADIN, 17, TRUE);
  spell_level(SPELL_EARTHQUAKE, CLASS_PALADIN, 18, TRUE);
  spell_level(SPELL_SENSE_LIFE, CLASS_PALADIN, 19, TRUE);
  spell_level(SPELL_SUMMON, CLASS_PALADIN, 20, TRUE);
  spell_level(SPELL_AID, CLASS_PALADIN, 22, TRUE);
  spell_level(SPELL_CURE_CRITIC, CLASS_PALADIN, 23, TRUE);
  spell_level(SKILL_DOUBLE, CLASS_PALADIN, 26, TRUE);
  spell_level(SPELL_REMOVE_CURSE, CLASS_PALADIN, 27, TRUE);
  spell_level(SPELL_DISPEL_EVIL, CLASS_PALADIN, 28, TRUE);
  spell_level(SPELL_WORD_OF_RECALL, CLASS_PALADIN, 30, TRUE);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_PALADIN, 32, TRUE);
  spell_level(SPELL_CURE_DISEASE, CLASS_PALADIN, 34, TRUE);
  spell_level(SPELL_REJUVENATE, CLASS_PALADIN, 36, TRUE);
  spell_level(SPELL_HEAL, CLASS_PALADIN, 38, TRUE);
  spell_level(SPELL_HOLY_WORD, CLASS_PALADIN, 40, TRUE);
  spell_level(SPELL_INFRAVISION, CLASS_PALADIN, 42, TRUE);
  spell_level(SPELL_SANCTUARY, CLASS_PALADIN, 45, TRUE);
  spell_level(SPELL_CALM, CLASS_PALADIN, 48, TRUE);
  spell_level(SPELL_GROUP_REJUVENATE, CLASS_PALADIN, 53, TRUE);
  spell_level(SPELL_WATERWALK, CLASS_PALADIN, 60, TRUE);
  spell_level(SPELL_GROUP_HEAL, CLASS_PALADIN, 66, TRUE);
  spell_level(SPELL_POWER_HEAL, CLASS_PALADIN, 68, TRUE);
  spell_level(SPELL_GROUP_POWER_HEAL, CLASS_PALADIN, 80, TRUE);

  /* WARRIOR */
  spell_level(SKILL_KICK, CLASS_WARRIOR, 1, TRUE);
  spell_level(SKILL_BASH, CLASS_WARRIOR, 6, TRUE);
  spell_level(SKILL_HEALWOUNDS, CLASS_WARRIOR, 14, TRUE);
  spell_level(SKILL_RESCUE, CLASS_WARRIOR, 16, TRUE);
  spell_level(SKILL_DOUBLE, CLASS_WARRIOR, 19, TRUE);
  spell_level(SKILL_BERSERK, CLASS_WARRIOR, 25, TRUE);
  spell_level(SKILL_GRAPPLE, CLASS_WARRIOR, 32, TRUE);
  spell_level(SKILL_TRIPLE, CLASS_WARRIOR, 45, TRUE);
  spell_level(SKILL_HURL, CLASS_WARRIOR, 66, TRUE);
  spell_level(SKILL_QUAD, CLASS_WARRIOR, 80, TRUE);

  /* RANGER */
  spell_level(SKILL_FORAGE, CLASS_RANGER, 1, TRUE);
  spell_level(SKILL_DIVINE, CLASS_RANGER, 3, TRUE);
  spell_level(SKILL_FILET, CLASS_RANGER, 10, TRUE);
  spell_level(SKILL_HEALWOUNDS, CLASS_RANGER, 14, TRUE);
  spell_level(SKILL_CAMOUFLAGE, CLASS_RANGER, 16, TRUE);
  spell_level(SKILL_SILENTWALK, CLASS_RANGER, 18, TRUE);
  spell_level(SKILL_SCAN, CLASS_RANGER, 21, TRUE);
  spell_level(SKILL_SKIN, CLASS_RANGER, 24, TRUE);
  spell_level(SKILL_BEFRIEND, CLASS_RANGER, 31, TRUE);
  spell_level(SKILL_SWIM, CLASS_RANGER, 35, TRUE);
  spell_level(SKILL_SNARE, CLASS_RANGER, 45, TRUE);
  spell_level(SKILL_TRACK, CLASS_RANGER, 50, TRUE);
  spell_level(SKILL_WHIRLWIND, CLASS_RANGER, 56, TRUE);
  spell_level(SKILL_DOUBLE, CLASS_RANGER, 66, TRUE);
  spell_level(SKILL_STUNTOUCH, CLASS_RANGER, 70, TRUE);

  /* THIEF */
  spell_level(SKILL_SNEAK, CLASS_THIEF, 1, TRUE);
  spell_level(SKILL_STEAL, CLASS_THIEF, 2, TRUE);
  spell_level(SKILL_HIDE, CLASS_THIEF, 3, TRUE);
  spell_level(SKILL_BACKSTAB, CLASS_THIEF, 4, TRUE);
  spell_level(SKILL_PICK_LOCK, CLASS_THIEF, 14, TRUE);
  spell_level(SKILL_HEALWOUNDS, CLASS_THIEF, 16, TRUE);
  spell_level(SKILL_REDIRECT, CLASS_THIEF, 31, TRUE);
  spell_level(SKILL_DISARM, CLASS_THIEF, 44, TRUE);
  spell_level(SKILL_DODGE, CLASS_THIEF, 56, TRUE);
  spell_level(SKILL_POISONBLADE, CLASS_THIEF, 65, TRUE);
  spell_level(SKILL_CIRCLE, CLASS_THIEF, 70, TRUE);
  spell_level(SKILL_DUAL_BACKSTAB, CLASS_THIEF, 72, TRUE);
  spell_level(SKILL_GOUGE, CLASS_THIEF, 77, TRUE);

  /* MONK */
  spell_level(SKILL_KICK, CLASS_MONK, 1, TRUE);
  spell_level(SKILL_SNEAK, CLASS_MONK, 8, TRUE);
  spell_level(SKILL_HEALWOUNDS, CLASS_MONK, 14, TRUE);
  spell_level(SKILL_DIVE, CLASS_MONK, 20, TRUE);
  spell_level(SKILL_DOUBLE, CLASS_MONK, 26, TRUE);
  spell_level(SKILL_FEATHERFOOT, CLASS_MONK, 33, TRUE);
  spell_level(SKILL_DODGE, CLASS_MONK, 41, TRUE);
  spell_level(SKILL_THROW, CLASS_MONK, 47, TRUE);
  spell_level(SKILL_DISARM, CLASS_MONK, 54, TRUE);
  spell_level(SKILL_DISRUPT, CLASS_MONK, 60, TRUE);
  spell_level(SKILL_TRIPLE, CLASS_MONK, 66, TRUE);
  spell_level(SKILL_MINDCLOUD, CLASS_MONK, 74, TRUE);

  /* PSIONICIST */
  spell_level(SKILL_EGO_WHIP, CLASS_PSIONICIST, 1, TRUE);
  spell_level(SKILL_BIOFEEDBACK, CLASS_PSIONICIST, 3, TRUE);
  spell_level(SKILL_LEND_HEALTH, CLASS_PSIONICIST, 6, TRUE);
  spell_level(SKILL_ADRENALIN_CONTROL, CLASS_PSIONICIST, 9, TRUE);
  spell_level(SKILL_LIFE_DETECTION, CLASS_PSIONICIST, 12, TRUE);
  spell_level(SKILL_TELEKINESIS, CLASS_PSIONICIST, 15, TRUE);
  spell_level(SKILL_BALLISTIC_ATTACK, CLASS_PSIONICIST, 17, TRUE);
  spell_level(SKILL_CHAMELEON_POWER, CLASS_PSIONICIST, 20, TRUE);
  spell_level(SKILL_BODY_WEAPONRY, CLASS_PSIONICIST, 23, TRUE);
  spell_level(SKILL_DOMINATION, CLASS_PSIONICIST, 26, TRUE);
  spell_level(SKILL_PSYCHIC_CRUSH, CLASS_PSIONICIST, 28, TRUE);
  spell_level(SKILL_CELL_ADJUSTMENT, CLASS_PSIONICIST, 31, TRUE);
  spell_level(SKILL_GRAFT_WEAPON, CLASS_PSIONICIST, 33, TRUE);
  spell_level(SKILL_FLESH_ARMOR, CLASS_PSIONICIST, 35, TRUE);
  spell_level(SKILL_DANGER_SENSE, CLASS_PSIONICIST, 38, TRUE);
  spell_level(SPELL_REMEMBER, CLASS_PSIONICIST, 40, TRUE);
  spell_level(SKILL_PROBABILITY_TRAVEL, CLASS_PSIONICIST, 41, TRUE);
  spell_level(SKILL_MOLECULAR_AGITATION, CLASS_PSIONICIST, 44, TRUE);
  spell_level(SKILL_CANNIBALIZE, CLASS_PSIONICIST, 47, TRUE);
  spell_level(SKILL_DIMENSION_DOOR, CLASS_PSIONICIST, 50, TRUE);
  spell_level(SKILL_LIFE_DRAINING, CLASS_PSIONICIST, 53, TRUE);
  spell_level(SKILL_FEEL_LIGHT, CLASS_PSIONICIST, 56, TRUE);
  spell_level(SKILL_PSIONIC_BLAST, CLASS_PSIONICIST, 59, TRUE);
  spell_level(SKILL_ENERGY_CONTAINMENT, CLASS_PSIONICIST, 62, TRUE);
  spell_level(SKILL_METAMORPHOSIS, CLASS_PSIONICIST, 66, TRUE);
  spell_level(SKILL_DISPLACEMENT, CLASS_PSIONICIST, 67, TRUE);
  spell_level(SKILL_CLAIRVOYANCE, CLASS_PSIONICIST, 70, TRUE);
  spell_level(SKILL_DISINTEGRATE, CLASS_PSIONICIST, 73, TRUE);
  spell_level(SKILL_COMPLETE_HEALING, CLASS_PSIONICIST, 76, TRUE);
  spell_level(SKILL_SPLIT_PERSONALITY, CLASS_PSIONICIST, 79, TRUE);
  spell_level(SKILL_STASIS_FIELD, CLASS_PSIONICIST, 82, TRUE);
  spell_level(SKILL_DEATH_FIELD, CLASS_PSIONICIST, 85, TRUE);
  spell_level(SKILL_MAGNIFY, CLASS_PSIONICIST, 88, TRUE);
}
