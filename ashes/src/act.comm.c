/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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
#include "screen.h"

#define MAX_VC_LENGTH 15
#define MAX_VCS 100

char vc_index[MAX_VCS][MAX_VC_LENGTH];

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern int MOBSay;

void mprog_speech_trigger(char *txt, struct char_data *mob);
struct char_data *get_soc_mes(struct char_data *ch, char *argument, char *act_ch, char *act_world, char *act_tch);

ACMD(do_say)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
  else {
    sprintf(buf, "$n says, '%s'", argument);
    MOBTrigger=FALSE;
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You say, '%s'", argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }
    if(MOBSay) {
      mprog_speech_trigger(argument, ch);
    }
  }
}


ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are not the member of a group!\r\n", ch);
    return;
  }
  if (!*argument)
    send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
  else {
    if (ch->master)
      k = ch->master;
    else
      k = ch;

    sprintf(buf, "$n tells the group, '%s'", argument);

    if (IS_AFFECTED(k, AFF_GROUP) && (k != ch)) {
      send_to_char(CCMAG(k, C_NRM), k);
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
      send_to_char(CCNRM(k, C_NRM), k);
    }
    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch)) {
        send_to_char(CCMAG(f->follower, C_NRM), f->follower);
	act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);
        send_to_char(CCNRM(f->follower, C_NRM), f->follower);
      }

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      send_to_char(CCMAG(ch, C_CMP), ch);
      sprintf(buf, "You tell the group, '%s'", argument);
      act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      send_to_char(CCNRM(ch, C_CMP), ch);
    }
  }
}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  struct char_data *tch;

  send_to_char(CCBRED(vict, C_NRM), vict);
  sprintf(buf, "$n tells you, '%s'", arg);
  act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(vict, C_NRM), vict);

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    send_to_char(CCBRED(ch, C_CMP), ch);
    sprintf(buf, "You tell $N, '%s'", arg);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    send_to_char(CCNRM(ch, C_CMP), ch);
  }

  if(!IS_NPC(ch))
    GET_LAST_TELL(vict) = GET_IDNUM(ch);

  if (GET_FORWARD_TELL(vict) != NOBODY) {
    for(tch=character_list;
        (tch!=NULL) && (GET_IDNUM(tch)!=GET_FORWARD_TELL(vict));
        tch = tch->next);
    if (tch == NULL)
      GET_FORWARD_TELL(vict)=NOBODY;
    else if (IS_NPC(tch) || tch->desc) {
      if((!PLR_FLAGGED(tch, PLR_WRITING)) && (!PLR_FLAGGED(tch, PLR_BUILDING))) {
        if((PRF_FLAGGED(tch, PRF_NOTELL) || ROOM_FLAGGED(tch->in_room, ROOM_SOUNDPROOF)) && (GET_LEVEL(ch) < LVL_ASST))
          act("$E can't hear your forward.", FALSE, vict, 0, tch, TO_CHAR | TO_SLEEP);
        else {
          send_to_char(CCBRED(tch, C_NRM), tch);
          sprintf(buf, "%s tells %s, '%s'\r\n", PERS(ch, vict), PERS(vict, tch), arg);
          send_to_char(buf, tch);
          send_to_char(CCNRM(tch, C_NRM), tch);
        }
      }
    }
  }
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (!(vict = get_player_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if ((ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) && (GET_LEVEL(ch) < LVL_ASST))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)	/* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.",
	FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_BUILDING))
    act("$E's building right now; try again later.",
	FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if ((PRF_FLAGGED(vict, PRF_NOTELL) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF)) && (GET_LEVEL(ch) < LVL_ASST))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else {
    perform_tell(ch, vict, buf2);
    if((!IS_NPC(vict)) && (PRF_FLAGGED(vict, PRF_AFK))) {
      sprintf(buf1, "%s is AFK and may not receive your message.\r\n", GET_NAME(vict));
      send_to_char(buf1, ch);
    }
  }
}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */
				     
    while (tch != NULL && GET_IDNUM(tch) != GET_LAST_TELL(ch))
      tch = tch->next;

    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else if (PRF_FLAGGED(ch, PRF_NOTELL))
      send_to_char("You can't tell other people while you have notell on.\r\n", ch);
    else if ((ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) && (GET_LEVEL(ch) < LVL_ASST))
      send_to_char("The walls seem to absorb your words.\r\n", ch);
    else if (!IS_NPC(tch) && !tch->desc)	/* linkless */
      act("$E's linkless at the moment.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else if (PLR_FLAGGED(tch, PLR_WRITING))
      act("$E's writing a message right now; try again later.",
          FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else if (PLR_FLAGGED(tch, PLR_BUILDING))
      act("$E's building right now; try again later.",
          FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else if ((PRF_FLAGGED(tch, PRF_NOTELL) || ROOM_FLAGGED(tch->in_room, ROOM_SOUNDPROOF)) && (GET_LEVEL(ch) < LVL_ASST))
      act("$E can't hear you.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else {
      perform_tell(ch, tch, argument);
      if((!IS_NPC(tch)) && (PRF_FLAGGED(tch, PRF_AFK))) {
        sprintf(buf1, "%s is AFK and may not receive your message.\r\n", PERS(tch, ch));
        send_to_char(buf1, ch);
      }
    }
  }
}


ACMD(do_forward)
{
  struct char_data *vict;

  if(IS_NPC(ch)) {
    send_to_char("Mobs can't forward tells!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if(!*arg) {
    GET_FORWARD_TELL(ch)=NOBODY;
    send_to_char("You will not forward tells to anyone.\r\n", ch);
  }
  else {
    if (!(vict = get_player_vis(ch, arg)))
      send_to_char(NOPERSON, ch);
    else if (ch == vict)
      send_to_char("You can't forward tells to youself!\r\n", ch);
    else {
      GET_FORWARD_TELL(ch)=GET_IDNUM(vict);
      sprintf(buf, "You will now forward tells to %s\r\n", GET_NAME(vict));
      send_to_char(buf, ch);
    }
  }
}


ACMD(do_spec_comm)
{
  struct char_data *vict;
  char *action_sing, *action_plur, *action_others;

  /* to keep pets, etc from being ordered do these things */
  if (!ch->desc)
    return;

  if (subcmd == SCMD_WHISPER) {
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
  } else {
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
    send_to_char(buf, ch);
  } else if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char(NOPERSON, ch);
  }
  else if (vict == ch)
    send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else {
    sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}


#define MAX_NOTE_LENGTH 1000	/* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper = 0, *pen = 0;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {		/* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {		/* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", penname);
      send_to_char(buf, ch);
      return;
    }
  } else {		/* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {	/* oops, a pen.. */
      pen = paper;
      paper = 0;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
	      papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen)
      paper = GET_EQ(ch, WEAR_HOLD);
    else
      pen = GET_EQ(ch, WEAR_HOLD);
  }


  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
    /* we can write - hooray! */
    send_to_char("Write your note.  End with '@' on a new line.\r\n", ch);
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
    sprintf(buf1, "\007\007*someone* %s\r\n", buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_ASST) {
	for (d = descriptor_list; d; d = d->next)
	  if (!d->connected && d->character)
            send_to_char((CAN_SEE(d->character, ch) ? buf : buf1), d->character);
      } else
	send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    if ((vict = get_char_vis(ch, arg)) != NULL) {
      send_to_char((CAN_SEE(vict, ch) ? buf : buf1), vict);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	send_to_char(OK, ch);
      else
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
      return;
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  extern int level_can_shout;
  extern int holler_move_cost;
  int action=0;
  struct char_data *vict=NULL;
  struct descriptor_data *i;
  char color_on[24];
  char to_ch[MAX_INPUT_LENGTH+100];
  char to_world[MAX_INPUT_LENGTH+100];
  char to_vict[MAX_INPUT_LENGTH+100];

  /* Array of flags which must _not_ be set in order for comm to be heard */
  static int channels[] = {
    0,
    PRF_DEAF,
    PRF_NOGOSS,
    PRF_NOAUCT,
    PRF_NOGRATZ,
    PRF_NOMUS,
    PRF_NOASAY,
    PRF_NOESP,
    PRF_NOFRAN,
    0
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string.
   */
  static char *com_msgs[][4] = {
    {"You cannot holler!!\r\n",
      "holler",
      "",
    KYEL},

    {"You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
    KYEL},

    {"You cannot gossip!!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KYEL},

    {"You cannot auction!!\r\n",
      "auction",
      "You aren't even on the channel!\r\n",
    KBBLU},

    {"You cannot congratulate!\r\n",
      "congrat",
      "You aren't even on the channel!\r\n",
    KGRN},

    {"You cannot sing!!\r\n",
      "sing",
      "You aren't even on the channel!\r\n",
    KCYN},

    {"You cannot talk on the assassing channel!\r\n",
     "assassin-say",
     "You aren't even on the channel!\r\n",
    KBMAG},
    {"You cannot do that in your present state!\r\n",
     "espanol",
     "You aren't even on the channel!\r\n",
    KMAG},
    {"You cannot do that in your present state!\r\n",
     "francais",
     "You aren't even on the channel!\r\n",
    KMAG}
  };

  /* to keep pets, etc from being ordered to shout */
  if((!ch->desc) && (ch->master))
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
    send_to_char("The walls seem to absorb your words.\r\n", ch);
    return;
  }
  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < level_can_shout) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
	    level_can_shout, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd]) && (!IS_NPC(ch))) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }
  /* skip leading spaces */
  skip_spaces(&argument);

  /* make sure that there is something there to say! */
  if (!*argument) {
    sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
	    com_msgs[subcmd][1], com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  if (subcmd == SCMD_ASAY) {
    if((!PLR_FLAGGED(ch, PLR_ASSASSIN)) && (GET_LEVEL(ch) < LVL_ASST)) {
      send_to_char("You's better leave that channel alone - you're not an assassin.\r\n", ch);
      return;
    }
  }
  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < holler_move_cost) {
      send_to_char("You're too exhausted to holler.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= holler_move_cost;
  }
  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  /* Check for social/emote */
  if(*argument=='*') {
    action=1;
    vict=get_soc_mes(ch, argument+1, to_ch, to_world, to_vict);
  }

  /* first, set up strings to be given to the communicator */
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    if (COLOR_LEV(ch) >= C_CMP) {
      if(action) {
        strcpy(buf, com_msgs[subcmd][1]);
        CAP(buf);
        sprintf(buf1, "%s%s: %s%s", color_on, buf, to_ch, KNRM);
      }
      else
        sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1], argument, KNRM);
    }
    else {
      if(action) {
        sprintf(buf1, "%s: %s", com_msgs[subcmd][1], to_ch);
        CAP(buf1);
      }
      else
        sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
    }
    act(buf1, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  }

  if(action) {
    sprintf(buf, "%s: ", com_msgs[subcmd][1]);
    CAP(buf);
  }
  else
    sprintf(buf, "$n %ss, '%s'", com_msgs[subcmd][1], argument);

  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && i != ch->desc && i->character &&
	!PRF_FLAGGED(i->character, channels[subcmd]) &&
	!PLR_FLAGGED(i->character, PLR_WRITING) &&
	!PLR_FLAGGED(i->character, PLR_BUILDING) &&
	!ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

      if ((subcmd == SCMD_SHOUT) &&
	  ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
	   GET_POS(i->character) < POS_RESTING))
	continue;

      if ((subcmd == SCMD_ASAY) && (!PLR_FLAGGED(i->character, PLR_ASSASSIN)) && (GET_LEVEL(i->character) < LVL_ASST))
        continue;

      if (COLOR_LEV(i->character) >= C_NRM)
	send_to_char(color_on, i->character);
      if(action) {
        if(i->character==vict) {
          sprintf(buf1, "%s%s", buf, to_vict);
          act(buf1, FALSE, ch, NULL, vict, TO_VICT | TO_SLEEP);
        }
        else {
          sprintf(buf1, "%s%s", buf, to_world);
          act(buf1, FALSE, ch, (struct obj_data *)i->character, vict, TO_OBJ | TO_SLEEP);
        }
      }
      else
        act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      if (COLOR_LEV(i->character) >= C_NRM)
	send_to_char(KNRM, i->character);
    }
  }
}


