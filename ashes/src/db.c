/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

#define __DB_C__

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world = NULL;	/* array of rooms		 */
int top_of_world = 0;		/* ref to top element of world	 */

struct char_data *character_list = NULL;	/* global linked list of
						 * chars	 */
struct index_data *mob_index;	/* index table for mobile file	 */
struct char_data *mob_proto;	/* prototypes for mobs		 */
int top_of_mobt = 0;		/* top of mobile index table	 */

struct obj_data *object_list = NULL;	/* global linked list of objs	 */
struct index_data *obj_index;	/* index table for object file	 */
struct obj_data *obj_proto;	/* prototypes for objs		 */
int top_of_objt = 0;		/* top of object index table	 */

struct zone_data *zone_table;	/* zone table			 */
int top_of_zone_table = 0;	/* top element of zone tab	 */
struct message_list fight_messages[MAX_MESSAGES];	/* fighting messages	 */

struct player_index_element *player_table = NULL;	/* index to plr file	 */
FILE *player_fl = NULL;		/* file desc of player file	 */
int top_of_p_table = 0;		/* ref to top of table		 */
int top_of_p_file = 0;		/* ref of size of p file	 */
long top_idnum = 0;		/* highest idnum in use		 */

int no_mail = 0;		/* mail disabled?		 */
int mini_mud = 0;		/* mini-mud mode?		 */
int no_rent_check = 0;		/* skip rent check on boot?	 */
time_t boot_time = 0;		/* time of mud boot		 */
int restrict_mud = 0;		/* level of game restriction	 */
int zlock_reset = 0;		/* ignore load probs?		 */
sh_int r_mortal_start_room;	/* rnum of mortal start room	 */
sh_int r_immort_start_room;	/* rnum of immort start room	 */
sh_int r_frozen_start_room;	/* rnum of frozen start room	 */

char *credits = NULL;		/* game credits			 */
char *news = NULL;		/* mud news			 */
char *motd = NULL;		/* message of the day - mortals */
char *imotd = NULL;		/* message of the day - immorts */
char *help = NULL;		/* help screen			 */
char *info = NULL;		/* info page			 */
char *wizlist = NULL;		/* list of higher gods		 */
char *immlist = NULL;		/* list of peon gods		 */
char *background = NULL;	/* background story		 */
char *handbook = NULL;		/* handbook for new immortals	 */
char *policies = NULL;		/* policies page		 */
char *greetings = NULL;		/* greetings page		 */

struct help_index_element *help_table = 0;	/* the help table	 */
int top_of_helpt = 0;		/* top of help index table	 */

struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;	/* the infomation about the weather */
struct reset_q_type reset_q;	/* queue of zones to be reset	 */
struct player_special_data dummy_mob;	/* dummy spec area for mobs	 */

struct update_list *updates=NULL;

long item_id;

int arena_restrict;

extern int MOBSay;

/* local functions */
void init_dummy_player(struct player_special_data *dummy);
void class_init_exp_tables(void);
MPROG_DATA* mprog_file_read(FILE *progfile, MPROG_DATA *mprg, struct index_data *pMobIndex);
void setup_dir(FILE * fl, int room, int dir);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode, char *fname);
void parse_room(FILE * fl, int virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
char *parse_object(FILE * obj_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_help(FILE *fl);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
void char_to_store(struct char_data * ch, struct char_file_u * st);
void store_to_char(struct char_file_u * st, struct char_data * ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
int file_to_string_alloc(char *name, char **buf);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(int zone, int cmd_no, char *message);
void reset_time(void);
void clear_char(struct char_data * ch);

/* external functions */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void weather_and_time(int mode);
void mag_assign_spells(void);
void boot_social_messages(void);
void update_obj_file(void);	/* In objsave.c */
void obj_dupe_check(void);	/* In objsave.c */
void init_vcs(void);
void sort_commands(void);
void sort_spells(void);
void load_banned(void);
void Read_Invalid_List(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
int hsort(const void *a, const void *b);
SPECIAL(damage_eq);

/* external vars */
extern int no_specials;

#define READ_SIZE 256

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* this is necessary for the autowiz system */
void reboot_wizlists(void)
{
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
}


ACMD(do_reboot)
{
  int i;

  one_argument(argument, arg);

  if (!str_cmp(arg, "all") || *arg == '*') {
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
    file_to_string_alloc(NEWS_FILE, &news);
    file_to_string_alloc(CREDITS_FILE, &credits);
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(IMOTD_FILE, &imotd);
    file_to_string_alloc(HELP_PAGE_FILE, &help);
    file_to_string_alloc(INFO_FILE, &info);
    file_to_string_alloc(POLICIES_FILE, &policies);
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
    file_to_string_alloc(BACKGROUND_FILE, &background);
    file_to_string_alloc(GREETINGS_FILE, &greetings);
  } else if (!str_cmp(arg, "wizlist"))
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
  else if (!str_cmp(arg, "immlist"))
    file_to_string_alloc(IMMLIST_FILE, &immlist);
  else if (!str_cmp(arg, "news"))
    file_to_string_alloc(NEWS_FILE, &news);
  else if (!str_cmp(arg, "credits"))
    file_to_string_alloc(CREDITS_FILE, &credits);
  else if (!str_cmp(arg, "motd"))
    file_to_string_alloc(MOTD_FILE, &motd);
  else if (!str_cmp(arg, "imotd"))
    file_to_string_alloc(IMOTD_FILE, &imotd);
  else if (!str_cmp(arg, "help"))
    file_to_string_alloc(HELP_PAGE_FILE, &help);
  else if (!str_cmp(arg, "info"))
    file_to_string_alloc(INFO_FILE, &info);
  else if (!str_cmp(arg, "policy"))
    file_to_string_alloc(POLICIES_FILE, &policies);
  else if (!str_cmp(arg, "handbook"))
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
  else if (!str_cmp(arg, "background"))
    file_to_string_alloc(BACKGROUND_FILE, &background);
  else if (!str_cmp(arg, "greeting"))
    file_to_string_alloc(GREETINGS_FILE, &greetings);
  else if (!str_cmp(arg, "xhelp")) {
    if (help_table) {
      for (i = 0; i <= top_of_helpt; i++) {
        if (help_table[i].keyword)
	  free(help_table[i].keyword);
        if (help_table[i].entry && !help_table[i].duplicate)
	  free(help_table[i].entry);
      }
      free(help_table);
    }
    top_of_helpt = 0;
    index_boot(DB_BOOT_HLP);
  } else {
    send_to_char("Unknown reload option.\r\n", ch);
    return;
  }

  send_to_char(OK, ch);
}


void boot_world(void)
{
  log("Loading zone table.");
  index_boot(DB_BOOT_ZON);

  log("Loading rooms.");
  index_boot(DB_BOOT_WLD);

  log("Renumbering rooms.");
  renum_world();

  log("Checking start rooms.");
  check_start_rooms();

  log("Loading mobs and generating index.");
  init_dummy_player(&dummy_mob);
  index_boot(DB_BOOT_MOB);

  log("Loading mobprogs.");
  index_boot(DB_BOOT_PRG);

  log("Loading objs and generating index.");
  index_boot(DB_BOOT_OBJ);

  log("Renumbering zone table.");
  renum_zone_table();

  if (!no_specials) {
    log("Loading shops.");
    index_boot(DB_BOOT_SHP);
  }
}


void check_arena(void)
{
  int i;

  arena_restrict=ARENA_OPEN;
  if(real_zone(ARENA_ZONE) < 0) {
    log("Non-existant arena zone: arena disabled.");
    arena_restrict=ARENA_CLOSED;
  }
  else {
    for(i=0; i<30; i++) {
      if(real_room(zone_table[real_zone(ARENA_ZONE)].bottom+i)<0) {
        if(i<10) {
          log("Single-combat room missing: arena disabled.");
          arena_restrict=ARENA_CLOSED;
          break;
        }
        else if(i<20) {
          log("Group-combat room missing: group challenge disabled.");
          arena_restrict=ARENA_NOGROUP;
          break;
        }
        else {
          log("All-combat room missing: challenge all disabled.");
          arena_restrict=ARENA_NOALL;
          break;
        }
      }
    }
  }
}


void read_update(void)
{
  char buf[80], *ptr, *ptr2;
  FILE *fp;
  struct update_list *newu;

  if((fp=fopen("update", "rb"))) {
    while(!feof(fp)) {
      fgets(buf, 80, fp);
      if(!feof(fp)) {
        ptr=buf;
        skip_spaces(&ptr);
        if((*ptr=='$') || (*ptr=='S'))
          break;
        for(ptr2=ptr; isdigit(*ptr2); ptr2++);
        (*ptr2)=0;
        if(*ptr) {
          CREATE(newu, struct update_list, 1);
          newu->nr=atoi(ptr);
          newu->next=updates;
          updates=newu;
        }
      }
    }
    fclose(fp);
    unlink("update");
  }
}
  

/* body of the booting system */
void boot_db(void)
{
  int i;

  log("Boot db -- BEGIN.");

  log("Resetting the game time:");
  reset_time();

  log("Initializing character classes.");
  class_init_exp_tables();

  log("Reading greetings, news, credits, help, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);
  file_to_string_alloc(GREETINGS_FILE, &greetings);


  boot_world();

  log("Loading help entries.");
  index_boot(DB_BOOT_HLP);

  log("Generating player index.");
  build_player_index();

  log("Loading fight messages.");
  load_messages();

  log("Loading social messages.");
  boot_social_messages();

  log("Assigning function pointers:");

  if (!no_specials) {
    log("   Mobiles.");
    assign_mobiles();

    log("   Shopkeepers.");
    assign_the_shopkeepers();

    log("   Objects.");
    assign_objects();

    log("   Rooms.");
    assign_rooms();
  }

  log("   Spells.");
  mag_assign_spells();

  log("Assigning spell and skill levels.");
  init_spell_levels();

  log("Sorting command list and spells.");
  sort_commands();
  sort_spells();

  log("Booting mail system.");
  if (!scan_file()) {
    log("    Mail boot failed -- Mail system disabled");
    no_mail = 1;
  }

  log("Reading banned site and invalid-name list.");
  load_banned();
  Read_Invalid_List();

  log("Initializing virtual channels.");
  init_vcs();

  if (!no_rent_check) {
    log("Deleting timed-out rent files:");
    update_obj_file();
    log("Done.");

    log("Reading object update list.");
    read_update();

    log("Scanning for duped items and dupe-marking reimb files.");
    obj_dupe_check();
  }

  log("Checking arena.");
  check_arena();

  for (i = 0; i <= top_of_zone_table; i++) {
    sprintf(buf2, "Resetting %s (rooms %d-%d).",
	    zone_table[i].name, zone_table[i].bottom,
	    zone_table[i].top);
    log(buf2);
    reset_zone(i);
  }

  reset_q.head = reset_q.tail = NULL;

  MOBTrigger = TRUE;
  MOBSay = TRUE;

  boot_time = time(0);

  log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
  long beginning_of_time = 650336715;
  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);

  if (time_info.hours <= 4)
    weather_info.sunlight = SUN_DARK;
  else if (time_info.hours == 5)
    weather_info.sunlight = SUN_RISE;
  else if (time_info.hours <= 20)
    weather_info.sunlight = SUN_LIGHT;
  else if (time_info.hours == 21)
    weather_info.sunlight = SUN_SET;
  else
    weather_info.sunlight = SUN_DARK;

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
	  time_info.day, time_info.month, time_info.year);
  log(buf);

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12))
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.pressure <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.pressure <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;
}

void init_dummy_player(struct player_special_data *dummy)
{
   int i;

   memset((char *) dummy, 0, sizeof(struct player_special_data));
   dummy->walkin = str_dup("enters");
   dummy->walkout = str_dup("leaves");
   dummy->poofin = str_dup("appears out of thin air.");
   dummy->poofout = str_dup("vanishes into thin air.");
   dummy->aliases = NULL;
   dummy->last_tell = -1;
   dummy->forward_tell = -1;
   dummy->zone_locked = -1;
   for(i=1; i<=MAX_SKILLS; i++) {
      dummy->saved.skills[i]=85;
   }
   for(i=0; i<MAX_REMEMBER; i++) {
      dummy->saved.remember[i]=NOWHERE;
   }
   dummy->saved.conditions[0]=-1;
   dummy->saved.conditions[1]=-1;
   dummy->saved.conditions[2]=-1;
   dummy->saved.olc_min1=dummy->saved.olc_min2=0;
   dummy->saved.olc_max1=dummy->saved.olc_max2=0;
   dummy->saved.num_classes = 1;
}

/* generate index table for the player file */
void build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
    if (errno != ENOENT) {
      perror("fatal error opening playerfile");
      exit(1);
    } else {
      log("No playerfile.  Creating a new one.");
      touch(PLAYER_FILE);
      if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
	perror("fatal error opening playerfile");
	exit(1);
      }
    }
  }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    sprintf(buf, "   %ld players in database.", recs);
    log(buf);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl)) {	/* new record */
      nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
      player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
    }
  }

  top_of_p_file = top_of_p_table = nr;
}



