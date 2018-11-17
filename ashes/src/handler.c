/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
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
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"

int use_exact;

/* external vars */
extern int top_of_world;
extern int top_of_zone_table;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct char_data *to_free_list;
extern char *MENU;

/* external functions */
void free_char(struct char_data * ch);
void stop_fighting(struct char_data * ch);
void remove_follower(struct char_data * ch);
void clearMemory(struct char_data * ch);

/* Checks all room procs */
void room_procs(void)
{
  int i;

  for(i=0; i<=top_of_world; i++) {
    if(GET_ROOM_SPEC(i)) {
      (GET_ROOM_SPEC(i))(world[i].people, world+i, 0, "");
    }
  }
}

/* Checks all item procs */
void item_procs(void)
{
  struct obj_data *i, *next;

  for(i=object_list; i; i=next) {
    next=i->next;
    if(i->item_number >= 0) {
      if(GET_OBJ_SPEC(i)) {
        (GET_OBJ_SPEC(i))(i->worn_by?i->worn_by:i->carried_by, i, 0, "");
      }
    }
  }
}

char *fname(char *namelist)
{
  static char holder[30];
  register char *point;

  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;

  *point = '\0';

  return (holder);
}


int isname(char *str, char *namelist)
{
  register char *curname, *curstr;

  if((!str) || (!namelist) || (!*str))
    return (0);

  curname = namelist;
  for (;;) {
    for (curstr = str;; curstr++, curname++) {
      if (!*curstr)
	return (1);

      if (!*curname)
	return (0);

      if (LOWER(*curstr) != LOWER(*curname))
	break;
    }

    /* skip to next name */

    for (; isalpha(*curname); curname++);
    if (!*curname)
      return (0);
    curname++;			/* first char of new name */
  }
}

int isexact(char *str, char *namelist)
{
  register char *curname, *curstr;

  if((!str) || (!namelist) || (!*str))
    return (0);

  curname = namelist;
  for (;;) {
    for (curstr = str;; curstr++, curname++) {
      if (!*curstr && !isalpha(*curname))
	return (1);

      if (!*curname)
	return (0);

      if (!*curstr || *curname == ' ')
	break;

      if (LOWER(*curstr) != LOWER(*curname))
	break;
    }

    /* skip to next name */

    for (; isalpha(*curname); curname++);
    if (!*curname)
      return (0);
    curname++;			/* first char of new name */
  }
}



