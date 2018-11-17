/* ************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
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

#include <sys/stat.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern char *class_abbrevs[];
extern int arena_restrict;


ACMD(do_quit)
{
  int canquit=1;
  void die(struct char_data * ch, struct char_data *killer);
  int Crash_quitrentsave(struct char_data * ch, int cost);
  int Crash_reportrentsave(struct char_data * ch, int cost);
  int Crash_forcerentsave(struct char_data * ch, int cost);
  extern int free_rent;
  struct descriptor_data *d, *next_d;

  if (IS_NPC(ch) || !ch->desc)
    return;

  if ((subcmd != SCMD_QUIT) && (subcmd != SCMD_QQUIT) && (subcmd != SCMD_RQUIT) &&
      (GET_LEVEL(ch) < LVL_HERO))
    send_to_char("You have to type quit--no less, to quit!\r\n", ch);
  else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
  else if ((ROOM_FLAGGED(ch->in_room, ROOM_NOQUIT)) && (GET_LEVEL(ch) < LVL_HERO))
    send_to_char("You may not quit in this room.\r\n", ch);
  else if (GET_POS(ch) < POS_STUNNED) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch, ch);
  } else {
    if(ARENA(ch).kill_mode) {
      send_to_char("You cannot quit during an arena challenge.\r\n", ch);
      return;
    }
    if(ch->player_specials->zone_locked>0) {
      send_to_char("You cannot quit while you have a zone locked.\r\n", ch);
      return;
    }
    if(GET_OLC_MODE(ch)) {
      send_to_char("You cannot quit while you are editting.\r\n", ch);
      return;
    }
    if(ch->char_specials.spcont) {
      send_to_char("You cannot quit while you have continuous affects.\r\n", ch);
      return;
    }
    if (free_rent) {
      if(subcmd == SCMD_QQUIT)
        Crash_forcerentsave(ch, 0);
      else if(subcmd == SCMD_RQUIT)
        canquit=Crash_reportrentsave(ch, 0);
      else
        canquit=Crash_quitrentsave(ch, 0);
    }
    if(canquit) {
      if (!GET_INVIS_LEV(ch))
        act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
      if(subcmd == SCMD_QQUIT)
        sprintf(buf, "%s force-rented and extracted (qquit).", GET_NAME(ch));
      else
        sprintf(buf, "%s has quit the game.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
      send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);

      /*
       * kill off all sockets connected to the same player as the one who is
       * trying to quit.  Helps to maintain sanity as well as prevent duping.
       */
      for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        if (d == ch->desc)
          continue;
        if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
          close_socket(d);
      }
      extract_char(ch);		/* Char is saved in extract char */
    }
  }
}



ACMD(do_save)
{
  int reimb_make(struct char_data *ch);

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (cmd) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
    one_argument(argument, arg);
    if((*arg) && (!strn_cmp("reimb", arg, strlen(arg)))) {
      reimb_make(ch);
      send_to_char("Reimb file updated.\r\n", ch);
    }
  }
  save_char(ch, NOWHERE);
  Crash_crashsave(ch);
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}