/* function to count how many hash-mark delimited records exist in a file */
int count_hash_records(FILE * fl)
{
  char buf[128];
  int count = 0;

  while (fgets(buf, 128, fl))
    if (*buf == '#')
      count++;

  return count;
}



void index_boot(int mode)
{
  char *index_filename, *prefix;
  FILE *index, *db_file;
  int rec_count = 0;

  switch (mode) {
  case DB_BOOT_WLD:
    prefix = WLD_PREFIX;
    break;
  case DB_BOOT_MOB:
    prefix = MOB_PREFIX;
    break;
  case DB_BOOT_OBJ:
    prefix = OBJ_PREFIX;
    break;
  case DB_BOOT_ZON:
    prefix = ZON_PREFIX;
    break;
  case DB_BOOT_SHP:
    prefix = SHP_PREFIX;
    break;
  case DB_BOOT_HLP:
    prefix = HLP_PREFIX;
    break;
  case DB_BOOT_PRG:
    prefix = PRG_PREFIX;
    break;
  default:
    log("SYSERR: Unknown subcommand to index_boot!");
    exit(1);
    break;
  }

  if (mini_mud)
    index_filename = MINDEX_FILE;
  else
    index_filename = INDEX_FILE;

  sprintf(buf2, "%s/%s", prefix, index_filename);

  if (!(index = fopen(buf2, "r"))) {
    sprintf(buf1, "Error opening index file '%s'", buf2);
    perror(buf1);
    exit(1);
  }

  /* first, count the number of records in the file so we can malloc */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      log("file listed in index not found");
      exit(1);
    } else {
      if (mode == DB_BOOT_ZON)
	rec_count++;
      else
	rec_count += count_hash_records(db_file);
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* Exit if 0 records, unless this is shops or progs */
  if (!rec_count) {
    if ((mode == DB_BOOT_SHP) || (mode == DB_BOOT_PRG)) {
      fclose(index);
      return;
    }
    log("SYSERR: boot error - 0 records counted");
    exit(1);
  }

  rec_count++;

  switch (mode) {
  case DB_BOOT_WLD:
    CREATE(world, struct room_data, rec_count);
    break;
  case DB_BOOT_MOB:
    CREATE(mob_proto, struct char_data, rec_count);
    CREATE(mob_index, struct index_data, rec_count);
    break;
  case DB_BOOT_OBJ:
    CREATE(obj_proto, struct obj_data, rec_count);
    CREATE(obj_index, struct index_data, rec_count);
    break;
  case DB_BOOT_ZON:
    CREATE(zone_table, struct zone_data, rec_count);
    break;
  case DB_BOOT_HLP:
    CREATE(help_table, struct help_index_element, rec_count * 2);
    break;
  }


  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      exit(1);
    }
    switch (mode) {
    case DB_BOOT_WLD:
    case DB_BOOT_OBJ:
    case DB_BOOT_MOB:
    case DB_BOOT_PRG:
      discrete_load(db_file, mode, buf2);
      break;
    case DB_BOOT_ZON:
      load_zones(db_file, buf2);
      break;
    case DB_BOOT_HLP:
      load_help(db_file);
      break;
    case DB_BOOT_SHP:
      boot_the_shops(db_file, buf2, rec_count);
      break;
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* sort the help index */
  if (mode == DB_BOOT_HLP) {
    qsort(help_table, top_of_helpt, sizeof(struct help_index_element), hsort);
    top_of_helpt--;
  }
  fclose(index);

}


void discrete_load(FILE * fl, int mode, char *fname)
{
  int nr = -1, last = 0, i;
  char line[256], prog_buf[MAX_INPUT_LENGTH];
  MPROG_DATA *mprg;

  char *modes[] = {"world", "mob", "obj", "", "", "", "prg"};

  for (;;) {
    /*
     * we have to do special processing with the obj files because they have
     * no end-of-record marker :(
     */
    if (mode != DB_BOOT_OBJ || nr < 0)
      if (!get_line(fl, line)) {
	fprintf(stderr, "Format error after %s #%d (%s)\n", modes[mode], nr, fname);
	exit(1);
      }
    if (*line == '$')
      return;

    if (*line == '#') {
      last = nr;
      if (sscanf(line, "#%d", &nr) != 1) {
	fprintf(stderr, "Format error after %s #%d (%s)\n", modes[mode], last, fname);
	exit(1);
      }
      if (nr >= 99999)
	return;
      else
	switch (mode) {
	case DB_BOOT_WLD:
	  parse_room(fl, nr);
	  break;
	case DB_BOOT_MOB:
	  parse_mobile(fl, nr);
	  break;
	case DB_BOOT_OBJ:
	  strcpy(line, parse_object(fl, nr));
	  break;
        case DB_BOOT_PRG:
          if((i=real_mobile(nr)) < 0) {
            sprintf(buf, "SYSERR: Mob prog attached to non-existant mob #%d, prog skipped", nr);
            log(prog_buf);
            while(*prog_buf != '|')
              fgets(prog_buf, MAX_INPUT_LENGTH, fl);
          }
          else {
            CREATE(mob_index[i].mobprogs, MPROG_DATA, 1);
            mprg=mob_index[i].mobprogs;
            mprg=mprog_file_read(fl, mprg, mob_index + i);
            mprg->next=NULL;
          }
          break;
	}
    } else {
      fprintf(stderr, "Format error in %s file near %s #%d (%s)\n",
	      modes[mode], modes[mode], nr, fname);
      fprintf(stderr, "Offending line: '%s'\n", line);
      exit(1);
    }
  }
}


long long asciiflag_conv(char *flag)
{
  long long flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++) {
    if (islower(*p))
      flags |= 1 << (*p - 'a');
    else if (isupper(*p))
      flags |= 1 << (26 + (*p - 'A'));

    if (!isdigit(*p))
      is_number = 0;
  }

  if (is_number)
    flags = atoll(flag);

  return flags;
}


/* load the rooms */
void parse_room(FILE * fl, int virtual_nr)
{
  static int room_nr = 0, zone = 0;
  int t[10], i;
  char line[256], flags[128];
  struct extra_descr_data *new_descr;

  sprintf(buf2, "room #%d", virtual_nr);

  while (virtual_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", virtual_nr);
      exit(1);
    }
  if (virtual_nr < zone_table[zone].bottom) {
    fprintf(stderr, "Room #%d is below zone %d.\n", virtual_nr, zone);
    exit(1);
  }
  world[room_nr].zone = zone;
  world[room_nr].number = virtual_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);

  if (!get_line(fl, line) || sscanf(line, " %d %s %d ", t, flags, t + 2) != 3) {
    fprintf(stderr, "Format error in room #%d\n", virtual_nr);
    exit(1);
  }
  /* t[0] is the zone number; ignored with the zone-file system */
  world[room_nr].room_flags = asciiflag_conv(flags);
  world[room_nr].sector_type = t[2];

  world[room_nr].func = NULL;
  world[room_nr].contents = NULL;
  world[room_nr].people = NULL;
  world[room_nr].light = 0;	/* Zero light sources */

  if (!get_line(fl, line) || sscanf(line, " %dd%d+%d %d %d", t, t + 1, t + 2,
      t + 3, t + 4) != 5) {
    fprintf(stderr, "Format error in room #%d\n", virtual_nr);
    exit(1);
  }
  world[room_nr].dt_numdice=t[0];
  world[room_nr].dt_sizedice=t[1];
  world[room_nr].dt_add=t[2];
  world[room_nr].dt_percent=t[3];
  world[room_nr].dt_repeat=t[4];

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf, "Format error in room #%d (expecting D/E/S)", virtual_nr);

  for (;;) {
    if (!get_line(fl, line)) {
      fprintf(stderr, "%s\n", buf);
      exit(1);
    }
    switch (*line) {
    case 'D':
      setup_dir(fl, room_nr, atoi(line + 1));
      break;
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(fl, buf2);
      new_descr->description = fread_string(fl, buf2);
      new_descr->next = world[room_nr].ex_description;
      world[room_nr].ex_description = new_descr;
      break;
    case 'S':			/* end of room */
      top_of_world = room_nr++;
      return;
      break;
    default:
      fprintf(stderr, "%s\n", buf);
      exit(1);
      break;
    }
  }
}