void affect_modify(struct char_data * ch, byte loc, sbyte mod, long long bitv, bool add)
{
  int maxabil;

  if (add) {
    SET_BIT(AFF_FLAGS(ch), bitv);
  } else {
    REMOVE_BIT(AFF_FLAGS(ch), bitv);
    mod = -mod;
  }


  maxabil = (IS_NPC(ch) ? 25 : ((GET_LEVEL(ch)>=LVL_HERO) ? 25 : 18));

  switch (loc) {
  case APPLY_NONE:
    break;

  case APPLY_STR:
    GET_STR(ch) += mod;
    break;
  case APPLY_DEX:
    GET_DEX(ch) += mod;
    break;
  case APPLY_INT:
    GET_INT(ch) += mod;
    break;
  case APPLY_WIS:
    GET_WIS(ch) += mod;
    break;
  case APPLY_CON:
    GET_CON(ch) += mod;
    break;
  case APPLY_CHA:
    GET_CHA(ch) += mod;
    break;

  case APPLY_AGE:
    ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
    break;

  case APPLY_CHAR_WEIGHT:
    GET_WEIGHT(ch) += mod;
    break;

  case APPLY_CHAR_HEIGHT:
    GET_HEIGHT(ch) += mod;
    break;

  case APPLY_MANA:
    GET_MAX_MANA(ch) += mod;
    break;

  case APPLY_HIT:
    GET_MAX_HIT(ch) += mod;
    break;

  case APPLY_MOVE:
    GET_MAX_MOVE(ch) += mod;
    break;

  case APPLY_MANA_REGEN:
    GET_MANA_REGEN_ADD(ch) += mod;
    break;

  case APPLY_HP_REGEN:
    GET_HP_REGEN_ADD(ch) += mod;
    break;

  case APPLY_MOVE_REGEN:
    GET_MOVE_REGEN_ADD(ch) += mod;
    break;

  case APPLY_AC:
    GET_AC(ch) += mod;
    break;

  case APPLY_HITROLL:
    GET_HITROLL(ch) += mod;
    break;

  case APPLY_DAMROLL:
    GET_DAMROLL(ch) += mod;
    break;

  case APPLY_SAVING_PARA:
    GET_SAVE(ch, SAVING_PARA) += mod;
    break;

  case APPLY_SAVING_ROD:
    GET_SAVE(ch, SAVING_ROD) += mod;
    break;

  case APPLY_SAVING_PETRI:
    GET_SAVE(ch, SAVING_PETRI) += mod;
    break;

  case APPLY_SAVING_BREATH:
    GET_SAVE(ch, SAVING_BREATH) += mod;
    break;

  case APPLY_SAVING_SPELL:
    GET_SAVE(ch, SAVING_SPELL) += mod;
    break;

  case APPLY_MR:
    GET_MR(ch) += mod;
    break;

  case APPLY_PR:
    GET_PR(ch) += mod;
    break;

  default:
    log("SYSERR: Unknown apply adjust attempt (handler.c, affect_modify).");
    break;

  } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data * ch)
{
  struct affected_type *af;
  int i, j, group=0;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
  }


  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

  ch->aff_abils = ch->real_abils;
  if(AFF_FLAGGED(ch, AFF_GROUP))
    group=1;
  AFF_FLAGS(ch)=ch->char_specials.saved.affected_by;
  if(group)
    SET_BIT(AFF_FLAGS(ch), AFF_GROUP);

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, TRUE);
  }


  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

  /* Make certain values are between 0..25, not < 0 and not > 25! */

  i = (IS_NPC(ch) ? 25 : ((GET_LEVEL(ch)>=LVL_HERO)? 25 : 18));

  GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), i));
  GET_INT(ch) = MAX(0, MIN(GET_INT(ch), i));
  GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), i));
  GET_CON(ch) = MAX(0, MIN(GET_CON(ch), i));
  GET_CHA(ch) = MAX(0, MIN(GET_CHA(ch), i));
  GET_STR(ch) = MAX(0, GET_STR(ch));

  if (IS_NPC(ch)||(GET_LEVEL(ch)>=LVL_HERO)) {
    GET_STR(ch) = MIN(GET_STR(ch), i);
  } else {
    if (GET_STR(ch) > 18) {
      i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);
      GET_ADD(ch) = MIN(i, 100);
      GET_STR(ch) = 18;
    }
  }
}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data * ch, struct affected_type * af)
{
  struct affected_type *affected_alloc;

  CREATE(affected_alloc, struct affected_type, 1);

  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;

  affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  affect_total(ch);
}



/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void affect_remove(struct char_data * ch, struct affected_type * af)
{
  struct affected_type *temp;

  assert(ch->affected);

  affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
  REMOVE_FROM_LIST(af, ch->affected, next);
  free(af);
  affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data * ch, int type)
{
  struct affected_type *hjp, *next;

  for (hjp = ch->affected; hjp; hjp = next) {
    next = hjp->next;
    if (hjp->type == type)
      affect_remove(ch, hjp);
  }
}



/*
 * Return if a char is affected by a spell (SPELL_XXX), NULL indicates
 * not affected
 */
bool affected_by_spell(struct char_data * ch, int type)
{
  struct affected_type *hjp;

  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == type)
      return TRUE;

  return FALSE;
}



void affect_join(struct char_data * ch, struct affected_type * af,
		      bool add_dur, bool avg_dur, bool add_mod, bool avg_mod)
{
  struct affected_type *hjp;
  long tmp;
  bool found = FALSE;

  for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {

    if ((hjp->type == af->type) && (hjp->location == af->location)) {
      tmp=af->duration;
      if (add_dur)
	tmp += hjp->duration;
      if (avg_dur)
	tmp >>= 1;
      af->duration = ((tmp > 30000) ? 30000 : ((tmp < -30000) ? -30000 : tmp));

      tmp=af->modifier;
      if (add_mod)
	tmp += hjp->modifier;
      if (avg_mod)
	tmp >>= 1;
      af->modifier = ((tmp > 100) ? 100 : ((tmp < -100) ? -100 : tmp));

      affect_remove(ch, hjp);
      affect_to_char(ch, af);
      found = TRUE;
    }
  }
  if (!found)
    affect_to_char(ch, af);
}


