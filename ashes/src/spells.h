/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

#define DEFAULT_STAFF_LVL	12
#define DEFAULT_WAND_LVL	12

#define CAST_UNDEFINED	-1
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4
#define CAST_ATTACK	5

#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)
#define MAG_PASS_ARG	(1 << 11)
#define MAG_CONTINUOUS	(1 << 12)


#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ICE_STORM               9
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_IMPLOSION              41
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD           45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD            46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_ARMOR            47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL             48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL           49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION            50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK	             51 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_IDENTIFY               52
#define SPELL_LIMITED_WISH           53
#define SPELL_RESTORE                54
#define SPELL_POWER_HEAL             55
#define SPELL_SPELL_SHIELD           56
#define SPELL_AID                    57
#define SPELL_WORD_OF_DEATH          58
#define SPELL_REJUVENATE             59
#define SPELL_CURE_SERIOUS           60
#define SPELL_FLAMESTRIKE            61
#define SPELL_FLY                    62
#define SPELL_MAGIC_LIGHT            63
#define SPELL_HASTE                  64
#define SPELL_SLOW                   65
#define SPELL_PROT_FROM_GOOD         66
#define SPELL_DIVINE_PROTECTION      67
#define SPELL_HOLY_WORD              68
#define SPELL_UNHOLY_WORD            69
#define SPELL_GROUP_INFRAVISION      70
#define SPELL_GROUP_POWER_HEAL       71
#define SPELL_GROUP_REJUVENATE       72
#define SPELL_GROUP_SANCTUARY        73
#define SPELL_CONE_OF_COLD           74
#define SPELL_REVITALIZE             75
#define SPELL_TELEPORT_NO_ERROR      76
#define SPELL_MAGIC_SHIELD           77
#define SPELL_REMEMBER               78
#define SPELL_CALM                   79
#define SPELL_RELOCATE               80
#define SPELL_LOWER_RESISTANCE       81
#define SPELL_BATTLE_RECALL          82
#define SPELL_AMNESIA                83
#define SPELL_GROUP_SUMMON           84
#define SPELL_GROUP_INVISIBLE        85
#define SPELL_CURE_DISEASE           86
/* Insert new spells here, up to MAX_SPELLS */
#define MAX_SPELLS		    150

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB              151 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  152 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  153 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  154 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             155 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_UNUSED1               156 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                157 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 158 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 159 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK		    160 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_DOUBLE		    161
#define SKILL_TRIPLE		    162
#define SKILL_QUAD		    163
#define SKILL_CIRCLE		    164
#define SKILL_GRAPPLE		    165
#define SKILL_BERSERK		    166
#define SKILL_SWIM		    167
#define SKILL_HEALWOUNDS	    168
#define SKILL_CAMOUFLAGE	    169
#define SKILL_FILET		    170
#define SKILL_DIVINE		    171
#define SKILL_SCAN		    172
#define SKILL_STUNTOUCH		    173
#define SKILL_BEFRIEND		    174
#define SKILL_FORAGE		    175
#define SKILL_SILENTWALK	    176
#define SKILL_SKIN		    177
#define SKILL_REDIRECT		    178
#define SKILL_DUAL_BACKSTAB	    179
#define SKILL_DISARM		    180
#define SKILL_HURL		    181
#define SKILL_WHIRLWIND		    182
#define SKILL_POISONBLADE	    183
#define SKILL_MEMORIZE		    184
#define SKILL_DISRUPT               185
#define SKILL_THROW                 186
#define SKILL_DIVE                  187
#define SKILL_MINDCLOUD             188
#define SKILL_GOUGE                 189
#define SKILL_DODGE                 190
#define SKILL_SNARE                 191
#define SKILL_FEATHERFOOT           192
#define SKILL_UNUSED2               193
#define PSI_START	194	/* Necisary marker for start of psi powers */
#define SKILL_CLAIRVOYANCE          194
#define SKILL_DANGER_SENSE          195
#define SKILL_FEEL_LIGHT            196
#define SKILL_DISINTEGRATE          197
#define SKILL_TELEKINESIS           198
#define SKILL_BALLISTIC_ATTACK      199
#define SKILL_MOLECULAR_AGITATION   200
#define SKILL_COMPLETE_HEALING      201
#define SKILL_DEATH_FIELD           202
#define SKILL_ENERGY_CONTAINMENT    203
#define SKILL_LIFE_DRAINING         204
#define SKILL_METAMORPHOSIS         205
#define SKILL_ADRENALIN_CONTROL     206
#define SKILL_BIOFEEDBACK           207
#define SKILL_BODY_WEAPONRY         208
#define SKILL_CELL_ADJUSTMENT       209
#define SKILL_CHAMELEON_POWER       210
#define SKILL_DISPLACEMENT          211
#define SKILL_FLESH_ARMOR           212
#define SKILL_GRAFT_WEAPON          213
#define SKILL_LEND_HEALTH           214
#define SKILL_PROBABILITY_TRAVEL    215
#define SKILL_DIMENSION_DOOR        216
#define SKILL_DOMINATION            217
#define SKILL_PSIONIC_BLAST         218
#define SKILL_EGO_WHIP              219
#define SKILL_LIFE_DETECTION        220
#define SKILL_PSYCHIC_CRUSH         221
#define SKILL_SPLIT_PERSONALITY     222
#define SKILL_CANNIBALIZE           223
#define SKILL_MAGNIFY               224
#define SKILL_STASIS_FIELD          225
#define PSI_END 	225	/* Necisary marker for end of psi powers */
/* New skills may be added here up to MAX_SKILLS (250) */