/* read direction data */
void setup_dir(FILE * fl, int room, int dir)
{
  int t[5];
  char line[256];

  sprintf(buf2, "room #%d, direction D%d", world[room].number, dir);

  CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
  world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
  world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    fprintf(stderr, "Format error, %s\n", buf2);
    exit(1);
  }
  if (sscanf(line, " %d %d %d ", t, t + 1, t + 2) != 3) {
    fprintf(stderr, "Format error, %s\n", buf2);
    exit(1);
  }
  if (t[0] == 1)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
  else if (t[0] == 2)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else if (t[0] == 3)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET;
  else if (t[0] == 4)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_SECRET;
  else if (t[0] == 5)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET | EX_HIDDEN;
  else if (t[0] == 6)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_SECRET | EX_HIDDEN;
  else
    world[room].dir_option[dir]->exit_info = 0;

  world[room].dir_option[dir]->key = t[1];
  world[room].dir_option[dir]->to_room = t[2];
}


/* make sure the start rooms exist & resolve their vnums to rnums */
void check_start_rooms(void)
{
  extern sh_int mortal_start_room;
  extern sh_int immort_start_room;
  extern sh_int frozen_start_room;

  if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
    log("SYSERR:  Mortal start room does not exist.  Change in config.c.");
    exit(1);
  }
  if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
    r_immort_start_room = r_mortal_start_room;
  }
  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
    r_frozen_start_room = r_mortal_start_room;
  }
}


/* resolve all vnums into rnums in the world */
void renum_world(void)
{
  register int room, door;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
	if (world[room].dir_option[door]->to_room != NOWHERE)
	  world[room].dir_option[door]->to_room =
	    real_room(world[room].dir_option[door]->to_room);
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table(void)
{
  int zone, cmd_no, a, b;

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      a = b = 0;
      switch (ZCMD.command) {
      case 'M':
	a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
	b = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'O':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (ZCMD.arg3 != NOWHERE)
	  b = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'G':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
        b = ZCMD.arg3 = real_mobile(ZCMD.arg3);
	break;
      case 'E':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
        b = ZCMD.arg3 = real_mobile(ZCMD.arg3);
	break;
      case 'P':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	b = ZCMD.arg3 = real_object(ZCMD.arg3);
	break;
      case 'D':
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	break;
      case 'R': /* rem obj from room */
        a = ZCMD.arg1 = real_room(ZCMD.arg1);
	b = ZCMD.arg2 = real_object(ZCMD.arg2);
        break;
      }
      if (a < 0 || b < 0) {
	if (!mini_mud)
	  log_zone_error(zone, cmd_no, "Invalid vnum, cmd disabled");
	ZCMD.command = '*';
      }
      else {
        if((ZCMD.command=='O')||(ZCMD.command=='P')||
           (ZCMD.command=='G')||(ZCMD.command=='E')) {
          if(GET_OBJ_TYPE(&obj_proto[ZCMD.arg1])==ITEM_KEY) {
/* don't change prob because of if/else, but still no limit
            ZCMD.prob=10000; */
            ZCMD.arg2=-1;
          }
        }
      }
    }
}



void parse_simple_mob(FILE *mob_f, int i, int nr)
{
  int j, t[10];
  long tmp1, tmp2;
  char line[256];

    mob_proto[i].real_abils.str = 13;
    mob_proto[i].real_abils.str_add = 0;
    mob_proto[i].real_abils.intel = 13;
    mob_proto[i].real_abils.wis = 13;
    mob_proto[i].real_abils.dex = 13;
    mob_proto[i].real_abils.con = 13;
    mob_proto[i].real_abils.cha = 13;

    get_line(mob_f, line);
    if (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
	  t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9) {
      fprintf(stderr, "Format error in mob #%d, first line after S flag\n"
	      "...expecting line of form '# # # #d#+# #d#+#'\n", nr);
      exit(1);
    }
    GET_LEVEL(mob_proto + i) = t[0];
    mob_proto[i].points.hitroll = t[1];
    mob_proto[i].points.armor = 10 * t[2];

    /* max hit = 0 is a flag that H, M, V is xdy+z */
    mob_proto[i].points.max_hit = 0;
    mob_proto[i].points.hit = t[3];
    mob_proto[i].points.mana = t[4];
    mob_proto[i].points.move = t[5];

    mob_proto[i].points.max_mana = 10;
    mob_proto[i].points.max_move = 50;

    mob_proto[i].mob_specials.damnodice = t[6];
    mob_proto[i].mob_specials.damsizedice = t[7];
    mob_proto[i].points.damroll = t[8];

    get_line(mob_f, line);
    sscanf(line, " %ld %ld ", &tmp1, &tmp2);
    GET_GOLD(mob_proto + i) = tmp1;
    GET_EXP(mob_proto + i) = tmp2;

    get_line(mob_f, line);
    if (sscanf(line, " %d %d %d ", t, t + 1, t + 2) != 3) {
      fprintf(stderr, "Format error in mob #%d, second line after S flag\n"
	      "...expecting line of form '# # #'\n", nr);
    }

    mob_proto[i].char_specials.position = t[0];
    mob_proto[i].mob_specials.default_pos = t[1];
    if(t[1]==POS_FIGHTING) {
      char fightbuf[80];
      sprintf(fightbuf, "mob %d: default pos fighting", nr);
      log(fightbuf);
    }
    mob_proto[i].player.sex = t[2];

    mob_proto[i].player.class = 0;
    mob_proto[i].player.weight = 180;
    mob_proto[i].player.height = 198;
    mob_proto[i].mob_specials.attacks = 2;

    for (j = 0; j < 3; j++)
      GET_COND(mob_proto + i, j) = -1;

    /*
     * these are saves, not applies
     */
    for (j = 0; j < 5; j++)
      GET_SAVE(mob_proto + i, j) = MAX(100-GET_LEVEL(mob_proto+i), 1);
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void interpret_espec(char *keyword, char *value, int i, int nr)
{
  int num_arg, matched = 0;

  num_arg = atoi(value);

  CASE("BareHandAttack") {
    RANGE(0, 99);
    mob_proto[i].mob_specials.attack_type = num_arg;
  }

  CASE("Str") {
    RANGE(3, 25);
    mob_proto[i].real_abils.str = num_arg;
  }

  CASE("StrAdd") {
    RANGE(0, 100);
    mob_proto[i].real_abils.str_add = num_arg;    
  }

  CASE("Int") {
    RANGE(3, 25);
    mob_proto[i].real_abils.intel = num_arg;
  }

  CASE("Wis") {
    RANGE(3, 25);
    mob_proto[i].real_abils.wis = num_arg;
  }

  CASE("Dex") {
    RANGE(3, 25);
    mob_proto[i].real_abils.dex = num_arg;
  }

  CASE("Con") {
    RANGE(3, 25);
    mob_proto[i].real_abils.con = num_arg;
  }

  CASE("Cha") {
    RANGE(3, 25);
    mob_proto[i].real_abils.cha = num_arg;
  }

  if (!matched) {
    fprintf(stderr, "Warning: unrecognized espec keyword %s in mob #%d\n",
	    keyword, nr);
  }    
}

#undef CASE
#undef RANGE

void parse_espec(char *buf, int i, int nr)
{
  char *ptr;

  if ((ptr = strchr(buf, ':')) != NULL) {
    *(ptr++) = '\0';
    while (isspace(*ptr))
      ptr++;
  } else
    ptr = "";

  interpret_espec(buf, ptr, i, nr);
}


void parse_enhanced_mob(FILE *mob_f, int i, int nr)
{
  char line[256];

  parse_simple_mob(mob_f, i, nr);

  while (get_line(mob_f, line)) {
    if (!strcmp(line, "E"))	/* end of the ehanced section */
      return;
    else if (*line == '#') {	/* we've hit the next mob, maybe? */
      fprintf(stderr, "Unterminated E section in mob #%d\n", nr);
      exit(1);
    } else
      parse_espec(line, i, nr);
  }

  fprintf(stderr, "Unexpected end of file reached after mob #%d\n", nr);
  exit(1);
}


void parse_ashes_mob(FILE *mob_f, int i, int nr, int ver)
{
  long t[8];
  char line[256];
  char f1[256], f2[256], f3[256];
  extern int(*spec_proc_table[MAX_SPECPROCS])(struct char_data *, void *, int, char *);

  parse_simple_mob(mob_f, i, nr);

  get_line(mob_f, line);
  if (sscanf(line, " %ld %ld %ld %ld %ld %ld %ld ",
             t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6) != 7) {
    fprintf(stderr, "Format error in mob #%d, first line after W flag\n"
                    "...expecting line of form '# # # # # # #'\n", nr);
    exit(1);
  }
  mob_proto[i].real_abils.str = t[0];
  mob_proto[i].real_abils.str_add = t[1];
  mob_proto[i].real_abils.intel = t[2];
  mob_proto[i].real_abils.wis = t[3];
  mob_proto[i].real_abils.dex = t[4];
  mob_proto[i].real_abils.con = t[5];
  mob_proto[i].real_abils.cha = t[6];

  get_line(mob_f, line);
  if (sscanf(line, " %ld %ld %ld ", t, t + 1, t + 2) != 3) {
    fprintf(stderr, "Format error in mob #%d, second line after W flag\n"
                    "...expecting line of form '# # #'\n", nr);
    exit(1);
  }
  mob_proto[i].mob_specials.size=t[1];
  mob_proto[i].player.weight=60*(t[1]+1);
  GET_CLASS(mob_proto+i)=t[2];
  if((t[0] > 0) && (t[0] < MAX_SPECPROCS)) {
    mob_proto[i].mob_specials.spec_proc=t[0];
    mob_index[i].func=spec_proc_table[t[0]];
    SET_BIT(MOB_FLAGS(mob_proto+i), MOB_SPEC);
  }
  else
    mob_proto[i].mob_specials.spec_proc=0;

  get_line(mob_f, line);
  if(ver > 2) {
    if (sscanf(line, " %ld %ld %ld %ld %ld %ld ", t, t + 1, t + 2, t + 3, t + 4, t + 5) != 6) {
      fprintf(stderr, "Format error in mob #%d, third line after W flag\n"
                      "...expecting line of form '# # # # # #'\n", nr);
      exit(1);
    }
  }
  else {
    if (sscanf(line, " %ld %ld %ld %ld %ld ", t, t + 1, t + 2, t + 3, t + 4) != 5) {
      fprintf(stderr, "Format error in mob #%d, third line after W flag\n"
                      "...expecting line of form '# # # # #'\n", nr);
      exit(1);
    }
  }
  mob_proto[i].points.armor = t[0];
  mob_proto[i].mob_specials.attack_type = t[1];
  mob_proto[i].mob_specials.attacks = t[2];
  mob_proto[i].char_specials.hp_regen_add = t[3];
  GET_MR(mob_proto+i)=t[4];
  if(ver > 2)
    GET_PR(mob_proto+i)=t[5];
  else
    GET_PR(mob_proto+i)=0;

  get_line(mob_f, line);
  if (sscanf(line, " %ld %ld %ld %ld %ld ", t, t + 1, t + 2, t + 3, t + 4) != 5) {
    fprintf(stderr, "Format error in mob #%d, fourth line after W flag\n"
                    "...expecting line of form '# # # # #'\n", nr);
    exit(1);
  }
  GET_SAVE(mob_proto + i, 0) = t[0];
  GET_SAVE(mob_proto + i, 1) = t[1];
  GET_SAVE(mob_proto + i, 2) = t[2];
  GET_SAVE(mob_proto + i, 3) = t[3];
  GET_SAVE(mob_proto + i, 4) = t[4];

  get_line(mob_f, line);
  if (sscanf(line, " %s %s %s ", f1, f2, f3) != 3) {
    fprintf(stderr, "Format error in mob #%d, fifth line after W flag\n"
                    "...expecting line of form '# # #'\n", nr);
    exit(1);
  }
  IMMUNE_FLAGS(mob_proto + i) = asciiflag_conv(f1);
  WEAK_FLAGS(mob_proto + i) = asciiflag_conv(f2);
  RESIST_FLAGS(mob_proto + i) = asciiflag_conv(f3);

  get_line(mob_f, line);
  if (sscanf(line, " %ld %ld ", t, t + 1) != 2) {
    fprintf(stderr, "Format error in mob #%d, sixth line after W flag\n"
                    "...expecting line of form '# #'\n", nr);
    exit(1);
  }
  GET_MOVE_RATE(mob_proto + i) = t[0];
  GET_FEROCITY(mob_proto + i) = t[1];

  if(ver > 1) {
    ACTIONS(mob_proto + i) = fread_string(mob_f, buf2);
  }

  return;
}


void parse_mobile(FILE * mob_f, int nr)
{
  static int i = 0;
  int j, t[10];
  char line[256], *tmpptr, letter;
  char f1[128], f2[128];

  mob_index[i].virtual = nr;
  mob_index[i].number = 0;
  mob_index[i].func = NULL;

  clear_char(mob_proto + i);

  mob_proto[i].player_specials = &dummy_mob;
  sprintf(buf2, "mob vnum %d", nr);

  /***** String data *** */
  mob_proto[i].player.name = fread_string(mob_f, buf2);
  tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);
  mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
  mob_proto[i].player.description = fread_string(mob_f, buf2);
  mob_proto[i].player.title = NULL;
  mob_proto[i].char_specials.prompt=str_dup("*");

  /* *** Numeric data *** */
  get_line(mob_f, line);
  t[4]=1;
  sscanf(line, "%s %s %d %c%d", f1, f2, t+2, &letter, t+4);
  MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
  SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
  (mob_proto+i)->char_specials.saved.affected_by = AFF_FLAGS(mob_proto + i) = asciiflag_conv(f2);
  GET_ALIGNMENT(mob_proto + i) = t[2];
  GET_IDNUM(mob_proto + 1) = -1;

  switch (letter) {
  case 'S':	/* Simple monsters */
    parse_simple_mob(mob_f, i, nr);
    break;
  case 'E':	/* Circle3 Enhanced monsters */
    parse_enhanced_mob(mob_f, i, nr);
    break;
  case 'W':	/* Full data for all the new stuff in ashes */
    parse_ashes_mob(mob_f, i, nr, t[4]);
    break;
  /* add new mob types here.. */
  default:
    fprintf(stderr, "Unsupported mob type '%c' in mob #%d\n", letter, nr);
    exit(1);
    break;
  }

  mob_proto[i].aff_abils = mob_proto[i].real_abils;

  for (j = 0; j < NUM_WEARS; j++)
    mob_proto[i].equipment[j] = NULL;

  mob_proto[i].nr = i;
  mob_proto[i].desc = NULL;

  top_of_mobt = i++;
}