/* move a player out of a room */
void char_from_room(struct char_data * ch)
{
  struct char_data *temp;

  if (ch == NULL || ch->in_room == NOWHERE) {
    log("SYSERR: NULL or NOWHERE in handler.c, char_from_room");
    exit(1);
  }

  if (FIGHTING(ch) != NULL)
    stop_fighting(ch);

  if (GET_EQ(ch, WEAR_LIGHT) != NULL) {
    if(AFF_FLAGGED(ch, AFF_MAGIC_LIGHT)) {
      world[ch->in_room].light--;
    }
    else if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT) {
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2)) {	/* Light is ON */
	world[ch->in_room].light--;
      }
    }
  }

  REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
  ch->in_room = NOWHERE;
  ch->next_in_room = NULL;
}


/* place a character in a room */
void char_to_room(struct char_data * ch, int room)
{
  if (!ch || room < 0 || room > top_of_world)
    log("SYSERR: Illegal value(s) passed to char_to_room");
  else {
    ch->next_in_room = world[room].people;
    world[room].people = ch;
    ch->in_room = room;

    if(AFF_FLAGGED(ch, AFF_MAGIC_LIGHT)) {
      world[room].light++;
    }
    else if (GET_EQ(ch, WEAR_LIGHT)) {
      if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT) {
	if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2)) {	/* Light ON */
	  world[room].light++;
        }
      }
    }
  }
  if(*zone_table[world[room].zone].locked_by)
    send_to_char("WARNING! This zone is locked for editting.\r\n", ch);
}

/* reset from_load to 0 on all items in an object */
void obj_moved(struct obj_data *o)
{
  if(o) {
    obj_moved(o->contains);
    obj_moved(o->next_content);
    o->from_load=0;
  }
}

/* give an object to a char   */
void obj_to_char(struct obj_data * object, struct char_data * ch)
{
  if (object && ch) {
    object->next_content = ch->carrying;
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;

    /* set flag for crash-save system */
    if(!IS_NPC(ch))
      SET_BIT(PLR_FLAGS(ch), PLR_CRASH);

    if(object->from_load) {
      object->from_load=0;
      obj_moved(object->contains);
    }

    if(IS_SET(GET_OBJ_EXTRA(object), ITEM_DUPE)) {
      char buf[256];
      sprintf(buf, "DUPE WARNING: %s [%d] carried by %s", object->short_description, GET_OBJ_VNUM(object), GET_NAME(ch));
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    }
  } else
    log("SYSERR: NULL obj or char passed to obj_to_char");
}


/* take an object from a char */
void obj_from_char(struct obj_data * object)
{
  struct obj_data *temp;

  if (object == NULL) {
    log("SYSERR: NULL object passed to obj_from_char");
    return;
  }
  REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

  /* set flag for crash-save system */
  if(!IS_NPC(object->carried_by))
    SET_BIT(PLR_FLAGS(object->carried_by), PLR_CRASH);

  IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(object->carried_by)--;
  object->carried_by = NULL;
  object->next_content = NULL;
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data * ch, int eq_pos)
{
  int factor;

  assert(GET_EQ(ch, eq_pos));

  if (!(GET_OBJ_TYPE(GET_EQ(ch, eq_pos)) == ITEM_ARMOR))
    return 0;

  switch (eq_pos) {

  case WEAR_BODY:
    factor = 3;
    break;			/* 30% */
  case WEAR_HEAD:
    factor = 2;
    break;			/* 20% */
  case WEAR_LEGS:
    factor = 2;
    break;			/* 20% */
  default:
    factor = 1;
    break;			/* all others 10% */
  }

  return (factor * GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0));
}