void arenasay(char *message)
{
  struct descriptor_data *d;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];

  sprintf(buf, "%sARENA:%s %s\r\n", KBMAG, KNRM, message);
  sprintf(buf1, "ARENA: %s\r\n", message);

  for(d=descriptor_list; d; d=d->next) {
    if ((!d->connected) && d->character &&
        (!PLR_FLAGGED(d->character, PLR_WRITING)) &&
        (!PLR_FLAGGED(d->character, PLR_BUILDING)) &&
        (!PRF_FLAGGED(d->character, PRF_NOARENA))) {
      if(COLOR_LEV(d->character) >= C_NRM)
        send_to_char(buf, d->character);
      else
        send_to_char(buf1, d->character);
    }
  }
}


ACMD(do_qcomm)
{
  struct descriptor_data *i;

  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char("You aren't even part of the quest!\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char("You cannot use the quest channel.\r\n", ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
	    CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_QSAY)
	sprintf(buf, "You quest-say, '%s'", argument);
      else
	strcpy(buf, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }

    if (subcmd == SCMD_QSAY)
      sprintf(buf, "$n quest-says, '%s'", argument);
    else
      strcpy(buf, argument);

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i != ch->desc &&
	  PRF_FLAGGED(i->character, PRF_QUEST) &&
          !PLR_FLAGGED(i->character, PLR_WRITING) &&
          !PLR_FLAGGED(i->character, PLR_BUILDING))
	act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
  }
}