/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS (201+)
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects (such as SPELL_IDENTIFY used with scrolls of
 *  identify) or non-players (such as NPC-only spells).
 */

#define FIRE_BREATH                  251
#define GAS_BREATH                   252
#define FROST_BREATH                 253
#define ACID_BREATH                  254
#define LIGHTNING_BREATH             255

#define TOP_SPELL_DEFINE	     299
/* NEW NPC/OBJECT SPELLS can be inserted here up to 299 */


/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     300
#define TYPE_STING                   301
#define TYPE_WHIP                    302
#define TYPE_SLASH                   303
#define TYPE_BITE                    304
#define TYPE_BLUDGEON                305
#define TYPE_CRUSH                   306
#define TYPE_POUND                   307
#define TYPE_CLAW                    308
#define TYPE_MAUL                    309
#define TYPE_THRASH                  310
#define TYPE_STAB                    311
#define TYPE_BLAST                   312
#define TYPE_PUNCH                   313
#define TYPE_PIERCE                  314
/* new attack types can be added here - up to TYPE_SILENT */
#define TYPE_SILENT		     315 /* This marks the end of weapon types */
/* special types (not necisarily have messages messages) - up to TYPE_SUFFERING */
#define TYPE_DT			     316
#define TYPE_SECOND_WEAPON	     317

/* Silent damage with a type */
#define TYPE_SFIRE		     393
#define TYPE_SICE		     394
#define TYPE_SENERGY		     395
#define TYPE_SBLUNT		     396
#define TYPE_SSLASH		     397
#define TYPE_SPIERCE		     398

#define TYPE_SUFFERING		     399



#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type {
   byte min_position;	/* Position for caster	 */
   int mana_min;	/* Min amount of mana used by a spell (highest lev) */
   int mana_max;	/* Max amount of mana used by a spell (lowest lev) */
   int mana_change;	/* Change in mana used by spell from lev to lev */

   int min_level[NUM_CLASSES];
   int routines;
   byte violent;
   byte prac;		/* If spell/skill is practicable */
   int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
   char	*singular;
   char	*plural;
};


#define ASPELL(spellname) \
void	spellname(int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj)

#define ACAST(spellname) \
int	spellname(int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj)

#define MANUAL_SPELL(spellname)	spellname(level, caster, cvict, ovict);
#define RETURN_SPELL(spellname)	return(spellname(level, caster, cvict, ovict));

ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_calm);
ASPELL(spell_cure_blind);
ASPELL(spell_heal);
ASPELL(spell_amnesia);
ASPELL(fire_breath);
ASPELL(frost_breath);
ASPELL(acid_breath);
ACAST(spell_summon);
ACAST(spell_group_summon);
ACAST(spell_teleport);
ACAST(spell_control_weather);
ACAST(spell_relocate);
ACAST(spell_teleport_no_error);
ACAST(spell_limited_wish);
ACAST(spell_remember);

/* basic magic calling functions */

int find_skill_num(char *name);

void mag_damage(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_group_switch(int level, struct char_data *ch, struct char_data *tch, 
  int spellnum, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, int savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, int savetype);

void mag_areas(int level, struct char_data *ch, int spellnum, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
 int spellnum, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
 int spellnum, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int type);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
  int spellnum, int type);

void mag_creations(int level, struct char_data *ch, int spellnum);

void mag_continuous(int level, struct char_data *ch, struct char_data *victim,
                    struct obj_data *obj, int spellnum, int savetype);

int	call_magic(struct char_data *caster, struct char_data *cvict,
  struct obj_data *ovict, int spellnum, int level, int casttype);

void	mag_objectmagic(struct char_data *ch, struct obj_data *obj,
			char *argument);

int	cast_spell(struct char_data *ch, struct char_data *tch,
  struct obj_data *tobj, int spellnum, int silent);


/* other prototypes */
void spell_level(int spell, int class, int level, char prac);
void init_spell_levels(void);
char *skill_name(int num);
