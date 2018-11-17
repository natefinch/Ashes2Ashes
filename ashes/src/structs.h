/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */


/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database	*/
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)

/* typedef int(*SPECFUNC)(stuct char_data *ch, void *me, int cmd, char *argument); */
#define MAX_SPECPROCS 32

/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHEAST      6
#define NORTHWEST      7
#define SOUTHEAST      8
#define SOUTHWEST      9


/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK		(1 << 0)   /* Dark			*/
#define ROOM_DEATH		(1 << 1)   /* Death trap		*/
#define ROOM_NOMOB		(1 << 2)   /* MOBs not allowed		*/
#define ROOM_INDOORS		(1 << 3)   /* Indoors			*/
#define ROOM_PEACEFUL		(1 << 4)   /* Violence not allowed	*/
#define ROOM_SOUNDPROOF		(1 << 5)   /* Shouts, gossip blocked	*/
#define ROOM_NOTRACK		(1 << 6)   /* Track won't go through	*/
#define ROOM_NOMAGIC		(1 << 7)   /* Magic not allowed		*/
#define ROOM_TUNNEL		(1 << 8)   /* room for only 1 pers	*/
#define ROOM_PRIVATE		(1 << 9)   /* Can't teleport in		*/
#define ROOM_GODROOM		(1 << 10)  /* LVL_GOD+ only allowed	*/
#define ROOM_NOTELEPORT		(1 << 11)  /* Can't teleport out of the room */
#define ROOM_NORELOCATE		(1 << 12)  /* Can't teleport into the room */
#define ROOM_NOQUIT		(1 << 13)  /* Can't quit from this room */
#define ROOM_NOFLEE		(1 << 14)  /* Can't flee into this room */
#define ROOM_BFS_MARK		(1 << 15)  /* (R) breath-first srch mrk	*/
#define ROOM_MAGIC_DARK		(1 << 16)  /* Can't see, even with light*/
#define ROOM_TRANSPORT_OK	(1 << 17)  /* Can beam up from there with launchpad items */
#define ROOM_SNARE		(1 << 18)  /* (R) Snare skill marker	*/
#define ROOM_FLY		(1 << 19)  /* Must be flying to enter	*/
#define ROOM_DIM_DOOR		(1 << 20)  /* Room has a dimension door */
#define ROOM_STASIS		(1 << 21)  /* Room has a stasis field	*/


/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR		(1 << 0)   /* Exit is a door		*/
#define EX_CLOSED		(1 << 1)   /* The door is closed	*/
#define EX_LOCKED		(1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF		(1 << 3)   /* Lock can't be picked	*/
#define EX_SECRET		(1 << 4)   /* Exit can't be seen when closed */
#define EX_HIDDEN		(1 << 5)   /* Exit can't be seen when open */


/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0		   /* Indoors			*/
#define SECT_CITY            1		   /* In a city			*/
#define SECT_FIELD           2		   /* In a field		*/
#define SECT_FOREST          3		   /* In a forest		*/
#define SECT_HILLS           4		   /* In the hills		*/
#define SECT_MOUNTAIN        5		   /* On a mountain		*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_UNDERWATER	     8		   /* Underwater		*/
#define SECT_FLYING	     9		   /* Wheee!			*/
#define SECT_DESERT	    10		   /* In the desert		*/
#define SECT_ROAD	    11		   /* On a road			*/
#define SECT_JUNGLE	    12		   /* In the jungle		*/


#define WILDERNESS(stype)      (((stype) == SECT_FIELD) || \
				((stype) == SECT_FOREST) || \
				((stype) == SECT_HILLS) || \
				((stype) == SECT_MOUNTAIN) || \
				((stype) == SECT_JUNGLE))

/* char and mob-related defines *****************************************/


/* PC classes */
#define CLASS_UNDEFINED	  -1
#define CLASS_MAGIC_USER  0
#define CLASS_CLERIC      1
#define CLASS_THIEF       2
#define CLASS_WARRIOR     3
#define CLASS_PALADIN     4
#define CLASS_RANGER      5
#define CLASS_ANTIPALADIN 6
#define CLASS_MONK        7
#define CLASS_PSIONICIST  8

#define NUM_CLASSES	  9  /* This must be the number of classes!! */

/* Stat caps */
#define MAX_MANA	1500
#define MAX_HIT		1500
#define MAX_MOVE	1500


/* NPC classes (currently unused - feel free to implement!) */
#define CLASS_OTHER       0
#define CLASS_UNDEAD      1
#define CLASS_HUMANOID    2
#define CLASS_ANIMAL      3
#define CLASS_DRAGON      4
#define CLASS_GIANT       5


/* NPC sizes */
#define SIZE_TINY	0
#define SIZE_SMALL	1
#define SIZE_MEDIUM	2
#define SIZE_LARGE	3
#define SIZE_HUGE	4
#define SIZE_ENORMOUS	5


/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2


/* Positions */
#define POS_DEAD       0	/* dead			*/
#define POS_MORTALLYW  1	/* mortally wounded	*/
#define POS_INCAP      2	/* incapacitated	*/
#define POS_STUNNED    3	/* stunned		*/
#define POS_SLEEPING   4	/* sleeping		*/
#define POS_RESTING    5	/* resting		*/
#define POS_SITTING    6	/* sitting		*/
#define POS_FIGHTING   7	/* fighting		*/
#define POS_STANDING   8	/* standing		*/
#define POS_FALLING    9	/* falling (for wizard of oz spec proc */

