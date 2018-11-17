/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"

int guest_num=1;

extern char *motd;
extern char *imotd;
extern char *background;
extern char *MENU;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int restrict_mud;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;

/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
void init_char(struct char_data *ch);
int create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
int mprog_commandtrap_trigger(int cmdnum, struct char_data *mob, struct char_data* ch);
int mprog_keyword_trigger(char *cmdline, struct char_data *mob, struct char_data *ch);


/* prototypes for all do_x functions. */
ACMD(do_accept);
ACMD(do_action);
ACMD(do_advance);
ACMD(do_alias);
ACMD(do_arena);
ACMD(do_assassin);
ACMD(do_assist);
ACMD(do_at);
ACMD(do_autoauction);
ACMD(do_autoreboot);
ACMD(do_award);
ACMD(do_backstab);
ACMD(do_backtrace);
ACMD(do_ban);
ACMD(do_bash);
ACMD(do_beam);
ACMD(do_befriend);
ACMD(do_berserk);
ACMD(do_bid);
ACMD(do_camouflage);
ACMD(do_cast);
ACMD(do_challenge);
ACMD(do_circle);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_consider);
ACMD(do_cont);
ACMD(do_credits);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_diagnose);
ACMD(do_disarm);
ACMD(do_discont);
ACMD(do_display);
ACMD(do_disrupt);
ACMD(do_dive);
ACMD(do_divine);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_extract);
ACMD(do_featherfoot);
ACMD(do_filet);
ACMD(do_flee);
ACMD(do_follow);
ACMD(do_forage);
ACMD(do_force);
ACMD(do_forcehold);
ACMD(do_forceremove);
ACMD(do_forcewear);
ACMD(do_forcewield);
ACMD(do_format);
ACMD(do_forward);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_gouge);
ACMD(do_grab);
ACMD(do_grant);
ACMD(do_grapple);
ACMD(do_grep);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_healwounds);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_hurl);
ACMD(do_identify);
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_load);
ACMD(do_look);
ACMD(do_medit);
ACMD(do_memorize);
ACMD(do_memory);
ACMD(do_mindcloud);
ACMD(do_mlist);
ACMD(do_move);
ACMD(do_mpasound);
ACMD(do_mpat);
ACMD(do_mpcalm);
ACMD(do_mpcast);
ACMD(do_mpdamage);
ACMD(do_mpdrop);
ACMD(do_mpecho);
ACMD(do_mpechoaround);
ACMD(do_mpechoat);
ACMD(do_mpforce);
ACMD(do_mpgoto);
ACMD(do_mpheal);
ACMD(do_mpjunk);
ACMD(do_mpkill);
ACMD(do_mpmload);
ACMD(do_mpoload);
ACMD(do_mppurge);
ACMD(do_mpscast);
ACMD(do_mpset);
ACMD(do_mpstat);
ACMD(do_mpstransfer);
ACMD(do_mpstun);
ACMD(do_mptransfer);
ACMD(do_mpunaffect);
ACMD(do_nodisturb);
ACMD(do_not_here);
ACMD(do_oedit);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_olcmenu);
ACMD(do_olist);
ACMD(do_order);
ACMD(do_page);
ACMD(do_pagelen);
ACMD(do_poisonblade);
ACMD(do_poofset);
ACMD(do_pour);
ACMD(do_practice);
ACMD(do_psi);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_reboot);
ACMD(do_redirect);
ACMD(do_redit);
ACMD(do_reimb);
ACMD(do_remember);
ACMD(do_remort);
ACMD(do_remove);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_request);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_retire);
ACMD(do_return);
ACMD(do_rlist);
ACMD(do_save);
ACMD(do_say);
ACMD(do_scan);
ACMD(do_score);
ACMD(do_send);
ACMD(do_set);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_silentwalk);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_skin);
ACMD(do_sleep);
ACMD(do_snare);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_stuntouch);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_throw);
ACMD(do_time);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_unban);
ACMD(do_ungroup);
ACMD(do_use);
ACMD(do_users);
ACMD(do_vcaction);
ACMD(do_vcdepart);
ACMD(do_vcemote);
ACMD(do_vcinvite);
ACMD(do_vcjoin);
ACMD(do_vckick);
ACMD(do_vclist);
ACMD(do_vcmoderator);
ACMD(do_vcsay);
ACMD(do_vcsquelch);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_wake);
ACMD(do_walkset);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_whirlwind);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizupdate);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_zbottom);
ACMD(do_zcreate);
ACMD(do_zdelete);
ACMD(do_zedit);
ACMD(do_zlifespan);
ACMD(do_zlist);
ACMD(do_zlock);
ACMD(do_zname);
ACMD(do_zopen);
ACMD(do_zpurge);
ACMD(do_zreset);
ACMD(do_zrmode);
ACMD(do_ztop);
ACMD(do_zunlock);


ACMD(do_iedit);
ACMD(do_pedit);

ACMD(do_delword);
ACMD(do_insword);



/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 * Level of -1 makes the command not show up in the command list
 */

