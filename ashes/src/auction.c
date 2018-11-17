/* ************************************************************************
*   File: auction.c                                Part of Ashes to Ashes *
*  Usage: auto-auction system                                             *
*                                                                         *
*  All rights reserved.                                                   *
*                                                                         *
*  Copyright (C) 1998 by Jesse Sterr                                      *
*  Ashes to Ashes is based on CircleMUD, Copyright (C) 1993, 1994.        *
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

extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern struct player_special_data dummy_mob;	/* dummy spec area for mobs */
ACMD(do_gen_comm);

struct auc {
  long id;
  int vnum;
  long owner;
  char name[80];
  int min;
  struct auc *next;
};

int going=0, cost=0;
struct char_data *bidder=NULL;
struct auc *item_list=NULL;

int auc_ok(struct auc *item)
{
  struct char_data *tch;
  struct obj_data *tobj;

  if(!item_list)
    return FALSE;
  for(tch=character_list; tch; tch=tch->next) {
    if((!IS_NPC(tch)) && (GET_IDNUM(tch)==item->owner))
      break;
  }
  if(!tch) {
    return FALSE;
  }
  else {
    for(tobj=object_list; tobj; tobj=tobj->next) {
      if((tobj->id==item->id) && (GET_OBJ_VNUM(tobj)==item->vnum))
        break;
    }
    if(!tobj) {
      return FALSE;
    }
    else {
      while(tobj->in_obj) {
        tobj=tobj->in_obj;
      }
      if((tobj->carried_by && (GET_IDNUM(tobj->carried_by)!=item->owner)) ||
         (tobj->worn_by && (GET_IDNUM(tobj->worn_by)!=item->owner)) ||
         ((!tobj->carried_by) && (!tobj->worn_by))) {
        return FALSE;
      }
    }
  }
  return TRUE;
}

void show_auction(struct char_data *ch)
{
  struct auc *temp;
  struct char_data *tch;
  struct obj_data *tobj;

  strcpy(buf, "Item                           Character            Min Bid\r\n");
  for(temp=item_list; temp; temp=temp->next) {
    if(auc_ok(temp)) { 
      for(tch=character_list; tch; tch=tch->next) {
        if((!IS_NPC(tch)) && (GET_IDNUM(tch)==temp->owner))
          break;
      }
      for(tobj=object_list; tobj; tobj=tobj->next) {
        if((tobj->id==temp->id) && (GET_OBJ_VNUM(tobj)==temp->vnum))
          break;
      }
      sprintf(buf1, "%-30s %-20s %7d\r\n", tobj->short_description, GET_NAME(tch), temp->min);
      if(strlen(buf) > MAX_STRING_LENGTH-150) {
        strcat(buf, "***OVERFLOW***\r\n");
        break;
      }
      else
        strcat(buf, buf1);
    }
  }
  page_string(ch->desc, buf, 1);
}