/* Grant flags: used by char_data.player_specials.grants */
#define GRNT_ASSASSIN	(1 << 0)  /* Can accept people as assassins	*/
#define GRNT_BAN	(1 << 1)  /* Can ban players			*/
#define GRNT_FORCE	(1 << 2)  /* Can force				*/
#define GRNT_FREEZE	(1 << 3)  /* Can freeze players			*/
#define GRNT_FREIMB	(1 << 4)  /* Can force reimb players		*/
#define GRNT_GECHO	(1 << 5)  /* Can gecho				*/
#define GRNT_GREP	(1 << 6)  /* Can grep logs			*/
#define GRNT_IEDIT	(1 << 7)  /* Can modify items in game		*/
#define GRNT_MUTE	(1 << 8)  /* Can mute players			*/
#define GRNT_NOTITLE	(1 << 9)  /* Can notitle players		*/
#define GRNT_PEDIT	(1 << 10) /* Can edit players			*/
#define GRNT_REIMB	(1 << 11) /* Can reimb players			*/
#define GRNT_RESTORE	(1 << 12) /* Can restore morts/immorts		*/
#define GRNT_SET	(1 << 13) /* Can set things on players		*/
#define GRNT_SNOOP	(1 << 14) /* Can snoop players			*/
#define GRNT_SWITCH	(1 << 15) /* Can switch into mobs		*/
#define GRNT_THAW	(1 << 16) /* Can thaw players			*/
#define GRNT_UNAFFECT	(1 << 17) /* Can unaffect players		*/
#define GRNT_UNBAN	(1 << 18) /* Can unban sites			*/
#define GRNT_ZCLOSE	(1 << 19) /* Can close zones			*/
#define GRNT_ZCREATE	(1 << 20) /* Can create new zones		*/
#define GRNT_ZOPEN	(1 << 21) /* Can open zones			*/
#define GRNT_ZRESET	(1 << 22) /* Can reset zones			*/
#define GRNT_ZTOP	(1 << 23) /* Can change the top on zones	*/
#define GRNT_KILL	(1 << 24) /* Can kill, even though forbiden because of level */
#define GRNT_SHOW	(1 << 25) /* Can show all things even when	*/
#define GRNT_ADVANCE	(1 << 26) /* Can advance people			*/
#define GRNT_REBOOT	(1 << 27) /* Can reboot the mud			*/
#define GRNT_WIZLOCK	(1 << 28) /* Can wizlock the mud		*/
#define GRNT_AWARD	(1 << 29) /* Can award quest points		*/


/* Player flags: used by char_data.char_specials.act */
#define PLR_ASSASSIN	(1 << 0)   /* Player is an assassin		*/
#define PLR_BUILDING	(1 << 1)   /* Player is building and !disturb	*/
#define PLR_FROZEN	(1 << 2)   /* Player is frozen			*/
#define PLR_DONTSET     (1 << 3)   /* Don't EVER set (ISNPC bit)	*/
#define PLR_WRITING	(1 << 4)   /* Player writing (board/mail/olc)	*/
#define PLR_MAILING	(1 << 5)   /* Player is writing mail		*/
#define PLR_CRASH	(1 << 6)   /* Player needs to be crash-saved	*/
#define PLR_SITEOK	(1 << 7)   /* Player has been site-cleared	*/
#define PLR_NOSHOUT	(1 << 8)   /* Player not allowed to shout/goss	*/
#define PLR_NOTITLE	(1 << 9)   /* Player not allowed to set title	*/
#define PLR_DELETED	(1 << 10)  /* Player deleted - space reusable	*/
#define PLR_LOADROOM	(1 << 11)  /* Player uses nonstandard loadroom	*/
#define PLR_NOWIZLIST	(1 << 12)  /* Player shouldn't be on wizlist	*/
#define PLR_NODELETE	(1 << 13)  /* Player shouldn't be deleted	*/
#define PLR_INVSTART	(1 << 14)  /* Player should enter game wizinvis	*/
#define PLR_CRYO	(1 << 15)  /* Player is cryo-saved (purge prog)	*/
#define PLR_SNOOP	(1 << 16)  /* Player is snooped into syslog	*/
#define PLR_TOURING	(1 << 17)  /* Player is on a tour		*/


/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         (1 << 0)  /* Mob has a callable spec-proc	*/
#define MOB_SENTINEL     (1 << 1)  /* Mob should not move		*/
#define MOB_SCAVENGER    (1 << 2)  /* Mob picks up stuff on the ground	*/
#define MOB_ISNPC        (1 << 3)  /* (R) Automatically set on all Mobs	*/
#define MOB_AWARE	 (1 << 4)  /* Mob can't be backstabbed		*/
#define MOB_AGGRESSIVE   (1 << 5)  /* Mob hits players in the room	*/
#define MOB_STAY_ZONE    (1 << 6)  /* Mob shouldn't wander out of zone	*/
#define MOB_WIMPY        (1 << 7)  /* Mob flees if severely injured	*/
#define MOB_AGGR_EVIL	 (1 << 8)  /* auto attack evil PC's		*/
#define MOB_AGGR_GOOD	 (1 << 9)  /* auto attack good PC's		*/
#define MOB_AGGR_NEUTRAL (1 << 10) /* auto attack neutral PC's		*/
#define MOB_MEMORY	 (1 << 11) /* remember attackers if attacked	*/
#define MOB_HELPER	 (1 << 12) /* attack PCs fighting other NPCs	*/
#define MOB_NOCHARM	 (1 << 13) /* Mob can't be charmed		*/
#define MOB_NOSUMMON	 (1 << 14) /* Mob can't be summoned		*/
#define MOB_NOSLEEP	 (1 << 15) /* Mob can't be slept		*/
#define MOB_NOBASH	 (1 << 16) /* Mob can't be bashed (e.g. trees)	*/
#define MOB_NOBLIND	 (1 << 17) /* Mob can't be blinded		*/
#define MOB_QHEAL	 (1 << 18) /* Mob quick-heals			*/
#define MOB_NOCURSE	 (1 << 19) /* Mob can't be cursed		*/
#define MOB_NOSLOW	 (1 << 20) /* Mob can't be slowed		*/
#define MOB_NOPOISON	 (1 << 21) /* Mob can't be poisoned		*/
#define MOB_NICE	 (1 << 22) /* Mob won't kill players that are stunned */
#define MOB_DOORSTOP	 (1 << 23) /* Mob can't be seen, killed, or affected */
#define MOB_HUNTER	 (1 << 24) /* Mob will track last person it was fighting (must have memory and not be sentinal also) */
#define MOB_FLESH_EATER  (1 << 25) /* Mob only affected by narual ac 2/3 */


