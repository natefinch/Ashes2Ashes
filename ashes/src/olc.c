/* ************************************************************************
*   File: olc.c                                    Part of Ashes to Ashes *
*  Usage: online creation                                                 *
*                                                                         *
*  All rights reserved.                                                   *
*                                                                         *
*  Copyright (C) 1997 by Jesse Sterr                                      *
*  Ashes to Ashes is based on CircleMUD, Copyright (C) 1993, 1994.        *
************************************************************************ */

#define __OLC_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "olc.h"
#include "spells.h"
#include "shop.h"
#include "boards.h"
#include "class.h"


/* external data */
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;
extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct char_data *mob_proto;
extern struct index_data *mob_index;
extern struct shop_data *shop_index;
extern int zlock_reset;
extern int top_of_world;
extern int top_of_objt;
extern int top_of_mobt;
extern int top_of_zone_table;
extern int top_shop;
extern struct str_app_type str_app[];
extern char *item_types[];
extern char *apply_types[];
extern char *room_bits[];
extern char *npc_class_types[];
extern char *genders[];
extern char *npc_size[];
extern char *position_types[];
extern char *weapon_types[];
extern char *spec_proc_names[];
extern char *action_bits[];
extern char *affected_bits[];
extern char *damtypes[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *item_types[];
extern char *container_bits[];
extern char *drinks[];
extern char *spells[];
extern char *sector_types[];
extern int movement_loss[];
extern char *room_bits[];
extern char *dirs[];
extern char *equipment_types[];
extern int(*spec_proc_table[MAX_SPECPROCS])(struct char_data *, void *, int, char *);
extern struct board_info_type board_info[NUM_OF_BOARDS];
extern FILE *player_fl;
extern char *pc_class_types[];
extern char *class_abbrevs[];
extern char *preference_bits[];
extern char *player_bits[];
extern char *grant_bits[];

SPECIAL(shop_keeper);
SPECIAL(damage_eq);
ACMD(do_olcmenu);
ACMD(do_zpurge);
ACMD(do_zreset);
void save_edit(struct char_data *ch);


ACMD(do_mlist)
{
  int zone, vzone;
  int nr, i;
  char *listbuf;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if((!*arg) || ((!is_number(arg)) && (*arg != '.'))) {
    send_to_char("You must specify a zone number.\r\n", ch);
    return;
  }

  if(*arg == '.')
    vzone=zone_table[world[ch->in_room].zone].number;
  else
    vzone=atoi(arg);

  if((vzone < 0) || ((zone=real_zone(vzone)) < 0)) {
    send_to_char("That zone doesn't exist.\r\n", ch);
    return;
  }

  CREATE(listbuf, char, 1);
  listbuf[0]=0;

  for(nr=zone_table[zone].bottom; nr <= zone_table[zone].top; nr++) {
    if((i=real_mobile(nr)) >= 0) {
      strcpy(buf1, GET_NAME(mob_proto+i));
      buf1[20]=0;
      sprintf(buf2, "HP:%dd%d+%d", mob_proto[i].points.hit, mob_proto[i].points.mana, mob_proto[i].points.move);
      sprintf(buf, "[%5d] %-20s LVL:%-3d %-15s HR:%-2d XP:%3ldk DAM:%dd%d+%d\r\n",
              nr, buf1, GET_LEVEL(mob_proto+i), buf2, mob_proto[i].points.hitroll,
              GET_EXP(mob_proto+i)/1000, mob_proto[i].mob_specials.damnodice,
              mob_proto[i].mob_specials.damsizedice, mob_proto[i].points.damroll);
      RECREATE(listbuf, char, strlen(listbuf)+strlen(buf)+1);
      strcat(listbuf, buf);
    }
  }

  page_string(ch->desc, listbuf, 1);
  free(listbuf);
}

ACMD(do_olist)
{
  int zone, vzone;
  int nr, i, j;
  char *listbuf;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if((!*arg) || ((!is_number(arg)) && (*arg != '.'))) {
    send_to_char("You must specify a zone number.\r\n", ch);
    return;
  }

  if(*arg == '.')
    vzone=zone_table[world[ch->in_room].zone].number;
  else
    vzone=atoi(arg);

  if((vzone < 0) || ((zone=real_zone(vzone)) < 0)) {
    send_to_char("That zone doesn't exist.\r\n", ch);
    return;
  }

  CREATE(listbuf, char, 1);
  listbuf[0]=0;

  for(nr=zone_table[zone].bottom; nr <= zone_table[zone].top; nr++) {
    if((i=real_object(nr)) >= 0) {
      sprinttype(GET_OBJ_TYPE(obj_proto+i), item_types, buf2);
      buf1[0]=0;
      switch(GET_OBJ_TYPE(obj_proto+i)) {
      case ITEM_WEAPON:
        sprintf(buf1, " %dd%d", obj_proto[i].obj_flags.value[1], obj_proto[i].obj_flags.value[2]);
        break;
      case ITEM_ARMOR:
        sprintf(buf1, " AC:%d", obj_proto[i].obj_flags.value[0]);
        break;
      }
      strcat(buf2, buf1);
      strcpy(buf1, obj_proto[i].short_description);
      buf1[20]=0;
      sprintf(buf, "[%5d] %-20s %-14s", nr, buf1, buf2);
      for(j=0; j < MAX_OBJ_AFFECT; j++) {
        if(obj_proto[i].affected[j].location) {
          sprinttype(obj_proto[i].affected[j].location, apply_types, buf2);
          sprintf(buf1, " %+d to %s", obj_proto[i].affected[j].modifier, buf2);
          strcat(buf, buf1);
        }
      }
      strcat(buf, "\r\n");
      RECREATE(listbuf, char, strlen(listbuf)+strlen(buf)+1);
      strcat(listbuf, buf);
    }
  }

  page_string(ch->desc, listbuf, 1);
  free(listbuf);
}

ACMD(do_rlist)
{
  int zone, vzone;
  int nr, i;
  char *listbuf;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if((!*arg) || ((!is_number(arg)) && (*arg != '.'))) {
    send_to_char("You must specify a zone number.\r\n", ch);
    return;
  }

  if(*arg == '.')
    vzone=zone_table[world[ch->in_room].zone].number;
  else
    vzone=atoi(arg);

  if((vzone < 0) || ((zone=real_zone(vzone)) < 0)) {
    send_to_char("That zone doesn't exist.\r\n", ch);
    return;
  }

  CREATE(listbuf, char, 1);
  listbuf[0]=0;

  for(nr=zone_table[zone].bottom; nr <= zone_table[zone].top; nr++) {
    if((i=real_room(nr)) >= 0) {
      strcpy(buf1, world[i].name);
      buf1[22]=0;
      sprintbit(world[i].room_flags, room_bits, buf2);
      sprintf(buf, "[%5d] %-22.22s  %s\r\n", nr, buf1, buf2);
      RECREATE(listbuf, char, strlen(listbuf)+strlen(buf)+1);
      strcat(listbuf, buf);
    }
  }

  page_string(ch->desc, listbuf, 1);
  free(listbuf);
}

ACMD(do_zlist)
{
  int zone, vzone;
  int nr, i, j, rnr, ri;
  struct char_data *mob;
  struct obj_data *obj;
  char *listbuf;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if((!*arg) || ((!is_number(arg)) && (*arg != '.'))) {
    send_to_char("You must specify a zone number.\r\n", ch);
    return;
  }

  if(*arg == '.')
    vzone=zone_table[world[ch->in_room].zone].number;
  else
    vzone=atoi(arg);

  if((vzone < 0) || ((zone=real_zone(vzone)) < 0)) {
    send_to_char("That zone doesn't exist.\r\n", ch);
    return;
  }

  if((!HAS_OLC(ch, vzone)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  CREATE(listbuf, char, 1);
  listbuf[0]=0;

  for(rnr=zone_table[zone].bottom; rnr <= zone_table[zone].top; rnr++) {
    if((ri=real_room(rnr)) >= 0) {
      strcpy(buf1, world[ri].name);
      buf1[22]=0;
      sprintbit(world[ri].room_flags, room_bits, buf2);
      sprintf(buf, "[%5d] %-22.22s  %s\r\n", rnr, buf1, buf2);
      RECREATE(listbuf, char, strlen(listbuf)+strlen(buf)+1);
      strcat(listbuf, buf);
      for(mob=world[ri].people; mob; mob=mob->next_in_room) {
        if(!IS_NPC(mob))
          continue;
        i=GET_MOB_RNUM(mob);
        nr=GET_MOB_VNUM(mob);
        strcpy(buf1, GET_NAME(mob_proto+i));
        buf1[20]=0;
        sprintf(buf2, "HP:%dd%d+%d", mob_proto[i].points.hit, mob_proto[i].points.mana, mob_proto[i].points.move);
        sprintf(buf, "  [%5d] %-20s LVL:%-3d %-15s HR:%-2d AC:%+4d DAM:%dd%d+%d\r\n",
                nr, buf1, GET_LEVEL(mob_proto+i), buf2, mob_proto[i].points.hitroll,
                compute_ac(mob_proto+i), mob_proto[i].mob_specials.damnodice,
                mob_proto[i].mob_specials.damsizedice, mob_proto[i].points.damroll);
        RECREATE(listbuf, char, strlen(listbuf)+strlen(buf)+1);
        strcat(listbuf, buf);
      }
      for(obj=world[ri].contents; obj; obj=obj->next_content) {
        i=GET_OBJ_RNUM(obj);
        nr=GET_OBJ_VNUM(obj);
        sprinttype(GET_OBJ_TYPE(obj_proto+i), item_types, buf2);
        buf1[0]=0;
        switch(GET_OBJ_TYPE(obj_proto+i)) {
        case ITEM_WEAPON:
          sprintf(buf1, " %dd%d", obj_proto[i].obj_flags.value[1], obj_proto[i].obj_flags.value[2]);
          break;
        case ITEM_ARMOR:
          sprintf(buf1, " AC:%d", obj_proto[i].obj_flags.value[0]);
          break;
        }
        strcat(buf2, buf1);
        strcpy(buf1, obj_proto[i].short_description);
        buf1[20]=0;
        sprintf(buf, "  [%5d] %-20s %-14s", nr, buf1, buf2);
        for(j=0; j < MAX_OBJ_AFFECT; j++) {
          if(obj_proto[i].affected[j].location) {
            sprinttype(obj_proto[i].affected[j].location, apply_types, buf2);
            sprintf(buf1, " %+d to %s", obj_proto[i].affected[j].modifier, buf2);
            strcat(buf, buf1);
          }
        }
        strcat(buf, "\r\n");
        RECREATE(listbuf, char, strlen(listbuf)+strlen(buf)+1);
        strcat(listbuf, buf);
      }
    }
  }

  page_string(ch->desc, listbuf, 1);
  free(listbuf);
}


void copy_obj(struct obj_data *copy, struct obj_data *obj)
{
  struct extra_descr_data *ex, *new_ex;

  *copy=*obj;
  copy->name=str_dup(obj->name);
  copy->description=str_dup(obj->description);
  copy->short_description=str_dup(obj->short_description);
  copy->action_description=str_dup(obj->action_description);
  copy->ex_description=NULL;
  for(ex=obj->ex_description; ex; ex=ex->next) {
    CREATE(new_ex, struct extra_descr_data, 1);
    new_ex->keyword=str_dup(ex->keyword);
    new_ex->description=str_dup(ex->description);
    new_ex->next=copy->ex_description;
    copy->ex_description=new_ex;
  }
  return;
}

void copy_mob(struct char_data *copy, struct char_data *mob)
{
  *copy=*mob;
  copy->char_specials.prompt=str_dup(mob->char_specials.prompt);
  copy->player.name=str_dup(mob->player.name);
  copy->player.short_descr=str_dup(mob->player.short_descr);
  copy->player.long_descr=str_dup(mob->player.long_descr);
  copy->player.description=str_dup(mob->player.description);
  if(ACTIONS(mob))
    ACTIONS(copy)=str_dup(ACTIONS(mob));
  else
    ACTIONS(copy)=NULL;
  return;
}

void copy_plr(struct char_data *copy, struct char_data *plr)
{
  struct char_data temp;
  struct player_special_data *tps;
  struct affected_type *af;
  int i, j;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(plr, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(plr, GET_EQ(plr, i)->affected[j].location,
		      GET_EQ(plr, i)->affected[j].modifier,
		      GET_EQ(plr, i)->obj_flags.bitvector, FALSE);
  }

  for (af = plr->affected; af; af = af->next)
    affect_modify(plr, af->location, af->modifier, af->bitvector, FALSE);

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(copy, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(copy, GET_EQ(copy, i)->affected[j].location,
		      GET_EQ(copy, i)->affected[j].modifier,
		      GET_EQ(copy, i)->obj_flags.bitvector, FALSE);
  }

  for (af = copy->affected; af; af = af->next)
    affect_modify(copy, af->location, af->modifier, af->bitvector, FALSE);

  temp=*plr;

  /* certain things should not be coppied over, take care of these here */
  temp.char_specials.fighting=copy->char_specials.fighting;
  temp.char_specials.spcont=copy->char_specials.spcont;
  CREATE(temp.player_specials, struct player_special_data, 1);
  *temp.player_specials=*plr->player_specials;
  temp.player_specials->aliases=copy->player_specials->aliases;
  temp.player_specials->arena=copy->player_specials->arena;
  temp.player_specials->zone_locked=copy->player_specials->zone_locked;
  temp.player_specials->olc_mode=copy->player_specials->olc_mode;
  temp.player_specials->olc_vnum=copy->player_specials->olc_vnum;
  temp.player_specials->olc_field=copy->player_specials->olc_field;
  temp.player_specials->olc_ptr=copy->player_specials->olc_ptr;
  temp.char_specials.carry_weight=copy->char_specials.carry_weight;
  temp.char_specials.carry_items=copy->char_specials.carry_items;
  temp.char_specials.vc=copy->char_specials.vc;
  temp.char_specials.vci=copy->char_specials.vci;
  temp.char_specials.vcm=copy->char_specials.vcm;
  temp.char_specials.vcs=copy->char_specials.vcs;
  temp.affected=copy->affected;
  for(i=0; i<NUM_WEARS; i++)
    GET_EQ(&temp, i)=GET_EQ(copy, i);
  temp.carrying=copy->carrying;
  temp.desc=copy->desc;
  temp.in_room=copy->in_room;
  temp.was_in_room=copy->was_in_room;
  temp.next_in_room=copy->next_in_room;
  temp.next=copy->next;
  temp.next_fighting=copy->next_fighting;
  temp.followers=copy->followers;
  temp.master=copy->master;
  tps=copy->player_specials;
  *copy=temp;
  copy->player_specials=tps;
  *copy->player_specials=*temp.player_specials;
  free(temp.player_specials);
  copy->player.name=str_dup(plr->player.name);
  if(plr->player.description)
    copy->player.description=str_dup(plr->player.description);
  else
    copy->player.description=NULL;
  copy->player.title=str_dup(plr->player.title);
  copy->char_specials.prompt=str_dup(plr->char_specials.prompt);
  copy->player_specials->walkin=str_dup(plr->player_specials->walkin);
  copy->player_specials->walkout=str_dup(plr->player_specials->walkout);
  copy->player_specials->poofin=str_dup(plr->player_specials->poofin);
  copy->player_specials->poofout=str_dup(plr->player_specials->poofout);
  copy->points.max_hit = MAX(copy->player_specials->saved.new_hit, copy->player_specials->saved.old_hit);
  copy->points.max_mana = MAX(copy->player_specials->saved.new_mana, copy->player_specials->saved.old_mana);
  copy->points.max_move = MAX(copy->player_specials->saved.new_move, copy->player_specials->saved.old_move);

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(plr, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(plr, GET_EQ(plr, i)->affected[j].location,
		      GET_EQ(plr, i)->affected[j].modifier,
		      GET_EQ(plr, i)->obj_flags.bitvector, TRUE);
  }
  for (af = plr->affected; af; af = af->next)
    affect_modify(plr, af->location, af->modifier, af->bitvector, TRUE);

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(copy, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(copy, GET_EQ(copy, i)->affected[j].location,
		      GET_EQ(copy, i)->affected[j].modifier,
		      GET_EQ(copy, i)->obj_flags.bitvector, TRUE);
  }
  for (af = copy->affected; af; af = af->next)
    affect_modify(copy, af->location, af->modifier, af->bitvector, TRUE);

  return;
}

void copy_room(struct room_data *copy, struct room_data *room)
{
  int i;
  struct extra_descr_data *ex, *new_ex;
  struct room_data temp;

  temp=*room;

  /* stuff that should not change on the copy */
  temp.light=copy->light;
  temp.contents=copy->contents;
  temp.people=copy->people;

  *copy=temp;
  if(room->name)
    copy->name=str_dup(room->name);
  else
    copy->name=str_dup("");
  if(room->description)
    copy->description=str_dup(room->description);
  else
    copy->description=str_dup("");
  copy->ex_description=NULL;
  for(ex=room->ex_description; ex; ex=ex->next) {
    CREATE(new_ex, struct extra_descr_data, 1);
    new_ex->keyword=str_dup(ex->keyword);
    new_ex->description=str_dup(ex->description);
    new_ex->next=copy->ex_description;
    copy->ex_description=new_ex;
  }
  for(i=0; i<NUM_OF_DIRS; i++) {
    if(room->dir_option[i]) {
      CREATE(copy->dir_option[i], struct room_direction_data, 1);
      *copy->dir_option[i]=*room->dir_option[i];
      copy->dir_option[i]->general_description=str_dup(room->dir_option[i]->general_description);
      copy->dir_option[i]->keyword=str_dup(room->dir_option[i]->keyword);
    }
  }
}

void free_room(struct room_data *room)
{
  int i;
  struct extra_descr_data *ex, *next;

  free(room->name);
  free(room->description);
  for(ex=room->ex_description; ex; ex=next) {
    next=ex->next;
    free(ex->keyword);
    free(ex->description);
    free(ex);
  }
  for(i=0; i<NUM_OF_DIRS; i++) {
    if(room->dir_option[i]) {
      free(room->dir_option[i]->general_description);
      free(room->dir_option[i]->keyword);
      free(room->dir_option[i]);
    }
  }
  free(room);
}

void free_partial_room(struct room_data *room)
{
  int i;
  struct extra_descr_data *ex, *next;

  free(room->name);
  free(room->description);
  for(ex=room->ex_description; ex; ex=next) {
    next=ex->next;
    free(ex->keyword);
    free(ex->description);
    free(ex);
  }
  for(i=0; i<NUM_OF_DIRS; i++) {
    if(room->dir_option[i]) {
      free(room->dir_option[i]->general_description);
      free(room->dir_option[i]->keyword);
      free(room->dir_option[i]);
    }
  }
}

void free_zcmd(struct zcmd_data *z)
{
  if(z) {
    free_zcmd(z->if_next);
    free_zcmd(z->else_next);
    free_zcmd(z->next);
    free(z);
  }
}

ACMD(do_medit)
{
  int zone, mob;
  struct char_data *tch;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if(GET_OLC_MODE(ch)) {
    send_to_char("You are already editting!\r\n", ch);
    return;
  }

  if((!*arg) || (!is_number(arg))) {
    send_to_char("You must specify a mob number.\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_AVTR)) {
    send_to_char("You can't edit in avatar mode.\r\n", ch);
    return;
  }

  mob=atoi(arg);

  for(zone=0; zone <= top_of_zone_table; zone++) {
    if((mob >= zone_table[zone].bottom) && (mob <= zone_table[zone].top))
      break;
  }
  if(zone > top_of_zone_table) {
    send_to_char("That mob number is not in any zone.\r\n", ch);
    return;
  }
  if((!HAS_OLC(ch, zone_table[zone].number)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  CREATE(tch, struct char_data, 1);
  clear_char(tch);

  tch->points.max_move=100;
  GET_STR(tch)=tch->real_abils.str=13;
  GET_ADD(tch)=tch->real_abils.str_add=0;
  GET_INT(tch)=tch->real_abils.intel=13;
  GET_WIS(tch)=tch->real_abils.wis=13;
  GET_DEX(tch)=tch->real_abils.dex=13;
  GET_CON(tch)=tch->real_abils.con=13;
  GET_CHA(tch)=tch->real_abils.cha=13;
  GET_SAVE(tch, 0)=100;
  GET_SAVE(tch, 1)=100;
  GET_SAVE(tch, 2)=100;
  GET_SAVE(tch, 3)=100;
  GET_SAVE(tch, 4)=100;
  MOB_FLAGS(tch)=MOB_ISNPC;
  GET_MOVE_RATE(tch)=GET_FEROCITY(tch)=1;
  GET_POS(tch)=GET_DEFAULT_POS(tch)=POS_STANDING;
  GET_MOVE(tch)=1;
  tch->player_specials=mob_proto[0].player_specials;
  tch->mob_specials.attacks=2;
  tch->mob_specials.move_rate=6;
  GET_HEIGHT(tch)=198;
  GET_WEIGHT(tch)=60;

  GET_OLC_PTR(ch)=tch;
  if(real_mobile(mob) >= 0) {
    copy_mob(tch, mob_proto+real_mobile(mob));
  }
  else {
    tch->char_specials.prompt=str_dup("*");
    tch->player.name=str_dup("");
    tch->player.short_descr=str_dup("");
    tch->player.long_descr=str_dup("");
    tch->player.description=str_dup("");
  }

  GET_OLC_MODE(ch)=OLC_MEDIT;
  GET_OLC_FIELD(ch)=0;
  GET_OLC_NUM(ch)=mob;
  if(PRF_FLAGGED(ch, PRF_NODISTURB))
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);

  do_olcmenu(ch, "", 0, 0);
}

ACMD(do_iedit)
{
  int worn_on, in_room;
  struct obj_data *obj, *temp, *in_obj;
  struct char_data *carried_by, *worn_by;

  if(GET_OLC_MODE(ch)) {
    send_to_char("You are already editting!\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_AVTR)) {
    send_to_char("You can't edit in avatar mode.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, arg);

  if (!(obj = get_object_in_equip_vis(ch, arg, ch->equipment, &worn_on))) {
    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
        send_to_char("Nothing here by that name.\r\n", ch);
        return;
      }
    }
  }

  if((GET_OBJ_RNUM(obj) < 0) || (GET_OBJ_VNUM(obj) < 0)) {
    send_to_char("That object cannot be editted.\r\n", ch);
    return;
  }

  worn_on=obj->worn_on;
  worn_by=obj->worn_by;
  in_room=obj->in_room;
  carried_by=obj->carried_by;
  in_obj=obj->in_obj;

  if(obj->worn_by)
    unequip_char(obj->worn_by, obj->worn_on);
  if(obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if(obj->carried_by)
    obj_from_char(obj);
  else if(obj->in_obj);
    obj_from_obj(obj);

  REMOVE_FROM_LIST(obj, object_list, next);

  obj->worn_on=worn_on;
  obj->worn_by=worn_by;
  obj->in_room=in_room;
  obj->carried_by=carried_by;
  obj->in_obj=in_obj;

  obj_index[GET_OBJ_RNUM(obj)].number--;

  GET_OLC_PTR(ch)=obj;

  GET_OLC_MODE(ch)=OLC_IEDIT;
  GET_OLC_FIELD(ch)=0;
  GET_OLC_NUM(ch)=GET_OBJ_VNUM(obj);
  if(PRF_FLAGGED(ch, PRF_NODISTURB))
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);

  do_olcmenu(ch, "", 0, 0);
}

ACMD(do_oedit)
{
  int zone, obj;
  struct obj_data *tobj;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if(GET_OLC_MODE(ch)) {
    send_to_char("You are already editting!\r\n", ch);
    return;
  }

  if((!*arg) || (!is_number(arg))) {
    send_to_char("You must specify an obj number.\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_AVTR)) {
    send_to_char("You can't edit in avatar mode.\r\n", ch);
    return;
  }

  obj=atoi(arg);

  for(zone=0; zone <= top_of_zone_table; zone++) {
    if((obj >= zone_table[zone].bottom) && (obj <= zone_table[zone].top))
      break;
  }
  if(zone > top_of_zone_table) {
    send_to_char("That obj number is not in any zone.\r\n", ch);
    return;
  }
  if((!HAS_OLC(ch, zone_table[zone].number)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  CREATE(tobj, struct obj_data, 1);
  clear_object(tobj);
  GET_OLC_PTR(ch)=tobj;
  if(real_object(obj) >= 0) {
    copy_obj(tobj, obj_proto+real_object(obj));
  }
  else {
    tobj->name=str_dup("");
    tobj->description=str_dup("");
    tobj->short_description=str_dup("");
    tobj->action_description=str_dup("");
    GET_OBJ_TYPE(tobj)=ITEM_OTHER;
    GET_OBJ_WEIGHT(tobj)=1;
  }

  GET_OLC_MODE(ch)=OLC_OEDIT;
  GET_OLC_FIELD(ch)=0;
  GET_OLC_NUM(ch)=obj;
  if(PRF_FLAGGED(ch, PRF_NODISTURB))
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);

  do_olcmenu(ch, "", 0, 0);
}

ACMD(do_pedit)
{
  struct char_data *vict, *tch;

  if(GET_OLC_MODE(ch)) {
    send_to_char("You are already editting!\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_AVTR)) {
    send_to_char("You can't edit in avatar mode.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  one_argument(argument, arg);

  if (!(vict = get_player_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }

  if((GET_LEVEL(vict) >= GET_LEVEL(ch)) && (ch != vict)) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }

  send_to_char("A beam of light scans over you, almost like a copy machine.\r\n", vict);

  CREATE(tch, struct char_data, 1);
  clear_char(tch);
  CREATE(tch->player_specials, struct player_special_data, 1);
  GET_OLC_PTR(ch)=tch;
  copy_plr(tch, vict);

  GET_OLC_MODE(ch)=OLC_PEDIT;
  GET_OLC_FIELD(ch)=0;
  GET_OLC_NUM(ch)=GET_IDNUM(tch);
  if(PRF_FLAGGED(ch, PRF_NODISTURB))
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);

  do_olcmenu(ch, "", 0, 0);
}

ACMD(do_redit)
{
  int zone, room;
  struct room_data *troom;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if(GET_OLC_MODE(ch)) {
    send_to_char("You are already editting!\r\n", ch);
    return;
  }

  if((!*arg) || (!is_number(arg))) {
    send_to_char("You must specify a room number.\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_AVTR)) {
    send_to_char("You can't edit in avatar mode.\r\n", ch);
    return;
  }

  room=atoi(arg);

  for(zone=0; zone <= top_of_zone_table; zone++) {
    if((room >= zone_table[zone].bottom) && (room <= zone_table[zone].top))
      break;
  }
  if(zone > top_of_zone_table) {
    send_to_char("That room number is not in any zone.\r\n", ch);
    return;
  }
  if((!HAS_OLC(ch, zone_table[zone].number)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  CREATE(troom, struct room_data, 1);
  memset((char *) troom, 0, sizeof(struct room_data));
  troom->number=room;
  troom->zone=zone;
  GET_OLC_PTR(ch)=troom;
  if(real_room(room) >= 0) {
    copy_room(troom, &world[real_room(room)]);
    REMOVE_BIT(troom->room_flags, ROOM_BFS_MARK);
    REMOVE_BIT(troom->room_flags, ROOM_SNARE);
    REMOVE_BIT(troom->room_flags, ROOM_DIM_DOOR);
/*    REMOVE_BIT(troom->room_flags, ROOM_STASIS); */
  }
  else {
    troom->name=str_dup("");
    troom->description=str_dup("");
  }

  GET_OLC_MODE(ch)=OLC_REDIT;
  GET_OLC_FIELD(ch)=0;
  GET_OLC_NUM(ch)=room;
  if(PRF_FLAGGED(ch, PRF_NODISTURB))
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);

  do_olcmenu(ch, "", 0, 0);
}

ACMD(do_zedit)
{
  int zone, vzone, count, c, prev_c;
  struct zcmd_data *tz, *temp, *prev, *scan, *start;
  struct reset_com *rcmd;

  skip_spaces(&argument);

  if(GET_OLC_MODE(ch)) {
    send_to_char("You are already editting!\r\n", ch);
    return;
  }

  if((!*argument) || (!is_number(argument))) {
    send_to_char("You must specify a zone number.\r\n", ch);
    return;
  }

  if(PRF_FLAGGED(ch, PRF_AVTR)) {
    send_to_char("You can't edit in avatar mode.\r\n", ch);
    return;
  }

  vzone=atoi(argument);

  zone=real_zone(vzone);
  if((zone > top_of_zone_table) || (zone < 0)) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }
  if((!HAS_OLC(ch, vzone)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }
  if((!zone_table[zone].closed) && (zone_table[zone].number != MIDGAARD_ZONE) &&
     (zone_table[zone].number != GOD_ZONE)) {
    send_to_char("You can't edit an open zone!.\r\n", ch);
    return;
  }

  CREATE(tz, struct zcmd_data, 1);
  tz->prev = tz->next = tz->if_next = tz->else_next = NULL;
  tz->arg1 = tz->arg2 = tz->arg3 = tz->arg4 = tz->prob = 0;
  tz->line = -1;
  tz->command = 0;
  GET_OLC_PTR(ch)=temp=prev=tz;

  for(count=0, rcmd=zone_table[zone].cmd; rcmd[count].command != 'S'; count++) {
    CREATE(temp, struct zcmd_data, 1);
    temp->command=rcmd[count].command;
    temp->arg1=rcmd[count].arg1;
    temp->arg2=rcmd[count].arg2;
    if((rcmd[count].command=='O')||(rcmd[count].command=='P')||
       (rcmd[count].command=='G')||(rcmd[count].command=='E')) {
      temp->arg2=rcmd[count].limit;
    }
    temp->arg3=rcmd[count].arg3;
    temp->arg4=rcmd[count].arg4;
    temp->line=count;
    temp->prob=rcmd[count].prob;
    temp->prev=prev;
    prev->next=temp;
    temp->next = temp->if_next = temp->else_next = NULL;
    prev=temp;
  }
  for( ; temp!=tz; temp=prev) {
    prev=temp->prev;
    if(rcmd[temp->line].if_flag) {
      count=temp->line;
      start=tz;
      while(1) {
        for(scan=start; scan; scan=scan->next)
          if(scan->line==rcmd[count].depend)
            break;
        if(scan) {
          if(count==temp->line) {
            temp->prev->next=temp->next;
            if(temp->next)
              temp->next->prev=temp->prev;
            if(rcmd[temp->line].if_flag > 0) {
              temp->next=scan->if_next;
              if(scan->if_next)
                scan->if_next->prev=temp;
              scan->if_next=temp;
              temp->prev=scan;
            }
            else {
              temp->next=scan->else_next;
              if(scan->else_next)
                scan->else_next->prev=temp;
              scan->else_next=temp;
              temp->prev=scan;
            }
            break;
          }
          else {
            for(prev_c=c=temp->line; c != count; prev_c=c, c=rcmd[c].depend);
            count=prev_c;
            if(rcmd[c].if_flag > 0)
              start=scan->if_next;
            else
              start=scan->else_next;
          }
        }
        else {
          count=rcmd[count].depend;
        }
      }
    }
  }

  GET_OLC_MODE(ch)=OLC_ZEDIT;
  GET_OLC_FIELD(ch)=0;
  GET_OLC_NUM(ch)=vzone;
  if(PRF_FLAGGED(ch, PRF_NODISTURB))
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);

  do_olcmenu(ch, "", 0, 0);
}


void do_stat_zone(struct char_data *ch, int zone)
{
  int count, c, prev_c;
  struct zcmd_data *tz, *temp, *prev, *scan, *start;
  struct reset_com *rcmd;
  char line[MAX_STRING_LENGTH], *zbuf;

  void print_zone_commands(struct zcmd_data *zcmd, int level, struct char_data *ch, char **zbuf);
  CREATE(tz, struct zcmd_data, 1);
  tz->prev = tz->next = tz->if_next = tz->else_next = NULL;
  tz->arg1 = tz->arg2 = tz->arg3 = tz->arg4 = tz->prob = 0;
  tz->line = -1;
  tz->command = 0;
  temp=prev=tz;
  for(count=0, rcmd=zone_table[zone].cmd; rcmd[count].command != 'S'; count++) {
    CREATE(temp, struct zcmd_data, 1);
    temp->command=rcmd[count].command;
    temp->arg1=rcmd[count].arg1;
    temp->arg2=rcmd[count].arg2;
    if((rcmd[count].command=='O')||(rcmd[count].command=='P')||
       (rcmd[count].command=='G')||(rcmd[count].command=='E')) {
      temp->arg2=rcmd[count].limit;
    }
    temp->arg3=rcmd[count].arg3;
    temp->arg4=rcmd[count].arg4;
    temp->line=count;
    temp->prob=rcmd[count].prob;
    temp->prev=prev;
    prev->next=temp;
    temp->next = temp->if_next = temp->else_next = NULL;
    prev=temp;
  }
  for( ; temp!=tz; temp=prev) {
    prev=temp->prev;
    if(rcmd[temp->line].if_flag) {
      count=temp->line;
      start=tz;
      while(1) {
        for(scan=start; scan; scan=scan->next)
          if(scan->line==rcmd[count].depend)
            break;
        if(scan) {
          if(count==temp->line) {
            temp->prev->next=temp->next;
            if(temp->next)
              temp->next->prev=temp->prev;
            if(rcmd[temp->line].if_flag > 0) {
              temp->next=scan->if_next;
              if(scan->if_next)
                scan->if_next->prev=temp;
              scan->if_next=temp;
              temp->prev=scan;
            }
            else {
              temp->next=scan->else_next;
              if(scan->else_next)
                scan->else_next->prev=temp;
              scan->else_next=temp;
              temp->prev=scan;
            }
            break;
          }
          else {
            for(prev_c=c=temp->line; c != count; prev_c=c, c=rcmd[c].depend);
            count=prev_c;
            if(rcmd[c].if_flag > 0)
              start=scan->if_next;
            else
              start=scan->else_next;
          }
        }
        else {
          count=rcmd[count].depend;
        }
      }
    }
  }
  sprintf(line, "#%d  %s\r\nBottom:[%d]   Top:[%d]\r\nReset mode:[%s]    Lifespan:[%d]    Age:[%d]\r\n%s %s\r\n",
          zone_table[zone].number, zone_table[zone].name, zone_table[zone].bottom,
          zone_table[zone].top, (zone_table[zone].reset_mode ? ((zone_table[zone].reset_mode==1)? "desertion" : "any time") : "never"),
          zone_table[zone].lifespan, zone_table[zone].age,
          (zone_table[zone].closed ? "Closed" : "Open"),
          (zone_table[zone].locked_by[0] ? zone_table[zone].locked_by : ""));
  zbuf=str_dup(line);
  if(tz->next) {
    print_zone_commands(tz, 0, ch, &zbuf);
  }
  else {
    RECREATE(zbuf, char, strlen(zbuf)+25);
    strcat(zbuf, "No zone commands.\r\n");
  }
  page_string(ch->desc, zbuf, 1);
  free(zbuf);
  free_zcmd(tz);
}


/* Write all database files for a zone whose rnum is j */
void save_zone(int j)
{
  int nr, i, k, arg1, arg3, exit_info, to_room;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct extra_descr_data *ex;
  FILE *fp;

  sprintf(fname, "world/zon/%d.zon", zone_table[j].number);
  if((fp=fopen(fname, "w"))) {
    sprintf(buf, "#%d\n", zone_table[j].number);
    fputs(buf, fp);
    sprintf(buf, "%s~\n", zone_table[j].name);
    fputs(buf, fp);
    sprintf(buf, "%d %d %d %d %d\n", zone_table[j].bottom, zone_table[j].top,
            zone_table[j].lifespan, zone_table[j].reset_mode, zone_table[j].closed);
    fputs(buf, fp);
    for(i=0; zone_table[j].cmd[i].command != 'S'; i++) {
      arg1=zone_table[j].cmd[i].arg1;
      arg3=zone_table[j].cmd[i].arg3;
      switch (zone_table[j].cmd[i].command) {
      case 'M':
        arg1 = mob_index[zone_table[j].cmd[i].arg1].virtual;
        arg3 = world[zone_table[j].cmd[i].arg3].number;
        break;
      case 'O':
        arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
        if (zone_table[j].cmd[i].arg3 != NOWHERE)
          arg3 = world[zone_table[j].cmd[i].arg3].number;
        break;
      case 'G':
        arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
        arg3 = mob_index[zone_table[j].cmd[i].arg3].virtual;
        break;
      case 'E':
        arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
        arg3 = mob_index[zone_table[j].cmd[i].arg3].virtual;
        break;
      case 'P':
        arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
        arg3 = obj_index[zone_table[j].cmd[i].arg3].virtual;
        break;
      case 'D':
        arg1 = world[zone_table[j].cmd[i].arg1].number;
        break;
      case 'R': /* rem obj from room */
        arg1 = world[zone_table[j].cmd[i].arg1].number;
        break;
      }
      if(zone_table[j].cmd[i].command=='R')
        sprintf(buf, "%c %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                arg1, obj_index[zone_table[j].cmd[i].arg2].virtual,
                zone_table[j].cmd[i].prob);
      else if(zone_table[j].cmd[i].command=='E')
        sprintf(buf, "%c %d %d %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                arg1, zone_table[j].cmd[i].limit,
                arg3, zone_table[j].cmd[i].arg4,
                zone_table[j].cmd[i].prob);
      else
        sprintf(buf, "%c %d %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                arg1, zone_table[j].cmd[i].limit,
                arg3, zone_table[j].cmd[i].prob);
      fputs(buf, fp);
    }
    fputs("S\n", fp);
    fputs("$\n", fp);
    fclose(fp);
  }

  sprintf(fname, "world/mob/%d.mob", zone_table[j].number);
  if((fp=fopen(fname, "w"))) {
    for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
      if((nr=real_mobile(i)) >= 0) {
        sprintf(buf, "#%d\n", i);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", mob_proto[nr].player.name);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", mob_proto[nr].player.short_descr);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", mob_proto[nr].player.long_descr);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", mob_proto[nr].player.description);
        fputs(buf, fp);
        sprintf(buf, "%lld %lld %d W3\n", MOB_FLAGS(mob_proto+nr), AFF_FLAGS(mob_proto+nr), GET_ALIGNMENT(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d %dd%d+%d %dd%d+%d\n", GET_LEVEL(mob_proto+nr), GET_HITROLL(mob_proto+nr),
                GET_AC(mob_proto+nr)/10, GET_HIT(mob_proto+nr), GET_MANA(mob_proto+nr), GET_MOVE(mob_proto+nr),
                mob_proto[nr].mob_specials.damnodice, mob_proto[nr].mob_specials.damsizedice, GET_DAMROLL(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%ld %ld\n", GET_GOLD(mob_proto+nr), GET_EXP(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d\n", GET_POS(mob_proto+nr), GET_DEFAULT_POS(mob_proto+nr), GET_SEX(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d %d %d %d %d\n", mob_proto[nr].real_abils.str, mob_proto[nr].real_abils.str_add,
                mob_proto[nr].real_abils.intel, mob_proto[nr].real_abils.wis, mob_proto[nr].real_abils.dex,
                mob_proto[nr].real_abils.con, mob_proto[nr].real_abils.cha);
        fputs(buf, fp);
        sprintf(buf, "%d %d %d\n", mob_proto[nr].mob_specials.spec_proc, mob_proto[nr].mob_specials.size, GET_CLASS(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d %d %d %d\n", GET_AC(mob_proto+nr), mob_proto[nr].mob_specials.attack_type,
                mob_proto[nr].mob_specials.attacks, mob_proto[nr].char_specials.hp_regen_add, GET_MR(mob_proto+nr), GET_PR(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d %d %d\n", GET_SAVE(mob_proto+nr, 0), GET_SAVE(mob_proto+nr, 1),
                GET_SAVE(mob_proto+nr, 2), GET_SAVE(mob_proto+nr, 3), GET_SAVE(mob_proto+nr, 4));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d\n", IMMUNE_FLAGS(mob_proto+nr), WEAK_FLAGS(mob_proto+nr), RESIST_FLAGS(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%d %d\n", GET_MOVE_RATE(mob_proto+nr), GET_FEROCITY(mob_proto+nr));
        fputs(buf, fp);
        sprintf(buf, "%s~\n", (ACTIONS(mob_proto+nr)?ACTIONS(mob_proto+nr):""));
        fputs(buf, fp);
      }
    }
    fputs("$\n", fp);
    fclose(fp);
  }

  sprintf(fname, "world/obj/%d.obj", zone_table[j].number);
  if((fp=fopen(fname, "w"))) {
    for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
      if((nr=real_object(i)) >= 0) {
        sprintf(buf, "#%d\n", i);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", obj_proto[nr].name);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", obj_proto[nr].short_description);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", obj_proto[nr].description);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", obj_proto[nr].action_description);
        fputs(buf, fp);
        sprintf(buf, "%d %ld %ld %lld\n", GET_OBJ_TYPE(obj_proto+nr), GET_OBJ_EXTRA(obj_proto+nr),
                GET_OBJ_WEAR(obj_proto+nr), obj_proto[nr].obj_flags.bitvector);
        fputs(buf, fp);
        sprintf(buf, "%d %d %d\n", obj_proto[nr].obj_flags.immune, obj_proto[nr].obj_flags.weak, obj_proto[nr].obj_flags.resist);
        fputs(buf, fp);
        sprintf(buf, "%d %d %d %d\n", GET_OBJ_VAL(obj_proto+nr, 0), GET_OBJ_VAL(obj_proto+nr, 1),
                GET_OBJ_VAL(obj_proto+nr, 2), GET_OBJ_VAL(obj_proto+nr, 3));
        fputs(buf, fp);
        sprintf(buf, "%d %d %d\n", GET_OBJ_WEIGHT(obj_proto+nr), GET_OBJ_COST(obj_proto+nr), GET_OBJ_RENT(obj_proto+nr));
        fputs(buf, fp);
        for(k=0; k<MAX_OBJ_AFFECT; k++) {
          if(obj_proto[nr].affected[k].location != APPLY_NONE) {
            fputs("A\n", fp);
            sprintf(buf, "%d %d\n", obj_proto[nr].affected[k].location, obj_proto[nr].affected[k].modifier);
            fputs(buf, fp);
          }
        }
        for(ex=obj_proto[nr].ex_description; ex; ex=ex->next) {
          fputs("E\n", fp);
          sprintf(buf, "%s~\n", ex->keyword);
          fputs(buf, fp);
          sprintf(buf, "%s~\n", ex->description);
          fputs(buf, fp);
        }
      }
    }
    fputs("$\n", fp);
    fclose(fp);
  }

  sprintf(fname, "world/wld/%d.wld", zone_table[j].number);
  if((fp=fopen(fname, "w"))) {
    for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
      if((nr=real_room(i)) >= 0) {
        sprintf(buf, "#%d\n", i);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", world[nr].name);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", world[nr].description);
        fputs(buf, fp);
        sprintf(buf, "%d %ld %d\n", zone_table[j].number, world[nr].room_flags, world[nr].sector_type);
        fputs(buf, fp);
        sprintf(buf, "%dd%d+%d %d %d\n", world[nr].dt_numdice, world[nr].dt_sizedice,
                world[nr].dt_add, world[nr].dt_percent, world[nr].dt_repeat);
        fputs(buf, fp);
        for(k=0; k<NUM_OF_DIRS; k++) {
          if(world[nr].dir_option[k]) {
            sprintf(buf, "D%d\n", k);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", world[nr].dir_option[k]->general_description);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", world[nr].dir_option[k]->keyword);
            fputs(buf, fp);
            if(IS_SET(world[nr].dir_option[k]->exit_info, EX_ISDOOR)) {
              if(IS_SET(world[nr].dir_option[k]->exit_info, EX_PICKPROOF))
                exit_info=2;
              else
                exit_info=1;
              if(IS_SET(world[nr].dir_option[k]->exit_info, EX_SECRET)) {
                exit_info+=2;
                if(IS_SET(world[nr].dir_option[k]->exit_info, EX_HIDDEN))
                  exit_info+=2;
              }
            }
            else
              exit_info=0;
            to_room=world[nr].dir_option[k]->to_room;
            if(to_room > 0)
              to_room=world[to_room].number;
            sprintf(buf, "%d %d %d\n", exit_info, world[nr].dir_option[k]->key, to_room);
            fputs(buf, fp);
          }
        }
        for(ex=world[nr].ex_description; ex; ex=ex->next) {
          fputs("E\n", fp);
          sprintf(buf, "%s~\n", ex->keyword);
          fputs(buf, fp);
          sprintf(buf, "%s~\n", ex->description);
          fputs(buf, fp);
        }
        fputs("S\n", fp);
      }
    }
    fputs("$\n", fp);
    fclose(fp);
  }
}


ACMD(do_delword)
{
  int i, line, word, space;
  char *ptr, *ptr2;

  skip_spaces(&argument);
  two_arguments(argument, arg, buf);

  if((!*arg) || (!*buf) || (!is_number(arg)) || (!is_number(buf))) {
    send_to_char("Usage: delword <line num> <word num>\r\n", ch);
    return;
  }

  if((!HAS_OLC(ch, zone_table[world[ch->in_room].zone].number)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  line=atoi(arg);
  word=atoi(buf);

  for(ptr=world[ch->in_room].description, i=1; (i<line) && (*ptr); ptr++) {
    if((*ptr) == '\n')
      i++;
  }
  if((!*ptr) || (line < 1)) {
    send_to_char("No such line.\r\n", ch);
    return;
  }

  if((line > 1) && ((*(ptr-1)) != '\r') && ((*(ptr+1)) == '\r'))
    ptr++;
  if(((*ptr)=='\r') || ((*ptr)=='\n'))
    ptr++;

  for(space=1, i=1; (i <= word) && (*ptr); ptr++) {
    if(((*ptr) == '\r') || ((*ptr) == '\n')) {
      word=-1;
      break;
    }
    if(isspace(*ptr)) {
      space=1;
    }
    else {
      if(space)
        i++;
      space=0;
    }
  }
  if((!*ptr) || (word < 1)) {
    send_to_char("No such word number on that line.\r\n", ch);
    return;
  }

  ptr--;

  for(ptr2=world[ch->in_room].description; *ptr2; ptr2++) {
    if(ptr2==ptr) {
      while((*ptr2) && (!isspace(*ptr2)) && ((*ptr2)!='\r') && ((*ptr2)!='\n'))
        ptr2++;
      while((*ptr2) && (isspace(*ptr2)) && ((*ptr2)!='\r') && ((*ptr2)!='\n'))
        ptr2++;
      if((!*ptr2) || ((*ptr2)=='\r') || ((*ptr2)=='\n')) {
        if(ptr > world[ch->in_room].description)
          ptr--;
        while((ptr > world[ch->in_room].description) && (isspace(*ptr)))
          ptr--;
        if(!isspace(*ptr))
          ptr++;
        if(!*ptr2)
          break;
      }
    }
    if(ptr2 > ptr)
      (*(ptr++))=(*ptr2);
  }
  *ptr=0;

  save_zone(world[ch->in_room].zone);
  send_to_char(OK, ch);
}

ACMD(do_format)
{
  int vnum, i, j, rnum;
  struct extra_descr_data *ex;
  struct char_data *tch;

  void format_string(char **str);

  skip_spaces(&argument);
  if(!*argument) {
    if((TOGGLE_BIT(PRF_FLAGS(ch), PRF_FORMAT)) & PRF_FORMAT)
      send_to_char("The mud will now automaticaly format OLC text strings for you.\r\n", ch);
    else
      send_to_char("The mud no longer format OLC text strings for you.\r\n", ch);
    return;
  }

  two_arguments(argument, arg, buf);
  if((!*buf) || (!is_number(buf))) {
    send_to_char("Usage: format [{mob | obj | room} <number>]\r\n", ch);
    return;
  }

  vnum=atoi(buf);
  for(i=0; i<=top_of_zone_table; i++) {
    if((vnum >= zone_table[i].bottom) && (vnum <= zone_table[i].top))
      break;
  }
  if(i > top_of_zone_table) {
    send_to_char("That vnum is outside of any zone.\r\n", ch);
    return;
  }
  if((!HAS_OLC(ch, zone_table[i].number)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  if(!strn_cmp("mob", arg, strlen(arg))) {
    if(real_mobile(vnum) >= 0) {
      format_string(&mob_proto[real_mobile(vnum)].player.description);
      save_zone(i);
      for(tch=character_list; tch; tch=tch->next) {
        if(IS_NPC(tch) && (GET_MOB_VNUM(tch)==vnum)) {
          tch->player.description=mob_proto[real_mobile(vnum)].player.description;
        }
      }
    }
    else
      send_to_char("No such mobile.\r\n", ch);
  }
  else if(!strn_cmp("obj", arg, strlen(arg))) {
    if(real_object(vnum) >= 0) {
      for(ex=obj_proto[real_object(vnum)].ex_description; ex; ex=ex->next) {
        format_string(&ex->description);
      }
      save_zone(i);
    }
    else
      send_to_char("No such object.\r\n", ch);
  }
  else if(!strn_cmp("room", arg, strlen(arg))) {
    if((rnum=real_room(vnum)) >= 0) {
      format_string(&world[rnum].description);
      for(ex=world[rnum].ex_description; ex; ex=ex->next) {
        format_string(&ex->description);
      }
      for(j=0; j<NUM_OF_DIRS; j++) {
        if(world[rnum].dir_option[j] && world[rnum].dir_option[j]->general_description)
          format_string(&world[rnum].dir_option[j]->general_description);
      }
      save_zone(i);
    }
    else
      send_to_char("No such object.\r\n", ch);
  }
  else {
    send_to_char("Usage: format [{mob | obj | room} <number>]\r\n", ch);
  }
}

ACMD(do_insword)
{
  int i, line, word, space, end=0;
  char *ptr, *ptr2, *new_str;

  skip_spaces(&argument);
  argument=two_arguments(argument, arg, buf);
  skip_spaces(&argument);

  if((!*arg) || (!*buf) || (!is_number(arg)) || (!is_number(buf))) {
    send_to_char("Usage: insword <line num> <word num> <word>\r\n", ch);
    return;
  }

  if((!HAS_OLC(ch, zone_table[world[ch->in_room].zone].number)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  line=atoi(arg);
  word=atoi(buf);

  for(ptr=world[ch->in_room].description, i=1; (i<line) && (*ptr); ptr++) {
    if((*ptr) == '\n')
      i++;
  }
  if((!*ptr) || (line < 1)) {
    send_to_char("No such line.\r\n", ch);
    return;
  }

  if((line > 1) && ((*(ptr-1)) != '\r') && ((*(ptr+1)) == '\r'))
    ptr++;
  if(((*ptr)=='\r') || ((*ptr)=='\n'))
    ptr++;

  for(space=1, i=1; (i <= word) && (*ptr); ptr++) {
    if(((*ptr) == '\r') || ((*ptr) == '\n')) {
      if(i == word)
        end=1;
      else
        word=-1;
      break;
    }
    if(isspace(*ptr)) {
      space=1;
    }
    else {
      if(space)
        i++;
      space=0;
    }
  }
  if((!*ptr) || (word < 1)) {
    send_to_char("No such word number on that line.\r\n", ch);
    return;
  }

  if(!end)
    ptr--;

  ptr2=world[ch->in_room].description;
  CREATE(new_str, char, strlen(ptr2)+strlen(argument)+3);

  for(i=0; ptr2 < ptr; ptr2++)
    new_str[i++]=*ptr2;
  if(end)
    new_str[i++]=' ';
  for(ptr=argument; *ptr; ptr++)
    new_str[i++]=*ptr;
  if(!end)
    new_str[i++]=' ';
  for(; *ptr2; ptr2++)
    new_str[i++]=*ptr2;
  new_str[i]=0;

  free(world[ch->in_room].description);
  world[ch->in_room].description=new_str;

  save_zone(world[ch->in_room].zone);
  send_to_char(OK, ch);
}

ACMD(do_olc)
{
  struct char_data *tch=NULL;
  int min, max, is_file=0, player_i=0;
  struct char_file_u tmp_store;

  skip_spaces(&argument);

  if(!*argument) {
    send_to_char("Usage: olc [file] <player> [none | min_zone1-max_zone1 [min_zone2-max_zone2]]\r\n", ch);
    return;
  }

  argument=one_argument(argument, arg);

  if(!str_cmp(arg, "file")) {
    is_file=1;
    skip_spaces(&argument);
    argument=one_argument(argument, arg);
    CREATE(tch, struct char_data, 1);
    clear_char(tch);
    if ((player_i = load_char(arg, &tmp_store)) > -1) {
      store_to_char(&tmp_store, tch);
      if (GET_LEVEL(tch) > GET_LEVEL(ch)) {
	free_char(tch);
        send_to_char("You can't see or set OLC on someone that level.\r\n", ch);
	return;
      }
    } else {
      free(tch);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  else {
    if(!(tch=get_player_vis(ch, arg))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  }

  skip_spaces(&argument);

  if(*argument) {
    if((GET_LEVEL(tch) >= GET_LEVEL(ch)) || (GET_LEVEL(ch) < LVL_ASST)) {
      send_to_char("You can only view OLC privileges for that person.\r\n", ch);
      if(is_file)
        free_char(tch);
      return;
    }

    min=max-1;
    argument=one_argument(argument, arg);
    if(!str_cmp(arg, "none")) {
      tch->player_specials->saved.olc_min1=0;
      tch->player_specials->saved.olc_max1=0;
      tch->player_specials->saved.olc_min2=0;
      tch->player_specials->saved.olc_max2=0;
    }
    else {
      sscanf(arg, " %d-%d ", &min, &max);
      if(((min < 0) || (max < 0)) || (max < min)) {
        send_to_char("Usage: olc [file] <player> [min_zone1-max_zone1 [min_zone2-max_zone2]]\r\n", ch);
        if(is_file)
          free_char(tch);
        return;
      }
      if((min == 0) || (max == 0)) {
        send_to_char("You cannot grant OLC privileges to zone 0\r\n", ch);
        if(is_file)
          free_char(tch);
        return;
      }
      tch->player_specials->saved.olc_min1=min;
      tch->player_specials->saved.olc_max1=max;
      tch->player_specials->saved.olc_min2=0;
      tch->player_specials->saved.olc_max2=0;

      skip_spaces(&argument);
      if(*argument) {
        min=max=-1;
        one_argument(argument, arg);
        sscanf(arg, " %d-%d ", &min, &max);
        if(((min < 0) || (max < 0)) || (max < min)) {
          send_to_char("Usage: olc [file] <player> [min_zone1-max_zone1 [min_zone2-max_zone2]]\r\n", ch);
          if(is_file)
            free_char(tch);
          return;
        }
        if((min == 0) || (max == 0)) {
          send_to_char("You cannot grant OLC privileges to zone 0\r\n", ch);
          if(is_file)
            free_char(tch);
          return;
        }
        tch->player_specials->saved.olc_min2=min;
        tch->player_specials->saved.olc_max2=max;
      }
    }
    send_to_char(OK, ch);
  }
  else {
    if((tch->player_specials->saved.olc_min1 > 0) || (tch->player_specials->saved.olc_max1 > 0)) {
      if((tch->player_specials->saved.olc_min2 > 0) || (tch->player_specials->saved.olc_max2 > 0)) {
        sprintf(buf, "%s has OLC privileges to zones %d-%d and %d-%d\r\n",
                GET_NAME(tch), tch->player_specials->saved.olc_min1,
                tch->player_specials->saved.olc_max1,
                tch->player_specials->saved.olc_min2,
                tch->player_specials->saved.olc_max2);
      }
      else {
        sprintf(buf, "%s has OLC privileges to zones %d-%d\r\n", GET_NAME(tch),
                tch->player_specials->saved.olc_min1,
                tch->player_specials->saved.olc_max1);
      }
    }
    else {
      sprintf(buf, "%s has no OLC privileges\r\n", GET_NAME(tch));
    }
    send_to_char(buf, ch);
  }

  if(is_file) {
    char_to_store(tch, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(tch);
  }
}

ACMD(do_zbottom)
{
  int zone, bottom, i;

  skip_spaces(&argument);
  two_arguments(argument, arg, buf1);

  if((!*arg) || (!*buf1) || (!is_number(arg)) || (!is_number(buf1))) {
    send_to_char("Usage: zbottom <zone_num> <bottom_vnum>\r\n", ch);
    return;
  }

  zone=atoi(arg);
  bottom=atoi(buf1);

  if(zone == 0) {
    send_to_char("You can't change the bottom of zone 0!\r\n", ch);
    return;
  }
  if((zone=real_zone(zone)) < 0) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }

  if(bottom < 0) {
    send_to_char("A negative vnum?!?\r\n", ch);
    return;
  }

  if(bottom > zone_table[zone].top) {
    send_to_char("You can't make the bottom higher than the top!\r\n", ch);
    return;
  }

  if(bottom > zone_table[zone].bottom) {
    for(i=zone_table[zone].bottom; i < bottom; i++)
      if((real_room(i) >= 0) || (real_mobile(i) >= 0) || (real_object(i) >= 0))
        break;
    if(i != bottom) {
      send_to_char("You cannot move the bottom that high,\r\nit would put a room, mob, or obj outside of any zone.\r\n", ch);
      return;
    }
  }

  if((zone != 0) && (bottom <= zone_table[zone-1].top)) {
    send_to_char("That would colide with the zone below it!\r\n", ch);
    return;
  }

  zone_table[zone].bottom=bottom;
  save_zone(zone);
  send_to_char(OK, ch);
}

void new_index(char *fname, char *suf, int number)
{
  int i;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  FILE *fp, *fl;

  if(!(fl=fopen(fname, "r")))
    return;

  if(!(fp=fopen("tmpindex", "w"))) {
    fclose(fl);
    return;
  }

  while(1) {
    fgets(buf, MAX_INPUT_LENGTH-2, fl);
    if(!feof(fl) && (*buf != '$')) {
      sscanf(buf, " %d.", &i);
      if(i > number) {
        sprintf(buf2, "%d.%s\n", number, suf);
        fputs(buf2, fp);
        number=100000;
      }
      fputs(buf, fp);
    }
    else {
      if(number < 100000) {
        sprintf(buf2, "%d.%s\n", number, suf);
        fputs(buf2, fp);
      }
      fputs("$\n", fp);
      break;
    }
  }
  fclose(fl);
  fclose(fp);
  unlink(fname);
  rename("tmpindex", fname);
}

ACMD(do_zcreate)
{
  int zone, bottom, top, i, j;
  char fname[MAX_INPUT_LENGTH];
  FILE *fp;

  skip_spaces(&argument);
  argument=two_arguments(argument, arg, buf);
  skip_spaces(&argument);
  one_argument(argument, buf1);

  if((!*arg) || (!*buf) || (!*buf1) || (!is_number(arg)) ||
     (!is_number(buf)) || (!is_number(buf1))) {
    send_to_char("Usage: zcreate <zone> <bottom> <top>\r\n", ch);
    return;
  }

  zone=atoi(arg);
  bottom=atoi(buf);
  top=atoi(buf1);

  for(i=1; i<=top_of_zone_table; i++) {
    if((zone_table[i-1].top < bottom) && (zone_table[i].bottom > top))
      break;
  }
  if(i > top_of_zone_table) {
    if(zone_table[top_of_zone_table].top >= bottom) {
      send_to_char("That vnum range conflicts with an existing zone.\r\n", ch);
      return;
    }
    if(zone_table[top_of_zone_table].number >= zone) {
      send_to_char("That zone number is used or is invalid with the vnum range.\r\n", ch);
      return;
    }
  }
  else {
    if((zone_table[i-1].number >= zone) || (zone_table[i].number <= zone)) {
      send_to_char("That zone number is used or is invalid with the vnum range.\r\n", ch);
      return;
    }
  }

  top_of_zone_table++;

  RECREATE(zone_table, struct zone_data, top_of_zone_table+1);
  for(j=top_of_zone_table; j>i; j--) {
    zone_table[j]=zone_table[j-1];
  }

  zone_table[i].name=str_dup("New zone");
  zone_table[i].lifespan=20;
  zone_table[i].age=1;
  zone_table[i].top=top;
  zone_table[i].bottom=bottom;
  zone_table[i].reset_mode=2;
  zone_table[i].number=zone;
  zone_table[i].closed=1;
  zone_table[i].locked_by[0]=0;
  CREATE(zone_table[i].cmd, struct reset_com, 1);
  zone_table[i].cmd[0].command='S';
  for(j=0; j<=top_of_world; j++) {
    if(world[j].zone >= i)
      world[j].zone++;
  }

  save_zone(i);
  sprintf(fname, "world/mob/%d.mob", zone);
  if((fp=fopen(fname, "w"))) {
    fputs("$\n", fp);
    fclose(fp);
  }
  sprintf(fname, "world/obj/%d.obj", zone);
  if((fp=fopen(fname, "w"))) {
    fputs("$\n", fp);
    fclose(fp);
  }
  sprintf(fname, "world/wld/%d.wld", zone);
  if((fp=fopen(fname, "w"))) {
    fputs("$\n", fp);
    fclose(fp);
  }
  sprintf(fname, "world/prg/%d.prg", zone);
  if((fp=fopen(fname, "w"))) {
    fputs("$\n", fp);
    fclose(fp);
  }

  new_index("world/mob/index", "mob", zone);
  new_index("world/obj/index", "obj", zone);
  new_index("world/wld/index", "wld", zone);
  new_index("world/zon/index", "zon", zone);
  new_index("world/prg/index", "prg", zone);

  send_to_char(OK, ch);
  sprintf(buf, "(GC) %s created zone #%d (%s)", GET_NAME(ch), zone, zone_table[real_zone(zone)].name);
  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
}

void remove_index(char *fname, int number)
{
  int i;
  char buf[MAX_INPUT_LENGTH];
  FILE *fp, *fl;

  if(!(fl=fopen(fname, "r")))
    return;

  if(!(fp=fopen("tmpindex", "w"))) {
    fclose(fl);
    return;
  }

  while(1) {
    fgets(buf, MAX_INPUT_LENGTH-2, fl);
    if(!feof(fl) && (*buf != '$')) {
      sscanf(buf, " %d.", &i);
      if(i != number) {
        fputs(buf, fp);
      }
    }
    else {
      fputs("$\n", fp);
      break;
    }
  }
  fclose(fl);
  fclose(fp);
  unlink(fname);
  rename("tmpindex", fname);
}

ACMD(do_zdelete)
{
  int zone, temp;
  char oldname[MAX_INPUT_LENGTH], newname[MAX_INPUT_LENGTH];

  if(subcmd != SCMD_DELETE) {
    send_to_char("If you really want to permanently delete a zone,\r\nyou have to type out the whole command.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  if((!*argument)||(!is_number(argument))) {
    send_to_char("Usage: zdelete <zone>\r\n", ch);
    return;
  }
  zone=atoi(argument);

  if(zone == 0) {
    send_to_char("You can't delete zone 0!\r\n", ch);
    return;
  }
  if(real_zone(zone) < 0) {
    send_to_char("There is no such zone.\r\n", ch);
    return;
  }

  temp=zone_table[real_zone(zone)].bottom;
  zone_table[real_zone(zone)].bottom=zone_table[real_zone(zone)].top;
  zone_table[real_zone(zone)].top=temp;

  sprintf(newname, "../../backup/%d.mob", zone);
  sprintf(oldname, "world/mob/%d.mob", zone);
  rename(oldname, newname);
  remove_index("world/mob/index", zone);

  sprintf(newname, "../../backup/%d.obj", zone);
  sprintf(oldname, "world/obj/%d.obj", zone);
  rename(oldname, newname);
  remove_index("world/obj/index", zone);

  sprintf(newname, "../../backup/%d.wld", zone);
  sprintf(oldname, "world/wld/%d.wld", zone);
  rename(oldname, newname);
  remove_index("world/wld/index", zone);

  sprintf(newname, "../../backup/%d.zon", zone);
  sprintf(oldname, "world/zon/%d.zon", zone);
  rename(oldname, newname);
  remove_index("world/zon/index", zone);

  sprintf(newname, "../../backup/%d.prg", zone);
  sprintf(oldname, "world/prg/%d.prg", zone);
  rename(oldname, newname);
  remove_index("world/prg/index", zone);

  sprintf(newname, "../../backup/%d.shp", zone);
  sprintf(oldname, "world/shp/%d.shp", zone);
  rename(oldname, newname);
  remove_index("world/shp/index", zone);

  send_to_char("Zone deleted, changes will take effect next reboot.\r\n", ch);
  sprintf(buf, "(GC) %s deleted zone #%d (%s)", GET_NAME(ch), zone, zone_table[real_zone(zone)].name);
  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
}

ACMD(do_zlifespan)
{
  int zone, vzone, lifespan;

  skip_spaces(&argument);
  two_arguments(argument, arg, buf1);

  if((!*arg) || (!*buf1) || (!is_number(arg)) || (!is_number(buf1))) {
    send_to_char("Usage: zlifespan <zone_num> <lifespan>\r\n", ch);
    return;
  }

  vzone=atoi(arg);
  lifespan=atoi(buf1);

  if((zone=real_zone(vzone)) < 0) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }

  if((!HAS_OLC(ch, vzone)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  if((lifespan < 3) || (lifespan > 300)) {
    send_to_char("Valid values for lifespan are 3 - 300\r\n", ch);
    return;
  }

  zone_table[zone].lifespan = lifespan;
  save_zone(zone);
  send_to_char(OK, ch);
}

ACMD(do_zlock)
{
  int zone;

  skip_spaces(&argument);
  if(!is_number(argument)) {
    send_to_char("Usage: zlock <zone>\r\n", ch);
    return;
  }
  zone=atoi(argument);

  if(zone == 0) {
    send_to_char("You can't lock zone 0!\r\n", ch);
    return;
  }
  if(real_zone(zone) < 0) {
    send_to_char("There is no such zone.\r\n", ch);
    return;
  }

  if((!HAS_OLC(ch, zone)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  if(ch->player_specials->zone_locked > 0) {
    send_to_char("You already have a zone locked.\r\n", ch);
    return;
  }

  if(!zone_table[real_zone(zone)].closed) {
    send_to_char("You can't lock an open zone!\r\n", ch);
    return;
  }

  ch->player_specials->zone_locked=zone;
  strcpy(zone_table[real_zone(zone)].locked_by, GET_NAME(ch));

  zlock_reset=1;
  do_zpurge(ch, argument, 0, 1);
  do_zreset(ch, argument, 0, 1);
  zlock_reset=0;

  sprintf(buf, "(GC) %s locked zone #%d (%s)", GET_NAME(ch), zone_table[real_zone(zone)].number, zone_table[real_zone(zone)].name);
  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);

  send_to_char(OK, ch);
}

ACMD(do_zname)
{
  int zone, vzone;

  skip_spaces(&argument);
  argument = one_argument(argument, arg);
  skip_spaces(&argument);

  if((!*arg) || (!is_number(arg)) || (!*argument)) {
    send_to_char("Usage: zname <zone_num> <name>\r\n", ch);
    return;
  }

  vzone=atoi(arg);

  if((zone=real_zone(vzone)) < 0) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }

  if((!HAS_OLC(ch, vzone)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }

  zone_table[zone].name = str_dup(argument);
  save_zone(zone);
  send_to_char(OK, ch);
}

ACMD(do_zopen)
{
  int zone;

  skip_spaces(&argument);
  one_argument(argument, arg);

  if((!*arg) || (!is_number(arg))) {
    if(subcmd != SCMD_CLOSE)
      send_to_char("Usage: zopen <zone_num>\r\n", ch);
    else
      send_to_char("Usage: zclose <zone_num>\r\n", ch);
    return;
  }

  zone=atoi(arg);

  if((zone=real_zone(zone)) < 0) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }

  if(subcmd != SCMD_CLOSE)
    zone_table[zone].closed=0;
  else
    zone_table[zone].closed=1;
  send_to_char(OK, ch);
  sprintf(buf, "(GC) %s has %s zone %d", GET_NAME(ch), (subcmd == SCMD_CLOSE) ? "closed" : "opened", zone_table[zone].number);
  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  save_zone(zone);
}
ACMD(do_zrmode)
{
  int zone, vzone, mode;
  skip_spaces(&argument);
  two_arguments(argument, arg, buf1);
  if((!*arg) || (!*buf1) || (!is_number(arg)) || (!is_number(buf1))) {
    send_to_char("Usage: zrmode <zone_num> <mode>\r\n"
                 "  mode can be: 0 - never reset\r\n"
                 "               1 - only reset when the zone is empty\r\n"
                 "               2 - reset whenever age=lifespan\r\n", ch);
    return;
  }
  vzone=atoi(arg);
  mode=atoi(buf1);
  if((zone=real_zone(vzone)) < 0) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }
  if((!HAS_OLC(ch, vzone)) && (GET_LEVEL(ch) < LVL_CIMP)) {
    send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
    return;
  }
  if((mode < 0) || (mode > 2)) {
    send_to_char("Valid values for reset mode are:\r\n"
                 "  0 - never reset\r\n"
                 "  1 - only reset when the zone is empty\r\n"
                 "  2 - reset whenever age=lifespan\r\n", ch);
    return;
  }
  zone_table[zone].reset_mode = mode;
  save_zone(zone);
  send_to_char(OK, ch);
}
ACMD(do_ztop)
{
  int zone, top, i;
  skip_spaces(&argument);
  two_arguments(argument, arg, buf1);
  if((!*arg) || (!*buf1) || (!is_number(arg)) || (!is_number(buf1))) {
    send_to_char("Usage: ztop <zone_num> <top_vnum>\r\n", ch);
    return;
  }
  zone=atoi(arg);
  top=atoi(buf1);
  if((zone=real_zone(zone)) < 0) {
    send_to_char("No such zone.\r\n", ch);
    return;
  }
  if((top < 0) || (top >= 99999)) {
    send_to_char("Invalid vnum.\r\n", ch);
    return;
  }
  if(top < zone_table[zone].bottom) {
    send_to_char("You can't make the top lower than the bottom!\r\n", ch);
    return;
  }
  if(top < zone_table[zone].top) {
    for(i=zone_table[zone].top; i > top; i--)      if((real_room(i) >= 0) || (real_mobile(i) >= 0) || (real_object(i) >= 0))
        break;
    if(i != top) {
      send_to_char("You cannot move the top that low,\r\nit would put a room, mob, or obj outside of any zone.\r\n", ch);
      return;
    }
  }
  if((zone != top_of_zone_table) && (top >= zone_table[zone+1].bottom)) {
    send_to_char("That would colide with the zone above it!\r\n", ch);
    return;
  }
  zone_table[zone].top=top;  save_zone(zone);
  send_to_char(OK, ch);
}
int reset_cmd_comp(struct reset_com orig, struct reset_com test)
{
  if(orig.command != test.command)
    return 0;
  switch(orig.command) {
  case 'M':
  case 'O':
  case 'P':
  case 'G':
    if((orig.arg1 == test.arg1) && (orig.arg3 == test.arg3))
      return 1;
    else
      return 0;
  case 'E':
    if((orig.arg1 == test.arg1) && (orig.arg3 == test.arg3) && (orig.arg4 == test.arg4))
      return 1;
    else
      return 0;
  case 'D':
    if((orig.arg1 == test.arg1) && (orig.arg2 == test.arg2))
      return 1;
    else
      return 0;
  default:
    return 0;
  }
}
void unlock_check_obj(struct reset_com **cmds, int *num_cmds, struct obj_data *o)
{
  int obj_cmd = (*num_cmds)-2;
  struct obj_data *obj;
  for(obj=o->contains; obj; obj=obj->next_content) {
    (*num_cmds)++;
    RECREATE(*cmds, struct reset_com, *num_cmds);
    (*cmds)[(*num_cmds)-2].command='P';
    (*cmds)[(*num_cmds)-2].arg1=GET_OBJ_RNUM(obj);
    (*cmds)[(*num_cmds)-2].limit=(*cmds)[(*num_cmds)-2].arg2=-1;
    (*cmds)[(*num_cmds)-2].arg3=GET_OBJ_RNUM(o);
    (*cmds)[(*num_cmds)-2].prob=10000;
    (*cmds)[(*num_cmds)-2].if_flag=1;
    (*cmds)[(*num_cmds)-2].depend=obj_cmd;
    (*cmds)[(*num_cmds)-2].line=*num_cmds-2;
    unlock_check_obj(cmds, num_cmds, obj);
  }
}
ACMD(do_zunlock)
{
  int i, j, zone, dir, room, mob_cmd, num_cmds=1, done=0;
  struct obj_data *obj;
  struct char_data *mob;
  struct reset_com tmp, *cmds;
  skip_spaces(&argument);
  one_argument(argument, arg);
  if(!*arg) {
    send_to_char("Usage: zunlock <save | nosave>\r\n", ch);
    return;
  }
  if(ch->player_specials->zone_locked < 1) {
    send_to_char("You don't have a zone locked.\r\n", ch);
    return;
  }
  if((zone=real_zone(ch->player_specials->zone_locked)) < 0) {
    send_to_char("Invalid zone locked.\r\n", ch);
    ch->player_specials->zone_locked=0;
    for(i=0; i<=top_of_zone_table; i++) {
      if(!str_cmp(zone_table[i].locked_by, GET_NAME(ch))) {
        ch->player_specials->zone_locked=zone_table[i].number;
        break;      }
    }
    if(ch->player_specials->zone_locked)
      send_to_char("Changed to correct zone.\r\n", ch);
    else
      send_to_char("Error corrected, no zone locked.\r\n", ch);
    return;
  }
  if(!strn_cmp("save", arg, strlen(arg))) {
    CREATE(cmds, struct reset_com, 1);
    for(i=zone_table[zone].bottom; i<=zone_table[zone].top; i++) {
      if((room=real_room(i)) >= 0) {
        for(dir=0; dir < NUM_OF_DIRS; dir++) {
          if(world[room].dir_option[dir] && IS_SET(world[room].dir_option[dir]->exit_info, EX_ISDOOR)) {
            num_cmds++;
            RECREATE(cmds, struct reset_com, num_cmds);
            cmds[num_cmds-2].command='D';
            cmds[num_cmds-2].arg1=room;
            cmds[num_cmds-2].limit=cmds[num_cmds-2].arg2=dir;
            cmds[num_cmds-2].prob=10000;
            cmds[num_cmds-2].if_flag=0;
            cmds[num_cmds-2].depend=0;
            cmds[num_cmds-2].line=num_cmds-2;
            cmds[num_cmds-2].arg3=0;
            if(IS_SET(world[room].dir_option[dir]->exit_info, EX_CLOSED))
              cmds[num_cmds-2].arg3++;
            if(IS_SET(world[room].dir_option[dir]->exit_info, EX_LOCKED))
              cmds[num_cmds-2].arg3++;
          }
        }
        for(obj=world[room].contents; obj; obj=obj->next_content) {
          if(GET_OBJ_RNUM(obj)<0)
            continue;
          num_cmds++;
          RECREATE(cmds, struct reset_com, num_cmds);
          cmds[num_cmds-2].command='O';
          cmds[num_cmds-2].arg1=GET_OBJ_RNUM(obj);
          cmds[num_cmds-2].limit=cmds[num_cmds-2].arg2=-1;
          cmds[num_cmds-2].arg3=room;
          cmds[num_cmds-2].prob=10000;
          cmds[num_cmds-2].if_flag=0;
          cmds[num_cmds-2].depend=0;
          cmds[num_cmds-2].line=num_cmds-2;
          unlock_check_obj(&cmds, &num_cmds, obj);
        }
        for(mob=world[room].people; mob; mob=mob->next_in_room) {
          if(IS_NPC(mob)) {
            mob_cmd=num_cmds-1;
            num_cmds++;
            RECREATE(cmds, struct reset_com, num_cmds);
            cmds[num_cmds-2].command='M';
            cmds[num_cmds-2].arg1=GET_MOB_RNUM(mob);
            cmds[num_cmds-2].limit=cmds[num_cmds-2].arg2=1;
            cmds[num_cmds-2].arg3=room;
            cmds[num_cmds-2].prob=10000;
            cmds[num_cmds-2].if_flag=0;
            cmds[num_cmds-2].depend=0;
            cmds[num_cmds-2].line=num_cmds-2;
            for(obj=mob->carrying; obj; obj=obj->next_content) {
              num_cmds++;
              RECREATE(cmds, struct reset_com, num_cmds);
              cmds[num_cmds-2].command='G';
              cmds[num_cmds-2].arg1=GET_OBJ_RNUM(obj);
              cmds[num_cmds-2].limit=cmds[num_cmds-2].arg2=-1;
              cmds[num_cmds-2].arg3=GET_MOB_RNUM(mob);
              cmds[num_cmds-2].prob=10000;
              cmds[num_cmds-2].if_flag=1;
              cmds[num_cmds-2].depend=mob_cmd;
              cmds[num_cmds-2].line=num_cmds-2;
              unlock_check_obj(&cmds, &num_cmds, obj);
            }
            for(j=0; j<NUM_WEARS; j++) {
              if(GET_EQ(mob, j)) {
                num_cmds++;
                RECREATE(cmds, struct reset_com, num_cmds);
                cmds[num_cmds-2].command='E';
                cmds[num_cmds-2].arg1=GET_OBJ_RNUM(GET_EQ(mob, j));
                cmds[num_cmds-2].limit=cmds[num_cmds-2].arg2=-1;
                cmds[num_cmds-2].arg3=GET_MOB_RNUM(mob);
                cmds[num_cmds-2].arg4=j;
                cmds[num_cmds-2].prob=10000;
                cmds[num_cmds-2].if_flag=1;
                cmds[num_cmds-2].depend=mob_cmd;
                cmds[num_cmds-2].line=num_cmds-2;
                unlock_check_obj(&cmds, &num_cmds, GET_EQ(mob, j));
              }
            }
          }
        }
      }
    }
    cmds[num_cmds-1].command='S';
    while(!done) {
      done=1;
      for(i=0; i<(num_cmds-1); i++) {
        for(j=0; zone_table[zone].cmd[j].command != 'S'; j++) {
          if(reset_cmd_comp(cmds[i], zone_table[zone].cmd[j])) {
            if(zone_table[zone].cmd[j].if_flag) {
              if(zone_table[zone].cmd[zone_table[zone].cmd[j].depend].command == '*') {
                cmds[i].if_flag=zone_table[zone].cmd[j].if_flag;
                cmds[i].depend=zone_table[zone].cmd[zone_table[zone].cmd[j].depend].line;
              }
              else
                continue;
            }
            done=0;
            cmds[i].limit=cmds[i].arg2=zone_table[zone].cmd[j].limit;
            if(cmds[i].command != 'D')
              cmds[i].arg3=zone_table[zone].cmd[j].arg3;
            cmds[i].arg4=zone_table[zone].cmd[j].arg4;
            cmds[i].line=i;
            cmds[i].prob=zone_table[zone].cmd[j].prob;
            zone_table[zone].cmd[j].command='*';
            zone_table[zone].cmd[j].line=i;
            break;
          }
        }
      }
    }
    done=0;
    while(!done) {
      done=1;
      for(i=0; i<(num_cmds-1); i++) {
        if(cmds[i].if_flag) {
          if(cmds[i].depend > i) {
            for(j=0; j<(num_cmds-1); j++) {
              if(j==i)
                continue;
              if(cmds[j].depend==i)
                cmds[j].depend=cmds[i].depend;
              else if((cmds[j].depend>i) && (cmds[j].depend<=cmds[i].depend))
                cmds[j].depend--;
            }
            for(j=0; zone_table[zone].cmd[j].command != 'S'; j++) {
              if(zone_table[zone].cmd[j].command == '*') {
                if(zone_table[zone].cmd[j].line==i)
                  zone_table[zone].cmd[j].line=cmds[i].depend;
                else if((zone_table[zone].cmd[j].line>i) && (zone_table[zone].cmd[j].line<=cmds[i].depend))
                  zone_table[zone].cmd[j].line--;
              }
            }
            tmp=cmds[i];
            for(j=i; j<tmp.depend; j++)
              cmds[j]=cmds[j+1];
            cmds[j]=tmp;
            cmds[j].depend--;
            done=0;
          }
        }
      }
    }
    i--;
    done=0;
    while(!done) {
      done=1;
      for(j=0; zone_table[zone].cmd[j].command != 'S'; j++) {
        if(zone_table[zone].cmd[j].if_flag && (zone_table[zone].cmd[j].command != '*') &&
           (zone_table[zone].cmd[zone_table[zone].cmd[j].depend].command == '*')) {
          num_cmds++;
          RECREATE(cmds, struct reset_com, num_cmds);
          cmds[num_cmds-2].command=zone_table[zone].cmd[j].command;
          cmds[num_cmds-2].if_flag=zone_table[zone].cmd[j].if_flag;
          cmds[num_cmds-2].arg1=zone_table[zone].cmd[j].arg1;
          cmds[num_cmds-2].arg2=zone_table[zone].cmd[j].limit;
          cmds[num_cmds-2].limit=zone_table[zone].cmd[j].limit;
          cmds[num_cmds-2].arg3=zone_table[zone].cmd[j].arg3;
          cmds[num_cmds-2].arg4=zone_table[zone].cmd[j].arg4;
          cmds[num_cmds-2].line=i;
          cmds[num_cmds-2].prob=zone_table[zone].cmd[j].prob;
          cmds[num_cmds-2].depend=zone_table[zone].cmd[zone_table[zone].cmd[j].depend].line;
          cmds[num_cmds-1].command='S';
          zone_table[zone].cmd[j].command='*';
          zone_table[zone].cmd[j].line=i++;
          done=0;
        }
      }
    }
    free(zone_table[zone].cmd);
    zone_table[zone].cmd=cmds;
    save_zone(zone);
    do_zpurge(ch, itoa(zone_table[zone].number), 0, 1);
    do_zreset(ch, itoa(zone_table[zone].number), 0, 1);
    zone_table[real_zone(ch->player_specials->zone_locked)].locked_by[0]=0;
    ch->player_specials->zone_locked=0;
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s unlocked zone #%d (%s)", GET_NAME(ch), zone_table[zone].number, zone_table[zone].name);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
  else if(!strn_cmp("nosave", arg, strlen(arg))) {
    do_zpurge(ch, itoa(zone_table[zone].number), 0, 1);
    do_zreset(ch, itoa(zone_table[zone].number), 0, 1);
    zone_table[real_zone(ch->player_specials->zone_locked)].locked_by[0]=0;
    ch->player_specials->zone_locked=0;
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s unlocked zone #%d (%s)", GET_NAME(ch), zone_table[zone].number, zone_table[zone].name);
    mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
  }
  else {
    send_to_char("Usage: zunlock <save | nosave>\r\n", ch);
  }
}
void print_spells(struct char_data *ch, char *begin, char *end)
{
  int i, n;
  char line[40];
  char buf[2*MAX_STRING_LENGTH];
  strcpy(buf, begin);
  for(i=0, n=1; *spells[i] != '\n'; i++) {
    if(olc_spells[i]) {
      sprintf(line, "%2d. %-22s", n, spells[i]);
      strcat(buf, line);
      if(!(n%3))
        strcat(buf, "\r\n");
      n++;
    }
  }
  if((n-1)%3)
    strcat(buf, "\r\n");
  strcat(buf, end);
  page_string(ch->desc, buf, 1);
}
void print_choices(char *names[], const int choices[], struct char_data *ch)
{
  int i, n;
  char buf[40];
  for(i=0, n=1; *names[i] != '\n'; i++) {
    if(choices[i]) {
      sprintf(buf, "%2d. %-22s", n, names[i]);
      send_to_char(buf, ch);
      if(!(n%3))
        send_to_char("\r\n", ch);
      n++;
    }
  }
  if((n-1)%3)
    send_to_char("\r\n", ch);
}
void print_sector_choices(char *names[], const int choices[], struct char_data *ch)
{
  int i, n;
  char buf[40], buf1[40];
  for(i=0, n=1; *names[i] != '\n'; i++) {
    if(choices[i]) {
      sprintf(buf1, "%s  %d", names[i], movement_loss[i]);
      sprintf(buf, "%2d. %-22s", n, buf1);
      send_to_char(buf, ch);
      if(!(n%3))
        send_to_char("\r\n", ch);
      n++;
    }
  }
  if((n-1)%3)
    send_to_char("\r\n", ch);
}
int get_choice(char *names[], const int choices[], int n)
{
  int i;
  if(n < 1)
    return(-1);
  for(i=0; *names[i] != '\n'; i++) {
    if(choices[i]) {
      if(--n == 0) {
        return(i);
      }
    }
  }
  return(-1);
}
#define RANGE(low, high)	(MIN(MAX(cmd, low), high))
void medit_menu(struct char_data *ch, int cmd)
{
  int thac0, temp, i;
  char line[MAX_STRING_LENGTH], tbuf1[MAX_INPUT_LENGTH], *ptr;
  char tbuf2[MAX_INPUT_LENGTH], tbuf3[MAX_INPUT_LENGTH];
  struct char_data *me=(struct char_data *)GET_OLC_PTR(ch);
  int class_get_thac0(struct char_data *ch);
  int hit_gain(struct char_data * ch);
  switch(GET_OLC_FIELD(ch)) {
  case 0:
    if(cmd == OLC_MENU_NUM) {
      if(PRF_FLAGGED(ch, PRF_NOMENU))
        return;
      sprintf(line, "1. MOB:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
      sprintf(line, "2. Aliases: %s\r\n", me->player.name);
      send_to_char(line, ch);
      sprintf(line, "3. Short description: %s\r\n", me->player.short_descr);
      send_to_char(line, ch);
      sprintf(line, "4. Long description: %s", (me->player.long_descr && *me->player.long_descr) ? me->player.long_descr : "\r\n");
      send_to_char(line, ch);
      sprintf(line, "5. Description:\r\n%s", (me->player.description && *me->player.description) ? me->player.description : "");
      send_to_char(line, ch);
      sprintf(line, "6. Str:%d    7. Str add:%d    8. Int:%d    9. Wis:%d\r\n",
              me->real_abils.str, me->real_abils.str_add, me->real_abils.intel,
              me->real_abils.wis);
      send_to_char(line, ch);
      sprintf(line, "10. Dex:%d    11. Con:%d    12. Cha:%d\r\n", me->real_abils.dex, me->real_abils.con, me->real_abils.cha);
      send_to_char(line, ch);
      sprinttype(GET_CLASS(me), npc_class_types, tbuf1);
      sprinttype(GET_SEX(me), genders, tbuf2);
      sprinttype(me->mob_specials.size, npc_size, tbuf3);
      sprintf(line, "13. Level:%d  14. Class: %s  15. Alignment:%+d\r\n", GET_LEVEL(me),
              tbuf1, GET_ALIGNMENT(me));
      send_to_char(line, ch);
      sprintf(line, "16. Sex: %s  17. Size: %s\r\n", tbuf2, tbuf3);
      send_to_char(line, ch);
      sprintf(line, "18. HP:%dd%d+%d   19. HP Regen:%+d Total[%d]   20. Exp:%ld   21. Gold:%ld\r\n",
              GET_HIT(me), GET_MANA(me), GET_MOVE(me), me->char_specials.hp_regen_add,
              hit_gain(me), GET_EXP(me), GET_GOLD(me));
      send_to_char(line, ch);
      thac0 = class_get_thac0(me);
      thac0 -= 5*GET_HITROLL(me);
      thac0 -= str_app[STRENGTH_APPLY_INDEX(me)].tohit;
      thac0 -= (int) ((GET_INT(me) - 13) / 1.5);
      thac0 -= (int) ((GET_WIS(me) - 13) / 1.5);
      thac0 = -thac0;
      sprintf(line, "22. Damage:%dd%d+%d  23. Ferocity:%d  24. HR:%d THAC0[%d]  25. Attacks:%d\r\n",
              me->mob_specials.damnodice, me->mob_specials.damsizedice, GET_DAMROLL(me),
              GET_FEROCITY(me), GET_HITROLL(me), thac0, me->mob_specials.attacks);
      send_to_char(line, ch);
      sprintf(line, "26. Move Rate:%d   27. Armor:%+d AC[%+d]   28. MR:%d   39. PR:%d\r\n",
              GET_MOVE_RATE(me), GET_AC(me), compute_ac(me), GET_MR(me), GET_PR(me));
      send_to_char(line, ch);
      sprinttype(GET_DEFAULT_POS(me), position_types, tbuf1);
      sprintf(line, "29. Defaul Pos: %s   30. Saves:[%d/%d/%d/%d/%d]\r\n", tbuf1,
              GET_SAVE(me, 0), GET_SAVE(me, 1), GET_SAVE(me, 2), GET_SAVE(me, 3), GET_SAVE(me, 4));
      send_to_char(line, ch);
      sprinttype(me->mob_specials.attack_type, weapon_types, tbuf2);
      sprinttype(me->mob_specials.spec_proc, spec_proc_names, tbuf3);
      sprintf(line, "31. Attack Type: %s  32. Spec Proc: %s\r\n", tbuf2, tbuf3);
      send_to_char(line, ch);
      sprintbit(MOB_FLAGS(me), action_bits, tbuf1);
      sprintf(line, "33. MOB Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(AFF_FLAGS(me), affected_bits, tbuf1);
      sprintf(line, "34. AFF Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(IMMUNE_FLAGS(me), damtypes, tbuf1);
      sprintf(line, "35. Immune Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(WEAK_FLAGS(me), damtypes, tbuf1);
      sprintf(line, "36. Weak Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(RESIST_FLAGS(me), damtypes, tbuf1);
      sprintf(line, "37. Resist Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintf(line, "38. Action list:\r\n%s", (ACTIONS(me) && *ACTIONS(me)) ? ACTIONS(me) : "");
      send_to_char(line, ch);
      send_to_char("99. Save\r\n", ch);
      send_to_char("0. Exit\r\n", ch);
    }
    else {
      if ((cmd > 39) && (cmd != 99))
        send_to_char("Invalid command.\r\n", ch);
      else
        GET_OLC_FIELD(ch)=cmd;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 1:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nEnter vnum to copy to.\r\nYou must quit the editor or save to complete the copy.\r\nOld Number:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
    }
    else {
      for(i=0; i<=top_of_zone_table; i++) {
        if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
          break;
      }
      if(HAS_OLC(ch, zone_table[i].number) || (GET_LEVEL(ch) >= LVL_CIMP))
        GET_OLC_NUM(ch)=cmd;
      else
        send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 2:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter a series of names seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->player.name) {
        free(me->player.name);
        me->player.name=NULL;
      }
      ch->desc->str = &me->player.name;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->player.name; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 3:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the name of the mob.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->player.short_descr) {
        free(me->player.short_descr);
        me->player.short_descr=NULL;
      }
      ch->desc->str = &me->player.short_descr;
      ch->desc->max_str = OLC_SHORT_LENGTH;
    }
    else {
      for(ptr=me->player.short_descr; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 4:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the text people will see when they see the mob in a room.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->player.long_descr) {
        free(me->player.long_descr);
        me->player.long_descr=NULL;
      }
      ch->desc->str = &me->player.long_descr;
      ch->desc->max_str = OLC_LONG_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 5:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter text people will see when they look at the mob.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->player.description) {
        free(me->player.description);
        me->player.description=NULL;
      }
      ch->desc->str = &me->player.description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 6:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nStrength [%d] (0 - 25)\r\n", me->real_abils.str);
      send_to_char(line, ch);
    }
    else {
      GET_STR(me)=me->real_abils.str=RANGE(0, 25);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 7:
    if(cmd == OLC_MENU_NUM) {
      if(me->real_abils.str != 18) {
        send_to_char("\r\n\nThis only applies if strength is 18.\r\n", ch);
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        sprintf(line, "\r\n\nStrength bonus [%d] (0 - 100)\r\n", me->real_abils.str_add);
        send_to_char(line, ch);
      }
    }
    else {
      GET_ADD(me)=me->real_abils.str_add=RANGE(0, 100);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 8:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nIntelligence [%d] (0 - 25)\r\n", me->real_abils.intel);
      send_to_char(line, ch);
    }
    else {
      GET_INT(me)=me->real_abils.intel=RANGE(0, 25);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 9:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nWisdom [%d] (0 - 25)\r\n", me->real_abils.wis);
      send_to_char(line, ch);
    }
    else {
      GET_WIS(me)=me->real_abils.wis=RANGE(0, 25);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 10:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nDexterity [%d] (0 - 25)\r\n", me->real_abils.dex);
      send_to_char(line, ch);
    }
    else {
      GET_DEX(me)=me->real_abils.dex=RANGE(0, 25);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 11:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nConstitution [%d] (0 - 25)\r\n", me->real_abils.con);
      send_to_char(line, ch);
    }
    else {
      GET_CON(me)=me->real_abils.con=RANGE(0, 25);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 12:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nCharisma [%d] (0 - 25)\r\n", me->real_abils.cha);
      send_to_char(line, ch);
    }
    else {
      GET_CHA(me)=me->real_abils.cha=RANGE(0, 25);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 13:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nLevel [%d] (1 - 200)\r\n", GET_LEVEL(me));
      send_to_char(line, ch);
    }
    else {
      GET_LEVEL(me)=RANGE(1, 200);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 14:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(GET_CLASS(me), npc_class_types, tbuf1);
      sprintf(line, "\r\n\nClass [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(npc_class_types, olc_classes, ch);
    }
    else {
      temp=get_choice(npc_class_types, olc_classes, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      GET_CLASS(me)=temp;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 15:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nAlignment [%d] (-1000 - 1000)\r\n", GET_ALIGNMENT(me));
      send_to_char(line, ch);
    }
    else {
      GET_ALIGNMENT(me)=RANGE(-1000, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 16:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(GET_SEX(me), genders, tbuf1);
      sprintf(line, "\r\n\nSex [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(genders, olc_sex, ch);
    }
    else {
      temp=get_choice(genders, olc_sex, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      GET_SEX(me)=temp;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 17:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->mob_specials.size, npc_size, tbuf1);
      sprintf(line, "\r\n\nSize [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(npc_size, olc_size, ch);
    }
    else {
      temp=get_choice(npc_size, olc_size, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      me->mob_specials.size=temp;
      GET_WEIGHT(me)=60*(temp+1);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 18:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\n"
                    "Hit Points\r\n"
                    "1. Number of Dice [%d] (0 - 100)\r\n"
                    "2. Size of Dice [%d] (0 - 1000)\r\n"
                    "3. Added Hit Points [%d] (1 - 30000)\r\n"
                    "0. Previous Menu\r\n",
              GET_HIT(me), GET_MANA(me), GET_MOVE(me));
      send_to_char(line, ch);
    }
    else {
      switch(RANGE(0, 3)) {
      case 0:
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=100;
        break;
      case 2:
        GET_OLC_FIELD(ch)=101;
        break;
      case 3:
        GET_OLC_FIELD(ch)=102;
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 19:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nHit point regen above normal (1 per level) [%d] (-200 - 1000)\r\n", me->char_specials.hp_regen_add);
      send_to_char(line, ch);
    }
    else {
      me->char_specials.hp_regen_add=RANGE(-200, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 20:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nExperience points [%ld] (0 - 1000000)\r\n", GET_EXP(me));
      send_to_char(line, ch);
    }
    else {
      GET_EXP(me)=RANGE(0, 1000000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 21:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nGold [%ld] (0 - 1000000)\r\n", GET_GOLD(me));
      send_to_char(line, ch);
    }
    else {
      GET_GOLD(me)=RANGE(0, 1000000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 22:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\n"
                    "Attack Damage\r\n"
                    "1. Number of Dice [%d] (0 - 127)\r\n"
                    "2. Size of Dice [%d] (0 - 127)\r\n"
                    "3. Damroll [%d] (0 - 127)\r\n"
                    "0. Previous Menu\r\n",
              me->mob_specials.damnodice, me->mob_specials.damsizedice, GET_DAMROLL(me));
      send_to_char(line, ch);
    }
    else {
      switch(RANGE(0, 3)) {
      case 0:
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=103;
        break;
      case 2:
        GET_OLC_FIELD(ch)=104;
        break;
      case 3:
        GET_OLC_FIELD(ch)=105;
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 23:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\n"
                    "Ferocity (frequency of agro check) [%d] (1 - 15)\r\n"
                    "  Note: you must also set the mob agressive\r\n"
                    "1. 10%% chance every 10 seconds\r\n"
                    "2. 20%% chance every 10 seconds\r\n"
                    "3. 30%% chance every 10 seconds\r\n"
                    "4. 40%% chance every 10 seconds\r\n"
                    "5. 50%% chance every 10 seconds\r\n"
                    "6. 60%% chance every 10 seconds\r\n"
                    "7. 70%% chance every 10 seconds\r\n"
                    "8. 80%% chance every 10 seconds\r\n"
                    "9. 90%% chance every 10 seconds\r\n"
                    "10. 100%% chance every 10 seconds\r\n"
                    "11. 20%% chance every 2 seconds\r\n"
                    "12. 40%% chance every 2 seconds\r\n"
                    "13. 60%% chance every 2 seconds\r\n"
                    "14. 80%% chance every 2 seconds\r\n"
                    "15. 100%% chance every 2 seconds\r\n", GET_FEROCITY(me));
      send_to_char(line, ch);
    }
    else {
      GET_FEROCITY(me)=RANGE(1, 15);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 24:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nHitroll [%d] (-30 - 127)\r\n", GET_HITROLL(me));
      send_to_char(line, ch);
    }
    else {
      GET_HITROLL(me)=RANGE(-30, 127);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 25:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of attacks every TWO rounds [%d] (0 - 40)\r\n", me->mob_specials.attacks);
      send_to_char(line, ch);
    }
    else {
      me->mob_specials.attacks=RANGE(0, 40);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 26:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMovement Rate (chance in 10 of moving every 10 seconds) [%d] (1 - 10)\r\n", GET_MOVE_RATE(me));
      send_to_char(line, ch);
    }
    else {
      GET_MOVE_RATE(me)=RANGE(1, 10);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 27:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nArmor [%d] (-1000 - 200)\r\n", GET_AC(me));
      send_to_char(line, ch);
    }
    else {
      GET_AC(me)=RANGE(-1000, 200);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 28:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMagic Resistance (%% chance of dispelling a harmful spell) [%d] (0 - 200)\r\n", GET_MR(me));
      send_to_char(line, ch);
    }
    else {
      GET_MR(me)=RANGE(0, 200);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 29:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(GET_DEFAULT_POS(me), position_types, tbuf1);
      sprintf(line, "\r\n\nDefault Position [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(position_types, olc_position, ch);
    }
    else {
      temp=get_choice(position_types, olc_position, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      GET_POS(me)=GET_DEFAULT_POS(me)=temp;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 30:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\n"
                    "Saving Throws (lower is better) (1 - 100)\r\n"
                    "1. vs. para [%d]\r\n"
                    "2. vs. rod [%d]\r\n"
                    "3. vs. preti [%d]\r\n"
                    "4. vs. breath [%d]\r\n"
                    "5. vs. spell [%d]\r\n\n"
                    "10. Change all saves\r\n"
                    "0. Previous Menu\r\n",
              GET_SAVE(me, 0), GET_SAVE(me, 1), GET_SAVE(me, 2), GET_SAVE(me, 3), GET_SAVE(me, 4));
      send_to_char(line, ch);
    }
    else {
      switch(RANGE(0, 10)) {
      case 0:
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=106;
        break;
      case 2:
        GET_OLC_FIELD(ch)=107;
        break;
      case 3:
        GET_OLC_FIELD(ch)=108;
        break;
      case 4:
        GET_OLC_FIELD(ch)=109;
        break;
      case 5:
        GET_OLC_FIELD(ch)=110;
        break;
      case 10:
        GET_OLC_FIELD(ch)=111;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 31:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->mob_specials.attack_type, weapon_types, tbuf1);
      sprintf(line, "\r\n\nAttack Type [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(weapon_types, olc_attack, ch);
    }
    else {
      temp=get_choice(weapon_types, olc_attack, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      me->mob_specials.attack_type=temp;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 32:
    if(cmd == OLC_MENU_NUM) {
      if(((i=real_mobile(GET_OLC_NUM(ch))) >= 0) && mob_index[real_mobile(GET_OLC_NUM(ch))].func) {
        if(mob_index[i].func == shop_keeper) {
          for(temp=0; temp < top_shop; temp++) {
            if(SHOP_KEEPER(temp) == i)
              break;
          }
          i=temp;
          temp=0;
          if(i < top_shop) {
            if(SHOP_FUNC(i) != spec_proc_table[me->mob_specials.spec_proc])
              temp=1;
          }
        }
        else {
          temp=0;
          if(mob_index[i].func != spec_proc_table[me->mob_specials.spec_proc])
            temp=1;
        }
        if(temp) {
          send_to_char("\r\n\nThis mob is assigned a spec proc in the code, it cannot be changed here.\r\n", ch);
          GET_OLC_FIELD(ch)=0;
          do_olcmenu(ch, "", 0, 0);
          return;
        }
      }
      sprinttype(me->mob_specials.spec_proc, spec_proc_names, tbuf1);
      sprintf(line, "\r\n\nSpecial Procedure [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(spec_proc_names, olc_spec, ch);
    }
    else {
      temp=get_choice(spec_proc_names, olc_spec, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      me->mob_specials.spec_proc=temp;
      if(temp)
        SET_BIT(MOB_FLAGS(me), MOB_SPEC);
      else
        REMOVE_BIT(MOB_FLAGS(me), MOB_SPEC);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 33:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(MOB_FLAGS(me), action_bits, tbuf1);
      sprintf(line, "\r\n\nMOB Flags [%s]\r\n"
                    "  [Note: Doorstop flag makes mob completely invulnerable and   ]\r\n"
                    "  [completely invisible, it is for when you want a mob prog    ]\r\n"
                    "  [in a room, but don't want a mob there to be seen or attacked]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(action_bits, olc_mob_bits, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(action_bits, olc_mob_bits, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(MOB_FLAGS(me), ((long long)1 << temp));
        if((((long long)1 << temp) == MOB_HUNTER) && (IS_SET(MOB_FLAGS(me), MOB_HUNTER))) {
          REMOVE_BIT(MOB_FLAGS(me), MOB_SENTINEL);
          SET_BIT(MOB_FLAGS(me), MOB_MEMORY);
        }
        if((((long long)1 << temp) == MOB_NICE) && (IS_SET(MOB_FLAGS(me), MOB_NICE))) {
          REMOVE_BIT(MOB_FLAGS(me), MOB_AGGRESSIVE);
          REMOVE_BIT(MOB_FLAGS(me), MOB_AGGR_EVIL);
          REMOVE_BIT(MOB_FLAGS(me), MOB_AGGR_GOOD);
          REMOVE_BIT(MOB_FLAGS(me), MOB_AGGR_NEUTRAL);
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 34:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(AFF_FLAGS(me), affected_bits, tbuf1);
      sprintf(line, "\r\n\nAFF Flags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(affected_bits, olc_mobaff_bits, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(affected_bits, olc_mobaff_bits, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(AFF_FLAGS(me), ((long long)1 << temp));
        TOGGLE_BIT(me->char_specials.saved.affected_by, ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 35:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(IMMUNE_FLAGS(me), damtypes, tbuf1);
      sprintf(line, "\r\n\nImmunities [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(IMMUNE_FLAGS(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 36:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(WEAK_FLAGS(me), damtypes, tbuf1);
      sprintf(line, "\r\n\nWeaknesses [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(WEAK_FLAGS(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 37:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(RESIST_FLAGS(me), damtypes, tbuf1);
      sprintf(line, "\r\n\nResistances [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(RESIST_FLAGS(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 38:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter commands the mob will perform, one per line. All commands up to a\r\n"
                   "delay or the end of the list will be done every 10 seconds. Use the word 'delay'\r\n"
                   "pause until the next 10 seconds. Delay can be used as many times as needed and\r\n"
                   "can be used multiple times in a row to delay longer than 10 seconds.\r\n"
                   "Enter @ on a blank line when done.\r\n", ch);
      if(ACTIONS(me)) {
        free(ACTIONS(me));
        ACTIONS(me)=NULL;
      }
      ch->desc->str = &ACTIONS(me);
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 39:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nPsionic Resistance (%% chance of nullifying a psionic attack) [%d] (0 - 100)\r\n", GET_PR(me));
      send_to_char(line, ch);
    }
    else {
      GET_PR(me)=RANGE(0, 100);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 99:
    send_to_char("Saving...\r\n", ch);
    save_edit(ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  case 100:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of hit dice [%d] (0 - 100)\r\n", GET_HIT(me));
      send_to_char(line, ch);
    }
    else {
      GET_HIT(me)=RANGE(0, 100);
      GET_OLC_FIELD(ch)=18;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 101:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSize of hit dice [%d] (0 - 1000)\r\n", GET_MANA(me));
      send_to_char(line, ch);
    }
    else {
      GET_MANA(me)=RANGE(0, 1000);
      GET_OLC_FIELD(ch)=18;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 102:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nAdditional hit points [%d] (1 - 30000)\r\n", GET_MOVE(me));
      send_to_char(line, ch);
    }
    else {
      GET_MOVE(me)=RANGE(1, 30000);
      GET_OLC_FIELD(ch)=18;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 103:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of damage dice [%d] (0 - 127)\r\n", me->mob_specials.damnodice);
      send_to_char(line, ch);
    }
    else {
      me->mob_specials.damnodice=RANGE(0, 127);
      GET_OLC_FIELD(ch)=22;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 104:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSize of damage dice [%d] (0 - 127)\r\n", me->mob_specials.damsizedice);
      send_to_char(line, ch);
    }
    else {
      me->mob_specials.damsizedice=RANGE(0, 127);
      GET_OLC_FIELD(ch)=22;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 105:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nAdditional damage [%d] (0 - 127)\r\n", GET_DAMROLL(me));
      send_to_char(line, ch);
    }
    else {
      GET_DAMROLL(me)=RANGE(0, 127);
      GET_OLC_FIELD(ch)=22;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 106:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSave vs. paralisation [%d] (1 - 100)\r\n", GET_SAVE(me, 0));
      send_to_char(line, ch);
    }
    else {
      GET_SAVE(me, 0)=RANGE(1, 100);
      GET_OLC_FIELD(ch)=30;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 107:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSave vs. rods (staves and wands) [%d] (1 - 100)\r\n", GET_SAVE(me, 1));
      send_to_char(line, ch);
    }
    else {
      GET_SAVE(me, 1)=RANGE(1, 100);
      GET_OLC_FIELD(ch)=30;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 108:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSave vs. petrification [%d] (1 - 100)\r\n", GET_SAVE(me, 2));
      send_to_char(line, ch);
    }
    else {
      GET_SAVE(me, 2)=RANGE(1, 100);
      GET_OLC_FIELD(ch)=30;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 109:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSave vs. breath weapons [%d] (1 - 100)\r\n", GET_SAVE(me, 3));
      send_to_char(line, ch);
    }
    else {
      GET_SAVE(me, 3)=RANGE(1, 100);
      GET_OLC_FIELD(ch)=30;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 110:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSave vs. spells [%d] (1 - 100)\r\n", GET_SAVE(me, 4));
      send_to_char(line, ch);
    }
    else {
      GET_SAVE(me, 4)=RANGE(1, 100);
      GET_OLC_FIELD(ch)=30;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 111:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nChange all saves (1 - 100)\r\n");
      send_to_char(line, ch);
    }
    else {
      GET_SAVE(me, 0)=GET_SAVE(me, 1)=GET_SAVE(me, 2)=GET_SAVE(me, 3)=GET_SAVE(me, 4)=RANGE(1, 100);
      GET_OLC_FIELD(ch)=30;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  default:
    send_to_char("Incomplete field.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  }
}
void print_obj_type_data(struct obj_data *j, struct char_data *ch)
{
  char line[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
  sprinttype(GET_OBJ_TYPE(j), item_types, buf);
  sprintf(line, "6. Type: %s", buf);
  strcpy(buf, "\r\n");
  switch(GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "   7. Hours left: Infinite\r\n");
    else
      sprintf(buf, "   7. Hours left:%d\r\n", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "   7. Spell Level:%d   8. Spell1: %s\r\n9. Spell2: %s   10. Spell3: %s\r\n",
            GET_OBJ_VAL(j, 0), skill_name(GET_OBJ_VAL(j, 1)),
            skill_name(GET_OBJ_VAL(j, 2)), skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "   7. Spell: %s\r\n8. Level:%d   9. Charges:%d   10. Max charges:%d\r\n",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    sprinttype(GET_OBJ_VAL(j, 3), weapon_types, buf2);
    sprintf(buf, "   7. Damage Dice:%dd%d   8. Weapon Type: %s\r\n",
	    GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), buf2);
    break;
  case ITEM_ARMOR:
    sprintf(buf, "   7. AC-apply:%d\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "   7. Weight Capacity:%d\r\n8. Lock Type: %s   9. Key Num:%d\r\n",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "   7. Capacity:%d   8. Contains:%d\r\n9. Liquid: %s   10. Poisoned: %s\r\n",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), buf2, YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_FOOD:
    sprintf(buf, "   7. Makes full:%d   8. Poisoned: %s\r\n", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "   7. Coins:%d\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_BEAMER:
    sprintf(buf, "   7. Beam down room:%d (%s)\r\n", GET_OBJ_VAL(j, 0), (real_room(GET_OBJ_VAL(j, 0)) >= 0) ? world[real_room(GET_OBJ_VAL(j, 0))].name : "NOWHERE");
    break;
  case ITEM_DAMAGEABLE:
    sprintf(buf, "   7. Durability:%d   8. Condition:%d\r\n", GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 2));
    break;
  }
  strcat(line, buf);
  send_to_char(line, ch);
}
void print_obj_type(struct obj_data *obj, struct char_data *ch, int field)
{
  int ok=0;
  char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
  switch(GET_OBJ_TYPE(obj)) {
  case ITEM_LIGHT:
    if(field == 7) {
      ok=1;
      if (GET_OBJ_VAL(obj, 2) == -1)
        strcpy(buf, "\r\n\nHours of light [Infinite] (-1(infinite) - 10000)\r\n");
      else
        sprintf(buf, "\r\n\nHours of light [%d] (-1(infinite) - 10000)\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    if(field == 7) {
      sprintf(buf, "\r\n\nLevel the spells are cast at [%d] (1 - 127)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else {
      sprintf(buf, "\r\n\nSpell [%s]\r\n", skill_name(GET_OBJ_VAL(obj, field-7)));
      print_spells(ch, buf, "0. NONE\r\n");
    }
    ok=1;
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    if(field == 7) {
      sprintf(buf, "\r\n\nSpell [%s]\r\n", skill_name(GET_OBJ_VAL(obj, 3)));
      print_spells(ch, buf, "0. NONE\r\n");
    }
    else if(field == 8) {
      sprintf(buf, "\r\n\nLevel the spell is cast at [%d] (1 - 127)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else if(field == 9) {
      sprintf(buf, "\r\n\nCharges in the item [%d] (0 - 100)\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    else {
      sprintf(buf, "\r\n\nMaximum charges the item can hold [%d] (1 - 100)\r\n", GET_OBJ_VAL(obj, 1));
      send_to_char(buf, ch);
    }
    ok=1;
    break;
  case ITEM_WEAPON:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\n"
                   "1. Number of damage dice [%d] (1 - 30)\r\n"
                   "2. Size of damage dice [%d] (1 - 60)\r\n"
                   "0. Previous Menu\r\n", GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    if(field == 8) {
      ok=1;
      sprinttype(GET_OBJ_VAL(obj, 3), weapon_types, buf2);
      sprintf(buf, "\r\n\nWeapon Type [%s] (note: thief skills work only with type STAB)\r\n", buf2);
      send_to_char(buf, ch);
      print_choices(weapon_types, olc_weapons, ch);
    }
    break;
  case ITEM_ARMOR:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nAC-apply (higher is better) [%d] (-40 - 40)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_CONTAINER:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nWeight Capacity [%d] (0 - 1000)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else if(field == 8) {
      ok=1;
      sprintbit(GET_OBJ_VAL(obj, 1), container_bits, buf2);
      sprintf(buf, "\r\n\nLock Type [%s]\r\n", buf2);
      send_to_char(buf, ch);
      print_choices(container_bits, olc_container_lock, ch);
      send_to_char("\r\n0. Previous Menu\r\n", ch);
    }
    else if(field == 9) {
      ok=1;
      sprintf(buf, "\r\n\nKey Number (vnum of key that unlocks this container, -1 = no key) [%d]\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    if(field == 7) {
      sprintf(buf, "\r\n\n"
                   "Maximum number of drinks it can contain [%d] (1 - %d)\r\n"
                   "  (Note: it depends on the liquid, but you use about 4 drinks\r\n"
                   "   when you're completely thirsty)\r\n", GET_OBJ_VAL(obj, 0),
              (GET_OBJ_TYPE(obj)==ITEM_DRINKCON) ? 10000 : 1000000);
      send_to_char(buf, ch);
    }
    else if(field == 8) {
      sprintf(buf, "\r\n\n"
                   "Number of drinks it contains [%d] (0 - %d)\r\n"
                   "  (Note: it depends on the liquid, but you use about 4 drinks\r\n"
                   "   when you're completely thirsty)\r\n", GET_OBJ_VAL(obj, 1),
              (GET_OBJ_TYPE(obj)==ITEM_DRINKCON) ? 10000 : 1000000);
      send_to_char(buf, ch);
    }
    else if(field == 9) {
      sprinttype(GET_OBJ_VAL(obj, 2), drinks, buf2);
      sprintf(buf, "\r\n\nType of liquid [%s]\r\n", buf2);
      send_to_char(buf, ch);
      print_choices(drinks, olc_drinks, ch);
    }
    else {
      sprintf(buf, "\r\n\nIf the liquid is poisoned [%s]\r\n0. No\r\n1. Yes\r\n", YESNO(GET_OBJ_VAL(obj, 3)));
      send_to_char(buf, ch);
    }
    ok=1;
    break;
  case ITEM_FOOD:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nMakes full [%d] (1 - 24)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else if(field == 8) {
      ok=1;
      sprintf(buf, "\r\n\nIf the food is poisoned [%s]\r\n0. No\r\n1. Yes\r\n", YESNO(GET_OBJ_VAL(obj, 3)));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_MONEY:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nAmmount of money in the stash [%d] (1 - 1000000)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_BEAMER:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nVnum of beam down room [%d]\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_DAMAGEABLE:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nHow easily it's damaged (lower is weaker) [%d] (1 - 100)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    if(field == 8) {
      ok=1;
      sprintf(buf, "\r\n\nStarting condition [%d] (0(perfect) - 9(almost dust))\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    break;
  }
  if(!ok) {
    send_to_char("Invalid command.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
  }
}
void change_obj_type(struct obj_data *obj, struct char_data *ch, int field, int cmd)
{
  int ok=0, temp;
  switch(GET_OBJ_TYPE(obj)) {
  case ITEM_LIGHT:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(-1, 10000);
    }
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 127);
    }
    else {
      if(cmd==0)
        temp=0;      else
        temp=get_choice(spells, olc_spells, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, field-7)=temp;
      }
    }
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    if(field == 7) {
      if(cmd==0)
        temp=0;
      else
        temp=get_choice(spells, olc_spells, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 3)=temp;
      }
    }
    else if(field == 8) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 127);
    }
    else if(field == 9) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(0, GET_OBJ_VAL(obj, 1));
    }
    else {
      ok=1;
      GET_OBJ_VAL(obj, 1)=RANGE(1, 100);
    }
    break;
  case ITEM_WEAPON:
    if(field == 7) {
      switch(cmd) {
      case 0:
        ok=1;
        break;
      case 1:
        GET_OLC_FIELD(ch)=105;
        break;
      case 2:
        GET_OLC_FIELD(ch)=106;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
    }
    if(field == 8) {
      if((temp=get_choice(weapon_types, olc_weapons, cmd)) < 0) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 3)=temp;
      }
    }
    break;
  case ITEM_ARMOR:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(-40, 40);
    }
    break;
  case ITEM_CONTAINER:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(0, 1000);
    }
    else if(field == 8) {
      if(cmd) {
        temp=get_choice(container_bits, olc_container_lock, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
        }
        else {
          TOGGLE_BIT(GET_OBJ_VAL(obj, 1), ((long long)1 << temp));
          if(IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) {
            SET_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED);
          }
          if(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) {
            SET_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE);
          }
        }
      }
      else {
        ok=1;
      }
    }
    else if(field == 9) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(-1, 99999);
    }
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, (GET_OBJ_TYPE(obj)==ITEM_DRINKCON) ? 10000 : 1000000);
    }
    else if(field == 8) {
      ok=1;
      GET_OBJ_VAL(obj, 1)=RANGE(1, GET_OBJ_VAL(obj, 0));
    }
    else if(field == 9) {
      if((temp=get_choice(drinks, olc_drinks, cmd)) < 0) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 2)=temp;
      }
    }
    else {
      switch(cmd) {
      case 0:
        ok=1;
        GET_OBJ_VAL(obj, 3)=0;
        break;
      case 1:
        ok=1;
        GET_OBJ_VAL(obj, 3)=1;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
    }
    break;
  case ITEM_FOOD:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 24);
    }
    else if(field == 8) {
      switch(cmd) {
      case 0:
        ok=1;
        GET_OBJ_VAL(obj, 3)=0;
        break;
      case 1:
        ok=1;
        GET_OBJ_VAL(obj, 3)=1;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
    }
    break;
  case ITEM_MONEY:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 1000000);
    }
    break;
  case ITEM_BEAMER:
    if(field == 7) {
      temp=RANGE(0, 99999);
      if(real_room(temp) < 0) {
        send_to_char("No such room.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 0)=temp;
      }
    }
    break;
  case ITEM_DAMAGEABLE:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 100);
    }
    if(field == 8) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(0, 9);
    }
    break;
  }
  if(ok)
    GET_OLC_FIELD(ch)=0;
}
void oedit_menu(struct char_data *ch, int cmd)
{
  int i, temp;
  struct extra_descr_data *ex;
  char line[MAX_STRING_LENGTH], tbuf1[MAX_STRING_LENGTH], *ptr;
  struct obj_data *me=(struct obj_data *)GET_OLC_PTR(ch);
  switch(GET_OLC_FIELD(ch)) {
  case 0:
    if(cmd == OLC_MENU_NUM) {
      if(PRF_FLAGGED(ch, PRF_NOMENU))
        return;
      sprintf(line, "1. OBJ:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
      sprintf(line, "2. Aliases: %s\r\n", me->name);
      send_to_char(line, ch);
      sprintf(line, "3. Short description: %s\r\n", me->short_description);
      send_to_char(line, ch);
      sprintf(line, "4. Long description: %s\r\n", me->description);
      send_to_char(line, ch);
      strcpy(line, "5. Extra descriptions:");
      for(ex=me->ex_description; ex; ex=ex->next)
        sprintf(line, "%s %s", line, ex->keyword);
      strcat(line, "\r\n");
      send_to_char(line, ch);
      print_obj_type_data(me, ch);
      if((GET_OBJ_TYPE(me) == ITEM_NOTE) || (GET_OBJ_TYPE(me) == ITEM_POTION) ||
         (GET_OBJ_TYPE(me) == ITEM_SCROLL) || (GET_OBJ_TYPE(me) == ITEM_STAFF) ||
         (GET_OBJ_TYPE(me) == ITEM_WAND)) {
        sprintf(line, "11. Action description: %s\r\n", me->action_description);
        send_to_char(line, ch);
      }
      sprintbit(GET_OBJ_WEAR(me), wear_bits, tbuf1);
      sprintf(line, "12. Wearable on: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(GET_OBJ_EXTRA(me), extra_bits, tbuf1);
      sprintf(line, "13. Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.bitvector, affected_bits, tbuf1);
      sprintf(line, "14. Sets affects: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.immune, damtypes, tbuf1);
      sprintf(line, "15. Strong resistance: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.weak, damtypes, tbuf1);
      sprintf(line, "16. Vulnerabilities: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.resist, damtypes, tbuf1);
      sprintf(line, "17. Resistance: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintf(line, "18. Weight:%d   19. Value:%d\r\n", GET_OBJ_WEIGHT(me), GET_OBJ_COST(me));
      send_to_char(line, ch);
      if(MAX_OBJ_AFFECT == 6) {
        for(i=0; i < MAX_OBJ_AFFECT; i++) {
          sprinttype(me->affected[i].location, apply_types, tbuf1);
          if(me->affected[i].location)
            sprintf(tbuf1, "%s %+d", tbuf1, me->affected[i].modifier);
          sprintf(line, "%2d. Affect%d: %s\r\n", 20+i, i+1, tbuf1);
          send_to_char(line, ch);
        }
      }
      else {
        send_to_char("ITEM AFFECTATION OLC NEEDS TO BE REWRITTEN\r\n", ch);
      }
      send_to_char("99. Save\r\n", ch);
      send_to_char("0. Exit\r\n", ch);
    }
    else {
      if ((cmd > 25) && (cmd != 99))
        send_to_char("Invalid command.\r\n", ch);
      else
        GET_OLC_FIELD(ch)=cmd;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 1:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nEnter vnum to copy to.\r\nYou must quit the editor or save to complete the copy.\r\nOld Number:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
    }
    else {
      for(i=0; i<=top_of_zone_table; i++) {
        if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
          break;
      }
      if(HAS_OLC(ch, zone_table[i].number) || (GET_LEVEL(ch) >= LVL_CIMP))
        GET_OLC_NUM(ch)=cmd;
      else
        send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 2:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter a series of names seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->name) {
        free(me->name);
        me->name=NULL;
      }
      ch->desc->str = &me->name;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->name; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 3:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the name of the object.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->short_description) {
        free(me->short_description);
        me->short_description=NULL;
      }
      ch->desc->str = &me->short_description;
      ch->desc->max_str = OLC_SHORT_LENGTH;
    }
    else {
      for(ptr=me->short_description; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 4:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the text people will see when they see the object in a room.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->description) {
        free(me->description);
        me->description=NULL;
      }
      ch->desc->str = &me->description;
      ch->desc->max_str = OLC_LONG_LENGTH;
    }
    else {
      for(ptr=me->description; *ptr; ptr++);
      for(; ((!*ptr) || (*ptr == '\r') || (*ptr == '\n')); ptr--);
      *(ptr+1) = 0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 5:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nExtra Descriptions:\r\n", ch);
      for(i=1, ex=me->ex_description; ex; i++, ex=ex->next) {
        sprintf(line, "\r\n%2d. %s\r\n%s", i, ex->keyword, ex->description);
        send_to_char(line, ch);
      }
      send_to_char("\r\n1. Add extra description\r\n2. Delete extra description\r\n3. Change extra description\r\n0. Previous Menu\r\n", ch);
    }
    else {
      switch(cmd) {
      case 0:
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=100;
        break;
      case 2:
        GET_OLC_FIELD(ch)=101;
        break;
      case 3:
        GET_OLC_FIELD(ch)=102;
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 6:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(GET_OBJ_TYPE(me), item_types, tbuf1);
      sprintf(line, "\r\n\nItem Type [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(item_types, olc_item_types, ch);
    }
    else {
      temp=get_choice(item_types, olc_item_types, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      GET_OBJ_TYPE(me)=temp;
      if(temp==ITEM_DAMAGEABLE)
        GET_OBJ_VAL(me, 0)=1;
      else
        GET_OBJ_VAL(me, 0)=0;
      GET_OBJ_VAL(me, 1)=0;
      GET_OBJ_VAL(me, 2)=0;
      GET_OBJ_VAL(me, 3)=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 7:
  case 8:
  case 9:
  case 10:
    if(cmd == OLC_MENU_NUM) {
      print_obj_type(me, ch, GET_OLC_FIELD(ch));
    }
    else {
      change_obj_type(me, ch, GET_OLC_FIELD(ch), cmd);
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 11:
    if((GET_OBJ_TYPE(me) == ITEM_NOTE) || (GET_OBJ_TYPE(me) == ITEM_POTION) ||
       (GET_OBJ_TYPE(me) == ITEM_SCROLL) || (GET_OBJ_TYPE(me) == ITEM_STAFF) ||
       (GET_OBJ_TYPE(me) == ITEM_WAND)) {
      if(cmd == OLC_MENU_NUM) {
        if(GET_OBJ_TYPE(me) == ITEM_NOTE)
          send_to_char("\r\n\nEnter the text people will see when they read the note.\r\nEnter @ on a blank line when done.\r\n", ch);
        else
          send_to_char("\r\n\n"
                       "Enter the text people will see when a person uses this item.\r\n"
                       "You can use the following $ codes:\r\n"
                       "$n - user\r\n"
                       "$N - victim\r\n"
                       "$m - him/her (user)\r\n"
                       "$M - him/her (victim)\r\n"
                       "$s - his/her (user)\r\n"
                       "$S - his/her (victim)\r\n"
                       "$e - he/she (user)\r\n"
                       "$E - he/she (victim)\r\n"
                       "$c - blank to user, s to others (for verb tense)\r\n"
                       "$k - blank to user, es to others (for verb tense)\r\n"
                       "$C - blank to victim, s to others (for verb tense)\r\n"
                       "$K - blank to victim, es to others (for verb tense)\r\n"
                       "$i - are to user (you are), is to others (Sarnoth is)\r\n"
                       "$I - are to victim (you are), is to others (fido is)\r\n"
                       "$o - first alias of item\r\n"
                       "$O - first alias of target object\r\n"
                       "$p - item short description\r\n"
                       "$P - short description of target object\r\n"
                       "$a - a/an (item)\r\n"
                       "$A - a/an (target object)\r\n"
                       "$$ - $\r\n"
                       "Enter @ on a blank line when done.\r\n", ch);
        if(me->action_description) {
          free(me->action_description);
          me->action_description=NULL;
        }
        ch->desc->str = &me->action_description;
        ch->desc->max_str = OLC_LONG_LENGTH;
      }
      else {
        for(ptr=me->action_description; (*ptr); ptr++);
        for(; ((*ptr == '\n') || (*ptr == '\r') || (!*ptr)); ptr--);
        *(ptr+1)=0;
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    else {
      send_to_char("Invalid command.\r\n", ch);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 12:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(GET_OBJ_WEAR(me), wear_bits, tbuf1);
      sprintf(line, "\r\n\nCan be worn on [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(wear_bits, olc_wear, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(wear_bits, olc_wear, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(GET_OBJ_WEAR(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 13:    if(cmd == OLC_MENU_NUM) {
      sprintbit(GET_OBJ_EXTRA(me), extra_bits, tbuf1);
      sprintf(line, "\r\n\nFlags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(extra_bits, olc_extra, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(extra_bits, olc_extra, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(GET_OBJ_EXTRA(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 14:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.bitvector, affected_bits, tbuf1);
      sprintf(line, "\r\n\nSets affects [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(affected_bits, olc_objaff_bits, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(affected_bits, olc_objaff_bits, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.bitvector, ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 15:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.immune, damtypes, tbuf1);
      sprintf(line, "\r\n\nProvides exceptional resistance to:\r\n[%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.immune, ((long long)1 << temp));
        if(IS_SET(me->obj_flags.immune, ((long long)1 << temp))) {
          REMOVE_BIT(me->obj_flags.weak, ((long long)1 << temp));
          REMOVE_BIT(me->obj_flags.resist, ((long long)1 << temp));
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 16:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.weak, damtypes, tbuf1);
      sprintf(line, "\r\n\nMakes weak against:\r\n[%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.weak, ((long long)1 << temp));
        if(IS_SET(me->obj_flags.weak, ((long long)1 << temp))) {
          REMOVE_BIT(me->obj_flags.immune, ((long long)1 << temp));
          REMOVE_BIT(me->obj_flags.resist, ((long long)1 << temp));
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 17:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.resist, damtypes, tbuf1);
      sprintf(line, "\r\n\nProvides some resistance to:\r\n[%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.resist, ((long long)1 << temp));
        if(IS_SET(me->obj_flags.resist, ((long long)1 << temp))) {
          REMOVE_BIT(me->obj_flags.weak, ((long long)1 << temp));
          REMOVE_BIT(me->obj_flags.immune, ((long long)1 << temp));
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 18:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nWeight [%d] (1 - 10000)\r\n", GET_OBJ_WEIGHT(me));
      send_to_char(line, ch);
    }
    else {
      GET_OBJ_WEIGHT(me)=RANGE(1, 10000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 19:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nValue [%d] (0 - 1000000)\r\n", GET_OBJ_COST(me));
      send_to_char(line, ch);
    }
    else {
      GET_OBJ_RENT(me)=GET_OBJ_COST(me)=RANGE(1, 1000000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
    i=GET_OLC_FIELD(ch)-20;
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->affected[i].location, apply_types, tbuf1);
      sprintf(line, "\r\n\nAffects [%s] by %+d\r\n", tbuf1, me->affected[i].modifier);
      send_to_char(line, ch);
      print_choices(apply_types, olc_apply, ch);
    }
    else {
      temp=get_choice(apply_types, olc_apply, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      me->affected[i].location=temp;
      GET_OLC_FIELD(ch) += 87;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 99:
    send_to_char("Saving...\r\n", ch);
    save_edit(ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  case 100:    if(cmd == OLC_MENU_NUM) {
      CREATE(ex, struct extra_descr_data, 1);
      ex->next=me->ex_description;
      me->ex_description=ex;
      ex->keyword=NULL;
      ex->description=str_dup("");
      send_to_char("\r\n\nEnter a series of names seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &ex->keyword;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->ex_description->keyword; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=113;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 101:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nExtra Descriptions:\r\n", ch);
      for(i=1, ex=me->ex_description; ex; i++, ex=ex->next) {
        sprintf(line, "\r\n%2d. %s\r\n%s", i, ex->keyword, ex->description);
        send_to_char(line, ch);
      }
      send_to_char("\r\n0. Previous Menu\r\nDelete which extra description?\r\n", ch);
    }
    else {
      if(cmd > 0) {
        for(i=1, ex=me->ex_description; ex; i++, ex=ex->next)
          if(i==cmd)
            break;
        if(ex) {
          struct extra_descr_data *temp;
          REMOVE_FROM_LIST(ex, me->ex_description, next);
          free(ex->keyword);
          free(ex->description);
          free(ex);
          GET_OLC_FIELD(ch)=5;
        }
        else {
          send_to_char("Invalid choice.\r\n", ch);
        }
      }
      else if(cmd == 0) {
        GET_OLC_FIELD(ch)=5;
      }
      else {
        send_to_char("Invalid choice.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 102:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nExtra Descriptions:\r\n", ch);
      for(i=1, ex=me->ex_description; ex; i++, ex=ex->next) {
        sprintf(line, "\r\n%2d. %s\r\n%s", i, ex->keyword, ex->description);
        send_to_char(line, ch);
      }
      send_to_char("\r\n0. Previous Menu\r\nChange which extra description?\r\n", ch);
    }
    else {
      if(cmd > 0) {
        for(i=1, ex=me->ex_description; ex; i++, ex=ex->next)
          if(i==cmd)
            break;
        if(ex) {
          struct extra_descr_data *temp;
          REMOVE_FROM_LIST(ex, me->ex_description, next);
          ex->next=me->ex_description;
          me->ex_description=ex;
          GET_OLC_FIELD(ch)=114;
        }
        else {
          send_to_char("Invalid choice.\r\n", ch);
        }
      }
      else if(cmd == 0) {
        GET_OLC_FIELD(ch)=5;
      }
      else {
        send_to_char("Invalid choice.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 105:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of damage dice [%d] (1 - 30)\r\n", GET_OBJ_VAL(me, 1));
      send_to_char(line, ch);
    }
    else {
      GET_OBJ_VAL(me, 1)=RANGE(1, 30);
      GET_OLC_FIELD(ch)=7;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 106:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSize of damage dice [%d] (1 - 60)\r\n", GET_OBJ_VAL(me, 2));
      send_to_char(line, ch);
    }
    else {
      GET_OBJ_VAL(me, 2)=RANGE(1, 60);
      GET_OLC_FIELD(ch)=7;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 107:
  case 108:
  case 109:
  case 110:
  case 111:
  case 112:
    i=GET_OLC_FIELD(ch)-107;
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->affected[i].location, apply_types, tbuf1);
      sprintf(line, "\r\n\nAffects %s by [%+d] (-50 - 50)\r\n", tbuf1, me->affected[i].modifier);
      send_to_char(line, ch);
    }
    else {
      me->affected[i].modifier=RANGE(-50, 50);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 113:
    if(cmd == OLC_MENU_NUM) {
      free(me->ex_description->description);
      me->ex_description->description=NULL;
      send_to_char("\r\n\nEnter the description.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &me->ex_description->description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=5;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 114:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nChange extra description:\r\n", ch);
      sprintf(line, "1. %s\r\n\n2.\r\n%s\r\n0. Previous Menu\r\n", me->ex_description->keyword, me->ex_description->description);
      send_to_char(line, ch);
    }
    else {
      switch(cmd) {
      case 0:
        GET_OLC_FIELD(ch)=102;
        break;
      case 1:
        GET_OLC_FIELD(ch)=115;
        break;
      case 2:
        GET_OLC_FIELD(ch)=116;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 115:
    if(cmd == OLC_MENU_NUM) {
      if(me->ex_description->keyword)
        free(me->ex_description->keyword);
      me->ex_description->keyword=NULL;
      send_to_char("\r\n\nEnter a series of names seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &me->ex_description->keyword;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->ex_description->keyword; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=114;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 116:
    if(cmd == OLC_MENU_NUM) {
      if(me->ex_description->description)
        free(me->ex_description->description);
      me->ex_description->description=NULL;
      send_to_char("\r\n\nEnter the description.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &me->ex_description->description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=114;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  default:
    send_to_char("Incomplete field.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  }
}
void redit_menu(struct char_data *ch, int cmd)
{
  int i, temp;
  struct extra_descr_data *ex;
  char line[MAX_STRING_LENGTH], tbuf1[MAX_STRING_LENGTH], *ptr;
  char tbuf2[MAX_INPUT_LENGTH];
  struct room_data *me=(struct room_data *)GET_OLC_PTR(ch);
  switch(GET_OLC_FIELD(ch)) {
  case 0:
    if(cmd == OLC_MENU_NUM) {
      if(PRF_FLAGGED(ch, PRF_NOMENU))
        return;
      sprintf(line, "1. ROOM:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
      sprintf(line, "2. Name: %s\r\n", me->name);
      send_to_char(line, ch);
      sprintf(line, "3. Description:\r\n%s", (me->description && *me->description) ? me->description : "");
      send_to_char(line, ch);
      strcpy(line, "4. Extra descriptions:");
      for(ex=me->ex_description; ex; ex=ex->next)
        sprintf(line, "%s %s", line, ex->keyword);
      strcat(line, "\r\n");
      send_to_char(line, ch);
      sprinttype(me->sector_type, sector_types, tbuf1);
      sprintf(line, "5. Sector Type: %s\r\n", tbuf1);
      send_to_char(line, ch);
      if(IS_SET(me->room_flags, ROOM_DEATH))
        sprintf(tbuf1, "%dd%d%+d or %d%%%s", me->dt_numdice, me->dt_sizedice,
                me->dt_add, me->dt_percent, me->dt_repeat ? ", repeats" : "");
      else
        strcpy(tbuf1, "NONE");
      sprintf(line, "6. Death Trap: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->room_flags, room_bits, tbuf1);
      sprintf(line, "7. Room Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      if(NUM_OF_DIRS == 10) {
        for(i=0; i<NUM_OF_DIRS; i++) {
          if(me->dir_option[i]) {
            if(IS_SET(me->dir_option[i]->exit_info, EX_ISDOOR)) {
              if(me->dir_option[i]->key < 0)
                strcpy(tbuf2, ", no key");
              else
                sprintf(tbuf2, ", key:%d", me->dir_option[i]->key);
            }
            else
              tbuf2[0]=0;
            if(real_room(GET_OLC_NUM(ch)) >= 0)
              sprintf(tbuf1, "to %d, %s%sdoor%s  (%s)", world[me->dir_option[i]->to_room].number,
                      (IS_SET(me->dir_option[i]->exit_info, EX_SECRET)?(IS_SET(me->dir_option[i]->exit_info, EX_HIDDEN)?"invis-secret ":"secret "):""),
                      (IS_SET(me->dir_option[i]->exit_info, EX_ISDOOR) ?(IS_SET(me->dir_option[i]->exit_info, EX_PICKPROOF)?"pickproof ":""):"no "),
                      tbuf2, world[me->dir_option[i]->to_room].name);
            else
              sprintf(tbuf1, "to %d, %s%sdoor%s  (%s)", me->dir_option[i]->to_room,
                      (IS_SET(me->dir_option[i]->exit_info, EX_SECRET)?(IS_SET(me->dir_option[i]->exit_info, EX_HIDDEN)?"invis-secret ":"secret "):""),
                      (IS_SET(me->dir_option[i]->exit_info, EX_ISDOOR) ?(IS_SET(me->dir_option[i]->exit_info, EX_PICKPROOF)?"pickproof ":""):"no "),
                      tbuf2, ((real_room(me->dir_option[i]->to_room) >= 0) ? (world[real_room(me->dir_option[i]->to_room)].name) : "NOWHERE"));
          }
          else {
            strcpy(tbuf1, "NONE");
          }
          sprintf(line, "%2d. %s exit: %s\r\n", 8+i, dirs[i], tbuf1);
          send_to_char(line, ch);
        }
      }
      else {
        send_to_char("ROOM EXIT OLC NEEDS TO BE REWRITTEN\r\n", ch);
      }
      send_to_char("99. Save\r\n", ch);
      send_to_char("0. Exit\r\n", ch);
    }
    else {
      if ((cmd > 17) && (cmd != 99))
        send_to_char("Invalid command.\r\n", ch);
      else
        GET_OLC_FIELD(ch)=cmd;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 1:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nEnter vnum to copy to.\r\nYou must quit the editor or save to complete the copy.\r\nOld Number:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
    }
    else {
      for(i=0; i<=top_of_zone_table; i++) {
        if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
          break;
      }
      if(HAS_OLC(ch, zone_table[i].number) || (GET_LEVEL(ch) >= LVL_CIMP)) {
        if((real_room(GET_OLC_NUM(ch)) < 0) && (real_room(cmd) >= 0)) {
          for(i=0; i<NUM_OF_DIRS; i++) {
            if(me->dir_option[i]) {
              if(me->dir_option[i]->to_room == GET_OLC_NUM(ch))
                me->dir_option[i]->to_room = real_room(cmd);
              else
                me->dir_option[i]->to_room = real_room(me->dir_option[i]->to_room);
            }
          }
        }
        if((real_room(GET_OLC_NUM(ch)) >= 0) && (real_room(cmd) < 0)) {
          for(i=0; i<NUM_OF_DIRS; i++) {
            if(me->dir_option[i]) {
              if(me->dir_option[i]->to_room == real_room(GET_OLC_NUM(ch)))
                me->dir_option[i]->to_room = cmd;
              else
                me->dir_option[i]->to_room = world[me->dir_option[i]->to_room].number;
            }
          }
        }
        GET_OLC_NUM(ch)=cmd;
      }
      else
        send_to_char("You don't have OLC privileges to that zone.\r\n", ch);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 2:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the name of the room.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->name) {
        free(me->name);
        me->name=NULL;
      }
      ch->desc->str = &me->name;
      ch->desc->max_str = OLC_SHORT_LENGTH;
    }
    else {
      for(ptr=me->name; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 3:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the room description.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->description) {
        free(me->description);
        me->description=NULL;
      }
      ch->desc->str = &me->description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 4:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nExtra Descriptions:\r\n", ch);
      for(i=1, ex=me->ex_description; ex; i++, ex=ex->next) {
        sprintf(line, "\r\n%2d. %s\r\n%s", i, ex->keyword, ex->description);
        send_to_char(line, ch);
      }
      send_to_char("\r\n1. Add extra description\r\n2. Delete extra description\r\n3. Change extra description\r\n0. Previous Menu\r\n", ch);
    }
    else {
      switch(cmd) {
      case 0:
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=100;
        break;
      case 2:
        GET_OLC_FIELD(ch)=101;
        break;
      case 3:
        GET_OLC_FIELD(ch)=102;
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 5:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->sector_type, sector_types, tbuf1);
      sprintf(line, "\r\n\nSector Type [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_sector_choices(sector_types, olc_sector, ch);
    }
    else {
      temp=get_choice(sector_types, olc_sector, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      me->sector_type=temp;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 6:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nDeath Trap info:\r\n", ch);
      sprintf(line, "1. Number of damage dice [%d] (0-100)\r\n", me->dt_numdice);
      send_to_char(line, ch);
      sprintf(line, "2. Size of damage dice [%d] (0-1000)\r\n", me->dt_sizedice);
      send_to_char(line, ch);
      sprintf(line, "3. Additional damage [%d] (0-30000)\r\n", me->dt_add);
      send_to_char(line, ch);
      sprintf(line, "4. Minimum percentage of maxhit the DT will do [%d] (0-99)\r\n", me->dt_percent);
      send_to_char(line, ch);
      sprintf(line, "\r\nDoes the death trap keep damaging you as you stand in it? [%s]\r\n", (me->dt_repeat ? "YES" : "NO"));
      send_to_char(line, ch);
      send_to_char("5. No\r\n", ch);
      send_to_char("6. Yes\r\n", ch);
      send_to_char("\r\n0. Previous Menu\r\n", ch);
      send_to_char("7. No death trap\r\n", ch);
    }
    else {
      switch(cmd) {
      case 0:
        SET_BIT(me->room_flags, ROOM_DEATH);
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=103;
        break;
      case 2:
        GET_OLC_FIELD(ch)=104;
        break;
      case 3:
        GET_OLC_FIELD(ch)=105;
        break;
      case 4:
        GET_OLC_FIELD(ch)=106;
        break;
      case 5:
        me->dt_repeat=0;
        break;
      case 6:
        me->dt_repeat=1;
        break;
      case 7:
        REMOVE_BIT(me->room_flags, ROOM_DEATH);
        me->dt_numdice = me->dt_sizedice = me->dt_add = me->dt_percent = me->dt_repeat = 0;
        GET_OLC_FIELD(ch)=0;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 7:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->room_flags, room_bits, tbuf1);
      sprintf(line, "\r\n\nRoom Flags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(room_bits, olc_room, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(room_bits, olc_room, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->room_flags, ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
    i=GET_OLC_FIELD(ch)-8;
    if(cmd == OLC_MENU_NUM) {
      if(!me->dir_option[i]) {
        CREATE(me->dir_option[i], struct room_direction_data, 1);
        me->dir_option[i]->general_description=str_dup("");
        me->dir_option[i]->keyword=str_dup("");
        me->dir_option[i]->exit_info=0;
        me->dir_option[i]->key=-1;
        me->dir_option[i]->to_room = ((real_room(GET_OLC_NUM(ch)) >= 0) ? 0 : world[0].number);
      }
      sprintf(tbuf1, "%s exit data:\r\n", dirs[GET_OLC_FIELD(ch)-8]);
      sprintf(line, "\r\n\n%s", CAP(tbuf1));
      send_to_char(line, ch);
      if(real_room(GET_OLC_NUM(ch)) >= 0)
        sprintf(tbuf1, "[%d] (%s)", world[me->dir_option[i]->to_room].number,
                world[me->dir_option[i]->to_room].name);
      else
        sprintf(tbuf1, "[%d] (%s)", me->dir_option[i]->to_room,
                ((real_room(me->dir_option[i]->to_room) >= 0) ? (world[real_room(me->dir_option[i]->to_room)].name) : "NOWHERE"));
      sprintf(line, "1. Leads to room %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintf(line, "2. Door type: [%s%s]\r\n", (IS_SET(me->dir_option[i]->exit_info, EX_SECRET)?(IS_SET(me->dir_option[i]->exit_info, EX_HIDDEN)?"hidden-secret ":"secret "):""), (IS_SET(me->dir_option[i]->exit_info, EX_ISDOOR) ? (IS_SET(me->dir_option[i]->exit_info, EX_PICKPROOF) ? "pickproof" : "normal") : "no door"));
      send_to_char(line, ch);
      if(me->dir_option[i]->key < 0)
        strcpy(tbuf2, "no key");
      else
        sprintf(tbuf2, "%d", me->dir_option[i]->key);
      sprintf(line, "3. Key [%s]\r\n", tbuf2);
      send_to_char(line, ch);
      sprintf(line, "4. Keywords: [%s]\r\n", me->dir_option[i]->keyword);
      send_to_char(line, ch);
      sprintf(line, "5. Description:\r\n%s", me->dir_option[i]->general_description);
      send_to_char(line, ch);
      send_to_char("\r\n0. Previous Menu\r\n6. Delete exit\r\n", ch);
    }
    else {
      switch(cmd) {
      case 0:
        GET_OLC_FIELD(ch)=0;
        break;
      case 1:
        GET_OLC_FIELD(ch)=117+i;
        break;
      case 2:
        GET_OLC_FIELD(ch)=117+i+NUM_OF_DIRS;
        break;
      case 3:
        GET_OLC_FIELD(ch)=117+i+(2*NUM_OF_DIRS);
        break;
      case 4:
        GET_OLC_FIELD(ch)=117+i+(3*NUM_OF_DIRS);
        break;
      case 5:
        GET_OLC_FIELD(ch)=117+i+(4*NUM_OF_DIRS);
        break;
      case 6:
        if(me->dir_option[i]->general_description)
          free(me->dir_option[i]->general_description);
        if(me->dir_option[i]->keyword)
          free(me->dir_option[i]->keyword);
        free(me->dir_option[i]);
        me->dir_option[i]=NULL;
        GET_OLC_FIELD(ch)=0;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 99:
    send_to_char("Saving...\r\n", ch);
    save_edit(ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  case 100:
    if(cmd == OLC_MENU_NUM) {
      CREATE(ex, struct extra_descr_data, 1);
      ex->next=me->ex_description;
      me->ex_description=ex;
      ex->keyword=NULL;
      ex->description=str_dup("");
      send_to_char("\r\n\nEnter a series of names seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &ex->keyword;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->ex_description->keyword; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=113;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 101:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nExtra Descriptions:\r\n", ch);
      for(i=1, ex=me->ex_description; ex; i++, ex=ex->next) {
        sprintf(line, "\r\n%2d. %s\r\n%s", i, ex->keyword, ex->description);
        send_to_char(line, ch);
      }
      send_to_char("\r\n0. Previous Menu\r\nDelete which extra description?\r\n", ch);
    }
    else {
      if(cmd > 0) {
        for(i=1, ex=me->ex_description; ex; i++, ex=ex->next)
          if(i==cmd)
            break;
        if(ex) {
          struct extra_descr_data *temp;
          REMOVE_FROM_LIST(ex, me->ex_description, next);
          free(ex->keyword);
          free(ex->description);
          free(ex);
          GET_OLC_FIELD(ch)=4;
        }
        else {
          send_to_char("Invalid choice.\r\n", ch);
        }
      }
      else if(cmd == 0) {
        GET_OLC_FIELD(ch)=4;
      }
      else {
        send_to_char("Invalid choice.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 102:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nExtra Descriptions:\r\n", ch);
      for(i=1, ex=me->ex_description; ex; i++, ex=ex->next) {
        sprintf(line, "\r\n%2d. %s\r\n%s", i, ex->keyword, ex->description);
        send_to_char(line, ch);
      }
      send_to_char("\r\n0. Previous Menu\r\nChange which extra description?\r\n", ch);
    }
    else {
      if(cmd > 0) {
        for(i=1, ex=me->ex_description; ex; i++, ex=ex->next)
          if(i==cmd)
            break;
        if(ex) {
          struct extra_descr_data *temp;
          REMOVE_FROM_LIST(ex, me->ex_description, next);
          ex->next=me->ex_description;
          me->ex_description=ex;
          GET_OLC_FIELD(ch)=114;
        }
        else {
          send_to_char("Invalid choice.\r\n", ch);
        }
      }
      else if(cmd == 0) {
        GET_OLC_FIELD(ch)=4;
      }
      else {
        send_to_char("Invalid choice.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 103:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of damage dice [%d] (0-100)\r\n", me->dt_numdice);
      send_to_char(line, ch);
    }
    else {
      me->dt_numdice = RANGE(0, 100);
      GET_OLC_FIELD(ch)=6;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 104:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nSize of damage dice [%d] (0-1000)\r\n", me->dt_sizedice);
      send_to_char(line, ch);
    }
    else {
      me->dt_sizedice = RANGE(0, 1000);
      GET_OLC_FIELD(ch)=6;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 105:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nAdditional damage [%d] (0-30000)\r\n", me->dt_add);
      send_to_char(line, ch);
    }
    else {
      me->dt_add = RANGE(0, 30000);
      GET_OLC_FIELD(ch)=6;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 106:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMinimum percentage of maxhit it will damage [%d] (0-99)\r\n", me->dt_percent);
      send_to_char(line, ch);
    }
    else {
      me->dt_percent = RANGE(0, 99);
      GET_OLC_FIELD(ch)=6;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 113:
    if(cmd == OLC_MENU_NUM) {
      free(me->ex_description->description);
      me->ex_description->description=NULL;
      send_to_char("\r\n\nEnter the description.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &me->ex_description->description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=4;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 114:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nChange extra description:\r\n", ch);
      sprintf(line, "1. %s\r\n\n2.\r\n%s\r\n0. Previous Menu\r\n", me->ex_description->keyword, me->ex_description->description);
      send_to_char(line, ch);
    }
    else {
      switch(cmd) {
      case 0:
        GET_OLC_FIELD(ch)=102;
        break;
      case 1:
        GET_OLC_FIELD(ch)=115;
        break;
      case 2:
        GET_OLC_FIELD(ch)=116;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 115:
    if(cmd == OLC_MENU_NUM) {
      if(me->ex_description->keyword)
        free(me->ex_description->keyword);
      me->ex_description->keyword=NULL;
      send_to_char("\r\n\nEnter a series of names seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &me->ex_description->keyword;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->ex_description->keyword; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=114;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 116:
    if(cmd == OLC_MENU_NUM) {
      if(me->ex_description->description)
        free(me->ex_description->description);
      me->ex_description->description=NULL;
      send_to_char("\r\n\nEnter the description.\r\nEnter @ on a blank line when done.\r\n", ch);
      ch->desc->str = &me->ex_description->description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=114;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 117:
  case 118:
  case 119:
  case 120:
  case 121:
  case 122:
  case 123:
  case 124:
  case 125:
  case 126:
    i=GET_OLC_FIELD(ch)-117;
    if(cmd == OLC_MENU_NUM) {
      if(real_room(GET_OLC_NUM(ch)) >= 0)
        sprintf(tbuf1, "[%d]", world[me->dir_option[i]->to_room].number);
      else
        sprintf(tbuf1, "[%d]", me->dir_option[i]->to_room);
      sprintf(line, "\r\n\nVnum of the room this exit leads to %s\r\n", tbuf1);
      send_to_char(line, ch);
    }
    else {
      temp=RANGE(0, 99999);
      if((temp == GET_OLC_NUM(ch)) || (real_room(temp) >= 0)) {
        if(real_room(GET_OLC_NUM(ch)) >= 0)
          me->dir_option[i]->to_room=real_room(temp);
        else
          me->dir_option[i]->to_room=temp;
        GET_OLC_FIELD(ch) -= 109;
      }
      else {
        send_to_char("No such room.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 127:
  case 128:
  case 129:
  case 130:
  case 131:
  case 132:
  case 133:
  case 134:
  case 135:
  case 136:
    i=GET_OLC_FIELD(ch)-127;
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nDoor type: [%s%s]\r\n", (IS_SET(me->dir_option[i]->exit_info, EX_SECRET)?(IS_SET(me->dir_option[i]->exit_info, EX_HIDDEN)?"hidden-secret ":"secret "):""), (IS_SET(me->dir_option[i]->exit_info, EX_ISDOOR) ? (IS_SET(me->dir_option[i]->exit_info, EX_PICKPROOF) ? "pickproof" : "normal") : "no door"));
      send_to_char(line, ch);
      send_to_char("1. no door\r\n"
                   "2. normal door\r\n"
                   "3. pickproof door\r\n"
                   "4. secret normal door\r\n"
                   "5. secret pickproof door\r\n"
                   "6. hidden-secret normal door\r\n"
                   "7. hidden-secret pickproof door\r\n", ch);
    }
    else {
      switch(cmd) {
      case 1:
        me->dir_option[i]->exit_info = 0;
        GET_OLC_FIELD(ch) -= 119;
        break;
      case 2:
        me->dir_option[i]->exit_info = EX_ISDOOR;
        GET_OLC_FIELD(ch) -= 119;
        break;
      case 3:
        me->dir_option[i]->exit_info = (EX_ISDOOR | EX_PICKPROOF);
        GET_OLC_FIELD(ch) -= 119;
        break;
      case 4:
        me->dir_option[i]->exit_info = (EX_ISDOOR | EX_SECRET);
        GET_OLC_FIELD(ch) -= 119;
        break;
      case 5:
        me->dir_option[i]->exit_info = (EX_ISDOOR | EX_PICKPROOF | EX_SECRET);
        GET_OLC_FIELD(ch) -= 119;
        break;
      case 6:
        me->dir_option[i]->exit_info = (EX_ISDOOR | EX_SECRET | EX_HIDDEN);
        GET_OLC_FIELD(ch) -= 119;
        break;
      case 7:
        me->dir_option[i]->exit_info = (EX_ISDOOR | EX_PICKPROOF | EX_SECRET | EX_HIDDEN);
        GET_OLC_FIELD(ch) -= 119;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 137:
  case 138:
  case 139:
  case 140:
  case 141:
  case 142:
  case 143:
  case 144:
  case 145:
  case 146:
    i=GET_OLC_FIELD(ch)-137;
    if(cmd == OLC_MENU_NUM) {
      if(me->dir_option[i]->key < 0)
        strcpy(tbuf2, "no key");
      else
        sprintf(tbuf2, "%d", me->dir_option[i]->key);
      sprintf(line, "\r\n\nVnum of the key that unlocks this door (-1 is no key) [%s]\r\n", tbuf2);
      send_to_char(line, ch);
    }
    else {
      me->dir_option[i]->key = RANGE(-1, 99999);
      GET_OLC_FIELD(ch) -= 129;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 147:
  case 148:
  case 149:
  case 150:
  case 151:
  case 152:
  case 153:
  case 154:
  case 155:
  case 156:
    i=GET_OLC_FIELD(ch)-147;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter any keywords for this exit (aliases for doors, terain, etc) seperated by spaces.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->dir_option[i]->keyword) {
        free(me->dir_option[i]->keyword);
        me->dir_option[i]->keyword=NULL;
      }
      ch->desc->str = &me->dir_option[i]->keyword;
      ch->desc->max_str = OLC_ALIAS_LENGTH;
    }
    else {
      for(ptr=me->dir_option[i]->keyword; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch) -= 139;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 157:
  case 158:
  case 159:
  case 160:
  case 161:
  case 162:
  case 163:
  case 164:
  case 165:
  case 166:
    i=GET_OLC_FIELD(ch)-157;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the text people will see when they look at this exit.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->dir_option[i]->general_description) {
        free(me->dir_option[i]->general_description);
        me->dir_option[i]->general_description=NULL;
      }
      ch->desc->str = &me->dir_option[i]->general_description;
      ch->desc->max_str = OLC_DESCRIPTION_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch) -= 149;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  default:
    send_to_char("Incomplete field.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  }
}
void iedit_print_obj_type_data(struct obj_data *j, struct char_data *ch)
{
  char line[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
  sprinttype(GET_OBJ_TYPE(j), item_types, buf);
  sprintf(line, "Type: %s", buf);
  strcpy(buf, "\r\n");
  switch(GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "   5. Hours left: Infinite\r\n");
    else
      sprintf(buf, "   5. Hours left:%d\r\n", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "   5. Spell Level:%d   6. Spell1: %s\r\n7. Spell2: %s   8. Spell3: %s\r\n",
            GET_OBJ_VAL(j, 0), skill_name(GET_OBJ_VAL(j, 1)),
            skill_name(GET_OBJ_VAL(j, 2)), skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "   5. Spell: %s\r\n6. Level:%d   7. Charges:%d   8. Max charges:%d\r\n",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    sprinttype(GET_OBJ_VAL(j, 3), weapon_types, buf2);
    sprintf(buf, "   5. Damage Dice:%dd%d   6. Weapon Type: %s\r\n",
	    GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), buf2);
    break;
  case ITEM_ARMOR:
    sprintf(buf, "   5. AC-apply:%d\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "   5. Weight Capacity:%d\r\n6. Lock Type: %s   7. Key Num:%d\r\n",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "   5. Capacity:%d   6. Contains:%d\r\n7. Liquid: %s   8. Poisoned: %s\r\n",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), buf2, YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_FOOD:
    sprintf(buf, "   5. Makes full:%d   6. Poisoned: %s\r\n", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "   5. Coins:%d\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_BEAMER:
    sprintf(buf, "   5. Beam down room:%d (%s)\r\n", GET_OBJ_VAL(j, 0), (real_room(GET_OBJ_VAL(j, 0)) >= 0) ? world[real_room(GET_OBJ_VAL(j, 0))].name : "NOWHERE");
    break;
  case ITEM_DAMAGEABLE:
    sprintf(buf, "   5. Durability:%d   6. Condition:%d\r\n", GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 2));
    break;
  }
  strcat(line, buf);
  send_to_char(line, ch);
}
void iedit_print_obj_type(struct obj_data *obj, struct char_data *ch, int field)
{
  int ok=0;
  char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
  field+=2;
  switch(GET_OBJ_TYPE(obj)) {
  case ITEM_LIGHT:
    if(field == 7) {
      ok=1;
      if (GET_OBJ_VAL(obj, 2) == -1)
        strcpy(buf, "\r\n\nHours of light [Infinite] (-1(infinite) - 10000)\r\n");
      else
        sprintf(buf, "\r\n\nHours of light [%d] (-1(infinite) - 10000)\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    if(field == 7) {
      sprintf(buf, "\r\n\nLevel the spells are cast at [%d] (1 - 127)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else {
      sprintf(buf, "\r\n\nSpell [%s]\r\n", skill_name(GET_OBJ_VAL(obj, field-7)));
      print_spells(ch, buf, "0. NONE\r\n");
    }
    ok=1;
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    if(field == 7) {
      sprintf(buf, "\r\n\nSpell [%s]\r\n", skill_name(GET_OBJ_VAL(obj, 3)));
      print_spells(ch, buf, "0. NONE\r\n");
    }
    else if(field == 8) {
      sprintf(buf, "\r\n\nLevel the spell is cast at [%d] (1 - 127)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else if(field == 9) {
      sprintf(buf, "\r\n\nCharges in the item [%d] (0 - 100)\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    else {
      sprintf(buf, "\r\n\nMaximum charges the item can hold [%d] (1 - 100)\r\n", GET_OBJ_VAL(obj, 1));
      send_to_char(buf, ch);
    }
    ok=1;
    break;
  case ITEM_WEAPON:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\n"
                   "1. Number of damage dice [%d] (1 - 30)\r\n"
                   "2. Size of damage dice [%d] (1 - 60)\r\n"
                   "0. Previous Menu\r\n", GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    if(field == 8) {
      ok=1;
      sprinttype(GET_OBJ_VAL(obj, 3), weapon_types, buf2);
      sprintf(buf, "\r\n\nWeapon Type [%s] (note: thief skills work only with type STAB)\r\n", buf2);
      send_to_char(buf, ch);
      print_choices(weapon_types, olc_weapons, ch);
    }
    break;
  case ITEM_ARMOR:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nAC-apply (higher is better) [%d] (-40 - 40)\r\n", GET_OBJ_VAL(obj, 0));
    }
    break;
  case ITEM_CONTAINER:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nWeight Capacity [%d] (0 - 1000)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else if(field == 8) {
      ok=1;
      sprintbit(GET_OBJ_VAL(obj, 1), container_bits, buf2);
      sprintf(buf, "\r\n\nLock Type [%s]\r\n", buf2);
      send_to_char(buf, ch);
      print_choices(container_bits, olc_container_lock, ch);
      send_to_char("\r\n0. Previous Menu\r\n", ch);
    }
    else if(field == 9) {
      ok=1;
      sprintf(buf, "\r\n\nKey Number (vnum of key that unlocks this container, -1 = no key) [%d]\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    if(field == 7) {
      sprintf(buf, "\r\n\n"
                   "Maximum number of drinks it can contain [%d] (1 - %d)\r\n"
                   "  (Note: it depends on the liquid, but you use about 4 drinks\r\n"
                   "   when you're completely thirsty)\r\n", GET_OBJ_VAL(obj, 0),
              (GET_OBJ_TYPE(obj)==ITEM_DRINKCON) ? 10000 : 1000000);
      send_to_char(buf, ch);
    }
    else if(field == 8) {
      sprintf(buf, "\r\n\n"
                   "Number of drinks it contains [%d] (0 - %d)\r\n"
                   "  (Note: it depends on the liquid, but you use about 4 drinks\r\n"
                   "   when you're completely thirsty)\r\n", GET_OBJ_VAL(obj, 1),
              (GET_OBJ_TYPE(obj)==ITEM_DRINKCON) ? 10000 : 1000000);
      send_to_char(buf, ch);
    }
    else if(field == 9) {
      sprinttype(GET_OBJ_VAL(obj, 2), drinks, buf2);
      sprintf(buf, "\r\n\nType of liquid [%s]\r\n", buf2);
      send_to_char(buf, ch);
      print_choices(drinks, olc_drinks, ch);
    }
    else {
      sprintf(buf, "\r\n\nIf the liquid is poisoned [%s]\r\n0. No\r\n1. Yes\r\n", YESNO(GET_OBJ_VAL(obj, 3)));
      send_to_char(buf, ch);
    }
    ok=1;
    break;
  case ITEM_FOOD:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nMakes full [%d] (1 - 24)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    else if(field == 8) {
      ok=1;
      sprintf(buf, "\r\n\nIf the food is poisoned [%s]\r\n0. No\r\n1. Yes\r\n", YESNO(GET_OBJ_VAL(obj, 3)));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_MONEY:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nAmmount of money in the stash [%d] (1 - 1000000)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_BEAMER:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nVnum of beam down room [%d]\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    break;
  case ITEM_DAMAGEABLE:
    if(field == 7) {
      ok=1;
      sprintf(buf, "\r\n\nHow easily it's damaged (lower is weaker) [%d] (1 - 100)\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    if(field == 8) {
      ok=1;
      sprintf(buf, "\r\n\nStarting condition [%d] (0(perfect) - 9(almost dust))\r\n", GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
    }
    break;
  }
  if(!ok) {
    send_to_char("Invalid command.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
  }
}
void iedit_change_obj_type(struct obj_data *obj, struct char_data *ch, int field, int cmd)
{
  int ok=0, temp;
  field+=2;
  switch(GET_OBJ_TYPE(obj)) {
  case ITEM_LIGHT:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(-1, 10000);
    }
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 127);
    }
    else {
      if(cmd==0)
        temp=0;      else
        temp=get_choice(spells, olc_spells, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, field-7)=temp;
      }
    }
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    if(field == 7) {
      if(cmd==0)
        temp=0;
      else
        temp=get_choice(spells, olc_spells, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 3)=temp;
      }
    }
    else if(field == 8) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 127);
    }
    else if(field == 9) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(0, GET_OBJ_VAL(obj, 1));
    }
    else {
      ok=1;
      GET_OBJ_VAL(obj, 1)=RANGE(1, 100);
    }
    break;
  case ITEM_WEAPON:
    if(field == 7) {
      switch(cmd) {
      case 0:
        ok=1;
        break;
      case 1:
        GET_OLC_FIELD(ch)=105;
        break;
      case 2:
        GET_OLC_FIELD(ch)=106;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
    }
    if(field == 8) {
      if((temp=get_choice(weapon_types, olc_weapons, cmd)) < 0) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 3)=temp;
      }
    }
    break;
  case ITEM_ARMOR:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(-40, 40);
    }
    break;
  case ITEM_CONTAINER:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(0, 1000);
    }
    else if(field == 8) {
      if(cmd) {
        temp=get_choice(container_bits, olc_container_lock, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
        }
        else {
          TOGGLE_BIT(GET_OBJ_VAL(obj, 1), ((long long)1 << temp));
          if(IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) {
            SET_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED);
          }
          if(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) {
            SET_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE);
          }
        }
      }
      else {
        ok=1;
      }
    }
    else if(field == 9) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(-1, 99999);
    }
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, (GET_OBJ_TYPE(obj)==ITEM_DRINKCON) ? 10000 : 1000000);
    }
    else if(field == 8) {
      ok=1;
      GET_OBJ_VAL(obj, 1)=RANGE(1, GET_OBJ_VAL(obj, 0));
    }
    else if(field == 9) {
      if((temp=get_choice(drinks, olc_drinks, cmd)) < 0) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 2)=temp;
      }
    }
    else {
      switch(cmd) {
      case 0:
        ok=1;
        GET_OBJ_VAL(obj, 3)=0;
        break;
      case 1:
        ok=1;
        GET_OBJ_VAL(obj, 3)=1;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
    }
    break;
  case ITEM_FOOD:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 24);
    }
    else if(field == 8) {
      switch(cmd) {
      case 0:
        ok=1;
        GET_OBJ_VAL(obj, 3)=0;
        break;
      case 1:
        ok=1;
        GET_OBJ_VAL(obj, 3)=1;
        break;
      default:
        send_to_char("Invalid choice.\r\n", ch);
        break;
      }
    }
    break;
  case ITEM_MONEY:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 1000000);
    }
    break;
  case ITEM_BEAMER:
    if(field == 7) {
      temp=RANGE(0, 99999);
      if(real_room(temp) < 0) {
        send_to_char("No such room.\r\n", ch);
      }
      else {
        ok=1;
        GET_OBJ_VAL(obj, 0)=temp;
      }
    }
    break;
  case ITEM_DAMAGEABLE:
    if(field == 7) {
      ok=1;
      GET_OBJ_VAL(obj, 0)=RANGE(1, 100);
    }
    if(field == 8) {
      ok=1;
      GET_OBJ_VAL(obj, 2)=RANGE(0, 9);
    }
    break;
  }
  if(ok)
    GET_OLC_FIELD(ch)=0;
}
void iedit_menu(struct char_data *ch, int cmd)
{
  int i, temp;
  char line[MAX_STRING_LENGTH], tbuf1[MAX_STRING_LENGTH];
  struct obj_data *me=(struct obj_data *)GET_OLC_PTR(ch);
  switch(GET_OLC_FIELD(ch)) {
  case 0:
    if(cmd == OLC_MENU_NUM) {
      if(PRF_FLAGGED(ch, PRF_NOMENU))
        return;
      sprintf(line, "OBJ:[%ld]\r\n", GET_OLC_NUM(ch));
      send_to_char(line, ch);
      sprintbit(GET_OBJ_WEAR(me), wear_bits, tbuf1);
      sprintf(line, "1. Wearable on: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(GET_OBJ_EXTRA(me), extra_bits, tbuf1);
      sprintf(line, "2. Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.bitvector, affected_bits, tbuf1);
      sprintf(line, "3. Sets affects: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintf(line, "4. Weight:%d\r\n", GET_OBJ_WEIGHT(me));
      send_to_char(line, ch);
      iedit_print_obj_type_data(me, ch); /* 5-8 */
      sprintbit(me->obj_flags.immune, damtypes, tbuf1);
      sprintf(line, "9. Strong resistance: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.weak, damtypes, tbuf1);
      sprintf(line, "10. Vulnerabilities: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(me->obj_flags.resist, damtypes, tbuf1);
      sprintf(line, "11. Resistance: %s\r\n", tbuf1);
      send_to_char(line, ch);
      if(MAX_OBJ_AFFECT == 6) {
        for(i=0; i < MAX_OBJ_AFFECT; i++) {
          sprinttype(me->affected[i].location, apply_types, tbuf1);
          if(me->affected[i].location)
            sprintf(tbuf1, "%s %+d", tbuf1, me->affected[i].modifier);
          sprintf(line, "%2d. Affect%d: %s\r\n", 12+i, i+1, tbuf1);
          send_to_char(line, ch);
        }
      }
      else {
        send_to_char("ITEM AFFECTATION OLC NEEDS TO BE REWRITTEN\r\n", ch);
      }
      send_to_char("0. Exit\r\n", ch);
    }
    else {
      if (cmd > 17)
        send_to_char("Invalid command.\r\n", ch);
      else
        GET_OLC_FIELD(ch)=cmd;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 1:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(GET_OBJ_WEAR(me), wear_bits, tbuf1);
      sprintf(line, "\r\n\nCan be worn on [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(wear_bits, olc_wear, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(wear_bits, olc_wear, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(GET_OBJ_WEAR(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 2:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(GET_OBJ_EXTRA(me), extra_bits, tbuf1);
      sprintf(line, "\r\n\nFlags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(extra_bits, olc_iedit_extra, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(extra_bits, olc_iedit_extra, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(GET_OBJ_EXTRA(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 3:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.bitvector, affected_bits, tbuf1);
      sprintf(line, "\r\n\nSets affects [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(affected_bits, olc_objaff_bits, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(affected_bits, olc_objaff_bits, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.bitvector, ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 4:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nWeight [%d] (1 - 10000)\r\n", GET_OBJ_WEIGHT(me));
      send_to_char(line, ch);
    }
    else {
      GET_OBJ_WEIGHT(me)=RANGE(1, 10000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 5:
  case 6:
  case 7:
  case 8:
    if(cmd == OLC_MENU_NUM) {
      iedit_print_obj_type(me, ch, GET_OLC_FIELD(ch));
    }
    else {
      iedit_change_obj_type(me, ch, GET_OLC_FIELD(ch), cmd);
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 9:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.immune, damtypes, tbuf1);
      sprintf(line, "\r\n\nProvides exceptional resistance to:\r\n[%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.immune, ((long long)1 << temp));
        if(IS_SET(me->obj_flags.immune, ((long long)1 << temp))) {
          REMOVE_BIT(me->obj_flags.weak, ((long long)1 << temp));
          REMOVE_BIT(me->obj_flags.resist, ((long long)1 << temp));
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 10:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.weak, damtypes, tbuf1);
      sprintf(line, "\r\n\nMakes weak against:\r\n[%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.weak, ((long long)1 << temp));
        if(IS_SET(me->obj_flags.weak, ((long long)1 << temp))) {
          REMOVE_BIT(me->obj_flags.immune, ((long long)1 << temp));
          REMOVE_BIT(me->obj_flags.resist, ((long long)1 << temp));
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 11:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->obj_flags.resist, damtypes, tbuf1);
      sprintf(line, "\r\n\nProvides some resistance to:\r\n[%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(damtypes, olc_damtype, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(damtypes, olc_damtype, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->obj_flags.resist, ((long long)1 << temp));
        if(IS_SET(me->obj_flags.resist, ((long long)1 << temp))) {
          REMOVE_BIT(me->obj_flags.weak, ((long long)1 << temp));
          REMOVE_BIT(me->obj_flags.immune, ((long long)1 << temp));
        }
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
    i=GET_OLC_FIELD(ch)-12;
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->affected[i].location, apply_types, tbuf1);
      sprintf(line, "\r\n\nAffects [%s] by %+d\r\n", tbuf1, me->affected[i].modifier);
      send_to_char(line, ch);
      print_choices(apply_types, olc_apply, ch);
    }
    else {
      temp=get_choice(apply_types, olc_apply, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      me->affected[i].location=temp;
      GET_OLC_FIELD(ch) += 88;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
    i=GET_OLC_FIELD(ch)-100;
    if(cmd == OLC_MENU_NUM) {
      sprinttype(me->affected[i].location, apply_types, tbuf1);
      sprintf(line, "\r\n\nAffects %s by [%+d] (-50 - 50)\r\n", tbuf1, me->affected[i].modifier);
      send_to_char(line, ch);
    }
    else {
      me->affected[i].modifier=RANGE(-50, 50);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  default:
    send_to_char("Incomplete field.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  }
}
void pedit_menu(struct char_data *ch, int cmd)
{
  int i, temp;
  char line[MAX_STRING_LENGTH], tbuf1[MAX_STRING_LENGTH], *ptr;
  char tbuf2[MAX_INPUT_LENGTH];
  struct char_data *me = (struct char_data *)GET_OLC_PTR(ch);
  switch(GET_OLC_FIELD(ch)) {
  case 0:
    if(cmd == OLC_MENU_NUM) {
      if(PRF_FLAGGED(ch, PRF_NOMENU))
        return;
      sprintf(line, "PLAYER:[%s]\r\n", GET_NAME(me));
      send_to_char(line, ch);
      sprintf(line, "1. Description:\r\n%s", (me->player.description && *me->player.description) ? me->player.description : "");
      send_to_char(line, ch);
      sprintf(line, "2. Title: %s\r\n", GET_TITLE(me));
      send_to_char(line, ch);
      sprintf(line, "3. Walkin: %s\r\n", WALKIN(me));
      send_to_char(line, ch);
      sprintf(line, "4. Walkout: %s\r\n", WALKOUT(me));
      send_to_char(line, ch);
      sprintf(line, "5. Poofin: %s\r\n", POOFIN(me));
      send_to_char(line, ch);
      sprintf(line, "6. Poofout: %s\r\n", POOFOUT(me));
      send_to_char(line, ch);
      sprintf(line, "7. Str:%d    8. Str add:%d    9. Int:%d    10. Wis:%d\r\n",
              me->real_abils.str, me->real_abils.str_add, me->real_abils.intel,
              me->real_abils.wis);
      send_to_char(line, ch);
      sprintf(line, "11. Dex:%d    12. Con:%d    13. Cha:%d\r\n", me->real_abils.dex, me->real_abils.con, me->real_abils.cha);
      send_to_char(line, ch);
      sprinttype(GET_CLASS(me), pc_class_types, tbuf1);
      sprinttype(GET_SEX(me), genders, tbuf2);
      sprintf(line, "14. Class: %s  15. Num Classes: %d  16. Sex: %s\r\n",
              tbuf1, GET_NUM_CLASSES(me), tbuf2);
      send_to_char(line, ch);
      if(NUM_CLASSES > 13) {
        strcpy(line, "PEDIT NEEDS A REWRITE FOR NEW NUMBER OF CLASSES\r\n");
      }
      else {
        strcpy(line, "Levels:  ");
        for(i=0; i<NUM_CLASSES; i++) {
          sprintf(tbuf1, "%d. %s: %d", 17+i, class_abbrevs[i], GET_CLASS_LEVEL(me, i));
          strcat(line, tbuf1);
          if((i+1) < NUM_CLASSES) {
            if((i+1)%4)
              strcat(line, "   ");
            else
              strcat(line, "\r\n         ");
          }
          else
           strcat(line, "\r\n");
        }
      }
      send_to_char(line, ch);
      sprintf(line, "Running totals: 30. Maxhit:%d   31. Maxmana:%d   32. Maxmove:%d\r\n",
              me->player_specials->saved.new_hit, me->player_specials->saved.new_mana,
              me->player_specials->saved.new_move);
      send_to_char(line, ch);
      sprintf(line, "Old totals: 33. Maxhit:%d   34. Maxmana:%d   35. Maxmove:%d\r\n",
              me->player_specials->saved.old_hit, me->player_specials->saved.old_mana,
              me->player_specials->saved.old_move);
      send_to_char(line, ch);
      sprintf(line, "Current points: 36. Hit:%d   37. Mana:%d   38. Move:%d\r\n",
              GET_HIT(me), GET_MANA(me),GET_MOVE(me));
      send_to_char(line, ch);
      sprintf(line, "39. Exp:%ld   40. Pracs:%d   41. Extra Pracs:%d\r\n", GET_EXP(me),
              GET_PRACTICES(me), me->player_specials->saved.extra_pracs);
      send_to_char(line, ch);
      sprintf(line, "42. Num Rerolls:%d   43. Reroll Level:%d   44. Quest points:%d\r\n",
              me->player_specials->saved.num_rerolls,
              me->player_specials->saved.reroll_level, GET_QP(me));
      send_to_char(line, ch);
      sprintf(line, "45. Natural AC Apply:%d   46. Unnatural Age Adjustment:%+d\r\n",
              me->player_specials->saved.inherent_ac_apply,
              me->player_specials->saved.age_add);
      send_to_char(line, ch);
      sprintf(line, "47. Gold:%ld   48. Bank Gold:%ld   49. Alignment:%d\r\n",
              GET_GOLD(me), GET_BANK_GOLD(me), GET_ALIGNMENT(me));
      send_to_char(line, ch);
      sprintf(line, "50. Weight:%d   51. Height:%d\r\n", me->player.weight,
              me->player.height);
      send_to_char(line, ch);
      sprintf(line, "52. Full:%d   53. Thirst:%d   54. Drunk:%d\r\n",
              GET_COND(me, 1), GET_COND(me, 2), GET_COND(me, 0));
      send_to_char(line, ch);
      sprintf(line, "55. Min OLC Zone1: %d   56. Max OLC Zone1: %d\r\n",
              me->player_specials->saved.olc_min1,
              me->player_specials->saved.olc_max1);
      send_to_char(line, ch);
      sprintf(line, "57. Min OLC Zone2: %d   58. Max OLC Zone2: %d\r\n",
              me->player_specials->saved.olc_min2,
              me->player_specials->saved.olc_max2);
      send_to_char(line, ch);
      sprintf(line, "59. Prompt: %s\r\n", me->char_specials.prompt);
      send_to_char(line, ch);
      sprintbit(me->char_specials.saved.affected_by, affected_bits, tbuf1);
      sprintf(line, "60. AFF Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(PRF_FLAGS(me), preference_bits, tbuf1);
      sprintf(line, "61. PRF Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(PLR_FLAGS(me), player_bits, tbuf1);
      sprintf(line, "62. PLR Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      sprintbit(GRNT_FLAGS(me), grant_bits, tbuf1);
      sprintf(line, "63. GRANT Flags: %s\r\n", tbuf1);
      send_to_char(line, ch);
      send_to_char("0. Exit\r\n", ch);
    }
    else {
      if (cmd > 63)
        send_to_char("Invalid command.\r\n", ch);
      else
        GET_OLC_FIELD(ch)=cmd;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 1:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the text people will see when they look at this player.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->player.description) {
        free(me->player.description);
        me->player.description=NULL;
      }
      ch->desc->str = &me->player.description;
      ch->desc->max_str = EXDSCR_LENGTH;
    }
    else {
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 2:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the player's title.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(GET_TITLE(me)) {
        free(GET_TITLE(me));
        GET_TITLE(me)=NULL;
      }
      ch->desc->str = &GET_TITLE(me);
      ch->desc->max_str = MAX_TITLE_LENGTH;
    }
    else {
      for(ptr=GET_TITLE(me); ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 3:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the player's walkin.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(WALKIN(me)) {
        free(WALKIN(me));
        WALKIN(me)=NULL;
      }
      ch->desc->str = &WALKIN(me);
      ch->desc->max_str = MAX_WALK_LENGTH;
    }
    else {
      for(ptr=WALKIN(me); ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 4:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the player's walkout.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(WALKOUT(me)) {
        free(WALKOUT(me));
        WALKOUT(me)=NULL;
      }
      ch->desc->str = &WALKOUT(me);
      ch->desc->max_str = MAX_WALK_LENGTH;
    }
    else {
      for(ptr=WALKOUT(me); ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 5:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the player's poofin.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(POOFIN(me)) {
        free(POOFIN(me));
        POOFIN(me)=NULL;
      }
      ch->desc->str = &POOFIN(me);
      ch->desc->max_str = MAX_WALK_LENGTH;
    }
    else {
      for(ptr=POOFIN(me); ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }    break;
  case 6:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the player's poofout.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(POOFOUT(me)) {
        free(POOFOUT(me));
        POOFOUT(me)=NULL;
      }
      ch->desc->str = &POOFOUT(me);
      ch->desc->max_str = MAX_WALK_LENGTH;
    }
    else {
      for(ptr=WALKOUT(me); ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 7:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nStrength [%d] (0 - %d)\r\n", me->real_abils.str, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      send_to_char(line, ch);
    }
    else {
      me->real_abils.str=RANGE(0, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 8:
    if(cmd == OLC_MENU_NUM) {
      if(me->real_abils.str != 18) {
        send_to_char("\r\n\nThis only applies if strength is 18.\r\n", ch);
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        sprintf(line, "\r\n\nStrength bonus [%d] (0 - 100)\r\n", me->real_abils.str_add);
        send_to_char(line, ch);
      }
    }
    else {
      me->real_abils.str_add=RANGE(0, 100);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 9:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nIntelligence [%d] (0 - %d)\r\n", me->real_abils.intel, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      send_to_char(line, ch);
    }
    else {
      me->real_abils.intel=RANGE(0, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 10:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nWisdom [%d] (0 - %d)\r\n", me->real_abils.wis, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      send_to_char(line, ch);
    }
    else {
      me->real_abils.wis=RANGE(0, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 11:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nDexterity [%d] (0 - %d)\r\n", me->real_abils.dex, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      send_to_char(line, ch);
    }
    else {
      me->real_abils.dex=RANGE(0, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 12:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nConstitution [%d] (0 - %d)\r\n", me->real_abils.con, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      send_to_char(line, ch);
    }
    else {
      me->real_abils.con=RANGE(0, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 13:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nCharisma [%d] (0 - %d)\r\n", me->real_abils.cha, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      send_to_char(line, ch);
    }
    else {
      me->real_abils.cha=RANGE(0, ((GET_LEVEL(me) >= LVL_HERO) ? 25 : 18));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 14:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(GET_CLASS(me), pc_class_types, tbuf1);
      sprintf(line, "\r\n\nClass [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(pc_class_types, olc_pc_classes, ch);
    }
    else {
      temp=get_choice(pc_class_types, olc_pc_classes, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      GET_CLASS(me)=temp;
      GET_LEVEL(me)=GET_CLASS_LEVEL(me, temp);
      cmd=GET_EXP(me);
      GET_EXP(me)=RANGE(GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)], GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)+1]-1);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 15:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of Classes [%d] (1 - %d)\r\n", GET_NUM_CLASSES(me), NUM_CLASSES);
      send_to_char(line, ch);
    }
    else {
      GET_NUM_CLASSES(me)=RANGE(1, NUM_CLASSES);
      cmd=GET_EXP(me);
      GET_EXP(me)=RANGE(GET_NUM_CLASSES(me)*exp_table[(int)GET_CLASS(me)][(int)GET_LEVEL(me)], GET_NUM_CLASSES(me)*exp_table[(int)GET_CLASS(me)][(int)GET_LEVEL(me)+1]-1);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 16:
    if(cmd == OLC_MENU_NUM) {
      sprinttype(GET_SEX(me), genders, tbuf1);
      sprintf(line, "\r\n\nSex [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(genders, olc_sex, ch);
    }
    else {
      temp=get_choice(genders, olc_sex, cmd);
      if(temp == -1) {
        send_to_char("Invalid choice.\r\n", ch);
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      GET_SEX(me)=temp;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
    temp=GET_OLC_FIELD(ch)-17;
    if(temp >= NUM_CLASSES) {
      send_to_char("Invalid command.\r\n", ch);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    else {
      if(cmd == OLC_MENU_NUM) {
        sprintf(line, "\r\n\n%s level [%d] (0 - %d)\r\n", pc_class_types[temp], GET_CLASS_LEVEL(me, temp), GET_LEVEL(ch));
        send_to_char(line, ch);
      }
      else {
        GET_CLASS_LEVEL(me, temp)=RANGE(0, GET_LEVEL(ch));
        if(GET_CLASS(me) == temp) {
          GET_CLASS_LEVEL(me, temp)=RANGE(1, GET_LEVEL(ch));
          GET_LEVEL(me)=GET_CLASS_LEVEL(me, temp);
          cmd=GET_EXP(me);
          GET_EXP(me)=RANGE(GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)], GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)+1]-1);
        }
        if(GET_CLASS_LEVEL(me, temp))
          SET_BIT(GET_CLASS_BITVECTOR(me), (1 << temp));
        else
          REMOVE_BIT(GET_CLASS_BITVECTOR(me), (1 << temp));
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 30:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nRunning maxhit total [%d] (0 - %d)\r\n", me->player_specials->saved.new_hit, MAX_HIT);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.new_hit=RANGE(0, MAX_HIT);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 31:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nRunning maxmana total [%d] (0 - %d)\r\n", me->player_specials->saved.new_mana, MAX_MANA);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.new_mana=RANGE(0, MAX_MANA);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 32:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nRunning maxmove total [%d] (0 - %d)\r\n", me->player_specials->saved.new_move, MAX_MOVE);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.new_move=RANGE(0, MAX_MOVE);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 33:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nPrevious maxhit total [%d] (0 - %d)\r\n", me->player_specials->saved.old_hit, MAX_HIT);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.old_hit=RANGE(0, MAX_HIT);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 34:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nPrevious maxmana total [%d] (0 - %d)\r\n", me->player_specials->saved.old_mana, MAX_MANA);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.old_mana=RANGE(0, MAX_MANA);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 35:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nRunning maxmove total [%d] (0 - %d)\r\n", me->player_specials->saved.old_move, MAX_MOVE);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.old_move=RANGE(0, MAX_MOVE);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 36:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nCurrent hit points [%d] (0 - %d)\r\n", GET_HIT(me), MAX(me->player_specials->saved.new_hit, me->player_specials->saved.old_hit));
      send_to_char(line, ch);
    }
    else {
      GET_HIT(me)=RANGE(0, MAX(me->player_specials->saved.new_hit, me->player_specials->saved.old_hit));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 37:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nCurrent mana [%d] (0 - %d)\r\n", GET_MANA(me), MAX(me->player_specials->saved.new_mana, me->player_specials->saved.old_mana));
      send_to_char(line, ch);
    }
    else {
      GET_MANA(me)=RANGE(0, MAX(me->player_specials->saved.new_mana, me->player_specials->saved.old_mana));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 38:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nCurrent move [%d] (0 - %d)\r\n", GET_MOVE(me), MAX(me->player_specials->saved.new_move, me->player_specials->saved.old_move));
      send_to_char(line, ch);
    }
    else {
      GET_MOVE(me)=RANGE(0, MAX(me->player_specials->saved.new_move, me->player_specials->saved.old_move));
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 39:
    temp=GET_CLASS(ch);
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nExperience [%ld] (%ld - %ld)\r\n", GET_EXP(me), GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)], GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)+1]-1);
      send_to_char(line, ch);
    }
    else {
      GET_EXP(me)=RANGE(GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)], GET_NUM_CLASSES(me)*exp_table[temp][(int)GET_LEVEL(me)+1]-1);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 40:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nPractices [%d] (0 - 1000)\r\n", GET_PRACTICES(me));
      send_to_char(line, ch);
    }
    else {
      GET_PRACTICES(me)=RANGE(0, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 41:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of practices not spent with the practice command [%d] (0 - 1000)\r\n", me->player_specials->saved.extra_pracs);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.extra_pracs=RANGE(0, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 42:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNumber of rerolls [%d] (0 - 100)\r\n", me->player_specials->saved.num_rerolls);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.num_rerolls=RANGE(0, 100);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 43:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nLeve at which they rerolled (for practice gaining) [%d] (0 - 100)\r\n", me->player_specials->saved.reroll_level);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.reroll_level=RANGE(0, 100);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 44:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nQuest points [%d] (0 - 30000)\r\n", GET_QP(me));
      send_to_char(line, ch);
    }
    else {
      GET_QP(me)=RANGE(0, 30000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 45:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nNatural AC apply [%d] (0 - 1000)\r\n", me->player_specials->saved.inherent_ac_apply);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.inherent_ac_apply=RANGE(0, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 46:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nUnnatural age adjustment [%d] (-100 - 100)\r\n", me->player_specials->saved.age_add);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.age_add=RANGE(-100, 100);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 47:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nGold [%ld] (0 - 1000000000)\r\n", GET_GOLD(me));
      send_to_char(line, ch);
    }
    else {
      GET_GOLD(me)=RANGE(0, 1000000000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 48:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nGold in ATM [%ld] (0 - 1000000000)\r\n", GET_BANK_GOLD(me));
      send_to_char(line, ch);
    }
    else {
      GET_BANK_GOLD(me)=RANGE(0, 1000000000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 49:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nAlignment [%d] (-1000 - 1000)\r\n", GET_ALIGNMENT(me));
      send_to_char(line, ch);
    }
    else {
      GET_ALIGNMENT(me)=RANGE(-1000, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 50:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nWeight (lbs) [%d] (100 - 200)\r\n", me->player.weight);
      send_to_char(line, ch);
    }
    else {
      me->player.weight=RANGE(100, 200);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 51:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nHeight (cm) [%d] (100 - 200)\r\n", me->player.height);
      send_to_char(line, ch);
    }
    else {
      me->player.height=RANGE(100, 200);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 52:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nFull (-1 is off) [%d] (-1 - 24)\r\n", GET_COND(me, 1));
      send_to_char(line, ch);
    }
    else {
      GET_COND(me, 1)=RANGE(-1, 24);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 53:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nThirst (-1 is off) [%d] (-1 - 24)\r\n", GET_COND(me, 2));
      send_to_char(line, ch);
    }
    else {
      GET_COND(me, 2)=RANGE(-1, 24);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 54:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nDrunk (-1 is off) [%d] (-1 - 24)\r\n", GET_COND(me, 0));
      send_to_char(line, ch);
    }
    else {
      GET_COND(me, 0)=RANGE(-1, 24);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 55:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMin OLC Zone1 [%d] (0 - 1000)\r\n", me->player_specials->saved.olc_min1);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.olc_min1=RANGE(0, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 56:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMax OLC Zone1 [%d] (%d - 1000)\r\n", me->player_specials->saved.olc_max1, me->player_specials->saved.olc_min1);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.olc_max1=RANGE(me->player_specials->saved.olc_min1, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 57:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMin OLC Zone2 [%d] (0 - 1000)\r\n", me->player_specials->saved.olc_min2);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.olc_min2=RANGE(0, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 58:
    if(cmd == OLC_MENU_NUM) {
      sprintf(line, "\r\n\nMax OLC Zone2 [%d] (%d - 1000)\r\n", me->player_specials->saved.olc_max2, me->player_specials->saved.olc_min2);
      send_to_char(line, ch);
    }
    else {
      me->player_specials->saved.olc_max2=RANGE(me->player_specials->saved.olc_min2, 1000);
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 59:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the player's prompt.\r\nEnter @ on a blank line when done.\r\n", ch);
      if(me->char_specials.prompt) {
        free(me->char_specials.prompt);
        me->char_specials.prompt=NULL;
      }
      ch->desc->str = &me->char_specials.prompt;
      ch->desc->max_str = MAX_PROMPT_LENGTH;
    }
    else {
      for(ptr=me->char_specials.prompt; ((*ptr != '\n') && (*ptr != '\r') && (*ptr)); ptr++);
      *ptr=0;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 60:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(me->char_specials.saved.affected_by, affected_bits, tbuf1);
      sprintf(line, "\r\n\nAFF Flags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(affected_bits, olc_aff, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(affected_bits, olc_aff, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(me->char_specials.saved.affected_by, ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 61:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(PRF_FLAGS(me), preference_bits, tbuf1);
      sprintf(line, "\r\n\nPRF Flags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(preference_bits, olc_prf, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(preference_bits, olc_prf, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(PRF_FLAGS(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 62:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(PLR_FLAGS(me), player_bits, tbuf1);
      sprintf(line, "\r\n\nPLR Flags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(player_bits, olc_plr, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(player_bits, olc_plr, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(PLR_FLAGS(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  case 63:
    if(cmd == OLC_MENU_NUM) {
      sprintbit(GRNT_FLAGS(me), grant_bits, tbuf1);
      sprintf(line, "\r\n\nGRNT Flags [%s]\r\n", tbuf1);
      send_to_char(line, ch);
      print_choices(grant_bits, olc_grnt, ch);
      send_to_char("0. Previous Menu\r\n", ch);
    }
    else {
      if(cmd) {
        temp=get_choice(grant_bits, olc_grnt, cmd);
        if(temp == -1) {
          send_to_char("Invalid choice.\r\n", ch);
          do_olcmenu(ch, "", 0, 0);
          return;
        }
        TOGGLE_BIT(GRNT_FLAGS(me), ((long long)1 << temp));
        do_olcmenu(ch, "", 0, 0);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
    }
    break;
  default:
    send_to_char("Incomplete field.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  }
}
int print_zone_command_list(struct zcmd_data *zcmd, int level, struct char_data *ch, char **zbuf, int number)
{
  int i;
  char buf[MAX_STRING_LENGTH], buf1[20];
  char buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
/* Change to deal with sucky os that doesn't like recursion
  if(zcmd) { */
  while(zcmd) {
    buf[0]=0;
    if(level > 0) {
      sprintf(buf, "%-4d", number);
      for(i=1; i<level; i++)
        strcat(buf, "  ");
      if(zcmd->if_next || zcmd->else_next)
        strcat(buf, "IF ");
      switch(zcmd->command) {
      case 'M':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, mob_proto[zcmd->arg1].player.short_descr);
        buf2[15]=0;
        strcpy(buf3, world[zcmd->arg3].name);
        buf3[15]=0;
        sprintf(buf, "%sm%d (%s) in room %d (%s)", buf,
                mob_index[zcmd->arg1].virtual, buf2, world[zcmd->arg3].number, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'O':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[15]=0;
        strcpy(buf3, world[zcmd->arg3].name);
        buf3[15]=0;
        sprintf(buf, "%so%d (%s) in room %d (%s)", buf,
                obj_index[zcmd->arg1].virtual, buf2, world[zcmd->arg3].number, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'P':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[15]=0;
        strcpy(buf3, obj_proto[zcmd->arg3].short_description);
        buf3[15]=0;
        sprintf(buf, "%s%d (%s) in obj %d (%s)", buf,
                obj_index[zcmd->arg1].virtual, buf2, obj_index[zcmd->arg3].virtual, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'G':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[15]=0;
        strcpy(buf3, mob_proto[zcmd->arg3].player.short_descr);
        buf3[15]=0;
        sprintf(buf, "%s%d (%s) to inventory of mob %d (%s)", buf,
                obj_index[zcmd->arg1].virtual, buf2, mob_index[zcmd->arg3].virtual, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'E':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[10]=0;
        strcpy(buf3, mob_proto[zcmd->arg3].player.short_descr);
        buf3[10]=0;
        sprintf(buf, "%s%d (%s) to mob %d (%s), %s", buf,
                obj_index[zcmd->arg1].virtual, buf2, mob_index[zcmd->arg3].virtual,
                buf3, equipment_types[zcmd->arg4]);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'D':
        buf1[0]=0;
        switch(zcmd->arg3) {
        case 0:
          strcpy(buf1, "open");
          break;
        case 1:
          strcpy(buf1, "closed");
          break;
        case 2:
          strcpy(buf1, "closed and locked");
          break;
        }
        sprintf(buf2, world[zcmd->arg1].name);
        buf2[20]=0;
        sprintf(buf, "%s%s exit of room %d (%s) %s", buf,
                dirs[zcmd->arg2], world[zcmd->arg1].number, buf2, buf1);
        sprintf(buf, "%s Prob:%d\r\n", buf, zcmd->prob);
        break;
      }
      RECREATE(*zbuf, char, strlen(*zbuf)+strlen(buf)+1);
      strcat(*zbuf, buf);
    }
    else {
      strcpy(buf, "0. TOP OF COMMAND TABLE\r\n");
      RECREATE(*zbuf, char, strlen(*zbuf)+strlen(buf)+1);
      strcat(*zbuf, buf);
      level++;
    }
    number++;
    if(zcmd->if_next)
      number=print_zone_command_list(zcmd->if_next, level+1, ch, zbuf, number);
    if(zcmd->else_next) {
      buf[0]=0;
      for(i=1; i<level; i++)
        strcat(buf, "  ");
      strcat(buf, "    ELSE\r\n");
      RECREATE(*zbuf, char, strlen(*zbuf)+strlen(buf)+1);
      strcat(*zbuf, buf);
      number=print_zone_command_list(zcmd->else_next, level+1, ch, zbuf, number);
    }
/* Change to deal with sucky os that doesn't like recursion
    if(zcmd->next)
      number=print_zone_command_list(zcmd->next, level, ch, zbuf, number); */
    zcmd=zcmd->next;
  }
  return(number);
}
void print_zone_commands(struct zcmd_data *zcmd, int level, struct char_data *ch, char **zbuf)
{
  int i;
  char buf[MAX_STRING_LENGTH], buf1[20];
  char buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
/* Change to deal with sucky os that doesn't like recursion
  if(zcmd) { */
  while(zcmd) {
    buf[0]=0;
    if(level > 0) {
      for(i=1; i<level; i++)
        strcat(buf, "  ");
      if(zcmd->if_next || zcmd->else_next)
        strcat(buf, "IF ");
      switch(zcmd->command) {
      case 'M':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, mob_proto[zcmd->arg1].player.short_descr);
        buf2[15]=0;
        strcpy(buf3, world[zcmd->arg3].name);
        buf3[15]=0;
        sprintf(buf, "%sm%d (%s) in room %d (%s)", buf,
                mob_index[zcmd->arg1].virtual, buf2, world[zcmd->arg3].number, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'O':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[15]=0;
        strcpy(buf3, world[zcmd->arg3].name);
        buf3[15]=0;
        sprintf(buf, "%so%d (%s) in room %d (%s)", buf,
                obj_index[zcmd->arg1].virtual, buf2, world[zcmd->arg3].number, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'P':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[15]=0;
        strcpy(buf3, obj_proto[zcmd->arg3].short_description);
        buf3[15]=0;
        sprintf(buf, "%s%d (%s) in obj %d (%s)", buf,
                obj_index[zcmd->arg1].virtual, buf2, obj_index[zcmd->arg3].virtual, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'G':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[15]=0;
        strcpy(buf3, mob_proto[zcmd->arg3].player.short_descr);
        buf3[15]=0;
        sprintf(buf, "%s%d (%s) to inventory of mob %d (%s)", buf,
                obj_index[zcmd->arg1].virtual, buf2, mob_index[zcmd->arg3].virtual, buf3);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'E':
        if(zcmd->arg2 < 0)
          strcpy(buf1, "none");
        else
          sprintf(buf1, "%d", zcmd->arg2);
        strcpy(buf2, obj_proto[zcmd->arg1].short_description);
        buf2[10]=0;
        strcpy(buf3, mob_proto[zcmd->arg3].player.short_descr);
        buf3[10]=0;
        sprintf(buf, "%s%d (%s) to mob %d (%s), %s", buf,
                obj_index[zcmd->arg1].virtual, buf2, mob_index[zcmd->arg3].virtual,
                buf3, equipment_types[zcmd->arg4]);
        sprintf(buf, "%s Prob:%d Lim:%s\r\n", buf, zcmd->prob, buf1);
        break;
      case 'D':
        buf1[0]=0;
        switch(zcmd->arg3) {
        case 0:
          strcpy(buf1, "open");
          break;
        case 1:
          strcpy(buf1, "closed");
          break;
        case 2:
          strcpy(buf1, "closed and locked");
          break;
        }
        sprintf(buf2, world[zcmd->arg1].name);
        buf2[20]=0;
        sprintf(buf, "%s%s exit of room %d (%s) %s", buf,
                dirs[zcmd->arg2], world[zcmd->arg1].number, buf2, buf1);
        sprintf(buf, "%s Prob:%d\r\n", buf, zcmd->prob);
        break;
      }
      RECREATE(*zbuf, char, strlen(*zbuf)+strlen(buf)+1);
      strcat(*zbuf, buf);
    }
    else
      level++;
    if(zcmd->if_next)
      print_zone_commands(zcmd->if_next, level+1, ch, zbuf);
    if(zcmd->else_next) {
      buf[0]=0;
      for(i=1; i<level; i++)
        strcat(buf, "  ");
      strcat(buf, "ELSE\r\n");
      RECREATE(*zbuf, char, strlen(*zbuf)+strlen(buf)+1);
      strcat(*zbuf, buf);
      print_zone_commands(zcmd->else_next, level+1, ch, zbuf);
    }
/* Change to deal with sucky os that doesn't like recursion
    if(zcmd->next)
      print_zone_commands(zcmd->next, level, ch, zbuf); */
    zcmd=zcmd->next;
  }
}
struct zcmd_data *get_zone_command_by_num(struct zcmd_data *zcmd, int *num, int look_for)
{
  struct zcmd_data *temp=NULL;
  if(zcmd) {
    (*num)++;
    if((*num)==look_for)
      return(zcmd);
    if((temp=get_zone_command_by_num(zcmd->if_next, num, look_for)))
      return(temp);
    if((temp=get_zone_command_by_num(zcmd->else_next, num, look_for)))
      return(temp);
    if((temp=get_zone_command_by_num(zcmd->next, num, look_for)))
      return(temp);
  }
  return(temp);
}
int count_zcmds(struct zcmd_data *z)
{
  int count=0;
  if(z) {
    count+=count_zcmds(z->if_next);
    count+=count_zcmds(z->else_next);
    count+=count_zcmds(z->next);
    count++;
  }
  return(count);
}
struct zcmd_data *copy_zcmd(struct zcmd_data *z)
{
  struct zcmd_data *temp=NULL;
  if(z) {
    CREATE(temp, struct zcmd_data, 1);
    temp->command=z->command;
    temp->arg1=z->arg1;
    temp->arg2=z->arg2;
    temp->arg3=z->arg3;
    temp->arg4=z->arg4;
    temp->line=z->line;
    temp->prob=z->prob;
    if((temp->if_next=copy_zcmd(z->if_next)))
      temp->if_next->prev=temp;
    if((temp->else_next=copy_zcmd(z->else_next)))
      temp->else_next->prev=temp;
    if((temp->next=copy_zcmd(z->next)))
      temp->next->prev=temp;
  }
  return(temp);
}
void zedit_menu(struct char_data *ch, int cmd)
{
  int i=0;
  char line[MAX_STRING_LENGTH], *zbuf;
  struct zcmd_data *zcmd, *temp;
  switch(GET_OLC_FIELD(ch)) {
  case 0:
    if(cmd == OLC_MENU_NUM) {
      if(PRF_FLAGGED(ch, PRF_NOMENU))
        return;
      sprintf(line, "%s\r\n", zone_table[real_zone(GET_OLC_NUM(ch))].name);
      zbuf=str_dup(line);
      print_zone_commands((struct zcmd_data *)GET_OLC_PTR(ch), 0, ch, &zbuf);
      RECREATE(zbuf, char, strlen(zbuf)+140);
      strcat(zbuf, "\r\n1. Add Command\r\n");
      strcat(zbuf, "2. Delete Command\r\n");
      strcat(zbuf, "3. Edit Command\r\n");
      strcat(zbuf, "4. Move Command\r\n");
      strcat(zbuf, "5. Copy Command\r\n");
      strcat(zbuf, "99. Save\r\n");
      strcat(zbuf, "0. Exit\r\n");
      page_string(ch->desc, zbuf, 1);
      free(zbuf);
    }
    else {
      if ((cmd > 5) && (cmd != 99))
        send_to_char("Invalid command.\r\n", ch);
      else
        GET_OLC_FIELD(ch)=cmd;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 1:
    strcpy(line, "\r\n\nAfter which line do you want to add a command?\r\n");
    i=1;
  case 2:
    if(!i)
      strcpy(line, "\r\n\nWhat command do you want to delete?\r\n");
    i=1;
  case 3:
    if(!i)
      strcpy(line, "\r\n\nWhat command do you want to edit?\r\n");
    i=1;
  case 4:
    if(!i)
      strcpy(line, "\r\n\nWhat command do you want to move?\r\n");
    i=1;
  case 5:
    if(!i)
      strcpy(line, "\r\n\nWhat command do you want to copy?\r\n");
    if(cmd == OLC_MENU_NUM) {
      if((!((struct zcmd_data *)GET_OLC_PTR(ch))->next) && (GET_OLC_FIELD(ch) > 1)) {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
        return;
      }
      zbuf=str_dup(line);
      print_zone_command_list((struct zcmd_data *)GET_OLC_PTR(ch), 0, ch, &zbuf, 0);
      page_string(ch->desc, zbuf, 1);
      free(zbuf);
    }
    else {
      if((((GET_OLC_FIELD(ch) > 2) && (cmd < 1)) || (cmd < 0)) || (cmd > count_zcmds(((struct zcmd_data *)GET_OLC_PTR(ch))->next))) {
        send_to_char("No such zone command.\r\n", ch);
      }
      else {
        i=0;
        if(cmd!=0) {
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=get_zone_command_by_num(((struct zcmd_data *)GET_OLC_PTR(ch))->next, &i, cmd);
          GET_OLC_FIELD(ch) += 5;
        }
        else {
          if(GET_OLC_FIELD(ch) == 2) {
            GET_OLC_FIELD(ch) = 0;
          }
          else {
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=((struct zcmd_data *)GET_OLC_PTR(ch));
            GET_OLC_FIELD(ch) += 5;
          }
        }
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 6:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\n1. No dependance on the previous command\r\n", ch);
      if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev != ((struct zcmd_data *)GET_OLC_PTR(ch))) {
        send_to_char("2. Depend on the previous command happening\r\n", ch);
        send_to_char("3. Depend on the previous command not happening\r\n", ch);
      }
    }
    else {
      if((((struct zcmd_data *)GET_OLC_PTR(ch))->prev == ((struct zcmd_data *)GET_OLC_PTR(ch))) && (cmd > 1))
        cmd=5;
      switch(cmd) {
      case 1:
        GET_OLC_FIELD(ch)=49;
        for(temp=((struct zcmd_data *)GET_OLC_PTR(ch))->prev; temp != ((struct zcmd_data *)GET_OLC_PTR(ch)); temp=temp->prev) {
          if((temp==temp->prev->if_next) || (temp==temp->prev->else_next)) {
            GET_OLC_FIELD(ch)=50;
            break;
          }
        }
        CREATE(zcmd, struct zcmd_data, 1);
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=zcmd;
        zcmd->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=zcmd;
        zcmd->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=zcmd;
        zcmd->if_next=NULL;
        zcmd->else_next=NULL;
        break;
      case 2:
        CREATE(zcmd, struct zcmd_data, 1);
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->if_next) {
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->if_next->prev=zcmd;
        }
        zcmd->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->if_next;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->if_next=zcmd;
        zcmd->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=zcmd;
        GET_OLC_FIELD(ch)=50;
        zcmd->if_next=NULL;
        zcmd->else_next=NULL;
        break;
      case 3:
        CREATE(zcmd, struct zcmd_data, 1);
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->else_next)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->else_next->prev=zcmd;
        zcmd->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->else_next;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->else_next=zcmd;
        zcmd->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=zcmd;
        GET_OLC_FIELD(ch)=50;
        zcmd->if_next=NULL;
        zcmd->else_next=NULL;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 7:
    if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev) {
      if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev == ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->if_next) {
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->if_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev;
      }
      else if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev == ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->else_next) {
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->else_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev;
      }
      else {
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev;
      }
      ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=NULL;
      free_zcmd(((struct zcmd_data *)GET_OLC_PTR(ch))->prev);
      ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
    }
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  case 8:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      zbuf=str_dup("\r\n\nEdit command:\r\n");
      temp=zcmd->next;
      zcmd->next=NULL;
      print_zone_commands(zcmd, 1, ch, &zbuf);
      zcmd->next=temp;
      RECREATE(zbuf, char, strlen(zbuf)+1000);
      sprintf(line, "1. Change Type [%c]\r\n", zcmd->command);
      strcat(zbuf, line);
      sprintf(line, "2. Change Probability [%d]\r\n", zcmd->prob);
      strcat(zbuf, line);
      if(zcmd->command != 'D') {
        sprintf(line, "3. Change Limit [%d]\r\n", zcmd->arg2);
        strcat(zbuf, line);
      }
      strcat(zbuf, "0. Previous Menu\r\n");
      page_string(ch->desc, zbuf, 1);
      free(zbuf);
    }
    else {
      int ok=0;
      switch(cmd) {
      case 0:
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
        GET_OLC_FIELD(ch)=0;
        ok=1;
        break;
      case 1:
        GET_OLC_FIELD(ch)=100;
        ok=1;
        break;
      case 2:
        GET_OLC_FIELD(ch)=101;
        ok=1;
        break;
      case 3:
        if(zcmd->command != 'D') {
          GET_OLC_FIELD(ch)=102;
          ok=1;
        }
        break;
      }
      if(!ok)
        send_to_char("Invalid command.\r\n", ch);
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 9:
    if(cmd == OLC_MENU_NUM) {
      zbuf=str_dup("\r\n\nAfter which command do you want to place this command?\r\n");
      print_zone_command_list((struct zcmd_data *)GET_OLC_PTR(ch), 0, ch, &zbuf, 0);
      page_string(ch->desc, zbuf, 1);
      free(zbuf);
    }
    else {
      if((cmd < 0) || (cmd > count_zcmds(((struct zcmd_data *)GET_OLC_PTR(ch))->next))) {
        send_to_char("No such zone command.\r\n", ch);
      }
      else {
        i=0;
        if(cmd!=0)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next=get_zone_command_by_num(((struct zcmd_data *)GET_OLC_PTR(ch))->next, &i, cmd);
        else
          ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next=((struct zcmd_data *)GET_OLC_PTR(ch));
        GET_OLC_FIELD(ch)=20;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 10:
    if(cmd == OLC_MENU_NUM) {
      zbuf=str_dup("\r\n\nAfter which command do you want to place this command?\r\n");
      print_zone_command_list((struct zcmd_data *)GET_OLC_PTR(ch), 0, ch, &zbuf, 0);
      page_string(ch->desc, zbuf, 1);
      free(zbuf);
    }
    else {
      if((cmd < 0) || (cmd > count_zcmds(((struct zcmd_data *)GET_OLC_PTR(ch))->next))) {
        send_to_char("No such zone command.\r\n", ch);
      }
      else {
        i=0;
        if(cmd!=0)
          ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next=get_zone_command_by_num(((struct zcmd_data *)GET_OLC_PTR(ch))->next, &i, cmd);
        else
          ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next=((struct zcmd_data *)GET_OLC_PTR(ch));
        GET_OLC_FIELD(ch)=21;
        zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=NULL;
        temp=copy_zcmd(((struct zcmd_data *)GET_OLC_PTR(ch))->prev);
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=zcmd;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=temp;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 20:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\n1. No dependance on the previous command\r\n", ch);
      if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next != ((struct zcmd_data *)GET_OLC_PTR(ch))) {
        send_to_char("2. Depend on the previous command happening\r\n", ch);
        send_to_char("3. Depend on the previous command not happening\r\n", ch);
      }
    }
    else {
      if((((struct zcmd_data *)GET_OLC_PTR(ch))->if_next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev) &&
         (((cmd==1) && (((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev)) ||
          ((cmd==2) && (((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev)) ||
          ((cmd==3) && (((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev)))) {
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev && ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next) {
          if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev == ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->if_next) {
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->if_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev;
          }
          else if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev == ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->else_next) {
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->else_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev;
          }
          else {
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next;
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev;
          }
          if((((struct zcmd_data *)GET_OLC_PTR(ch))->if_next == ((struct zcmd_data *)GET_OLC_PTR(ch))) && (cmd > 1))
            cmd=5;
          switch(cmd) {
          case 1:
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next;
            break;
          case 2:
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next;
            break;
          case 3:
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next;
            break;
          default:
            send_to_char("Invalid command\r\n", ch);
            do_olcmenu(ch, "", 0, 0);
            return;
            break;
          }
        }
      }
      ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next=NULL;
      ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 21:
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\n1. No dependance on the previous command\r\n", ch);
      if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next != ((struct zcmd_data *)GET_OLC_PTR(ch))) {
        send_to_char("2. Depend on the previous command happening\r\n", ch);
        send_to_char("3. Depend on the previous command not happening\r\n", ch);
      }
    }
    else {
      if((((struct zcmd_data *)GET_OLC_PTR(ch))->if_next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev) &&
         (((cmd==1) && (((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev)) ||
          ((cmd==2) && (((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev)) ||
          ((cmd==3) && (((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next != ((struct zcmd_data *)GET_OLC_PTR(ch))->prev)))) {
        if(((struct zcmd_data *)GET_OLC_PTR(ch))->prev && ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next) {
          if((((struct zcmd_data *)GET_OLC_PTR(ch))->if_next == ((struct zcmd_data *)GET_OLC_PTR(ch))) && (cmd > 1))
            cmd=5;
          switch(cmd) {
          case 1:
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next;
            break;
          case 2:
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->if_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next;
            break;
          case 3:
            if(((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next)
              ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->next=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next->else_next=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
            ((struct zcmd_data *)GET_OLC_PTR(ch))->prev->prev=((struct zcmd_data *)GET_OLC_PTR(ch))->if_next;
            break;
          default:
            send_to_char("Invalid command\r\n", ch);
            do_olcmenu(ch, "", 0, 0);
            return;
            break;
          }
        }
      }
      ((struct zcmd_data *)GET_OLC_PTR(ch))->if_next=NULL;
      ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
      GET_OLC_FIELD(ch)=0;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 49: /* no depend menu */
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\n1. Load mob in room\r\n", ch);
      send_to_char("2. Load obj in room\r\n", ch);
      send_to_char("3. Set door state\r\n", ch);
    }
    else {
      switch(cmd) {
      case 1:
        zcmd->command='M';
        GET_OLC_FIELD(ch)=51;
        break;
      case 2:
        zcmd->command='O';
        GET_OLC_FIELD(ch)=52;
        break;
      case 3:
        zcmd->command='D';
        GET_OLC_FIELD(ch)=53;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 50: /* depend menu */
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\n1. Load mob in room\r\n", ch);
      send_to_char("2. Load obj in room\r\n", ch);
      send_to_char("3. Load obj in obj\r\n", ch);
      send_to_char("4. Load obj in inventory of mob\r\n", ch);
      send_to_char("5. Load obj equipped on mob\r\n", ch);
      send_to_char("6. Set door state\r\n", ch);
    }
    else {
      switch(cmd) {
      case 1:
        zcmd->command='M';
        GET_OLC_FIELD(ch)=51;
        break;
      case 2:
        zcmd->command='O';
        GET_OLC_FIELD(ch)=52;
        break;
      case 3:
        zcmd->command='P';
        GET_OLC_FIELD(ch)=52;
        break;
      case 4:
        zcmd->command='G';
        GET_OLC_FIELD(ch)=52;
        break;
      case 5:
        zcmd->command='E';
        GET_OLC_FIELD(ch)=52;
        break;
      case 6:
        zcmd->command='D';
        GET_OLC_FIELD(ch)=53;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 51:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the vnum of the mob to load\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_mobile(cmd) >= 0) {
        zcmd->arg1=real_mobile(cmd);
        GET_OLC_FIELD(ch)=54;
      }
      else {
        send_to_char("No such mob.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 52:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the vnum of the obj to load\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_object(cmd) >= 0) {
        zcmd->arg1=real_object(cmd);
        switch(zcmd->command) {
        case 'O':
          GET_OLC_FIELD(ch)=54;
          break;
        case 'P':
          GET_OLC_FIELD(ch)=55;
          break;
        case 'G':
          GET_OLC_FIELD(ch)=56;
          break;
        case 'E':
          GET_OLC_FIELD(ch)=56;
          break;
        default:
          send_to_char("ERROR\r\n", ch);
          GET_OLC_FIELD(ch)=0;
          break;
        }
      }
      else {
        send_to_char("No such object.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 53:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the vnum of the room in which to set an exit\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_room(cmd) >= 0) {
        for(i=0; i<=top_of_zone_table; i++) {
          if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
            break;
        }
        if((i > top_of_zone_table) || (zone_table[i].number != GET_OLC_NUM(ch))) {
          send_to_char("That room is outside of this zone!\r\n", ch);
        }
        else {
          zcmd->arg1=real_room(cmd);
          GET_OLC_FIELD(ch)=57;
        }
      }
      else {
        send_to_char("No such room.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 54:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nLoad in what room (vnum)?\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_room(cmd) >= 0) {
        for(i=0; i<=top_of_zone_table; i++) {
          if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
            break;
        }
        if((i > top_of_zone_table) || (zone_table[i].number != GET_OLC_NUM(ch))) {
          send_to_char("That room is outside of this zone!\r\n", ch);
        }
        else {
          zcmd->arg3=real_room(cmd);
          GET_OLC_FIELD(ch)=60;
        }
      }
      else {
        send_to_char("No such room.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 55:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nLoad in what object (vnum)?\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_object(cmd) >= 0) {
        zcmd->arg3=real_object(cmd);
        GET_OLC_FIELD(ch)=60;
      }
      else {
        send_to_char("No such object.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 56:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nLoad on what mob (vnum)?\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_mobile(cmd) >= 0) {
        zcmd->arg3=real_mobile(cmd);
        if(zcmd->command=='G')
          GET_OLC_FIELD(ch)=60;
        else
          GET_OLC_FIELD(ch)=58;
      }
      else {
        send_to_char("No such mob.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 57:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nSet the door on which exit?\r\n", ch);
      for(i=0; i<NUM_OF_DIRS; i++) {
        sprintf(line, "%2d. %s\r\n", i+1, dirs[i]);
        send_to_char(line, ch);
      }
    }
    else {
      if((cmd < 1) || (cmd > NUM_OF_DIRS)) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        zcmd->arg2=cmd-1;
        GET_OLC_FIELD(ch)=59;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 58:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEquip in which position?\r\n", ch);
      for(i=0; i<NUM_WEARS; i++) {
        sprintf(line, "%2d. %s\r\n", i+1, equipment_types[i]);
        send_to_char(line, ch);
      }
    }
    else {
      if((cmd < 1) || (cmd > NUM_WEARS)) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        zcmd->arg4=cmd-1;
        GET_OLC_FIELD(ch)=60;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 59:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nSet exit to what state?\r\n"
                    "1. Open\r\n"
                    "2. Closed\r\n"
                    "3. Closed and locked\r\n", ch);
    }
    else {
      switch(cmd) {
      case 1:
      case 2:
      case 3:
        zcmd->arg3=cmd-1;
        GET_OLC_FIELD(ch)=60;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 60:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("Probability (chance in 10000) (1 - 10000)\r\n", ch);
    }
    else {
      zcmd->prob=RANGE(1, 10000);
      if(zcmd->command == 'D') {
        GET_OLC_FIELD(ch)=0;
        ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
      }
      else if(zcmd->command == 'M')
        GET_OLC_FIELD(ch)=61;
      else
        GET_OLC_FIELD(ch)=62;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 61:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("Limit (number of copies of the mob will load from this command\r\n"
                   "       while the old mobs are still alive, usualy 1) (1 - 20)\r\n", ch);
    }
    else {
      zcmd->arg2=RANGE(1, 20);
      GET_OLC_FIELD(ch)=0;
      ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 62:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("Limit (Number of zone resets it will attempt to load this object,\r\n"
                   "       for instance, with a limit of 1 it will only have a chance\r\n"
                   "       to load when the mud reboots. With a limit of 2, it would\r\n"
                   "       have a chance to load when the mud rebooted next time the\r\n"
                   "       zone reset. A limit of -1 is no limit.) (-1 - 100)", ch);
    }
    else {
      zcmd->arg2=RANGE(-1, 100);
      GET_OLC_FIELD(ch)=0;
      ((struct zcmd_data *)GET_OLC_PTR(ch))->prev=NULL;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 99:
    send_to_char("Saving...\r\n", ch);
    save_edit(ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  case 100:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\n1. Load mob in room\r\n", ch);
      send_to_char("2. Load obj in room\r\n", ch);
      send_to_char("3. Load obj in obj\r\n", ch);
      send_to_char("4. Load obj in inventory of mob\r\n", ch);
      send_to_char("5. Load obj equipped on mob\r\n", ch);
      send_to_char("6. Set door state\r\n", ch);
    }
    else {
      switch(cmd) {
      case 1:
        zcmd->command='M';
        zcmd->arg2=MAX(1, zcmd->arg2);
        GET_OLC_FIELD(ch)=151;
        break;
      case 2:
        zcmd->command='O';
        GET_OLC_FIELD(ch)=152;
        break;
      case 3:
        zcmd->command='P';
        GET_OLC_FIELD(ch)=152;
        break;
      case 4:
        zcmd->command='G';
        GET_OLC_FIELD(ch)=152;
        break;
      case 5:
        zcmd->command='E';
        GET_OLC_FIELD(ch)=152;
        break;
      case 6:
        zcmd->command='D';
        GET_OLC_FIELD(ch)=153;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 101:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("Probability (chance in 10000) (1 - 10000)\r\n", ch);
    }
    else {
      zcmd->prob=RANGE(1, 10000);
      GET_OLC_FIELD(ch)=8;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 102:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(zcmd->command == 'M')
      GET_OLC_FIELD(ch)=161;
    else
      GET_OLC_FIELD(ch)=162;
    do_olcmenu(ch, "", 0, 0);
    break;
  case 151:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the vnum of the mob to load\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_mobile(cmd) >= 0) {
        zcmd->arg1=real_mobile(cmd);
        GET_OLC_FIELD(ch)=154;
      }
      else {
        send_to_char("No such mob.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 152:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the vnum of the obj to load\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_object(cmd) >= 0) {
        zcmd->arg1=real_object(cmd);
        switch(zcmd->command) {
        case 'O':
          GET_OLC_FIELD(ch)=154;
          break;
        case 'P':
          GET_OLC_FIELD(ch)=155;
          break;
        case 'G':
          GET_OLC_FIELD(ch)=156;
          break;
        case 'E':
          GET_OLC_FIELD(ch)=156;
          break;
        default:
          send_to_char("ERROR\r\n", ch);
          GET_OLC_FIELD(ch)=0;
          break;
        }
      }
      else {
        send_to_char("No such object.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 153:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEnter the vnum of the room in which to set an exit\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_room(cmd) >= 0) {
        for(i=0; i<=top_of_zone_table; i++) {
          if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
            break;
        }
        if((i > top_of_zone_table) || (zone_table[i].number != GET_OLC_NUM(ch))) {
          send_to_char("That room is outside of this zone!\r\n", ch);
        }
        else {
          zcmd->arg1=real_room(cmd);
          GET_OLC_FIELD(ch)=157;
        }
      }
      else {
        send_to_char("No such room.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 154:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nLoad in what room (vnum)?\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_room(cmd) >= 0) {
        for(i=0; i<=top_of_zone_table; i++) {
          if((cmd >= zone_table[i].bottom) && (cmd <= zone_table[i].top))
            break;
        }
        if((i > top_of_zone_table) || (zone_table[i].number != GET_OLC_NUM(ch))) {
          send_to_char("That room is outside of this zone!\r\n", ch);
        }
        else {
          zcmd->arg3=real_room(cmd);
          GET_OLC_FIELD(ch)=8;
        }
      }
      else {
        send_to_char("No such room.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 155:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nLoad in what object (vnum)?\r\n", ch);    }
    else {
      cmd=RANGE(0, 99999);
      if(real_object(cmd) >= 0) {
        zcmd->arg3=real_object(cmd);
        GET_OLC_FIELD(ch)=8;
      }
      else {
        send_to_char("No such object.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 156:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nLoad on what mob (vnum)?\r\n", ch);
    }
    else {
      cmd=RANGE(0, 99999);
      if(real_mobile(cmd) >= 0) {
        zcmd->arg3=real_mobile(cmd);
        if(zcmd->command=='G')
          GET_OLC_FIELD(ch)=8;
        else
          GET_OLC_FIELD(ch)=158;
      }
      else {
        send_to_char("No such mob.\r\n", ch);
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 157:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nSet the door on which exit?\r\n", ch);
      for(i=0; i<NUM_OF_DIRS; i++) {
        sprintf(line, "%2d. %s\r\n", i+1, dirs[i]);
        send_to_char(line, ch);
      }
    }
    else {
      if((cmd < 1) || (cmd > NUM_OF_DIRS)) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        zcmd->arg2=cmd-1;
        GET_OLC_FIELD(ch)=159;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 158:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nEquip in which position?\r\n", ch);
      for(i=0; i<NUM_WEARS; i++) {
        sprintf(line, "%2d. %s\r\n", i+1, equipment_types[i]);
        send_to_char(line, ch);
      }
    }
    else {
      if((cmd < 1) || (cmd > NUM_WEARS)) {
        send_to_char("Invalid choice.\r\n", ch);
      }
      else {
        zcmd->arg4=cmd-1;
        GET_OLC_FIELD(ch)=8;      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 159:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;    if(cmd == OLC_MENU_NUM) {
      send_to_char("\r\n\nSet exit to what state?\r\n"
                    "1. Open\r\n"
                    "2. Closed\r\n"
                    "3. Closed and locked\r\n", ch);
    }
    else {
      switch(cmd) {
      case 1:
      case 2:
      case 3:
        zcmd->arg3=cmd-1;
        GET_OLC_FIELD(ch)=8;
        break;
      default:
        send_to_char("Invalid command.\r\n", ch);
        break;
      }
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 161:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("Limit (number of copies of the mob will load from this command\r\n"
                   "       while the old mobs are still alive, usualy 1) (1 - 20)\r\n", ch);
    }
    else {
      zcmd->arg2=RANGE(1, 20);
      GET_OLC_FIELD(ch)=8;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  case 162:
    zcmd=((struct zcmd_data *)GET_OLC_PTR(ch))->prev;
    if(cmd == OLC_MENU_NUM) {
      send_to_char("Limit (Number of zone resets it will attempt to load this object,\r\n"
                   "       for instance, with a limit of 1 it will only have a chance\r\n"
                   "       to load when the mud reboots. With a limit of 2, it would\r\n"
                   "       have a chance to load when the mud rebooted next time the\r\n"
                   "       zone reset. A limit of -1 is no limit.) (-1 - 100)", ch);
    }
    else {
      zcmd->arg2=RANGE(-1, 100);
      GET_OLC_FIELD(ch)=8;
      do_olcmenu(ch, "", 0, 0);
    }
    break;
  default:
    send_to_char("Incomplete field.\r\n", ch);
    GET_OLC_FIELD(ch)=0;
    do_olcmenu(ch, "", 0, 0);
    break;
  }
}
void dynamic_zedit_mob_fix(struct zcmd_data *zcmd, int i)
{
  if(!zcmd)
    return;
  switch (zcmd->command) {
  case 'M':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    break;
  case 'G':
    if(zcmd->arg3 >= i)
      zcmd->arg3++;
    break;
  case 'E':
    if(zcmd->arg3 >= i)
      zcmd->arg3++;
    break;
  }
  dynamic_zedit_mob_fix(zcmd->if_next, i);
  dynamic_zedit_mob_fix(zcmd->else_next, i);
  dynamic_zedit_mob_fix(zcmd->next, i);
}
void dynamic_zedit_obj_fix(struct zcmd_data *zcmd, int i)
{
  if(!zcmd)
    return;
  switch (zcmd->command) {
  case 'O':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    break;
  case 'G':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    break;
  case 'E':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    break;
  case 'P':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    if(zcmd->arg3 >= i)
      zcmd->arg3++;
    break;
  case 'R':
    if(zcmd->arg2 >= i)
      zcmd->arg2++;
    break;
  }
  dynamic_zedit_obj_fix(zcmd->if_next, i);
  dynamic_zedit_obj_fix(zcmd->else_next, i);
  dynamic_zedit_obj_fix(zcmd->next, i);
}
void dynamic_zedit_room_fix(struct zcmd_data *zcmd, int i)
{
  if(!zcmd)
    return;
  switch (zcmd->command) {
  case 'M':
    if(zcmd->arg3 >= i)
      zcmd->arg3++;
    break;
  case 'O':
    if(zcmd->arg3 >= i)
      zcmd->arg3++;
    break;
  case 'D':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    break;
  case 'R':
    if(zcmd->arg1 >= i)
      zcmd->arg1++;
    break;
  }
  dynamic_zedit_room_fix(zcmd->if_next, i);
  dynamic_zedit_room_fix(zcmd->else_next, i);
  dynamic_zedit_room_fix(zcmd->next, i);
}
int make_zone_commands(struct reset_com *cmd, struct zcmd_data *zcmd, int line)
{
  struct zcmd_data *temp;
  if(zcmd) {
    cmd[line].command=zcmd->command;
    cmd[line].arg1=zcmd->arg1;
    cmd[line].limit=cmd[line].arg2=zcmd->arg2;
    cmd[line].arg3=zcmd->arg3;
    cmd[line].arg4=zcmd->arg4;
    cmd[line].line=zcmd->line=line;
    cmd[line].prob=zcmd->prob;
    cmd[line].ok=0;
    temp=zcmd;
    while(temp->prev) {
      if(temp==temp->prev->if_next) {
        cmd[line].depend=temp->prev->line;
        cmd[line].if_flag=1;
        break;
      }
      else if(temp==temp->prev->else_next) {
        cmd[line].depend=temp->prev->line;
        cmd[line].if_flag=-1;
        break;
      }
      else {
        temp=temp->prev;
      }
    }
    if(!temp->prev) {
      cmd[line].depend=0;
      cmd[line].if_flag=0;
    }
    line++;
    line=make_zone_commands(cmd, zcmd->if_next, line);
    line=make_zone_commands(cmd, zcmd->else_next, line);
    line=make_zone_commands(cmd, zcmd->next, line);
  }
  return(line);
}
#define ZCMD zone_table[zone].cmd[cmd_no]
/* resulve vnums into rnums in the zone reset tables */
void re_renum_zone_table(int rlimit, int mlimit, int olimit)
{
  int zone, cmd_no;
  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      switch (ZCMD.command) {
      case 'M':
	if(ZCMD.arg1 >= mlimit)
          ZCMD.arg1++;
	if(ZCMD.arg3 >= rlimit)
          ZCMD.arg3++;
	break;
      case 'O':
	if(ZCMD.arg1 >= olimit)
          ZCMD.arg1++;
	if(ZCMD.arg3 >= rlimit)
	  ZCMD.arg3++;
	break;
      case 'G':
	if(ZCMD.arg1 >= olimit)
          ZCMD.arg1++;
        if(ZCMD.arg3 >=mlimit)
          ZCMD.arg3++;
	break;
      case 'E':
	if(ZCMD.arg1 >= olimit)
          ZCMD.arg1++;
        if(ZCMD.arg3 >= mlimit)
          ZCMD.arg3++;
	break;
      case 'P':
	if(ZCMD.arg1 >= olimit)
          ZCMD.arg1++;
	if(ZCMD.arg3 >= olimit)
          ZCMD.arg3++;
	break;
      case 'D':
	if(ZCMD.arg1 >= rlimit)
          ZCMD.arg1++;
	break;
      case 'R':
        if(ZCMD.arg1 >= rlimit)
          ZCMD.arg1++;
	if(ZCMD.arg2 >= olimit)
          ZCMD.arg2++;
        break;
      }
    }
}
void save_edit(struct char_data *ch)
{
  int i, j, k, nr, exit_info, to_room, count, arg1, arg3, dir, shop_nr, index;
  struct char_data *tch;
  struct obj_data *tobj;
  struct extra_descr_data *ex, *tex;
  struct zcmd_data *tempz;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  FILE *fp;
  extern sh_int r_mortal_start_room;
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;
  switch(GET_OLC_MODE(ch)) {
  case OLC_MEDIT:
    if(real_mobile(GET_OLC_NUM(ch)) >= 0) {
      sprintf(buf, "(GC) %s has editted mob %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      GET_MOB_RNUM((struct char_data *)GET_OLC_PTR(ch))=real_mobile(GET_OLC_NUM(ch));
      free(mob_proto[real_mobile(GET_OLC_NUM(ch))].char_specials.prompt);
      free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.name);
      free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.short_descr);
      free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.long_descr);
      free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.description);
      free(ACTIONS(mob_proto+real_mobile(GET_OLC_NUM(ch))));
      copy_mob(mob_proto+real_mobile(GET_OLC_NUM(ch)), (struct char_data *)GET_OLC_PTR(ch));
      for(tch=character_list; tch; tch=tch->next) {
        if(IS_NPC(tch) && (GET_MOB_RNUM(tch)==GET_MOB_RNUM((struct char_data *)GET_OLC_PTR(ch)))) {
          tch->char_specials.prompt=mob_proto[real_mobile(GET_OLC_NUM(ch))].char_specials.prompt;
          tch->player.name=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.name;
          tch->player.short_descr=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.short_descr;
          tch->player.long_descr=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.long_descr;
          tch->player.description=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.description;
          ACTIONS(tch)=ACTIONS(mob_proto+real_mobile(GET_OLC_NUM(ch)));
        }
      }
      if((!MOB_FLAGGED((struct char_data *)GET_OLC_PTR(ch), MOB_SPEC)) || (((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc > 0)) {
        if(mob_index[i=real_mobile(GET_OLC_NUM(ch))].func == shop_keeper) {
          for(j=0; j<top_shop; j++) {
            if(SHOP_KEEPER(j)==i)
              SHOP_FUNC(j)=spec_proc_table[((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc];
          }
        }
        else
          mob_index[real_mobile(GET_OLC_NUM(ch))].func=spec_proc_table[((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc];
      }
    }
    else {
      sprintf(buf, "(GC) %s has created mob %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      for(i=0; i<=top_of_mobt; i++)
        if(mob_index[i].virtual > GET_OLC_NUM(ch))
          break;
      GET_MOB_RNUM((struct char_data *)GET_OLC_PTR(ch))=i;
      top_of_mobt++;
      RECREATE(mob_proto, struct char_data, top_of_mobt+1);
      RECREATE(mob_index, struct index_data, top_of_mobt+1);
      for(j=top_of_mobt; j>i; j--) {
        mob_index[j]=mob_index[j-1];
        mob_proto[j]=mob_proto[j-1];
        GET_MOB_RNUM(mob_proto+j)++;
      }
      mob_index[i].virtual=GET_OLC_NUM(ch);
      mob_index[i].number=0;
      mob_index[i].progtypes=0;
      mob_index[i].mobprogs=NULL;
      mob_index[i].func=spec_proc_table[((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc];
      copy_mob(mob_proto+i, (struct char_data *)GET_OLC_PTR(ch));
      for(j=0; j<top_shop; j++) {
        if(SHOP_KEEPER(j) >= i)
          SHOP_KEEPER(j)++;
      }
      for(tch=character_list; tch; tch=tch->next) {
        if(IS_NPC(tch)) {
          if(GET_MOB_RNUM(tch) >= i) {
            GET_MOB_RNUM(tch)++;
          }
        }
        else {
          if(GET_OLC_MODE(tch)==OLC_ZEDIT) {
            dynamic_zedit_mob_fix(GET_OLC_PTR(tch), i);
          }
        }
      }
      re_renum_zone_table(top_of_world+1, i, top_of_objt+1);
    }
    for(j=0; j<=top_of_zone_table; j++) {
      if((GET_OLC_NUM(ch) >= zone_table[j].bottom) && (GET_OLC_NUM(ch) <= zone_table[j].top))
        break;
    }
    if(j > top_of_zone_table) {
      send_to_char("Error: mob is not in any zone, not saved.\r\n", ch);
    }
    else {
      sprintf(fname, "world/mob/%d.mob", zone_table[j].number);
      if(!(fp=fopen(fname, "w"))) {
        send_to_char("Error opening file, not saved.\r\n", ch);
      }
      else {
        for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
          if((nr=real_mobile(i)) >= 0) {
            sprintf(buf, "#%d\n", i);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", mob_proto[nr].player.name);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", mob_proto[nr].player.short_descr);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", mob_proto[nr].player.long_descr);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", mob_proto[nr].player.description);
            fputs(buf, fp);
            sprintf(buf, "%lld %lld %d W3\n", MOB_FLAGS(mob_proto+nr), AFF_FLAGS(mob_proto+nr), GET_ALIGNMENT(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d %dd%d+%d %dd%d+%d\n", GET_LEVEL(mob_proto+nr), GET_HITROLL(mob_proto+nr),
                    GET_AC(mob_proto+nr)/10, GET_HIT(mob_proto+nr), GET_MANA(mob_proto+nr), GET_MOVE(mob_proto+nr),
                    mob_proto[nr].mob_specials.damnodice, mob_proto[nr].mob_specials.damsizedice, GET_DAMROLL(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%ld %ld\n", GET_GOLD(mob_proto+nr), GET_EXP(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d\n", GET_POS(mob_proto+nr), GET_DEFAULT_POS(mob_proto+nr), GET_SEX(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d %d %d %d %d\n", mob_proto[nr].real_abils.str, mob_proto[nr].real_abils.str_add,
                    mob_proto[nr].real_abils.intel, mob_proto[nr].real_abils.wis, mob_proto[nr].real_abils.dex,
                    mob_proto[nr].real_abils.con, mob_proto[nr].real_abils.cha);
            fputs(buf, fp);
            sprintf(buf, "%d %d %d\n", mob_proto[nr].mob_specials.spec_proc, mob_proto[nr].mob_specials.size, GET_CLASS(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d %d %d %d\n", GET_AC(mob_proto+nr), mob_proto[nr].mob_specials.attack_type,
                    mob_proto[nr].mob_specials.attacks, mob_proto[nr].char_specials.hp_regen_add, GET_MR(mob_proto+nr), GET_PR(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d %d %d\n", GET_SAVE(mob_proto+nr, 0), GET_SAVE(mob_proto+nr, 1),
                    GET_SAVE(mob_proto+nr, 2), GET_SAVE(mob_proto+nr, 3), GET_SAVE(mob_proto+nr, 4));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d\n", IMMUNE_FLAGS(mob_proto+nr), WEAK_FLAGS(mob_proto+nr), RESIST_FLAGS(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%d %d\n", GET_MOVE_RATE(mob_proto+nr), GET_FEROCITY(mob_proto+nr));
            fputs(buf, fp);
            sprintf(buf, "%s~\n", (ACTIONS(mob_proto+nr)?ACTIONS(mob_proto+nr):""));
            fputs(buf, fp);
          }
        }
        fputs("$\n", fp);
        fclose(fp);
      }
    }
    break;
  case OLC_OEDIT:
    if(real_object(GET_OLC_NUM(ch)) >= 0) {
      sprintf(buf, "(GC) %s has editted obj %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      GET_OBJ_RNUM((struct obj_data *)GET_OLC_PTR(ch))=real_object(GET_OLC_NUM(ch));
      free(obj_proto[real_object(GET_OLC_NUM(ch))].name);
      free(obj_proto[real_object(GET_OLC_NUM(ch))].description);
      free(obj_proto[real_object(GET_OLC_NUM(ch))].short_description);
      free(obj_proto[real_object(GET_OLC_NUM(ch))].action_description);
      for(ex=obj_proto[real_object(GET_OLC_NUM(ch))].ex_description; ex; ex=tex) {
        tex=ex->next;
        free(ex->keyword);
        free(ex->description);
        free(ex);
      }
      if(GET_OBJ_TYPE((struct obj_data *)GET_OLC_PTR(ch))==ITEM_DAMAGEABLE)
        obj_index[real_object(GET_OLC_NUM(ch))].func=damage_eq;
      copy_obj(obj_proto+real_object(GET_OLC_NUM(ch)), (struct obj_data *)GET_OLC_PTR(ch));
      for(tobj=object_list; tobj; tobj=tobj->next) {
        if(GET_OBJ_RNUM(tobj)==GET_OBJ_RNUM((struct obj_data *)GET_OLC_PTR(ch))) {
          tobj->name=obj_proto[real_object(GET_OLC_NUM(ch))].name;
          tobj->description=obj_proto[real_object(GET_OLC_NUM(ch))].description;
          tobj->short_description=obj_proto[real_object(GET_OLC_NUM(ch))].short_description;
          tobj->action_description=obj_proto[real_object(GET_OLC_NUM(ch))].action_description;
          tobj->ex_description=obj_proto[real_object(GET_OLC_NUM(ch))].ex_description;
        }
      }
    }
    else {
      sprintf(buf, "(GC) %s has created obj %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      for(i=0; i<=top_of_objt; i++)
        if(obj_index[i].virtual > GET_OLC_NUM(ch))
          break;
      GET_OBJ_RNUM((struct obj_data *)GET_OLC_PTR(ch))=i;
      top_of_objt++;
      RECREATE(obj_index, struct index_data, top_of_objt+1);
      RECREATE(obj_proto, struct obj_data, top_of_objt+1);
      for(j=top_of_objt; j>i; j--) {
        obj_index[j]=obj_index[j-1];
        obj_proto[j]=obj_proto[j-1];
        GET_OBJ_RNUM(obj_proto+j)++;
      }
      obj_index[i].virtual=GET_OLC_NUM(ch);
      obj_index[i].number=0;
      obj_index[i].progtypes=0;
      obj_index[i].mobprogs=NULL;
      if(GET_OBJ_TYPE((struct obj_data *)GET_OLC_PTR(ch))==ITEM_DAMAGEABLE)
        obj_index[i].func=damage_eq;
      else
        obj_index[i].func=NULL;
      copy_obj(obj_proto+i, (struct obj_data *)GET_OLC_PTR(ch));
      for (j = 0; j < NUM_OF_BOARDS; j++)
        if (BOARD_RNUM(j) >= i)
          BOARD_RNUM(j)++;
      for(tobj=object_list; tobj; tobj=tobj->next) {
        if(GET_OBJ_RNUM(tobj) >= i) {
          GET_OBJ_RNUM(tobj)++;
        }
      }
      for(tch=character_list; tch; tch=tch->next) {
        if(!IS_NPC(tch)) {
          if(GET_OLC_MODE(tch)==OLC_ZEDIT) {
            dynamic_zedit_obj_fix(GET_OLC_PTR(tch), i);
          }
        }
      }
      for(shop_nr=0; shop_nr<top_shop; shop_nr++) {
        for(index = 0; SHOP_PRODUCT(shop_nr, index) != NOTHING; index++) {
          if(SHOP_PRODUCT(shop_nr, index) >= i)
            SHOP_PRODUCT(shop_nr, index)++;
        }
      }
      re_renum_zone_table(top_of_world+1, top_of_mobt+1, i);
    }
    for(j=0; j<=top_of_zone_table; j++) {
      if((GET_OLC_NUM(ch) >= zone_table[j].bottom) && (GET_OLC_NUM(ch) <= zone_table[j].top))
        break;
    }
    if(j > top_of_zone_table) {
      send_to_char("Error: obj is not in any zone, not saved.\r\n", ch);
    }
    else {
      sprintf(fname, "world/obj/%d.obj", zone_table[j].number);
      if(!(fp=fopen(fname, "w"))) {
        send_to_char("Error opening file, not saved.\r\n", ch);
      }
      else {
        for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
          if((nr=real_object(i)) >= 0) {
            sprintf(buf, "#%d\n", i);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", obj_proto[nr].name);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", obj_proto[nr].short_description);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", obj_proto[nr].description);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", obj_proto[nr].action_description);
            fputs(buf, fp);
            sprintf(buf, "%d %ld %ld %lld\n", GET_OBJ_TYPE(obj_proto+nr), GET_OBJ_EXTRA(obj_proto+nr),
                   GET_OBJ_WEAR(obj_proto+nr), obj_proto[nr].obj_flags.bitvector);
            fputs(buf, fp);
            sprintf(buf, "%d %d %d\n", obj_proto[nr].obj_flags.immune, obj_proto[nr].obj_flags.weak, obj_proto[nr].obj_flags.resist);
            fputs(buf, fp);
            sprintf(buf, "%d %d %d %d\n", GET_OBJ_VAL(obj_proto+nr, 0), GET_OBJ_VAL(obj_proto+nr, 1),
                    GET_OBJ_VAL(obj_proto+nr, 2), GET_OBJ_VAL(obj_proto+nr, 3));
            fputs(buf, fp);
            sprintf(buf, "%d %d %d\n", GET_OBJ_WEIGHT(obj_proto+nr), GET_OBJ_COST(obj_proto+nr), GET_OBJ_RENT(obj_proto+nr));
            fputs(buf, fp);
            for(k=0; k<MAX_OBJ_AFFECT; k++) {
              if(obj_proto[nr].affected[k].location != APPLY_NONE) {
                fputs("A\n", fp);
                sprintf(buf, "%d %d\n", obj_proto[nr].affected[k].location, obj_proto[nr].affected[k].modifier);
                fputs(buf, fp);
              }
            }
            for(ex=obj_proto[nr].ex_description; ex; ex=ex->next) {
              fputs("E\n", fp);
              sprintf(buf, "%s~\n", ex->keyword);
              fputs(buf, fp);
              sprintf(buf, "%s~\n", ex->description);
              fputs(buf, fp);
            }
          }
        }
        fputs("$\n", fp);
        fclose(fp);
      }
    }
    break;
  case OLC_REDIT:
    ((struct room_data *)GET_OLC_PTR(ch))->number=GET_OLC_NUM(ch);
    if(real_room(GET_OLC_NUM(ch)) >= 0) {
      sprintf(buf, "(GC) %s has editted room %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      free_partial_room(world+real_room(GET_OLC_NUM(ch)));
      copy_room(world+real_room(GET_OLC_NUM(ch)), (struct room_data *)GET_OLC_PTR(ch));
    }
    else {
      sprintf(buf, "(GC) %s has created room %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      for(i=0; i<=top_of_world; i++)
        if(world[i].number > GET_OLC_NUM(ch))
          break;
      for(j=0; j<=top_of_world; j++) {
        for(dir=0; dir<NUM_OF_DIRS; dir++) {
          if(world[j].dir_option[dir])
            if(world[j].dir_option[dir]->to_room >= i)
              world[j].dir_option[dir]->to_room++;
        }
      }
      top_of_world++;
      RECREATE(world, struct room_data, top_of_world+1);
      for(j=top_of_world; j>i; j--) {
        world[j]=world[j-1];
      }
      copy_room(world+i, (struct room_data *)GET_OLC_PTR(ch));
      world[i].light=0;
      world[i].contents=NULL;
      world[i].people=NULL;
      for(dir=0; dir<NUM_OF_DIRS; dir++) {
        if(world[i].dir_option[dir])
          if(world[i].dir_option[dir]->to_room > NOWHERE) {
            ((struct room_data *)GET_OLC_PTR(ch))->dir_option[dir]->to_room=world[i].dir_option[dir]->to_room=real_room(world[i].dir_option[dir]->to_room);
          }
      }
      for(tch=character_list; tch; tch=tch->next) {
        if(tch->in_room >= i)
          tch->in_room++;
        if(tch->was_in_room >= i)
          tch->was_in_room++;
        if(!IS_NPC(tch)) {
          if(GET_OLC_MODE(tch)==OLC_ZEDIT) {
            dynamic_zedit_room_fix(GET_OLC_PTR(tch), i);
          }
        }
      }
      for(tobj=object_list; tobj; tobj=tobj->next)
        if(tobj->in_room >= i)
          tobj->in_room++;
      if(r_mortal_start_room >= i)
        r_mortal_start_room++;
      if(r_immort_start_room >= i)
        r_immort_start_room++;
      if(r_frozen_start_room >= i)
        r_frozen_start_room++;
      re_renum_zone_table(i, top_of_mobt+1, top_of_objt+1);
    }
    for(j=0; j<=top_of_zone_table; j++) {
      if((GET_OLC_NUM(ch) >= zone_table[j].bottom) && (GET_OLC_NUM(ch) <= zone_table[j].top))
        break;
    }
    if(j > top_of_zone_table) {
      send_to_char("Error: obj is not in any zone, not saved.\r\n", ch);
    }
    else {
      sprintf(fname, "world/wld/%d.wld", zone_table[j].number);
      if(!(fp=fopen(fname, "w"))) {
        send_to_char("Error opening file, not saved.\r\n", ch);
      }
      else {
        for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
          if((nr=real_room(i)) >= 0) {
            sprintf(buf, "#%d\n", i);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", world[nr].name);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", world[nr].description);
            fputs(buf, fp);
            sprintf(buf, "%d %ld %d\n", zone_table[j].number, world[nr].room_flags, world[nr].sector_type);
            fputs(buf, fp);
            sprintf(buf, "%dd%d+%d %d %d\n", world[nr].dt_numdice, world[nr].dt_sizedice,
                    world[nr].dt_add, world[nr].dt_percent, world[nr].dt_repeat);
            fputs(buf, fp);
            world[nr].zone=j;
            for(k=0; k<NUM_OF_DIRS; k++) {
              if(world[nr].dir_option[k]) {
                sprintf(buf, "D%d\n", k);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", world[nr].dir_option[k]->general_description);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", world[nr].dir_option[k]->keyword);
                fputs(buf, fp);
                if(IS_SET(world[nr].dir_option[k]->exit_info, EX_ISDOOR)) {
                  if(IS_SET(world[nr].dir_option[k]->exit_info, EX_PICKPROOF))
                    exit_info=2;
                  else
                    exit_info=1;
                  if(IS_SET(world[nr].dir_option[k]->exit_info, EX_SECRET)) {
                    exit_info+=2;
                    if(IS_SET(world[nr].dir_option[k]->exit_info, EX_HIDDEN))
                      exit_info+=2;
                  }
                }
                else
                  exit_info=0;
                to_room=world[nr].dir_option[k]->to_room;
                if(to_room > 0)
                  to_room=world[to_room].number;
                sprintf(buf, "%d %d %d\n", exit_info, world[nr].dir_option[k]->key, to_room);
                fputs(buf, fp);
              }
            }
            for(ex=world[nr].ex_description; ex; ex=ex->next) {
              fputs("E\n", fp);
              sprintf(buf, "%s~\n", ex->keyword);
              fputs(buf, fp);
              sprintf(buf, "%s~\n", ex->description);
              fputs(buf, fp);
            }
            fputs("S\n", fp);
          }
        }
        fputs("$\n", fp);
        fclose(fp);
      }
    }
    break;
  case OLC_ZEDIT:
    if((j=real_zone(GET_OLC_NUM(ch))) < 0) {
      send_to_char("Error: that zone does not exist, not saved.\r\n", ch);
    }
    else {
      sprintf(buf, "(GC) %s has editted zone %ld", GET_NAME(ch), GET_OLC_NUM(ch));
      mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
      tempz=(struct zcmd_data *)GET_OLC_PTR(ch);
      tempz->line=-1;
      tempz=tempz->next;
      count=count_zcmds(tempz);
      RECREATE(zone_table[j].cmd, struct reset_com, count+1);
      make_zone_commands(zone_table[j].cmd, tempz, 0);
      zone_table[j].cmd[count].command='S';
      sprintf(fname, "world/zon/%d.zon", zone_table[j].number);
      if(!(fp=fopen(fname, "w"))) {
        send_to_char("Error opening file, not saved.\r\n", ch);
      }
      else {
        sprintf(buf, "#%d\n", zone_table[j].number);
        fputs(buf, fp);
        sprintf(buf, "%s~\n", zone_table[j].name);
        fputs(buf, fp);
        sprintf(buf, "%d %d %d %d %d\n", zone_table[j].bottom, zone_table[j].top,
                zone_table[j].lifespan, zone_table[j].reset_mode, zone_table[j].closed);
        fputs(buf, fp);
        for(i=0; i<count; i++) {
          arg1=zone_table[j].cmd[i].arg1;
          arg3=zone_table[j].cmd[i].arg3;
          switch (zone_table[j].cmd[i].command) {
          case 'M':
            arg1 = mob_index[zone_table[j].cmd[i].arg1].virtual;
            arg3 = world[zone_table[j].cmd[i].arg3].number;
            break;
          case 'O':
            arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
            if (zone_table[j].cmd[i].arg3 != NOWHERE)
              arg3 = world[zone_table[j].cmd[i].arg3].number;
            break;
          case 'G':
            arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
            arg3 = mob_index[zone_table[j].cmd[i].arg3].virtual;
            break;
          case 'E':
            arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
            arg3 = mob_index[zone_table[j].cmd[i].arg3].virtual;
            break;
          case 'P':
            arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
            arg3 = obj_index[zone_table[j].cmd[i].arg3].virtual;
            break;
          case 'D':
            arg1 = world[zone_table[j].cmd[i].arg1].number;
            break;
          case 'R': /* rem obj from room */
            arg1 = world[zone_table[j].cmd[i].arg1].number;
            break;
          }
          if(zone_table[j].cmd[i].command=='R')
            sprintf(buf, "%c %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                    zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                    arg1, obj_index[zone_table[j].cmd[i].arg2].virtual,
                    zone_table[j].cmd[i].prob);
          else if(zone_table[j].cmd[i].command=='E')
            sprintf(buf, "%c %d %d %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                    zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                    arg1, zone_table[j].cmd[i].arg2,
                    arg3, zone_table[j].cmd[i].arg4,
                    zone_table[j].cmd[i].prob);
          else
            sprintf(buf, "%c %d %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                    zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                    arg1, zone_table[j].cmd[i].arg2,
                    arg3, zone_table[j].cmd[i].prob);
          fputs(buf, fp);
        }
        fputs("S\n", fp);
        fputs("$\n", fp);
        fclose(fp);
      }
      do_zpurge(ch, itoa(zone_table[j].number), 0, 1);
      do_zreset(ch, itoa(zone_table[j].number), 0, 1);
    }
    break;
  }
}
void confirm_edit(struct char_data *ch, int cmd)
{
  int i, j, k, nr, exit_info, to_room, count, arg1, arg3, dir, shop_nr, index;
  struct char_data *tch;
  struct obj_data *tobj;
  struct extra_descr_data *ex, *tex;
  struct zcmd_data *tempz;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  FILE *fp;
  extern sh_int r_mortal_start_room;
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;
  if((cmd != 1) && (cmd != 0)) {
    if(GET_OLC_FIELD(ch) == -1)
      send_to_char("Do you realy want to quit editting?\r\n0. No\r\n1. Yes\r\n", ch);
    else
      send_to_char("Save changes?\r\n0. No\r\n1. Yes\r\n", ch);
  }
  else {
    if(GET_OLC_FIELD(ch) == -1) {
      if(cmd == 1) {
        GET_OLC_FIELD(ch) = -2;
        if(GET_OLC_MODE(ch)==OLC_IEDIT) {
          do_olcmenu(ch, "1", 0, 0);
          return;
        }
        send_to_char("Save changes?\r\n0. No\r\n1. Yes\r\n", ch);
      }
      else {
        GET_OLC_FIELD(ch)=0;
        do_olcmenu(ch, "", 0, 0);
      }
      return;
    }
    switch(GET_OLC_MODE(ch)) {
    case OLC_MEDIT:
      if(cmd == 1) {
        if(real_mobile(GET_OLC_NUM(ch)) >= 0) {
          sprintf(buf, "(GC) %s has editted mob %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          GET_MOB_RNUM((struct char_data *)GET_OLC_PTR(ch))=real_mobile(GET_OLC_NUM(ch));
          free(mob_proto[real_mobile(GET_OLC_NUM(ch))].char_specials.prompt);
          free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.name);
          free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.short_descr);
          free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.long_descr);
          free(mob_proto[real_mobile(GET_OLC_NUM(ch))].player.description);
          free(ACTIONS(mob_proto+real_mobile(GET_OLC_NUM(ch))));
          copy_mob(mob_proto+real_mobile(GET_OLC_NUM(ch)), (struct char_data *)GET_OLC_PTR(ch));
          for(tch=character_list; tch; tch=tch->next) {
            if(IS_NPC(tch) && (GET_MOB_RNUM(tch)==GET_MOB_RNUM((struct char_data *)GET_OLC_PTR(ch)))) {
              tch->char_specials.prompt=mob_proto[real_mobile(GET_OLC_NUM(ch))].char_specials.prompt;
              tch->player.name=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.name;
              tch->player.short_descr=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.short_descr;
              tch->player.long_descr=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.long_descr;
              tch->player.description=mob_proto[real_mobile(GET_OLC_NUM(ch))].player.description;
              ACTIONS(tch)=ACTIONS(mob_proto+real_mobile(GET_OLC_NUM(ch)));
            }
          }
          if((!MOB_FLAGGED((struct char_data *)GET_OLC_PTR(ch), MOB_SPEC)) || (((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc > 0)) {
            if(mob_index[i=real_mobile(GET_OLC_NUM(ch))].func == shop_keeper) {
              for(j=0; j<top_shop; j++) {
                if(SHOP_KEEPER(j)==i)
                  SHOP_FUNC(j)=spec_proc_table[((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc];
              }
            }
            else
              mob_index[real_mobile(GET_OLC_NUM(ch))].func=spec_proc_table[((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc];
          }
        }
        else {
          sprintf(buf, "(GC) %s has created mob %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          for(i=0; i<=top_of_mobt; i++)
            if(mob_index[i].virtual > GET_OLC_NUM(ch))
              break;
          GET_MOB_RNUM((struct char_data *)GET_OLC_PTR(ch))=i;
          top_of_mobt++;
          RECREATE(mob_proto, struct char_data, top_of_mobt+1);
          RECREATE(mob_index, struct index_data, top_of_mobt+1);
          for(j=top_of_mobt; j>i; j--) {
            mob_index[j]=mob_index[j-1];
            mob_proto[j]=mob_proto[j-1];
            GET_MOB_RNUM(mob_proto+j)++;
          }
          mob_index[i].virtual=GET_OLC_NUM(ch);
          mob_index[i].number=0;
          mob_index[i].progtypes=0;
          mob_index[i].mobprogs=NULL;
          mob_index[i].func=spec_proc_table[((struct char_data *)GET_OLC_PTR(ch))->mob_specials.spec_proc];
          copy_mob(mob_proto+i, (struct char_data *)GET_OLC_PTR(ch));
          for(j=0; j<top_shop; j++) {
            if(SHOP_KEEPER(j) >= i)
              SHOP_KEEPER(j)++;
          }
          for(tch=character_list; tch; tch=tch->next) {
            if(IS_NPC(tch)) {
              if(GET_MOB_RNUM(tch) >= i) {
                GET_MOB_RNUM(tch)++;
              }
            }
            else {
              if(GET_OLC_MODE(tch)==OLC_ZEDIT) {
                dynamic_zedit_mob_fix(GET_OLC_PTR(tch), i);
              }
            }
          }
          re_renum_zone_table(top_of_world+1, i, top_of_objt+1);
        }
        for(j=0; j<=top_of_zone_table; j++) {
          if((GET_OLC_NUM(ch) >= zone_table[j].bottom) && (GET_OLC_NUM(ch) <= zone_table[j].top))
            break;
        }
        if(j > top_of_zone_table) {
          send_to_char("Error: mob is not in any zone, not saved.\r\n", ch);
        }
        else {
          sprintf(fname, "world/mob/%d.mob", zone_table[j].number);
          if(!(fp=fopen(fname, "w"))) {
            send_to_char("Error opening file, not saved.\r\n", ch);
          }
          else {
            for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
              if((nr=real_mobile(i)) >= 0) {
                sprintf(buf, "#%d\n", i);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", mob_proto[nr].player.name);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", mob_proto[nr].player.short_descr);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", mob_proto[nr].player.long_descr);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", mob_proto[nr].player.description);
                fputs(buf, fp);
                sprintf(buf, "%lld %lld %d W3\n", MOB_FLAGS(mob_proto+nr), AFF_FLAGS(mob_proto+nr), GET_ALIGNMENT(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d %dd%d+%d %dd%d+%d\n", GET_LEVEL(mob_proto+nr), GET_HITROLL(mob_proto+nr),
                        GET_AC(mob_proto+nr)/10, GET_HIT(mob_proto+nr), GET_MANA(mob_proto+nr), GET_MOVE(mob_proto+nr),
                        mob_proto[nr].mob_specials.damnodice, mob_proto[nr].mob_specials.damsizedice, GET_DAMROLL(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%ld %ld\n", GET_GOLD(mob_proto+nr), GET_EXP(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d\n", GET_POS(mob_proto+nr), GET_DEFAULT_POS(mob_proto+nr), GET_SEX(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d %d %d %d %d\n", mob_proto[nr].real_abils.str, mob_proto[nr].real_abils.str_add,
                        mob_proto[nr].real_abils.intel, mob_proto[nr].real_abils.wis, mob_proto[nr].real_abils.dex,
                        mob_proto[nr].real_abils.con, mob_proto[nr].real_abils.cha);
                fputs(buf, fp);
                sprintf(buf, "%d %d %d\n", mob_proto[nr].mob_specials.spec_proc, mob_proto[nr].mob_specials.size, GET_CLASS(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d %d %d %d\n", GET_AC(mob_proto+nr), mob_proto[nr].mob_specials.attack_type,
                        mob_proto[nr].mob_specials.attacks, mob_proto[nr].char_specials.hp_regen_add, GET_MR(mob_proto+nr), GET_PR(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d %d %d\n", GET_SAVE(mob_proto+nr, 0), GET_SAVE(mob_proto+nr, 1),
                        GET_SAVE(mob_proto+nr, 2), GET_SAVE(mob_proto+nr, 3), GET_SAVE(mob_proto+nr, 4));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d\n", IMMUNE_FLAGS(mob_proto+nr), WEAK_FLAGS(mob_proto+nr), RESIST_FLAGS(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%d %d\n", GET_MOVE_RATE(mob_proto+nr), GET_FEROCITY(mob_proto+nr));
                fputs(buf, fp);
                sprintf(buf, "%s~\n", (ACTIONS(mob_proto+nr)?ACTIONS(mob_proto+nr):""));
                fputs(buf, fp);
              }
            }
            fputs("$\n", fp);
            fclose(fp);
          }
        }
      }
      free_char((struct char_data *)GET_OLC_PTR(ch));
      break;
    case OLC_OEDIT:
      if(cmd == 1) {
        if(real_object(GET_OLC_NUM(ch)) >= 0) {
          sprintf(buf, "(GC) %s has editted obj %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          GET_OBJ_RNUM((struct obj_data *)GET_OLC_PTR(ch))=real_object(GET_OLC_NUM(ch));
          free(obj_proto[real_object(GET_OLC_NUM(ch))].name);
          free(obj_proto[real_object(GET_OLC_NUM(ch))].description);
          free(obj_proto[real_object(GET_OLC_NUM(ch))].short_description);
          free(obj_proto[real_object(GET_OLC_NUM(ch))].action_description);
          for(ex=obj_proto[real_object(GET_OLC_NUM(ch))].ex_description; ex; ex=tex) {
            tex=ex->next;
            free(ex->keyword);
            free(ex->description);
            free(ex);
          }
          if(GET_OBJ_TYPE((struct obj_data *)GET_OLC_PTR(ch))==ITEM_DAMAGEABLE)
            obj_index[real_object(GET_OLC_NUM(ch))].func=damage_eq;
          copy_obj(obj_proto+real_object(GET_OLC_NUM(ch)), (struct obj_data *)GET_OLC_PTR(ch));
          for(tobj=object_list; tobj; tobj=tobj->next) {
            if(GET_OBJ_RNUM(tobj)==GET_OBJ_RNUM((struct obj_data *)GET_OLC_PTR(ch))) {
              tobj->name=obj_proto[real_object(GET_OLC_NUM(ch))].name;
              tobj->description=obj_proto[real_object(GET_OLC_NUM(ch))].description;
              tobj->short_description=obj_proto[real_object(GET_OLC_NUM(ch))].short_description;
              tobj->action_description=obj_proto[real_object(GET_OLC_NUM(ch))].action_description;
              tobj->ex_description=obj_proto[real_object(GET_OLC_NUM(ch))].ex_description;
            }
          }
        }
        else {
          sprintf(buf, "(GC) %s has created obj %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          for(i=0; i<=top_of_objt; i++)
            if(obj_index[i].virtual > GET_OLC_NUM(ch))
              break;
          GET_OBJ_RNUM((struct obj_data *)GET_OLC_PTR(ch))=i;
          top_of_objt++;
          RECREATE(obj_index, struct index_data, top_of_objt+1);
          RECREATE(obj_proto, struct obj_data, top_of_objt+1);
          for(j=top_of_objt; j>i; j--) {
            obj_index[j]=obj_index[j-1];
            obj_proto[j]=obj_proto[j-1];
           GET_OBJ_RNUM(obj_proto+j)++;
          }
          obj_index[i].virtual=GET_OLC_NUM(ch);
          obj_index[i].number=0;
          obj_index[i].progtypes=0;
          obj_index[i].mobprogs=NULL;
          if(GET_OBJ_TYPE((struct obj_data *)GET_OLC_PTR(ch))==ITEM_DAMAGEABLE)
            obj_index[i].func=damage_eq;
          else
            obj_index[i].func=NULL;
          copy_obj(obj_proto+i, (struct obj_data *)GET_OLC_PTR(ch));
          for (j = 0; j < NUM_OF_BOARDS; j++)
            if (BOARD_RNUM(j) >= i)
              BOARD_RNUM(j)++;
          for(tobj=object_list; tobj; tobj=tobj->next) {
            if(GET_OBJ_RNUM(tobj) >= i) {
              GET_OBJ_RNUM(tobj)++;
            }
          }
          for(tch=character_list; tch; tch=tch->next) {
            if(!IS_NPC(tch)) {
              if(GET_OLC_MODE(tch)==OLC_ZEDIT) {
                dynamic_zedit_obj_fix(GET_OLC_PTR(tch), i);
              }
            }
          }
          for(shop_nr=0; shop_nr<top_shop; shop_nr++) {
            for(index = 0; SHOP_PRODUCT(shop_nr, index) != NOTHING; index++) {
              if(SHOP_PRODUCT(shop_nr, index) >= i)
                SHOP_PRODUCT(shop_nr, index)++;
            }
          }
          re_renum_zone_table(top_of_world+1, top_of_mobt+1, i);
        }
        for(j=0; j<=top_of_zone_table; j++) {
          if((GET_OLC_NUM(ch) >= zone_table[j].bottom) && (GET_OLC_NUM(ch) <= zone_table[j].top))
            break;
        }
        if(j > top_of_zone_table) {
          send_to_char("Error: obj is not in any zone, not saved.\r\n", ch);
        }
        else {
          sprintf(fname, "world/obj/%d.obj", zone_table[j].number);
          if(!(fp=fopen(fname, "w"))) {
            send_to_char("Error opening file, not saved.\r\n", ch);
          }
          else {
            for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
              if((nr=real_object(i)) >= 0) {
                sprintf(buf, "#%d\n", i);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", obj_proto[nr].name);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", obj_proto[nr].short_description);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", obj_proto[nr].description);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", obj_proto[nr].action_description);
                fputs(buf, fp);
                sprintf(buf, "%d %ld %ld %lld\n", GET_OBJ_TYPE(obj_proto+nr), GET_OBJ_EXTRA(obj_proto+nr),
                        GET_OBJ_WEAR(obj_proto+nr), obj_proto[nr].obj_flags.bitvector);
                fputs(buf, fp);
                sprintf(buf, "%d %d %d\n", obj_proto[nr].obj_flags.immune, obj_proto[nr].obj_flags.weak, obj_proto[nr].obj_flags.resist);
                fputs(buf, fp);
                sprintf(buf, "%d %d %d %d\n", GET_OBJ_VAL(obj_proto+nr, 0), GET_OBJ_VAL(obj_proto+nr, 1),
                        GET_OBJ_VAL(obj_proto+nr, 2), GET_OBJ_VAL(obj_proto+nr, 3));
                fputs(buf, fp);
                sprintf(buf, "%d %d %d\n", GET_OBJ_WEIGHT(obj_proto+nr), GET_OBJ_COST(obj_proto+nr), GET_OBJ_RENT(obj_proto+nr));
                fputs(buf, fp);
                for(k=0; k<MAX_OBJ_AFFECT; k++) {
                  if(obj_proto[nr].affected[k].location != APPLY_NONE) {
                    fputs("A\n", fp);
                    sprintf(buf, "%d %d\n", obj_proto[nr].affected[k].location, obj_proto[nr].affected[k].modifier);
                    fputs(buf, fp);
                  }
                }
                for(ex=obj_proto[nr].ex_description; ex; ex=ex->next) {
                  fputs("E\n", fp);
                  sprintf(buf, "%s~\n", ex->keyword);
                  fputs(buf, fp);
                  sprintf(buf, "%s~\n", ex->description);
                  fputs(buf, fp);
                }
              }
            }
            fputs("$\n", fp);
            fclose(fp);
          }
        }
      }
      free_obj((struct obj_data *)GET_OLC_PTR(ch));
      break;
    case OLC_REDIT:
      if(cmd == 1) {
        ((struct room_data *)GET_OLC_PTR(ch))->number=GET_OLC_NUM(ch);
        if(real_room(GET_OLC_NUM(ch)) >= 0) {
          sprintf(buf, "(GC) %s has editted room %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          free_partial_room(world+real_room(GET_OLC_NUM(ch)));
          copy_room(world+real_room(GET_OLC_NUM(ch)), (struct room_data *)GET_OLC_PTR(ch));
        }
        else {
          sprintf(buf, "(GC) %s has created room %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          for(i=0; i<=top_of_world; i++)
            if(world[i].number > GET_OLC_NUM(ch))
              break;
          for(j=0; j<=top_of_world; j++) {
            for(dir=0; dir<NUM_OF_DIRS; dir++) {
              if(world[j].dir_option[dir])
                if(world[j].dir_option[dir]->to_room >= i)
                  world[j].dir_option[dir]->to_room++;
            }
          }
          top_of_world++;
          RECREATE(world, struct room_data, top_of_world+1);
          for(j=top_of_world; j>i; j--) {
            world[j]=world[j-1];
          }
          copy_room(world+i, (struct room_data *)GET_OLC_PTR(ch));
          world[i].light=0;
          world[i].contents=NULL;
          world[i].people=NULL;
          for(dir=0; dir<NUM_OF_DIRS; dir++) {
            if(world[i].dir_option[dir])
              if(world[i].dir_option[dir]->to_room > NOWHERE)
                world[i].dir_option[dir]->to_room=real_room(world[i].dir_option[dir]->to_room);
          }
          for(tch=character_list; tch; tch=tch->next) {
            if(tch->in_room >= i)
              tch->in_room++;
            if(tch->was_in_room >= i)
              tch->was_in_room++;
            if(!IS_NPC(tch)) {
              if(GET_OLC_MODE(tch)==OLC_ZEDIT) {
                dynamic_zedit_room_fix(GET_OLC_PTR(tch), i);
              }
            }
          }
          for(tobj=object_list; tobj; tobj=tobj->next)
            if(tobj->in_room >= i)
              tobj->in_room++;
          if(r_mortal_start_room >= i)
            r_mortal_start_room++;
          if(r_immort_start_room >= i)
            r_immort_start_room++;
          if(r_frozen_start_room >= i)
            r_frozen_start_room++;
          re_renum_zone_table(i, top_of_mobt+1, top_of_objt+1);
        }
        for(j=0; j<=top_of_zone_table; j++) {
          if((GET_OLC_NUM(ch) >= zone_table[j].bottom) && (GET_OLC_NUM(ch) <= zone_table[j].top))
            break;
        }
        if(j > top_of_zone_table) {
          send_to_char("Error: obj is not in any zone, not saved.\r\n", ch);
        }
        else {
          sprintf(fname, "world/wld/%d.wld", zone_table[j].number);
          if(!(fp=fopen(fname, "w"))) {
            send_to_char("Error opening file, not saved.\r\n", ch);
          }
          else {
            for(i=zone_table[j].bottom; i <= zone_table[j].top; i++) {
              if((nr=real_room(i)) >= 0) {
                sprintf(buf, "#%d\n", i);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", world[nr].name);
                fputs(buf, fp);
                sprintf(buf, "%s~\n", world[nr].description);
                fputs(buf, fp);
                sprintf(buf, "%d %ld %d\n", zone_table[j].number, world[nr].room_flags, world[nr].sector_type);
                fputs(buf, fp);
                sprintf(buf, "%dd%d+%d %d %d\n", world[nr].dt_numdice, world[nr].dt_sizedice,
                        world[nr].dt_add, world[nr].dt_percent, world[nr].dt_repeat);
                fputs(buf, fp);
                world[nr].zone=j;
                for(k=0; k<NUM_OF_DIRS; k++) {
                  if(world[nr].dir_option[k]) {
                    sprintf(buf, "D%d\n", k);
                    fputs(buf, fp);
                    sprintf(buf, "%s~\n", world[nr].dir_option[k]->general_description);
                    fputs(buf, fp);
                    sprintf(buf, "%s~\n", world[nr].dir_option[k]->keyword);
                    fputs(buf, fp);
                    if(IS_SET(world[nr].dir_option[k]->exit_info, EX_ISDOOR)) {
                      if(IS_SET(world[nr].dir_option[k]->exit_info, EX_PICKPROOF))
                        exit_info=2;
                      else
                        exit_info=1;
                      if(IS_SET(world[nr].dir_option[k]->exit_info, EX_SECRET)) {
                        exit_info+=2;
                        if(IS_SET(world[nr].dir_option[k]->exit_info, EX_HIDDEN))
                          exit_info+=2;
                      }
                    }
                    else
                      exit_info=0;
                    to_room=world[nr].dir_option[k]->to_room;
                    if(to_room > 0)
                      to_room=world[to_room].number;
                    sprintf(buf, "%d %d %d\n", exit_info, world[nr].dir_option[k]->key, to_room);
                    fputs(buf, fp);
                  }
                }
                for(ex=world[nr].ex_description; ex; ex=ex->next) {
                  fputs("E\n", fp);
                  sprintf(buf, "%s~\n", ex->keyword);
                  fputs(buf, fp);
                  sprintf(buf, "%s~\n", ex->description);
                  fputs(buf, fp);
                }
                fputs("S\n", fp);
              }
            }
            fputs("$\n", fp);
            fclose(fp);
          }
        }
      }
      free_room((struct room_data *)GET_OLC_PTR(ch));
      break;
    case OLC_IEDIT:
      if(cmd == 1) {
        sprintf(buf, "(GC) %s has editted in-game obj %ld", GET_NAME(ch), GET_OLC_NUM(ch));
        mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
        ((struct obj_data *)GET_OLC_PTR(ch))->next=object_list;
        object_list=(struct obj_data *)GET_OLC_PTR(ch);
        if(((struct obj_data *)GET_OLC_PTR(ch))->worn_by) {
          ((struct obj_data *)GET_OLC_PTR(ch))->in_room=NOWHERE;
          ((struct obj_data *)GET_OLC_PTR(ch))->in_obj=NULL;
          if(GET_EQ(ch, ((struct obj_data *)GET_OLC_PTR(ch))->worn_on)) {
            obj_to_char((struct obj_data *)GET_OLC_PTR(ch), ch);
            ((struct obj_data *)GET_OLC_PTR(ch))->worn_on=-1;
            ((struct obj_data *)GET_OLC_PTR(ch))->worn_by=NULL;
          }
          else {
            ((struct obj_data *)GET_OLC_PTR(ch))->carried_by=NULL;
            equip_char(ch, (struct obj_data *)GET_OLC_PTR(ch), ((struct obj_data *)GET_OLC_PTR(ch))->worn_on);
          }
        }
        else if(((struct obj_data *)GET_OLC_PTR(ch))->in_room != NOWHERE) {
          obj_to_room((struct obj_data *)GET_OLC_PTR(ch), ((struct obj_data *)GET_OLC_PTR(ch))->in_room);
          ((struct obj_data *)GET_OLC_PTR(ch))->carried_by=NULL;
          ((struct obj_data *)GET_OLC_PTR(ch))->worn_on=-1;
          ((struct obj_data *)GET_OLC_PTR(ch))->worn_by=NULL;
          ((struct obj_data *)GET_OLC_PTR(ch))->in_obj=NULL;
        }
        else {
          obj_to_char((struct obj_data *)GET_OLC_PTR(ch), ch);
          ((struct obj_data *)GET_OLC_PTR(ch))->in_room=NOWHERE;
          ((struct obj_data *)GET_OLC_PTR(ch))->worn_on=-1;
          ((struct obj_data *)GET_OLC_PTR(ch))->worn_by=NULL;
          ((struct obj_data *)GET_OLC_PTR(ch))->in_obj=NULL;
        }
      }
      break;
    case OLC_PEDIT:
      if(cmd == 1) {
        for(tch=character_list; tch; tch=tch->next) {
          if(GET_IDNUM(tch)==GET_OLC_NUM(ch))
            break;
        }
        if(tch) {
          sprintf(buf, "(GC) %s has editted player %s", GET_NAME(ch), get_name_by_id(GET_OLC_NUM(ch)));
          mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
          free(tch->player.name);
          free(tch->player.description);
          free(tch->player.title);
          free(tch->char_specials.prompt);
          free(tch->player_specials->walkin);
          free(tch->player_specials->walkout);
          free(tch->player_specials->poofin);
          free(tch->player_specials->poofout);
          copy_plr(tch, (struct char_data *)GET_OLC_PTR(ch));
          send_to_char("You are momentarily overcome by a strong compression.\r\nYou feel different.\r\n", tch);
        }
        else {
          send_to_char("Your victim is no longer in the game, no saved.\r\n", ch);
        }
      }
      free_char((struct char_data *)GET_OLC_PTR(ch));
      break;
    case OLC_ZEDIT:
      if(cmd == 1) {
        if((j=real_zone(GET_OLC_NUM(ch))) < 0) {
          send_to_char("Error: that zone does not exist, not saved.\r\n", ch);
        }
        else {
          sprintf(buf, "(GC) %s has editted zone %ld", GET_NAME(ch), GET_OLC_NUM(ch));
          mudlog(buf, CMP, GET_LEVEL(ch), TRUE);
          tempz=(struct zcmd_data *)GET_OLC_PTR(ch);
          tempz->line=-1;
          tempz=tempz->next;
          count=count_zcmds(tempz);
          RECREATE(zone_table[j].cmd, struct reset_com, count+1);
          make_zone_commands(zone_table[j].cmd, tempz, 0);
          zone_table[j].cmd[count].command='S';
          sprintf(fname, "world/zon/%d.zon", zone_table[j].number);
          if(!(fp=fopen(fname, "w"))) {
            send_to_char("Error opening file, not saved.\r\n", ch);
          }
          else {
            sprintf(buf, "#%d\n", zone_table[j].number);
            fputs(buf, fp);
            sprintf(buf, "%s~\n", zone_table[j].name);
            fputs(buf, fp);
            sprintf(buf, "%d %d %d %d %d\n", zone_table[j].bottom, zone_table[j].top,
                    zone_table[j].lifespan, zone_table[j].reset_mode, zone_table[j].closed);
            fputs(buf, fp);
            for(i=0; i<count; i++) {
              arg1=zone_table[j].cmd[i].arg1;
              arg3=zone_table[j].cmd[i].arg3;
              switch (zone_table[j].cmd[i].command) {
              case 'M':
                arg1 = mob_index[zone_table[j].cmd[i].arg1].virtual;
                arg3 = world[zone_table[j].cmd[i].arg3].number;
                break;
              case 'O':
                arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
                if (zone_table[j].cmd[i].arg3 != NOWHERE)
                  arg3 = world[zone_table[j].cmd[i].arg3].number;
                break;
              case 'G':
                arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
                arg3 = mob_index[zone_table[j].cmd[i].arg3].virtual;
                break;
              case 'E':
                arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
                arg3 = mob_index[zone_table[j].cmd[i].arg3].virtual;
                break;
              case 'P':
                arg1 = obj_index[zone_table[j].cmd[i].arg1].virtual;
                arg3 = obj_index[zone_table[j].cmd[i].arg3].virtual;
                break;
              case 'D':
                arg1 = world[zone_table[j].cmd[i].arg1].number;
                break;
              case 'R': /* rem obj from room */
                arg1 = world[zone_table[j].cmd[i].arg1].number;
                break;
              }
              if(zone_table[j].cmd[i].command=='R')
                sprintf(buf, "%c %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                        zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                        arg1, obj_index[zone_table[j].cmd[i].arg2].virtual,
                        zone_table[j].cmd[i].prob);
              else if(zone_table[j].cmd[i].command=='E')
                sprintf(buf, "%c %d %d %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                        zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                        arg1, zone_table[j].cmd[i].arg2,
                        arg3, zone_table[j].cmd[i].arg4,
                        zone_table[j].cmd[i].prob);
              else
                sprintf(buf, "%c %d %d %d %d %d %d\n", zone_table[j].cmd[i].command,
                        zone_table[j].cmd[i].if_flag, zone_table[j].cmd[i].depend,
                        arg1, zone_table[j].cmd[i].arg2,
                        arg3, zone_table[j].cmd[i].prob);
              fputs(buf, fp);
            }
            fputs("S\n", fp);
            fputs("$\n", fp);
            fclose(fp);
          }
          do_zpurge(ch, itoa(zone_table[j].number), 0, 1);
          do_zreset(ch, itoa(zone_table[j].number), 0, 1);
        }
      }
      free_zcmd((struct zcmd_data *)GET_OLC_PTR(ch));
      break;
    }
    GET_OLC_MODE(ch)=0;
    GET_OLC_FIELD(ch)=0;
    GET_OLC_NUM(ch)=0;
    GET_OLC_PTR(ch)=NULL;
    REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
  }
}
ACMD(do_olcmenu)
{
  int command=0;
  skip_spaces(&argument);
  if(!*argument)
    command=OLC_MENU_NUM;
  else if(is_number(argument)) {
    command=atoi(argument);
  }
  if((GET_OLC_FIELD(ch) == 0) && (command == 0)) {
    send_to_char("Do you really want to quit editting?\r\n0. No\r\n1. Yes\r\n", ch);
    GET_OLC_FIELD(ch)=-1;
  }
  else if(GET_OLC_FIELD(ch) >= 0) {
    switch(GET_OLC_MODE(ch)) {
    case OLC_MEDIT:
      medit_menu(ch, command);
      break;
    case OLC_OEDIT:
      oedit_menu(ch, command);
      break;
    case OLC_REDIT:
      redit_menu(ch, command);
      break;
    case OLC_IEDIT:
      iedit_menu(ch, command);
      break;
    case OLC_PEDIT:
      pedit_menu(ch, command);
      break;
    case OLC_ZEDIT:
      zedit_menu(ch, command);
      break;
    }
  }
  else {
    confirm_edit(ch, command);
  }
}