void equip_char(struct char_data * ch, struct obj_data * obj, int pos)
{
  int j;
  int invalid_class(struct char_data *ch, struct obj_data *obj);
  void check_fall(struct char_data *ch, struct obj_data *obj);

  assert(pos >= 0 && pos < NUM_WEARS);

  if (GET_EQ(ch, pos)) {
    sprintf(buf, "SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
	    obj->short_description);
    log(buf);
    return;
  }
  if (obj->carried_by) {
    log("SYSERR: EQUIP: Obj is carried_by when equip.");
    return;
  }
  if (obj->in_room != NOWHERE) {
    log("SYSERR: EQUIP: Obj is in_room when equip.");
    return;
  }

  if(IS_NPC(ch) || (GET_LEVEL(ch) < LVL_HERO)) {
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) ||
        invalid_class(ch, obj)) {
      act("You are zapped by $p and instantly let go of it.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n is zapped by $p and instantly lets go of it.", FALSE, ch, obj, 0, TO_ROOM);
      obj_to_char(obj, ch);	/* changed to drop in inventory instead of ground */
      if(ch->in_room != NOWHERE) {
        check_fall(ch, NULL);
      }
      return;
    }
  }

  GET_EQ(ch, pos) = obj;
  obj->worn_by = ch;
  obj->worn_on = pos;

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) -= apply_ac(ch, pos);

  if (ch->in_room != NOWHERE) {
    if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2))	/* if light is ON */
	world[ch->in_room].light++;
  } else
    log("SYSERR: ch->in_room = NOWHERE when equipping char.");

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, TRUE);

  affect_total(ch);
}



struct obj_data *unequip_char(struct char_data * ch, int pos)
{
  int j;
  struct obj_data *obj;

  assert(pos >= 0 && pos < NUM_WEARS);
  assert(GET_EQ(ch, pos));

  obj = GET_EQ(ch, pos);
  obj->worn_by = NULL;
  obj->worn_on = -1;

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) += apply_ac(ch, pos);

  if (ch->in_room != NOWHERE) {
    if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2))	/* if light is ON */
	world[ch->in_room].light--;
  } else
    log("SYSERR: ch->in_room = NOWHERE when unequipping char.");

  GET_EQ(ch, pos) = NULL;

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, FALSE);

  affect_total(ch);

  return (obj);
}


int get_number(char **name)
{
  int i;
  char *ppos;
  char number[MAX_INPUT_LENGTH];

  *number = '\0';

  use_exact=0;

  if ((ppos = strchr(*name, '.'))) {
    *ppos++ = '\0';
    strcpy(number, *name);
    strcpy(*name, ppos);

    for (i = 0; *(number + i); i++)
      if (!isdigit(*(number + i)))
	return 0;

    return (atoi(number));
  }

  use_exact=1;

  return 1;
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data * list)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = list; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (++j == number)
        return i;

  return NULL;
}


/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data * list)
{
  struct obj_data *i;

  for (i = list; i; i = i->next_content)
    if (GET_OBJ_RNUM(i) == num)
      return i;

  return NULL;
}


/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = object_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->name))
      if (++j == number)
        return i;

  return NULL;
}


/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
  struct obj_data *i;

  for (i = object_list; i; i = i->next)
    if (GET_OBJ_RNUM(i) == nr)
      return i;

  return NULL;
}



/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  if(use_exact) {
    for(i=world[room].people; i; i=i->next_in_room)
      if(isexact(tmp, i->player.name))
        return(i);
  }

  for (i = world[room].people; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, i->player.name))
      if (++j == number)
	return i;

  return NULL;
}


/* search a room for a player, and return a pointer if found..  */
struct char_data *get_player_room(char *name, int room)
{
   struct char_data *i, *ab=NULL;

   for (i = world[room].people; i; i = i->next_in_room) {
      if (!IS_NPC(i) && !str_cmp(i->player.name, name))
         return i;
      if (!IS_NPC(i) && isname(name, i->player.name))
	 if(!ab)
            ab=i;
   }

   return ab;
}


/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player(tmp);

  if(use_exact) {
    for(i=character_list; i; i=i->next)
      if(isexact(tmp, i->player.name))
        return i;
  }

  for (i = character_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->player.name))
      if (++j == number)
	return i;

  return NULL;
}


/* search all over the world for a player, and return a pointer if found */
struct char_data *get_player(char *name)
{
   struct char_data *i, *ab=NULL;

   for (i = character_list; i; i = i->next) {
      if (!IS_NPC(i) && !str_cmp(i->player.name, name))
         return i;
      if (!IS_NPC(i) && isname(name, i->player.name))
	 if(!ab)
            ab=i;
   }

   return ab;
}


/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if((GET_MOB_RNUM(i) == nr) && IS_NPC(i))
      return i;

  return NULL;
}



