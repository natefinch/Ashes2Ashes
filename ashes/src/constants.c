/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
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

const char circlemud_version[] = {
"Ashes to Ashes v.1.0, a heavy modification of CircleMUD v.3.00 bpl 11\r\n"};


/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */


/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "northeast",
  "northwest",
  "southeast",
  "southwest",
  "\n"
};

/* dir list with abreviated diagonal dirs */
const char *abbr_dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "ne",
  "nw",
  "se",
  "sw",
  "\n"
};

/* entering strings for walkin */
const char *edirs[] =
{
  "the south",
  "the west",
  "the north",
  "the east",
  "below",
  "above",
  "the southwest",
  "the southeast",
  "the northwest",
  "the northeast",
  "\n"
};

/* ROOM_x */
const char *room_bits[] = {
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "!TELEPORT",
  "!RELOCATE",
  "!QUIT",
  "!FLEE",
  "*",				/* BFS MARK */
  "MAGIC-DARK",
  "BEAMUP",
  "SNARE",
  "FLY",
  "DIM-DOOR",
  "STASIS",
  "\n"
};


/* EX_x */
const char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "SECRET",
  "HIDDEN",
  "\n"
};


/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "In Flight",
  "Desert",
  "Road",
  "Jungle",
  "\n"
};


/* SEX_x */
const char *genders[] =
{
  "Neutral",
  "Male",
  "Female",
  "\n"
};


/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "Falling",
  "\n"
};


/* PLR_x */
const char *player_bits[] = {
  "ASSASSIN",
  "BUILDING",
  "FROZEN",
  "DONTSET",
  "WRITING",
  "MAILING",
  "CSH",
  "SITEOK",
  "NOSHOUT",
  "NOTITLE",
  "DELETED",
  "LOADRM",
  "!WIZL",
  "!DEL",
  "INVST",
  "CRYO",
  "SNOOP",
  "TOURING",
  "\n"
};


/* MOB_x */
const char *action_bits[] = {
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "AWARE",
  "AGGR",
  "STAY-ZONE",
  "WIMPY",
  "AGGR_EVIL",
  "AGGR_GOOD",
  "AGGR_NEUTRAL",
  "MEMORY",
  "HELPER",
  "!CHARM",
  "!SUMMN",
  "!SLEEP",
  "!BASH",
  "!BLIND",
  "QHEAL",
  "!CURSE",
  "!SLOW",
  "!POISON",
  "NICE",
  "DOORSTOP",
  "HUNTER",
  "FLESH_EATER",
  "\n"
};


/* PRF_x */
const char *preference_bits[] = {
  "BRIEF",
  "COMPACT",
  "DEAF",
  "!TELL",
  "!ASAY",
  "!MUS",
  "REQ-ASSASSIN",
  "NOEXITS",
  "!HASS",
  "QUEST",
  "SUMMONABLE",
  "!REP",
  "LIGHT",
  "C1",
  "C2",
  "!WIZ",
  "L1",
  "L2",
  "!AUC",
  "!GOS",
  "!GTZ",
  "ROOMMFLAGS",
  "NODISTURB",
  "MORTLOG",
  "!ESP",
  "!FRAN",
  "AFK",
  "FORMAT",
  "!ARENA",
  "AVATAR",
  "COMBINE",
  "AUTOGOLD",
  "AUTOSPLIT",
  "AUTOLOOT",
  "\n"
};

/* GRA_x */
const char *grant_bits[] =
{
  "ASSASSIN",
  "BAN",
  "FORCE",
  "FREEZE",
  "FREIMB",
  "GECHO",
  "GREP",
  "IEDIT",
  "MUTE",
  "NOTITLE",
  "PEDIT",
  "REIMB",
  "RESTORE",
  "SET",
  "SNOOP",
  "SWITCH",
  "THAW",
  "UNAFFECT",
  "UNBAN",
  "ZCLOSE",
  "ZCREATE",
  "ZOPEN",
  "ZRESET",
  "ZTOP",
  "KILL",
  "SHOW",
  "ADVANCE",
  "REBOOT",
  "WIZLOCK",
  "AWARD",
  "\n"
};

/* AFF_x */
const char *affected_bits[] =
{
  "BLIND",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "WATERWALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "SPELL-SHIELD",
  "MAGIC-SHIELD",
  "SNEAK",
  "HIDE",
  "FLY",
  "CHARM",
  "(UN)HOLY",
  "SLOW",
  "HASTE",
  "MAGIC-LIGHT",
  "DIVINE-PROT",
  "LOWER-MR",
  "DISEASED",
  "FEEL_LIGHT",
  "ENERGY_CONT",
  "METAMORPHOSIS",
  "SPLIT_PERSONALITY",
  "MAGNIFY",
  "\n"
};