ACMD(do_beam)
{
  int i, up_ok=1;
  char arg[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct obj_data *o;

  for(o=world[ch->in_room].contents; o; o=o->next_content) {
    if(GET_OBJ_TYPE(o)==ITEM_BEAMER)
      break;
  }
  if(!o) {
    up_ok=0;
    if(!((o=GET_EQ(ch, WEAR_HOLD)) && (GET_OBJ_TYPE(o)==ITEM_BEAMER))) {
      for(o=ch->carrying; o; o=o->next_content) {
        if(GET_OBJ_TYPE(o)==ITEM_BEAMER)
          break;
      }
    }
  }
  if(!o) {
    send_to_char("Don't you think you need a teleporting device for that?\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  argument=one_argument(argument, arg);
  if(str_cmp(arg, "up") && str_cmp(arg, "down")) {
    send_to_char("You may only use a teleporter to beam UP or DOWN.\r\n", ch);
    return;
  }

  if(!str_cmp(arg, "down")) {
    if(((i=real_room(GET_OBJ_VAL(o, 0))) < 0) || (!world[i].zone)){
      send_to_char("This transporter seems to be malfunctioning.\r\n", ch);
      sprintf(buf, "WARNING: %s [%d] has invalid beamer location", o->short_description, GET_OBJ_VNUM(o));
      mudlog(buf, NRM, LVL_ASST, TRUE);
      return;
    }
    send_to_char("The teleporter starts to hum...\r\nSuddenly you find yourself in a different place.\r\n", ch);
    act("$n activates the teleporter and vanishes.", TRUE, ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, i);
    act("$n materializes in the middle of the room.", TRUE, ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    return;
  }
  else {
    if(!up_ok) {
      send_to_char("Sorry, this device is one way only.\r\n", ch);
      return;
    }
    skip_spaces(&argument);
    one_argument(argument, arg);
    tch=get_player_vis(ch, arg);
    if(!tch) {
      send_to_char("Cannot find specified target.\r\n", ch);
      return;
    }
    if(tch==ch) {
      send_to_char("You can't beam yourself up.\r\n", ch);
      return;
    }
    if((world[tch->in_room].zone == world[ch->in_room].zone) && (ROOM_FLAGGED(tch->in_room, ROOM_TRANSPORT_OK))) {
      act("$n disappears suddenly.", TRUE, tch, NULL, NULL, TO_ROOM);
      send_to_char("You blink and find yourself in a different room!\r\n", tch);
      act("$n activates the teleporter and $N appears.", FALSE, ch, NULL, tch, TO_ROOM);
      sprintf(buf, "You activate the teleporter and %s appears.", GET_NAME(tch));
      send_to_char(buf, ch);
      char_from_room(tch);
      char_to_room(tch, ch->in_room);
    }
    else {
      sprintf(buf, "The teleporter fails, %s is too far away.\r\n", GET_NAME(tch));
      send_to_char(buf, ch);
    }
  }
}


ACMD(do_practice)
{
  void list_skills(struct char_data * ch);

  one_argument(argument, arg);

  if (*arg)
    send_to_char("You can only practice skills in your guild.\r\n", ch);
  else
    list_skills(ch);
}



ACMD(do_visible)
{
  void appear(struct char_data * ch);
  void perform_immort_vis(struct char_data *ch);

  if (GET_LEVEL(ch) >= LVL_HERO) {
    perform_immort_vis(ch);
    return;
  }

  if IS_AFFECTED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  } else
    send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if ((strstr(argument, "(") || strstr(argument, ")")) && (GET_LEVEL(ch) < LVL_ASST))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_TITLE_LENGTH) {
    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
	    MAX_TITLE_LENGTH);
    send_to_char(buf, ch);
  } else {
    set_title(ch, argument);
    sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
    send_to_char(buf, ch);
  }
}


int perform_group(struct char_data *ch, struct char_data *vict)
{
  if (IS_AFFECTED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return 0;

  SET_BIT(AFF_FLAGS(vict), AFF_GROUP);
  if (ch != vict)
    act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
  act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
  return 1;
}


void print_group(struct char_data *ch)
{
  struct char_data *k;
  struct follow_type *f;

  char *print_class_levels(struct char_data *ch);

  if (!IS_AFFECTED(ch, AFF_GROUP))
    send_to_char("But you are not the member of a group!\r\n", ch);
  else {
    send_to_char("Your group consists of:\r\n", ch);

    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP)) {
      sprintf(buf, "  %s [%3d/%3dH %3d/%3dM %3d/%3dV] [%s] $N (Head of group)",
              (IS_NPC(k)?(MOB_FLAGGED(k, MOB_NOSUMMON)?"S":" "):((PRF_FLAGGED(k, PRF_SUMMONABLE) ? " " : "S"))),
              GET_HIT(k), GET_MAX_HIT(k), GET_MANA(k), GET_MAX_MANA(k), GET_MOVE(k),
              GET_MAX_MOVE(k), print_class_levels(k));
      act(buf, FALSE, ch, 0, k, TO_CHAR | TO_SLEEP);
    }

    for (f = k->followers; f; f = f->next) {
      if (!IS_AFFECTED(f->follower, AFF_GROUP))
	continue;

      sprintf(buf, "  %s [%3d/%3dH %3d/%3dM %3d/%3dV] [%s] $N",
              (IS_NPC(f->follower)?(MOB_FLAGGED(f->follower, MOB_NOSUMMON)?"S":" "):((PRF_FLAGGED(f->follower, PRF_SUMMONABLE) ? " " : "S"))),
	      GET_HIT(f->follower), GET_MAX_HIT(f->follower), GET_MANA(f->follower), GET_MAX_MANA(f->follower),
	      GET_MOVE(f->follower), GET_MAX_MOVE(f->follower), print_class_levels(f->follower));
      act(buf, FALSE, ch, 0, f->follower, TO_CHAR | TO_SLEEP);
    }
  }
}



ACMD(do_group)
{
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }

  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char("Everyone following you is already in your group.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!IS_AFFECTED(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
	act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT(AFF_FLAGS(vict), AFF_GROUP);
      if(ARENA(vict).challenged_by == -1)
        ARENA(vict).challenged_by=0;
      if(ARENA(vict).challenging==(GET_IDNUM(ch->master?ch->master:ch)+1))
        ARENA(vict).challenging=0;
    }
  }
}



ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  struct char_data *tch;
  void stop_follower(struct char_data * ch);

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }
    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (IS_AFFECTED(f->follower, AFF_GROUP)) {
        if(ARENA(f->follower).challenged_by == -1)
          ARENA(f->follower).challenged_by=0;
        if(ARENA(f->follower).challenging==(GET_IDNUM(ch)+1))
          ARENA(f->follower).challenging=0;
	REMOVE_BIT(AFF_FLAGS(f->follower), AFF_GROUP);
	send_to_char(buf2, f->follower);
        if (!IS_AFFECTED(f->follower, AFF_CHARM))
	  stop_follower(f->follower);
      }
    }

    REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
    send_to_char("You disband the group.\r\n", ch);
    if(ARENA(ch).challenged_by == -1)
      ARENA(ch).challenged_by=0;
    if(ARENA(ch).challenging==(GET_IDNUM(ch)+1))
      ARENA(ch).challenging=0;
    return;
  }
  if (!(tch = get_char_room_vis(ch, buf))) {
    send_to_char("There is no such person!\r\n", ch);
    return;
  }
  if (tch->master != ch) {
    send_to_char("That person is not following you!\r\n", ch);
    return;
  }

  if (!IS_AFFECTED(tch, AFF_GROUP)) {
    send_to_char("That person isn't in your group.\r\n", ch);
    return;
  }

  REMOVE_BIT(AFF_FLAGS(tch), AFF_GROUP);

  act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
 
  stop_follower(tch);

  if(ARENA(tch).challenged_by == -1)
    ARENA(tch).challenged_by=0;
  if(ARENA(tch).challenging==(GET_IDNUM(ch)+1))
    ARENA(tch).challenging=0;
}