/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       (1 << 0)  /* Room descs won't normally be shown	*/
#define PRF_COMPACT     (1 << 1)  /* No extra CRLF pair before prompts	*/
#define PRF_DEAF	(1 << 2)  /* Can't hear shouts			*/
#define PRF_NOTELL	(1 << 3)  /* Can't receive tells		*/
#define PRF_NOASAY	(1 << 4)  /* Can't hear asay			*/
#define PRF_NOMUS	(1 << 5)  /* Can't hear music			*/
#define PRF_REQ_ASS	(1 << 6)  /* Requesting assassin		*/
#define PRF_NOEXITS	(1 << 7)  /* Can't see exits in a room		*/
#define PRF_NOHASSLE	(1 << 8)  /* Aggr mobs won't attack		*/
#define PRF_QUEST	(1 << 9)  /* On quest				*/
#define PRF_SUMMONABLE	(1 << 10) /* Can be summoned			*/
#define PRF_NOREPEAT	(1 << 11) /* No repetition of comm commands	*/
#define PRF_HOLYLIGHT	(1 << 12) /* Can see in dark			*/
#define PRF_COLOR_1	(1 << 13) /* Color (low bit)			*/
#define PRF_COLOR_2	(1 << 14) /* Color (high bit)			*/
#define PRF_NOWIZ	(1 << 15) /* Can't hear wizline			*/
#define PRF_LOG1	(1 << 16) /* On-line System Log (low bit)	*/
#define PRF_LOG2	(1 << 17) /* On-line System Log (high bit)	*/
#define PRF_NOAUCT	(1 << 18) /* Can't hear auction channel		*/
#define PRF_NOGOSS	(1 << 19) /* Can't hear gossip channel		*/
#define PRF_NOGRATZ	(1 << 20) /* Can't hear grats channel		*/
#define PRF_ROOMFLAGS	(1 << 21) /* Can see room flags (ROOM_x)	*/
#define PRF_NODISTURB	(1 << 22) /* Doesn't see anything when editing	*/
#define PRF_MORTLOG	(1 << 23) /* Has mortlog on			*/
#define PRF_NOESP	(1 << 24) /* Can't hear spanish channel		*/
#define PRF_NOFRAN	(1 << 25) /* Can't hear french channel		*/
#define PRF_AFK		(1 << 26) /* Is AFK				*/
#define PRF_FORMAT	(1 << 27) /* Auto formats text in OLC		*/
#define PRF_NOARENA	(1 << 28) /* Can't hear arena channel		*/
#define PRF_AVTR	(1 << 29) /* Immortal can be killed (physical form) */
#define PRF_COMBINE	(1 << 30) /* Items in inventory will be combined */
#define PRF_AUTOGOLD	((long long)1 << 31) /* Auto loot gold		*/
#define PRF_AUTOSPLIT	((long long)1 << 32) /* Auto split gold		*/
#define PRF_AUTOLOOT	((long long)1 << 33) /* Auto loot items		*/
#define PRF_NOMENU	((long long)1 << 34) /* Ignore don't display olc menu */


/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_BLIND             (1 << 0)	   /* (R) Char is blind		*/
#define AFF_INVISIBLE         (1 << 1)	   /* Char is invisible		*/
#define AFF_DETECT_ALIGN      (1 << 2)	   /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      (1 << 3)	   /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      (1 << 4)	   /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        (1 << 5)	   /* Char can sense hidden life*/
#define AFF_WATERWALK	      (1 << 6)	   /* Char can walk on water	*/
#define AFF_SANCTUARY         (1 << 7)	   /* Char protected by sanct.	*/
#define AFF_GROUP             (1 << 8)	   /* (R) Char is grouped	*/
#define AFF_CURSE             (1 << 9)	   /* Char is cursed		*/
#define AFF_INFRAVISION       (1 << 10)	   /* Char can see in dark	*/
#define AFF_POISON            (1 << 11)	   /* (R) Char is poisoned	*/
#define AFF_PROTECT_EVIL      (1 << 12)	   /* Char protected from evil  */
#define AFF_PROTECT_GOOD      (1 << 13)	   /* Char protected from good  */
#define AFF_SLEEP             (1 << 14)	   /* (R) Char magically asleep	*/
#define AFF_NOTRACK	      (1 << 15)	   /* Char can't be tracked	*/
#define AFF_SPELLSHIELD	      (1 << 16)	   /* Room for future expansion	*/
#define AFF_MAGICSHIELD	      (1 << 17)	   /* Room for future expansion	*/
#define AFF_SNEAK             (1 << 18)	   /* Char can move quietly	*/
#define AFF_HIDE              (1 << 19)	   /* Char is hidden		*/
#define AFF_FLY		      (1 << 20)	   /* Room for future expansion	*/
#define AFF_CHARM             (1 << 21)	   /* Char is charmed		*/
#define AFF_UN_HOLY           (1 << 22)    /* Holy/Unholy word		*/
#define AFF_SLOW              (1 << 23)    /* Slowed			*/
#define AFF_HASTE             (1 << 24)    /* Hasted			*/
#define AFF_MAGIC_LIGHT       (1 << 25)    /* Magic light		*/
#define AFF_DIVINE_PROT       (1 << 26)    /* Divine protection		*/
#define AFF_LOWER_MR          (1 << 27)    /* Lower resistance		*/
#define AFF_DISEASE           (1 << 28)    /* Will spread disease	*/
#define AFF_FEEL_LIGHT        (1 << 29)    /* Can see despite blindness */
#define AFF_ENERGY_CONT       (1 << 30)    /* Save for half from fire/ice/energy */
#define AFF_METAMORPHOSIS     ((long long)1 << 31) /* Char is transformed, gives a bonus attack */
#define AFF_SPLIT_PERSONALITY ((long long)1 << 32) /* Psi powers have half delay */
#define AFF_MAGNIFY           ((long long)1 << 33) /* Double power psi skills */