/* CON_x */
const char *connected_types[] = {
  "Playing",
  "Disconnecting",
  "Get name",
  "Confirm name",
  "Get password",
  "Get new PW",
  "Confirm new PW",
  "Select sex",
  "Select class",
  "Reading MOTD",
  "Main Menu",
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete 1",
  "Self-Delete 2",
  "Stat Rolling",
  "Rerolling",
  "Delete reason",
  "\n"
};


/* WEAR_x - for eq list */
const char *where[] = {
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               "
};


/* WEAR_x - for stat */
const char *equipment_types[] = {
  "Used as light",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "\n"
};


/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQ_CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "BEAMER_DEVICE",
  "DAMAGEABLE",
  "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "\n"
};


/* ITEM_x (extra bits) */
const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVISIBLE",
  "MAGIC",
  "!DROP",
  "BLESS",
  "!GOOD",
  "!EVIL",
  "!NEUTRAL",
  "!MAGE",
  "!CLERIC",
  "!THIEF",
  "!WARRIOR",
  "!SELL",
  "!PALADIN",
  "!RANGER",
  "!APAL",
  "!MONK",
  "<DUPE!>",
  "NOQUIT (YOU SHOULD NOT SEE THIS!)",
  "!PSI",
  "STRICT_CLASS",
  "STRONG_RESTRICT",
  "NOLOCATE",
  "\n"
};


/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "AGE",
  "CHAR-WEIGHT",
  "CHAR-HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "MANA-REGEN",
  "HIT-REGEN",
  "MOVE-REGEN",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING-PARA",
  "SAVING-ROD",
  "SAVING-PETRI",
  "SAVING-BREATH",
  "SAVING-SPELL",
  "MAGIC-RESISTANCE",
  "PSIONIC-RESISTANCE",
  "\n"
};


/* CONT_x */
const char *container_bits[] = {
  "CLOSEABLE",
  "PICKPROOF",
  "CLOSED",
  "LOCKED",
  "\n",
};


/* LIQ_x */
const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};


/* other constants for liquids ******************************************/


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13}
};


/* color of the various drinks */
const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear"
};


/* level of fullness for drink containers */
const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[] = {
  {-5, -4, 0, 0},	/* str = 0 */
  {-5, -4, 3, 1},	/* str = 1 */
  {-3, -2, 3, 2},
  {-3, -1, 10, 3},
  {-2, -1, 25, 4},
  {-2, -1, 55, 5},	/* str = 5 */
  {-1, 0, 80, 6},
  {-1, 0, 90, 7},
  {0, 0, 100, 8},
  {0, 0, 100, 9},
  {0, 0, 115, 10},	/* str = 10 */
  {0, 0, 115, 11},
  {0, 0, 140, 12},
  {0, 0, 140, 13},
  {0, 0, 170, 14},
  {0, 0, 170, 15},	/* str = 15 */
  {0, 1, 195, 16},
  {1, 1, 220, 18},
  {1, 2, 255, 20},	/* dex = 18 */
  {3, 7, 640, 40},
  {3, 8, 700, 40},	/* str = 20 */
  {4, 9, 810, 40},
  {4, 10, 970, 40},
  {5, 11, 1130, 40},
  {6, 12, 1440, 40},
  {7, 14, 1750, 40},	/* str = 25 */
  {1, 3, 280, 22},	/* str = 18/0 - 18-50 */
  {2, 3, 305, 24},	/* str = 18/51 - 18-75 */
  {2, 4, 330, 26},	/* str = 18/76 - 18-90 */
  {2, 5, 380, 28},	/* str = 18/91 - 18-99 */
  {3, 6, 480, 30}	/* str = 18/100 */
};



/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[] = {
  {-99, -99, -90, -99, -60},	/* dex = 0 */
  {-90, -90, -60, -90, -50},	/* dex = 1 */
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},	/* dex = 5 */
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},	/* dex = 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},		/* dex = 15 */
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},		/* dex = 18 */
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},		/* dex = 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}		/* dex = 25 */
};