/* put an object in a room */
void obj_to_room(struct obj_data * object, int room)
{
  extern int obj_decay_time;
  void check_fall(struct char_data *ch, struct obj_data *obj);

  if (!object || room < 0 || room > top_of_world)
    log("SYSERR: Illegal value(s) passed to obj_to_room");
  else {
    object->next_content = world[room].contents;
    world[room].contents = object;
    object->in_room = room;
    object->carried_by = NULL;
    if(*zone_table[world[room].zone].locked_by)
      GET_OBJ_TIMER(object) = 0;
    else
      GET_OBJ_TIMER(object) = obj_decay_time+number(0, 3);
    if(object->from_load) {
      object->from_load=0;
      obj_moved(object->contains);
    }
    if(IS_SET(GET_OBJ_EXTRA(object), ITEM_DUPE)) {
      char buf[256];
      sprintf(buf, "DUPE WARNING: %s [%d] in room %s [%d]", object->short_description, GET_OBJ_VNUM(object), world[room].name, world[room].number);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    }
  }
  check_fall(NULL, object);
}


/* Take an object from a room */
void obj_from_room(struct obj_data * object)
{
  struct obj_data *temp;

  if (!object || object->in_room == NOWHERE) {
    log("SYSERR: NULL object or obj not in a room passed to obj_from_room");
    return;
  }

  REMOVE_FROM_LIST(object, world[object->in_room].contents, next_content);

  object->in_room = NOWHERE;
  object->next_content = NULL;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data * obj, struct obj_data * obj_to)
{
  struct obj_data *tmp_obj;

  if (!obj || !obj_to || obj == obj_to) {
    log("SYSERR: NULL object or same source and target obj passed to obj_to_obj");
    return;
  }

  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;

  for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

  /* top level object.  Subtract weight from inventory if necessary. */
  GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
  if (tmp_obj->carried_by)
    IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
  if(obj->from_load) {
    obj->from_load=0;
    obj_moved(obj->contains);
  }
  if(IS_SET(GET_OBJ_EXTRA(obj), ITEM_DUPE)) {
    char buf[256], buf1[40];
    if(tmp_obj->carried_by)
      sprintf(buf1, " (carried by %s)", GET_NAME(tmp_obj->carried_by));
    else
      strcpy(buf1, "");
    sprintf(buf, "DUPE WARNING: %s [%d] in %s%s", obj->short_description, GET_OBJ_VNUM(obj), obj_to->short_description, buf1);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
  }
}


/* remove an object from an object */
void obj_from_obj(struct obj_data * obj)
{
  struct obj_data *temp, *obj_from;

  if (obj->in_obj == NULL) {
    log("error (handler.c): trying to illegally extract obj from obj");
    return;
  }
  obj_from = obj->in_obj;
  REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

  /* Subtract weight from containers container */
  for (temp = obj->in_obj; temp->in_obj; temp = temp->in_obj)
    GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);

  /* Subtract weight from char that carries the object */
  GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
  if (temp->carried_by)
    IS_CARRYING_W(temp->carried_by) -= GET_OBJ_WEIGHT(obj);

  obj->in_obj = NULL;
  obj->next_content = NULL;
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data * list, struct char_data * ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}


void item_new_owner(struct obj_data *item, struct char_data *ch)
{
   if(item) {
      object_list_new_owner(item->contains, ch);
      item->carried_by=ch;
   }
}


/* Extract an object from the world */
void extract_obj(struct obj_data * obj)
{
  struct obj_data *temp;

  if (obj->worn_by != NULL)
    if (unequip_char(obj->worn_by, obj->worn_on) != obj)
      log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->in_obj)
    obj_from_obj(obj);

  /* Get rid of the contents of the object, as well. */
  while (obj->contains)
    extract_obj(obj->contains);

  REMOVE_FROM_LIST(obj, object_list, next);

  if (GET_OBJ_RNUM(obj) >= 0)
    (obj_index[GET_OBJ_RNUM(obj)].number)--;
  free_obj(obj);
}



void update_object(struct obj_data * obj, int use)
{
  if (GET_OBJ_TIMER(obj) > 0)
    GET_OBJ_TIMER(obj) -= use;
  if (obj->contains)
    update_object(obj->contains, use);
  if (obj->next_content)
    update_object(obj->next_content, use);
}