void vcact(char *str, int hide_invisible, struct char_data *ch, void *vict_obj, int type, int vcnum)
{
  register char *strp, *point, *i;
  struct char_data *to;
  struct descriptor_data *tmpdesc;
  static char buf[MAX_INPUT_LENGTH];


  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING) && (tmpdesc->character->char_specials.vc == vcnum)) {
      to = tmpdesc->character;
      if (((to != ch) || (type == TO_CHAR)) && !((type == TO_NOTVICT) && ((to == (struct char_data*) vict_obj) || (to == ch)))) {
        if (((type == TO_CHAR) && (to == ch))  || (type != TO_CHAR)) {
          if (((type == TO_VICT) && (to == (struct char_data*)vict_obj)) || (type != TO_VICT)) {
            for (strp = str, point = buf; ; ) {
              if (*strp == '$') {
                switch (*(++strp)) {
                case 'n':
                  i = GET_NAME(ch);
                  break;
                case 'N':
                  i = GET_NAME((struct char_data *) vict_obj);
                  break;
                case 'm':
                  i = HMHR(ch);
                  break;
                case 'M':
                  i = HMHR((struct char_data *) vict_obj);
                  break;
                case 's':
                  i = HSHR(ch);
                  break;
                case 'S':
                  i = HSHR((struct char_data *) vict_obj);
                  break;
                case 'e':
                  i = HSSH(ch);
                  break;
                case 'E':
                  i = HSSH((struct char_data *) vict_obj);
                  break;
                case 'O':
                  i = OBJN((struct obj_data *) vict_obj, to);
                  break;
                case 'P':
                  i = OBJS((struct obj_data *) vict_obj, to);
                  break;
                case 'A':
                  i = SANA((struct obj_data *) vict_obj);
                  break;
                case 'T':
                  i = (char *) vict_obj;
                  break;
                case 'F':
                  i = fname((char *) vict_obj);
                  break;
                case '$':
                  i = "$";
                  break;
                default:
                  i = "";
                  log("SYSERR: Illegal $-code to vcact():");
                  strcpy(buf1, "SYSERR: ");
                  strcat(buf1, str);
                  log(buf1);
                  break;
                }
                while ((*point = *(i++)))
                  ++point;
                ++strp;
              }
              else if (!(*(point++) = *(strp++)))
                break;
            }

            *(--point) = '\n';
            *(++point) = '\r';
            *(++point) = '\0';
            if(to->desc) {
              sprintf(buf1, "%s(%s) %s%s%s\n\r", VC_CHANNAME_COL, vc_index[vcnum], VC_ACT_COL, buf, KNRM);
              sprintf(buf2, "(%s) %s\n\r", vc_index[vcnum], buf);
              if (COLOR_LEV(to) >= 2) SEND_TO_Q(CAP(buf1), to->desc);
              else SEND_TO_Q(CAP(buf2), to->desc);
	    }
          }
        }
      }
    }
  }
}


