/* ************************************************************************
*   File: db.h                                          Part of CircleMUD *
*  Usage: header file for database handling                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD	0
#define DB_BOOT_MOB	1
#define DB_BOOT_OBJ	2
#define DB_BOOT_ZON	3
#define DB_BOOT_SHP	4
#define DB_BOOT_HLP	5
#define DB_BOOT_PRG	6

/* names of various files and directories */
#define INDEX_FILE	"index"		/* index of world files		*/
#define MINDEX_FILE	"index.mini"	/* ... and for mini-mud-mode	*/
#define WLD_PREFIX	"world/wld"	/* room definitions		*/
#define MOB_PREFIX	"world/mob"	/* monster prototypes		*/
#define OBJ_PREFIX	"world/obj"	/* object prototypes		*/
#define ZON_PREFIX	"world/zon"	/* zon defs & command tables	*/
#define SHP_PREFIX	"world/shp"	/* shop definitions		*/
#define PRG_PREFIX	"world/prg"	/* Mob program			*/
#define HLP_PREFIX	"text/help"	/* for HELP <keyword>		*/

#define CREDITS_FILE	"text/credits"	/* for the 'credits' command	*/
#define NEWS_FILE	"text/news"	/* for the 'news' command	*/
#define MOTD_FILE	"text/motd"	/* messages of the day / mortal	*/
#define IMOTD_FILE	"text/imotd"	/* messages of the day / immort	*/
#define HELP_PAGE_FILE	"text/help/screen" /* for HELP <CR>		*/
#define INFO_FILE	"text/info"	/* for INFO			*/
#define WIZLIST_FILE	"text/wizlist"	/* for WIZLIST			*/
#define IMMLIST_FILE	"text/immlist"	/* for IMMLIST			*/
#define BACKGROUND_FILE	"text/background" /* for the background story	*/
#define POLICIES_FILE	"text/policies"	/* player policies/rules	*/
#define HANDBOOK_FILE	"text/handbook"	/* handbook for new immorts	*/
#define GREETINGS_FILE  "text/greetings" /* title screen		*/

#define IDEA_FILE	"misc/ideas"	/* for the 'idea'-command	*/
#define TYPO_FILE	"misc/typos"	/*         'typo'		*/
#define BUG_FILE	"misc/bugs"	/*         'bug'		*/
#define MESS_FILE	"misc/messages"	/* damage messages		*/
#define SOCMESS_FILE	"misc/socials"	/* messgs for social acts	*/
#define XNAME_FILE	"misc/xnames"	/* invalid name substrings	*/

#define PLAYER_FILE	"etc/players"	/* the player database		*/
#define MAIL_FILE	"etc/plrmail"	/* for the mudmail system	*/
#define BAN_FILE	"etc/badsites"	/* for the siteban system	*/


/* public procedures in db.c */
void	boot_db(void);
int	create_entry(char *name);
void	zone_update(void);
int	real_zone(int virtual);
int	real_room(int virtual);
char	*fread_string(FILE *fl, char *error);
long	get_id_by_name(char *name);
char	*get_name_by_id(long id);

void	char_to_store(struct char_data *ch, struct char_file_u *st);
void	store_to_char(struct char_file_u *st, struct char_data *ch);
int	load_char(char *name, struct char_file_u *char_element);
void	save_char(struct char_data *ch, sh_int load_room);
void	init_char(struct char_data *ch);
struct char_data* create_char(void);
struct char_data *read_mobile(int nr, int type);
int	real_mobile(int virtual);
int	vnum_mobile(char *searchname, struct char_data *ch, bool exact);
void	clear_char(struct char_data *ch);
void	reset_char(struct char_data *ch);
void	free_char(struct char_data *ch);

struct obj_data *create_obj(void);
void	clear_object(struct obj_data *obj);
void	free_obj(struct obj_data *obj);
int	real_object(int virtual);
struct obj_data *read_object(int nr, int type);
int	vnum_object(char *searchname, struct char_data *ch, bool exact);
int	vnum_room(char *searchname, struct char_data *ch, bool exact);
int	vnum_zone(char *searchname, struct char_data *ch, bool exact);