/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING	 0		/* Playing - Nominal state	*/
#define CON_CLOSE	 1		/* Disconnecting		*/
#define CON_GET_NAME	 2		/* By what name ..?		*/
#define CON_NAME_CNFRM	 3		/* Did I get that right, x?	*/
#define CON_PASSWORD	 4		/* Password:			*/
#define CON_NEWPASSWD	 5		/* Give me a password for x	*/
#define CON_CNFPASSWD	 6		/* Please retype password:	*/
#define CON_QSEX	 7		/* Sex?				*/
#define CON_QCLASS	 8		/* Class?			*/
#define CON_RMOTD	 9		/* PRESS RETURN after MOTD	*/
#define CON_MENU	 10		/* Your choice: (main menu)	*/
#define CON_EXDESC	 11		/* Enter a new description:	*/
#define CON_CHPWD_GETOLD 12		/* Changing passwd: get old	*/
#define CON_CHPWD_GETNEW 13		/* Changing passwd: get new	*/
#define CON_CHPWD_VRFY   14		/* Verify new password		*/
#define CON_DELCNF1	 15		/* Delete confirmation 1	*/
#define CON_DELCNF2	 16		/* Delete confirmation 2	*/
#define CON_STAT_CONF	 17		/* Stat rolling			*/
#define CON_REROLLING	 18		/* Rerolling			*/
#define CON_DELMSG	 19		/* Entering reason for delete	*/


/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WEAR_WIELD     16
#define WEAR_HOLD      17

#define NUM_WEARS      18	/* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1		/* Item is a light source	*/
#define ITEM_SCROLL     2		/* Item is a scroll		*/
#define ITEM_WAND       3		/* Item is a wand		*/
#define ITEM_STAFF      4		/* Item is a staff		*/
#define ITEM_WEAPON     5		/* Item is a weapon		*/
#define ITEM_FIREWEAPON 6		/* Unimplemented		*/
#define ITEM_MISSILE    7		/* Unimplemented		*/
#define ITEM_TREASURE   8		/* Item is a treasure, not gold	*/
#define ITEM_ARMOR      9		/* Item is armor		*/
#define ITEM_POTION    10 		/* Item is a potion		*/
#define ITEM_WORN      11		/* Unimplemented		*/
#define ITEM_OTHER     12		/* Misc object			*/
#define ITEM_TRASH     13		/* Trash - shopkeeps won't buy	*/
#define ITEM_TRAP      14		/* Unimplemented		*/
#define ITEM_CONTAINER 15		/* Item is a container		*/
#define ITEM_NOTE      16		/* Item is note 		*/
#define ITEM_DRINKCON  17		/* Item is a drink container	*/
#define ITEM_KEY       18		/* Item is a key		*/
#define ITEM_FOOD      19		/* Item is food			*/
#define ITEM_MONEY     20		/* Item is money (gold)		*/
#define ITEM_PEN       21		/* Item is a pen		*/
#define ITEM_BOAT      22		/* Item is a boat		*/
#define ITEM_FOUNTAIN  23		/* Item is a fountain		*/
#define ITEM_BEAMER    24		/* Lets you beam up/down within zone */
#define ITEM_DAMAGEABLE 25		/* Weakens with hits		*/


/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE		(1 << 0)  /* Item can be takes		*/
#define ITEM_WEAR_FINGER	(1 << 1)  /* Can be worn on finger	*/
#define ITEM_WEAR_NECK		(1 << 2)  /* Can be worn around neck 	*/
#define ITEM_WEAR_BODY		(1 << 3)  /* Can be worn on body 	*/
#define ITEM_WEAR_HEAD		(1 << 4)  /* Can be worn on head 	*/
#define ITEM_WEAR_LEGS		(1 << 5)  /* Can be worn on legs	*/
#define ITEM_WEAR_FEET		(1 << 6)  /* Can be worn on feet	*/
#define ITEM_WEAR_HANDS		(1 << 7)  /* Can be worn on hands	*/
#define ITEM_WEAR_ARMS		(1 << 8)  /* Can be worn on arms	*/
#define ITEM_WEAR_SHIELD	(1 << 9)  /* Can be used as a shield	*/
#define ITEM_WEAR_ABOUT		(1 << 10) /* Can be worn about body 	*/
#define ITEM_WEAR_WAIST 	(1 << 11) /* Can be worn around waist 	*/
#define ITEM_WEAR_WRIST		(1 << 12) /* Can be worn on wrist 	*/
#define ITEM_WEAR_WIELD		(1 << 13) /* Can be wielded		*/
#define ITEM_WEAR_HOLD		(1 << 14) /* Can be held		*/