void init_vcs(void)
{
   int i;
   
   for (i = 0; i != MAX_VCS; i++) {
      strcpy(vc_index[i], "\n");
   }
}


int available_virtual_channel(void)
{
   int i;

   for (i = 0; str_cmp(vc_index[i], "\n"); i++);

   if (i > MAX_VCS) i = -1;

   return i;
}

int start_virtual_channel(char channelname[MAX_VC_LENGTH])
{
   int chan_num;

   chan_num = available_virtual_channel();

   if(chan_num > -1) strcpy(vc_index[chan_num], channelname);

   return chan_num;
}

void end_virtual_channel(int chan_num)
{
  struct descriptor_data *tmpdesc;

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected) {
      if (tmpdesc->character->char_specials.vc == chan_num) {
        tmpdesc->character->char_specials.vc = -1;
        tmpdesc->character->char_specials.vcs = FALSE;
        tmpdesc->character->char_specials.vcm = FALSE;
      }
    }
  }

  strcpy(vc_index[chan_num], "\n");
}


ACMD(do_vcjoin)
{
  char chan_name[MAX_INPUT_LENGTH];
  int chan_num;
  struct descriptor_data *tmpdesc;

  ACMD(do_vcdepart);

  chan_num = 0;

  if(argument)
    skip_spaces(&argument);

  if ((!argument)||(!(*argument)))  {
    if(ch->char_specials.vci >= 0) {
      if(ch->char_specials.vc >= 0)
        do_vcdepart(ch, "", 0, 0);
      chan_num=ch->char_specials.vci;
      sprintf(buf, "Now talking on virtual channel %s\n\r", vc_index[chan_num]);
      ch->char_specials.vc = chan_num;
      ch->char_specials.vcm = FALSE;
      ch->char_specials.vcs = FALSE;
      ch->char_specials.vci = -1;
      sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has joined this virtual channel.%s\n\r\n\r", VC_CHANNAME_COL, 
              vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(ch), KNRM);
      sprintf(buf1, "(%s) [SYSTEM]: %s has joined this virtual channel.\n\r\n\r", vc_index[ch->char_specials.vc],
              GET_NAME(ch));
      for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
        if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
          if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
            if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
            else send_to_char(buf1, tmpdesc->character);
          }
        }
      }
    }
    else {
      send_to_char("Usage: vcjoin <channelname>\n\r",ch);
    }
    return;
  }

  one_argument(argument, chan_name);
  chan_name[MAX_VC_LENGTH-1]=0;

  while (chan_num != MAX_VCS && str_cmp(chan_name, vc_index[chan_num])) chan_num++;

  if (chan_num == MAX_VCS)  {
    if (ch->char_specials.vc > -1)  {
      send_to_char("Departing old virtual channel.\n\r\n\r",ch);
      if (ch->char_specials.vcm) {
        sprintf(buf, "%s(%s) %s[SYSTEM]: %sModerator has exited the channel. Closing virtual channel.%s\n\r\n\r", VC_CHANNAME_COL,
                vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, KNRM);
        sprintf(buf1, "(%s) [SYSTEM]: Moderator has exited the channel. Closing virtual channel.\n\r\n\r",
                vc_index[ch->char_specials.vc]);
        for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
          if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
            if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
              if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
              else send_to_char(buf1, tmpdesc->character);
            }
          }
        }
        end_virtual_channel(ch->char_specials.vc);
      }
      else {
        sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has exited the channel.%s\n\r\n\r",VC_CHANNAME_COL,
                vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(ch), KNRM);
        sprintf(buf1, "(%s) [SYSTEM]: %s has exited the chanel.\n\r\n\r", vc_index[ch->char_specials.vc],
                GET_NAME(ch));
        for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
          if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
            if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
              if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
              else send_to_char(buf1, tmpdesc->character);
            }
          }
        }
      }
    }

    send_to_char("Starting new virtual channel.\n\r\n\r",ch);
    chan_num = start_virtual_channel(chan_name);
    if(chan_num > -1) {
      ch->char_specials.vc = chan_num;
      ch->char_specials.vcm = TRUE;
      ch->char_specials.vcs = FALSE;
      if (ch->char_specials.vci == chan_num)
        ch->char_specials.vci = -1;
      sprintf(buf, "%s(%s) %s[SYSTEM]: %sBeginning new channel. Enjoy!%s\n\r\n\r", VC_CHANNAME_COL,
              vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, KNRM);
      sprintf(buf1, "(%s) [SYSTEM]: Beginning new channel. Enjoy!\n\r\n\r", vc_index[ch->char_specials.vc]);
      for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
        if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
          if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
            if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
            else send_to_char(buf1, tmpdesc->character);
          }
        }
      }
    }
    else {
      send_to_char("Error creating channel: too many channels open.\n\rPlease resport this to a coder.\r\n",ch);
    }
    return;
  }

  if (ch->char_specials.vci != chan_num) {
    send_to_char("You are not currently invited to that virtual channel.\n\r\n\r",ch);
    return;
  }

  if(ch->char_specials.vc > -1) {
    send_to_char("Departing old virtual channel.\n\r\n\r",ch);
    if (ch->char_specials.vcm) {
      sprintf(buf, "%s(%s) %s[SYSTEM]: %sModerator has exited the channel. Closing virtual channel.%s\n\r\n\r", VC_CHANNAME_COL,
              vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, KNRM);
      sprintf(buf1, "(%s) [SYSTEM]: Moderator has exited the channel. Closing virtual channel.\n\r\n\r",
              vc_index[ch->char_specials.vc]);
      for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
        if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING))
        if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
          if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
          else send_to_char(buf1, tmpdesc->character);
        }
      }
      end_virtual_channel(ch->char_specials.vc);
    }
    else {
      sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has exited the channel.%s\n\r\n\r",VC_CHANNAME_COL,
              vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(ch), KNRM);
      sprintf(buf1, "(%s) [SYSTEM]: %s has exited the chanel.\n\r\n\r", vc_index[ch->char_specials.vc],
              ch->player.name);
      for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
        if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
          if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
            if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
            else send_to_char(buf1, tmpdesc->character);
          }
        }
      }
    }
  }
  sprintf(buf, "Now talking on virtual channel %s\n\r", vc_index[chan_num]);
  ch->char_specials.vc = chan_num;
  ch->char_specials.vcm = FALSE;
  ch->char_specials.vcs = FALSE;
  ch->char_specials.vci = -1;
  sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has joined this virtual channel.%s\n\r\n\r", VC_CHANNAME_COL, 
          vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(ch), KNRM);
  sprintf(buf1, "(%s) [SYSTEM]: %s has joined this virtual channel.\n\r\n\r", vc_index[ch->char_specials.vc],
          GET_NAME(ch));
  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
        else send_to_char(buf1, tmpdesc->character);
      }
    }
  }

  return;
}