void update_auction(void)
{
  struct auc *temp;
  struct char_data *syschar, *tch;
  struct obj_data *tobj;
  char buf[MAX_STRING_LENGTH];

  if(item_list) {
    syschar=create_char();
    GET_LEVEL(syschar)=LVL_IMPL;
    GET_NAME(syschar)=str_dup("The Auctioneer");
    syschar->in_room=0;
    syschar->player_specials=&dummy_mob;
    for(tch=character_list; tch; tch=tch->next) {
      if((!IS_NPC(tch)) && (GET_IDNUM(tch)==item_list->owner))
        break;
    }
    if(!tch) {
      sprintf(buf, "The %s has been removed from bidding.", item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      sprintf(buf, "Its owner is no longer playing.");
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      going=-100;
    }
    else {
      for(tobj=object_list; tobj; tobj=tobj->next) {
        if((tobj->id==item_list->id) && (GET_OBJ_VNUM(tobj)==item_list->vnum))
          break;
      }
      if(!tobj) {
        sprintf(buf, "The %s has been removed from bidding.", item_list->name);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
        sprintf(buf, "It is no longer in the game.");
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
        going=-100;
      }
      else {
        while(tobj->in_obj) {
          tobj=tobj->in_obj;
        }
        if((tobj->carried_by && (GET_IDNUM(tobj->carried_by)!=item_list->owner)) ||
           (tobj->worn_by && (GET_IDNUM(tobj->worn_by)!=item_list->owner)) ||
           ((!tobj->carried_by) && (!tobj->worn_by))) {
          sprintf(buf, "The %s has been removed from bidding.", item_list->name);
          do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
          sprintf(buf, "It is no longer on the original owner.");
          do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
          going=-100;
        }
        else {
          if(bidder) {
            for(tch=character_list; tch; tch=tch->next) {
              if(tch==bidder)
                break;
            }
            if(!tch) {
              strcpy(buf, "The last bidder is no longer playing, starting the bidding over.");
              do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
              cost=0;
              going=0;
              bidder=NULL;
            }
            else {
              if((GET_GOLD(tch) + GET_BANK_GOLD(tch)) < cost) {
                sprintf(buf, "%s no longer has enough gold, starting the bidding over.", GET_NAME(tch));
                do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
                cost=0;
                going=0;
                bidder=NULL;
              }
            }
          }
        }
      }
    }
    switch(going) {
    case -100:
      cost=0;
      going=0;
      bidder=NULL;
      do {
        temp=item_list->next;
        free(item_list);
        item_list=temp;
      } while(item_list && (!auc_ok(item_list)));
      if(item_list) {
        sprintf(buf, "Next up for bidding: %s.", item_list->name);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      }
      break;
    case 0:
      switch(number(1, 5)) {
      case 1:
        strcpy(buf1, "wonderful");
        break;
      case 2:
        strcpy(buf1, "lovely");
        break;
      case 3:
        strcpy(buf1, "handy");
        break;
      case 4:
        strcpy(buf1, "well made");
        break;
      case 5:
        strcpy(buf1, "very interesting");
        break;
      }
      if(bidder) {
        sprintf(buf, "How about %d, can I get %d coins for this %s %s.",
                MAX((int)(cost*1.1), cost+1), MAX((int)(cost*1.1), cost+1),
                buf1, item_list->name);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
        going++;
      }
      else {
        sprintf(buf, "Up for bidding is this %s %s.", buf1, item_list->name);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
        sprintf(buf, "Minimum bid, %d coins.", item_list->min);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
        going--;
      }
      break;
    case -3:
      sprintf(buf, "The %s has been removed from bidding.", item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      sprintf(buf, "No one was interested enough to pay the small price of %d coins.", item_list->min);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      cost=0;
      going=0;
      bidder=NULL;
      do {
        temp=item_list->next;
        free(item_list);
        item_list=temp;
      } while(item_list && (!auc_ok(item_list)));
      if(item_list) {
        sprintf(buf, "Next up for bidding: %s.", item_list->name);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      }
      break;
    case -2:
      switch(number(1, 5)) {
      case 1:
        strcpy(buf1, "amazing");
        break;
      case 2:
        strcpy(buf1, "exquisite");
        break;
      case 3:
        strcpy(buf1, "invaluable");
        break;
      case 4:
        strcpy(buf1, "hand crafted");
        break;
      case 5:
        strcpy(buf1, "mesmerizing");
        break;
      }
      sprintf(buf, "Last chance to get this %s %s.", buf1, item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      sprintf(buf, "It's a steal at a mere %d coins.", item_list->min);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      going--;
      break;
    case -1:
      switch(number(1, 5)) {
      case 1:
        strcpy(buf1, "amazing");
        break;
      case 2:
        strcpy(buf1, "exquisite");
        break;
      case 3:
        strcpy(buf1, "invaluable");
        break;
      case 4:
        strcpy(buf1, "hand crafted");
        break;
      case 5:
        strcpy(buf1, "mesmerizing");
        break;
      }
      sprintf(buf, "Surely someone must want this %s %s.", buf1, item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      sprintf(buf, "Only %d coins.", item_list->min);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      going--;
      break;
    case 1:
      switch(number(1, 5)) {
      case 1:
        strcpy(buf1, "superb");
        break;
      case 2:
        strcpy(buf1, "great looking");
        break;
      case 3:
        strcpy(buf1, "very useful");
        break;
      case 4:
        strcpy(buf1, "high quality");
        break;
      case 5:
        strcpy(buf1, "extraordinary");
        break;
      }
      sprintf(buf, "Just %d coins and this %s %s could be yours.",
              MAX((int)(cost*1.1), cost+1), buf1, item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      going++;
      break;
    case 2:
      sprintf(buf, "Alright, %d. Just %d coins and this %s could be yours.",
              MAX((int)(cost*1.05), cost+1), MAX((int)(cost*1.05), cost+1),
              item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      going++;
      break;
    case 3:
      sprintf(buf, "%s going once to %s for %d coins.", item_list->name, GET_NAME(bidder), cost);
      do_gen_comm(syschar, CAP(buf), 0, SCMD_AUCTION);
      going++;
      break;
    case 4:
      sprintf(buf, "%s going twice to %s for %d coins.", item_list->name, GET_NAME(bidder), cost);
      do_gen_comm(syschar, CAP(buf), 0, SCMD_AUCTION);
      going++;
      break;
    case 5:
      sprintf(buf, "%s sold to %s for %d coins.", item_list->name, GET_NAME(bidder), cost);
      do_gen_comm(syschar, CAP(buf), 0, SCMD_AUCTION);
      for(tobj=object_list; tobj; tobj=tobj->next) {
        if((tobj->id==item_list->id) && (GET_OBJ_VNUM(tobj)==item_list->vnum))
          break;
      }
      if(tobj->in_obj) {
        obj_from_obj(tobj);
      }
      if(tobj->carried_by) {
        obj_from_char(tobj);
      }
      if(tobj->worn_by) {
        unequip_char(tobj->worn_by, tobj->worn_on);
      }
      for(tch=character_list; tch; tch=tch->next) {
        if((!IS_NPC(tch)) && (GET_IDNUM(tch)==item_list->owner))
          break;
      }
      obj_to_char(tobj, bidder);
      GET_GOLD(tch)+=cost;
      sprintf(buf, "The Auctioneer takes the %s from you and gives you %d coins.\r\n", item_list->name, cost);
      send_to_char(buf, tch);
      sprintf(buf, "The Auctioneer takes %d coins from you and gives you the %s.\r\n", cost, item_list->name);
      send_to_char(buf, bidder);
      if(GET_LEVEL(tch) >= LVL_HERO) {
        sprintf(buf, "(GC) %s auctioned %s(%d) to %s.", GET_NAME(tch), tobj->short_description, GET_OBJ_VNUM(tobj), GET_NAME(bidder));
        mudlog(buf, NRM, GET_LEVEL(tch), TRUE);
      }
      if(GET_GOLD(bidder) < cost) {
        cost-=GET_GOLD(bidder);
        GET_BANK_GOLD(bidder)-=cost;
        GET_GOLD(bidder)=0;
      }
      else
        GET_GOLD(bidder)-=cost;
      cost=0;
      going=0;
      bidder=NULL;
      do {
        temp=item_list->next;
        free(item_list);
        item_list=temp;
      } while(item_list && (!auc_ok(item_list)));
      if(item_list) {
        sprintf(buf, "Next up for bidding: %s.", item_list->name);
        do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      }
      break;
    }
    character_list=syschar->next;
    syschar->player_specials=NULL;
    free_char(syschar);
  }
}


ACMD(do_autoauction)
{
  struct obj_data *obj;
  struct auc *temp, *trace;
  char *ptr;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char("You cannot auction!!\r\n", ch);
    return;
  }
  if (PRF_FLAGGED(ch, PRF_NOAUCT) && (!IS_NPC(ch))) {
    send_to_char("You aren't even on the auction channel!\r\n", ch);
    return;
  }

  two_arguments(argument, arg, buf);
  if(!*arg) {
    send_to_char("You must specify and item to auction off.\r\n", ch);
    return;
  }
  if(!(obj=get_obj_in_list_vis(ch, arg, ch->carrying))) {
    send_to_char("You aren't carrying anything like that.\r\n", ch);
    return;
  }
  if(!*buf) {
    send_to_char("You must specify a minimum bid.\r\n", ch);
    return;
  }
  if(!is_number(buf)) {
    send_to_char("Minimum bid must be a NUMBER!\r\n", ch);
    return;
  }

  CREATE(temp, struct auc, 1);
  temp->id=obj->id;
  temp->vnum=GET_OBJ_VNUM(obj);
  temp->owner=GET_IDNUM(ch);
  strcpy(buf2, obj->short_description);
  buf2[79]=0;
  ptr=any_one_arg(buf2, buf1);
  skip_spaces(&ptr);
  if((!str_cmp(buf1, "an")) || (!str_cmp(buf1, "a")) || (!str_cmp(buf1, "the")))
    strcpy(temp->name, ptr);
  else
    strcpy(temp->name, buf2);
  temp->min=MAX(atoi(buf), 1);
  temp->next=NULL;
  if(!item_list)
    item_list=temp;
  else {
    for(trace=item_list; trace->next; trace=trace->next);
    trace->next=temp;
  }
  send_to_char(OK, ch);
}

ACMD(do_bid)
{
  int i;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *syschar;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char("You cannot auction!!\r\n", ch);
    return;
  }
  if (PRF_FLAGGED(ch, PRF_NOAUCT) && (!IS_NPC(ch))) {
    send_to_char("You aren't even on the auction channel!\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if(!item_list) {
    send_to_char("There is nothing to bid on.\r\n", ch);
    return;
  }
  if((i=atoi(arg)) < 1)
    send_to_char("You can only bid positive values.\r\n", ch);
  else if(i > (GET_GOLD(ch)+GET_BANK_GOLD(ch)))
    send_to_char("You don't have that much money.\r\n", ch);
  else if(i <= cost) {
    sprintf(buf, "The bidding is already up to %d.\r\n", cost);
    send_to_char(buf, ch);
  }
  else if(cost) {
    if(i < MAX((int)(cost*1.05), cost+1)) {
      sprintf(buf, "Sorry, the minimum raise is 5%%. The lowest you can bid is %d.\r\n", MAX((int)(cost*1.05), cost+1));
      send_to_char(buf, ch);
    }
    else {
      cost=i;
      bidder=ch;
      going=0;
      syschar=create_char();
      GET_LEVEL(syschar)=LVL_IMPL;
      GET_NAME(syschar)=str_dup("The Auctioneer");
      syschar->in_room=0;
      syschar->player_specials=&dummy_mob;
      sprintf(buf, "%s bids %d coins on the %s.", GET_NAME(ch), cost, item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      character_list=syschar->next;
      syschar->player_specials=NULL;
      free_char(syschar);
    }
  }
  else {
    if(i < item_list->min) {
      sprintf(buf, "The minimum bid is %d.\r\n", item_list->min);
      send_to_char(buf, ch);
    }
    else {
      cost=i;
      bidder=ch;
      going=0;
      syschar=create_char();
      GET_LEVEL(syschar)=LVL_IMPL;
      GET_NAME(syschar)=str_dup("The Auctioneer");
      syschar->in_room=0;
      syschar->player_specials=&dummy_mob;
      sprintf(buf, "%s bids %d coins on the %s.", GET_NAME(ch), cost, item_list->name);
      do_gen_comm(syschar, buf, 0, SCMD_AUCTION);
      character_list=syschar->next;
      syschar->player_specials=NULL;
      free_char(syschar);
    }
  }
}

ACMD(do_identify)
{
  struct obj_data *tobj;
  ASPELL(spell_identify);

  if(!item_list) {
    send_to_char("There is no item up for auction right now.\r\n", ch);
    return;
  }
  for(tobj=object_list; tobj; tobj=tobj->next) {
    if((tobj->id==item_list->id) && (GET_OBJ_VNUM(tobj)==item_list->vnum))
      break;
  }
  if(!tobj)
    send_to_char("The item up for auction is no longer in the game.\r\n", ch);
  else
    spell_identify(100, ch, NULL, tobj);
}