/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW          (1 << 0)	/* Item is glowing		*/
#define ITEM_HUM           (1 << 1)	/* Item is humming		*/
#define ITEM_NORENT        (1 << 2)	/* Item cannot be rented	*/
#define ITEM_NODONATE      (1 << 3)	/* Item cannot be donated	*/
#define ITEM_NOINVIS	   (1 << 4)	/* Item cannot be made invis	*/
#define ITEM_INVISIBLE     (1 << 5)	/* Item is invisible		*/
#define ITEM_MAGIC         (1 << 6)	/* Item is magical		*/
#define ITEM_NODROP        (1 << 7)	/* Item is cursed: can't drop	*/
#define ITEM_BLESS         (1 << 8)	/* Item is blessed		*/
#define ITEM_ANTI_GOOD     (1 << 9)	/* Not usable by good people	*/
#define ITEM_ANTI_EVIL     (1 << 10)	/* Not usable by evil people	*/
#define ITEM_ANTI_NEUTRAL  (1 << 11)	/* Not usable by neutral people	*/
#define ITEM_ANTI_MAGIC_USER (1 << 12)	/* Not usable by mages		*/
#define ITEM_ANTI_CLERIC   (1 << 13)	/* Not usable by clerics	*/
#define ITEM_ANTI_THIEF	   (1 << 14)	/* Not usable by thieves	*/
#define ITEM_ANTI_WARRIOR  (1 << 15)	/* Not usable by warriors	*/
#define ITEM_NOSELL	   (1 << 16)	/* Shopkeepers won't touch it	*/
#define ITEM_ANTI_PALADIN  (1 << 17)	/* Not usable by paladins	*/
#define ITEM_ANTI_RANGER   (1 << 18)	/* Not usable by rangers	*/
#define ITEM_ANTI_ANTIPALADIN (1 << 19)	/* Not usable by antipaladins	*/
#define ITEM_ANTI_MONK     (1 << 20)	/* Not usable by monks		*/
#define ITEM_DUPE          (1 << 21)	/* Item is duped!		*/
#define ITEM_NOQUIT        (1 << 22)	/* Item need extracting to quit */
#define ITEM_ANTI_PSIONICIST (1 << 23)	/* Not usable by psionicists	*/
#define ITEM_STRICT_CLASSES (1 << 24)	/* Item only wearable by person of exact class(es) allowed */
#define ITEM_STRONG_RESTRICT (1 << 25)	/* Anti flags prevent multiclass combo also */
#define ITEM_NOLOCATE      (1 << 26)	/* Item cannot be located	*/


/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0	/* No effect			*/
#define APPLY_STR               1	/* Apply to strength		*/
#define APPLY_DEX               2	/* Apply to dexterity		*/
#define APPLY_INT               3	/* Apply to constitution	*/
#define APPLY_WIS               4	/* Apply to wisdom		*/
#define APPLY_CON               5	/* Apply to constitution	*/
#define APPLY_CHA		6	/* Apply to charisma		*/
#define APPLY_AGE               7	/* Apply to age			*/
#define APPLY_CHAR_WEIGHT       8	/* Apply to weight		*/
#define APPLY_CHAR_HEIGHT       9	/* Apply to height		*/
#define APPLY_MANA             10	/* Apply to max mana		*/
#define APPLY_HIT              11	/* Apply to max hit points	*/
#define APPLY_MOVE             12	/* Apply to max move points	*/
#define APPLY_MANA_REGEN       13	/* Apply to mana regen		*/
#define APPLY_HP_REGEN         14	/* Apply to hp regen		*/
#define APPLY_MOVE_REGEN       15	/* Apply to move regen		*/
#define APPLY_AC               16	/* Apply to Armor Class		*/
#define APPLY_HITROLL          17	/* Apply to hitroll		*/
#define APPLY_DAMROLL          18	/* Apply to damage roll		*/
#define APPLY_SAVING_PARA      19	/* Apply to save throw: paralz	*/
#define APPLY_SAVING_ROD       20	/* Apply to save throw: rods	*/
#define APPLY_SAVING_PETRI     21	/* Apply to save throw: petrif	*/
#define APPLY_SAVING_BREATH    22	/* Apply to save throw: breath	*/
#define APPLY_SAVING_SPELL     23	/* Apply to save throw: spells	*/
#define APPLY_MR               24	/* Apply to magic resistance	*/
#define APPLY_PR               25	/* Apply to psionic resistance	*/


/* Types of damage for immune/resist/weak */
#define DAMTYPE_NONE	0		/* No type			*/
#define DAMTYPE_FIRE	(1 << 0)	/* Heat related attacks		*/
#define DAMTYPE_ICE	(1 << 1)	/* Cold related attacks		*/
#define DAMTYPE_ENERGY	(1 << 2)	/* Electric and blasting attacks */
#define DAMTYPE_BLUNT	(1 << 3)	/* Clubs, maces, etc		*/
#define DAMTYPE_SLASH	(1 << 4)	/* Swords, claws, etc		*/
#define DAMTYPE_PIERCE	(1 << 5)	/* Daggers, arrows, etc		*/


/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)	/* Container can be closed	*/
#define CONT_PICKPROOF      (1 << 1)	/* Container is pickproof	*/
#define CONT_CLOSED         (1 << 2)	/* Container is closed		*/
#define CONT_LOCKED         (1 << 3)	/* Container is locked		*/


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15


/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2


/* Sun state for weather_data */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


/* other #defined constants **********************************************/

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 */
#define LVL_IMPL	105
#define LVL_CIMP	104
#define LVL_ASST	103
#define LVL_IMMORT	102
#define LVL_HERO	101

#define TITLE_IMPL	" IMP  "
#define TITLE_CIMP	"ADMIN "
#define TITLE_ASST	"STAFF "
#define TITLE_IMMORT	"IMM   "
#define TITLE_HERO	"TOKEN "

#define LVL_FREEZE	LVL_CIMP
#define AFFECT_SAME_LEVEL LVL_CIMP /* Level at which imms can affect imms of same level */

#define NUM_OF_DIRS	10	/* number of directions in a room (nsewud) */
/* Originaly in interpreter.h, moved here for NUM_OF_DIRS */
#define IS_MOVE(cmdnum) (cmdnum >= 1 && cmdnum <= NUM_OF_DIRS)

#define OPT_USEC	100000	/* 10 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC

