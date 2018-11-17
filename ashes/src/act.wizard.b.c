/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "class.h"

/*   external vars  */
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct char_data *combat_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[NUM_CLASSES][LVL_IMPL + 1];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[];
extern struct wis_app_type wis_app[];
extern int top_of_zone_table;
extern int restrict_mud;
extern int top_of_world;
extern int top_of_mobt;
extern int top_of_objt;
extern int top_of_p_table;
extern int arena_restrict;
extern char *spec_proc_names[];

/* for objects */
extern char *item_types[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *container_bits[];
extern char *drinks[];
extern char *damtypes[];
extern char *weapon_types[];

/* for rooms */
extern char *dirs[];
extern char *room_bits[];
extern char *exit_bits[];
extern char *sector_types[];

/* for chars */
extern char *spells[];
extern char *equipment_types[];
extern char *affected_bits[];
extern char *apply_types[];
extern char *pc_class_types[];
extern char *npc_class_types[];
extern char *action_bits[];
extern char *player_bits[];
extern char *grant_bits[];
extern char *preference_bits[];
extern char *position_types[];
extern char *connected_types[];

struct char_data *get_soc_mes(struct char_data *ch, char *argument, char *act_ch, char *act_world, char *act_tch);


ACMD(do_award)
{
  int qp, is_file=0, player_i = 0;
  struct char_file_u tmp_store;
  struct char_data *vict, *cbuf=NULL;

  skip_spaces(&argument);
  argument=one_argument(argument, arg);
  argument=one_argument(argument, buf);

  if(!str_cmp(arg, "file")) {
    is_file=TRUE;
    strcpy(arg, buf);
    one_argument(argument, buf);
  }

  if((!*arg) || (!*buf) || (!is_number(buf))) {
    send_to_char("Usage: award [file] <name> <# of qp>", ch);
    return;
  }

  if(is_file) {
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    if ((player_i = load_char(arg, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      if ((GET_LEVEL(cbuf) >= GET_LEVEL(ch)) && str_cmp(GET_NAME(ch), GET_NAME(cbuf))) {
	free_char(cbuf);
        send_to_char("You can't do that!\r\n", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  else if (!(vict = get_player_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }

  qp=atoi(buf);
  qp=MIN(MAX(qp, -1000), 1000);
  if(GET_QP(vict)+qp > 30000)
    qp=30000-GET_QP(vict);
  if(GET_QP(vict)+qp < 0)
    qp = -GET_QP(vict);

  if((GET_LEVEL(vict) < GET_LEVEL(ch)) || (vict==ch)) {
    GET_QP(vict) += qp;
    if(qp >= 0) {
      sprintf(buf, "(GC) %s awarded %d quest points to %s", GET_NAME(ch), qp, GET_NAME(vict));
      sprintf(buf1, "%d quest points awarded to %s.\r\n", qp, GET_NAME(vict));
      sprintf(buf2, "You have been awarded %d quest points.\r\n", qp);
    }
    else {
      sprintf(buf, "(GC) %s took %d quest points away from %s", GET_NAME(ch), -qp, GET_NAME(vict));
      sprintf(buf1, "%d quest points taken from %s.\r\n", -qp, GET_NAME(vict));
      sprintf(buf2, "%d quest points have been taken from you.\r\n", qp);
    }
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    send_to_char(buf1, ch);
    send_to_char(buf2, vict);
  }
  else {
    send_to_char("You can't do that!\r\n", ch);
  }

  if(is_file) {
    char_to_store(vict, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free(cbuf);
  }
}

ACMD(do_autoreboot)
{
  extern int auto_reboot;
  extern int server_boot;

  if(server_boot) {
    send_to_char("You can't change the reboot, THE SERVER IS GOING DOWN.\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if(!str_cmp(arg, "now")) {
    if(auto_reboot < 2) {
      auto_reboot=2;
      send_to_char("Auto-rebooting now.\r\n", ch);
      sprintf(buf, "(GC) %s started auto-reboot.", GET_NAME(ch));
    }
  }
  else if(auto_reboot) {
    auto_reboot=0;
    send_to_char("Auto-reboot is now off.\r\n", ch);
    sprintf(buf, "(GC) %s turned off auto-reboot.", GET_NAME(ch));
  }
  else {
    auto_reboot=1;
    send_to_char("Auto-reboot is now on.\r\n", ch);
    sprintf(buf, "(GC) %s turned on auto-reboot.", GET_NAME(ch));
  }

  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);

  return;
}

ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes.. but what?\r\n", ch);
  else {
    if (subcmd == SCMD_EMOTE)
      sprintf(buf, "$n %s", argument);
    else
      strcpy(buf, argument);
    MOBTrigger=FALSE;
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      MOBTrigger=FALSE;
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
  }
}


ACMD(do_send)
{
  struct char_data *vict;

  delete_doubledollar(argument);

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
sh_int find_target_room(struct char_data * ch, char *rawroomstr)
{
  int tmp;
  sh_int location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return NOWHERE;
  }
  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("That object is not available.\r\n", ch);
      return NOWHERE;
    }
  } else {
    send_to_char("No such creature or object around.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (GET_LEVEL(ch) < LVL_ASST) {
    if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
	world[location].people && world[location].people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return NOWHERE;
    }
  }
  return location;
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  int location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("What do you want to do there?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  sh_int location;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (POOFOUT(ch))
    sprintf(buf, "$n %s", POOFOUT(ch));
  else
    strcpy(buf, "$n disappears in a puff of smoke.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);

  if (POOFIN(ch))
    sprintf(buf, "$n %s", POOFIN(ch));
  else
    strcpy(buf, "$n appears with an ear-splitting bang.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}



ACMD(do_trans)
{
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf)))
      send_to_char(NOPERSON, ch);
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char("Go transfer someone your own size.\r\n", ch);
	return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);
      sprintf(buf, "(GC) %s has transferred %s to %s [%d].", GET_NAME(ch),
              GET_NAME(victim), world[ch->in_room].name, world[ch->in_room].number);
      mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    }
  } else {			/* Trans All */
    if (GET_LEVEL(ch) < LVL_CIMP) {
      send_to_char("I think not.\r\n", ch);
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && i->character != ch) {
	victim = i->character;
	if (GET_LEVEL(victim) >= GET_LEVEL(ch))
	  continue;
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, ch->in_room);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	look_at_room(victim, 0);
      }
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s has transferred everyone to %s [%d].", GET_NAME(ch),
            world[ch->in_room].name, world[ch->in_room].number);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}



ACMD(do_teleport)
{
  struct char_data *victim;
  sh_int target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    send_to_char(OK, ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
    sprintf(buf, "(GC) %s has teleported %s to %s [%d].", GET_NAME(ch),
            GET_NAME(victim), world[victim->in_room].name, world[victim->in_room].number);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}



ACMD(do_vnum)
{
  argument=two_arguments(argument, buf, buf2);
  one_argument(argument, arg);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj") &&
      !is_abbrev(buf, "room") && !is_abbrev(buf, "zone"))) {
    send_to_char("Usage: vnum { obj | mob | room | zone } <name> [exact]\r\n", ch);
    return;
  }

  if(!*arg || strn_cmp("exact", arg, strlen(arg))) {
    if (is_abbrev(buf, "mob"))
      if (!vnum_mobile(buf2, ch, FALSE))
        send_to_char("No mobiles by that name.\r\n", ch);

    if (is_abbrev(buf, "obj"))
      if (!vnum_object(buf2, ch, FALSE))
        send_to_char("No objects by that name.\r\n", ch);

    if (is_abbrev(buf, "room"))
      if (!vnum_room(buf2, ch, FALSE))
        send_to_char("No rooms by that name.\r\n", ch);

    if (is_abbrev(buf, "zone"))
      if (!vnum_zone(buf2, ch, FALSE))
        send_to_char("No zones by that name.\r\n", ch);
  }
  else {
    if (is_abbrev(buf, "mob"))
      if (!vnum_mobile(buf2, ch, TRUE))
        send_to_char("No mobiles by that name.\r\n", ch);

    if (is_abbrev(buf, "obj"))
      if (!vnum_object(buf2, ch, TRUE))
        send_to_char("No objects by that name.\r\n", ch);

    if (is_abbrev(buf, "room"))
      if (!vnum_room(buf2, ch, TRUE))
        send_to_char("No rooms by that name.\r\n", ch);

    if (is_abbrev(buf, "zone"))
      if (!vnum_zone(buf2, ch, TRUE))
        send_to_char("No zones by that name.\r\n", ch);
  }
}



void do_stat_room(struct char_data * ch, int room_num)
{
  struct extra_descr_data *desc;
  struct room_data *rm = &world[room_num];
  int i, found = 0;
  struct obj_data *j = 0;
  struct char_data *k = 0;

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
	  zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
          CCNRM(ch, C_NRM), room_num, buf2);
  send_to_char(buf, ch);

  sprintbit((long) rm->room_flags, room_bits, buf2);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
	  (rm->func == NULL) ? "None" : "Exists", buf2);
  send_to_char(buf, ch);

  if(ROOM_FLAGGED(room_num, ROOM_DEATH)) {
    sprintf(buf, "DT: %dd%d+%d or %d%%%s\r\n", rm->dt_numdice, rm->dt_sizedice,
            rm->dt_add, rm->dt_percent, (rm->dt_repeat ? ", continuous damage" : ""));
    send_to_char(buf, ch);
  }

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
		world[rm->dir_option[i]->to_room].number, CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	   rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
    }
  }
}