ACMD(do_vcsay)
{
  struct descriptor_data *tmpdesc;

  if (ch->char_specials.vc < 0) {
    send_to_char("Uhhh...Being *ON* a virtual channel first *MIGHT* help!\n\r", ch);
    return;
  }

  if (!argument) {
    send_to_char("What, cat got your tongue?\n\r",ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("What, cat got your tongue?\n\r",ch);
    return;
  }

  if (ch->char_specials.vcs) {
    send_to_char("Sorry, but you've been squelched on that virtual channel.\n\r",ch);
    return;
  }

  sprintf(buf1, "(%s) [%s]: %s\n\r", vc_index[ch->char_specials.vc], GET_NAME(ch), argument);
  sprintf(buf2, "%s(%s) %s[%s]: %s%s%s\n\r", VC_CHANNAME_COL, vc_index[ch->char_specials.vc], VC_CHARNAME_COL,
          GET_NAME(ch), VC_TEXT_COL, argument, KNRM);

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM)
          send_to_char(buf2, tmpdesc->character);
        else
          send_to_char(buf1, tmpdesc->character);
      }
    }
  }
}


ACMD(do_vcinvite)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH];

  if (ch->char_specials.vc < 0) {
    send_to_char("Uhh - Being *ON* a virtual channel *MIGHT* help. :)\n\r",ch);
    return;
  }

  if (!argument) {
    send_to_char("Yes, but invite who?\n\r",ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, name);

  if (!*name) {
    send_to_char("Yes, but invite who?\n\r",ch);
    return;
  }

  if (!ch->char_specials.vcm) {
    send_to_char("You need to be the moderator of a channel to invite someone to join.\n\r",ch);
    return;
  }

  if (!(victim = get_player_vis(ch, name))) {
    send_to_char("I don't see that person around anywhere.\n\r",ch);
    return;
  }

  if(ch==victim) {
    send_to_char("What is the point of that?\n\r",ch);
    return;
  }

  sprintf(buf, "%s has been invited to channel %s.\n\r", GET_NAME(victim), vc_index[ch->char_specials.vc]);
  send_to_char(buf, ch);
  sprintf(buf, "%s invites you to join channel \"%s\".\n\r", GET_NAME(ch), vc_index[ch->char_specials.vc]);
  send_to_char(buf, victim);
  victim->char_specials.vci = ch->char_specials.vc;
}