#define PULSE_ZONE      (5 RL_SEC)
#define PULSE_MOBILE    (10 RL_SEC)
#define PULSE_VIOLENCE  (2 RL_SEC)
#define PULSE_SPELL     (2 RL_SEC)	/* Should be same as violence, generaly */
#define PULSE_ROOM	(75 RL_SEC)	/* once per tic */
#define PULSE_ITEM	(75 RL_SEC)	/* once per tic */
#define PULSE_AUCTION	(13 RL_SEC)

#define SMALL_BUFSIZE		1024
#define LARGE_BUFSIZE		(12 * 1024)
#define GARBAGE_SPACE		32

#define MAX_STRING_LENGTH	8192
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		60
#define MAX_PROMPT_LENGTH	80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_NAME_LENGTH		15  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_WALK_LENGTH		80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH	80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH		50  /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH		240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE		3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS		250 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT		6 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_ALIAS		30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_ALIAS_LENGTH	12  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_ALIAS_REPLACE	128 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_REMEMBER		4   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_MORT_SAVED_MAIL	2
#define MAX_IMM_SAVED_MAIL	10
#define MAX_ADMIN_SAVED_MAIL	25
#define MAX_PROMPT_TOKENS	18


/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef char			bool;

#ifndef CIRCLE_WINDOWS
typedef char			byte;
#endif

typedef sh_int	room_num;
typedef sh_int	obj_num;


/* For saving aliases */
struct salias {
   char a_orig[MAX_ALIAS_LENGTH+1];
   char a_new[MAX_ALIAS_REPLACE+1];
   int a_type;
};

/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/


/* object flags; used in obj_data */
struct obj_flag_data {
   int	value[4];	/* Values of the item (see list)    */
   byte type_flag;	/* Type of item			    */
   long	wear_flags;	/* Where you can wear it	    */
   long	extra_flags;	/* If it hums, glows, etc.	    */
   int	weight;		/* Weigt what else                  */
   int	cost;		/* Value when sold (gp.)            */
   int	cost_per_day;	/* Cost to keep pr. real day        */
   sbyte timer;		/* Timer for object                 */
   long long bitvector;	/* To set chars bits                */
   int  immune;		/* To set chars immunities          */
   int  weak;		/* To set chars weaknesses          */
   int  resist;		/* To set chars resistances         */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};


/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_num item_number;		/* Where in data-base			*/
   room_num in_room;		/* In what room -1 when conta/carr	*/

   long id;			/* Unique item ID num			*/

   int cmd;			/* Zone command that loaded this item	*/
   bool from_load;		/* If obj is still where it loaded	*/

   byte poisoned;		/* for poisonblade skill		*/

   struct obj_flag_data obj_flags; /* Object information		*/
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   char	*name;                    /* Title of object :get etc.        */
   char	*description;		  /* When in room                     */
   char	*short_description;       /* when worn/carry/in cont.         */
   char	*action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;	  /* Worn by?			      */
   sh_int worn_on;		  /* Worn where?		      */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */
struct obj_file_elem {
   obj_num item_number;

   int	value[4];
   long wear_flags;
   long	extra_flags;
   int	weight;
   int	timer; /* make this a sbyte next time convert or wipe rent files */
   long long	bitvector;
   int  immune;
   int  weak;
   int  resist;
   sh_int worn_on;
   long id;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
   long	time;
   int	rentcode;
   int	net_cost_per_diem;
   long	gold;
   long	account;
   int	nitems;
   int	spare0;
   int	spare1;
   int	spare2;
   int	spare3;
   int	spare4;
   int	spare5;
   int	spare6;
   int	spare7;
};
/* ======================================================================= */


/* room-related structures ************************************************/


struct room_direction_data {
   char	*general_description;       /* When look DIR.			*/

   char	*keyword;		/* for open/close			*/

   sh_int exit_info;		/* Exit info				*/
   obj_num key;			/* Key's number (-1 for no key)		*/
   room_num to_room;		/* Where direction leads (NOWHERE)	*/
};


/* ================== Memory Structure for room ======================= */
struct room_data {
   room_num number;		/* Rooms number	(vnum)		      */
   sh_int zone;                 /* Room zone (for resetting)          */
   byte	sector_type;            /* sector type (move/hide)            */
   char	*name;                  /* Rooms name 'You are ...'           */
   char	*description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   long room_flags;		/* DEATH,DARK ... etc                 */

   sh_int dt_numdice;		/*                                    */
   sh_int dt_sizedice;		/* For DT damage computation          */
   sh_int dt_add;		/*                                    */
   ubyte dt_percent;		/* Min percentage damage from DT      */
   sbyte dt_repeat;		/* Keep damaging every pulse_violence */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
};
/* ====================================================================== */


/* char-related structures ************************************************/

/* MOB Prog data types */
struct mob_prog_act_list {
  struct mob_prog_act_list *next;
  char *buf;
  struct char_data *ch;
  struct obj_data *obj;
  void *vo;
};
typedef struct mob_prog_act_list MPROG_ACT_LIST;

struct mob_prog_data {
  struct mob_prog_data *next;
  int type;
  char *arglist;
  char *comlist;
};
typedef struct mob_prog_data MPROG_DATA;

extern bool MOBTrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
#define COMMANDTRAP_PROG 2048
#define KEYWORD_PROG    4096


/* Arena related defines and structures */
#define ARENA_OPEN 	0
#define ARENA_NOALL	1
#define ARENA_NOGROUP	2
#define ARENA_CLOSED	3

#define GOD_ZONE	12
#define MIDGAARD_ZONE	30
#define QUEST_ZONE	119
#define ARENA_ZONE 	221

struct player_arena_data {
  long kill_mode;	/* Can only kill those with same kill mode	*/
  long challenging;	/* ID of player you're challenging (-1 for all)	*/
  long challenged_by;	/* ID of player that challenged you (-1 for groups) */
  sh_int hit;		/* Hp at time of entering arena			*/
  sh_int mana;		/* Mana at time of entering the arena		*/
  sh_int move;		/* Move at time of entering the arena		*/
};