#define REAL 0
#define VIRTUAL 1

/* structure for the reset commands */
struct reset_com {
   char	command;	/* current command                      */

   int	if_flag;	/* if >0, exe if depend cmd exe
                           if <0, exe if depend cmd didn't exe  */
   int	arg1;		/*                                      */
   int	arg2;		/* Arguments to the command             */
   int	arg3;		/*                                      */
   int  arg4;		/*					*/
   int  limit;		/* Original copy of arg2 (which changes)*/
   int  i;		/* Counter for mob limits to save cpu useage */
   int	line;		/* line number this command appears on  */
   int	prob;		/* probability (in 10000) of command happening */
   int	depend;		/* command this command depends on if if_flag set */
   int  ok;             /*
                         * 1 if command exe'd ok
                         * 0 if command didn't exe because of itself
                         * -1 if command didn't exe because of a previous command
                         */
                        /*
                         * arg2 is the limiting number
                         * for mobs, it's the number of copies
                         * that can exist in the game at once
                         * from this specific command
                         * for objs, it's the number of resets
                         * the item will try to load for
                         */

   /* 
	*  Commands:			*
	*  'M': Read a mobile		*
	*  'O': Read an object		*
	*  'P': Put obj in obj		*
	*  'G': Obj to char		*
	*  'E': Obj to char equip	*
	*  'R': Remove obj from room	*
	*  'D': Set state of door	*
   */
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data {
   char	*name;		    /* name of this zone                  */
   int	lifespan;           /* how long between resets (minutes)  */
   int	age;                /* current age of this zone (minutes) */
   int	top;                /* upper limit for rooms in this zone */
   int	bottom;             /* lower limit for rooms in this zone */

   int	reset_mode;         /* conditions for reset (see below)   */
   int	number;		    /* virtual number of this zone	  */
   struct reset_com *cmd;   /* command table for reset	          */
   int  closed;             /* if zone is closed                  */
   char locked_by[40];      /* who has the zone locked            */

   /*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
   */
};



/* for queueing zones for update   */
struct reset_q_element {
   int	zone_to_reset;            /* ref to zone_data */
   struct reset_q_element *next;
};



/* structure for the update queue     */
struct reset_q_type {
   struct reset_q_element *head;
   struct reset_q_element *tail;
};



struct player_index_element {
   char	*name;
   long id;
};


struct help_index_element {
   char	*keyword;
   char *entry;
   int duplicate;
};


/* don't change these */
#define BAN_NOT 	0
#define BAN_NEW 	1
#define BAN_SELECT	2
#define BAN_ALL		3

#define BANNED_SITE_LENGTH    50
struct ban_list_element {
   char	site[BANNED_SITE_LENGTH+1];
   int	type;
   time_t date;
   char	name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next;
};


/* Structure for items to be updated in rent and reimb files */
struct update_list {
  int nr;
  struct update_list *next;
};


/* global buffering system */

#ifdef __DB_C__
char	buf[MAX_STRING_LENGTH];
char	buf1[MAX_STRING_LENGTH];
char	buf2[MAX_STRING_LENGTH];
char	arg[MAX_STRING_LENGTH];
char    err_buf[MAX_STRING_LENGTH];
#else
extern char	buf[MAX_STRING_LENGTH];
extern char	buf1[MAX_STRING_LENGTH];
extern char	buf2[MAX_STRING_LENGTH];
extern char	arg[MAX_STRING_LENGTH];
extern char     err_buf[MAX_STRING_LENGTH];
#endif

#ifndef __CONFIG_C__
extern char	*OK;
extern char	*NOPERSON;
extern char	*NOEFFECT;
#endif

#define bug(x, y) {sprintf(err_buf, (x), (y)); log(err_buf); }
#define bug2(x) {sprintf(err_buf, (x)); log(err_buf); }