ACMD(do_vcsquelch)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH];
  struct descriptor_data *tmpdesc;

  if (ch->char_specials.vc < 0) {
    send_to_char("Uhh - Being *ON* a virtual channel *MIGHT* help. :)\n\r",ch);
    return;
  }

  if (!ch->char_specials.vcm) {
    send_to_char("You need to be the moderator of a channel to squelch someone.\n\r",ch);
    return;
  }

  if(!argument) {
    send_to_char("Squelch who?\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, name);

  if(!*name) {
    send_to_char("Squelch who?\r\n", ch);
    return;
  }

  if(!(victim = get_player(name))) {
    send_to_char("I don't see that person around anywhere.\n\r",ch);
    return;
  }

  if (victim->char_specials.vc != ch->char_specials.vc) {
    if(!(get_player_vis(ch, name))) {
      send_to_char("I don't see that person around anywhere.\n\r",ch);
      return;
    }
    send_to_char("That person isn't on the same virtual channel.\n\r",ch);
    return;
  }

  if (victim->char_specials.vcs == TRUE) {
    sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has been unsquelched.%s\n\r\n\r", VC_CHANNAME_COL, 
            vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(victim), KNRM);
    sprintf(buf1, "(%s) [SYSTEM]: %s has been unsquelched.\n\r\n\r", vc_index[ch->char_specials.vc],
            GET_NAME(victim));
  }
  else {
    sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has been squelched.%s\n\r\n\r", VC_CHANNAME_COL, 
            vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(victim), KNRM);
    sprintf(buf1, "(%s) [SYSTEM]: %s has been squelched.\n\r\n\r", vc_index[ch->char_specials.vc],
            GET_NAME(victim));
  }

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM)
          send_to_char(buf, tmpdesc->character);
        else
          send_to_char(buf1, tmpdesc->character);
      }
    }
  }

  if (victim->char_specials.vcs == TRUE)
    victim->char_specials.vcs = FALSE;
  else
    victim->char_specials.vcs = TRUE;
}