void update_char_objects(struct char_data * ch)
{
  int i;

  if (GET_EQ(ch, WEAR_LIGHT) != NULL)
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2) > 0) {
	i = --GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2);
	if (i == 1) {
	  act("Your light begins to flicker and fade.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light begins to flicker and fade.", FALSE, ch, 0, 0, TO_ROOM);
	} else if (i == 0) {
	  act("Your light sputters out and dies.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light sputters out and dies.", FALSE, ch, 0, 0, TO_ROOM);
	  world[ch->in_room].light--;
	}
      }

  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      update_object(GET_EQ(ch, i), 2);

  if (ch->carrying)
    update_object(ch->carrying, 1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data * ch)
{
  struct char_data *k, *temp;
  struct descriptor_data *t_desc;
  struct obj_data *obj;
  int i, freed = 0;
  int was_nowhere;

  extern struct char_data *combat_list;

  ACMD(do_return);

  void die_follower(struct char_data * ch);

  if (!IS_NPC(ch) && !ch->desc) {
    for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
      if (t_desc->original == ch)
	do_return(t_desc->character, "", 0, 0);
  }
/*
  if (ch->in_room == NOWHERE) {
    log("SYSERR: NOWHERE extracting char. (handler.c, extract_char)");
    exit(1);
  }
*/
  if (ch->followers || ch->master)
    die_follower(ch);

  /* Forget snooping, if applicable */
  if (ch->desc) {
    if (ch->desc->snooping) {
      sprintf(buf, "(GC) %s stopped snooping %s.", GET_NAME(ch), GET_NAME(ch->desc->snooping->character));
      log(buf);
      ch->desc->snooping->snoop_by = NULL;
      ch->desc->snooping = NULL;
    }
    if (ch->desc->snoop_by) {
      SEND_TO_Q("Your victim is no longer among us.\r\n",
		ch->desc->snoop_by);
      sprintf(buf, "(GC) %s stopped snooping %s.", GET_NAME(ch->desc->snoop_by->character), GET_NAME(ch));
      log(buf);
      ch->desc->snoop_by->snooping = NULL;
      ch->desc->snoop_by = NULL;
    }
  }

  if(ch->in_room == NOWHERE) {
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i))
        obj_to_char(unequip_char(ch, i), ch);
    while(ch->carrying)
      extract_obj(ch->carrying);
  }
  else {
    /* transfer objects to room, if any */
    while (ch->carrying) {
      obj = ch->carrying;
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
    }

    /* transfer equipment to room, if any */
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i))
        obj_to_room(unequip_char(ch, i), ch->in_room);
  }

  if (FIGHTING(ch))
    stop_fighting(ch);

  for (k = combat_list; k; k = temp) {
    temp = k->next_fighting;
    if (FIGHTING(k) == ch)
      stop_fighting(k);
  }

  if(ch->in_room != NOWHERE) {
    was_nowhere=0;
    char_from_room(ch);
  }
  else {
    was_nowhere=1;
  }

  /* pull the char from the list */
  REMOVE_FROM_LIST(ch, character_list, next);

  if (ch->desc && ch->desc->original)
    do_return(ch, "", 0, 0);

  if (!IS_NPC(ch)) {
    if(!was_nowhere) {
      save_char(ch, NOWHERE);
      Crash_delete_crashfile(ch);
    }
  } else {
    if (GET_MOB_RNUM(ch) > -1)		/* if mobile */
      mob_index[GET_MOB_RNUM(ch)].number--;
    clearMemory(ch);		/* Only NPC's can have memory */
    ch->next=to_free_list;
    to_free_list=ch;
    freed = 1;
  }

  if (!freed && ch->desc != NULL) {
    GET_POS(ch)=POS_DEAD;
    STATE(ch->desc) = CON_MENU;
    SEND_TO_Q(MENU, ch->desc);
  } else {  /* if a player gets purged from within the game */
    if (!freed) {
      ch->next=to_free_list;
      to_free_list=ch;
    }
  }
}



/* ***********************************************************************
* Here follows high-level versions of some earlier routines, ie functions*
* which incorporate the actual player-data                               *.
*********************************************************************** */


struct char_data *get_player_vis(struct char_data * ch, char *name)
{
  struct char_data *i, *ab=NULL;

  for (i = character_list; i; i = i->next) {
    if (!IS_NPC(i) && !str_cmp(i->player.name, name) && CAN_SEE(ch, i))
      return i;
    if (!IS_NPC(i) && isname(name, i->player.name) && CAN_SEE(ch, i))
      if(!ab)
        ab=i;
  }