void do_stat_object(struct char_data * ch, struct obj_data * j)
{
  int i, virtual, found;
  struct obj_data *j2;
  struct extra_descr_data *desc;

  virtual = GET_OBJ_VNUM(j);
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
  if (GET_OBJ_RNUM(j) >= 0)
    strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
   CCGRN(ch, C_NRM), virtual, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf2);
  send_to_char(buf, ch);
  sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Can be worn on  : ", ch);
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits   : ", ch);
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Extra flags     : ", ch);
  sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Immunity flags  : ", ch);
  sprintbit(j->obj_flags.immune, damtypes, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Resistance flags: ", ch);
  sprintbit(j->obj_flags.resist, damtypes, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Weakness flags  : ", ch);
  sprintbit(j->obj_flags.weak, damtypes, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j));
  send_to_char(buf, ch);

  sprintf(buf, "Item ID: [%ld]\r\n", j->id);
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%d", world[j->in_room].number);
    strcat(buf, buf2);
  }
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Worn by: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "Hours left: Infinite");
    else
      sprintf(buf, "Hours left: [%d]", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "Spells: (Level %d) %s, %s, %s", GET_OBJ_VAL(j, 0),
	    skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
	    skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "Spell: %s at level %d, %d (of %d) charges remaining",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    sprinttype(GET_OBJ_VAL(j, 3), weapon_types, buf2);
    sprintf(buf, "Todam: %dd%d, Type: %s",
	    GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), buf2);
    break;
  case ITEM_ARMOR:
    sprintf(buf, "AC-apply: [%d]", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    sprintf(buf, "Spell: %d, - Hitpoints: %d",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "Weight capacity: %d, Lock Type: %s, Key Num: %d, Corpse: %s",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "Capacity: %d, Contains: %d, Poisoned: %s, Liquid: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
	    buf2);
    break;
  case ITEM_NOTE:
    sprintf(buf, "Tongue: %d", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    strcpy(buf, "");
    break;
  case ITEM_FOOD:
    sprintf(buf, "Makes full: %d, Poisoned: %s", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "Coins: %d", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_BEAMER:
    sprintf(buf, "Beam down room: %s [%d]", (real_room(GET_OBJ_VAL(j, 0)) >= 0) ? world[real_room(GET_OBJ_VAL(j, 0))].name : "NOWHERE", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_DAMAGEABLE:
    sprintf(buf, "Durability: %d, Condition: %d", GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 2));
    break;
  default:
    sprintf(buf, "Values 0-3: [%d] [%d] [%d] [%d]",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  if (j->contains) {
    sprintf(buf, "\r\nContents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);

  send_to_char("\r\n", ch);
}


void do_stat_character(struct char_data * ch, struct char_data * k)
{
  int i, i2, found = 0, thac0;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  extern struct attack_hit_type attack_hit_text[];
  extern struct str_app_type str_app[];

  thac0 = class_get_thac0(k);
  thac0 -= str_app[STRENGTH_APPLY_INDEX(k)].tohit;
  thac0 -= 5*GET_HITROLL(k);
  thac0 -= (int) ((GET_INT(k) - 13) / 1.5);	/* Intelligence helps! */
  thac0 -= (int) ((GET_WIS(k) - 13) / 1.5);	/* So does wisdom */
  thac0 *= -1;

  switch (GET_SEX(k)) {
  case SEX_NEUTRAL:    strcpy(buf, "NEUTRAL-SEX");   break;
  case SEX_MALE:       strcpy(buf, "MALE");          break;
  case SEX_FEMALE:     strcpy(buf, "FEMALE");        break;
  default:             strcpy(buf, "ILLEGAL-SEX!!"); break;
  }

  sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%5d]\r\n",
	  (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	  GET_NAME(k), GET_IDNUM(k), world[k->in_room].number);
  send_to_char(strcat(buf, buf2), ch);
  if (IS_MOB(k)) {
    sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
	    k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));
  send_to_char(buf, ch);

  sprintf(buf, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    strcpy(buf, "Monster Class: ");
    sprinttype(k->player.class, npc_class_types, buf2);
  } else {
    strcpy(buf, "Class: ");
    sprinttype(k->player.class, pc_class_types, buf2);
  }
  strcat(buf, buf2);

  sprintf(buf2, ", Lev: [%s%3d%s], XP: [%s%9ld%s], Align: [%4d]\r\n",
	  CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	  CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
	  GET_ALIGNMENT(k));
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';

    sprintf(buf, "Created: [%s], Last Logon: [%s], Played [%ldh %ldm], Age [%d]\r\n",
	    buf1, buf2, k->player.time.played / 3600,
	    ((k->player.time.played / 60) % 60), age(k).year);
    send_to_char(buf, ch);

    sprintf(buf, "Multiclass levels:");
    for(i=0; i<NUM_CLASSES; i++) {
      sprintf(buf1, " %s:[%d]", class_abbrevs[i], GET_CLASS_LEVEL(k, i));
      strcat(buf, buf1);
      if(i==4)
        strcat(buf, "\r\n");
    }
    send_to_char(strcat(buf, "\r\n"), ch);

    sprintf(buf, "Natural ac:[%d], Unnatural age:[%+d], Rerolls:[%d]\r\n",
            k->player_specials->saved.inherent_ac_apply,
            k->player_specials->saved.age_add,
            k->player_specials->saved.num_rerolls);
    send_to_char(buf, ch);

    sprintf(buf, "Hometown: [%d], Speaks: [%d/%d/%d], Pracs: [%d], Needed to multiclass: [%d]\r\n",
	 k->player.hometown, GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2),
	    GET_PRACTICES(k), (GET_NUM_CLASSES(k)*k->player_specials->saved.extra_pracs)/(GET_NUM_CLASSES(k)+1));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Str: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	  "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]\r\n",
	  CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Real abils: Str:[%s%s%d/%d%s] Int:[%s%s%d%s] Wis:[%s%s%d%s] Dex:[%s%s%d%s] Con:[%s%s%d%s] Cha:[%s%s%d%s]\r\n",
	  CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), k->real_abils.str, k->real_abils.str_add, CCNRM(ch, C_NRM),
	  CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), k->real_abils.intel, CCNRM(ch, C_NRM),
	  CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), k->real_abils.wis, CCNRM(ch, C_NRM),
	  CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), k->real_abils.dex, CCNRM(ch, C_NRM),
	  CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), k->real_abils.con, CCNRM(ch, C_NRM),
	  CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), k->real_abils.cha, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Hit p.:[%s%d/%d+%d%s]  Mana p.:[%s%d/%d+%d%s]  Move p.:[%s%d/%d+%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Coins: [%9ld], Bank: [%9ld] (Total: %ld)\r\n",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));
  send_to_char(buf, ch);

  sprintf(buf, "AC: [%3d], THAC0: [%3d], Hitroll: [%2d], Damroll: [%2d], QP: [%d]\r\n",
	  compute_ac(k), thac0, k->points.hitroll, k->points.damroll, GET_QP(k));
  send_to_char(buf, ch);

  sprintf(buf, "Magic Resistance: [%d], Psionic Resistance: [%d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
          GET_MR(k), GET_PR(k), GET_SAVE(k, 0), GET_SAVE(k, 1), GET_SAVE(k, 2),
          GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);

  sprintf(buf, "Resistance %%: heat:%d, cold:%d, energy:%d, blunt:%d, slash:%d, pierce:%d\r\n",
          get_damage_reduction(k, DAMTYPE_FIRE), get_damage_reduction(k, DAMTYPE_ICE),
          get_damage_reduction(k, DAMTYPE_ENERGY), get_damage_reduction(k, DAMTYPE_BLUNT),
          get_damage_reduction(k, DAMTYPE_SLASH), get_damage_reduction(k, DAMTYPE_PIERCE));
  send_to_char(buf, ch);

  sprinttype(GET_POS(k), position_types, buf2);
  sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	  (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

  if (IS_NPC(k)) {
    strcat(buf, ", Attack type: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
  }
  if (k->desc) {
    sprinttype(k->desc->connected, connected_types, buf2);
    strcat(buf, ", Connected: ");
    strcat(buf, buf2);
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  strcpy(buf, "Default position: ");
  sprinttype((k->mob_specials.default_pos), position_types, buf2);
  strcat(buf, buf2);

  sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintbit(MOB_FLAGS(k), action_bits, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
    if((GET_LEVEL(ch) < LVL_IMPL) && PLR_FLAGGED(k, PLR_SNOOP)) {
      REMOVE_BIT(PLR_FLAGS(k), PLR_SNOOP);
      sprintbit(PLR_FLAGS(k), player_bits, buf2);
      SET_BIT(PLR_FLAGS(k), PLR_SNOOP);
    }
    else
      sprintbit(PLR_FLAGS(k), player_bits, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF_FLAGS(k), preference_bits, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(GRNT_FLAGS(k), grant_bits, buf2);
    sprintf(buf, "GRANT: %s%s%s%s\r\n", CCBLD(ch, C_NRM), CCBLU(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (IS_MOB(k)) {
    sprintf(buf, "MOB Spec-Proc: %s%s, NPC Bare Hand Dam: %dd%d\r\n",
	    (mob_index[GET_MOB_RNUM(k)].func ? (((k->mob_specials.spec_proc>0)&&(k->mob_specials.spec_proc<MAX_SPECPROCS)) ? spec_proc_names[k->mob_specials.spec_proc] : "Exists") : (mob_index[GET_MOB_RNUM(k)].progtypes ? "mobprog" : "None")),
            (((mob_index[GET_MOB_RNUM(k)].func)&&(mob_index[GET_MOB_RNUM(k)].progtypes)) ? " + mobprog" : ""),
	    k->mob_specials.damnodice, k->mob_specials.damsizedice);
    send_to_char(buf, ch);
    sprintf(buf, "NPC Num attacks: %d, NPC ferocity: %d, NPC move rate: %d\r\n",
            k->mob_specials.attacks, GET_FEROCITY(k), GET_MOVE_RATE(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Carried: weight: %d, items: %d; ",
	  IS_CARRYING_W(k), IS_CARRYING_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  sprintf(buf, "Hunger: %d, Thirst: %d, Drunk: %d\r\n",
	  GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
  send_to_char(buf, ch);

  sprintf(buf, "Master is: %s, Followers are:",
	  ((k->master) ? GET_NAME(k->master) : "<none>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  if(IS_NPC(k)) {
    sprintf(buf, "Action list:\r\n%s", (ACTIONS(k) && *ACTIONS(k)) ? ACTIONS(k) : "");
    send_to_char(buf, ch);
  }

  /* Showing the bitvector */
  sprintbit(AFF_FLAGS(k), affected_bits, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
	      CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
      if (aff->modifier) {
	sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
	strcat(buf, buf2);
      }
      if (aff->bitvector) {
	if (*buf2)
	  strcat(buf, ", sets ");
	else
	  strcat(buf, "sets ");
	sprintbit(aff->bitvector, affected_bits, buf2);
	strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
}


ACMD(do_stat)
{
  struct char_data *victim = 0;
  struct obj_data *object = 0;
  struct char_file_u tmp_store;
  int tmp, zone;

  void do_stat_zone(struct char_data *ch, int zone);

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char("Stats on who or what?\r\n", ch);
    return;
  } else if (is_abbrev(buf1, "room")) {
    if(!*buf2)
      do_stat_room(ch, ch->in_room);
    else {
      if(!isdigit(*buf2)) {
        send_to_char("You must specify a room vnum.\r\n", ch);
        return;
      }
      if((tmp=real_room(atol(buf2)))<0) {
        send_to_char("That room does not exist.\r\n", ch);
        return;
      }
      do_stat_room(ch, tmp);
    }
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Stats on which mobile?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      if (load_char(buf2, &tmp_store) > -1) {
	store_to_char(&tmp_store, victim);
        victim->in_room=0;
	if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	else
	  do_stat_character(ch, victim);
        victim->in_room=NOWHERE;
	free_char(victim);
      } else {
	send_to_char("There is no such player.\r\n", ch);
	free(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char("Stats on which object?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)))
	do_stat_object(ch, object);
      else
	send_to_char("No such object around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "zone")) {
    if (!*buf2)
      send_to_char("Stats on which zone?\r\n", ch);
    else {
      if (is_number(buf2) && ((zone = real_zone(atoi(buf2))) >= 0))
	do_stat_zone(ch, zone);
      else
	send_to_char("No such zone.\r\n", ch);
    }
  } else {
    if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
      do_stat_object(ch, object);
    else if ((victim = get_char_room_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}


ACMD(do_shutdown)
{
  struct char_data *tch;
  extern int circle_shutdown, circle_reboot;

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("If you want to shut something down, say so!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  for(tch=character_list; tch; tch=tch->next) {
    if(!IS_NPC(tch)) {
      save_char(tch, NOWHERE);
      Crash_crashsave(tch);
    }
  }

  if (!*arg) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    sprintf(buf, "(GC) Reboot by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Rebooting.. come back in 5 minutes.\r\n");
    touch("../.fastboot");
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "die")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down for maintenance.\r\n");
    touch("../.killbuild");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down for maintenance.\r\n");
    touch("../bpause");
    circle_shutdown = 1;
  } else
    send_to_char("Unknown shutdown option.\r\n", ch);
}


void stop_snooping(struct char_data * ch)
{
  if (!ch->desc->snooping)
    send_to_char("You aren't snooping anyone.\r\n", ch);
  else {
    sprintf(buf, "(GC) %s stopped snooping %s.", GET_NAME(ch), (ch->desc->snooping->character&&GET_NAME(ch->desc->snooping->character)?GET_NAME(ch->desc->snooping->character):""));
    log(buf);
    send_to_char("You stop snooping.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  struct char_data *victim, *tch;
  struct descriptor_data *d;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg) {
    stop_snooping(ch);
    return;
  }
  else if (!(victim = get_char_vis(ch, arg))) {
    send_to_char("No such person around.\r\n", ch);
    return;
  }
  else if (!victim->desc) {
    for(d=descriptor_list; d; d=d->next) {
      if(d->original==victim) {
        victim=d->character;
        break;
      }
    }
    if(!d) {
      send_to_char("There's no link.. nothing to snoop.\r\n", ch);
      return;
    }
  }

  if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Busy already. \r\n", ch);
  else if (victim->desc->snooping == ch->desc)
    send_to_char("Don't be stupid.\r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (!IMM_AFF_IMM(ch, tch, FALSE)) {
      send_to_char("You can't.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;

    if(victim!=tch)
      sprintf(buf1, "(%s)", GET_NAME(tch));
    else
      strcpy(buf1, "");
    sprintf(buf, "(GC) %s snooping %s%s.", GET_NAME(ch), GET_NAME(victim), buf1);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}



ACMD(do_switch)
{
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char("You're already switched.\r\n", ch);
  else if (!*arg)
    send_to_char("Switch with who?\r\n", ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such character.\r\n", ch);
  else if (ch == victim)
    send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
  else if (victim->desc)
    send_to_char("You can't do that, the body is already in use!\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_CIMP) && !IS_NPC(victim))
    send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
  else if ((!IS_NPC(victim)) && (GET_LEVEL(victim) >= LVL_HERO) && (GET_LEVEL(ch) < LVL_IMPL))
    send_to_char("You cannot switch into an immortal's body.\r\n", ch);
  else {
    send_to_char(OK, ch);

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;

    sprintf(buf, "(GC) %s switching into %s%s.", GET_NAME(ch), GET_NAME(victim), (IS_NPC(victim)?"":"(mort)"));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);

    /* JE 2/22/95 */
    /* if someone switched into your original body, disconnect them */
    if (ch->desc->original->desc)
      close_socket(ch->desc->original->desc);

    sprintf(buf, "(GC) %s has returned to %s original body.", GET_NAME(ch->desc->original), HSHR(ch->desc->original));
    mudlog(buf, NRM, GET_LEVEL(ch->desc->original), TRUE);

    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}



ACMD(do_load)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num, i;

  if(subcmd!=SCMD_FAKE) {
    if((GET_LEVEL(ch) < LVL_ASST) &&
       ((ch->player_specials->zone_locked < 1) ||
        (zone_table[world[ch->in_room].zone].number != ch->player_specials->zone_locked))) {
      send_to_char("You can only load things in a zone you have locked.\r\n", ch);
      return;
    }
  }

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    sprintf(buf, "Usage: %sload { obj | mob } <number>\r\n", (subcmd==SCMD_FAKE)?"fake":"");
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    sprintf(buf, "(GC) %s has loaded mob %d (%s) at %s [%d].", GET_NAME(ch),
            number, GET_NAME(mob), world[ch->in_room].name, world[ch->in_room].number);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    if(subcmd==SCMD_FAKE) {
      obj->obj_flags.value[0]=0;
      obj->obj_flags.value[1]=0;
      obj->obj_flags.value[2]=0;
      obj->obj_flags.value[3]=0;
      obj->obj_flags.type_flag=13;
      obj->obj_flags.cost=0;
      obj->obj_flags.bitvector=0;
      obj->obj_flags.immune=0;
      obj->obj_flags.resist=0;
      obj->obj_flags.weak=0;
      for(i=0; i<MAX_OBJ_AFFECT; i++) {
        obj->affected[i].location=APPLY_NONE;
        obj->affected[i].modifier=0;
      }
      sprintf(buf, "(GC) %s has loaded a fake of obj %d (%s) at %s [%d].", GET_NAME(ch),
              number, obj->short_description, world[ch->in_room].name, world[ch->in_room].number);
      mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    }
    else {
      sprintf(buf, "(GC) %s has loaded obj %d (%s) at %s [%d].", GET_NAME(ch),
              number, obj->short_description, world[ch->in_room].name, world[ch->in_room].number);
      mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    }
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}



ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}




/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  ACMD(do_vcdepart);

  if((GET_LEVEL(ch) < LVL_ASST) && (!HAS_OLC(ch, zone_table[world[ch->in_room].zone].number))) {
    send_to_char("You can only purge things in a zone you have OLC to.\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  if (*buf) {		/* argument supplied. destroy single object or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict)) {
        if(GET_LEVEL(ch) <= GET_LEVEL(vict)) {
          sprintf(buf, "You can't purge %s!\r\n", HMHR(vict));
          send_to_char(buf, ch);
          return;
        }
        if(GET_LEVEL(ch) < LVL_ASST) {
          send_to_char("You can't purge players!\r\n", ch);
          return;
        }
      }

      if((!IS_NPC(vict)) && (GET_OLC_MODE(vict) || (vict->player_specials->zone_locked > 0))) {
        send_to_char("Purging someone who is editting is a bad idea.\r\n", ch);
        return;
      }
      if(ch->char_specials.spcont) {
        send_to_char("You can't purge someone with continuous affects.\r\n", ch);
        return;
      }

      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      sprintf(buf, "(GC) %s has purged %s at %s [%d].", GET_NAME(ch), GET_NAME(vict),
              world[ch->in_room].name, world[ch->in_room].number);
      if (!IS_NPC(vict)) {
	mudlog(buf, BRF, LVL_ASST, TRUE);
	if (vict->desc) {
	  close_socket(vict->desc);
	  vict->desc = NULL;
	}
        if(vict->char_specials.vc >= 0)
          do_vcdepart(vict, "", 0, 0);
      }
      else {
        mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      sprintf(buf, "(GC) %s has purged %s at %s [%d].", GET_NAME(ch), obj->short_description,
              world[ch->in_room].name, world[ch->in_room].number);
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char("Nothing here by that name.\r\n", ch);
      return;
    }

    send_to_char(OK, ch);
  } else {			/* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict))
	extract_char(vict);
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
    sprintf(buf, "(GC) %s purged room %d (%s).", GET_NAME(ch),
            world[ch->in_room].number, world[ch->in_room].name);
    mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
  }
}


ACMD(do_zpurge)
{
  int room, zone, i;
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  one_argument(argument, buf);

  if((!*buf) || (*buf == '.'))
    zone=world[ch->in_room].zone;
  else
    zone=real_zone(atoi(buf));

  if(zone < 0) {
    send_to_char("That zone doesn't exist.\r\n", ch);
    return;
  }

  if((GET_LEVEL(ch) < LVL_CIMP) && (!HAS_OLC(ch, zone_table[zone].number))) {
    send_to_char("You can only purge a zone you have OLC to.\r\n", ch);
    return;
  }

  for(room=zone_table[zone].bottom; room <= zone_table[zone].top; room++) {
    if((i=real_room(room)) >= 0) {
      if(subcmd != 1)
        send_to_room("A wall of flame passes through the zone!.\r\n", i);
      for (vict = world[i].people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (IS_NPC(vict))
          extract_char(vict);
      }

      for (obj = world[i].contents; obj; obj = next_o) {
        next_o = obj->next_content;
        extract_obj(obj);
      }
    }
  }
  if(subcmd != 1) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s has purged zone %s [%d].", GET_NAME(ch),
            zone_table[zone].name, zone_table[zone].number);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}


ACMD(do_advance)
{
  struct char_data *victim;
  char *name = arg, *level = buf2;
  int newlevel, oldlevel, i;

  ACMD(do_remort);

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_player_vis(ch, name))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (!IMM_AFF_IMM(ch, victim, FALSE)) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }

  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Yeah, right.\r\n", ch);
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char("They are already at that level.\r\n", ch);
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) {
    if(GET_LEVEL(victim) >= LVL_HERO) {
      if(newlevel < LVL_HERO) {
        strcpy(buf2, GET_NAME(victim));
        do_remort(ch, buf2, 0, 0);
      }
      else {
        GET_LEVEL(victim)=newlevel;
        for(i=0; i<NUM_CLASSES; i++)
          GET_CLASS_LEVEL(victim, i)=newlevel;
        send_to_char("You are momentarily enveloped by darkness!\r\n"
                     "You feel somewhat diminished.\r\n", victim);
      }
    }
    else {
      send_to_char("Sorry, you cannot reverse-advance characters.\r\n", ch);
      return;
    }
  } else {
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }

  send_to_char(OK, ch);

  sprintf(buf, "(GC) %s has advanced %s to level %d (from %d).",
	  GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);
  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);

  if(newlevel >= LVL_HERO) {
    GET_NUM_CLASSES(victim)=1;
    GET_LEVEL(victim)=newlevel;
    GET_CLASS_BITVECTOR(victim)=0xFFFFFFFF;
    for(i=0; i<NUM_CLASSES; i++)
      GET_CLASS_LEVEL(victim, i)=newlevel;
    GET_EXP(victim)=exp_table[(int)GET_CLASS(victim)][newlevel];
  }
  else {
    gain_exp_regardless(victim,
         (GET_NUM_CLASSES(victim)*exp_table[(int)GET_CLASS(victim)][newlevel]) - GET_EXP(victim));
  }

  if((oldlevel >= LVL_HERO) || (newlevel >= LVL_HERO))
  {
    char buf[100];
    extern int use_autowiz;
    extern int min_wizlist_lev;
    pid_t getpid(void);

    if (use_autowiz) {
      save_char(victim, NOWHERE);
      sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_HERO, IMMLIST_FILE, (int) getpid());
      mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
      system(buf);
    }
  }
  save_char(victim, NOWHERE);
}



ACMD(do_restore)
{
  struct char_data *vict;
  int i;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to restore?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else {
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);

    if ((GET_LEVEL(ch) >= LVL_ASST) && (GET_LEVEL(vict) >= LVL_HERO) && (!IS_NPC(vict))) {
      for (i = 1; i <= MAX_SKILLS; i++)
	SET_SKILL(vict, i, 100);

      vict->real_abils.str_add = 100;
      vict->real_abils.intel = 25;
      vict->real_abils.wis = 25;
      vict->real_abils.dex = 25;
      vict->real_abils.str = 25;
      vict->real_abils.con = 25;
      vict->real_abils.cha = 25;

      vict->aff_abils = vict->real_abils;

      SET_BIT(PRF_FLAGS(vict), PRF_NOHASSLE | PRF_HOLYLIGHT | PRF_ROOMFLAGS);

      GET_COND(vict, THIRST)=-1;
      GET_COND(vict, FULL)=-1;
      GET_COND(vict, DRUNK)=-1;
    }
    update_pos(vict);
    affect_total(vict);
    save_char(vict, NOWHERE);
    send_to_char(OK, ch);
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
    sprintf(buf, "(GC) %s has restored %s.", GET_NAME(ch), GET_NAME(vict));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}


void perform_immort_vis(struct char_data *ch)
{
  void appear(struct char_data *ch);

  if (GET_INVIS_LEV(ch) == 0 && !IS_AFFECTED(ch, AFF_HIDE | AFF_INVISIBLE)) {
    send_to_char("You are already fully visible.\r\n", ch);
    return;
  }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("You are now fully visible.\r\n", ch);
}


void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  sprintf(buf, "Your invisibility level is %d.\r\n", level);
  send_to_char(buf, ch);
}
  

ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("You can't do that!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char("You can't go invisible above your own level.\r\n", ch);
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);

  if (!*argument)
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    delete_doubledollar(argument);
    sprintf(buf, "%s\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character && pt->character != ch)
	send_to_char(buf, pt->character);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      send_to_char(buf, ch);
    sprintf(buf, "(GC) %s used gecho.", GET_NAME(ch));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}


ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:    msg = &(POOFIN(ch));    break;
  case SCMD_POOFOUT:   msg = &(POOFOUT(ch));   break;
  default:    return;    break;
  }

  skip_spaces(&argument);

  if (!*argument)
  {
    send_to_char(*msg, ch);
    send_to_char("\r\n", ch);
  }
  else
  {
    if(strlen(argument) > MAX_WALK_LENGTH) {
      sprintf(buf, "Your %s can be no longer than %d characters.\r\n",
              (subcmd==SCMD_POOFIN ? "poofin" : "poofout"), MAX_WALK_LENGTH);
      send_to_char(buf, ch);
      return;
    }
    free(*msg);
    *msg = str_dup(argument);
    send_to_char(OK, ch);
  }

}



ACMD(do_dc)
{
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char("Usage: DC <connection number> (type USERS for a list)\r\n", ch);
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    return;
  }
  close_socket(d);
  sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
  send_to_char(buf, ch);
  sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
  mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
}



ACMD(do_wizlock)
{
  int value;
  char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    restrict_mud = value;
    when = "now";
    sprintf(buf, "(GC) %s changed wizlock to %d.", GET_NAME(ch), restrict_mud);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  } else
    when = "currently";

  switch (restrict_mud) {
  case 0:
    sprintf(buf, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    sprintf(buf, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
	    restrict_mud, when);
    break;
  }
  send_to_char(buf, ch);
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;
  extern time_t boot_time;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sprintf(buf, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
	    ((d == 1) ? "" : "s"), h, m);
  }

  send_to_char(buf, ch);
}



ACMD(do_last)
{
  struct char_file_u chdata;
  extern char *class_abbrevs[];

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }
  if (load_char(arg, &chdata) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }
  if ((chdata.level > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You are not sufficiently godly for that!\r\n", ch);
    return;
  }
  sprintf(buf, "[%5ld] [%-3d%s] %-12s : %-18s : %-20s\r\n",
	  chdata.char_specials_saved.idnum, (int) chdata.level,
	  class_abbrevs[(int) chdata.class], chdata.name, chdata.host,
	  ctime(&chdata.last_logon));
  send_to_char(buf, ch);
}


ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char to_force[MAX_INPUT_LENGTH + 2];

  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n has forced you to %s.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_CIMP) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg)))
      send_to_char(NOPERSON, ch);
    else if ((GET_LEVEL(ch) <= GET_LEVEL(vict)) && (!IS_NPC(vict)))
      send_to_char("No, no, no!\r\n", ch);
    else {
      send_to_char(OK, ch);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      sprintf(buf, "(GC) %s forced %s to %s.", GET_NAME(ch), GET_NAME(vict), to_force);
      mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced room %s [%d] to %s.", GET_NAME(ch),
            world[ch->in_room].name, world[ch->in_room].number, to_force);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if ((GET_LEVEL(vict) >= GET_LEVEL(ch)) && (!IS_NPC(vict)))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced all to %s.", GET_NAME(ch), to_force);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (i->connected || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}


ACMD(do_wiznet)
{
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_HERO;
  struct char_data *vict=NULL;
  char to_ch[MAX_INPUT_LENGTH+100];
  char to_world[MAX_INPUT_LENGTH+100];
  char to_vict[MAX_INPUT_LENGTH+100];

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
		 "       wiznet *<level> <emotetext> | wiz @\r\n", ch);
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_HERO);
      if (level > GET_LEVEL(ch)) {
	send_to_char("You can't wizline above your own level.\r\n", ch);
	return;
      }
    } else if (emote)
      argument++;
    break;
  case '@':
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_HERO &&
	  !PRF_FLAGGED(d->character, PRF_NOWIZ) && CAN_SEE(ch, d->character)) {
	if (!any) {
	  sprintf(buf1, "Gods online:\r\n");
	  any = TRUE;
	}
	sprintf(buf1, "%s  %s", buf1, GET_NAME(d->character));
	if (PLR_FLAGGED(d->character, PLR_MAILING))
	  sprintf(buf1, "%s (Writing mail)\r\n", buf1);
	else if (PLR_FLAGGED(d->character, PLR_BUILDING))
	  sprintf(buf1, "%s (Building)\r\n", buf1);
	else if (PLR_FLAGGED(d->character, PLR_WRITING))
	  sprintf(buf1, "%s (Writing)\r\n", buf1);
	else
	  sprintf(buf1, "%s\r\n", buf1);
      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_HERO &&
	  PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  CAN_SEE(ch, d->character)) {
	if (!any) {
	  sprintf(buf1, "%sGods offline:\r\n", buf1);
	  any = TRUE;
	}
	sprintf(buf1, "%s  %s\r\n", buf1, GET_NAME(d->character));
      }
    }
    send_to_char(buf1, ch);
    return;
    break;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }
  if(!emote)
    delete_doubledollar(argument);
  skip_spaces(&argument);
  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }
  if(GET_INVIS_LEV(ch))
    sprintf(buf, "(i%d)", GET_INVIS_LEV(ch));
  else
    strcpy(buf, "");
  if(emote)
    vict=get_soc_mes(ch, argument, to_ch, to_world, to_vict);
  if (level > LVL_HERO) {
    if(emote) {
      sprintf(buf1, "Wiznet%s<%d>: ", buf, level);
    }
    else {
      sprintf(buf1, "%s%s: <%d> %s\r\n", GET_NAME(ch), buf, level, argument);
      sprintf(buf2, "Someone%s: <%d> %s\r\n", buf, level, argument);
    }
  } else {
    if(emote) {
      sprintf(buf1, "Wiznet%s: ", buf);
    }
    else {
      sprintf(buf1, "%s%s: %s\r\n", GET_NAME(ch), buf, argument);
      sprintf(buf2, "Someone%s: %s\r\n", buf, argument);
    }
  }
  for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) && (GET_LEVEL(d->character) >= level) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
	(!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING | PLR_BUILDING))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      send_to_char(CCCYN(d->character, C_NRM), d->character);
      if(emote) {
        if(d->character==ch) {
          sprintf(buf2, "%s%s", buf1, to_ch);
          act(buf2, FALSE, ch, NULL, vict, TO_CHAR | TO_SLEEP);
        }
        else if(d->character==vict) {
          sprintf(buf2, "%s%s", buf1, to_vict);
          act(buf2, FALSE, ch, NULL, vict, TO_VICT | TO_SLEEP);
        }
        else {
          sprintf(buf2, "%s%s", buf1, to_world);
          act(buf2, FALSE, ch, (struct obj_data *)d->character, vict, TO_OBJ | TO_SLEEP);
        }
      }
      else {
        if (CAN_SEE(d->character, ch))
          send_to_char(buf1, d->character);
        else
          send_to_char(buf2, d->character);
      }
      send_to_char(CCNRM(d->character, C_NRM), d->character);
    }
  }
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
}
ACMD(do_zreset)
{
  void reset_zone(int zone);
  int i, j;
  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    if(GET_LEVEL(ch) < LVL_CIMP) {
      send_to_char("You can't do that!\r\n", ch);
      return;
    }
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reset world.\r\n", ch);
    sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
    mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    i=real_zone(j);
  }
  if (i >= 0 && i <= top_of_zone_table) {
    if((GET_LEVEL(ch) < LVL_ASST) && (!HAS_OLC(ch, zone_table[i].number))) {
      send_to_char("You can only reset zones you have OLC to.\r\n", ch);
      return;
    }
    reset_zone(i);
    if(subcmd != 1) {
      sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
              zone_table[i].name);
      send_to_char(buf, ch);
      sprintf(buf, "(GC) %s reset zone %d (%s).", GET_NAME(ch), zone_table[i].number, zone_table[i].name);
      mudlog(buf, NRM, MAX(LVL_ASST, GET_INVIS_LEV(ch)), TRUE);
    }
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}
/*
 *  General fn for wizcommands of the sort: cmd <player>
 */
ACMD(do_wizutil)
{
  struct char_data *vict, *k, *temp;
  long result;
  void roll_real_abils(struct char_data *ch);
  one_argument(argument, arg);
  if((subcmd == SCMD_REROLL) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("That is not a command on Ashes to Ashes, please retype.\r\n", ch);
    return;
  }
  if (!*arg)
    send_to_char("Yes, but for whom?!?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg)))
    send_to_char("There is no such player.\r\n", ch);
  else if (IS_NPC(vict) && (subcmd!=SCMD_UNAFFECT))
    send_to_char("You can't do that to a mob!\r\n", ch);
  else if ((!IMM_AFF_IMM(ch, vict, FALSE)) && (!IS_NPC(vict)))
    send_to_char("Hmmm...you'd better not.\r\n", ch);
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      if(GET_LEVEL(vict) >= LVL_HERO) {
        send_to_char("What is the point of rerolling an immortal?\r\n", ch);
        return;
      }
      if(ch->char_specials.spcont) {
        send_to_char("You can't reroll someone with continuous affects.\r\n", ch);
        return;
      }
      send_to_char("Rerolling...\r\n", ch);
      sprintf(buf, "(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
      mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
      send_to_char("You beging tingling from head to foot. The sensation spreads inward\r\n"
                   "through your body and the world fades from view.\r\n", vict);
      act("Ribbons of light beging to swirl around $n. They close in, distorting\r\n"
          "$s flesh so that it becomes harder and harder to distinguish it from\r\n"
          "the ribbons. In a matter of seconds $e has become a rapidly shrinking\r\n"
          "column of colored light which soon dissapears.", TRUE, vict, 0, 0, TO_ROOM);
      vict->was_in_room=world[vict->in_room].number;
      if (FIGHTING(vict))
        stop_fighting(vict);
      for (k = combat_list; k; k = temp) {
        temp = k->next_fighting;
        if (FIGHTING(k) == vict)
          stop_fighting(k);
      }
      vict->desc->prompt_mode = 0;
      vict->player_specials->saved.reroll_level=GET_LEVEL(vict);
      vict->player_specials->saved.rerolling=STATE(vict->desc) = CON_REROLLING;
      save_char(vict, NOWHERE);
      char_from_room(vict);
      REMOVE_FROM_LIST(vict, character_list, next);
      roll_real_abils(vict);
      SEND_TO_Q("\r\nYour abilities are:\r\n", vict->desc);
      sprintf(buf, "       Strength: %2d/%02d\r\n", GET_STR(vict), GET_ADD(vict));
      SEND_TO_Q(buf, vict->desc);
      sprintf(buf, "   Intelligence: %2d\r\n", GET_INT(vict));
      SEND_TO_Q(buf, vict->desc);
      sprintf(buf, "         Wisdom: %2d\r\n", GET_WIS(vict));
      SEND_TO_Q(buf, vict->desc);
      sprintf(buf, "      Dexterity: %2d\r\n", GET_DEX(vict));
      SEND_TO_Q(buf, vict->desc);
      sprintf(buf, "   Constitution: %2d\r\n", GET_CON(vict));
      SEND_TO_Q(buf, vict->desc);
      sprintf(buf, "       Charisma: %2d\r\n", GET_CHA(vict));
      SEND_TO_Q(buf, vict->desc);
      SEND_TO_Q("\nKeep these statistics (y/n)? ", vict->desc);
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_ASST, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_ASST, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Your victim is already pretty cold.\r\n", ch);
	return;
      }
      SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n", vict);
      send_to_char("Frozen.\r\n", ch);
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_ASST, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
	sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	   GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	send_to_char(buf, ch);
	return;
      }
      sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_ASST, GET_INVIS_LEV(ch)), TRUE);
      REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
      send_to_char("Thawed.\r\n", ch);
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected) {
	while (vict->affected)
	  affect_remove(vict, vict->affected);
	send_to_char("There is a brief flash of light!\r\n"
		     "You feel slightly different.\r\n", vict);
	send_to_char("All spells removed.\r\n", ch);
        sprintf(buf, "(GC) %s unaffected %s.", GET_NAME(ch), GET_NAME(vict));
        mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
      } else {
	send_to_char("Your victim does not have any affections!\r\n", ch);
	return;
      }
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_wizutil (act.wizard.c)");
      break;
    }
    save_char(vict, NOWHERE);
  }
}
/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */
void print_zone_to_buf(char *bufptr, int zone)
{
  sprintf(bufptr, "%s%3d %-25.25s Age: %3d; Reset: %3d (%1d); Range: %5d-%5d; %c%c\r\n",
	  bufptr, zone_table[zone].number, zone_table[zone].name,
	  zone_table[zone].age, zone_table[zone].lifespan,
	  zone_table[zone].reset_mode, zone_table[zone].bottom,
          zone_table[zone].top, (*zone_table[zone].locked_by?'L':' '),
          (zone_table[zone].closed?'C':' '));
}
ACMD(do_show)
{
  struct char_file_u vbuf;
  int i, j, k, l, con;
  char self = 0;
  struct char_data *vict;
  struct obj_data *obj;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];
  char buf[MAX_STRING_LENGTH*3];
  extern char *class_abbrevs[];
  extern char *genders[];
  extern int buf_switches, buf_largecount, buf_overflows;
  void show_shops(struct char_data * ch, char *value);
  void Crash_listreimb(struct char_data * ch, char *name);
  void show_auction(struct char_data *ch);
  struct show_struct {
    char *cmd;
    char level;
  } fields[] = {
    { "nothing",	0  },				/* 0 */
    { "zones",		LVL_HERO },			/* 1 */
    { "player",		LVL_ASST },
    { "rent",		LVL_ASST },
    { "stats",		LVL_IMMORT },
    { "deathtraps",	LVL_ASST },
    { "godrooms",	LVL_ASST },
    { "shops",		LVL_IMMORT },
    { "reimburse",	LVL_ASST },
    { "auction",	LVL_ASST },
    { "\n", 0 }
  };
  skip_spaces(&argument);
  if (!*argument) {
    strcpy(buf, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if ((fields[i].level <= GET_LEVEL(ch)) || (GRNT_FLAGGED(ch, GRNT_SHOW)))
	sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }
  strcpy(arg, two_arguments(argument, field, value));
  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;
  if ((GET_LEVEL(ch) < fields[l].level) &&  (!GRNT_FLAGGED(ch, GRNT_SHOW))) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (!strcmp(value, "."))
    self = 1;
  buf[0] = '\0';
  switch (l) {
  case 1:			/* zone */
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf, world[ch->in_room].zone);
    else if (*value && isdigit(*value)) {
      k=l=-1;
      sscanf(value, "%d-%d", &k, &l);
      if(l==-1) {
        l=k;
        if(real_zone(k)<0) {
          send_to_char("That is not a valid zone.\r\n", ch);
          return;
        }
      }
      for (i = 0; i <= top_of_zone_table; i++)
        if((zone_table[i].number >= k) && (zone_table[i].number <= l))
          print_zone_to_buf(buf, i);
    } else
      for (i = 0; i <= top_of_zone_table; i++)
	print_zone_to_buf(buf, i);
    page_string(ch->desc, buf, 1);
    break;
  case 2:			/* player */
    if (!*value) {
      send_to_char("A name would help.\r\n", ch);
      return;
    }
    if (load_char(value, &vbuf) < 0) {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
    if(vbuf.level > GET_LEVEL(ch)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return;
    }
    sprintf(buf, "Player: %-12s (%s) [%-3d%s]\r\n", vbuf.name,
      genders[(int) vbuf.sex], vbuf.level, class_abbrevs[(int) vbuf.class]);
    sprintf(buf,
	 "%sAu: %-8ld  Bal: %-8ld  Exp: %-8ld  Align: %-5d  Lessons: %-3d\r\n",
	    buf, vbuf.points.gold, vbuf.points.bank_gold, vbuf.points.exp,
	    vbuf.char_specials_saved.alignment,
	    vbuf.player_specials_saved.spells_to_learn);
    strcpy(birth, ctime(&vbuf.birth));
    sprintf(buf,
	    "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
	    buf, birth, ctime(&vbuf.last_logon), (int) (vbuf.played / 3600),
	    (int) (vbuf.played / 60 % 60));
    send_to_char(buf, ch);
    break;
  case 3:
    Crash_listrent(ch, value);
    break;
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    sprintf(buf, "Current stats:\r\n");
    sprintf(buf, "%s  %5d players in game  %5d connected\r\n", buf, i, con);
    sprintf(buf, "%s  %5d registered\r\n", buf, top_of_p_table + 1);
    sprintf(buf, "%s  %5d mobiles          %5d prototypes\r\n",
	    buf, j, top_of_mobt + 1);
    sprintf(buf, "%s  %5d objects          %5d prototypes\r\n",
	    buf, k, top_of_objt + 1);
    sprintf(buf, "%s  %5d rooms            %5d zones\r\n",
	    buf, top_of_world + 1, top_of_zone_table + 1);
    sprintf(buf, "%s  %5d large bufs\r\n", buf, buf_largecount);
    sprintf(buf, "%s  %5d buf switches     %5d overflows\r\n", buf,
	    buf_switches, buf_overflows);
    send_to_char(buf, ch);
    break;
  case 5:
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
		world[i].number, world[i].name);
    page_string(ch->desc, buf, 1);
    break;
  case 6:
#define GOD_ROOMS_ZONE 12
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i < top_of_world; i++)
      if (zone_table[world[i].zone].number == GOD_ROOMS_ZONE)
	sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, j++, world[i].number,
		world[i].name);
    page_string(ch->desc, buf, 1);
    break;
  case 7:
    show_shops(ch, value);
    break;
  case 8:
    Crash_listreimb(ch, value);
    break;
  case 9:
    show_auction(ch);
    break;
  default:
    send_to_char("Sorry, I don't understand that.\r\n", ch);
    break;
  }
}
#define PC   1
#define NPC  2
#define BOTH 3
#define MISC	0
#define BINARY	1
#define NUMBER	2
#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }
#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))
ACMD(do_set)
{
  int i, l, setok=1, nr, update_wizl=0;
  struct char_data *vict = NULL, *cbuf = NULL;
  struct char_file_u tmp_store;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], val_arg[MAX_INPUT_LENGTH];
  char *va, oldname[25];
  char f1[MAX_INPUT_LENGTH], f2[MAX_INPUT_LENGTH];
  int on = 0, off = 0, value = 0;
  char is_file = 0, is_mob = 0, is_player = 0;
  int player_i = 0;
  int parse_class(char arg);
  long get_id_by_name(char *name);
  struct set_struct {
    char *cmd;
    char level;
    char pcnpc;
    char type;
  }          fields[] = {
   { "brief",		LVL_ASST, 	PC, 	BINARY },  /* 0 */
   { "invstart", 	LVL_ASST, 	PC, 	BINARY },  /* 1 */
   { "title",		LVL_ASST, 	PC, 	MISC },
   { "summonable", 	LVL_ASST, 	PC, 	BINARY },
   { "maxhit",		LVL_ASST, 	BOTH, 	NUMBER },
   { "maxmana", 	LVL_ASST, 	BOTH, 	NUMBER },  /* 5 */
   { "maxmove", 	LVL_ASST, 	BOTH, 	NUMBER },
   { "hit", 		LVL_ASST, 	BOTH, 	NUMBER },
   { "mana",		LVL_ASST, 	BOTH, 	NUMBER },
   { "move",		LVL_ASST, 	BOTH, 	NUMBER },
   { "align",		LVL_ASST, 	BOTH, 	NUMBER },  /* 10 */
   { "str",		LVL_ASST, 	BOTH, 	NUMBER },
   { "stradd",		LVL_ASST, 	BOTH, 	NUMBER },
   { "int", 		LVL_ASST, 	BOTH, 	NUMBER },
   { "wis", 		LVL_ASST, 	BOTH, 	NUMBER },
   { "dex", 		LVL_ASST, 	BOTH, 	NUMBER },  /* 15 */
   { "con", 		LVL_ASST, 	BOTH, 	NUMBER },
   { "sex", 		LVL_ASST, 	BOTH, 	MISC },
   { "ac", 		LVL_ASST, 	BOTH, 	NUMBER },
   { "gold",		LVL_ASST, 	BOTH, 	NUMBER },
   { "bank",		LVL_ASST, 	PC, 	NUMBER },  /* 20 */
   { "exp", 		LVL_ASST, 	BOTH, 	NUMBER },
   { "hitroll", 	LVL_ASST, 	BOTH, 	NUMBER },
   { "damroll", 	LVL_ASST, 	BOTH, 	NUMBER },
   { "invis",		LVL_ASST, 	PC, 	NUMBER },
   { "nohassle", 	LVL_ASST, 	PC, 	BINARY },  /* 25 */
   { "frozen",		LVL_ASST, 	PC, 	BINARY },
   { "practices", 	LVL_ASST, 	PC, 	NUMBER },
   { "drunk",		LVL_ASST, 	BOTH, 	MISC },
   { "hunger",		LVL_ASST, 	BOTH, 	MISC },
   { "thirst",		LVL_ASST, 	BOTH, 	MISC },    /* 30 */
   { "level",		LVL_ASST, 	BOTH, 	NUMBER },
   { "room",		LVL_ASST, 	BOTH, 	NUMBER },
   { "roomflags", 	LVL_ASST, 	PC, 	BINARY },
   { "siteok",		LVL_ASST, 	PC, 	BINARY },
   { "deleted", 	LVL_CIMP, 	PC, 	BINARY },  /* 35 */
   { "class",		LVL_ASST, 	BOTH, 	MISC },
   { "nowizlist", 	LVL_CIMP, 	PC, 	BINARY },
   { "quest",		LVL_ASST, 	PC, 	BINARY },
   { "loadroom", 	LVL_ASST, 	PC, 	MISC },
   { "color",		LVL_ASST, 	PC, 	BINARY },  /* 40 */
   { "passwd",		LVL_CIMP, 	PC, 	MISC },
   { "nodelete", 	LVL_ASST, 	PC, 	BINARY },
   { "cha",		LVL_ASST, 	BOTH, 	NUMBER },
   { "noshout",		LVL_ASST,	PC,	BINARY },
   { "notitle",		LVL_ASST,	PC,	BINARY },  /* 45 */
   { "name",            LVL_ASST,       PC,     MISC },
   { "rerolls",         LVL_ASST,       PC,     NUMBER },
   { "ageadd",          LVL_ASST,       PC,     NUMBER },
   { "naturalac",       LVL_ASST,       PC,     NUMBER },
   { "snooplog",        LVL_IMPL,       PC,     BINARY },
   { "questpoints",     LVL_ASST,       PC,     NUMBER },
   { "\n", 0, BOTH, MISC }
  };
  extern struct player_index_element *player_table;
  void Crash_crashsave(struct char_data * ch);
  int _parse_name(char *arg, char *name);
  int reserved_word(char *argument);
  int Valid_Name(char *newname);
  int find_name(char *name);
  half_chop(argument, name, buf);
  if(!*name) {
    strcpy(buf, "Usage: set [file] <victim> <field> <value>\r\n");
    strcat(buf, "You can set the following fields:");
    for(i=0, l=0; *fields[i].cmd != '\n'; i++) {
      if(GET_LEVEL(ch) >= fields[i].level) {
        if(!(l%4))
          strcat(buf, "\r\n");
        sprintf(buf, "%s%-19s", buf, fields[i].cmd);
        l++;
      }
    }
    send_to_char(strcat(buf, "\r\n"), ch);
    return;
  }
  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  }
  half_chop(buf, field, buf);
  strcpy(val_arg, buf);
  if (!*name || !*field) {
    send_to_char("Usage: set [file] <victim> <field> <value>\r\n", ch);
    return;
  }
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name))) {
	send_to_char("There is no such player.\r\n", ch);
	return;
      }
    } else {
      if (!(vict = get_char_vis(ch, name))) {
	send_to_char("There is no such creature.\r\n", ch);
	return;
      }
    }
  } else if (is_file) {
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    if ((player_i = load_char(name, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      if (!IMM_AFF_IMM(ch, cbuf, FALSE)) {
	free_char(cbuf);
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  if((!IS_NPC(vict)) && (!IMM_AFF_IMM(ch, vict, FALSE))) {
    send_to_char("Maybe that's not such a great idea...\r\n", ch);
    if(is_file)
      free_char(cbuf);
    return;
  }
  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;
  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (IS_NPC(vict) && !(fields[l].pcnpc & NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    if(is_file)
      free_char(cbuf);
    return;
  } else if (!IS_NPC(vict) && !(fields[l].pcnpc & PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    if(is_file)
      free_char(cbuf);
    return;
  }
  if (fields[l].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be on or off.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
  } else if (fields[l].type == NUMBER) {
    value = atoi(val_arg);
  }
  strcpy(buf, "Okay.");  /* can't use OK macro here 'cause of \r\n */
  switch (l) {
  case 0:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2:
    set_title(vict, val_arg);
    sprintf(buf, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
    break;
  case 3:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
    on = !on;			/* so output will be correct */
    break;
  case 4:
    vict->points.max_hit = RANGE(1, MAX_HIT);
    affect_total(vict);
    break;
  case 5:
    vict->points.max_mana = RANGE(1, MAX_MANA);
    affect_total(vict);
    break;
  case 6:
    vict->points.max_move = RANGE(1, MAX_MOVE);
    affect_total(vict);
    break;
  case 7:
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 8:
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 9:
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 10:
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 11:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_HERO)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.str = value;
    vict->real_abils.str_add = 0;
    affect_total(vict);
    break;
  case 12:
    vict->real_abils.str_add = RANGE(0, 100);
    if (value > 0)
      vict->real_abils.str = 18;
    affect_total(vict);
    break;
  case 13:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_HERO)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.intel = value;
    affect_total(vict);
    break;
  case 14:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_HERO)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.wis = value;
    affect_total(vict);
    break;
  case 15:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_HERO)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.dex = value;
    affect_total(vict);
    break;
  case 16:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_HERO)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.con = value;
    affect_total(vict);
    break;
  case 17:
    if (!str_cmp(val_arg, "male"))
      vict->player.sex = SEX_MALE;
    else if (!str_cmp(val_arg, "female"))
      vict->player.sex = SEX_FEMALE;
    else if (!str_cmp(val_arg, "neutral"))
      vict->player.sex = SEX_NEUTRAL;
    else {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    break;
  case 18:
    vict->points.armor = RANGE(-1000, 100);
    affect_total(vict);
    break;
  case 19:
    GET_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 20:
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21:
    vict->points.exp = RANGE(0, 250000000);
    break;
  case 22:
    vict->points.hitroll = RANGE(-20, 50);
    affect_total(vict);
    break;
  case 23:
    vict->points.damroll = RANGE(-20, 50);
    affect_total(vict);
    break;
  case 24:
    if ((GET_LEVEL(ch) < LVL_CIMP) && (GET_LEVEL(vict) < LVL_HERO)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 25:
    if ((GET_LEVEL(ch) < LVL_CIMP) && (GET_LEVEL(vict) < LVL_HERO)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 26:
    if (ch == vict) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 27:
    GET_PRACTICES(vict) = RANGE(0, 1000);
    break;
  case 28:
  case 29:
  case 30:
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (l - 28)) = (char) -1;
      sprintf(buf, "%s's %s now off.", GET_NAME(vict), fields[l].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (l - 28)) = (char) value;
      sprintf(buf, "%s's %s set to %d.", GET_NAME(vict), fields[l].cmd,
	      value);
    } else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    break;
  case 31:
    if (value > GET_LEVEL(ch) || value > LVL_IMPL) {
      send_to_char("You can't do that.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    RANGE(0, LVL_IMPL);
    if((vict->player.level >= LVL_HERO)||(value >= LVL_HERO))
      update_wizl=1;
    if(!IS_NPC(vict) && ((value >= LVL_HERO) || (GET_LEVEL(vict) >= LVL_HERO))) {
      sprintf(buf2, "%s %d", GET_NAME(vict), value);
      if(is_file) {
        vict->next=character_list;
        character_list=vict;
        char_to_room(vict, 0);
      }
      do_advance(ch, buf2, 0, 0);
      if(is_file) {
        character_list=vict->next;
        char_from_room(vict);
      }
    }
    else
      vict->player.level = (byte) value;
    break;
  case 32:
    if ((i = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    char_from_room(vict);
    char_to_room(vict, i);
    break;
  case 33:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
    break;
  case 34:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 35:
    if ((GET_LEVEL(vict) >= LVL_CIMP)) {
      send_to_char("You cannot change that.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 36:
    if ((i = parse_class(*val_arg)) == CLASS_UNDEFINED) {
      send_to_char("That is not a class.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    GET_CLASS(vict) = i;
    break;
  case 37:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 38:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
    break;
  case 39:
    if (!str_cmp(val_arg, "off"))
      REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
    else if (is_number(val_arg)) {
      value = atoi(val_arg);
      if (real_room(value) != NOWHERE) {
        SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
	GET_LOADROOM(vict) = value;
	sprintf(buf, "%s will enter at room #%d.", GET_NAME(vict),
		GET_LOADROOM(vict));
      } else {
	sprintf(buf, "That room does not exist!");
      }
    } else {
      strcpy(buf, "Must be 'off' or a room's virtual number.\r\n");
    }
    break;
  case 40:
    SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
    break;
  case 41:
    if (!is_file)
      return;
    strncpy(tmp_store.pwd, CRYPT(val_arg, tmp_store.name), MAX_PWD_LENGTH);
    tmp_store.pwd[MAX_PWD_LENGTH] = '\0';
    sprintf(buf, "Password changed to '%s'.", val_arg);
    break;
  case 42:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;
  case 43:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_HERO)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.cha = value;
    affect_total(vict);
    break;
  case 44:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOSHOUT);
    break;
  case 45:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOTITLE);
    break;
  case 46:
    va=val_arg;
    skip_spaces(&va);
    anyonearg(va, va);
    va[0]=UPPER(va[0]);
    strcpy(val_arg, va);
    if((_parse_name(val_arg, f1)) || (strlen(f1) < 2) ||
       (strlen(f1) > MAX_NAME_LENGTH) || fill_word(f1) ||
       reserved_word(f1) || (!Valid_Name(f1))) {
      send_to_char("Invalid name.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    if(get_id_by_name(val_arg) != -1) {
      send_to_char("That name is already taken.\r\n", ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    if((nr=find_name(GET_NAME(vict))) < 0) {
      sprintf(buf, "%s not in player table!\r\nName not changed.\r\n", GET_NAME(vict));
      send_to_char(buf, ch);
      if(is_file)
        free_char(cbuf);
      return;
    }
    strcpy(oldname, GET_NAME(vict));
    if(player_table[nr].name)
      free(player_table[nr].name);
    CREATE(player_table[nr].name, char, strlen(val_arg) + 1);
    for (i = 0; (*(player_table[nr].name + i) = LOWER(*(val_arg + i))); i++);
    if (get_filename(GET_NAME(vict), f1, CRASH_FILE)) {
      if (get_filename(val_arg, f2, CRASH_FILE)) {
        unlink(f2);
        rename(f1, f2);
      }
    }
    if (get_filename(GET_NAME(vict), f1, REIMB_FILE)) {
      if (get_filename(val_arg, f2, REIMB_FILE)) {
        unlink(f2);
        rename(f1, f2);
      }
    }
    if (get_filename(GET_NAME(vict), f1, ALIAS_FILE)) {
      if (get_filename(val_arg, f2, ALIAS_FILE)) {
        unlink(f2);
        rename(f1, f2);
      }
    }
    if (get_filename(GET_NAME(vict), f1, SAVEMAIL_FILE)) {
      if (get_filename(val_arg, f2, SAVEMAIL_FILE)) {
        unlink(f2);
        rename(f1, f2);
      }
    }
    if(GET_NAME(vict))
      free(GET_NAME(vict));
    GET_NAME(vict)=str_dup(val_arg);
    break;
  case 47:
    GET_REROLLS(vict)=RANGE(0, 200);
    break;
  case 48:
    vict->player_specials->saved.age_add=RANGE(-100, 100);
    break;
  case 49:
    vict->player_specials->saved.inherent_ac_apply=RANGE(0, 1000);
    break;
  case 50:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SNOOP);
    break;
  case 51:
    GET_QP(vict) = RANGE(0, 30000);
    break;
  default:
    sprintf(buf, "Can't set that!");
    setok=0;
    break;
  }
  if (fields[l].type == BINARY) {
    sprintf(buf, "%s %s for %s.\r\n", fields[l].cmd, ONOFF(on),
	    GET_NAME(vict));
    CAP(buf);
  } else if (fields[l].type == NUMBER) {
    sprintf(buf, "%s's %s set to %d.\r\n", GET_NAME(vict),
	    fields[l].cmd, value);
  } else
    strcat(buf, "\r\n");
  send_to_char(CAP(buf), ch);
  if (!is_file && !IS_NPC(vict)) {
    save_char(vict, NOWHERE);
  }
  if(setok) {
    sprintf(buf, "(GC) %s set %s's %s to %s%s.", GET_NAME(ch), (l==46 ? oldname : GET_NAME(vict)), fields[l].cmd, (fields[l].type == BINARY ? ONOFF(on) : (l==41?"******":val_arg)), (is_file?" [FILE]":""));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
  if (is_file) {
    char_to_store(vict, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
    send_to_char("Saved in file.\r\n", ch);
  }
  if(update_wizl)
  {
    char buf[100];
    extern int use_autowiz;
    extern int min_wizlist_lev;
    pid_t getpid(void);
    if (use_autowiz) {
      sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
        WIZLIST_FILE, LVL_HERO, IMMLIST_FILE, (int) getpid());
      mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
      system(buf);
    }
  }
}
ACMD(do_grep)
{
  FILE *fp;
  char field[MAX_INPUT_LENGTH], search_str[MAX_INPUT_LENGTH], *ptr, *ptr2;
  int i, j, index=0, num_lines=0;
  long *lines=NULL;
  char *grep_buf;
  struct grep_struct {
    char *cmd;
    char level;
  } fields[] = {
    { "nothing",	0  },				/* 0 */
    { "buildlog",	LVL_IMMORT },
    { "buildlog1",	LVL_IMMORT },
    { "buildlog2",	LVL_IMMORT },
    { "buildlog3",	LVL_IMMORT },
    { "buildlog4",	LVL_IMMORT },			/* 5 */
    { "buildlog5",	LVL_IMMORT },
    { "buildlog6",	LVL_IMMORT },
    { "buildcmds",	LVL_IMMORT },
    { "builderrs",	LVL_IMMORT },
    { "\n", }
  };
  skip_spaces(&argument);
  if (!*argument) {
    send_to_char("Usage: grep [lines_from_end] <option> <search string>\r\n", ch);
    strcpy(buf, "Grep options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
	sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }
  half_chop(argument, field, search_str);
  if(is_number(field) && (atoi(field) > 0)) {
    num_lines=atoi(field)+1;
    CREATE(lines, long, num_lines);
    for(i=0; i<num_lines; i++)
      lines[i]=0;
    strcpy(argument, search_str);
    half_chop(argument, field, search_str);
  }
  for (i = 0; *(fields[i].cmd) != '\n'; i++)
    if (!strncmp(field, fields[i].cmd, strlen(field)))
      break;
  if (GET_LEVEL(ch) < fields[i].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  switch(i) {
  case 1:
    fp=fopen("../buildlog", "r");
    break;
  case 2:
    fp=fopen("../log/buildlog.1", "r");
    break;
  case 3:
    fp=fopen("../log/buildlog.2", "r");
    break;
  case 4:
    fp=fopen("../log/buildlog.3", "r");
    break;
  case 5:
    fp=fopen("../log/buildlog.4", "r");
    break;
  case 6:
    fp=fopen("../log/buildlog.5", "r");
    break;
  case 7:
    fp=fopen("../log/buildlog.6", "r");
    break;
  case 8:
    fp=fopen("../log/buildcmds", "r");
    break;
  case 9:
    fp=fopen("../log/builderrs", "r");
    break;
  default:
    send_to_char("Unrecognized grep option.\r\n", ch);
    return;
  }
  if(!fp) {
    send_to_char("Could not open file.\r\n", ch);
    return;
  }
  CREATE(grep_buf, char, 1);
  grep_buf[0]=0;
  if(num_lines) {
    while(1) {
      fgets(buf, MAX_STRING_LENGTH-2, fp);
      if(!feof(fp)) {
        lines[index++]=ftell(fp);
        if(index==num_lines)
          index=0;
      }
      else {
        if(index==num_lines)
          index=-1;
        fseek(fp, lines[index+1], SEEK_SET);
        fgets(buf, MAX_STRING_LENGTH-2, fp);
        break;
      }
    }
  }
  else
    fgets(buf, MAX_STRING_LENGTH-2, fp);
  if(subcmd != SCMD_IGREP) {
    while(!feof(fp)) {
      if(strstr(buf, search_str)) {
        for(ptr=buf; *ptr; ptr++);
        for(; ((*ptr == '\n') || (*ptr == '\r') || (!*ptr)); ptr--);
        *(ptr+1) = '\r';
        *(ptr+2) = '\n';
        *(ptr+3) = 0;
        RECREATE(grep_buf, char, strlen(grep_buf)+strlen(buf)+1);
        strcat(grep_buf, buf);
        if(strlen(grep_buf) > MAX_STRING_LENGTH*10) {
          RECREATE(grep_buf, char, strlen(grep_buf)+20);
          strcat(grep_buf, "***OVERFLOW***\r\n");
          break;
        }
      }
      fgets(buf, MAX_STRING_LENGTH-2, fp);
    }
  }
  else {
    for(ptr2=search_str; *ptr2; ptr2++)
      *ptr2=LOWER(*ptr2);
    while(!feof(fp)) {
      for(ptr2=buf; *ptr2; ptr2++)
        *ptr2=LOWER(*ptr2);
      if(strstr(buf, search_str)) {
        for(ptr=buf; *ptr; ptr++);
        for(; ((*ptr == '\n') || (*ptr == '\r') || (!*ptr)); ptr--);
        *(ptr+1) = '\r';
        *(ptr+2) = '\n';
        *(ptr+3) = 0;
        RECREATE(grep_buf, char, strlen(grep_buf)+strlen(buf)+1);
        strcat(grep_buf, buf);
        if(strlen(grep_buf) > MAX_STRING_LENGTH*10) {
          RECREATE(grep_buf, char, strlen(grep_buf)+20);
          strcat(grep_buf, "***OVERFLOW***\r\n");
          break;
        }
      }
      fgets(buf, MAX_STRING_LENGTH-2, fp);
    }
  }
  page_string(ch->desc, grep_buf, 1);
  free(grep_buf);
  fclose(fp);
}
ACMD(do_grant)
{
  int length, cmd1, i, player_i=0;
  char is_file=FALSE;
  char *arg, c[MAX_INPUT_LENGTH], vname[MAX_INPUT_LENGTH];
  struct char_file_u tmp_store;
  struct char_data *vict, *cbuf=NULL;
  struct grant_struct {
    char *command;
    long grant;
    char minimum_level;
  } other_grants[] = {
    { "kill",	GRNT_KILL,	LVL_CIMP },
    { "show",	GRNT_SHOW,	LVL_CIMP },
    { "\n", 0, 0 }
  };
  skip_spaces(&argument);
  if(!*argument)
  {
    send_to_char("Usage: grant [file] <player> <command>\r\n", ch);
    send_to_char("You can grant the following commands:\r\n", ch);
    for (i=0, cmd1 = 0; *cmd_info[cmd1].command != '\n'; cmd1++)
      if ((cmd_info[cmd1].grant) && (GET_LEVEL(ch) >= cmd_info[cmd1].minimum_level)) {
        sprintf(buf, "%-14s ", cmd_info[cmd1].command);
        send_to_char(buf, ch);
        i++;
        if(!(i%5))
          send_to_char("\r\n", ch);
      }
    for(cmd1=0; *other_grants[cmd1].command != '\n'; cmd1++)
      if((other_grants[cmd1].grant) && (GET_LEVEL(ch) >= other_grants[cmd1].minimum_level)) {
        sprintf(buf, "%-14s ", other_grants[cmd1].command);
        send_to_char(buf, ch);
        i++;
        if(!(i%5))
          send_to_char("\r\n", ch);
      }
    if((i%5))
      send_to_char("\r\n", ch);
    return;
  }
  arg = any_one_arg(argument, vname);
  skip_spaces(&arg);
  arg = any_one_arg(arg, c);
  if(!str_cmp(vname, "file")) {
    is_file=TRUE;
    strcpy(vname, c);
    any_one_arg(arg, c);
  }
  if (!*vname || !*c) {
    send_to_char("Who do you wish to grant what?\r\n", ch);
    return;
  }
  if(is_file) {
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    if ((player_i = load_char(vname, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
	free_char(cbuf);
        send_to_char("Doesn't that seem a bit foolish?", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  else if (!(vict = get_player_vis(ch, vname))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  if (GET_LEVEL(vict) >= GET_LEVEL(ch))
    send_to_char("Doesn't that seem a bit foolish?", ch);
  else {
    for (length = strlen(c), cmd1 = 0; *cmd_info[cmd1].command != '\n'; cmd1++)
      if (!strncmp(cmd_info[cmd1].command, c, length))
        if (GET_LEVEL(ch) >= cmd_info[cmd1].minimum_level)
	  break;
    if ((*cmd_info[cmd1].command == '\n') || (!cmd_info[cmd1].grant)) {
      for (cmd1 = 0; *other_grants[cmd1].command != '\n'; cmd1++)
        if (!strncmp(other_grants[cmd1].command, c, length))
          if (GET_LEVEL(ch) >= other_grants[cmd1].minimum_level)
	    break;
      if ((*other_grants[cmd1].command == '\n') || (!other_grants[cmd1].grant)) {
        send_to_char("You cannot grant that command.\r\n", ch);
        return;
      }
      if(GRNT_FLAGGED(vict, other_grants[cmd1].grant)) {
        REMOVE_BIT(GRNT_FLAGS(vict), other_grants[cmd1].grant);
        send_to_char(OK, ch);
        sprintf(buf, "The %s grant has been taken away from you!\r\n", other_grants[cmd1].command);
        send_to_char(buf, vict);
        sprintf(buf, "(GC) %s has taken the %s grant from %s.", GET_NAME(ch), other_grants[cmd1].command, GET_NAME(vict));
        mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
      }
      else {
        if (GET_LEVEL(vict) < LVL_HERO)
          send_to_char("You can't grant to mortals!\r\n", ch);
        else {
          SET_BIT(GRNT_FLAGS(vict), other_grants[cmd1].grant);
          send_to_char(OK, ch);
          sprintf(buf, "You have been granted %s!\r\n", other_grants[cmd1].command);
          send_to_char(buf, vict);
          sprintf(buf, "(GC) %s has granted %s to %s.", GET_NAME(ch), other_grants[cmd1].command, GET_NAME(vict));
          mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
        }
      }
    }
    else {
      if(GRNT_FLAGGED(vict, cmd_info[cmd1].grant)) {
        REMOVE_BIT(GRNT_FLAGS(vict), cmd_info[cmd1].grant);
        send_to_char(OK, ch);
        sprintf(buf, "The %s grant has been taken away from you!\r\n", cmd_info[cmd1].command);
        send_to_char(buf, vict);
        sprintf(buf, "(GC) %s has taken the %s grant from %s.", GET_NAME(ch), cmd_info[cmd1].command, GET_NAME(vict));
        mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
      }
      else {
        if (GET_LEVEL(vict) < LVL_HERO)
          send_to_char("You can't grant to mortals!\r\n", ch);
        else {
          SET_BIT(GRNT_FLAGS(vict), cmd_info[cmd1].grant);
          send_to_char(OK, ch);
          sprintf(buf, "You have been granted %s!\r\n", cmd_info[cmd1].command);
          send_to_char(buf, vict);
          sprintf(buf, "(GC) %s has granted %s to %s.", GET_NAME(ch), cmd_info[cmd1].command, GET_NAME(vict));
          mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
        }
      }
    }
  }
  if(is_file) {
    char_to_store(vict, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free(cbuf);
  }
  return;
}
static char *logtypes[] = {
"off", "brief", "normal", "complete", "\n"};
ACMD(do_syslog)
{
  int tp;
  one_argument(argument, arg);
  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
         (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));
  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}
ACMD(do_arena)
{
  char arg[MAX_INPUT_LENGTH];
  skip_spaces(&argument);
  argument=one_argument(argument, arg);
  if(!*arg) {
    switch(arena_restrict) {
      case ARENA_OPEN:
        send_to_char("The arena is open to everyone.\r\n", ch);
        break;
      case ARENA_NOALL:
        send_to_char("The arena is closed to challenging all.\r\n", ch);
        break;
      case ARENA_NOGROUP:
        send_to_char("The arena is only open to one-on-one challenges.\r\n", ch);
        break;
      case ARENA_CLOSED:
        send_to_char("The arena is closed.\r\n", ch);
        break;
      default:
        send_to_char("The arena appears to be screwed up.\r\n", ch);
        break;
    }
  }
  if(str_cmp(arg, "restrict")) {
    send_to_char("usage: arena [restrict <closed | groups | all | open>]", ch);
    return;
  }
  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!strn_cmp("closed", arg, strlen(arg))) {
    send_to_char("The arena is closed.\r\n", ch);
    arena_restrict=ARENA_CLOSED;
    sprintf(buf, "(GC) %s changed the arena restriction to closed.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(GET_LEVEL(ch), LVL_ASST), TRUE);
  }
  else if(!strn_cmp("groups", arg, strlen(arg))) {
    send_to_char("The arena is now open only to one-on-one challenges.\r\n", ch);
    arena_restrict=ARENA_NOGROUP;
    sprintf(buf, "(GC) %s changed the arena restriction to groups.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(GET_LEVEL(ch), LVL_ASST), TRUE);
  }
  else if(!strn_cmp("all", arg, strlen(arg))) {
    send_to_char("The arena is now closed to challenging all.\r\n", ch);
    arena_restrict=ARENA_NOALL;
    sprintf(buf, "(GC) %s changed the arena restriction to all.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(GET_LEVEL(ch), LVL_ASST), TRUE);
  }
  else if(!strn_cmp("open", arg, strlen(arg))) {
    send_to_char("The arena is open to everyone.\r\n", ch);
    arena_restrict=ARENA_OPEN;
    sprintf(buf, "(GC) %s changed the arena restriction to open.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(GET_LEVEL(ch), LVL_ASST), TRUE);
  }
  else {
    send_to_char("usage: arena [restrict <closed | groups | all | open>]", ch);
  }
}
ACMD(do_assassin)
{
  struct char_data *vict;
  skip_spaces(&argument);
  two_arguments(argument, arg, buf);
  if((!*arg)||(!*buf)) {
    send_to_char("Usage: assassin <player> <accept | reject>", ch);
    return;
  }
  if(!(vict=get_player_vis(ch, arg))) {
    send_to_char("No such player around.\r\n", ch);
    return;
  }
  if((GET_LEVEL(ch) <= GET_LEVEL(vict)) && (ch!=vict)) {
    send_to_char("You may not change the assassin status of a character that level!\r\n", ch);
    return;
  }
  if(!strn_cmp("accept", buf, strlen(buf))) {
    if(!PRF_FLAGGED(vict, PRF_REQ_ASS)) {
      sprintf(buf, "You can't. %s is not requesting to be an assassin.\r\n", GET_NAME(vict));
      send_to_char(buf, ch);
      return;
    }
    SET_BIT(PLR_FLAGS(vict), PLR_ASSASSIN);
    REMOVE_BIT(PRF_FLAGS(vict), PRF_REQ_ASS);
    sprintf(buf, "%s is now an assassin.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    sprintf(buf, "%s has accepted you in to the Circle of Assassins.\r\n", PERS(ch, vict));
    send_to_char(buf, vict);
    sprintf(buf, "(GC) %s made %s an assassin.", GET_NAME(ch), GET_NAME(vict));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
  else if(!strn_cmp("reject", buf, strlen(buf))) {
    if(!PLR_FLAGGED(vict, PLR_ASSASSIN)) {
      sprintf(buf, "%s is not an assassin.\r\n", GET_NAME(vict));
      send_to_char(buf, ch);
      return;
    }
    REMOVE_BIT(PLR_FLAGS(vict), PLR_ASSASSIN);
    sprintf(buf, "%s is no longer an assassin.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    sprintf(buf, "%s has revoked your membership in the Circle of Assassins.\r\n", PERS(ch, vict));
    send_to_char(buf, vict);
    sprintf(buf, "(GC) %s revoked %s's assassinship.", GET_NAME(ch), GET_NAME(vict));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
  else {
    send_to_char("You may only specify ACCEPT or REJECT.\r\n", ch);
  }
}
ACMD(do_remort)
{
  int i;
  struct char_data *vict;
  extern int use_autowiz;
  extern int min_wizlist_lev;
  pid_t getpid(void);
  void do_start(struct char_data * ch);
  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!(vict=get_player_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  if(GET_LEVEL(vict) < LVL_HERO) {
    send_to_char("They already ARE a mort.\r\n", ch);
    return;
  }
  if(!IMM_AFF_IMM(ch, vict, FALSE)) {
    send_to_char("You can't do that!\r\n", ch);
    return;
  }
  for(i=0; i<NUM_CLASSES; i++) {
    GET_CLASS_LEVEL(vict, i)=0;
  }
  GET_NUM_CLASSES(vict)=1;
  GET_CLASS_BITVECTOR(vict)=(1 << GET_CLASS(vict));
  GRNT_FLAGS(vict)=0;
  vict->real_abils.str=18;
  vict->real_abils.str_add=0;
  vict->real_abils.intel=17;
  vict->real_abils.wis=17;
  vict->real_abils.dex=17;
  vict->real_abils.con=17;
  vict->real_abils.cha=17;
  vict->player_specials->saved.inherent_ac_apply=0;
  vict->player_specials->saved.age_add=0;
  vict->player_specials->saved.num_rerolls=0;
  vict->player_specials->saved.olc_min1 = vict->player_specials->saved.olc_min2 = 0;
  vict->player_specials->saved.olc_max1 = vict->player_specials->saved.olc_max2 = 0;
  vict->player_specials->saved.new_mana = vict->points.max_mana = 0;
  vict->points.mana = GET_MAX_MANA(vict);
  vict->player_specials->saved.new_hit = vict->points.hit = GET_MAX_HIT(vict) = 0;
  vict->player_specials->saved.new_move = vict->points.max_move = 0;
  vict->points.move = GET_MAX_MOVE(vict);
  vict->points.armor = 100;
  for (i = 1; i <= MAX_SKILLS; i++) {
    SET_SKILL(vict, i, 0)
  }
  vict->char_specials.saved.affected_by = 0;
  for (i = 0; i < 5; i++)
    GET_SAVE(vict, i) = 0;
  for (i = 0; i < 3; i++)
    GET_COND(vict, i) = 24;
  GET_LOADROOM(vict) = NOWHERE;
  for(i=0; i<MAX_REMEMBER; i++)
    GET_REMEMBER(vict, i)=NOWHERE;
  vict->player_specials->saved.rerolling=0;
  vict->player_specials->saved.reroll_level=0;
  PRF_FLAGS(vict)=0;
  REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
  affect_total(vict);
  do_start(vict);
  save_char(vict, NOWHERE);
  sprintf(buf, "(GC) %s remorted %s.", GET_NAME(ch), GET_NAME(vict));
  mudlog(buf, BRF, MAX(LVL_ASST, GET_INVIS_LEV(ch)), TRUE);
  if (use_autowiz) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
            WIZLIST_FILE, LVL_HERO, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}
ACMD(do_reimb)
{
  int player_i=0, i;
  struct char_data *vict, *cbuf=NULL;
  struct char_file_u tmp_store;
  int reimb_load(struct char_data * ch, struct char_data *reimber, int force);
  void Crash_quietload(struct char_data * ch);
  void Crash_crashsave(struct char_data * ch);
  skip_spaces(&argument);
  argument=one_argument(argument, arg);
  if(!str_cmp(arg, "file")) {
    one_argument(argument, arg);
    if(!*arg) {
      send_to_char("You must specify a player.\r\n", ch);
      return;
    }
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    if ((player_i = load_char(arg, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
    Crash_quietload(vict);
    if(reimb_load(vict, ch, subcmd)) {
      sprintf(buf, "Could not reimb %s.\r\n", arg);
      send_to_char(buf, ch);
    }
    else {
      sprintf(buf, "(GC) %s %sreimbed %s [FILE].", GET_NAME(ch), (subcmd?"force-":""), arg);
      mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
    }
    Crash_crashsave(vict);
    for(i=0; i<NUM_WEARS; i++)
      if(GET_EQ(vict, i))
        extract_obj(GET_EQ(vict, i));
    while(vict->carrying)
      extract_obj(vict->carrying);
    char_to_store(vict, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free(cbuf);
    return;
  }
  if(!(vict=get_player_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  if(reimb_load(vict, ch, subcmd)) {
    sprintf(buf, "Could not reimb %s.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
  }
  else {
    sprintf(buf, "(GC) %s %sreimbed %s.", GET_NAME(ch), (subcmd?"force-":""), GET_NAME(vict));
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
}
ACMD(do_wizupdate)
{
  char buf[100];
  extern int use_autowiz;
  extern int min_wizlist_lev;
  pid_t getpid(void);
  if (use_autowiz) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
            WIZLIST_FILE, LVL_HERO, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}
ACMD(do_backtrace)
{
  int room, i, dir, overflow=0;
  if(!ch->desc)
    return;
  skip_spaces(&argument);
  if(!*argument) {
    room=ch->in_room;
  }
  else {
    one_argument(argument, arg);
    if(!is_number(arg)) {
      send_to_char("Usage: backtrace <room_num>\r\n", ch);
      return;
    }
    if((room=real_room(atoi(arg))) < 0) {
      send_to_char("No such room.\r\n", ch);
      return;
    }
  }
  sprintf(buf, "Rooms with exits leading to room %d:\r\n", world[room].number);
  for(i=0; i<=top_of_world; i++) {
    for(dir=0; dir<NUM_OF_DIRS; dir++) {
      if(world[i].dir_option[dir]&&(world[i].dir_option[dir]->to_room==room)) {
        sprintbit(world[i].dir_option[dir]->exit_info, exit_bits, buf2);
        sprintf(buf1, "%d (%s), %s %s, Key[%5d], Keywrd: %s\r\n", world[i].number,
                world[i].name, dirs[dir], buf2, world[i].dir_option[dir]->key,
                world[i].dir_option[dir]->keyword ? world[i].dir_option[dir]->keyword : "None");
        if(strlen(buf) > MAX_STRING_LENGTH-150) {
          strcat(buf, "***OVERFLOW***\r\n");
          overflow=1;
          break;
        }
        else {
          strcat(buf, buf1);
        }
      }
    }
    if(overflow)
      break;
  }
  page_string(ch->desc, buf, 1);
}