ACMD(do_report)
{
  struct char_data *k;
  struct follow_type *f;

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are not a member of any group!\r\n", ch);
    return;
  }
  sprintf(buf, "%s reports: %d/%d+%dH, %d/%d+%dM, %d/%d+%dV\r\n",
	  GET_NAME(ch), GET_HIT(ch), GET_MAX_HIT(ch), hit_gain(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch), mana_gain(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch), move_gain(ch));

  CAP(buf);

  k = (ch->master ? ch->master : ch);

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower != ch)
      send_to_char(buf, f->follower);
  if (k != ch)
    send_to_char(buf, k);
  send_to_char("You report to the group.\r\n", ch);
  send_to_char(buf, ch);
}



ACMD(do_split)
{
  int amount, num, share;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room))
	num++;

    if (num && IS_AFFECTED(ch, AFF_GROUP))
      share = amount / num;
    else {
      send_to_char("With whom do you wish to share your gold?\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k)) && k != ch) {
      GET_GOLD(k) += share;
      sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	      amount, share);
      send_to_char(buf, k);
    }
    for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch) {
	GET_GOLD(f->follower) += share;
	sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
		amount, share);
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}



ACMD(do_use)
{
  struct obj_data *mag_item;
  int equipped = 1;

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      equipped = 0;
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_use");
      return;
      break;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char("You can only recite scrolls.", ch);
      return;
    }
    break;
  case SCMD_USE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char("You can't seem to figure out how to use it.\r\n", ch);
      return;
    }
    break;
  }

  mag_objectmagic(ch, mag_item, buf);
}



ACMD(do_wimpy)
{
  int wimp_lev;

  one_argument(argument, arg);

  if (!*arg) {
    if (GET_WIMP_LEV(ch)) {
      sprintf(buf, "Your current wimp level is %d hit points.\r\n",
	      GET_WIMP_LEV(ch));
      send_to_char(buf, ch);
      return;
    } else {
      send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
      return;
    }
  }
  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg))) {
      if (wimp_lev < 0)
	send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
      else if (wimp_lev > GET_MAX_HIT(ch))
	send_to_char("That doesn't make much sense, now does it?\r\n", ch);
      else {
	sprintf(buf, "Okay, you'll wimp out if you drop below %d hit points.\r\n",
		wimp_lev);
	send_to_char(buf, ch);
	GET_WIMP_LEV(ch) = wimp_lev;
      }
    } else {
      send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", ch);
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char("Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n", ch);

  return;

}


ACMD(do_display)
{
/*  size_t i; */

  if (IS_NPC(ch)) {
    send_to_char("Mosters don't need displays.  Go away.\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Current prompt:\r\n", ch);
    send_to_char(ch->char_specials.prompt, ch);
    send_to_char("\r\n", ch);
    return;
  }
  if(strlen(argument)<=MAX_PROMPT_LENGTH)
  {
    if(ch->char_specials.prompt)
      free(ch->char_specials.prompt);
    ch->char_specials.prompt=str_dup(argument);
    send_to_char(OK, ch);
  }
  else
    send_to_char("Prompt too long, it must be 80 characters or less.\r\n", ch);
}



ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, *filename, buf[MAX_STRING_LENGTH];
  struct stat fbuf;
  extern int max_filesize;
  time_t ct;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(buf, CMP, LVL_IMMORT, FALSE);

  if (stat(filename, &fbuf) < 0) {
    perror("Error statting file");
    return;
  }
  if (fbuf.st_size >= max_filesize) {
    send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  fprintf(fl, "%-11s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
	  world[ch->in_room].number, argument);
  fclose(fl);
  send_to_char("Okay.  Thanks!\r\n", ch);
}