ACMD(do_vckick)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH];
  struct descriptor_data *tmpdesc;

  if (ch->char_specials.vc < 0) {
    send_to_char("Uhh - Being *ON* a virtual channel *MIGHT* help. :)\n\r",ch);
    return;
  }

  if (!ch->char_specials.vcm) {
    send_to_char("You need to be the moderator of a channel to kick someone.\n\r",ch);
    return;
  }

  if(!argument) {
    send_to_char("Kick who?\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, name);

  if(!*name) {
    send_to_char("Kick who?\r\n", ch);
    return;
  }

  if (!(victim = get_player(name))) {
    send_to_char("I don't see that person around anywhere.\n\r",ch);
    return;
  } 

  if (victim->char_specials.vc != ch->char_specials.vc) {
    if (!(get_player_vis(ch, name))) {
      send_to_char("I don't see that person around anywhere.\n\r",ch);
      return;
    } 
    send_to_char("That person isn't on the same virtual channel.\n\r",ch);
    return;
  }

  sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has been kicked off of this channel.%s\n\r\n\r", VC_CHANNAME_COL,
          vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(victim), KNRM);
  sprintf(buf1, "(%s) [SYSTEM]: %s has been kicked off of this channel.\n\r\n\r", vc_index[ch->char_specials.vc],
          GET_NAME(victim));

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM)
          send_to_char(buf, tmpdesc->character);
        else
          send_to_char(buf1, tmpdesc->character);
      }
    }
  }

  victim->char_specials.vc = -1;
  victim->char_specials.vcm = FALSE;
  victim->char_specials.vcs = FALSE;
}


ACMD(do_vcdepart)
{
  struct descriptor_data *tmpdesc;
  struct char_data *tch;

  if (ch->char_specials.vc < 0) {
    send_to_char("Uhh - Being *ON* a virtual channel *MIGHT* help. :)\n\r",ch);
    return;
  }

  if (ch->char_specials.vcm) {
    sprintf(buf, "%s(%s) %s[SYSTEM]: %sThe moderator has exited the channel. Closing this virtual channel.%s\n\r\n\r",
            VC_CHANNAME_COL, vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, KNRM);
    sprintf(buf1, "(%s) [SYSTEM]: The moderator has exited the channel. Closing this virtual channel.\n\r\n\r",
            vc_index[ch->char_specials.vc]);
    for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
      if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
        if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
          if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
          else send_to_char(buf1, tmpdesc->character);
        }
      }
    }

    for(tch=character_list; tch; tch=tch->next) {
      if((!IS_NPC(tch))&&(tch->char_specials.vci==ch->char_specials.vc))
        tch->char_specials.vci = -1;
    }

    end_virtual_channel(ch->char_specials.vc);
    return;
  }

  sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s has exited the channel.%s\n\r", VC_CHANNAME_COL,
          vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(ch), KNRM);
  sprintf(buf1, "(%s) [SYSTEM]: %s has exited the channel.\n\r", vc_index[ch->char_specials.vc],
          GET_NAME(ch));

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM) send_to_char(buf, tmpdesc->character);
        else send_to_char(buf1, tmpdesc->character);
      }
    }
  }
  ch->char_specials.vc = -1;
  ch->char_specials.vcs = FALSE;
  ch->char_specials.vcm = FALSE;
}