  return ab;
}


struct char_data *get_char_room_vis(struct char_data * ch, char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* JE 7/18/94 :-) :-) */
  if (!str_cmp(name, "self") || !str_cmp(name, "me"))
    return ch;

  /* 0.<name> means PC with name */
  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_room_vis(ch, tmp);

  if(use_exact) {
    for(i = world[ch->in_room].people; i; i = i->next_in_room)
      if(isexact(tmp, i->player.name))
        if(CAN_SEE(ch, i))
          return i;
  }

  for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
    if (isname(tmp, i->player.name))
      if (CAN_SEE(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


struct char_data *get_player_room_vis(struct char_data *ch, char *name)
{
   struct char_data *i, *ab=NULL;

   for (i = world[ch->in_room].people; i; i = i->next_in_room) {
      if (!IS_NPC(i) && !str_cmp(i->player.name, name) && CAN_SEE(ch, i))
         return i;
      if (!IS_NPC(i) && isname(name, i->player.name) && CAN_SEE(ch, i))
	 if(!ab)
            ab=i;
   }

   return ab;
}


struct char_data *get_char_vis(struct char_data * ch, char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* check the room first */
  if ((i = get_char_room_vis(ch, name)) != NULL)
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_vis(ch, tmp);

  if(use_exact) {
    for(i=character_list; i; i=i->next)
      if(isexact(tmp, i->player.name) && CAN_SEE(ch, i))
        return i;
  }

  for (i = character_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->player.name) && CAN_SEE(ch, i))
      if (++j == number)
	return i;

  return NULL;
}



struct obj_data *get_obj_in_list_vis(struct char_data * ch, char *name,
				              struct obj_data * list)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = list; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data * ch, char *name)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return i;

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


/* search the entire world for an object in an open zone, and return a pointer  */
struct obj_data *get_open_obj_vis(struct char_data * ch, char *name)
{
  struct obj_data *i, *temp;
  int j = 0, number, zone, vnum;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return i;

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i)) {
        if((vnum=GET_OBJ_VNUM(i)) > -1) {
          if (i->carried_by) {
            if(IS_NPC(i->carried_by))
              vnum=world[i->carried_by->in_room].number;
            else {
              if (++j == number)
                return i;
              continue;
            }
          }
          else if (i->worn_by) {
            if(IS_NPC(i->worn_by))
              vnum=world[i->worn_by->in_room].number;
            else {
              if (++j == number)
                return i;
              continue;
            }
          }
          else if (i->in_obj) {
            for(temp=i; temp->in_obj; temp=temp->in_obj);
            if (temp->carried_by) {
              if(IS_NPC(temp->carried_by))
                vnum=world[temp->carried_by->in_room].number;
              else {
                if (++j == number)
                  return i;
                continue;
              }
            }
            else if (temp->worn_by) {
              if(IS_NPC(temp->worn_by))
                vnum=world[temp->worn_by->in_room].number;
              else {
                if (++j == number)
                  return i;
                continue;
              }
            }
            else {
              vnum=GET_OBJ_VNUM(temp);
            }
          }
          for(zone=0; zone <= top_of_zone_table; zone++) {
            if((vnum >= zone_table[zone].bottom) && (vnum <= zone_table[zone].top))
              break;
          }
          if((zone > top_of_zone_table) || zone_table[zone].closed)
            continue;
          if (++j == number)
            return i;
        }
      }

  return NULL;
}


struct obj_data *get_obj_world(struct char_data * ch, char *name)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = object_list; i && (j <= number); i = i->next)
    if (isexact(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


struct obj_data *get_object_in_equip_vis(struct char_data * ch,
		           char *arg, struct obj_data * equipment[], int *j)
{
  for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name))
	  return (equipment[(*j)]);

  return NULL;
}


struct obj_data *get_object_in_equip(char *arg, struct obj_data * equipment[], int *j)
{
  for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
    if (equipment[(*j)])
      if (isname(arg, equipment[(*j)]->name))
        return (equipment[(*j)]);

  return NULL;
}