/* [dex] apply (all) */
struct dex_app_type dex_app[] = {
  {-7, -7, 6},		/* dex = 0 */
  {-6, -6, 5},		/* dex = 1 */
  {-4, -4, 5},
  {-3, -3, 4},
  {-2, -2, 3},
  {-1, -1, 2},		/* dex = 5 */
  {0, 0, 1},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},		/* dex = 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},		/* dex = 15 */
  {1, 1, -2},
  {2, 2, -3},
  {2, 2, -4},		/* dex = 18 */
  {3, 3, -4},
  {3, 3, -4},		/* dex = 20 */
  {4, 4, -5},
  {4, 4, -5},
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6}		/* dex = 25 */
};



/* [con] apply (all) */
struct con_app_type con_app[] = {
  {-4, 20},		/* con = 0 */
  {-3, 25},		/* con = 1 */
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},		/* con = 5 */
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},		/* con = 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},		/* con = 15 */
  {2, 95},
  {2, 97},
  {3, 99},		/* con = 18 */
  {3, 99},
  {4, 99},		/* con = 20 */
  {5, 99},
  {5, 99},
  {5, 99},
  {6, 99},
  {6, 99}		/* con = 25 */
};



/* [int] apply (all) */
struct int_app_type int_app[] = {
  {3},		/* int = 0 */
  {5},		/* int = 1 */
  {7},
  {8},
  {9},
  {10},		/* int = 5 */
  {11},
  {12},
  {13},
  {15},
  {17},		/* int = 10 */
  {19},
  {22},
  {25},
  {30},
  {35},		/* int = 15 */
  {40},
  {45},
  {50},		/* int = 18 */
  {53},
  {55},		/* int = 20 */
  {56},
  {57},
  {58},
  {59},
  {60}		/* int = 25 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[] = {
  {0},	/* wis = 0 */
  {0},  /* wis = 1 */
  {0},
  {0},
  {0},
  {0},  /* wis = 5 */
  {0},
  {0},
  {0},
  {0},
  {0},  /* wis = 10 */
  {0},
  {2},
  {2},
  {3},
  {3},  /* wis = 15 */
  {3},
  {4},
  {5},	/* wis = 18 */
  {6},
  {6},  /* wis = 20 */
  {6},
  {6},
  {7},
  {7},
  {7}  /* wis = 25 */
};


const char *spec_proc_names[] =
{
  "NONE",
  "cityguard",
  "postmaster",
  "guildmaster",
  "eat corpses",
  "pick up junk",
  "poison bite",
  "thief",
  "magic user",
  "undead",
  "cleric",
  "random breath",
  "fire breath",
  "acid breath",
  "frost breath",
  "gas breath",
  "lightning breath",
  "bash",
  "berserk",
  "kick",
  "warrior",
  "\n"
};


const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",		/* 0 */
  "You feel less protected.",	/* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",		/* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Ice Storm!",
  "!Color Spray!",		/* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",		/* 15 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "You can no longer sense magic.",/* 20 */
  "You can no longer sense poison.",
  "!Dispel Evil!",
  "!Earthquake!",
  "",				/* Enchant weapon, used by chaos spec */
  "!Energy Drain!",		/* 25 */
  "!Fireball!",
  "!Harm!",
  "!Heal!",
  "You feel yourself exposed.",
  "!Lightning Bolt!",		/* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel a good power leave you.",
  "!Remove Curse!",		/* 35 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",			/* 40 */
  "!Implosion!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your surroundings.",
  "!Animate Dead!",		/* 45 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",	/* 50 */
  "Your feet seem less boyant.",
  "!Identify!",
  "Your wishes vanish.",
  "!Restore!",
  "!Power Heal!",			/* 55 */
  "You feel more vulnerable to spells.",
  "You feel less aided.",
  "!Word of Death!",
  "!Rejuvenate!",
  "!Cure Serious!",			/* 60 */
  "!Flamestrike!",
  "You float gently to the ground.",
  "The glowing ball above your head fades.",
  "You slow down.",
  "You can move normaly again.",	/* 65 */
  "You feel an evil power leave you.",
  "You no longer feel the protection of your diety.",
  "You feel less holy.",
  "You feel less unholy.",
  "!Group Infravision!",		/* 70 */
  "!Group Power Heal!",
  "!Group Rejuvenate!",
  "!Group Sanctuary!",
  "!Cone of Cold!",
  "!Revitalize!",			/* 75 */
  "!Teleport no Error!",
  "You feel more vulnerable to magical attacks.",
  "!Remember!",
  "!Calm!",
  "!Relocate!",				/* 80 */
  "You feel your resistance return to normal.",
  "!Battle Recall!",
  "!Amnedia!",
  "!Group Summon!",
  "!Group Invisible!",			/* 85 */
  "Your fever breaks."
};