ACMD(do_vclist)
{
  struct char_data *tempch;

  if (ch->char_specials.vc < 0) {
    send_to_char("You aren't even ON a virtual channel!\n\r",ch);
    return;
  }

  sprintf(buf, "People Currently On Virtual Channel \"%s\":\n\r", vc_index[ch->char_specials.vc]);
  send_to_char(buf, ch);
  send_to_char("------------------------------------------------------\n\r",ch);

  for(tempch=character_list; tempch; tempch=tempch->next) {
    if(!IS_NPC(tempch)) {
      if(tempch->char_specials.vc == ch->char_specials.vc) {
        sprintf(buf, "%s%s%s\r\n", GET_NAME(tempch),
                (tempch->char_specials.vcm?" *Moderator*":""),
                (tempch->desc?"":" (linkless)"));
        send_to_char(buf, ch);
      }
    }
  }
}

ACMD(do_vcemote)
{
  struct descriptor_data *tmpdesc;

  if (ch->char_specials.vc < 0) {
    send_to_char("It would help to be *ON* a virtual channel, first. :)\n\r",ch);
    return;
  }

  if (ch->char_specials.vcs) {
    send_to_char("Sorry, you are squelched on this virtual channel.\n\r",ch);
    return;
  }

  if (!argument) {
    send_to_char("Okay, but *what* do you want to virtual-channel-emote?\n\r",ch);
    return;
  }

  skip_spaces(&argument);

  if(!*argument) {
    send_to_char("Okay, but *what* do you want to virtual-channel-emote?\n\r",ch);
    return;
  }

  sprintf(buf, "%s(%s) %s%s %s%s\n\r", VC_CHANNAME_COL, vc_index[ch->char_specials.vc], VC_EMOTE_COL, GET_NAME(ch), argument, KNRM);
  sprintf(buf1, "(%s) %s %s\n\r", vc_index[ch->char_specials.vc], GET_NAME(ch), argument);

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM)
          send_to_char(buf, tmpdesc->character);
        else
          send_to_char(buf1, tmpdesc->character);
      }
    }
  }
}


ACMD(do_vcmoderator)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH];
  struct descriptor_data *tmpdesc;

  if (ch->char_specials.vc < 0) {
    send_to_char("Uhh - Being *ON* a virtual channel *MIGHT* help. :)\n\r",ch);
    return;
  }

  if (!ch->char_specials.vcm) {
    send_to_char("You need to be the moderator of a channel to change ownership.\n\r",ch);
    return;
  }

  if(!argument) {
    send_to_char("Make who the moderator?\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, name);

  if(!*name) {
    send_to_char("Make who the moderator?\r\n", ch);
    return;
  }

  if (!(victim = get_player(name))) {
    send_to_char("I don't see that person around anywhere.\n\r",ch);
    return;
  }

  if (victim == ch) {
    send_to_char("What is the point of that?\n\r",ch);
    return;
  }

  if (victim->char_specials.vc != ch->char_specials.vc) {
    if (!(get_player_vis(ch, name))) {
      send_to_char("I don't see that person around anywhere.\n\r",ch);
      return;
    } 
    send_to_char("That person isn't on the same virtual channel.\n\r",ch);
    return;
  }

  if (victim->char_specials.vcs == TRUE) {
    send_to_char("That person can't be the moderator, they are squelched.\n\r",ch);
    return;
  }

  sprintf(buf, "%s(%s) %s[SYSTEM]: %s%s is the new moderator of this channel.%s\n\r\n\r", VC_CHANNAME_COL, 
          vc_index[ch->char_specials.vc], VC_CHARNAME_COL, VC_TEXT_COL, GET_NAME(victim), KNRM);
  sprintf(buf1, "(%s) [SYSTEM]: %s is the new moderator of this channel.\n\r\n\r", vc_index[ch->char_specials.vc],
          GET_NAME(victim));

  for (tmpdesc = descriptor_list; tmpdesc; tmpdesc = tmpdesc->next) {
    if (!tmpdesc->connected && !PLR_FLAGGED(tmpdesc->character, PLR_WRITING | PLR_BUILDING)) {
      if (tmpdesc->character->char_specials.vc == ch->char_specials.vc) {
        if (COLOR_LEV(tmpdesc->character) >= C_NRM)
          send_to_char(buf, tmpdesc->character);
        else
          send_to_char(buf1, tmpdesc->character);
      }
    }
  }

  victim->char_specials.vcm = TRUE;
  ch->char_specials.vcm = FALSE;
}