/* read all objects from obj file; generate index and prototypes */
char *parse_object(FILE * obj_f, int nr)
{
  static int i = 0, retval;
  static char line[256];
  int t[10], j;
  char *tmpptr;
  char f1[256], f2[256], f3[256];
  struct extra_descr_data *new_descr;

  obj_index[i].virtual = nr;
  obj_index[i].number = 0;
  obj_index[i].func = NULL;

  clear_object(obj_proto + i);
  obj_proto[i].in_room = NOWHERE;
  obj_proto[i].item_number = i;

  sprintf(buf2, "object #%d", nr);

  /* *** string data *** */
  if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL) {
    obj_proto[i].name = str_dup("<NO NAME>");
    /* Due to all the stupid RoI items with no names causing errors, this was changed
    fprintf(stderr, "Null obj name or format error at or near %s\n", buf2);
    exit(1);
    */
  }
  tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);

  tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    *tmpptr = UPPER(*tmpptr);
  obj_proto[i].action_description = fread_string(obj_f, buf2);

  /* *** numeric data *** */
  /* now it reads char affects bitvector too */
  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, " %d %s %s %s", t, f1, f2, f3)) != 4) {
    fprintf(stderr, "Format error in first numeric line (expecting 4 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.type_flag = t[0];
  if(t[0]==ITEM_DAMAGEABLE)
    obj_index[i].func = damage_eq;
  obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
  obj_proto[i].obj_flags.wear_flags = asciiflag_conv(f2);
  obj_proto[i].obj_flags.bitvector = asciiflag_conv(f3);

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, " %s %s %s", f1, f2, f3)) != 3) {
    fprintf(stderr, "Format error in bitvector line (expecting 3 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.immune = asciiflag_conv(f1);
  obj_proto[i].obj_flags.weak = asciiflag_conv(f2);
  obj_proto[i].obj_flags.resist = asciiflag_conv(f3);

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4) {
    fprintf(stderr, "Format error in second numeric line (expecting 4 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.value[0] = t[0];
  obj_proto[i].obj_flags.value[1] = t[1];
  obj_proto[i].obj_flags.value[2] = t[2];
  obj_proto[i].obj_flags.value[3] = t[3];

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d", t, t + 1, t + 2)) != 3) {
    fprintf(stderr, "Format error in third numeric line (expecting 3 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.cost = t[1];
  obj_proto[i].obj_flags.cost_per_day = t[2];

  /* check to make sure that weight of containers exceeds curr. quantity */
  if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON) {
/*    || obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN) { */
    if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
      obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
  }

  if(obj_proto[i].obj_flags.cost<0)
    obj_proto[i].obj_flags.cost=0;
  if(obj_proto[i].obj_flags.cost_per_day<0)
    obj_proto[i].obj_flags.cost_per_day=0;

  /* *** extra descriptions and affect fields *** */

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    obj_proto[i].affected[j].location = APPLY_NONE;
    obj_proto[i].affected[j].modifier = 0;
  }

  strcat(buf2, ", after numeric constants (expecting E/A/#xxx)");
  j = 0;

  for (;;) {
    if (!get_line(obj_f, line)) {
      fprintf(stderr, "Format error in %s\n", buf2);
      exit(1);
    }
    switch (*line) {
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(obj_f, buf2);
      new_descr->description = fread_string(obj_f, buf2);
      new_descr->next = obj_proto[i].ex_description;
      obj_proto[i].ex_description = new_descr;
      break;
    case 'A':
      if (j >= MAX_OBJ_AFFECT) {
	fprintf(stderr, "Too many A fields (%d max), %s\n", MAX_OBJ_AFFECT, buf2);
	exit(1);
      }
      get_line(obj_f, line);
      sscanf(line, " %d %d ", t, t + 1);
      obj_proto[i].affected[j].location = t[0];
      obj_proto[i].affected[j].modifier = t[1];
      j++;
      break;
    case '$':
    case '#':
      top_of_objt = i++;
      return line;
      break;
    default:
      fprintf(stderr, "Format error in %s\n", buf2);
      exit(1);
      break;
    }
  }
}


#define Z	zone_table[zone]

/* load the zone table and command tables */
void load_zones(FILE * fl, char *zonename)
{
  static int zone = 0;
  int cmd_no = 0, num_of_cmds = 0, line_num = 0, tmp, error;
  char *ptr, buf[256], zname[256];

  strcpy(zname, zonename);

  while (get_line(fl, buf))
    num_of_cmds++;		/* this should be correct within 3 or so */
  rewind(fl);

  if (num_of_cmds == 0) {
    fprintf(stderr, "%s is empty!\n", zname);
    exit(0);
  } else
    CREATE(Z.cmd, struct reset_com, num_of_cmds);

  line_num += get_line(fl, buf);

  if (sscanf(buf, "#%d", &Z.number) != 1) {
    fprintf(stderr, "Format error in %s, line %d\n", zname, line_num);
    exit(0);
  }
  sprintf(buf2, "beginning of zone #%d", Z.number);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL)	/* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);
  if (sscanf(buf, " %d %d %d %d %d ", &Z.bottom, &Z.top, &Z.lifespan, &Z.reset_mode, &Z.closed) != 5) {
    fprintf(stderr, "Format error in 5-constant line of %s", zname);
    exit(0);
  }
  Z.locked_by[0]=0;

  cmd_no = 0;

  for (;;) {
    if ((tmp = get_line(fl, buf)) == 0) {
      fprintf(stderr, "Format error in %s - premature end of file\n", zname);
      exit(0);
    }
    line_num += tmp;
    ptr = buf;
    skip_spaces(&ptr);

    if ((ZCMD.command = *ptr) == '*')
      continue;

    ptr++;

    if (ZCMD.command == 'S' || ZCMD.command == '$') {
      ZCMD.command = 'S';
      break;
    }
    error = 0;
    if (strchr("MOEPDG", ZCMD.command) == NULL) {	/* a 5-arg command */
      if (sscanf(ptr, " %d %d %d %d %d ", &tmp, &ZCMD.depend, &ZCMD.arg1,
                 &ZCMD.arg2, &ZCMD.prob) != 5)
	error = 1;
    } else {
      if(ZCMD.command=='E') {
        if (sscanf(ptr, " %d %d %d %d %d %d %d ", &tmp, &ZCMD.depend,
                   &ZCMD.arg1, &ZCMD.arg2, &ZCMD.arg3, &ZCMD.arg4, &ZCMD.prob) != 7)
          error = 1;
      }
      else {
        if (sscanf(ptr, " %d %d %d %d %d %d ", &tmp, &ZCMD.depend, &ZCMD.arg1,
                   &ZCMD.arg2, &ZCMD.arg3, &ZCMD.prob) != 6)
          error = 1;
      }
    }

    if(cmd_no==0)
      ZCMD.depend=-1;

    if(ZCMD.depend >= cmd_no)
      error=1;

    if(ZCMD.depend < 0)
      ZCMD.depend=0;

    ZCMD.if_flag = tmp;

    ZCMD.limit=ZCMD.arg2;

    if (error) {
      fprintf(stderr, "Format error in %s, line %d: '%s'\n", zname, line_num, buf);
      exit(0);
    }

    ZCMD.line = line_num;
    cmd_no++;
  }

  top_of_zone_table = zone++;
}