#define TOG_OFF 0
#define TOG_ON  1

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_tog)
{
  long long result;
  extern int nameserver_is_slow;

  char *tog_messages[][2] = {
    {"You are now safe from summoning by other players.\r\n",
     "You may now be summoned by other players.\r\n"},
    {"Nohassle disabled.\r\n",
     "Nohassle enabled.\r\n"},
    {"Brief mode off.\r\n",
     "Brief mode on.\r\n"},
    {"Compact mode off.\r\n",
     "Compact mode on.\r\n"},
    {"You can now hear tells.\r\n",
     "You are now deaf to tells.\r\n"},
    {"You can now hear auctions.\r\n",
     "You are now deaf to auctions.\r\n"},
    {"You can now hear shouts.\r\n",
     "You are now deaf to shouts.\r\n"},
    {"You can now hear gossip.\r\n",
     "You are now deaf to gossip.\r\n"},
    {"You can now hear the congratulation messages.\r\n",
     "You are now deaf to the congratulation messages.\r\n"},
    {"You can now hear the Wiz-channel.\r\n",
     "You are now deaf to the Wiz-channel.\r\n"},
    {"You are no longer part of the Quest.\r\n",
     "Okay, you are part of the Quest!\r\n"},
    {"You will no longer see the room flags.\r\n",
     "You will now see the room flags.\r\n"},
    {"You will now have your communication repeated.\r\n",
     "You will no longer have your communication repeated.\r\n"},
    {"HolyLight mode off.\r\n",
     "HolyLight mode on.\r\n"},
    {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
     "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
    {"Exits will be shown automaticaly.\r\n",
     "Exits will no longer be shown automaticaly.\r\n"},
    {"Okay, you can now hear the wonderful singers on this mud.\r\n",
     "You will no longer be tortured by the horrible singers on this mud.\r\n"},
    {"You are now back at the keyboard.\r\n",
     "You are now AFK.\r\n"},
    {"You will no longer hear player deaths and advancements.\r\n",
     "You will now hear player deaths and advancements.\r\n"},
    {"You are now on the assassin channel.\r\n",
     "You are now deaf to the assassin channel.\r\n"},
    {"You are now on the spanish channel.\r\n",
     "You are no longer listening to the spanish channel.\r\n"},
    {"Tu vas ecouter en francais maintenant.\r\n",
     "You are no longer listening to the french channel.\r\n"},
    {"You are now listening to the arena channel.\r\n",
     "You are no longer listening to the arena channel.\r\n"},
    {"You return to full immortality.\r\n",
     "You shed your immortal invulnerability and take on a physical form.\r\n"},
    {"You will no longer see items combined.\r\n",
     "Same items will now be combined.\r\n"},
    {"You will no longer loot gold automaticaly.\r\n",
     "You will automaticaly loot gold.\r\n"},
    {"You will no longer loot automaticaly.\r\n",
     "You will automaticaly loot everything from corpses.\r\n"},
    {"You will no longer split gold automaticaly.\r\n",
     "You will automaticaly split gold.\r\n"},
    {"You will now see olc menus.\r\n",
     "You will no longer see olc menus.\r\n"}
  };


  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_NOSUMMON:
    result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
    break;
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    break;
  case SCMD_NOAUCTION:
    result = PRF_TOG_CHK(ch, PRF_NOAUCT);
    break;
  case SCMD_DEAF:
    result = PRF_TOG_CHK(ch, PRF_DEAF);
    break;
  case SCMD_NOGOSSIP:
    result = PRF_TOG_CHK(ch, PRF_NOGOSS);
    break;
  case SCMD_NOGRATZ:
    result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    break;
  case SCMD_QUEST:
    result = PRF_TOG_CHK(ch, PRF_QUEST);
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    break;
  case SCMD_SLOWNS:
    result = (nameserver_is_slow = !nameserver_is_slow);
    sprintf(buf, "(GC) %s has changed nameserver_is_slow to %s.", GET_NAME(ch),
            (result?"ON":"OFF"));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_NOEXITS);
    break;
  case SCMD_NOMUS:
    result = PRF_TOG_CHK(ch, PRF_NOMUS);
    break;
  case SCMD_AFK:
    result = PRF_TOG_CHK(ch, PRF_AFK);
    if(result)
      act("$n is AFK.", TRUE, ch, NULL, NULL, TO_ROOM);
    else
      act("$n is back at the keyboard.", TRUE, ch, NULL, NULL, TO_ROOM);
    break;
  case SCMD_MORTLOG:
    result = PRF_TOG_CHK(ch, PRF_MORTLOG);
    break;
  case SCMD_NOASAY:
    if((!PLR_FLAGGED(ch, PLR_ASSASSIN)) && (GET_LEVEL(ch) < LVL_ASST)) {
      send_to_char("You aren't even an assassin!\r\n", ch);
      return;
    }
    result = PRF_TOG_CHK(ch, PRF_NOASAY);
    break;
  case SCMD_NOESP:
    result = PRF_TOG_CHK(ch, PRF_NOESP);
    break;
  case SCMD_NOFRAN:
    result = PRF_TOG_CHK(ch, PRF_NOFRAN);
    break;
  case SCMD_NOARENA:
    result = PRF_TOG_CHK(ch, PRF_NOARENA);
    break;
  case SCMD_AVTR:
    if(GET_OLC_MODE(ch)) {
      send_to_char("Not while you're editting!\r\n", ch);
      return;
    }
    result = PRF_TOG_CHK(ch, PRF_AVTR);
    break;
  case SCMD_COMBINE:
    result = PRF_TOG_CHK(ch, PRF_COMBINE);
    break;
  case SCMD_AUTOGOLD:
    result = PRF_TOG_CHK(ch, PRF_AUTOGOLD);
    break;
  case SCMD_AUTOLOOT:
    result = PRF_TOG_CHK(ch, PRF_AUTOLOOT);
    break;
  case SCMD_AUTOSPLIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOSPLIT);
    break;
  case SCMD_NOMENU:
    result = PRF_TOG_CHK(ch, PRF_NOMENU);
    break;
  default:
    log("SYSERR: Unknown subcmd in do_gen_toggle");
    return;
    break;
  }

  if (result)
    send_to_char(tog_messages[subcmd][TOG_ON], ch);
  else
    send_to_char(tog_messages[subcmd][TOG_OFF], ch);

  return;
}


ACMD(do_nodisturb)
{
  long toggle;

  toggle=PRF_TOG_CHK(ch, PRF_NODISTURB);
  if(toggle) {
    send_to_char("You will no longer get messages while building", ch);
    if(GET_OLC_MODE(ch)) {
      SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    }
  }
  else {
    send_to_char("You will now get messages while building", ch);
    if(GET_OLC_MODE(ch)) {
      REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    }
  }
}