char *money_desc(int amount)
{
  static char buf[128];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  if (amount == 1)
    strcpy(buf, "a gold coin");
  else if (amount <= 10)
    strcpy(buf, "a tiny pile of gold coins");
  else if (amount <= 20)
    strcpy(buf, "a handful of gold coins");
  else if (amount <= 75)
    strcpy(buf, "a little pile of gold coins");
  else if (amount <= 200)
    strcpy(buf, "a small pile of gold coins");
  else if (amount <= 1000)
    strcpy(buf, "a pile of gold coins");
  else if (amount <= 5000)
    strcpy(buf, "a big pile of gold coins");
  else if (amount <= 10000)
    strcpy(buf, "a large heap of gold coins");
  else if (amount <= 20000)
    strcpy(buf, "a huge mound of gold coins");
  else if (amount <= 75000)
    strcpy(buf, "an enormous mound of gold coins");
  else if (amount <= 150000)
    strcpy(buf, "a small mountain of gold coins");
  else if (amount <= 250000)
    strcpy(buf, "a mountain of gold coins");
  else if (amount <= 500000)
    strcpy(buf, "a huge mountain of gold coins");
  else if (amount <= 1000000)
    strcpy(buf, "an enormous mountain of gold coins");
  else
    strcpy(buf, "an absolutely colossal mountain of gold coins");

  return buf;
}


struct obj_data *create_money(int amount)
{
  struct obj_data *obj;
  struct extra_descr_data *new_descr;
  char buf[200];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  obj = create_obj();
  CREATE(new_descr, struct extra_descr_data, 1);

  if (amount == 1) {
    obj->name = str_dup("coin gold");
    obj->short_description = str_dup("a gold coin");
    obj->description = str_dup("One miserable gold coin is lying here.");
    new_descr->keyword = str_dup("coin gold");
    new_descr->description = str_dup("It's just one miserable little gold coin.");
  } else {
    obj->name = str_dup("coins gold");
    obj->short_description = str_dup(money_desc(amount));
    sprintf(buf, "%s is lying here.", money_desc(amount));
    obj->description = str_dup(CAP(buf));

    new_descr->keyword = str_dup("coins gold");
    if (amount < 10) {
      sprintf(buf, "There are %d coins.", amount);
      new_descr->description = str_dup(buf);
    } else if (amount < 100) {
      sprintf(buf, "There are about %d coins.", 10 * (amount / 10));
      new_descr->description = str_dup(buf);
    } else if (amount < 1000) {
      sprintf(buf, "It looks to be about %d coins.", 100 * (amount / 100));
      new_descr->description = str_dup(buf);
    } else if (amount < 100000) {
      sprintf(buf, "You guess there are, maybe, %d coins.",
	      1000 * ((amount / 1000) + number(0, (amount / 1000))));
      new_descr->description = str_dup(buf);
    } else
      new_descr->description = str_dup("There are a LOT of coins.");
  }

  new_descr->next = NULL;
  obj->ex_description = new_descr;

  GET_OBJ_TYPE(obj) = ITEM_MONEY;
  GET_OBJ_WEAR(obj) = ITEM_WEAR_TAKE;
  GET_OBJ_VAL(obj, 0) = amount;
  GET_OBJ_COST(obj) = amount;
  obj->item_number = NOTHING;

  return obj;
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data * ch,
		     struct char_data ** tar_ch, struct obj_data ** tar_obj)
{
  int i, found;
  char name[256];

  one_argument(arg, name);

  if (!*name)
    return (0);

  *tar_ch = NULL;
  *tar_obj = NULL;

  if (IS_SET(bitvector, FIND_CHAR_ROOM)) {	/* Find person in room */
    if ((*tar_ch = get_char_room_vis(ch, name))) {
      return (FIND_CHAR_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*tar_ch = get_char_vis(ch, name))) {
      return (FIND_CHAR_WORLD);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
      if (GET_EQ(ch, i) && isname(name, GET_EQ(ch, i)->name) == 1) {
	*tar_obj = GET_EQ(ch, i);
	found = TRUE;
      }
    if (found) {
      return (FIND_OBJ_EQUIP);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
      return (FIND_OBJ_INV);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) {
      return (FIND_OBJ_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*tar_obj = get_obj_vis(ch, name))) {
      return (FIND_OBJ_WORLD);
    }
  }
  return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg)
{
  if (!strcmp(arg, "all"))
    return FIND_ALL;
  else if (!strncmp(arg, "all.", 4)) {
    strcpy(arg, arg + 4);
    return FIND_ALLDOT;
  } else
    return FIND_INDIV;
}