#undef Z


void get_one_line(FILE *fl, char *buf)
{
  if (fgets(buf, READ_SIZE, fl) == NULL) {
    log("error reading help file: not terminated with $?");
    exit(1);
  }

  buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}


void load_help(FILE *fl)
{
  char key[READ_SIZE+1], next_key[READ_SIZE+1], entry[32384];
  char line[READ_SIZE+1], *scan;
  struct help_index_element el;

  /* get the first keyword line */
  get_one_line(fl, key);
  while (*key != '$') {
    /* read in the corresponding help entry */
    strcpy(entry, strcat(key, "\r\n"));
    get_one_line(fl, line);
    while (*line != '#') {
      strcat(entry, strcat(line, "\r\n"));
      get_one_line(fl, line);
    }

    /* now, add the entry to the index with each keyword on the keyword line */
    el.duplicate = 0;
    el.entry = str_dup(entry);
    scan = one_word(key, next_key);
    while (*next_key) {
      el.keyword = str_dup(next_key);
      help_table[top_of_helpt++] = el;
      el.duplicate++;
      scan = one_word(scan, next_key);
    }

    /* get next keyword line (or $) */
    get_one_line(fl, key);
  }
}


int hsort(const void *a, const void *b)
{
  struct help_index_element *a1, *b1;

  a1 = (struct help_index_element *) a;
  b1 = (struct help_index_element *) b;

  return (str_cmp(a1->keyword, b1->keyword));
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */



int vnum_mobile(char *searchname, struct char_data * ch, bool exact)
{
  int nr, found = 0;
  char *vnum_buf;

  CREATE(vnum_buf, char, 1);
  vnum_buf[0]=0;

  if(exact) {
    for (nr = 0; nr <= top_of_mobt; nr++) {
      if (isexact(searchname, mob_proto[nr].player.name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                mob_index[nr].virtual,
                mob_proto[nr].player.short_descr);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }
  else {
    for (nr = 0; nr <= top_of_mobt; nr++) {
      if (isname(searchname, mob_proto[nr].player.name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                mob_index[nr].virtual,
                mob_proto[nr].player.short_descr);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }

  page_string(ch->desc, vnum_buf, 1);
  free(vnum_buf);
  return(found);
}



int vnum_object(char *searchname, struct char_data * ch, bool exact)
{
  int nr, found = 0;
  char *vnum_buf;

  CREATE(vnum_buf, char, 1);
  vnum_buf[0]=0;

  if(exact) {
    for (nr = 0; nr <= top_of_objt; nr++) {
      if (isexact(searchname, obj_proto[nr].name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                obj_index[nr].virtual,
                obj_proto[nr].short_description);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }
  else {
    for (nr = 0; nr <= top_of_objt; nr++) {
      if (isname(searchname, obj_proto[nr].name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                obj_index[nr].virtual,
                obj_proto[nr].short_description);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }

  page_string(ch->desc, vnum_buf, 1);
  free(vnum_buf);
  return(found);
}


int vnum_room(char *searchname, struct char_data * ch, bool exact)
{
  int nr, found = 0;
  char *vnum_buf;

  CREATE(vnum_buf, char, 1);
  vnum_buf[0]=0;

  if(exact) {
    for (nr = 0; nr <= top_of_world; nr++) {
      if (isexact(searchname, world[nr].name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                world[nr].number,
                world[nr].name);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }
  else {
    for (nr = 0; nr <= top_of_world; nr++) {
      if (isname(searchname, world[nr].name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                world[nr].number,
                world[nr].name);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }

  page_string(ch->desc, vnum_buf, 1);
  free(vnum_buf);
  return(found);
}


int vnum_zone(char *searchname, struct char_data * ch, bool exact)
{
  int nr, found = 0;
  char *vnum_buf;

  CREATE(vnum_buf, char, 1);
  vnum_buf[0]=0;

  if(exact) {
    for (nr = 0; nr <= top_of_zone_table; nr++) {
      if (isexact(searchname, zone_table[nr].name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                zone_table[nr].number,
                zone_table[nr].name);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }
  else {
    for (nr = 0; nr <= top_of_zone_table; nr++) {
      if (isname(searchname, zone_table[nr].name)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                zone_table[nr].number,
                zone_table[nr].name);
        RECREATE(vnum_buf, char, strlen(buf)+strlen(vnum_buf)+1);
        strcat(vnum_buf, buf);
      }
    }
  }

  page_string(ch->desc, vnum_buf, 1);
  free(vnum_buf);
  return(found);
}


/* create a character, and add it to the char list */
struct char_data *create_char(void)
{
  struct char_data *ch;

  CREATE(ch, struct char_data, 1);
  clear_char(ch);
  ch->next = character_list;
  character_list = ch;

  return ch;
}


/* create a new mobile from a prototype */
struct char_data *read_mobile(int nr, int type)
{
  int i;
  struct char_data *mob;

  if (type == VIRTUAL) {
    if ((i = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
      return (0);
    }
  } else
    i = nr;

  CREATE(mob, struct char_data, 1);
  clear_char(mob);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

  if (!mob->points.max_hit) {
    mob->points.max_hit = MAX(MIN(dice(mob->points.hit, mob->points.mana)+mob->points.move, 30000), 1);
  } else
    mob->points.max_hit = MAX(MIN(number(mob->points.hit, mob->points.mana), 30000), 1);

  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  GET_POS(mob) = GET_DEFAULT_POS(mob);

  mob_index[i].number++;

  return mob;
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
  struct obj_data *obj;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  obj->next = object_list;
  object_list = obj;

  return obj;
}


/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type)
{
  struct obj_data *obj;
  int i;

  if (nr < 0) {
    log("SYSERR: trying to create obj with negative num!");
    return NULL;
  }
  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
      sprintf(buf, "Object (V) %d does not exist in database.", nr);
      return NULL;
    }
  } else
    i = nr;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  *obj = obj_proto[i];
  obj->next = object_list;
  object_list = obj;

  if((++item_id)>2147483646L) {
    item_id=0;
  }
  obj->id=item_id;

  obj_index[i].number++;

  return obj;
}



#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  int i;
  struct reset_q_element *update_u, *temp;
  static int timer = 0;
  char buf[128];

  /* jelson 10/22/92 */
  if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {
    /* one minute has passed */
    /*
     * NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
     * factor of 60
     */

    timer = 0;

    /* since one minute has passed, increment zone ages */
    for (i = 0; i <= top_of_zone_table; i++) {
      if (zone_table[i].age < zone_table[i].lifespan &&
	  zone_table[i].reset_mode)
	(zone_table[i].age)++;

      if (zone_table[i].age >= zone_table[i].lifespan &&
	  zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
	/* enqueue zone */

	CREATE(update_u, struct reset_q_element, 1);

	update_u->zone_to_reset = i;
	update_u->next = 0;

	if (!reset_q.head)
	  reset_q.head = reset_q.tail = update_u;
	else {
	  reset_q.tail->next = update_u;
	  reset_q.tail = update_u;
	}

	zone_table[i].age = ZO_DEAD;
      }
    }
  }	/* end - one minute has passed */


  /* dequeue zones (if possible) and reset */
  /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
  for (update_u = reset_q.head; update_u; update_u = update_u->next)
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      sprintf(buf, "Auto zone reset: %s",
	      zone_table[update_u->zone_to_reset].name);
      mudlog(buf, CMP, LVL_ASST, FALSE);
      /* dequeue */
      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u;
	     temp = temp->next);

	if (!update_u->next)
	  reset_q.tail = temp;

	temp->next = update_u->next;
      }

      free(update_u);
      break;
    }
}

void log_zone_error(int zone, int cmd_no, char *message)
{
  char buf[256];

  sprintf(buf, "SYSERR: error in zone file: %s", message);
  mudlog(buf, CMP, LVL_HERO, TRUE);

  sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	  ZCMD.command, zone_table[zone].number, ZCMD.line);
  mudlog(buf, CMP, LVL_HERO, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = -1; }

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int cmd_no, last_cmd = 0, i, j, reset_ok;
  struct char_data *mob = NULL, *tch;
  struct obj_data *obj, *obj_to, *tobj;

  /* call all room procs in zone with -1 cmd */
  for(i=zone_table[zone].bottom; i <= zone_table[zone].top; i++) {
    if((j=real_room(i)) >= 0) {
      if(GET_ROOM_SPEC(j)) {
        (GET_ROOM_SPEC(j))(world[j].people, world+j, -1, "");
      }
    }
  }

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
    ZCMD.i=0;
  }
  for(tch=character_list; tch; tch=tch->next) {
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      if((tch->nr==ZCMD.arg1)&&(tch->mob_specials.cmd==ZCMD.line))
        ZCMD.i++;
    }
  }

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {

    ZCMD.ok=0;
    reset_ok=0;

    if((ZCMD.if_flag > 0) && (zone_table[zone].cmd[ZCMD.depend].ok!=1)) {
      ZCMD.ok=-1;
      continue;
    }

    if((ZCMD.command=='O')||(ZCMD.command=='P')||
       (ZCMD.command=='G')||(ZCMD.command=='E')) {
      if(ZCMD.arg2>0) {
        reset_ok=1;
        ZCMD.arg2--;
      }
    }

    if((ZCMD.if_flag < 0) && (zone_table[zone].cmd[ZCMD.depend].ok!=0)) {
      ZCMD.ok=-1;
      continue;
    }

    if((ZCMD.prob < number(1, 10000))&&(!zlock_reset))
      continue;

    switch (ZCMD.command) {
    case '*':			/* ignore command */
      last_cmd = -1;
      break;

    case 'M':			/* read a mobile */
      if (ZCMD.i < ZCMD.arg2) {
	mob = read_mobile(ZCMD.arg1, REAL);
        mob->mob_specials.cmd=ZCMD.line;
	char_to_room(mob, ZCMD.arg3);
        mob->mob_specials.from_load=1;
	last_cmd = 1;
      } else
	last_cmd = -1;
      break;

    case 'O':			/* read an object */
      if (ZCMD.arg2 || reset_ok) {
	if (ZCMD.arg3 >= 0) {
          for(i=0, tobj=world[ZCMD.arg3].contents; tobj; tobj=tobj->next_content) {
            if((tobj->cmd==ZCMD.line)&&(tobj->item_number==ZCMD.arg1)&&(tobj->from_load))
              i++;
          }
          if(!i) {
            obj = read_object(ZCMD.arg1, REAL);
            obj->cmd=ZCMD.line;
            obj_to_room(obj, ZCMD.arg3);
            GET_OBJ_TIMER(obj) = 0;
            obj->from_load=1;
            last_cmd = 1;
          }
          else
            last_cmd = -1;
	} else {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj->in_room = NOWHERE;
          obj->from_load=1;
	  last_cmd = 1;
	}
      } else
	last_cmd = -1;
      break;

    case 'P':			/* object to object */
      if (ZCMD.arg2 || reset_ok) {
	obj = read_object(ZCMD.arg1, REAL);
        for(obj_to=object_list; obj_to; obj_to=obj_to->next) {
          if((obj_to->item_number==ZCMD.arg3)&&(obj_to->from_load))
            break;
        }
	if (!obj_to) {
	  ZONE_ERROR("target obj not found");
          extract_obj(obj);
	  break;
	}
        for(i=0, tobj=obj_to->contains; tobj; tobj=tobj->next_content) {
          if((tobj->cmd==ZCMD.line)&&(tobj->item_number==ZCMD.arg1)&&(tobj->from_load))
            i++;
        }
        if(!i) {
          obj->cmd=ZCMD.line;
          obj_to_obj(obj, obj_to);
          obj->from_load=1;
          last_cmd = 1;
        }
        else {
          extract_obj(obj);
          last_cmd = -1;
        }
      } else
	last_cmd = -1;
      break;

    case 'G':			/* obj_to_char */
      for(mob=character_list; mob; mob=mob->next) {
        if((mob->nr==ZCMD.arg3)&&(mob->mob_specials.from_load))
          break;
      }
      if (!mob) {
	ZONE_ERROR("attempt to give obj to non-existant mob");
	break;
      }
      for(i=0, tobj=mob->carrying; tobj; tobj=tobj->next_content) {
        if((tobj->cmd==ZCMD.line)&&(tobj->item_number==ZCMD.arg1)&&(tobj->from_load))
          i++;
      }
      if ((!i)&&(ZCMD.arg2 || reset_ok)) {
	obj = read_object(ZCMD.arg1, REAL);
	obj_to_char(obj, mob);
        obj->cmd=ZCMD.line;
        obj->from_load=1;
	last_cmd = 1;
      } else
	last_cmd = -1;
      break;

    case 'E':			/* object to equipment list */
      for(mob=character_list; mob; mob=mob->next) {
        if((mob->nr==ZCMD.arg3)&&(mob->mob_specials.from_load))
          break;
      }
      if (!mob) {
	ZONE_ERROR("trying to equip non-existant mob");
	break;
      }
      if (ZCMD.arg2 || reset_ok) {
	if (ZCMD.arg4 < 0 || ZCMD.arg4 >= NUM_WEARS) {
	  ZONE_ERROR("invalid equipment pos number");
	} else {
          if(GET_EQ(mob, ZCMD.arg4)) {
            ZONE_ERROR("mob already equiped");
          }
          else {
            obj = read_object(ZCMD.arg1, REAL);
            equip_char(mob, obj, ZCMD.arg4);
            obj->cmd=ZCMD.line;
            obj->from_load=1;
            last_cmd = 1;
          }
	}
      } else
	last_cmd = -1;
      break;

    case 'R': /* rem obj from room */
      if ((obj = get_obj_in_list_num(ZCMD.arg2, world[ZCMD.arg1].contents)) != NULL) {
        obj_from_room(obj);
        extract_obj(obj);
        last_cmd = 1;
      }
      else
        last_cmd = -1;
      break;


    case 'D':			/* set state of door */
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
	  (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
	ZONE_ERROR("door does not exist");
      }
      else {
	switch (ZCMD.arg3) {
	case 0:
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_CLOSED);
	  break;
	case 1:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 2:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  break;
	}
        last_cmd = 1;
      }
      break;

    default:
      ZONE_ERROR("unknown cmd in reset table; cmd disabled");
      ZCMD.command = '*';
      break;
    }
    ZCMD.ok=last_cmd;
  }

  zone_table[zone].age = 0;

  /* Done loading, so take from_load off all mobs */
  for(mob=character_list; mob; mob=mob->next)
    mob->mob_specials.from_load=0;

}



/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  struct char_data *i;

  /* old way, faster, but doesn't catch linkless, people taking advantage
  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected) {
      if(i->character->in_room>0) {
        if(world[i->character->in_room].zone == zone_nr)
          return 0;
      }
      else {
        if((i->character->was_in_room > 0) && (world[i->character->was_in_room].zone == zone_nr))
          return 0;
      }
    }
  }
  */
  /* This catches all players */
  for (i = character_list; i; i = i->next) {
    if (!IS_NPC(i)) {
      if(i->in_room>0) {
        if(world[i->in_room].zone == zone_nr)
          return 0;
      }
      else {
        if(i->was_in_room > 0)
          if(world[i->was_in_room].zone == zone_nr)
            return 0;
      }
    }
  }
  return 1;
}


/*************************************************************************
*  stuff related to the save/load player system				 *
*********************************************************************** */
long get_id_by_name(char *name)
{
  int i;
  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp((player_table + i)->name, arg))
      return ((player_table + i)->id);
  return -1;
}
char *get_name_by_id(long id)
{
  int i;
  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);
  return NULL;
}
/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u * char_element)
{
  int player_i;
  int find_name(char *name);
  if ((player_i = find_name(name)) >= 0) {
    fseek(player_fl, (long) (player_i * (long)sizeof(struct char_file_u)), SEEK_SET);
    fread(char_element, sizeof(struct char_file_u), 1, player_fl);
    return (player_i);
  } else
    return (-1);
}
/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
void save_char(struct char_data * ch, sh_int load_room)
{
  int i;
  struct char_file_u st;
  char file_name[128];
  struct alias *a;
  struct salias sa;
  FILE *fp;
  if (IS_NPC(ch) || !ch->desc || GET_PFILEPOS(ch) < 0)
    return;
  char_to_store(ch, &st);
  strncpy(st.host, ch->desc->host, HOST_LENGTH);
  st.host[HOST_LENGTH] = '\0';
  if (!PLR_FLAGGED(ch, PLR_LOADROOM)) {
    if (load_room == NOWHERE)
      st.player_specials_saved.load_room = NOWHERE;
    else
      st.player_specials_saved.load_room = world[load_room].number;
  }
  strcpy(st.pwd, GET_PASSWD(ch));
  fseek(player_fl, GET_PFILEPOS(ch) * (long)sizeof(struct char_file_u), SEEK_SET);
  fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
  /* Save aliases */
  if((!get_filename(GET_NAME(ch), file_name, ALIAS_FILE))||((fp=fopen(file_name, "wb"))==NULL)) {
    log("Error, writing alias file");
  }
  else {
    for(i=0, a=GET_ALIASES(ch); a&&(i<MAX_ALIAS); i++, a=a->next)
    {
      if(strlen(a->alias) > MAX_ALIAS_LENGTH)
        a->alias[MAX_ALIAS_LENGTH]=0;
      strcpy(sa.a_orig, a->alias);
      if(strlen(a->replacement) > MAX_ALIAS_REPLACE)
        a->replacement[MAX_ALIAS_REPLACE]=0;
      strcpy(sa.a_new, a->replacement);
      sa.a_type=a->type;
      fwrite(&sa, sizeof(struct salias), 1, fp);
    }
    fclose(fp);
  }
}
/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, struct char_data * ch)
{
  int i;
  struct salias sa;
  struct alias *talias;
  char file_name[128];
  FILE *fp;
  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);
  st->walkin[MAX_WALK_LENGTH]=0;
  st->walkout[MAX_WALK_LENGTH]=0;
  st->poofin[MAX_WALK_LENGTH]=0;
  st->poofout[MAX_WALK_LENGTH]=0;
  WALKIN(ch)=str_dup(st->walkin);
  WALKOUT(ch)=str_dup(st->walkout);
  POOFIN(ch)=str_dup(st->poofin);
  POOFOUT(ch)=str_dup(st->poofout);
  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_LEVEL(ch) = st->level;
  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  st->title[MAX_TITLE_LENGTH]=0;
  st->description[EXDSCR_LENGTH]=0;
  ch->player.title = str_dup(st->title);
  ch->player.description = str_dup(st->description);
  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);
  ch->player.weight = st->weight;
  ch->player.height = st->height;
  ch->real_abils = st->abilities;
  ch->aff_abils = st->abilities;
  ch->points = st->points;
  ch->char_specials.saved = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;
  st->prompt[MAX_PROMPT_LENGTH]=0;
  ch->char_specials.prompt=str_dup(st->prompt);
  ch->points.armor = 100;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;
  if (ch->player.name == NULL)
    CREATE(ch->player.name, char, strlen(st->name) + 1);
  st->name[MAX_NAME_LENGTH]=0;
  strcpy(ch->player.name, st->name);
  st->pwd[MAX_PWD_LENGTH]=0;
  strcpy(ch->player.passwd, st->pwd);
  /* Add all spell effects */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);
  }
  affect_total(ch);
  /* Load aliases */
  if(!((!get_filename(GET_NAME(ch), file_name, ALIAS_FILE))||((fp=fopen(file_name, "rb"))==NULL))) {
    for(i=0; (i<MAX_ALIAS)&&(!feof(fp)); i++)
    {
      CREATE(talias, struct alias, 1);
      fread(&sa, sizeof(struct salias), 1, fp);
      if(!feof(fp)) {
        talias->alias=str_dup(sa.a_orig);
        talias->replacement=str_dup(sa.a_new);
        talias->type=sa.a_type;
        talias->next=GET_ALIASES(ch);
        GET_ALIASES(ch)=talias;
      }
    }
    fclose(fp);
  }
  /*
   * If you're not poisioned and you've been away for more than an hour of
   * real time, we'll set your HMV back to full
   */
  if (!IS_AFFECTED(ch, AFF_POISON) &&
      (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
  }
}				/* store_to_char */
/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data * ch, struct char_file_u * st)
{
  int i;
  struct affected_type *af;
  struct obj_data *char_eq[NUM_WEARS];
  /* Unaffect everything a character can be affected by */
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      char_eq[i] = unequip_char(ch, i);
    else
      char_eq[i] = NULL;
  }
  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *af;
      st->affected[i].next = 0;
      af = af->next;
    } else {
      st->affected[i].type = 0;	/* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }
  /*
   * remove the affections so that the raw values are stored; otherwise the
   * effects are doubled when the char logs back in.
   */
  while (ch->affected)
    affect_remove(ch, ch->affected);
  if ((i >= MAX_AFFECT) && af && af->next)
    log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");
  ch->aff_abils = ch->real_abils;
  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);
  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = GET_CLASS(ch);
  st->level = GET_LEVEL(ch);
  st->abilities = ch->real_abils;
  st->points = ch->points;
  st->char_specials_saved = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;
  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;
  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';
  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';
  /* Save walks */
  if (WALKIN(ch))
  {
    strncpy(st->walkin, WALKIN(ch), MAX_WALK_LENGTH);
    st->walkin[MAX_WALK_LENGTH]=0;
  }
  else
    *st->walkin = '\0';
  if (WALKOUT(ch))
  {
    strncpy(st->walkout, WALKOUT(ch), MAX_WALK_LENGTH);
    st->walkout[MAX_WALK_LENGTH]=0;
  }
  else
    *st->walkout = '\0';
  /* Save poofs */
  if (POOFIN(ch))
  {
    strncpy(st->poofin, POOFIN(ch), MAX_WALK_LENGTH);
    st->poofin[MAX_WALK_LENGTH]=0;
  }
  else
    *st->poofin = '\0';
  if (POOFOUT(ch))
  {
    strncpy(st->poofout, POOFOUT(ch), MAX_WALK_LENGTH);
    st->poofout[MAX_WALK_LENGTH]=0;
  }
  else
    *st->poofout = '\0';
  /* Save prompt */
  if (ch->char_specials.prompt)
  {
    strncpy(st->prompt, ch->char_specials.prompt, MAX_PROMPT_LENGTH);
    st->prompt[MAX_PROMPT_LENGTH]=0;
  }
  else
    *st->prompt = '\0';
  strcpy(st->name, GET_NAME(ch));
  /* add spell and eq affections back in now */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);
  }
  for (i = 0; i < NUM_WEARS; i++) {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }
  affect_total(ch);
}				/* Char to store */
/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;
  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *)
	       realloc(player_table, sizeof(struct player_index_element) *
		       (++top_of_p_table + 1)))) {
    perror("create entry");
    exit(1);
  }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);
  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);
  return (top_of_p_table);
}
/************************************************************************
*  funcs of a (more or less) general utility nature			*
********************************************************************** */
/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[512], *rslt, *str;
  register char *point;
  int done = 0, length = 0, templength = 0;
  *buf = '\0';
  do {
    if (!fgets(tmp, 512, fl)) {
      fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
	      error);
      exit(1);
    }
    /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL) {
      *point = '\0';
      done = 1;
    } else {
      point = tmp + strlen(tmp) - 1;
      *(point++) = '\r';
      *(point++) = '\n';
      *point = '\0';
    }
    templength = strlen(tmp);
    if (length + templength >= MAX_STRING_LENGTH) {
      log("SYSERR: fread_string: string too large (db.c)");
      log(error);
      exit(1);
    } else {
      strcat(buf + length, tmp);
      length += templength;
    }
  } while (!done);
  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0) {
    str=fix_string(buf);
    CREATE(rslt, char, strlen(str) + 1);
    strcpy(rslt, str);
  } else {
    rslt = str_dup("");
  }
  return rslt;
}
/* release memory allocated for a char struct */
void free_char(struct char_data * ch)
{
  int i;
  struct alias *a;
  struct spcontinuous *sp;
  void free_alias(struct alias * a);
  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {
    while ((a = GET_ALIASES(ch)) != NULL) {
      GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
      free_alias(a);
    }
    if (ch->player_specials->poofin)
      free(ch->player_specials->poofin);
    if (ch->player_specials->poofout)
      free(ch->player_specials->poofout);
    if (ch->player_specials->walkin)
      free(ch->player_specials->walkin);
    if (ch->player_specials->walkout)
      free(ch->player_specials->walkout);
    free(ch->player_specials);
    if (IS_NPC(ch))
      log("SYSERR: Mob had player_specials allocated!");
  }
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (GET_NAME(ch))
      free(GET_NAME(ch));
    if (ch->player.title)
      free(ch->player.title);
    if (ch->player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description)
      free(ch->player.description);
    if (ch->char_specials.prompt)
      free(ch->char_specials.prompt);
    if (ACTIONS(ch))
      free(ACTIONS(ch));
  } else if ((i = GET_MOB_RNUM(ch)) > -1) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
      free(ch->player.name);
    if (ch->player.title && ch->player.title != mob_proto[i].player.title)
      free(ch->player.title);
    if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description && ch->player.description != mob_proto[i].player.description)
      free(ch->player.description);
    if (ch->char_specials.prompt && ch->char_specials.prompt != mob_proto[i].char_specials.prompt)
      free(ch->char_specials.prompt);
    if (ACTIONS(ch) && ACTIONS(ch) != ACTIONS(mob_proto+i))
      free(ACTIONS(ch));
  }
  while (ch->affected)
    affect_remove(ch, ch->affected);
  while (ch->char_specials.spcont) {
    sp=ch->char_specials.spcont->next;
    if(ch->char_specials.spcont->spspell==SKILL_DIMENSION_DOOR) {
      send_to_room("The portal closes.\r\n", ch->char_specials.spcont->spdata1);
      send_to_room("The portal closes.\r\n", ch->char_specials.spcont->spdata2);
      REMOVE_BIT(ROOM_FLAGS(ch->char_specials.spcont->spdata1), ROOM_DIM_DOOR);
      REMOVE_BIT(ROOM_FLAGS(ch->char_specials.spcont->spdata2), ROOM_DIM_DOOR);
    }
    else if(ch->char_specials.spcont->spspell==SKILL_STASIS_FIELD) {
      send_to_room("Time returns to normal.\r\n", ch->char_specials.spcont->spdata1);
      REMOVE_BIT(ROOM_FLAGS(ch->char_specials.spcont->spdata1), ROOM_STASIS);
    }
    free(ch->char_specials.spcont);
    ch->char_specials.spcont=sp;
  }
  free(ch);
}
/* release memory allocated for an obj struct */
void free_obj(struct obj_data * obj)
{
  int nr;
  struct extra_descr_data *this, *next_one;
  if ((nr = GET_OBJ_RNUM(obj)) == -1) {
    if (obj->name)
      free(obj->name);
    if (obj->description)
      free(obj->description);
    if (obj->short_description)
      free(obj->short_description);
    if (obj->action_description)
      free(obj->action_description);
    if (obj->ex_description)
      for (this = obj->ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
  } else {
    if (obj->name && obj->name != obj_proto[nr].name)
      free(obj->name);
    if (obj->description && obj->description != obj_proto[nr].description)
      free(obj->description);
    if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
      free(obj->short_description);
    if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
      free(obj->action_description);
    if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
      for (this = obj->ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
  }
  free(obj);
}
/* used to file_to_string_alloc, higher memory limit */
int file_to_large_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[READ_SIZE+3];
  *buf = '\0';
  if (!(fl = fopen(name, "r"))) {
    sprintf(tmp, "Error reading %s", name);
    perror(tmp);
    return (-1);
  }
  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");
    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 1 > (3*MAX_STRING_LENGTH)) {
        sprintf(buf, "SYSERR: %s: string too big (%d max)", name,
		(3*MAX_STRING_LENGTH));
	log(buf);
	*buf = '\0';
        fclose(fl);
	return -1;
      }
      strcat(buf, tmp);
    }
  } while (!feof(fl));
  fclose(fl);
  return (0);
}
/* read contets of a text file, alloc space, point buf to it */
int file_to_string_alloc(char *name, char **buf)
{
  char temp[3*MAX_STRING_LENGTH];
  if (*buf)
    free(*buf);
  if (file_to_large_string(name, temp) < 0) {
    *buf = "";
    return -1;
  } else {
    *buf = str_dup(temp);
    return 0;
  }
}
/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[READ_SIZE+3];
  *buf = '\0';
  if (!(fl = fopen(name, "r"))) {
    sprintf(tmp, "Error reading %s", name);
    perror(tmp);
    return (-1);
  }
  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");
    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
        sprintf(buf, "SYSERR: %s: string too big (%d max)", name,
		MAX_STRING_LENGTH);
	log(buf);
	*buf = '\0';
        fclose(fl);
	return -1;
      }
      strcat(buf, tmp);
    }
  } while (!feof(fl));
  fclose(fl);
  return (0);
}
/* clear some of the the working variables of a char */
void reset_char(struct char_data * ch)
{
  int i;
  char buf[40];
  struct char_data *syschar;
  struct affected_type *af;
  ACMD(do_advance);
  for (i = 0; i < NUM_WEARS; i++)
    GET_EQ(ch, i) = NULL;
  for(af=ch->affected; af; af=af->next) {
    if(af->duration < 0)
      af->duration=0;
  }
  ch->followers = NULL;
  ch->master = NULL;
  ch->in_room = NOWHERE;
  ch->carrying = NULL;
  ch->next = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room = NULL;
  FIGHTING(ch) = NULL;
  ch->char_specials.position = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;
  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;
  GET_LAST_TELL(ch) = NOBODY;
  GET_FORWARD_TELL(ch) = NOBODY;
  GET_OLC_MODE(ch)=GET_OLC_FIELD(ch)=GET_OLC_NUM(ch)=0;
  GET_OLC_PTR(ch)=NULL;
  REMOVE_BIT(PLR_FLAGS(ch), PLR_TOURING);
/* Impl is highest level now, so anyone higher is cheating or a bug */
  if(GET_LEVEL(ch) > LVL_IMPL)
    GET_LEVEL(ch)=1;
  if((!str_cmp(GET_NAME(ch), "Sarnoth")) || (!str_cmp(GET_NAME(ch), "Asita"))) {
    if((GET_LEVEL(ch) < LVL_IMPL) && (GET_LEVEL(ch) > 0) && (!ch->player_specials->saved.rerolling)) {
      ch->next=character_list;
      character_list=ch;
      char_to_room(ch, 0);
      syschar=create_char();
      GET_LEVEL(syschar)=LVL_IMPL;
      GET_NAME(syschar)=str_dup("SYSTEM");
      CREATE(syschar->player_specials, struct player_special_data, 1);
      SET_BIT(PRF_FLAGS(syschar), PRF_HOLYLIGHT);
      char_to_room(syschar, 0);
      sprintf(buf, "%s %d", GET_NAME(ch), LVL_IMPL);
      do_advance(syschar, buf, 0, 0);
      char_from_room(syschar);
      character_list=syschar->next;
      free_char(syschar);
      char_from_room(ch);
      character_list=ch->next;
    }
  }
  else if(GET_LEVEL(ch) >= LVL_IMPL) {
    GET_LEVEL(ch)=1;
  }
}
/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void clear_char(struct char_data * ch)
{
  memset((char *) ch, 0, sizeof(struct char_data));
  ch->in_room = NOWHERE;
  GET_PFILEPOS(ch) = -1;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch) = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->char_specials.vc = -1;
  ch->char_specials.vci = -1;
  ch->char_specials.vcm = FALSE;
  ch->char_specials.vcs = FALSE;
  GET_AC(ch) = 100;		/* Basic Armor */
  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;
}
void clear_object(struct obj_data * obj)
{
  memset((char *) obj, 0, sizeof(struct obj_data));
  obj->item_number = NOTHING;
  obj->in_room = NOWHERE;
  obj->worn_on = -1;
}
/* initialize a new character only if class is set */
void init_char(struct char_data * ch)
{
  int i;
  /* create a player_special structure */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);
  GET_STR(ch) = 9;
  GET_INT(ch) = 9;
  GET_WIS(ch) = 9;
  GET_DEX(ch) = 9;
  GET_CON(ch) = 9;
  ch->player_specials->saved.olc_min1=ch->player_specials->saved.olc_min2=0;
  ch->player_specials->saved.olc_max1=ch->player_specials->saved.olc_max2=0;
  /* *** if this is our first player --- he be God *** */
  if (top_of_p_table == 0) {
    GET_EXP(ch) = 7000000;
    GET_LEVEL(ch) = LVL_IMPL;
    ch->player_specials->saved.old_hit = ch->points.max_hit = 500;
    ch->player_specials->saved.old_mana = ch->points.max_mana = 100;
    ch->player_specials->saved.old_move = ch->points.max_move = 82;
    GET_STR(ch) = 25;
    GET_INT(ch) = 25;
    GET_WIS(ch) = 25;
    GET_DEX(ch) = 25;
    GET_CON(ch) = 25;
  }
  set_title(ch, NULL);
  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.description = NULL;
  ch->player.hometown = 1;
  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
  for (i = 0; i < MAX_TONGUE; i++)
    GET_TALK(ch, i) = 0;
  /* make favors for sex */
  if (ch->player.sex == SEX_MALE) {
    ch->player.weight = number(120, 180);
    ch->player.height = number(160, 200);
  } else {
    ch->player.weight = number(110, 175);
    ch->player.height = number(150, 190);
  }
  ch->player_specials->saved.new_mana = ch->points.max_mana = 0;
  ch->points.mana = GET_MAX_MANA(ch);
  ch->player_specials->saved.new_hit = ch->points.hit = GET_MAX_HIT(ch) = 0;
  ch->player_specials->saved.new_move = ch->points.max_move = 0;
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.armor = 100;
  player_table[GET_PFILEPOS(ch)].id = GET_IDNUM(ch) = ++top_idnum;
  for (i = 1; i <= MAX_SKILLS; i++) {
    if (GET_LEVEL(ch) < LVL_IMPL)
      SET_SKILL(ch, i, 0)
    else
      SET_SKILL(ch, i, 100);
  }
  ch->char_specials.saved.affected_by = 0;
  for (i = 0; i < 5; i++)
    GET_SAVE(ch, i) = 0;
  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : 24);
  GET_LOADROOM(ch) = NOWHERE;
  WALKIN(ch)=str_dup("enters");
  WALKOUT(ch)=str_dup("leaves");
  POOFIN(ch)=str_dup("appears with a loud bang.");
  POOFOUT(ch)=str_dup("vanishes in a bright flash.");
  ch->char_specials.prompt=str_dup("*");
  for(i=0; i<MAX_REMEMBER; i++)
    GET_REMEMBER(ch, i)=NOWHERE;
}
/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
  int bot, top, mid;
  bot = 0;
  top = top_of_world;
  /* perform binary search on world-table */
  for (;;) {
    mid = (bot + top) / 2;
    if ((world + mid)->number == virtual)
      return mid;
    if (bot >= top)
      return NOWHERE;
    if ((world + mid)->number > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
  int bot, top, mid;
  bot = 0;
  top = top_of_mobt;
  /* perform binary search on mob-table */
  for (;;) {
    mid = (bot + top) / 2;
    if ((mob_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
  int bot, top, mid;
  bot = 0;
  top = top_of_objt;
  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;
    if ((obj_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
/* returns the real number of the zone with given virtual number */
int real_zone(int virtual)
{
  int bot, top, mid;
  bot = 0;
  top = top_of_zone_table;
  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;
    if ((zone_table + mid)->number == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((zone_table + mid)->number > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
/* MOB Prog functions */
int mprog_name_to_type ( char *name )
{
   if ( !str_cmp( name, "in_file_prog"   ) )    return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )    return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) )    return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )    return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )    return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )    return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )    return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )    return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )    return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) )    return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )    return BRIBE_PROG;
   if ( !str_cmp( name, "commandtrap_prog" ) )  return COMMANDTRAP_PROG;
   if ( !str_cmp( name, "keyword_prog"   ) )    return KEYWORD_PROG;
   return( ERROR_PROG );
}
char fread_letter(FILE *fp)
{
  char c;
  do {
   c = getc(fp);
  } while (isspace(c));
  return c;
}
char *fread_word(FILE *fp)
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;
    do
    {
        cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );
    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = ' ';
    }
    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace(*pword) || *pword == '~' : *pword == cEnd )
        {
            if ( cEnd == ' ' || cEnd == '~' )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }
    log("SYSERR: Fread_word: word too long.");
    exit( 1 );
    return NULL;
}
/* This routine reads in scripts of MOBprograms from a file */
MPROG_DATA* mprog_file_read( FILE *progfile, MPROG_DATA *mprg,
                            struct index_data *pMobIndex )
{
  char        garbage[MAX_INPUT_LENGTH+1];
  MPROG_DATA *mprg2;
  char        letter;
  bool        done = FALSE;
  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug2( "empty mobprog file.");
       exit( 1 );
     break;
    default:
       bug2( "in mobprog file syntax error.");
       exit( 1 );
     break;
  }
  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
        bug2( "mobprog file type error");
        exit( 1 );
      break;
     case IN_FILE_PROG:
        bug2( "mprog file contains a call to file.");
        exit( 1 );
      break;
     default:
        strcat(buf2, "Error in mobprog file");
        pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
        mprg2->arglist       = fread_string( progfile,buf2 );
        mprg2->comlist       = fread_string( progfile,buf2 );
        switch ( letter = fread_letter( progfile ) )
        {
          case '>':
             mprg2->next = (MPROG_DATA *)malloc( sizeof( MPROG_DATA ) );
             mprg2       = mprg2->next;
             mprg2->next = NULL;
           break;
          case '|':
             done = TRUE;
           break;
          default:
             bug2( "syntax error in mobprog file.");
             exit( 1 );
           break;
        }
      break;
    }
  }
  fgets(garbage, MAX_INPUT_LENGTH, progfile);
  return mprg2;
}