const struct command_info cmd_info[] = {
  { "RESERVED", 0, 0, 0, 0, 0 },	/* this must be first -- for specprocs */

  /* directions must come before other commands but after RESERVED */
  { "north"    , POS_STANDING, do_move     , 0, SCMD_NORTH, 0 },
  { "east"     , POS_STANDING, do_move     , 0, SCMD_EAST, 0 },
  { "south"    , POS_STANDING, do_move     , 0, SCMD_SOUTH, 0 },
  { "west"     , POS_STANDING, do_move     , 0, SCMD_WEST, 0 },
  { "up"       , POS_STANDING, do_move     , 0, SCMD_UP, 0 },
  { "down"     , POS_STANDING, do_move     , 0, SCMD_DOWN, 0 },
  { "ne"       , POS_STANDING, do_move     , 0, SCMD_NE, 0 },
  { "nw"       , POS_STANDING, do_move     , 0, SCMD_NW, 0 },
  { "se"       , POS_STANDING, do_move     , 0, SCMD_SE, 0 },
  { "sw"       , POS_STANDING, do_move     , 0, SCMD_SW, 0 },
  { "northeast", POS_STANDING, do_move     , 0, SCMD_NE, 0 },
  { "northwest", POS_STANDING, do_move     , 0, SCMD_NW, 0 },
  { "southeast", POS_STANDING, do_move     , 0, SCMD_SE, 0 },
  { "southwest", POS_STANDING, do_move     , 0, SCMD_SW, 0 },

  /* now, the main list */
  { "at"       , POS_DEAD    , do_at       , LVL_HERO, 0, 0 },
  { "accuse"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "accept"   , POS_STANDING, do_accept   , 0, 0, 0 },
  { "ack"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "addict"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "adrenalincontrol",POS_STANDING,do_psi , 1, SKILL_ADRENALIN_CONTROL, 0},
  { "advance"  , POS_DEAD    , do_advance  , LVL_CIMP, 0, GRNT_ADVANCE },
  { "afk"      , POS_DEAD    , do_gen_tog  , 0, SCMD_AFK, 0 },
  { "agree"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "ahem"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "alias"    , POS_DEAD    , do_alias    , 0, 0, 0 },
  { "annoy"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "applaud"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "apologize", POS_SITTING , do_action   , 0, 0, 0 },
  { "appraise" , POS_RESTING , do_not_here , 0, 0, 0 },
  { "arena"    , POS_DEAD    , do_arena    , LVL_ASST, 0, 0 },
  { "assist"   , POS_FIGHTING, do_assist   , 1, 0, 0 },
  { "asay"     , POS_DEAD    , do_gen_comm , 0, SCMD_ASAY, 0 },
  { "ask"      , POS_RESTING , do_spec_comm, 0, SCMD_ASK, 0 },
  { "assassin" , POS_DEAD    , do_assassin , LVL_ASST, 0, GRNT_ASSASSIN },
  { "auction"  , POS_SLEEPING, do_gen_comm , 0, SCMD_AUCTION, 0 },
  { "autogold" , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOGOLD, 0 },
  { "autoauction",POS_SLEEPING,do_autoauction,0,0, 0 },
  { "autoloot" , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOLOOT, 0 },
  { "autoreboot",POS_DEAD    , do_autoreboot,LVL_CIMP, 0, GRNT_REBOOT },
  { "autosplit", POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOSPLIT, 0 },
  { "avatar"   , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_AVTR, 0 },
  { "award"    , POS_DEAD    , do_award    , LVL_CIMP, 0, GRNT_AWARD },

  { "bounce"   , POS_STANDING, do_action   , 0, 0, 0 },
  { "backstab" , POS_STANDING, do_backstab , 1, 0, 0 },
  { "backtrace", POS_DEAD    , do_backtrace, LVL_HERO, 0, 0 },
  { "badger"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "baffle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "bah"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "balance"  , POS_STANDING, do_not_here , 1, 0, 0 },
  { "ballisticattack",POS_FIGHTING,do_psi  , 1, SKILL_BALLISTIC_ATTACK, 0},
  { "ban"      , POS_DEAD    , do_ban      , LVL_CIMP, 0, GRNT_BAN },
  { "bark"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "bash"     , POS_FIGHTING, do_bash     , 1, 0, 0 },
  { "beckon"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "bearhug"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "beam"     , POS_STANDING, do_beam     , 1, 0, 0 },
  { "befriend" , POS_STANDING, do_befriend , 1, 0, 0 },
  { "beg"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "berserk"  , POS_FIGHTING, do_berserk  , 0, 0, 0 },
  { "bid"      , POS_SLEEPING, do_bid      , 0, 0, 0 },
  { "biofeedback",POS_FIGHTING,do_psi      , 1, SKILL_BIOFEEDBACK, 0},
  { "bite"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "blush"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "bleed"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "blink"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "blow"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "boggle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "bodyweaponry",POS_STANDING,do_psi     , 1, SKILL_BODY_WEAPONRY, 0},
  { "bonk"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "boogie"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "book"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "bow"      , POS_STANDING, do_action   , 0, 0, 0 },
  { "brief"    , POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF, 0 },
  { "brag"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "brb"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "buy"      , POS_STANDING, do_not_here , 0, 0, 0 },
  { "bucket"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "bug"      , POS_DEAD    , do_gen_write, 0, SCMD_BUG, 0 },
  { "burp"     , POS_RESTING , do_action   , 0, 0, 0 },

  { "cast"     , POS_SITTING , do_cast     , 1, 0, 0 },
  { "cackle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "camouflage",POS_STANDING, do_camouflage,1, 0, 0 },
  { "cannonball",POS_SITTING , do_action   , 0, 0, 0 },
  { "cannibalize",POS_STANDING,do_psi      , 1, SKILL_CANNIBALIZE, 0},
  { "caress"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "celladjustment",POS_STANDING,do_psi   , 1, SKILL_CELL_ADJUSTMENT, 0},
  { "chuckle"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "challenge", POS_STANDING, do_challenge, 0, 0, 0 },
  { "chameleonpower",POS_STANDING,do_psi   , 1, SKILL_CHAMELEON_POWER, 0},
  { "check"    , POS_STANDING, do_not_here , 1, 0, 0 },
  { "cheer"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "circle"   , POS_FIGHTING, do_circle   , 1, 0, 0 },
  { "close"    , POS_SITTING , do_gen_door , 0, SCMD_CLOSE, 0 },
  { "clap"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "clairvoyance",POS_STANDING,do_psi     , 1, SKILL_CLAIRVOYANCE, 0},
  { "clear"    , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR, 0 },
  { "clinton"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "cls"      , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR, 0 },
  { "consider" , POS_RESTING , do_consider , 0, 0, 0 },
  { "comb"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "combine"  , POS_DEAD    , do_gen_tog  , 0, SCMD_COMBINE, 0 },
  { "completehealing",POS_STANDING,do_psi  , 1, SKILL_COMPLETE_HEALING, 0},
  { "confuse"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "continuous",POS_DEAD    , do_cont     , 0, 0, 0 },
  { "color"    , POS_DEAD    , do_color    , 0, 0, 0 },
  { "collapse" , POS_RESTING , do_action   , 0, 0, 0 },
  { "comfort"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "commands" , POS_DEAD    , do_commands , 0, SCMD_COMMANDS, 0 },
  { "compact"  , POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT, 0 },
  { "cornholio", POS_RESTING , do_action   , 0, 0, 0 },
  { "cough"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "cower"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "cry"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "credits"  , POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS, 0 },
  { "cringe"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "cuddle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "curse"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "curtsey"  , POS_STANDING, do_action   , 0, 0, 0 },

  { "dance"    , POS_STANDING, do_action   , 0, 0, 0 },
  { "dangersense",POS_STANDING,do_psi      , 1, SKILL_DANGER_SENSE, 0},
  { "date"     , POS_DEAD    , do_date     , LVL_HERO, SCMD_DATE, 0 },
  { "daydream" , POS_SLEEPING, do_action   , 0, 0, 0 },
  { "dc"       , POS_DEAD    , do_dc       , LVL_IMMORT, 0, 0 },
  { "deposit"  , POS_STANDING, do_not_here , 1, 0, 0 },
  { "deathfield",POS_FIGHTING,do_psi       , 1, SKILL_DEATH_FIELD, 0},
  { "delword"  , POS_DEAD    , do_delword  , LVL_HERO, 0, 0 },
  { "diagnose" , POS_RESTING , do_diagnose , 0, 0, 0 },
  { "dimensiondoor",POS_STANDING,do_psi    , 1, SKILL_DIMENSION_DOOR, 0},
  { "disarm"   , POS_FIGHTING, do_disarm   , 1, 0, 0 },
  { "discontinue",POS_DEAD   , do_discont  , 0, 0, 0 },
  { "disintegrate",POS_FIGHTING,do_psi     , 1, SKILL_DISINTEGRATE, 0},
  { "display"  , POS_DEAD    , do_display  , 0, 0, 0 },
  { "displacement",POS_FIGHTING,do_psi     , 1, SKILL_DISPLACEMENT, 0},
  { "disrupt"  , POS_FIGHTING, do_disrupt  , 1, 0, 0 },
  { "dive"     , POS_FIGHTING, do_dive     , 1, 0, 0 },
  { "divine"   , POS_STANDING, do_divine   , 1, 0, 0 },
  { "donate"   , POS_RESTING , do_drop     , 0, SCMD_DONATE, 0 },
  { "doh"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "domination",POS_STANDING, do_psi      , 1, SKILL_DOMINATION, 0},
  { "drop"     , POS_RESTING , do_drop     , 0, SCMD_DROP, 0 },
  { "draw"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "drink"    , POS_RESTING , do_drink    , 0, SCMD_DRINK, 0 },
  { "drool"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "duck"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "dualbackstab",POS_STANDING,do_backstab, 1, SCMD_DUAL, 0 },
  { "duh"      ,  POS_RESTING, do_action   , 0, 0, 0 },

  { "eat"      , POS_RESTING , do_eat      , 0, SCMD_EAT, 0 },
  { "echo"     , POS_SLEEPING, do_echo     , LVL_IMMORT, SCMD_ECHO, 0 },
  { "eek"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "egowhip"  , POS_FIGHTING, do_psi      , 1, SKILL_EGO_WHIP, 0},
  { "emote"    , POS_RESTING , do_echo     , 1, SCMD_EMOTE, 0 },
  { ":"        , POS_RESTING , do_echo     , 1, SCMD_EMOTE, 0 },
  { "embrace"  , POS_STANDING, do_action   , 0, 0, 0 },
  { "enter"    , POS_STANDING, do_enter    , 0, 0, 0 },
  { "energycontainment",POS_FIGHTING,do_psi, 1, SKILL_ENERGY_CONTAINMENT, 0},
  { "envy"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "equipment", POS_SLEEPING, do_equipment, 0, 0, 0 },
  { "espanol"  , POS_SLEEPING, do_gen_comm , 1, SCMD_ESP, 0 },
  { "examine"  , POS_SITTING , do_examine  , 0, 0, 0 },
  { "exits"    , POS_RESTING , do_exits    , 0, 0, 0 },
  { "extract"  , POS_DEAD    , do_extract  , LVL_CIMP, 0, 0 },
  { "eyebrow"  , POS_RESTING , do_action   , 0, 0, 0 },

  { "force"    , POS_SLEEPING, do_force    , LVL_CIMP, 0, GRNT_FORCE },
  { "fakeload" , POS_DEAD    , do_load     , LVL_HERO, SCMD_FAKE, 0 },
  { "faint"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "fart"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "featherfoot",POS_STANDING,do_featherfoot,1,0, 0 },
  { "feellight", POS_FIGHTING, do_psi      , 1, SKILL_FEEL_LIGHT, 0},
  { "ffff"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "fill"     , POS_STANDING, do_pour     , 0, SCMD_FILL, 0 },
  { "filet"    , POS_STANDING, do_filet    , 1, 0, 0 },
  { "flee"     , POS_FIGHTING, do_flee     , 1, 0, 0 },
  { "flesharmor",POS_STANDING, do_psi      , 1, SKILL_FLESH_ARMOR, 0},
  { "flex"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "flip"     , POS_STANDING, do_action   , 0, 0, 0 },
  { "flirt"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "flutter"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "follow"   , POS_RESTING , do_follow   , 0, 0, 0 },
  { "fondle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "forcewear", POS_DEAD    , do_forcewear, LVL_HERO, 0, 0 },
  { "forcehold", POS_DEAD    , do_forcehold, LVL_HERO, 0, 0 },
  { "forceremove",POS_DEAD   , do_forceremove,LVL_HERO,0, 0 },
  { "forcewield",POS_DEAD    , do_forcewield,LVL_HERO, 0, 0 },
  { "forage"   , POS_STANDING, do_forage   , 1, 0, 0 },
  { "format"   , POS_DEAD    , do_format   , LVL_HERO, 0, 0 },
  { "forward"  , POS_SLEEPING, do_forward  , 0, 0, 0 },
  { "french"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "francais" , POS_SLEEPING, do_gen_comm , 0, SCMD_FRENCH, 0 },
  { "freeze"   , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_FREEZE, GRNT_FREEZE },
  { "freimb"   , POS_DEAD    , do_reimb    , LVL_CIMP, SCMD_FORCE, GRNT_FREIMB },
  { "frown"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "fume"     , POS_RESTING , do_action   , 0, 0, 0 },

  { "get"      , POS_RESTING , do_get      , 0, 0, 0 },
  { "gasp"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "gag"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "gecho"    , POS_DEAD    , do_gecho    , LVL_ASST, 0, GRNT_GECHO },
  { "give"     , POS_RESTING , do_give     , 0, 0, 0 },
  { "giggle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "glare"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "goto"     , POS_SLEEPING, do_goto     , LVL_HERO, 0, 0 },
  { "gossip"   , POS_SLEEPING, do_gen_comm , 0, SCMD_GOSSIP, 0 },
  { "gold"     , POS_RESTING , do_gold     , 0, 0, 0 },
  { "goose"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "gouge"    , POS_FIGHTING, do_gouge    , 1, 0, 0 },
  { "group"    , POS_SLEEPING, do_group    , 1, 0, 0 },
  { "grab"     , POS_RESTING , do_grab     , 0, 0, 0 },
  { "graftweapon",POS_STANDING,do_psi      , 1, SKILL_GRAFT_WEAPON, 0},
  { "grapple"  , POS_FIGHTING, do_grapple  , 0, 0, 0 },
  { "grats"    , POS_SLEEPING, do_gen_comm , 0, SCMD_GRATZ, 0 },
  { "grant"    , POS_DEAD    , do_grant    , LVL_CIMP, 0, 0 },
  { "greet"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "grep"     , POS_DEAD    , do_grep     , LVL_CIMP, 0, GRNT_GREP },
  { "grin"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "groan"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "grope"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "grovel"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "growl"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "grumble"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "gsay"     , POS_SLEEPING, do_gsay     , 0, 0, 0 },
  { "gtell"    , POS_SLEEPING, do_gsay     , 0, 0, 0 },
  { "gulp"     , POS_SITTING , do_action   , 0, 0, 0 },

  { "help"     , POS_DEAD    , do_help     , 0, 0, 0 },
  { "handbook" , POS_DEAD    , do_gen_ps   , LVL_HERO, SCMD_HANDBOOK, 0 },
  { "handshake", POS_SITTING , do_action   , 0, 0, 0 },
  { "hasta"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "headbutt" , POS_SITTING , do_action   , 0, 0, 0 },
  { "healwounds",POS_STANDING, do_healwounds,1, 0, 0 },
  { "hit"      , POS_FIGHTING, do_hit      , 0, 0, 0 },
  { "hiccup"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "hide"     , POS_RESTING , do_hide     , 1, 0, 0 },
  { "hmm"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "hold"     , POS_RESTING , do_grab     , 1, 0, 0 },
  { "holler"   , POS_RESTING , do_gen_comm , 1, SCMD_HOLLER, 0 },
  { "holylight", POS_DEAD    , do_gen_tog  , LVL_HERO, SCMD_HOLYLIGHT, 0 },
  { "hop"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "hug"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "hurl"     , POS_FIGHTING, do_hurl     , 1, 0, 0 },

  { "inventory", POS_DEAD    , do_inventory, 0, 0, 0 },
  { "ice"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "idea"     , POS_DEAD    , do_gen_write, 0, SCMD_IDEA, 0 },
  { "identify" , POS_RESTING , do_identify , 0, 0, 0 },
  { "iedit"    , POS_DEAD    , do_iedit    , LVL_CIMP, 0, GRNT_IEDIT },
  { "igrep"    , POS_DEAD    , do_grep     , LVL_CIMP, SCMD_IGREP, GRNT_GREP },
  { "immlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST, 0 },
  { "imotd"    , POS_DEAD    , do_gen_ps   , LVL_HERO, SCMD_IMOTD, 0 },
  { "insult"   , POS_RESTING , do_insult   , 0, 0, 0 },
  { "info"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO, 0 },
  { "innocent" , POS_RESTING , do_action   , 0, 0, 0 },
  { "insword"  , POS_DEAD    , do_insword  , LVL_HERO, 0, 0 },
  { "invis"    , POS_DEAD    , do_invis    , LVL_HERO, 0, 0 },

  { "junk"     , POS_RESTING , do_drop     , 0, SCMD_JUNK, 0 },
  { "jk"       , POS_RESTING , do_action   , 0, 0, 0 },

  { "kill"     , POS_FIGHTING, do_kill     , 0, 0, 0 },
  { "kick"     , POS_FIGHTING, do_kick     , 1, 0, 0 },
  { "kiss"     , POS_RESTING , do_action   , 0, 0, 0 },

  { "look"     , POS_RESTING , do_look     , 0, SCMD_LOOK, 0 },
  { "laugh"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "lap"      , POS_SITTING , do_action   , 0, 0, 0 },
  { "lag"      , POS_SITTING , do_action   , 0, 0, 0 },
  { "last"     , POS_DEAD    , do_last     , LVL_IMMORT, 0, 0 },
  { "leave"    , POS_STANDING, do_leave    , 0, 0, 0 },
  { "leer"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "lendhealth",POS_FIGHTING, do_psi      , 1, SKILL_LEND_HEALTH, 0},
  { "levels"   , POS_DEAD    , do_levels   , 0, 0, 0 },
  { "list"     , POS_STANDING, do_not_here , 0, 0, 0 },
  { "lick"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "lifedetection",POS_STANDING,do_psi    , 1, SKILL_LIFE_DETECTION, 0},
  { "lifedraining",POS_FIGHTING,do_psi     , 1, SKILL_LIFE_DRAINING, 0},
  { "load"     , POS_DEAD    , do_load     , LVL_HERO, 0, 0 },
  { "lock"     , POS_SITTING , do_gen_door , 0, SCMD_LOCK, 0 },
  { "love"     , POS_RESTING , do_action   , 0, 0, 0 },

  { "moan"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "massage"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "magnify"  , POS_STANDING, do_psi      , 1, SKILL_MAGNIFY, 0},
  { "mail"     , POS_STANDING, do_not_here , 1, 0, 0 },
  { "marry"    , POS_SITTING , do_action   , 0, 0, 0 },
  { "medit"    , POS_DEAD    , do_medit    , LVL_HERO, 0, 0 },
  { "memory"   , POS_DEAD    , do_memory   , 0, 0, 0 },
  { "memorize" , POS_DEAD    , do_memorize , 0, 0, 0 },
  { "metamorphosis",POS_STANDING,do_psi    , 1, SKILL_METAMORPHOSIS, 0},
  { "mindcloud", POS_STANDING, do_mindcloud, 1, 0, 0 },
  { "mlist"    , POS_DEAD    , do_mlist    , LVL_HERO, 0, 0 },
  { "mmm"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "mosh"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "molecularagitation",POS_FIGHTING,do_psi,1, SKILL_MOLECULAR_AGITATION, 0},
  { "mortlog"  , POS_DEAD    , do_gen_tog  , 0, SCMD_MORTLOG, 0 },
  { "motd"     , POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD, 0 },
  { "move"     , POS_STANDING, do_not_here , 0, 0, 0 },
  { "mpstat"   , POS_DEAD    , do_mpstat   , LVL_IMMORT, 0, 0 },
  { "murder"   , POS_FIGHTING, do_hit      , 0, 0, 0 },
  { "music"    , POS_SLEEPING, do_gen_comm , 0, SCMD_MUS, 0 },
  { "mute"     , POS_DEAD    , do_wizutil  , LVL_ASST, SCMD_SQUELCH, GRNT_MUTE },
  { "mutter"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "mpasound" , POS_DEAD    , do_mpasound , -1, 0, 0 },
  { "mpat"     , POS_DEAD    , do_mpat     , -1, 0, 0 },
  { "mpcalm"   , POS_DEAD    , do_mpcalm   , -1, 0, 0 },
  { "mpcast"   , POS_DEAD    , do_mpcast   , -1, 0, 0 },
  { "mpdamage" , POS_DEAD    , do_mpdamage , -1, 0, 0 },
  { "mpdrop"   , POS_DEAD    , do_mpdrop   , -1, 0, 0 },
  { "mpecho"   , POS_DEAD    , do_mpecho   , -1, 0, 0 },
  { "mpechoaround",POS_DEAD  , do_mpechoaround,-1,0,0 },
  { "mpechoat" , POS_DEAD    , do_mpechoat , -1, 0, 0 },
  { "mpforce"  , POS_DEAD    , do_mpforce  , -1, 0, 0 },
  { "mpgoto"   , POS_DEAD    , do_mpgoto   , -1, 0, 0 },
  { "mpheal"   , POS_DEAD    , do_mpheal   , -1, 0, 0 },
  { "mpjunk"   , POS_DEAD    , do_mpjunk   , -1, 0, 0 },
  { "mpkill"   , POS_DEAD    , do_mpkill   , -1, 0, 0 },
  { "mpmload"  , POS_DEAD    , do_mpmload  , -1, 0, 0 },
  { "mpoload"  , POS_DEAD    , do_mpoload  , -1, 0, 0 },
  { "mppurge"  , POS_DEAD    , do_mppurge  , -1, 0, 0 },
  { "mpscast"  , POS_DEAD    , do_mpscast  , -1, 0, 0 },
  { "mpset"    , POS_DEAD    , do_mpset    , -1, 0, 0 },
  { "mpstransfer",POS_DEAD   , do_mpstransfer,-1,0, 0 },
  { "mpstun"   , POS_DEAD    , do_mpstun   , -1, 0, 0 },
  { "mptransfer",POS_DEAD    , do_mptransfer,-1, 0, 0 },
  { "mpunaffect",POS_DEAD    , do_mpunaffect,-1, 0, 0 },

  { "news"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS, 0 },
  { "narf"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "nibble"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "nixon"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "nod"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "noauction", POS_DEAD    , do_gen_tog  , 0, SCMD_NOAUCTION, 0 },
  { "noarena"  , POS_DEAD    , do_gen_tog  , 0, SCMD_NOARENA, 0 },
  { "noasay"   , POS_DEAD    , do_gen_tog  , 0, SCMD_NOASAY, 0 },
  { "nodisturb", POS_DEAD    , do_nodisturb, LVL_HERO, 0, 0 },
  { "noexits"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOEXIT, 0 },
  { "noespanol", POS_DEAD    , do_gen_tog  , 0, SCMD_NOESP, 0 },
  { "nofrancais",POS_DEAD    , do_gen_tog  , 0, SCMD_NOFRAN, 0 },
  { "nogossip" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP, 0 },
  { "nognog"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "nograts"  , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGRATZ, 0 },
  { "nohassle" , POS_DEAD    , do_gen_tog  , LVL_HERO, SCMD_NOHASSLE, 0 },
  { "nomusic"  , POS_DEAD    , do_gen_tog  , 0, SCMD_NOMUS, 0 },
  { "nomenu"   , POS_DEAD    , do_gen_tog  , LVL_HERO, SCMD_NOMENU, 0 },
  { "noogie"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "noodle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "norepeat" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT, 0 },
  { "noshout"  , POS_SLEEPING, do_gen_tog  , 1, SCMD_DEAF, 0 },
  { "nosummon" , POS_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON, 0 },
  { "notell"   , POS_DEAD    , do_gen_tog  , 1, SCMD_NOTELL, 0 },
  { "notitle"  , POS_DEAD    , do_wizutil  , LVL_ASST, SCMD_NOTITLE, GRNT_NOTITLE },
  { "nowiznet" , POS_DEAD    , do_gen_tog  , LVL_HERO, SCMD_NOWIZ, 0 },
  { "nuzzle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "nudge"    , POS_RESTING , do_action   , 0, 0, 0 },

  { "order"    , POS_RESTING , do_order    , 1, 0, 0 },
  { "oedit"    , POS_DEAD    , do_oedit    , LVL_HERO, 0, 0 },
  { "offer"    , POS_STANDING, do_not_here , 1, 0, 0 },
  { "olc"      , POS_DEAD    , do_olc      , LVL_HERO, 0, 0 },
  { "olist"    , POS_DEAD    , do_olist    , LVL_HERO, 0, 0 },
  { "ooo"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "open"     , POS_SITTING , do_gen_door , 0, SCMD_OPEN, 0 },
  { "opp"      , POS_RESTING , do_action   , 0, 0, 0 },

  { "put"      , POS_RESTING , do_put      , 0, 0, 0 },
  { "pat"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "page"     , POS_DEAD    , do_page     , LVL_HERO, 0, 0 },
  { "pagelength",POS_SLEEPING, do_pagelen  , 0, 0, 0 },
  { "peck"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "pedit"    , POS_DEAD    , do_pedit    , LVL_CIMP, 0, GRNT_PEDIT },
  { "peer"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "pick"     , POS_STANDING, do_gen_door , 1, SCMD_PICK, 0 },
  { "pinch"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "pour"     , POS_STANDING, do_pour     , 0, SCMD_POUR, 0 },
  { "point"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "poisonblade",POS_STANDING,do_poisonblade,0,0, 0 },
  { "poke"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "policy"   , POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES, 0 },
  { "ponder"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "poofin"   , POS_DEAD    , do_poofset  , LVL_HERO, SCMD_POOFIN, 0 },
  { "poofout"  , POS_DEAD    , do_poofset  , LVL_HERO, SCMD_POOFOUT, 0 },
  { "potty"    , POS_SITTING , do_action   , 0, 0, 0 },
  { "pout"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "pounce"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "practice" , POS_SLEEPING, do_practice , 1, 0, 0 },
  { "pray"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "prompt"   , POS_DEAD    , do_display  , 0, 0, 0 },
  { "probabilitytravel",POS_FIGHTING,do_psi, 1, SKILL_PROBABILITY_TRAVEL, 0},
  { "propose"  , POS_SITTING , do_action   , 0, 0, 0 },
  { "psionicblast",POS_FIGHTING,do_psi     , 1, SKILL_PSIONIC_BLAST, 0},
  { "psychiccrush",POS_FIGHTING,do_psi     , 1, SKILL_PSYCHIC_CRUSH, 0},
  { "pthbt"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "puke"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "punch"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "purr"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "purge"    , POS_DEAD    , do_purge    , LVL_HERO, 0, 0 },
  { "push"     , POS_STANDING, do_not_here , 0, 0, 0 },
  { "puzzle"   , POS_SITTING , do_action   , 0, 0, 0 },

  { "quaff"    , POS_RESTING , do_use      , 0, SCMD_QUAFF, 0 },
  { "qecho"    , POS_DEAD    , do_qcomm    , LVL_IMMORT, SCMD_QECHO, 0 },
  { "qqui"     , POS_SLEEPING, do_quit     , 0, 0, 0 },
  { "qquit"    , POS_SLEEPING, do_quit     , 0, SCMD_QQUIT, 0 },
  { "qsay"     , POS_RESTING , do_qcomm    , 0, SCMD_QSAY, 0 },
  { "quasimodo", POS_RESTING , do_action   , 0, 0, 0, },
  { "quest"    , POS_DEAD    , do_gen_tog  , 0, SCMD_QUEST, 0 },
  { "qui"      , POS_SLEEPING, do_quit     , 0, 0, 0 },
  { "quit"     , POS_SLEEPING, do_quit     , 0, SCMD_QUIT, 0 },

  { "reply"    , POS_SLEEPING, do_reply    , 0, 0, 0 },
  { "read"     , POS_RESTING , do_look     , 0, SCMD_READ, 0 },
  { "recite"   , POS_RESTING , do_use      , 0, SCMD_RECITE, 0 },
  { "receive"  , POS_STANDING, do_not_here , 1, 0, 0 },
  { "recharge" , POS_RESTING , do_not_here , 0, 0, 0 },
  { "redit"    , POS_DEAD    , do_redit    , LVL_HERO, 0, 0 },
  { "redirect" , POS_FIGHTING, do_redirect , 1, 0, 0 },
  { "reimburse", POS_DEAD    , do_reimb    , LVL_ASST, 0, GRNT_REIMB },
  { "reload"   , POS_DEAD    , do_reboot   , LVL_CIMP, 0, 0 },
  { "remove"   , POS_RESTING , do_remove   , 0, 0, 0 },
  { "remember" , POS_STANDING, do_remember , 1, 0, 0 },
  { "remort"   , POS_DEAD    , do_remort   , LVL_CIMP, 0, GRNT_ADVANCE },
  { "rent"     , POS_STANDING, do_not_here , 1, 0, 0 },
  { "report"   , POS_DEAD    , do_report   , 0, 0, 0 },
  { "repair"   , POS_RESTING , do_not_here , 0, 0, 0 },
  { "request"  , POS_DEAD    , do_request  , 75, 0, 0 },
  { "reroll"   , POS_DEAD    , do_wizutil  , 0, SCMD_REROLL, 0 },
  { "rest"     , POS_SLEEPING, do_rest     , 0, 0, 0 },
  { "rescue"   , POS_FIGHTING, do_rescue   , 1, 0, 0 },
  { "restore"  , POS_DEAD    , do_restore  , LVL_CIMP, 0, GRNT_RESTORE },
  { "return"   , POS_DEAD    , do_return   , 0, 0, 0 },
  { "retire"   , POS_STANDING, do_retire   , 0, 0, 0 },
  { "rlist"    , POS_DEAD    , do_rlist    , LVL_HERO, 0, 0 },
  { "roll"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "rofl"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "roomflags", POS_DEAD    , do_gen_tog  , LVL_HERO, SCMD_ROOMFLAGS, 0 },
  { "rose"     , POS_SITTING , do_action   , 0, 0, 0 },
  { "rqui"     , POS_SLEEPING, do_quit     , 0, 0, 0 },
  { "rquit"    , POS_SLEEPING, do_quit     , 0, SCMD_RQUIT, 0 },
  { "ruffle"   , POS_STANDING, do_action   , 0, 0, 0 },

  { "say"      , POS_RESTING , do_say      , 0, 0, 0 },
  { "'"        , POS_RESTING , do_say      , 0, 0, 0 },
  { "salute"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "save"     , POS_SLEEPING, do_save     , 0, 0, 0 },
  { "score"    , POS_DEAD    , do_score    , 0, 0, 0 },
  { "scan"     , POS_STANDING, do_scan     , 1, 0, 0 },
  { "scream"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "scratch"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "sell"     , POS_STANDING, do_not_here , 0, 0, 0 },
  { "send"     , POS_SLEEPING, do_send     , LVL_IMMORT, 0, 0 },
  { "set"      , POS_DEAD    , do_set      , LVL_CIMP, 0, GRNT_SET },
  { "shout"    , POS_RESTING , do_gen_comm , 0, SCMD_SHOUT, 0 },
  { "shake"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "shiver"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "show"     , POS_DEAD    , do_show     , LVL_HERO, 0, 0 },
  { "shrug"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "shutdow"  , POS_DEAD    , do_shutdown , LVL_CIMP, 0, GRNT_REBOOT },
  { "shutdown" , POS_DEAD    , do_shutdown , LVL_CIMP, SCMD_SHUTDOWN, GRNT_REBOOT },
  { "sigh"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "silentwalk",POS_STANDING, do_silentwalk,1, 0, 0 },
  { "sing"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "sip"      , POS_RESTING , do_drink    , 0, SCMD_SIP, 0 },
  { "sit"      , POS_SLEEPING, do_sit      , 0, 0, 0 },
  { "skillset" , POS_SLEEPING, do_skillset , LVL_CIMP, 0, GRNT_SET },
  { "skin"     , POS_STANDING, do_skin     , 1, 0, 0 },
  { "sleep"    , POS_SLEEPING, do_sleep    , 0, 0, 0 },
  { "slap"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "slowns"   , POS_DEAD    , do_gen_tog  , LVL_CIMP, SCMD_SLOWNS, 0 },
  { "smile"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "smack"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "smirk"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "smooch"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "snuggle"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "snap"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "snare"    , POS_STANDING, do_snare    , 1, 0, 0 },
  { "snarl"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "sneeze"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "sneak"    , POS_STANDING, do_sneak    , 1, 0, 0 },
  { "snicker"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "sniff"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "snore"    , POS_SLEEPING, do_action   , 0, 0, 0 },
  { "snowball" , POS_STANDING, do_action   , LVL_HERO, 0, 0 },
  { "snoop"    , POS_DEAD    , do_snoop    , LVL_CIMP, 0, GRNT_SNOOP },
  { "socials"  , POS_DEAD    , do_commands , 0, SCMD_SOCIALS, 0 },
  { "soap"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "sob"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "split"    , POS_SITTING , do_split    , 1, 0, 0 },
  { "spank"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "spam"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "spit"     , POS_STANDING, do_action   , 0, 0, 0 },
  { "splitpersonality",POS_STANDING,do_psi , 1, SKILL_SPLIT_PERSONALITY, 0},
  { "squeeze"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "squeal"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "stand"    , POS_SLEEPING, do_stand    , 0, 0, 0 },
  { "stat"     , POS_DEAD    , do_stat     , LVL_HERO, 0, 0 },
  { "stare"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "stasisfield",POS_FIGHTING,do_psi      , 1, SKILL_STASIS_FIELD, 0},
  { "steal"    , POS_STANDING, do_steal    , 1, 0, 0 },
  { "steam"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "stifle"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "stroke"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "strut"    , POS_STANDING, do_action   , 0, 0, 0 },
  { "stuntouch", POS_FIGHTING, do_stuntouch, 1, 0, 0 },
  { "sulk"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "sweat"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "sweep"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "switch"   , POS_DEAD    , do_switch   , LVL_ASST, 0, GRNT_SWITCH },
  { "swoon"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "syslog"   , POS_DEAD    , do_syslog   , LVL_HERO, 0, 0 },

  { "tell"     , POS_DEAD    , do_tell     , 0, 0, 0 },
  { "tackle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "take"     , POS_RESTING , do_get      , 0, 0, 0 },
  { "tango"    , POS_STANDING, do_action   , 0, 0, 0 },
  { "tap"      , POS_STANDING, do_action   , 0, 0, 0 },
  { "taunt"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "taste"    , POS_RESTING , do_eat      , 0, SCMD_TASTE, 0 },
  { "teleport" , POS_DEAD    , do_teleport , LVL_HERO, 0, 0 },
  { "telekinesis",POS_FIGHTING,do_psi      , 1, SKILL_TELEKINESIS, 0},
  { "thank"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "think"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "thaw"     , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_THAW, GRNT_THAW },
  { "throw"    , POS_FIGHTING, do_throw    , 1, 0, 0 },
  { "title"    , POS_DEAD    , do_title    , 0, 0, 0 },
  { "tickle"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "time"     , POS_DEAD    , do_time     , 0, 0, 0 },
  { "toggle"   , POS_DEAD    , do_toggle   , 0, 0, 0 },
  { "tongue"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "track"    , POS_STANDING, do_track    , 0, 0, 0 },
  { "train"    , POS_STANDING, do_not_here , 2, 0, 0 },
  { "transfer" , POS_SLEEPING, do_trans    , LVL_HERO, 0, 0 },
  { "twiddle"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "twirl"    , POS_SITTING , do_action   , 0, 0, 0 },
  { "twitch"   , POS_RESTING , do_action   , 0, 0, 0 },
  { "typo"     , POS_DEAD    , do_gen_write, 0, SCMD_TYPO, 0 },
  { "type"     , POS_RESTING , do_action   , 0, 0, 0 },

  { "unlock"   , POS_SITTING , do_gen_door , 0, SCMD_UNLOCK, 0 },
  { "unaffect" , POS_DEAD    , do_wizutil  , LVL_CIMP, SCMD_UNAFFECT, GRNT_UNAFFECT },
  { "unban"    , POS_DEAD    , do_unban    , LVL_CIMP, 0, GRNT_UNBAN },
  { "ungroup"  , POS_DEAD    , do_ungroup  , 0, 0, 0 },
  { "uninvis"  , POS_RESTING , do_not_here , 0, 0, 0 },
  { "uptime"   , POS_DEAD    , do_date     , LVL_HERO, SCMD_UPTIME, 0 },
  { "use"      , POS_SITTING , do_use      , 1, SCMD_USE, 0 },
  { "users"    , POS_DEAD    , do_users    , LVL_HERO, 0, 0 },

  { "vcjoin"   , POS_DEAD    , do_vcjoin   , 0, 0, 0 },
  { "vcaction" , POS_DEAD    , do_vcaction , 0, 0, 0 },
  { "<"        , POS_DEAD    , do_vcaction , 0, 0, 0 },
  { "vcdepart" , POS_DEAD    , do_vcdepart , 0, 0, 0 },
  { "vcemote"  , POS_DEAD    , do_vcemote  , 0, 0, 0 },
  { ","        , POS_DEAD    , do_vcemote  , 0, 0, 0 },
  { "vcinvite" , POS_DEAD    , do_vcinvite , 0, 0, 0 },
  { "vckick"   , POS_DEAD    , do_vckick   , 0, 0, 0 },
  { "vclist"   , POS_DEAD    , do_vclist   , 0, 0, 0 },
  { "vcmoderator",POS_DEAD   , do_vcmoderator,0,0, 0 },
  { "vcsay"    , POS_DEAD    , do_vcsay    , 0, 0, 0 },
  { "."        , POS_DEAD    , do_vcsay    , 0, 0, 0 },
  { "vcsquelch", POS_DEAD    , do_vcsquelch, 0, 0, 0 },
  { "value"    , POS_STANDING, do_not_here , 0, 0, 0 },
  { "version"  , POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION, 0 },
  { "visible"  , POS_RESTING , do_visible  , 1, 0, 0 },
  { "violin"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "vnum"     , POS_DEAD    , do_vnum     , LVL_HERO, 0, 0 },
  { "vstat"    , POS_DEAD    , do_vstat    , LVL_HERO, 0, 0 },

  { "wake"     , POS_SLEEPING, do_wake     , 0, 0, 0 },
  { "walkin"   , POS_DEAD    , do_walkset  , 0, SCMD_WALKIN, 0 },
  { "walkout"  , POS_DEAD    , do_walkset  , 0, SCMD_WALKOUT, 0 },
  { "waltz"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "wave"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "wear"     , POS_RESTING , do_wear     , 0, 0, 0 },
  { "weather"  , POS_RESTING , do_weather  , 0, 0, 0 },
  { "wedgie"   , POS_SITTING , do_action   , 0, 0, 0 },
  { "who"      , POS_DEAD    , do_who      , 0, 0, 0 },
  { "whoami"   , POS_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI, 0 },
  { "where"    , POS_RESTING , do_where    , 1, 0, 0 },
  { "wheedle"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "whisper"  , POS_RESTING , do_spec_comm, 0, SCMD_WHISPER, 0 },
  { "whine"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "whirlwind", POS_FIGHTING, do_whirlwind, 1, 0, 0 },
  { "whistle"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "wield"    , POS_RESTING , do_wield    , 0, 0, 0 },
  { "wiggle"   , POS_STANDING, do_action   , 0, 0, 0 },
  { "wimpy"    , POS_DEAD    , do_wimpy    , 0, 0, 0 },
  { "wince"    , POS_RESTING , do_action   , 0, 0, 0 },
  { "wind"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "wink"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "withdraw" , POS_STANDING, do_not_here , 1, 0, 0 },
  { "wiznet"   , POS_DEAD    , do_wiznet   , LVL_HERO, 0, 0 },
  { ";"        , POS_DEAD    , do_wiznet   , LVL_HERO, 0, 0 },
  { "/"        , POS_DEAD    , do_wiznet   , LVL_HERO, 0, 0 },
  { "wizhelp"  , POS_SLEEPING, do_commands , LVL_HERO, SCMD_WIZHELP, 0 },
  { "wizlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST, 0 },
  { "wizlock"  , POS_DEAD    , do_wizlock  , LVL_CIMP, 0, GRNT_WIZLOCK },
  { "wizupdate", POS_DEAD    , do_wizupdate, LVL_CIMP, 0, 0 },
  { "worship"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "woowoo"      , POS_RESTING , do_action   , 0, 0, 0 },
  { "write"    , POS_STANDING, do_write    , 1, 0, 0 },
  { "wtf"      , POS_RESTING , do_action   , 0, 0, 0 },

  { "yawn"     , POS_RESTING , do_action   , 0, 0, 0 },
  { "yodel"    , POS_RESTING , do_action   , 0, 0, 0 },

  { "zerbert"  , POS_RESTING , do_action   , 0, 0, 0 },
  { "zbottom"  , POS_DEAD    , do_zbottom  , LVL_CIMP, 0, GRNT_ZTOP },
  { "zclose"   , POS_DEAD    , do_zopen    , LVL_CIMP, SCMD_CLOSE, GRNT_ZCLOSE },
  { "zcreate"  , POS_DEAD    , do_zcreate  , LVL_CIMP, 0, GRNT_ZCREATE },
  { "zdelet"   , POS_DEAD    , do_zdelete  , LVL_CIMP, 0, 0 },
  { "zdelete"  , POS_DEAD    , do_zdelete  , LVL_CIMP, SCMD_DELETE, 0 },
  { "zedit"    , POS_DEAD    , do_zedit    , LVL_HERO, 0, 0 },
  { "zlock"    , POS_DEAD    , do_zlock    , LVL_HERO, 0, 0 },
  { "zlifespan", POS_DEAD    , do_zlifespan, LVL_HERO, 0, 0 },
  { "zlist"    , POS_DEAD    , do_zlist    , LVL_HERO, 0, 0 },
  { "zname"    , POS_DEAD    , do_zname    , LVL_HERO, 0, 0 },
  { "zopen"    , POS_DEAD    , do_zopen    , LVL_CIMP, 0, GRNT_ZOPEN },
  { "zpurge"   , POS_DEAD    , do_zpurge   , LVL_HERO, 0, 0 },
  { "zreset"   , POS_DEAD    , do_zreset   , LVL_CIMP, 0, GRNT_ZRESET },
  { "zrmode"   , POS_DEAD    , do_zrmode   , LVL_HERO, 0, 0 },
  { "ztop"     , POS_DEAD    , do_ztop     , LVL_CIMP, 0, GRNT_ZTOP },
  { "zunlock"  , POS_DEAD    , do_zunlock  , LVL_HERO, 0, 0 },

  { "\n", 0, 0, 0, 0, 0 } };	/* this must be last */


char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

char *reserved[] =
{
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  char *line;
  struct char_data *tch;
  extern int no_specials;

  REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  if(PRF_FLAGGED(ch, PRF_AFK)) {
    if(str_cmp(argument, "afk"))
      send_to_char("You don't look afk.\r\n", ch);
  }

  if(ch->desc && ch->desc->original) {
    sprintf(buf, "(GC) %s(switched): %s", GET_NAME(ch->desc->original), argument);
    mudlog(buf, NRM, LVL_IMPL+1, TRUE);
  }

  /* just drop to next line for hitting CR */
  skip_spaces(&argument);
  if (!*argument) {
    if((!IS_NPC(ch))&&(GET_LEVEL(ch)>=LVL_HERO)&&GET_OLC_MODE(ch)) {
      do_olcmenu(ch, argument, 0, 0);
    }
    return;
  }

  /* Handles OLC menu choices */
  if((!IS_NPC(ch))&&(GET_LEVEL(ch)>=LVL_HERO)&&is_number(argument)) {
    if(GET_OLC_MODE(ch)) {
      do_olcmenu(ch, argument, 0, 0);
    }
    else {
      send_to_char("You aren't editting anything.\r\n", ch);
    }
    return;
  }

  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalnum(*argument)) {
    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument + 1;
  } else
    line = any_one_arg(argument, arg);

  /* otherwise, find the command */
  for (length = strlen(arg), cmd = 0; ((cmd<=NUM_OF_DIRS+4)&&(*cmd_info[cmd].command != '\n')); cmd++)
    if (!strncmp(cmd_info[cmd].command, arg, length))
      break;
  if(cmd>NUM_OF_DIRS) {
    if(cmd<=NUM_OF_DIRS+4) {
      cmd-=4;
    }
    else {
      if(AFF_FLAGGED(ch, AFF_INVISIBLE)) {
        if(affected_by_spell(ch, SKILL_MINDCLOUD))
          affect_from_char(ch, SKILL_MINDCLOUD);
      }
      for ( ; *cmd_info[cmd].command != '\n'; cmd++) {
        if (!strncmp(cmd_info[cmd].command, arg, length)) {
          if ((GET_LEVEL(ch) >= cmd_info[cmd].minimum_level) || (GRNT_FLAGGED(ch, cmd_info[cmd].grant))) {
            break;
          }
        }
        if(*cmd_info[cmd].command != arg[0]) {
          while(((*cmd_info[cmd].command != arg[0])&&(*cmd_info[cmd].command != '\n'))) cmd++;
          cmd--;
        }
      }
    }
  }

  if((GET_CON(ch) < 3) && (GET_POS(ch) > POS_STUNNED)) {
    send_to_char("You collapse as a result of your low constitution.\r\n", ch);
    GET_POS(ch) = POS_STUNNED;
  }

  if (*cmd_info[cmd].command == '\n') {
    for(tch=world[ch->in_room].people; tch; tch=tch->next_in_room) {
      if(IS_MOB(tch))
        if(IS_SET(mob_index[tch->nr].progtypes, KEYWORD_PROG))
          if(mprog_keyword_trigger(argument, tch, ch))
            return;
    }
    send_to_char("That is not a command on Ashes to Ashes, please retype.\r\n", ch);
  }
  else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_CIMP)
    send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
  else if (cmd_info[cmd].command_pointer == NULL)
    send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC(ch) && (cmd_info[cmd].minimum_level >= LVL_HERO))
    send_to_char("Mobs can't use immortal commands.\r\n", ch);
  else if (GET_POS(ch) < cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
      break;
    case POS_STUNNED:
      send_to_char("All you can do right now is think about the stars!\r\n", ch);
      break;
    case POS_SLEEPING:
      send_to_char("In your dreams, or what?\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("Maybe you should get on your feet first?\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
  } else if (no_specials || !special(ch, cmd, line))
    ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));
}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias *find_alias(struct alias *alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)	/* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
	return alias_list;

    alias_list = alias_list->next;
  }

  return NULL;
}


void free_alias(struct alias *a)
{
  if (a->alias)
    free(a->alias);
  if (a->replacement)
    free(a->replacement);
  free(a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
  char *repl;
  struct alias *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {			/* no argument specified -- list currently defined aliases */
    send_to_char("Currently defined aliases:\r\n", ch);
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
	sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
	send_to_char(buf, ch);
	a = a->next;
      }
    }
  } else {			/* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
      if (a == NULL)
	send_to_char("No such alias.\r\n", ch);
      else
	send_to_char("Alias deleted.\r\n", ch);
    } else {			/* otherwise, either add or redefine an alias */
      if (!str_cmp(arg, "alias")) {
	send_to_char("You can't alias 'alias'.\r\n", ch);
	return;
      }
      CREATE(a, struct alias, 1);
      a->alias = str_dup(arg);
      delete_doubledollar(repl);
      a->replacement = str_dup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
	a->type = ALIAS_COMPLEX;
      else
	a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      send_to_char("Alias added.\r\n", ch);
    }
  }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  temp = strtok(strcpy(buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
	strcpy(write_point, tokens[num]);
	write_point += strlen(tokens[num]);
      } else if (*temp == ALIAS_GLOB_CHAR) {
	strcpy(write_point, orig);
	write_point += strlen(orig);
      } else if(*temp == '$') { /* redouble $ for act safety */
	*(write_point++) = '$';
	*(write_point++) = '$';
      }
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias *a, *tmp;

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return 0;

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return 0;

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return 0;

  if (a->type == ALIAS_SIMPLE) {
    strcpy(orig, a->replacement);
    return 0;
  } else {
    perform_complex_alias(&d->input, ptr, a);
    return 1;
  }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, char **list, bool exact)
{
  register int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;			/* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }

  return -1;
}


int is_number(char *str)
{
  if((*str)=='-')
    str++;

  while (*str)
    if (!isdigit(*(str++)))
      return 0;

  return 1;
}


void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}


char *delete_doubledollar(char *string)
{
  char *read, *write;

  if ((write = strchr(string, '$')) == NULL)
    return string;

  read = write;

  while (*read)
    if ((*(write++) = *(read++)) == '$')
      if (*read == '$')
	read++;

  *write = '\0';

  return string;
}


int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while (*argument && !isspace(*argument)) {
      *(first_arg++) = LOWER(*argument);
      argument++;
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}

/* same as any_one_arg but doesn't change case */
char *anyonearg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = (*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}

/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg); /* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 * 
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return 0;

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return 0;

  if (!*arg1)
    return 1;
  else
    return 0;
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
  int cmd;

  for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  /* special in room? */
  if (GET_ROOM_SPEC(ch->in_room) != NULL)
    if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return 1;

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++)
    if(GET_EQ(ch, j))
      if(GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
        if(GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
          return 1;

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (GET_MOB_SPEC(k) != NULL)
      if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
	return 1;

  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (IS_MOB(k))
      if (IS_SET(mob_index[k->nr].progtypes, COMMANDTRAP_PROG))
        if (mprog_commandtrap_trigger(cmd, k, ch))
          return 1;

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return i;
  }

  return -1;
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha(*arg))
      return 1;

  if (!i)
    return 1;

  return 0;
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3


int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
      if (!target) {
	target = k->original;
	mode = UNSWITCH;
      }
      if (k->character)
	k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && (GET_IDNUM(k->character) == id)) {
      if (!target && STATE(k) == CON_PLAYING) {
	SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
	target = k->character;
	mode = USURP;
      }
      if(!target && k->character->player_specials->saved.rerolling) {
        extract_char(k->character);
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
    }
  }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;
    if (IS_NPC(ch))
      continue;
    if (GET_IDNUM(ch) != id)
      continue;
    /* ignore chars with descriptors (already handled by above step) */
    if (ch->desc)
      continue;
    /* don't extract the target char we've found one already */
    if (ch == target)
      continue;
    /* we don't already have a target and found a candidate for switching */
    if (!target) {
      target = ch;
      mode = RECON;
      continue;
    }
    /* we've found a duplicate - blow him away, dumping his eq in limbo. */
    if (ch->in_room != NOWHERE)
      char_from_room(ch);
    char_to_room(ch, 1);
    extract_char(ch);
  }
  /* no target for swicthing into was found - allow login to continue */
  if (!target)
    return 0;
  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
  STATE(d) = CON_PLAYING;
  switch (mode) {
  case RECON:
    SEND_TO_Q("Reconnecting.\r\n", d);
    act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
    break;
  case USURP:
    SEND_TO_Q("You take over your own body, already in use!\r\n", d);
    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	"$n's body has been taken over by a new spirit!",
	TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s has re-logged in from %s... disconnecting old socket.",
	    GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
    break;
  case UNSWITCH:
    SEND_TO_Q("Reconnecting to unswitched char.", d);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
    break;
  }
  return 1;
}
int new_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  if((!d->character) || (!GET_NAME(d->character)))
    return 0;
  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;
    if (k == d)
      continue;
    if (k->character && GET_NAME(k->character) && !str_cmp(GET_NAME(k->character), GET_NAME(d->character))) {
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", d);
      sprintf(buf, "%s [%s] has re-logged in during creation, aborting create.", GET_NAME(d->character), d->host);
      mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
      STATE(k) = CON_CLOSE;
      STATE(d) = CON_CLOSE;
      return 1;
    }
  }
  return 0;
}
/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  char fname[MAX_STRING_LENGTH];
  int player_i, load_result;
  char tmp_name[MAX_INPUT_LENGTH];
  struct char_file_u tmp_store;
  struct obj_data *o, *next_o;
  struct char_data *temp;
  extern sh_int r_mortal_start_room;
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;
  extern const char *class_menu;
  extern int max_bad_pws;
  sh_int load_room;
  int load_char(char *name, struct char_file_u *char_element);
  int parse_class(char arg);
  void update_pos(struct char_data * victim);	/* fight.c */
  void roll_real_abils(struct char_data * ch);
  
  skip_spaces(&arg);
  switch (STATE(d)) {
  case CON_GET_NAME:		/* wait for input of name */
    if (d->character == NULL) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }
    if (!*arg)
      close_socket(d);
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
	  strlen(tmp_name) > MAX_NAME_LENGTH ||
	  fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
	SEND_TO_Q("Invalid name, please try another.\r\n"
		  "Name: ", d);
	return;
      }
      if ((str_cmp(arg, "guest"))&&((player_i = load_char(tmp_name, &tmp_store)) > -1)) {
	store_to_char(&tmp_store, d->character);
	GET_PFILEPOS(d->character) = player_i;
	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
	  free_char(d->character);
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  CREATE(d->character->player_specials, struct player_special_data, 1);
	  d->character->desc = d;
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  GET_PFILEPOS(d->character) = player_i;
	  sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NAME_CNFRM;
	} else {
	  /* undo it just in case they are set */
	  REMOVE_BIT(PLR_FLAGS(d->character),
		     PLR_WRITING | PLR_MAILING | PLR_CRYO | PLR_BUILDING);
	  SEND_TO_Q("Password: ", d);
	  echo_off(d);
	  d->idle_tics = 0;
	  STATE(d) = CON_PASSWORD;
	}
      } else {
	/* player unknown -- make new character */
	if (!Valid_Name(tmp_name)) {
	  SEND_TO_Q("Invalid name, please try another.\r\n", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}
        if(!(str_cmp(arg, "guest"))) {
          /* make guest char */
          if (isbanned(d->host) >= BAN_NEW) {
            sprintf(buf, "Request for guest char denied from [%s] (siteban)", d->host);
            mudlog(buf, CMP, LVL_ASST, TRUE);
            SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
            STATE(d) = CON_CLOSE;
            return;
          }
          if (restrict_mud) {
            SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
            sprintf(buf, "Request for guest char denied from %s (wizlock)", d->host);
            mudlog(buf, NRM, LVL_ASST, TRUE);
            STATE(d) = CON_CLOSE;
            return;
          }
          sprintf(tmp_name, "guest%d", guest_num++);
          CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
          strcpy(d->character->player.name, CAP(tmp_name));
          strncpy(GET_PASSWD(d->character), CRYPT("tsueg", GET_NAME(d->character)), MAX_PWD_LENGTH);
          *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';
          sprintf(buf, "Your name will be %s.\r\n", d->character->player.name);
          SEND_TO_Q(buf, d);
          SEND_TO_Q("This is a temporary character you can use to check out\r\n", d);
          SEND_TO_Q("the mud and it's different classes. It will be deleted\r\n", d);
          SEND_TO_Q("automaticaly when you quit or lose link. Feel free to\r\n", d);
          SEND_TO_Q("use as many guest characters as you want to check us\r\n", d);
          SEND_TO_Q("out but please don't use more than 2 at one time.\r\n\n", d);
          SEND_TO_Q("What is your sex (M/F)? ", d);
          STATE(d) = CON_QSEX;
        }
        else {
          CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
          strcpy(d->character->player.name, CAP(tmp_name));
          sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
          SEND_TO_Q(buf, d);
          STATE(d) = CON_NAME_CNFRM;
        }
      }
    }
    break;
  case CON_NAME_CNFRM:		/* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
	sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
		GET_NAME(d->character), d->host);
	mudlog(buf, CMP, LVL_ASST, TRUE);
	SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (restrict_mud) {
	SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
	sprintf(buf, "Request for new char %s denied from %s (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_ASST, TRUE);
	STATE(d) = CON_CLOSE;
	return;
      }
/* dupe check by name here so they can't log in multiple of same name for creation */
      if (new_dupe_check(d))
	return;
      SEND_TO_Q("New character.\r\n", d);
      sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      echo_off(d);
      STATE(d) = CON_NEWPASSWD;
    } else if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Okay, what IS it, then? ", d);
      free(d->character->player.name);
      d->character->player.name = NULL;
      STATE(d) = CON_GET_NAME;
    } else {
      SEND_TO_Q("Please type Yes or No: ", d);
    }
    break;
  case CON_PASSWORD:		/* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */
    echo_on(d);    /* turn echo back on */
    if (!*arg)
      close_socket(d);
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
	sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	mudlog(buf, BRF, LVL_HERO, TRUE);
	GET_BAD_PWS(d->character)++;
	save_char(d->character, NOWHERE);
	if (++(d->bad_pws) >= max_bad_pws) {	/* 3 strikes and you're out. */
	  SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
	  STATE(d) = CON_CLOSE;
	} else {
	  SEND_TO_Q("Wrong password.\r\nPassword: ", d);
	  echo_off(d);
	}
	return;
      }
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;
      save_char(d->character, NOWHERE);
      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
		GET_NAME(d->character), d->host);
	mudlog(buf, CMP, LVL_ASST, TRUE);
	return;
      }
      if (GET_LEVEL(d->character) < restrict_mud) {
	SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_ASST, TRUE);
	return;
      }
      /* check and make sure no other copies of this player are logged in */
      if (perform_dupe_check(d))
	return;
      if (PLR_FLAGGED(d->character, PLR_INVSTART)&&(GET_LEVEL(d->character) >= LVL_HERO))
	GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
      if(GET_LEVEL(d->character) < LVL_HERO)
	GET_INVIS_LEV(d->character) = 0;
      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      mudlog(buf, BRF, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
      if (GET_LEVEL(d->character) >= LVL_HERO)
        SEND_TO_Q(imotd, d);
      else
        SEND_TO_Q(motd, d);
      if (load_result) {
	sprintf(buf, "\r\n\r\n\007\007\007"
		"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		CCRED(d->character, C_SPR), load_result,
		(load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	SEND_TO_Q(buf, d);
	GET_BAD_PWS(d->character) = 0;
      }
      SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
    break;
  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_NAME(d->character)), MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';
    SEND_TO_Q("\r\nPlease retype password: ", d);
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;
    break;
  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
		MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      if (STATE(d) == CON_CNFPASSWD)
	STATE(d) = CON_NEWPASSWD;
      else
	STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);
    if (STATE(d) == CON_CNFPASSWD) {
      SEND_TO_Q("What is your sex (M/F)? ", d);
      STATE(d) = CON_QSEX;
    } else {
      save_char(d->character, NOWHERE);
      echo_on(d);
      SEND_TO_Q("\r\nDone.\n\r", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }
    break;
  case CON_QSEX:		/* query sex of new user         */
    switch (*arg) {
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("That is not a sex..\r\n"
		"What IS your sex? ", d);
      return;
      break;
    }
    SEND_TO_Q(class_menu, d);
    STATE(d) = CON_QCLASS;
    break;
  case CON_QCLASS:
    load_result = parse_class(*arg);
    if (load_result == CLASS_UNDEFINED) {
      SEND_TO_Q("\r\nThat's not a class.\r\nClass: ", d);
      return;
    }
    GET_CLASS(d->character) = load_result;
    GET_CLASS_BITVECTOR(d->character) = (1 << load_result);
    GET_NUM_CLASSES(d->character) = 1;
    roll_real_abils(d->character);
    SEND_TO_Q("\r\nYour abilities are:\r\n", d);
    sprintf(buf, "       Strength: %2d/%02d\r\n", GET_STR(d->character), GET_ADD(d->character));
    SEND_TO_Q(buf, d);
    sprintf(buf, "   Intelligence: %2d\r\n", GET_INT(d->character));
    SEND_TO_Q(buf, d);
    sprintf(buf, "         Wisdom: %2d\r\n", GET_WIS(d->character));
    SEND_TO_Q(buf, d);
    sprintf(buf, "      Dexterity: %2d\r\n", GET_DEX(d->character));
    SEND_TO_Q(buf, d);
    sprintf(buf, "   Constitution: %2d\r\n", GET_CON(d->character));
    SEND_TO_Q(buf, d);
    sprintf(buf, "       Charisma: %2d\r\n", GET_CHA(d->character));
    SEND_TO_Q(buf, d);
    SEND_TO_Q("\nKeep these statistics (y/n)? ", d);
    STATE(d)=CON_STAT_CONF;
    break;
  case CON_STAT_CONF:
  case CON_REROLLING:
    switch(*arg) {
    case 'y':
    case 'Y':
      if (GET_PFILEPOS(d->character) < 0)
        GET_PFILEPOS(d->character) = create_entry(GET_NAME(d->character));
      if(STATE(d)==CON_STAT_CONF) {
        init_char(d->character);
      }
      else {
        d->character->next = character_list;
        character_list = d->character;
        if(d->character->was_in_room <= 0)
          d->character->was_in_room=world[r_mortal_start_room].number;
        char_to_room(d->character, real_room(d->character->was_in_room));
        d->character->was_in_room=NOWHERE;
        do_start(d->character);
        STATE(d)=CON_PLAYING;
        send_to_char("Slowly the tingling fades and the world returns. Examining yourself you\r\n"
                     "discover you are now in a different body!\r\n", d->character);
        affect_total(d->character);
        save_char(d->character, NOWHERE);
        act("A column of red mist slowly rises from the floor. It grows denser until\r\n"
            "you cannot see into it. After a few seconds the mist dissipates and an\r\n"
            "unfamiliar person stands before you.", TRUE, d->character, 0, 0, TO_ROOM);
        return;
      }
      save_char(d->character, NOWHERE);
      Crash_crashsave(d->character);
      SEND_TO_Q("\r\n\n", d);
      SEND_TO_Q(motd, d);
      SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
      sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
      mudlog(buf, NRM, LVL_HERO, TRUE);
      break;
    case 'n':
    case 'N':
      roll_real_abils(d->character);
      SEND_TO_Q("\r\n\nYour abilities are:\r\n", d);
      sprintf(buf, "       Strength: %2d/%02d\r\n", GET_STR(d->character), GET_ADD(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "   Intelligence: %2d\r\n", GET_INT(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "         Wisdom: %2d\r\n", GET_WIS(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "      Dexterity: %2d\r\n", GET_DEX(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "   Constitution: %2d\r\n", GET_CON(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "       Charisma: %2d\r\n", GET_CHA(d->character));
      SEND_TO_Q(buf, d);
      SEND_TO_Q("\nKeep these statistics (y/n)? ", d);
      break;
    default:
      SEND_TO_Q("\r\nYou must answer yes or no: ", d);
      break;
    }
    break;
  case CON_RMOTD:		/* read CR after printing motd   */
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_MENU;
    break;
  case CON_MENU:		/* get selection from main menu  */
    switch (*arg) {
    case '0':
      if(d->character->char_specials.spcont) {
        struct spcontinuous *c;
        for(c=d->character->char_specials.spcont; c; c=c->next)
          c->sptimer=0;
        send_to_char("Discontinuing active powers, try again in a few seconds...\r\n", d->character);
        break;
      }
      if(d->character->char_specials.vc >= 0)
        do_vcdepart(d->character, "", 0, 0);
      close_socket(d);
      break;
    case '1':
      reset_char(d->character);
      if (PLR_FLAGGED(d->character, PLR_INVSTART)&&(GET_LEVEL(d->character) >= LVL_HERO))
	GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
      save_char(d->character, NOWHERE);
      send_to_char(WELC_MESSG, d->character);
      d->character->next = character_list;
      character_list = d->character;
      if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
	load_room = real_room(load_room);
      /* If char was saved with NOWHERE, or real_room above failed... */
      if (load_room == NOWHERE) {
	if (GET_LEVEL(d->character) >= LVL_HERO) {
	  load_room = r_immort_start_room;
	} else {
	  load_room = r_mortal_start_room;
	}
      }
      if (PLR_FLAGGED(d->character, PLR_FROZEN))
	load_room = r_frozen_start_room;
      STATE(d) = CON_PLAYING;
      if (!GET_LEVEL(d->character)) {
        if(real_room(4)!=NOWHERE)
          char_to_room(d->character, real_room(4));
        else
          char_to_room(d->character, load_room);
        if(!d->character->player_specials->saved.rerolling) {
          do_start(d->character);
          send_to_char(START_MESSG, d->character);
          SET_BIT(PRF_FLAGS(d->character), PRF_MORTLOG);
        }
      }
      else {
        char_to_room(d->character, load_room);
      }
      if(!d->character->player_specials->saved.rerolling) {
        act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
        look_at_room(d->character, 0);
      }
      load_result = Crash_load(d->character);
      for(o=d->character->carrying; o; o=next_o) {
        next_o=o->next_content;
        if(IS_SET(GET_OBJ_EXTRA(o), ITEM_NORENT) || (GET_OBJ_TYPE(o) == ITEM_KEY)) {
          extract_obj(o);
        }
      }
      if (has_mail(GET_IDNUM(d->character)))
	send_to_char("You have mail waiting.\r\n", d->character);
      if (load_result == 2) {	/* rented items lost */
	send_to_char("\r\n\007You could not afford your rent!\r\n"
	  "Your possesions have been donated to the Salvation Army!\r\n",
		     d->character);
      }
      update_pos(d->character);
      if(d->character->player_specials->saved.rerolling) {
        char_from_room(d->character);
        REMOVE_FROM_LIST(d->character, character_list, next);
        STATE(d)=CON_REROLLING;
        roll_real_abils(d->character);
        SEND_TO_Q("\r\nYour abilities are:\r\n", d);
        sprintf(buf, "       Strength: %2d/%02d\r\n", GET_STR(d->character), GET_ADD(d->character));
        SEND_TO_Q(buf, d);
        sprintf(buf, "   Intelligence: %2d\r\n", GET_INT(d->character));
        SEND_TO_Q(buf, d);
        sprintf(buf, "         Wisdom: %2d\r\n", GET_WIS(d->character));
        SEND_TO_Q(buf, d);
        sprintf(buf, "      Dexterity: %2d\r\n", GET_DEX(d->character));
        SEND_TO_Q(buf, d);
        sprintf(buf, "   Constitution: %2d\r\n", GET_CON(d->character));
        SEND_TO_Q(buf, d);
        sprintf(buf, "       Charisma: %2d\r\n", GET_CHA(d->character));
        SEND_TO_Q(buf, d);
        SEND_TO_Q("\nKeep these statistics (y/n)? ", d);
        return;
      }
      d->prompt_mode = 1;
      break;
    case '2':
      SEND_TO_Q("Enter the text you'd like others to see when they look at you.\r\n", d);
      SEND_TO_Q("Terminate with a '@' on a new line.\r\n", d);
      if (d->character->player.description) {
	SEND_TO_Q("Old description:\r\n", d);
	SEND_TO_Q(d->character->player.description, d);
	free(d->character->player.description);
	d->character->player.description = NULL;
      }
      d->str = &d->character->player.description;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_EXDESC;
      break;
    case '3':
      page_string(d, background, 0);
      STATE(d) = CON_RMOTD;
      break;
    case '4':
      SEND_TO_Q("\r\nEnter your old password: ", d);
      echo_off(d);
      STATE(d) = CON_CHPWD_GETOLD;
      break;
    case '5':
      SEND_TO_Q("\r\nEnter your password for verification: ", d);
      echo_off(d);
      STATE(d) = CON_DELCNF1;
      break;
    default:
      SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
      SEND_TO_Q(MENU, d);
      break;
    }
    break;
  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      echo_on(d);
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
      return;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    break;
  case CON_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    } else {
      SEND_TO_Q("Please take a moment to tell us why you are deleting,\r\n"
                "especially if you intend to leave the mud permanently.\r\n", d);
      SEND_TO_Q("Terminate with a '@' on a new line.\r\n", d);
      if (d->character->player.description) {
	free(d->character->player.description);
	d->character->player.description = NULL;
      }
      d->str = &d->character->player.description;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_DELMSG;
    }
    break;
  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
	SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
	SEND_TO_Q("Character not deleted.\r\n\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (GET_LEVEL(d->character) < LVL_CIMP)
	SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
      save_char(d->character, NOWHERE);
      Crash_delete_file(GET_NAME(d->character));
      alias_delete(GET_NAME(d->character));
      if (get_filename(GET_NAME(d->character), fname, SAVEMAIL_FILE))
        unlink(fname);
      sprintf(buf, "Character '%s' deleted!\r\n"
	      "Goodbye.\r\n", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "%s (lev %d) has self-deleted because %s", GET_NAME(d->character),
	      GET_LEVEL(d->character), d->character->player.description);
      mudlog(buf, NRM, LVL_HERO, TRUE);
      STATE(d) = CON_CLOSE;
      return;
    } else {
      if (d->character->player.description) {
	free(d->character->player.description);
	d->character->player.description = str_dup("");
      }
      SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }
    break;
  case CON_CLOSE:
    close_socket(d);
    break;
  default:
    log("SYSERR: Nanny: illegal state of con'ness; closing connection");
    close_socket(d);
    break;
  }
}