/* memory structure for characters */
struct memory_rec_struct {
   long	id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   int hours, day, month;
   sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   long	played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char	passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char	*name;	       /* PC / NPC s name (kill ...  )         */
   char	*short_descr;  /* for NPC 'actions'                    */
   char	*long_descr;   /* for 'look'			       */
   char	*description;  /* Extra descriptions                   */
   char	*title;        /* PC / NPC's title                     */
   byte sex;           /* PC / NPC's sex                       */
   byte class;         /* PC / NPC's class		       */
   sh_int level;       /* PC / NPC's level                     */
   sh_int hometown;    /* PC s Hometown (zone)                 */
   struct time_data time;  /* PC's AGE in days                 */
   sh_int weight;      /* PC / NPC's weight                    */
   ubyte height;       /* PC / NPC's height                    */
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   sh_int mana;
   sh_int max_mana;     /* Max move for PC/NPC			   */
   sh_int hit;
   sh_int max_hit;      /* Max hit for PC/NPC                      */
   sh_int move;
   sh_int max_move;     /* Max move for PC/NPC                     */

   int armor;           /* -1000..1000 make this sh_int next convert/wipe */
   long	gold;           /* Money carried                           */
   long	bank_gold;	/* Gold the char has in a bank account	   */
   long	exp;            /* The experience of the player            */

   sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
   sbyte damroll;       /* Any bonus or penalty to the damage roll */
};


/* 
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   int	alignment;		/* +-1000 for alignments, make this sh_int next convert/wipe */
   long	idnum;			/* player's idnum; -1 for mobiles	*/
   long long	act;		/* act flag for NPC's; player flag for PC's */

   long long	affected_by;	/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};

/* Data for continuous spells */
struct spcontinuous {
   int sptimer;			/* Spell timmer for			*/
   int spspell;			/* Spell number for			*/
   int spcost;			/* Cost per iteration			*/
   void *sptarget;		/* Target, can be char, item, etc	*/
   long spdata1;		/* Data					*/
   long spdata2;
   long spdata3;
   long spdata4;
   struct spcontinuous *next;
};

/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
   struct char_data *fighting;	/* Opponent				*/
   struct char_data *hunting;	/* Char hunted by this char		*/

   char *prompt;		/* character's prompt string		*/

   byte position;		/* Standing, fighting, sleeping, etc.	*/

   sh_int spdrained;		/* Timer for casting draining spells	*/
   sh_int spnocast;		/* Timer for casting after draining	*/
   struct spcontinuous *spcont;	/* Continuous spell info		*/

   int	carry_weight;		/* Carried weight			*/
   byte carry_items;		/* Number of items carried		*/
   int	timer;			/* Timer for update			*/

   sh_int  mana_regen_add;	/* Mana regen gain from affectations	*/
   sh_int  hp_regen_add;	/* Hp regen gain from affectations	*/
   sh_int  move_regen_add;	/* Move regen gain from affectations	*/

   sh_int  vc;			/* virtual channel char belongs to	*/
   sh_int  vci;			/* virtual channel char is invited to	*/
   sh_int  vcm;			/* is this the moderator		*/
   sh_int  vcs;			/* is the char squelched		*/

   sh_int  magic_resistance;	/* magic resistance percentage		*/
   sh_int  psionic_resistance;	/* psionic resistance percentage	*/
   sh_int  stun_length;		/* cannot attack for this many combat rounds */
   long long affected_by;	/* This is the real bitvector for what
                                 * a char is affected by, other is what
                                 * is saved in file */

   struct char_special_data_saved saved; /* constants saved in plrfile	*/
};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   byte skills[MAX_SKILLS+1];	/* array of skills plus skill 0		*/
   byte PADDING0;		/* used to be spells_to_learn		*/
   bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
   int  remember[MAX_REMEMBER]; /* remember slots for teleport no error, etc */
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   byte freeze_level;		/* Level of god who froze char, if any	*/
   sh_int invis_level;		/* level of invisibility		*/
   room_num load_room;		/* Which room to place char in		*/
   long long	pref;		/* preference flags for PC's.		*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   sbyte conditions[3];         /* Drunk, full, thirsty			*/
   int inherent_ac_apply;	/* Natural ac				*/
   sbyte age_add;		/* Age bonus in years			*/
   ubyte num_rerolls;		/* Number of rerolls			*/
   long	grants;			/* Grant bitvector			*/
   long classes;		/* Class bitvector for multiclassing	*/
   int num_classes;		/* Number of classes for multiclassing	*/
   int level[32];		/* Levels in each class for multiclassing */
   int extra_pracs;		/* Practices earned but not used on practicing */
   int old_hit;			/* Maxhit from previous class		*/
   int old_mana;		/* Maxmana from previous class		*/
   int old_move;		/* Maxmove from previous class		*/
   int new_hit;			/* Maxhit from current class		*/
   int new_mana;		/* Maxmana from current class		*/
   int new_move;		/* Maxmove from current class		*/
   int reroll_level;		/* Level char rerolled at, don't gain pracs till then or till multi */
   ubyte rerolling;		/* If character was rerolling (for crash recovery) */
   int spells_to_learn;		/* How many can you learn yet this level*/
   int olc_min1;		/* For olc ranges			*/
   int olc_max1;
   int olc_min2;
   int olc_max2;

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */
   ubyte page_length;		/* length of 1 screen in pager		*/
   ubyte spareb1;
   ubyte spareb2;
   ubyte spareb3;
   ubyte spareb4;
   ubyte spareb5;
   int qp;			/* quest points				*/
   int sparei1;
   int sparei2;
   int sparei3;
   int sparei4;
   int sparei5;
   long	sparel1;
   long	sparel2;
   long	sparel3;
   long	sparel4;
   long sparel5;
   long sparel6;
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   char *walkin;		/* Description for arival of char	*/
   char *walkout;		/* Description for departure of char	*/
   char	*poofin;		/* Description on arrival of a god.     */
   char	*poofout;		/* Description upon a god's exit.       */
   struct alias *aliases;	/* Character's aliases			*/
   long last_tell;		/* idnum of last tell from		*/
   long forward_tell;		/* idnum of char to forward tell to	*/
   struct player_arena_data arena; /* info for arena combat		*/
   int zone_locked;		/* what zone the char has locked	*/
   int olc_mode;		/* type of thing being editted		*/
   long olc_vnum;		/* vnum of thing being editted		*/
   int olc_field;		/* field being modified			*/
   void *olc_ptr;		/* pointer to thing being editted	*/
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   byte last_direction;     /* The last direction the monster went     */
   sh_int attack_type;      /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   memory_rec *memory;	    /* List of attackers to remember	       */
   byte damnodice;          /* The number of damage dice's	       */
   byte damsizedice;        /* The size of the damage dice's           */
   int wait_state;	    /* Wait state for bashed mobs	       */
   byte size;               /* Size as specified in OLC                */
   byte attacks;            /* Number of attacks every 2 rounds        */
   byte move_rate;          /* Movement rate (1(100 sec)-10(10 sec))   */
   byte ferocity;           /* Agro speed (1(100 sec)-10(10 sec))      */
   sh_int spec_proc;        /* Spec proc assigned to mob in OLC        */
   int immune;              /* immunity bitvector                      */
   int weak;                /* weakness bitvector                      */
   int resist;              /* resistance bitvector                    */
   int cmd;                 /* Zone command number that loaded this mob */
   bool from_load;          /* If mob was just loaded                  */
   MPROG_ACT_LIST *mpact;   /* For mob prog act triggers               */
   int mpactnum;            /*                                         */
   char *actions;           /* String of commands the mob will execute */
   int action_index;        /* Current location in actions string      */
};


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long long bitvector;  /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
struct char_data {
   int pfilepos;			 /* playerfile pos		  */
   sh_int nr;                            /* Mob's rnum			  */
   room_num in_room;                     /* Location (real room number)	  */
   room_num was_in_room;		 /* location for linkdead people  */