const char *npc_class_types[] = {
  "Other",
  "Undead",
  "Humanoid",
  "Animal",
  "Dragon",
  "Giant",
  "\n"
};



const char *npc_size[] = {
  "Tiny",
  "Small",
  "Medium",
  "Large",
  "Huge",
  "Enormous",
  "\n"
};



const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4,
  9,
  8,
  7,
  6
};


const int movement_loss[] =
{
  1,	/* Inside     */
  2,	/* City       */
  2,	/* Field      */
  3,	/* Forest     */
  4,	/* Hills      */
  7,	/* Mountains  */
  3,	/* Swimming   */
  3,	/* Unswimable */
  1,	/* Flying     */
  5,    /* Underwater */
  6,    /* Desert     */
  1,    /* Road       */
  7     /* Jungle     */
};


const char *weekdays[] = {
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"
};


const char *month_name[] = {
  "Month of Withering",		/* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};


const char *weapon_types[] = {
  "HIT",
  "STING",
  "WHIP",
  "SLASH",
  "BITE",
  "BLUDGEON",
  "CRUSH",
  "POUND",
  "CLAW",
  "MAUL",
  "THRASH",
  "STAB",
  "BLAST",
  "PUNCH",
  "PIERCE",
  "\n"
};


const char *damtypes[] = {
  "FIRE",
  "ICE",
  "ENERGY",
  "BLUNT",
  "SLASH",
  "PIERCE",
  "\n"
};

const int resist_apply[] = {
  4,	/* #define WEAR_LIGHT      0 */
  3,	/* #define WEAR_FINGER_R   1 */
  3,	/* #define WEAR_FINGER_L   2 */
  3,	/* #define WEAR_NECK_1     3 */
  3,	/* #define WEAR_NECK_2     4 */
  8,	/* #define WEAR_BODY       5 */
  7,	/* #define WEAR_HEAD       6 */
  8,	/* #define WEAR_LEGS       7 */
  5,	/* #define WEAR_FEET       8 */
  5,	/* #define WEAR_HANDS      9 */
  7,	/* #define WEAR_ARMS      10 */
  8,	/* #define WEAR_SHIELD    11 */
  8,	/* #define WEAR_ABOUT     12 */
  6,	/* #define WEAR_WAIST     13 */
  3,	/* #define WEAR_WRIST_R   14 */
  3,	/* #define WEAR_WRIST_L   15 */
  7,	/* #define WEAR_WIELD     16 */
  4	/* #define WEAR_HOLD      17 */
};

const int dam_type_spell[] = {
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_FIRE,
  DAMTYPE_ENERGY,
  DAMTYPE_NONE,
  DAMTYPE_ICE,
  DAMTYPE_BLUNT,
  DAMTYPE_ENERGY,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_BLUNT,
  DAMTYPE_NONE,
  DAMTYPE_ENERGY,
  DAMTYPE_FIRE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_ENERGY,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_ENERGY,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_BLUNT,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_FIRE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_ICE
};


const int dam_type_skill[] = {
  DAMTYPE_PIERCE,
  DAMTYPE_BLUNT,
  DAMTYPE_NONE,
  DAMTYPE_BLUNT,
  DAMTYPE_NONE,
  DAMTYPE_BLUNT,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_PIERCE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_PIERCE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_PIERCE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_FIRE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_BLUNT,
  DAMTYPE_FIRE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE,
  DAMTYPE_NONE
};

const int dam_type_npcspecial[] = {
  DAMTYPE_FIRE,
  DAMTYPE_NONE,
  DAMTYPE_ICE,
  DAMTYPE_NONE,
  DAMTYPE_ENERGY
};

const int dam_type_attack[] = {
  DAMTYPE_BLUNT,
  DAMTYPE_PIERCE,
  DAMTYPE_SLASH | DAMTYPE_BLUNT,
  DAMTYPE_SLASH,
  DAMTYPE_PIERCE,
  DAMTYPE_BLUNT,
  DAMTYPE_BLUNT,
  DAMTYPE_BLUNT,
  DAMTYPE_SLASH,
  DAMTYPE_SLASH | DAMTYPE_BLUNT,
  DAMTYPE_SLASH,
  DAMTYPE_PIERCE,
  DAMTYPE_ENERGY,
  DAMTYPE_BLUNT,
  DAMTYPE_PIERCE
};