ACMD(do_request)
{
  if(IS_NPC(ch)) {
    send_to_char("Go away, you can kill without assassinship.\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if(!*arg) {
    send_to_char("You would like to request something, but what?\r\n", ch);
    return;
  }
  if(!strn_cmp("assassin", arg, strlen(arg))) {
    if(PLR_FLAGGED(ch, PLR_ASSASSIN)) {
      send_to_char("You are already an assassin.\r\n", ch);
      return;
    }
    if(PRF_FLAGGED(ch, PRF_REQ_ASS)) {
      sprintf(buf, "%s has withdraw %s request for assassinship.", GET_NAME(ch), HSHR(ch));
      mudlog(buf, CMP, LVL_ASST, FALSE);
      REMOVE_BIT(PRF_FLAGS(ch), PRF_REQ_ASS);
      send_to_char("You are no longer requesting to be an assassin.\r\n", ch);
    }
    else {
      sprintf(buf, "%s has requested assassinship.", GET_NAME(ch));
      mudlog(buf, CMP, LVL_ASST, FALSE);
      SET_BIT(PRF_FLAGS(ch), PRF_REQ_ASS);
      send_to_char("You are now requesting to be an assassin.\r\n", ch);
    }
  }
  else {
    send_to_char("I don't understand that request.\r\n", ch);
  }
}


int check_ready(struct char_data *check, struct char_data *leader, char **msg)
{
  if(ARENA(check).challenging!=(GET_IDNUM(leader)+1))
    return -1;
  if(zone_table[world[check->in_room].zone].number==ARENA_ZONE) {
    sprintf(*msg, "Cannot continue, %s is already in the arena.", GET_NAME(check));
    return 0;
  }
  if((zone_table[world[check->in_room].zone].number!=30) && (zone_table[world[check->in_room].zone].number!=31)) {
    sprintf(*msg, "Cannot continue, %s is not in midgaard.", GET_NAME(check));
    return 0;
  }
  if(FIGHTING(check)) {
    sprintf(*msg, "Cannot continue, %s is fighting.", GET_NAME(check));
    return 0;
  }
  if(ROOM_FLAGGED(check->in_room, ROOM_NOTELEPORT)) {
    sprintf(*msg, "Cannot continue, %s cannot be pulled from %s room.", GET_NAME(check), HSHR(check));
    return 0;
  }
  if((!check->desc)||(STATE(check->desc)!=CON_PLAYING)) {
    sprintf(*msg, "Cannot continue, %s is linkless or at the main menu.", GET_NAME(check));
    return 0;
  }
  return 1;
}

ACMD(do_accept)
{
  struct char_data *tch;
  struct follow_type *f;
  int i, start_room, ready, overflow=0;
  char *message;
  char buf[MAX_STRING_LENGTH];

  if(IS_NPC(ch)) {
    send_to_char("Only real players can fight in the arena.\r\n", ch);
    return;
  }

  if(zone_table[world[ch->in_room].zone].number==ARENA_ZONE) {
    send_to_char("You cannot accept another challenge while in the arena.\r\n", ch);
    return;
  }

  if((zone_table[world[ch->in_room].zone].number!=30) && (zone_table[world[ch->in_room].zone].number!=31)) {
    send_to_char("You must be in Midgaard to enter the arena.\r\n", ch);
    return;
  }

  if(!ARENA(ch).challenged_by) {
    send_to_char("There is no challenge to accept.\r\n", ch);
    return;
  }

  if(arena_restrict>=ARENA_CLOSED) {
    send_to_char("Sorry, the arena is closed at the moment.\r\n", ch);
    return;
  }

  if(ARENA(ch).challenged_by == -1) {
    if(arena_restrict>=ARENA_NOGROUP) {
      send_to_char("Sorry, the arena is closed to groups right now.\r\n", ch);
      return;
    }
    sprintf(buf, "%s has agreed to group combat.\r\n", GET_NAME(ch));
    if(ch->master) {
      tch=ch->master;
      send_to_char(buf, tch);
    }
    else
      tch=ch;
    ARENA(ch).challenging=GET_IDNUM(tch)+1;
    i=1;
    CREATE(message, char, MAX_INPUT_LENGTH);
    ready=check_ready(tch, tch, &message);
    for(f=tch->followers; f; f=f->next) {
      if(!AFF_FLAGGED(f->follower, AFF_GROUP))
        continue;
      if(IS_NPC(f->follower))
        continue;
      send_to_char(buf, f->follower);
      i++;
      switch(check_ready(f->follower, tch, &message)) {
      case -1:
        ready=-1;
        break;
      case 0:
        if(ready>0)
          ready=0;
        break;
      }      
    }
    if(i<3) {
      send_to_char("You must have at least 3 people to to group combat.\r\n", tch);
      for(f=tch->followers; f; f=f->next) {
        if(!AFF_FLAGGED(f->follower, AFF_GROUP))
          continue;
        if(IS_NPC(f->follower))
          continue;
        send_to_char("You must have at least 3 people to to group combat.\r\n", f->follower);
      }
    }
    if(!ready) {
      send_to_char(message, tch);
      for(f=tch->followers; f; f=f->next) {
        if(!AFF_FLAGGED(f->follower, AFF_GROUP))
          continue;
        if(IS_NPC(f->follower))
          continue;
        send_to_char(message, f->follower);
      }
    }
    else if(ready==1) {
      sprintf(buf, "Group combat has begun for: %s", GET_NAME(tch));
      ARENA(tch).kill_mode=ARENA(tch).challenging;
      ARENA(tch).challenging=0;
      ARENA(tch).challenged_by = -1;
      ARENA(tch).hit=GET_HIT(tch);
      ARENA(tch).mana=GET_MANA(tch);
      ARENA(tch).move=GET_MOVE(tch);
      for(i=0; i<15; i++) {
        if(!world[(start_room=real_room(zone_table[real_zone(ARENA_ZONE)].bottom+number(10, 19)))].people) {
          break;
        }
      }
      act("$n steps through a portal to the arena.", TRUE, tch, NULL, NULL, TO_ROOM);
      char_from_room(tch);
      char_to_room(tch, start_room);
      act("$n appears through a portal.", TRUE, tch, NULL, NULL, TO_ROOM);
      look_at_room(tch, 0);
      for(f=tch->followers; f; f=f->next) {
        if(!AFF_FLAGGED(f->follower, AFF_GROUP))
          continue;
        if(IS_NPC(f->follower))
          continue;
        ARENA(f->follower).kill_mode=ARENA(tch).kill_mode;
        ARENA(f->follower).challenging=0;
        ARENA(f->follower).challenged_by = -1;
        ARENA(f->follower).hit=GET_HIT(f->follower);
        ARENA(f->follower).mana=GET_MANA(f->follower);
        ARENA(f->follower).move=GET_MOVE(f->follower);
        for(i=0; i<15; i++) {
          if(!world[(start_room=real_room(zone_table[real_zone(ARENA_ZONE)].bottom+number(10, 19)))].people) {
            break;
          }
        }
        act("$n steps through a portal to the arena.", TRUE, f->follower, NULL, NULL, TO_ROOM);
        char_from_room(f->follower);
        char_to_room(f->follower, start_room);
        act("$n appears through a portal.", TRUE, f->follower, NULL, NULL, TO_ROOM);
        look_at_room(f->follower, 0);
        if(strlen(buf) > MAX_STRING_LENGTH-100) {
          if(!overflow) {
            overflow=1;
            strcat(buf, ", etc");
          }
        }
        else
          sprintf(buf, "%s, %s", buf, GET_NAME(f->follower));
      }
      arenasay(buf);
    }
    free(message);
  }
  else {
    for(tch=character_list; tch; tch=tch->next) {
      if((!IS_NPC(tch)) && (tch!=ch) && (GET_IDNUM(tch)==(ARENA(ch).challenged_by-1)))
        break;
    }
    if(!tch) {
      send_to_char("Your challenger seems to have left.\r\n", ch);
      ARENA(ch).challenged_by=0;
      return;
    }
    if(zone_table[world[tch->in_room].zone].number==ARENA_ZONE) {
      act("$n is in the arena already, $e will have to re-challenge\r\nyou when $e is finished.", FALSE, tch, NULL, ch, TO_VICT);
      sprintf(buf, "%s accepted your challenge, but cannot fight because you are already\r\nin the arena. Re-challenge when you are out.\r\n", GET_NAME(ch));
      send_to_char(buf, tch);
      return;
    }
    if((zone_table[world[tch->in_room].zone].number!=30) && (zone_table[world[tch->in_room].zone].number!=31)) {
      act("$n is not in Midgaard, $e will have to re-challenge you when $e is.", FALSE, tch, NULL, ch, TO_VICT);
      sprintf(buf, "%s accepted your challenge, but cannot fight because you are\r\nnot in Midgaard. Re-challenge when you are out.\r\n", GET_NAME(ch));
      send_to_char(buf, tch);
      return;
    }
    if(ROOM_FLAGGED(ch->in_room, ROOM_NOTELEPORT)) {
      send_to_char("The fight cannot begin because you cannot be pulled from your room.\r\n", ch);
      return;
    }
    if(FIGHTING(tch)) {
      act("$n is currently fighting.\r\nThe fight can begin when $e finishes and accepts the challenge.", FALSE, tch, NULL, ch, TO_VICT);
      sprintf(buf, "%s has accepted your challenge, but combat cannot begin because you\r\nare fighting. Type 'accept' when you are done to begin.\r\n", GET_NAME(ch));
      send_to_char(buf, tch);
      ARENA(tch).challenged_by=GET_IDNUM(ch)+1;
      ARENA(ch).challenging=GET_IDNUM(ch)+1;
      return;
    }
    if(ROOM_FLAGGED(tch->in_room, ROOM_NOTELEPORT)) {
      act("$n cannot be pulled from $s room.", FALSE, tch, NULL, ch, TO_VICT);
      sprintf(buf, "%s accepted your challenge, but you cannot be pulled from your room.\r\n", GET_NAME(ch));
      send_to_char(buf, tch);
      return;
    }
    if(ARENA(tch).challenging != (GET_IDNUM(ch)+1)) {
      act("$n is no longer challenging you.\r\nIf you still wish to fight, challenge $m.", FALSE, tch, NULL, ch, TO_VICT);
      ARENA(ch).challenged_by=0;
      return;
    }
    if((!tch->desc)||(STATE(tch->desc)!=CON_PLAYING)) {
      sprintf(buf, "%s is linkless or at the menu screen, try again later.\r\n", GET_NAME(tch));
      send_to_char(buf, ch);
      return;
    }
    ARENA(ch).hit=GET_HIT(ch);
    ARENA(ch).mana=GET_MANA(ch);
    ARENA(ch).move=GET_MOVE(ch);
    ARENA(ch).kill_mode=ARENA(ch).challenged_by;
    ARENA(ch).challenged_by=0;
    ARENA(ch).challenging=0;
    ARENA(tch).hit=GET_HIT(tch);
    ARENA(tch).mana=GET_MANA(tch);
    ARENA(tch).move=GET_MOVE(tch);
    ARENA(tch).kill_mode=ARENA(ch).kill_mode;
    ARENA(tch).challenged_by=0;
    ARENA(tch).challenging=0;
    for(i=0; i<5; i++) {
      if((!world[real_room(zone_table[real_zone(ARENA_ZONE)].bottom+i)].people)&&(!world[real_room(zone_table[real_zone(ARENA_ZONE)].bottom+i+5)].people))
        break;
    }
    if(i==5) {
      i=number(0, 4);
    }
    if(number(0, 1)) {
      i=zone_table[real_zone(ARENA_ZONE)].bottom+i;
      start_room=i+5;
    }
    else {
      start_room=zone_table[real_zone(ARENA_ZONE)].bottom+i;
      i=start_room+5;
    }
    act("$n steps through a portal to the arena.", TRUE, ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    act("$n steps through a portal to the arena.", TRUE, tch, NULL, NULL, TO_ROOM);
    char_from_room(tch);
    char_to_room(ch, real_room(i));
    act("$n appears through a portal.", TRUE, ch, NULL, NULL, TO_ROOM);
    look_at_room(ch, 0);
    char_to_room(tch, real_room(start_room));
    act("$n appears through a portal.", TRUE, tch, NULL, NULL, TO_ROOM);
    look_at_room(tch, 0);
    sprintf(buf, "%s has accepted %s's challenge.", GET_NAME(ch), GET_NAME(tch));
    arenasay(buf);
  }
}


ACMD(do_challenge)
{
  struct char_data *tch;
  struct follow_type *f;
  int i, start_room;

  if(IS_NPC(ch)) {
    send_to_char("Only real players can make challenges in the arena.\r\n", ch);
    return;
  }

  if(zone_table[world[ch->in_room].zone].number==ARENA_ZONE) {
    send_to_char("You cannot make additional challenges while in the arena.\r\n", ch);
    return;
  }

  if((zone_table[world[ch->in_room].zone].number!=30) && (zone_table[world[ch->in_room].zone].number!=31)) {
    send_to_char("You must be in Midgaard to enter the arena.\r\n", ch);
    return;
  }

  if(arena_restrict>=ARENA_CLOSED) {
    send_to_char("Sorry, the arena is closed at the moment.\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_NOARENA)) {
    send_to_char("You cannot make challenges when you're off the arena channel.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, argument);
  if(!*argument) {
    send_to_char("usage: challenge <none | player_name | group | all>", ch);
    return;
  }

  if(!str_cmp(argument, "none")) {
    send_to_char("Ok, you are no longer challenging anyone.\r\n", ch);
    ARENA(ch).challenging=0;
    return;
  }
  else if(!str_cmp(argument, "all")) {
    if(arena_restrict>=ARENA_NOALL) {
      send_to_char("Sorry, the arena is closed to challenging all right now.\r\n", ch);
      return;
    }
    if(ROOM_FLAGGED(ch->in_room, ROOM_NOTELEPORT)) {
      send_to_char("You cannot be pulled from your room.\r\n", ch);
      return;
    }
    for(i=0; i<15; i++) { /* Try 15 times to find an empty random room */
      start_room=zone_table[real_zone(ARENA_ZONE)].bottom+number(20, 29);
      if(!world[real_room(start_room)].people)
        break;
    }
    ARENA(ch).hit=GET_HIT(ch);
    ARENA(ch).mana=GET_MANA(ch);
    ARENA(ch).move=GET_MOVE(ch);
    ARENA(ch).kill_mode = -1;
    ARENA(ch).challenging=0;
    ARENA(ch).challenged_by=0;
    act("$n steps through a portal to the arena.", TRUE, ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(start_room));
    act("$n appears through a portal.", TRUE, ch, NULL, NULL, TO_ROOM);
    look_at_room(ch, 0);
    sprintf(buf, "%s is taking on all challengers.", GET_NAME(ch));
    arenasay(buf);
  }
  else if(!str_cmp(argument, "group")) {
    if(arena_restrict>=ARENA_NOGROUP) {
      send_to_char("Sorry, the arena is closed to groups right now.\r\n", ch);
      return;
    }
    if(!AFF_FLAGGED(ch, AFF_GROUP)) {
      send_to_char("You can't start a group challenge, you aren't part of a group!", ch);
    }
    send_to_char("You propose group combat in the arena.\r\n", ch);
    sprintf(buf, "%s has proposed group combat in the arena.\r\n", GET_NAME(ch));
    if(ch->master) {
      tch=ch->master;
      send_to_char(buf, tch);
      if(AFF_FLAGGED(tch, AFF_GROUP)&&(!IS_NPC(tch)))
        i=2;
      else
        i=1;
    }
    else {
      tch=ch;
      i=1;
    }
    ARENA(ch).challenging=GET_IDNUM(tch)+1;
    ARENA(ch).challenged_by = -1;
    ARENA(tch).challenged_by = -1;
    for(f=tch->followers; f; f=f->next) {
      if(ch==f->follower)
        continue;
      if(!AFF_FLAGGED(f->follower, AFF_GROUP))
        continue;
      if(IS_NPC(f->follower))
        continue;
      i++;
      send_to_char(buf, f->follower);
      ARENA(f->follower).challenged_by = -1;
    }
    if(i<3) {
      send_to_char("You must have at least 3 people to to group combat.\r\n", tch);
      for(f=tch->followers; f; f=f->next) {
        if(!AFF_FLAGGED(f->follower, AFF_GROUP))
          continue;
        if(IS_NPC(f->follower))
          continue;
        send_to_char("You must have at least 3 people to to group combat.\r\n", f->follower);
        ARENA(f->follower).challenged_by=0;
      }
      ARENA(ch).challenging=0;
      ARENA(tch).challenged_by=0;
    }
  }
  else {
    tch=get_player_vis(ch, argument);
    if(!tch) {
      send_to_char("There is no such player around.", ch);
      return;
    }
    if(tch==ch) {
      send_to_char("You can't challenge yourself!\r\n", ch);
      return;
    }
    if((!tch->desc)||(STATE(tch->desc)!=CON_PLAYING)) {
      sprintf(buf, "%s is linkless or at the menu screen, try again later.\r\n", GET_NAME(tch));
      send_to_char(buf, ch);
      return;
    }
    if(PRF_FLAGGED(tch, PRF_NOARENA)) {
      act("$n is not on the arena channel, so you cannot challenge $m.", FALSE, tch, NULL, ch, TO_VICT);
      return;
    }
    if(ARENA(tch).kill_mode == -1) {
      sprintf(buf, "%s is currently challenging all in the arena.\r\n", GET_NAME(tch));
      send_to_char(buf, ch);
      return;
    }
    if(ARENA(tch).kill_mode > 0) {
      sprintf(buf, "%s is already fighting in the arena.\r\n", GET_NAME(tch));
      send_to_char(buf, ch);
      return;
    }
    ARENA(tch).challenged_by=GET_IDNUM(ch)+1;
    ARENA(ch).challenging=GET_IDNUM(tch)+1;
    sprintf(buf, "%s has challenged you to combat in the arena.\r\n", GET_NAME(ch));
    send_to_char(buf, tch);
    sprintf(buf, "%s has challenged %s.", GET_NAME(ch), GET_NAME(tch));
    arenasay(buf);
  }
}


ACMD(do_retire)
{
  struct char_data *tch, *other;

  if(IS_NPC(ch)) {
    send_to_char("Only players can fight in the arena, thus only players can retire.\r\n", ch);
    return;
  }

  if(zone_table[world[ch->in_room].zone].number!=ARENA_ZONE) {
    send_to_char("You aren't even in the arena.\r\n", ch);
    return;
  }

  if(!ARENA(ch).kill_mode) {
    send_to_char("You're just here as a spectator, you'll have to walk out.\r\n", ch);
    return;
  }

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
  sprintf(buf, "%s has left the arena.", GET_NAME(ch));
  arenasay(buf);
  if(ARENA(ch).kill_mode != -1) { /* not challenging all */
    if(ARENA(ch).challenged_by) { /* group challenge */
      for(other=NULL, tch=character_list; tch; tch=tch->next) {
        if((tch!=ch) && (ARENA(tch).kill_mode==ARENA(ch).kill_mode)) {
          if(other) {
            break;
          }
          else {
            other=tch;
          }
        }
      }
      if((!tch) && other) {
        sprintf(buf, "%s has won the group contest!", GET_NAME(other));
        arenasay(buf);
        GET_HIT(other)=ARENA(other).hit;
        GET_MANA(other)=ARENA(other).mana;
        GET_MOVE(other)=ARENA(other).move;
        if(GET_HIT(other)>0)
          GET_POS(other)=POS_STANDING;
        else
          update_pos(other);
        char_from_room(other);
        char_to_room(other, real_room(3072));
        act("$n appears through a portal.", TRUE, other, NULL, NULL, TO_ROOM);
        look_at_room(other, 0);
        ARENA(other).kill_mode=0;
        ARENA(other).challenging=0;
        ARENA(other).challenged_by=0;
        ARENA(other).hit=0;
        ARENA(other).mana=0;
        ARENA(other).move=0;
      }
    }
    else { /* personal challenge */
      for(tch=character_list; tch; tch=tch->next) {
        if((tch!=ch) && (ARENA(tch).kill_mode==ARENA(ch).kill_mode)) {
          GET_HIT(tch)=ARENA(tch).hit;
          GET_MANA(tch)=ARENA(tch).mana;
          GET_MOVE(tch)=ARENA(tch).move;
          if(GET_HIT(tch)>0)
            GET_POS(tch)=POS_STANDING;
          else
            update_pos(tch);
          char_from_room(tch);
          char_to_room(tch, real_room(3072));
          act("$n appears through a portal.", TRUE, tch, NULL, NULL, TO_ROOM);
          look_at_room(tch, 0);
          ARENA(tch).kill_mode=0;
          ARENA(tch).challenging=0;
          ARENA(tch).challenged_by=0;
          ARENA(tch).hit=0;
          ARENA(tch).mana=0;
          ARENA(tch).move=0;
          sprintf(buf, "%s has triumphed over %s!", GET_NAME(tch), GET_NAME(ch));
          arenasay(buf);
          break;
        }
      }
    }
  }
  ARENA(ch).kill_mode=0;
  ARENA(ch).challenging=0;
  ARENA(ch).challenged_by=0;
  ARENA(ch).hit=0;
  ARENA(ch).mana=0;
  ARENA(ch).move=0;
  return;
}

ACMD(do_pagelen)
{
  int length;

  one_argument(argument, arg);

  if(!*arg) {
    if(PAGE_LEN(ch))
      sprintf(buf, "Page length: %d\r\n", PAGE_LEN(ch));
    else
      strcpy(buf, "Page length: DEFAULT\r\n");
    send_to_char(buf, ch);
    return;
  }

  length=atoi(arg);
  if((!is_number(arg)) || (length < 0) || (length > 200)) {
    send_to_char("Valid values are 0-200. 0 will use the default length.\r\n", ch);
    return;
  }

  PAGE_LEN(ch)=length;
  send_to_char(OK, ch);
}