   struct char_player_data player;       /* Normal data                   */
   struct char_ability_data real_abils;	 /* Abilities without modifiers   */
   struct char_ability_data aff_abils;	 /* Abils with spells/stones/etc  */
   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;	/* PC/NPC specials	  */
   struct player_special_data *player_specials; /* PC specials		  */
   struct mob_special_data mob_specials;	/* NPC specials		  */

   struct affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   struct char_data *next_in_room;     /* For room->people - list         */
   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */

   struct follow_type *followers;        /* List of chars followers       */
   struct char_data *master;             /* Who is char following?        */

};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
   /* char_player_data */
   char	name[MAX_NAME_LENGTH+1];
   char	description[EXDSCR_LENGTH+1];
   char	title[MAX_TITLE_LENGTH+1];
   byte sex;
   byte class;
   byte level;
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   long	played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;

   char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];

   time_t last_logon;		/* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];	/* host of last logon */
   char walkin[MAX_WALK_LENGTH+1];
   char walkout[MAX_WALK_LENGTH+1];
   char poofin[MAX_WALK_LENGTH+1];
   char poofout[MAX_WALK_LENGTH+1];
   char prompt[MAX_PROMPT_LENGTH+1];
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char	*text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};


struct mail_list {
   long to_who;
   struct mail_list *next;
};


struct descriptor_data {
   socket_t	descriptor;	/* file descriptor for socket		*/
   char	host[HOST_LENGTH+1];	/* hostname				*/
   byte	bad_pws;		/* number of bad pw attemps this login	*/
   byte idle_tics;		/* tics idle at password prompt		*/
   int	connected;		/* mode of 'connectedness'		*/
   int	wait;			/* wait for how many loops		*/
   int	desc_num;		/* unique num assigned to desc		*/
   time_t login_time;		/* when the person connected		*/
   char *showstr_head;		/* for keeping track of an internal str	*/
   char **showstr_vector;	/* for paging through texts		*/
   int  showstr_count;		/* number of pages to page through	*/
   int  showstr_page;		/* which page are we currently showing?	*/
   char	**str;			/* for the modify-str system		*/
   size_t max_str;		/*		-			*/
   struct mail_list *mail_to;	/* name list for mail system		*/
   int	prompt_mode;		/* control of prompt-printing		*/
   char	inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
   char	last_input[MAX_INPUT_LENGTH]; /* the last input			*/
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
   char *output;		/* ptr to the current output buffer	*/
   int  bufptr;			/* ptr to end of current output		*/
   int	bufspace;		/* space left in the output buffer	*/
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;		/* q of unprocessed input		*/
   struct char_data *character;	/* linked to char			*/
   struct char_data *original;	/* original char if switched		*/
   struct descriptor_data *snooping; /* Who is this char snooping	*/
   struct descriptor_data *snoop_by; /* And who is snooping this char	*/
   struct descriptor_data *next; /* link to next descriptor		*/
};


/* other miscellaneous structures ***************************************/


struct msg_type {
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;	/* messages when death			*/
   struct msg_type miss_msg;	/* messages when miss			*/
   struct msg_type hit_msg;	/* messages when hit			*/
   struct msg_type god_msg;	/* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


struct weather_data {
   int	pressure;	/* How is the pressure ( Mb ) */
   int	change;		/* How fast and what way does it change. */
   int	sky;		/* How is the sky. */
   int	sunlight;	/* And how much sun. */
};


/* element in monster and object index-tables   */
struct index_data {
   int	virtual;    /* virtual number of this mob/obj           */
   int	number;     /* number of existing units of this mob/obj	*/
   int  progtypes;  /* program types for MOBProg                */
   MPROG_DATA *mobprogs;  /* programs for MOBProg               */
   SPECIAL(*func);
};
