/* ************************************************************************
*   File: objsave.c                                     Part of CircleMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
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
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

/* these factors should be unique integers */
#define RENT_FACTOR 	1
#define CRYO_FACTOR 	4

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int min_rent_cost;
extern long item_id;

/* Extern functions */
ACMD(do_tell);
SPECIAL(receptionist);
SPECIAL(cryogenicist);

int wear_bitv[] = {
  ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
  ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
  ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
  ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
  ITEM_WEAR_WIELD, ITEM_WEAR_TAKE};

struct dupe_list {
  long item_id;
  long vnum;
  long player_id;
  bool duped;
  struct dupe_list *less;
  struct dupe_list *more;
} *dupes;

void add_dupe_entry(struct obj_file_elem *o, long id)
{
  struct dupe_list *new_dupe, *d=dupes;

  CREATE(new_dupe, struct dupe_list, 1);
  new_dupe->item_id=o->id;
  new_dupe->vnum=o->item_number;
  new_dupe->player_id=id;
  new_dupe->duped=FALSE;

  if(o->id > item_id)
    item_id=o->id;

  while(1) {
    if(d->item_id < o->id) {
      if((d->more==NULL)||(d->more->item_id > o->id)) {
        new_dupe->more=d->more;
        d->more=new_dupe;
        if((new_dupe->more) && (new_dupe->more->less) && (new_dupe->more->less->item_id < o->id)) {
          new_dupe->less=new_dupe->more->less;
          new_dupe->more->less=NULL;
        }
        else
          new_dupe->less=NULL;
        break;
      }
      else {
        d=d->more;
      }
    }
    else {
      if((d->less==NULL)||(d->less->item_id <= o->id)) {
        new_dupe->less=d->less;
        d->less=new_dupe;
        if((new_dupe->less) && (new_dupe->less->more) && (new_dupe->less->more->item_id > o->id)) {
          new_dupe->more=new_dupe->less->more;
          new_dupe->less->more=NULL;
        }
        else
          new_dupe->more=NULL;
        break;
      }
      else {
        d=d->less;
      }
    }
  }
}

struct dupe_list *find_dupe_entry(struct obj_file_elem *o)
{
  struct dupe_list *d=dupes;

  while(1) {
    if(d->item_id < o->id) {
      if(d->more) {
        d=d->more;
      }
      else {
        return(NULL);
      }
    }
    else if(d->item_id > o->id) {
      if(d->less) {
        d=d->less;
      }
      else {
        return(NULL);
      }
    }
    else {
      if(d->vnum==o->item_number) {
        return(d);
      }
      else if(d->less) {
        d=d->less;
      }
      else {
        return(NULL);
      }
    }
  }
}

void destroy_dupe_list(struct dupe_list *d)
{
  if(d->more) {
    destroy_dupe_list(d->more);
  }
  if(d->less) {
    destroy_dupe_list(d->less);
  }
  free(d);
}

void redo_check(struct obj_file_elem *o)
{
  struct dupe_list *d;

  if(o->item_number < 0)
    return;

  if(o->id > item_id)
    item_id=o->id;

  if((d=find_dupe_entry(o))) {
    if(d->duped==TRUE) {
      SET_BIT(o->extra_flags, ITEM_DUPE);
    }
    else {
      REMOVE_BIT(o->extra_flags, ITEM_DUPE);
    }
  }
  return;
}

void reimb_check(struct obj_file_elem *o)
{
  struct dupe_list *d;

  if(o->item_number < 0)
    return;

  if(o->id > item_id)
    item_id=o->id;

  if((d=find_dupe_entry(o)))
    SET_BIT(o->extra_flags, ITEM_DUPE);
  else
    REMOVE_BIT(o->extra_flags, ITEM_DUPE);
  return;
}

int add_check(struct obj_file_elem *o, long id)
{
  struct dupe_list *d;

  if(o->item_number < 0)
    return 0;

  if((d=find_dupe_entry(o))) {
    SET_BIT(o->extra_flags, ITEM_DUPE);
    if(d->player_id==id) {
      d->duped=TRUE;
      return 1;
    }
    else if(d->duped==FALSE) {
      FILE *fl, *fp;
      char fname[MAX_STRING_LENGTH], tmpname[]="temprent2.tmp";
      struct obj_file_elem object;
      struct rent_info rent;

      d->duped=TRUE;

      if(!get_name_by_id(d->player_id))
        return 0;

      sprintf(fname, "DUPE WARNING: %s has duped eq", get_name_by_id(d->player_id));
      log(fname);

      if (!get_filename(get_name_by_id(d->player_id), fname, CRASH_FILE))
        return 0;
      if (!(fl = fopen(fname, "r+b"))) {
        if (errno != ENOENT) {	/* if it fails, NOT because of no file */
          sprintf(buf1, "SYSERR: READING OBJECT FILE %s(3), DUPE CHECK INCOMPLETE", fname);
          perror(buf1);
        }
        return 0;
      }

      if (!(fp = fopen(tmpname, "wb"))) {
        if (errno != ENOENT) {	/* if it fails, NOT because of no file */
          perror("SYSERR: CREATING TEMP OBJ FILE(3), DUPE CHECK INCOMPLETE");
        }
        fclose(fl);
        return 0;
      }

      if (!feof(fl))
        fread(&rent, sizeof(struct rent_info), 1, fl);

      if(fwrite(&rent, sizeof(struct rent_info), 1, fp)<1) {
        perror("SYSERR: WRITING TO TEMPFILE(7), DUPE CHECK INCOMPLETE");
        fclose(fl);
        fclose(fp);
        return 0;
      }

      while (!feof(fl)) {
        fread(&object, sizeof(struct obj_file_elem), 1, fl);
        if (ferror(fl)) {
          fclose(fl);
          fclose(fp);
          return 0;
        }
        if (!feof(fl)) {
          redo_check(&object);
          if(fwrite(&object, sizeof(struct obj_file_elem), 1, fp)<1) {
            perror("SYSERR: WRITING TO TEMPFILE(8), DUPE CHECK INCOMPLETE");
            fclose(fl);
            fclose(fp);
            return 0;
          }
        }
      }
      fclose(fl);
      fclose(fp);
      unlink(fname);
      rename(tmpname, fname);
      return 0;
    }
    else {
      return 0;
    }
  }
  else {
    REMOVE_BIT(o->extra_flags, ITEM_DUPE);
    add_dupe_entry(o, id);
    return 0;
  }
}

void o_to_store(struct obj_data * obj, struct obj_file_elem *object)
{
  int j;

  object->item_number = GET_OBJ_VNUM(obj);
  object->value[0] = GET_OBJ_VAL(obj, 0);
  object->value[1] = GET_OBJ_VAL(obj, 1);
  object->value[2] = GET_OBJ_VAL(obj, 2);
  object->value[3] = GET_OBJ_VAL(obj, 3);
  object->wear_flags = GET_OBJ_WEAR(obj);
  object->extra_flags = GET_OBJ_EXTRA(obj);
  object->weight = GET_OBJ_WEIGHT(obj);
  object->timer = GET_OBJ_TIMER(obj);
  object->bitvector = obj->obj_flags.bitvector;
  object->immune = obj->obj_flags.immune;
  object->weak = obj->obj_flags.weak;
  object->resist = obj->obj_flags.resist;
  object->worn_on = obj->worn_on;
  object->id = obj->id;

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    object->affected[j] = obj->affected[j];
}

void mark_reimb(char *name)
{
  FILE *fl, *fp;
  char fname[MAX_STRING_LENGTH], tmpname[]="tempreimb.tmp";
  struct obj_file_elem object;
  struct rent_info rent;
  struct update_list *uptr;
  struct obj_data *obj;

  extern struct update_list *updates;

  if (!get_filename(name, fname, REIMB_FILE))
    return;
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING REIMB FILE %s, DUPE CHECK INCOMPLETE", fname);
      perror(buf1);
    }
    return;
  }

  if (!(fp = fopen(tmpname, "wb"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      perror("SYSERR: CREATING TEMP REIMB FILE, DUPE CHECK INCOMPLETE");
    }
    fclose(fl);
    return;
  }

  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if(fwrite(&rent, sizeof(struct rent_info), 1, fp)<1) {
    perror("SYSERR: WRITING TO TEMPFILE(5), DUPE CHECK INCOMPLETE");
    fclose(fl);
    fclose(fp);
    return;
  }

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      fclose(fp);
      return;
    }
    if (!feof(fl)) {
      for(uptr=updates; uptr; uptr=uptr->next) {
        if(uptr->nr == object.item_number)
        if((obj=read_object(uptr->nr, VIRTUAL))) {
          obj->id=object.id;
          o_to_store(obj, &object);
          extract_obj(obj);
          object.worn_on=-1;
        }
      }

      reimb_check(&object);
      if(fwrite(&object, sizeof(struct obj_file_elem), 1, fp)<1) {
        perror("SYSERR: WRITING TO TEMPFILE(6), DUPE CHECK INCOMPLETE");
        fclose(fl);
        fclose(fp);
        return;
      }
    }
  }
  fclose(fl);
  fclose(fp);
  unlink(fname);
  rename(tmpname, fname);
}

void check_rent(char *name)
{
  FILE *fl, *fp;
  char fname[MAX_STRING_LENGTH], tmpname[]="temprent.tmp";
  struct obj_file_elem object;
  struct rent_info rent;
  struct update_list *uptr;
  struct obj_data *obj;
  int redo=0, duped=0;

  extern struct update_list *updates;

  if (!get_filename(name, fname, CRASH_FILE))
    return;
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING OBJECT FILE %s(1), DUPE CHECK INCOMPLETE", fname);
      perror(buf1);
    }
    return;
  }

  if (!(fp = fopen(tmpname, "wb"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      perror("SYSERR: CREATING TEMP OBJ FILE(1), DUPE CHECK INCOMPLETE");
    }
    fclose(fl);
    return;
  }

  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if(fwrite(&rent, sizeof(struct rent_info), 1, fp)<1) {
    perror("SYSERR: WRITING TO TEMPFILE(1), DUPE CHECK INCOMPLETE");
    fclose(fl);
    fclose(fp);
    return;
  }

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      fclose(fp);
      return;
    }
    if (!feof(fl)) {
      for(uptr=updates; uptr; uptr=uptr->next) {
        if(uptr->nr == object.item_number)
        if((obj=read_object(uptr->nr, VIRTUAL))) {
          obj->id=object.id;
          o_to_store(obj, &object);
          extract_obj(obj);
          object.worn_on=-1;
        }
      }

      redo+=add_check(&object, get_id_by_name(name));
      if(IS_SET(object.extra_flags, ITEM_DUPE))
        duped=1;
      if(fwrite(&object, sizeof(struct obj_file_elem), 1, fp)<1) {
        perror("SYSERR: WRITING TO TEMPFILE(2), DUPE CHECK INCOMPLETE");
        fclose(fl);
        fclose(fp);
        return;
      }
    }
  }
  fclose(fl);
  fclose(fp);
  unlink(fname);
  rename(tmpname, fname);

  if(redo) {
    if (!(fl = fopen(fname, "r+b"))) {
      if (errno != ENOENT) {	/* if it fails, NOT because of no file */
        sprintf(buf1, "SYSERR: READING OBJECT FILE %s(2), DUPE CHECK INCOMPLETE", fname);
        perror(buf1);
      }
      return;
    }

    if (!(fp = fopen(tmpname, "wb"))) {
      if (errno != ENOENT) {	/* if it fails, NOT because of no file */
        perror("SYSERR: CREATING TEMP OBJ FILE(2), DUPE CHECK INCOMPLETE");
      }
      fclose(fl);
      return;
    }

    if (!feof(fl))
      fread(&rent, sizeof(struct rent_info), 1, fl);

    if(fwrite(&rent, sizeof(struct rent_info), 1, fp)<1) {
      perror("SYSERR: WRITING TO TEMPFILE(3), DUPE CHECK INCOMPLETE");
      fclose(fl);
      fclose(fp);
      return;
    }

    while (!feof(fl)) {
      fread(&object, sizeof(struct obj_file_elem), 1, fl);
      if (ferror(fl)) {
        fclose(fl);
        fclose(fp);
        return;
      }
      if (!feof(fl)) {
        redo_check(&object);
        if(fwrite(&object, sizeof(struct obj_file_elem), 1, fp)<1) {
          perror("SYSERR: WRITING TO TEMPFILE(4), DUPE CHECK INCOMPLETE");
          fclose(fl);
          fclose(fp);
          return;
        }
      }
    }
    fclose(fl);
    fclose(fp);
    unlink(fname);
    rename(tmpname, fname);
  }

  if(duped) {
    sprintf(fname, "DUPE WARNING: %s has duped eq", name);
    log(fname);
  }
}

void obj_dupe_check(void)
{
  int i;

  CREATE(dupes, struct dupe_list, 1);
  dupes->item_id=0;
  dupes->vnum=-1;
  dupes->player_id=-1;
  dupes->duped=0;
  dupes->less=NULL;
  dupes->more=NULL;

  for (i = 0; i <= top_of_p_table; i++) {
    check_rent((player_table + i)->name);
  }

  log("Rent check done");

  for (i = 0; i <= top_of_p_table; i++) {
    mark_reimb((player_table + i)->name);
  }

  log("Reimb mark done");

  destroy_dupe_list(dupes);
  return;
}


void alias_delete(char *name)
{
  char buf[MAX_INPUT_LENGTH];

  if(get_filename(name, buf, ALIAS_FILE)) {
    unlink(buf);
  }
}

void equip_reset(struct obj_data *o)
{
  if(o) {
    equip_reset(o->next_content);
    equip_reset(o->contains);
    o->worn_on=-1;
  }
}

int mail_limit(int level)
{
  if(level < LVL_HERO)
    return MAX_MORT_SAVED_MAIL;
  else if(level < LVL_CIMP)
    return MAX_IMM_SAVED_MAIL;
  else
    return MAX_ADMIN_SAVED_MAIL;
}

struct obj_data *Obj_from_store(struct obj_file_elem object)
{
  struct obj_data *obj;
  int j;

  if (real_object(object.item_number) > -1) {
    obj = read_object(object.item_number, VIRTUAL);
    GET_OBJ_VAL(obj, 0) = object.value[0];
    GET_OBJ_VAL(obj, 1) = object.value[1];
    GET_OBJ_VAL(obj, 2) = object.value[2];
    GET_OBJ_VAL(obj, 3) = object.value[3];
    GET_OBJ_WEAR(obj) = object.wear_flags;
    GET_OBJ_EXTRA(obj) = object.extra_flags;
    GET_OBJ_WEIGHT(obj) = object.weight;
    GET_OBJ_TIMER(obj) = object.timer;
    obj->obj_flags.bitvector = object.bitvector;
    obj->obj_flags.immune = object.immune;
    obj->obj_flags.weak = object.weak;
    obj->obj_flags.resist = object.resist;
    obj->worn_on = object.worn_on;
    obj->id = object.id;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
      obj->affected[j] = object.affected[j];

    return obj;
  } else
    return NULL;
}



int Obj_to_store(struct obj_data * obj, FILE * fl)
{
  int j;
  struct obj_file_elem object;

  object.item_number = GET_OBJ_VNUM(obj);
  object.value[0] = GET_OBJ_VAL(obj, 0);
  object.value[1] = GET_OBJ_VAL(obj, 1);
  object.value[2] = GET_OBJ_VAL(obj, 2);
  object.value[3] = GET_OBJ_VAL(obj, 3);
  object.wear_flags = GET_OBJ_WEAR(obj);
  object.extra_flags = GET_OBJ_EXTRA(obj);
  object.weight = GET_OBJ_WEIGHT(obj);
  object.timer = GET_OBJ_TIMER(obj);
  object.bitvector = obj->obj_flags.bitvector;
  object.immune = obj->obj_flags.immune;
  object.weak = obj->obj_flags.weak;
  object.resist = obj->obj_flags.resist;
  object.worn_on = obj->worn_on;
  object.id = obj->id;

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    object.affected[j] = obj->affected[j];

  if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
    perror("Error writing object in Obj_to_store");
    return 0;
  }
  return 1;
}



int Crash_delete_file(char *name)
{
  char filename[50];
  FILE *fl;

  if (!get_filename(name, filename, CRASH_FILE))
    return 0;
  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) {	/* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
      perror(buf1);
    }
    return 0;
  }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
      perror(buf1);
    }
  }
  return (1);
}


int Crash_delete_crashfile(struct char_data * ch)
{
  char fname[MAX_INPUT_LENGTH];
  struct rent_info rent;
  FILE *fl;

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 0;
  if (!(fl = fopen(fname, "rb"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: checking for crash file %s (3)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

  if (rent.rentcode == RENT_CRASH)
    Crash_delete_file(GET_NAME(ch));

  return 1;
}


int Crash_clean_file(char *name)
{
  char fname[MAX_STRING_LENGTH], filetype[20];
  struct rent_info rent;
  extern int rent_file_timeout, crash_file_timeout;
  FILE *fl;

  if (!get_filename(name, fname, CRASH_FILE))
    return 0;
  /*
   * open for write so that permission problems will be flagged now, at boot
   * time.
   */
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: OPENING OBJECT FILE %s (4)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

  if ((rent.rentcode == RENT_CRASH) ||
      (rent.rentcode == RENT_FORCED) || (rent.rentcode == RENT_TIMEDOUT)) {
    if (rent.time < time(0) - (crash_file_timeout * SECS_PER_REAL_DAY)) {
      Crash_delete_file(name);
      switch (rent.rentcode) {
      case RENT_CRASH:
	strcpy(filetype, "crash");
	break;
      case RENT_FORCED:
	strcpy(filetype, "forced rent");
	break;
      case RENT_TIMEDOUT:
	strcpy(filetype, "idlesave");
	break;
      default:
	strcpy(filetype, "UNKNOWN!");
	break;
      }
      sprintf(buf, "    Deleting %s's %s file.", name, filetype);
      log(buf);
      return 1;
    }
    /* Must retrieve rented items w/in 30 days */
  } else if (rent.rentcode == RENT_RENTED)
    if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY)) {
      Crash_delete_file(name);
      sprintf(buf, "    Deleting %s's rent file.", name);
      log(buf);
      return 1;
    }
  return (0);
}


void update_obj_file(void)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    Crash_clean_file((player_table + i)->name);
  }
  return;
}


void Crash_listrent(struct char_data * ch, char *name)
{
  FILE *fl;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  int overflow=0;
  struct obj_file_elem object;
  struct obj_data *obj;
  struct rent_info rent;

  if (!get_filename(name, fname, CRASH_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no rent file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  switch (rent.rentcode) {
  case RENT_RENTED:
    strcat(buf, "Rent\r\n");
    break;
  case RENT_CRASH:
    strcat(buf, "Crash\r\n");
    break;
  case RENT_CRYO:
    strcat(buf, "Cryo\r\n");
    break;
  case RENT_TIMEDOUT:
  case RENT_FORCED:
    strcat(buf, "TimedOut\r\n");
    break;
  default:
    strcat(buf, "Undef\r\n");
    break;
  }
  strcat(buf, ctime(&rent.time));
  strcat(buf, "\r");
  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      return;
    }
    if (!feof(fl))
      if (real_object(object.item_number) > -1) {
	obj = read_object(object.item_number, VIRTUAL);
        if(strlen(buf) > MAX_STRING_LENGTH-120) {
          if(!overflow) {
            strcat(buf, "***OVERFLOW***\r\n");
            overflow=1;
          }
        }
        else {
          sprintf(buf, "%s [%5d] (%5dau) %-20s id:%ld%s\r\n", buf,
                  object.item_number, GET_OBJ_COST(obj),
                  obj->short_description, object.id,
                  (IS_SET(object.extra_flags, ITEM_DUPE)?" <DUPE>":""));
        }
	extract_obj(obj);
      }
  }
  page_string(ch->desc, buf, 1);

  fclose(fl);
}



int Crash_write_rentcode(struct char_data * ch, FILE * fl, struct rent_info * rent)
{
  if (fwrite(rent, sizeof(struct rent_info), 1, fl) < 1) {
    perror("Writing rent code.");
    return 0;
  }
  return 1;
}



int Crash_load(struct char_data * ch)
/* return values:
	0 - successful load, keep char in rent room.
	1 - load failure or load of crash items -- put char in temple.
	2 - rented equipment lost (no $)
*/
{
  void Crash_crashsave(struct char_data * ch);
  int Crash_is_unrentable(struct obj_data * obj);

  FILE *fl;
  char fname[MAX_STRING_LENGTH];
  char tmp_message[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct obj_data *o;
  struct rent_info rent;
  int cost, orig_rent_code, message_length, i;
  float num_of_days;

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 1;
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
      perror(buf1);
      send_to_char("\r\n********************* NOTICE *********************\r\n"
		   "There was a problem loading your objects from disk.\r\n"
		   "Contact a God for assistance.\r\n", ch);
    }
    sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    return 1;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if (rent.rentcode == RENT_RENTED || rent.rentcode == RENT_TIMEDOUT) {
    num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
    cost = (int) (rent.net_cost_per_diem * num_of_days);
    if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
      fclose(fl);
      sprintf(buf, "%s entering game, rented equipment lost (no $).",
	      GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
      Crash_crashsave(ch);
      return 2;
    } else {
      GET_BANK_GOLD(ch) -= MAX(cost - GET_GOLD(ch), 0);
      GET_GOLD(ch) = MAX(GET_GOLD(ch) - cost, 0);
      save_char(ch, NOWHERE);
    }
  }
  switch (orig_rent_code = rent.rentcode) {
  case RENT_RENTED:
    sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_CRASH:
    sprintf(buf, "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_CRYO:
    sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    break;
  case RENT_FORCED:
  case RENT_TIMEDOUT:
    sprintf(buf, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    break;
  default:
    sprintf(buf, "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
    mudlog(buf, BRF, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    break;
  }

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      perror("Reading crash file: Crash_load.");
      fclose(fl);
      return 1;
    }
    if (!feof(fl)) {
      if(!(o=Obj_from_store(object)))
        continue;
      if(Crash_is_unrentable(o)) {
        extract_obj(o);
      }
      else if((GET_OBJ_VNUM(o)<=zone_table[real_zone(12)].top)&&(GET_OBJ_VNUM(o)>=zone_table[real_zone(12)].bottom)&&(GET_LEVEL(ch)<LVL_HERO)) {
        extract_obj(o);
      }
      else {
        if(o->worn_on > -1) {
          if(!CAN_WEAR(o, wear_bitv[o->worn_on])) {
            o->worn_on = -1;
            obj_to_char(o, ch);
          }
          else if(GET_EQ(ch, o->worn_on)) {
            o->worn_on = -1;
            obj_to_char(o, ch);
          }
          else if((o->worn_on == WEAR_LIGHT) && (GET_OBJ_TYPE(o) != ITEM_LIGHT)) {
            o->worn_on = -1;
            obj_to_char(o, ch);
          }
          else
            equip_char(ch, o, o->worn_on);
        }
        else
          obj_to_char(o, ch);
        if(IS_OBJ_STAT(o, ITEM_DUPE)) {
          sprintf(buf, "DUPE WARNING: %s entering with a duped item: %s [%d]", GET_NAME(ch), o->short_description, GET_OBJ_VNUM(o));
          mudlog(buf, BRF, LVL_IMMORT, TRUE);
        }
        if(GET_LEVEL(ch) >= LVL_HERO) {
          o->obj_flags.value[0]=0;
          o->obj_flags.value[1]=0;
          o->obj_flags.value[2]=0;
          o->obj_flags.value[3]=0;
          o->obj_flags.type_flag=13;
          o->obj_flags.cost=0;
          o->obj_flags.bitvector=0;
          o->obj_flags.immune=0;
          o->obj_flags.resist=0;
          o->obj_flags.weak=0;
          for(i=0; i<MAX_OBJ_AFFECT; i++) {
            o->affected[i].location=APPLY_NONE;
            o->affected[i].modifier=0;
          }
        }
      }
    }
  }

  /* turn this into a crash file by re-writing the control block */
  rent.rentcode = RENT_CRASH;
  rent.time = time(0);
  rewind(fl);
  Crash_write_rentcode(ch, fl, &rent);

  fclose(fl);

  if (get_filename(GET_NAME(ch), fname, SAVEMAIL_FILE)) {
    if (!(fl = fopen(fname, "r+b"))) {
      if (errno != ENOENT) {	/* if it fails, NOT because of no file */
        sprintf(buf1, "SYSERR: READING SAVED MAIL FILE %s", fname);
        perror(buf1);
      }
    }
    else {
      while(!feof(fl)) {
        message_length=0;
        while(!feof(fl)) {
          fread(&tmp_message[message_length], sizeof(char), 1, fl);
          if(feof(fl)) {
            tmp_message[message_length]=0;
            break;
          }
          if(!tmp_message[message_length])
            break;
          message_length++;
          if(message_length >= (MAX_STRING_LENGTH-1)) {
            tmp_message[MAX_STRING_LENGTH-1]=0;
            break;
          }
        }
        if(message_length) {
          o = create_obj();
          o->item_number = NOTHING;
          o->name = str_dup("mail paper letter");
          o->short_description = str_dup("a piece of mail");
          o->description = str_dup("Someone has left a piece of mail here.");
          GET_OBJ_TYPE(o) = ITEM_NOTE;
          GET_OBJ_WEAR(o) = ITEM_WEAR_TAKE | ITEM_WEAR_HOLD;
          GET_OBJ_WEIGHT(o) = 1;
          GET_OBJ_COST(o) = 30;
          GET_OBJ_RENT(o) = 10;
          o->action_description = str_dup(tmp_message);
          obj_to_char(o, ch);
        }
      }
      fclose(fl);
      unlink(fname);
    }
  }

  if ((orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO))
    return 0;
  else
    return 1;
}

void Crash_quietload(struct char_data * ch)
{
  FILE *fl;
  char fname[MAX_STRING_LENGTH];
  char tmp_message[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct obj_data *o;
  struct rent_info rent;
  int message_length;

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return;
  if (!(fl = fopen(fname, "r+b"))) {
    return;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      perror("Reading crash file: Crash_load.");
      fclose(fl);
      return;
    }
    if (!feof(fl)) {
      if(!(o=Obj_from_store(object)))
        continue;
      if(o->worn_on > -1) {
        if(!CAN_WEAR(o, wear_bitv[o->worn_on])) {
          o->worn_on = -1;
          obj_to_char(o, ch);
        }
        else if(GET_EQ(ch, o->worn_on)) {
          o->worn_on = -1;
          obj_to_char(o, ch);
        }
        else if((o->worn_on == WEAR_LIGHT) && (GET_OBJ_TYPE(o) != ITEM_LIGHT)) {
          o->worn_on = -1;
          obj_to_char(o, ch);
        }
        else
          equip_char(ch, o, o->worn_on);
      }
      else
        obj_to_char(o, ch);
    }
  }

  if (get_filename(GET_NAME(ch), fname, SAVEMAIL_FILE)) {
    if (!(fl = fopen(fname, "r+b"))) {
      if (errno != ENOENT) {	/* if it fails, NOT because of no file */
        sprintf(buf1, "SYSERR: READING SAVED MAIL FILE %s", fname);
        perror(buf1);
      }
    }
    else {
      while(!feof(fl)) {
        message_length=0;
        while(!feof(fl)) {
          fread(&tmp_message[message_length], sizeof(char), 1, fl);
          if(feof(fl)) {
            tmp_message[message_length]=0;
            break;
          }
          if(!tmp_message[message_length])
            break;
          message_length++;
          if(message_length >= (MAX_STRING_LENGTH-1)) {
            tmp_message[MAX_STRING_LENGTH-1]=0;
            break;
          }
        }
        if(message_length) {
          o = create_obj();
          o->item_number = NOTHING;
          o->name = str_dup("mail paper letter");
          o->short_description = str_dup("a piece of mail");
          o->description = str_dup("Someone has left a piece of mail here.");
          GET_OBJ_TYPE(o) = ITEM_NOTE;
          GET_OBJ_WEAR(o) = ITEM_WEAR_TAKE | ITEM_WEAR_HOLD;
          GET_OBJ_WEIGHT(o) = 1;
          GET_OBJ_COST(o) = 30;
          GET_OBJ_RENT(o) = 10;
          o->action_description = str_dup(tmp_message);
          obj_to_char(o, ch);
        }
      }
      fclose(fl);
      unlink(fname);
    }
  }
}

int savemail(FILE *fp, struct obj_data *o, int limit)
{
  int retv=0;

  if(o) {
    retv+=savemail(fp, o->next_content, limit);
    if(limit && (retv >= limit))
      return(retv);
    retv+=savemail(fp, o->contains, limit);
    if(limit && (retv >= limit))
      return(retv);
    if((o->item_number == NOTHING) && (!str_cmp(o->short_description, "a piece of mail"))) {
      if(fwrite(o->action_description, sizeof(char), strlen(o->action_description)+1, fp) != (strlen(o->action_description)+1)) {
        sprintf(buf1, "SYSERR: WRITING SAVED MAIL FILE");
        perror(buf1);
        return(retv);
      }
      retv++;
    }
  }
  return(retv);
}

int Crash_savemail(struct char_data *ch, int limit)
{
  FILE *fp;
  char fname[MAX_STRING_LENGTH];
  int retv;

  if (!get_filename(GET_NAME(ch), fname, SAVEMAIL_FILE)) {
    sprintf(buf1, "SYSERR: GETTING MAIL NAME FOR %s", GET_NAME(ch));
    perror(buf1);
    return 0;
  }
  if (!(fp = fopen(fname, "wb"))) {
    sprintf(buf1, "SYSERR: CREATING SAVED MAIL FILE %s", fname);
    perror(buf1);
    return 0;
  }
  retv=savemail(fp, ch->carrying, limit);
  fclose(fp);
  if(!retv)
    unlink(fname);
  return(retv);
}


void Crash_remove_mail(struct obj_data *o, struct obj_data **mail_list)
{
  struct char_data *tch=NULL;
  struct obj_data *tobj=NULL;

  if(o) {
    Crash_remove_mail(o->next_content, mail_list);
    Crash_remove_mail(o->contains, mail_list);
    if((o->item_number == NOTHING) && (!str_cmp(o->short_description, "a piece of mail"))) {
      if(o->in_obj) {
        tobj=o->in_obj;
        obj_from_obj(o);
        o->in_obj=tobj;
        o->next_content=(*mail_list);
        *mail_list=o;
      }
      else {
        tch=o->carried_by;
        obj_from_char(o);
        o->carried_by=tch;
        o->next_content=(*mail_list);
        *mail_list=o;
      }
    }
  }
}


void Crash_restore_mail(struct obj_data **mail_list)
{
  struct char_data *tch=NULL;
  struct obj_data *tobj=NULL;
  struct obj_data *next;

  while(*mail_list) {
    next=(*mail_list)->next_content;
    (*mail_list)->next_content=NULL;
    if((*mail_list)->in_obj) {
      tobj=(*mail_list)->in_obj;
      (*mail_list)->in_obj=NULL;
      obj_to_obj((*mail_list), tobj);
    }
    else {
      tch=(*mail_list)->carried_by;
      (*mail_list)->carried_by=NULL;
      obj_to_char((*mail_list), tch);
    }
    (*mail_list)=next;
  }
}


int Crash_save(struct obj_data * obj, FILE * fp)
{
  struct obj_data *tmp;
  int result;

  if (obj) {
    Crash_save(obj->contains, fp);
    Crash_save(obj->next_content, fp);
    result = Obj_to_store(obj, fp);

    for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

    if (!result)
      return 0;
  }
  return TRUE;
}


void Crash_restore_weight(struct obj_data * obj)
{
  if (obj) {
    Crash_restore_weight(obj->contains);
    Crash_restore_weight(obj->next_content);
    if (obj->in_obj)
      GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
  }
}



void Crash_extract_objs(struct obj_data * obj)
{
  if (obj) {
    Crash_extract_objs(obj->contains);
    Crash_extract_objs(obj->next_content);
    extract_obj(obj);
  }
}


int Crash_is_unrentable(struct obj_data * obj)
{
  if (!obj)
    return 0;

  if (IS_OBJ_STAT(obj, ITEM_NORENT) || GET_OBJ_RENT(obj) < 0 ||
      GET_OBJ_RNUM(obj) <= NOTHING || GET_OBJ_TYPE(obj) == ITEM_KEY)
    return 1;

  return 0;
}


void Crash_extract_norents(struct obj_data * obj)
{
  if (obj) {
    Crash_extract_norents(obj->contains);
    Crash_extract_norents(obj->next_content);
    if (Crash_is_unrentable(obj))
      extract_obj(obj);
  }
}


void Crash_extract_expensive(struct obj_data * obj)
{
  struct obj_data *tobj, *max;

  max = obj;
  for (tobj = obj; tobj; tobj = tobj->next_content)
    if (GET_OBJ_RENT(tobj) > GET_OBJ_RENT(max))
      max = tobj;
  extract_obj(max);
}



void Crash_calculate_rent(struct obj_data * obj, int *cost)
{
  if (obj) {
    *cost += MAX(0, GET_OBJ_RENT(obj));
    Crash_calculate_rent(obj->contains, cost);
    Crash_calculate_rent(obj->next_content, cost);
  }
}


void Crash_crashsave(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  Crash_savemail(ch, 0);

  equip_reset(ch->carrying);

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  rent.rentcode = RENT_CRASH;
  rent.time = time(0);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp)) {
    fclose(fp);
    return;
  }
  Crash_restore_weight(ch->carrying);

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j)) {
      if (!Crash_save(GET_EQ(ch, j), fp)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch, j));
    }
  fclose(fp);
  REMOVE_BIT(PLR_FLAGS(ch), PLR_CRASH);
}


void Crash_idlesave(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  int cost;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j))
      obj_to_char(unequip_char(ch, j), ch);

  Crash_extract_norents(ch->carrying);

  cost = 0;
  Crash_calculate_rent(ch->carrying, &cost);
  cost <<= 1;			/* forcerent cost is 2x normal rent */
  while ((cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) && ch->carrying) {
    Crash_extract_expensive(ch->carrying);
    cost = 0;
    Crash_calculate_rent(ch->carrying, &cost);
    cost <<= 1;
  }

  if (!ch->carrying) {
    fclose(fp);
    Crash_delete_file(GET_NAME(ch));
    return;
  }
  rent.net_cost_per_diem = cost;

  rent.rentcode = RENT_TIMEDOUT;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void Crash_rentsave(struct char_data * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  struct obj_data *o;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  equip_reset(ch->carrying);

  for (j = 0; j < NUM_WEARS; j++) {
    if (GET_EQ(ch, j)) {
      obj_to_char(o=unequip_char(ch, j), ch);
      o->worn_on=j;
    }
  }

  Crash_extract_norents(ch->carrying);

  rent.net_cost_per_diem = cost;
  rent.rentcode = RENT_RENTED;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void Crash_cryosave(struct char_data * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j))
      obj_to_char(unequip_char(ch, j), ch);

  Crash_extract_norents(ch->carrying);

  GET_GOLD(ch) = MAX(0, GET_GOLD(ch) - cost);

  rent.rentcode = RENT_CRYO;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  rent.net_cost_per_diem = 0;
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
  SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
}


/* ************************************************************************
* Routines used for the receptionist					  *
************************************************************************* */

void Crash_rent_deadline(struct char_data * ch, struct char_data * recep,
			      long cost)
{
  long rent_deadline;

  if (!cost)
    return;

  rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / cost);
  sprintf(buf,
      "$n tells you, 'You can rent for %ld day%s with the gold you have\r\n"
	  "on hand and in the bank.'\r\n",
	  rent_deadline, (rent_deadline > 1) ? "s" : "");
  act(buf, FALSE, recep, 0, ch, TO_VICT);
}

int Crash_report_unrentables(struct char_data * ch, struct obj_data * obj)
{
  char buf[128];
  int has_norents = 0;

  if (obj) {
    if (Crash_is_unrentable(obj)) {
      has_norents = 1;
      sprintf(buf, "You cannot store %s.", OBJS(obj, ch));
      send_to_char(buf, ch);
    }
    has_norents += Crash_report_unrentables(ch, obj->contains);
    has_norents += Crash_report_unrentables(ch, obj->next_content);
  }
  return (has_norents);
}



void Crash_report_rent(struct char_data * ch, struct char_data * recep,
		            struct obj_data * obj, long *cost, long *nitems, int display, int factor)
{
  static char buf[256];

  if (obj) {
    if (!Crash_is_unrentable(obj)) {
      (*nitems)++;
      *cost += MAX(0, (GET_OBJ_RENT(obj) * factor));
      if (display) {
	sprintf(buf, "$n tells you, '%5d coins for %s..'",
		(GET_OBJ_RENT(obj) * factor), OBJS(obj, ch));
	act(buf, FALSE, recep, 0, ch, TO_VICT);
      }
    }
    Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor);
    Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
  }
}



int Crash_offer_rent(struct char_data * ch, struct char_data * receptionist,
		         int display, int factor)
{
  extern int max_obj_save;	/* change in config.c */
  char buf[MAX_INPUT_LENGTH];
  int i;
  long totalcost = 0, numitems = 0, norent = 0;

  norent = Crash_report_unrentables(ch, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    norent += Crash_report_unrentables(ch, GET_EQ(ch, i));

  if (norent)
    return 0;

  totalcost = min_rent_cost * factor;

  Crash_report_rent(ch, receptionist, ch->carrying, &totalcost, &numitems, display, factor);

  for (i = 0; i < NUM_WEARS; i++)
    Crash_report_rent(ch, receptionist, GET_EQ(ch, i), &totalcost, &numitems, display, factor);

  if (!numitems) {
    act("$n tells you, 'But you are not carrying anything!  Just quit!'",
	FALSE, receptionist, 0, ch, TO_VICT);
    return (0);
  }
  if (numitems > max_obj_save) {
    sprintf(buf, "$n tells you, 'Sorry, but I cannot store more than %d items.'",
	    max_obj_save);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    return (0);
  }
  if (display) {
    sprintf(buf, "$n tells you, 'Plus, my %d coin fee..'",
	    min_rent_cost * factor);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    sprintf(buf, "$n tells you, 'For a total of %ld coins%s.'",
	    totalcost, (factor == RENT_FACTOR ? " per day" : ""));
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    if (totalcost > GET_GOLD(ch)) {
      act("$n tells you, '...which I see you can't afford.'",
	  FALSE, receptionist, 0, ch, TO_VICT);
      return (0);
    } else if (factor == RENT_FACTOR)
      Crash_rent_deadline(ch, receptionist, totalcost);
  }
  return (totalcost);
}



int gen_receptionist(struct char_data * ch, struct char_data * recep,
		         int cmd, char *arg, int mode)
{
  int cost = 0;
  extern int free_rent;
  sh_int save_room;
  char *action_table[] = {"smile", "dance", "sigh", "blush", "burp",
  "cough", "fart", "twiddle", "yawn"};

  ACMD(do_action);

  if (!ch->desc || IS_NPC(ch))
    return FALSE;

  if (!cmd && !number(0, 5)) {
    do_action(recep, "", find_command(action_table[number(0, 8)]), 0);
    return FALSE;
  }
  if (!CMD_IS("offer") && !CMD_IS("rent"))
    return FALSE;
  if (!AWAKE(recep)) {
    send_to_char("She is unable to talk to you...\r\n", ch);
    return TRUE;
  }
  if (!CAN_SEE(recep, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return TRUE;
  }
  if (free_rent) {
    act("$n tells you, 'Rent is free here.  Just quit, and your objects will be saved!'",
	FALSE, recep, 0, ch, TO_VICT);
    return 1;
  }
  if (CMD_IS("rent")) {
    if (!(cost = Crash_offer_rent(ch, recep, FALSE, mode)))
      return TRUE;
    if (mode == RENT_FACTOR)
      sprintf(buf, "$n tells you, 'Rent will cost you %d gold coins per day.'", cost);
    else if (mode == CRYO_FACTOR)
      sprintf(buf, "$n tells you, 'It will cost you %d gold coins to be frozen.'", cost);
    act(buf, FALSE, recep, 0, ch, TO_VICT);
    if (cost > GET_GOLD(ch)) {
      act("$n tells you, '...which I see you can't afford.'",
	  FALSE, recep, 0, ch, TO_VICT);
      return TRUE;
    }
    if (cost && (mode == RENT_FACTOR))
      Crash_rent_deadline(ch, recep, cost);

    if (mode == RENT_FACTOR) {
      act("$n stores your belongings and helps you into your private chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_rentsave(ch, cost);
      sprintf(buf, "%s has rented (%d/day, %ld tot.)", GET_NAME(ch),
	      cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
    } else {			/* cryo */
      act("$n stores your belongings and helps you into your private chamber.\r\n"
	  "A white mist appears in the room, chilling you to the bone...\r\n"
	  "You begin to lose consciousness...",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_cryosave(ch, cost);
      sprintf(buf, "%s has cryo-rented.", GET_NAME(ch));
      SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
    }

    mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(ch)), TRUE);
    act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
    save_room = ch->in_room;
    extract_char(ch);
    save_char(ch, save_room);
  } else {
    Crash_offer_rent(ch, recep, TRUE, mode);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  return TRUE;
}


SPECIAL(receptionist)
{
  return (gen_receptionist(ch, me, cmd, argument, RENT_FACTOR));
}


SPECIAL(cryogenicist)
{
  return (gen_receptionist(ch, me, cmd, argument, CRYO_FACTOR));
}


void Crash_save_all(void)
{
  struct descriptor_data *d;
  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
	Crash_crashsave(d->character);
	save_char(d->character, NOWHERE);
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_CRASH);
      }
    }
  }
}

void Crash_number_rentables(struct obj_data * obj, int *n)
{
  if (obj) {
    if(!Crash_is_unrentable(obj))
      (*n)+=1;
    Crash_number_rentables(obj->contains, n);
    Crash_number_rentables(obj->next_content, n);
  }
}

void Crash_extract_least(struct obj_data * obj)
{
  struct obj_data *tobj, *min;

  min = obj;
  for (tobj = obj; tobj; tobj = tobj->next_content) {
    if (GET_OBJ_COST(tobj) < GET_OBJ_COST(min))
      min = tobj;
    else if(GET_OBJ_COST(tobj) == GET_OBJ_COST(min))
      if(GET_OBJ_RNUM(tobj) < GET_OBJ_RNUM(min))
        min = tobj;
  }
  extract_obj(min);
}

struct obj_data * return_least_nomark(struct obj_data *min, struct obj_data *obj)
{
  if(((GET_OBJ_COST(min) > GET_OBJ_COST(obj)) && (!IS_OBJ_STAT(obj, ITEM_NOQUIT))) ||
     IS_OBJ_STAT(min, ITEM_NOQUIT))
    min=obj;
  else if((GET_OBJ_COST(min) == GET_OBJ_COST(obj)) && (!IS_OBJ_STAT(obj, ITEM_NOQUIT)))
    if(GET_OBJ_RNUM(obj) < GET_OBJ_RNUM(min))
    min=obj;
  if(obj->contains)
    min=return_least_nomark(min, obj->contains);
  if(obj->next_content)
    min=return_least_nomark(min, obj->next_content);
  return(min);
}

void Crash_report_mark_least(struct char_data *ch)
{
  int i;
  char buf[128];
  struct obj_data *min=NULL;

  if(ch->carrying)
    min=return_least_nomark(ch->carrying, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++) {
    if(GET_EQ(ch, i)) {
      if(!min)
        min=GET_EQ(ch, i);
      min=return_least_nomark(min, GET_EQ(ch, i));
    }
  }
  GET_OBJ_EXTRA(min) |= ITEM_NOQUIT;
  sprintf(buf, "%s will be junked with qquit (excess item).\r\n", OBJS(min, ch));
  send_to_char(buf, ch);
}

void Crash_unmark(struct obj_data * obj)
{
  if (obj) {
    REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_NOQUIT);
    Crash_unmark(obj->contains);
    Crash_unmark(obj->next_content);
  }
}

int Crash_report_mark_norents(struct char_data * ch, struct obj_data * obj)
{
  char buf[128];
  int has_norents = 0;

  if (obj) {
    if (Crash_is_unrentable(obj)) {
      has_norents = 1;
      sprintf(buf, "%s will be junked with qquit (!rent item).\r\n", OBJS(obj, ch));
      send_to_char(buf, ch);
      GET_OBJ_EXTRA(obj) |= ITEM_NOQUIT;
    }
    has_norents += Crash_report_mark_norents(ch, obj->contains);
    has_norents += Crash_report_mark_norents(ch, obj->next_content);
  }
  return (has_norents);
}

int Crash_report_norents(struct char_data * ch, struct obj_data * obj)
{
  char buf[128];
  int has_norents = 0;

  if (obj) {
    if (Crash_is_unrentable(obj)) {
      has_norents = 1;
      sprintf(buf, "You cannot store %s.\r\n", OBJS(obj, ch));
      send_to_char(buf, ch);
    }
    has_norents += Crash_report_norents(ch, obj->contains);
    has_norents += Crash_report_norents(ch, obj->next_content);
  }
  return (has_norents);
}

int Crash_quitrentsave(struct char_data * ch, int cost)
{
  int n, m, i, norent;
  struct obj_data *mail=NULL;
  extern int max_obj_save;

  m=Crash_savemail(ch, 0);

  equip_reset(ch->carrying);

  Crash_remove_mail(ch->carrying, &mail);
  n=0;
  Crash_number_rentables(ch->carrying, &n);
  for (i = 0; i < NUM_WEARS; i++)
    Crash_number_rentables(GET_EQ(ch, i), &n);

  norent=Crash_report_norents(ch, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    norent+=Crash_report_norents(ch, GET_EQ(ch, i));

  if(n > max_obj_save) {
    sprintf(buf, "You have %d too many items (not counting unrentables).\r\n", n-max_obj_save);
    send_to_char(buf, ch);
    send_to_char("To automaticaly junk !rent and excess items by least value, use QQUIT.\r\n", ch);
    Crash_restore_mail(&mail);
    return 0;
  }

  if(m > mail_limit(GET_LEVEL(ch))) {
    sprintf(buf, "You have %d pieces of mail, you can only store %d.\r\nTo automaticaly junk mail, use QQUIT.", m, mail_limit(GET_LEVEL(ch)));
    send_to_char(buf, ch);
    Crash_restore_mail(&mail);
    return 0;
  }

  if(norent>0) {
    send_to_char("To automaticaly junk !rent items, use QQUIT.\r\n", ch);
    Crash_restore_mail(&mail);
    return 0;
  }

  Crash_rentsave(ch, cost);
  Crash_restore_mail(&mail);
  Crash_extract_norents(ch->carrying);
  return 1;
}

int Crash_empty_containers(struct char_data *ch, struct obj_data *o)
{
  struct obj_data *obj, *obj2;

  for(obj=o; obj; obj=obj->next_content) {
    if(obj->contains) {
      while((obj2=obj->contains)) {
        obj_from_obj(obj2);
        obj_to_char(obj2, ch);
      }
      return 1;
    }
  }
  return 0;
}

void Crash_forcerentsave(struct char_data *ch, int cost)
{
  int n=0, i;
  struct obj_data *o;
  extern int max_obj_save;

  equip_reset(ch->carrying);

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      obj_to_char(o=unequip_char(ch, i), ch);
      o->worn_on=i;
    }
  }

  while(Crash_empty_containers(ch, ch->carrying));

  Crash_savemail(ch, mail_limit(GET_LEVEL(ch)));

  Crash_extract_norents(ch->carrying);

  Crash_number_rentables(ch->carrying, &n);

  if(n > max_obj_save) {
    for(i=0; i < n-max_obj_save; i++) {
      Crash_extract_least(ch->carrying);
    }
  }

  Crash_rentsave(ch, cost);
  return;
}

int Crash_reportrentsave(struct char_data * ch, int cost)
{
  int n, m, i, norent, nosave=0;
  struct obj_data *mail=NULL;
  extern int max_obj_save;

  m=Crash_savemail(ch, 0);

  equip_reset(ch->carrying);

  Crash_remove_mail(ch->carrying, &mail);
  n=0;
  Crash_number_rentables(ch->carrying, &n);
  for (i = 0; i < NUM_WEARS; i++)
    Crash_number_rentables(GET_EQ(ch, i), &n);

  norent=Crash_report_mark_norents(ch, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    norent+=Crash_report_mark_norents(ch, GET_EQ(ch, i));

  if(n > max_obj_save) {
    for(i=0; i < n-max_obj_save; i++) {
      Crash_report_mark_least(ch);
    }
    nosave=1;
  }

  if(m > mail_limit(GET_LEVEL(ch))) {
    sprintf(buf, "You have %d pieces of mail, you can only store %d.\r\nThe last %d will be junked.",
            m, mail_limit(GET_LEVEL(ch)), m-mail_limit(GET_LEVEL(ch)));
    send_to_char(buf, ch);
    nosave=1;
  }

  if(norent>0) {
    nosave=1;
  }

  Crash_unmark(ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    Crash_unmark(GET_EQ(ch, i));

  if(nosave) {
    Crash_restore_mail(&mail);
    return 0;
  }

  Crash_rentsave(ch, cost);
  Crash_restore_mail(&mail);
  Crash_extract_norents(ch->carrying);
  return 1;
}

/*
 * Functions needed for reimb system
 */

void reimb_delete(char *name)
{
  char filename[50];

  if (!get_filename(name, filename, REIMB_FILE))
    return;

  if (unlink(filename) < 0) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting reimb file %s", filename);
      perror(buf1);
    }
  }
  return;
}


int reimb_save(struct obj_data * obj, FILE * fp)
{
  struct obj_data *tmp;
  int result;

  if (obj) {
    reimb_save(obj->contains, fp);
    reimb_save(obj->next_content, fp);
    if(Crash_is_unrentable(obj))
      result=1;
    else
      result = Obj_to_store(obj, fp);

    for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

    if (!result)
      return 0;
  }
  return TRUE;
}

/* returns 1 on success, 0 on fail */
int reimb_make(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return 0;

  if (!get_filename(GET_NAME(ch), buf, REIMB_FILE))
    return 0;

  if (!(fp = fopen(buf, "wb")))
    return 0;

  rent.net_cost_per_diem = 0;
  rent.rentcode = RENT_RENTED;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return 0;
  }
  if (!reimb_save(ch->carrying, fp)) {
    fclose(fp);
    return 0;
  }
  Crash_restore_weight(ch->carrying);
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j)) {
      if(!reimb_save(GET_EQ(ch, j), fp)) {
        fclose(fp);
        return 0;
      }
      Crash_restore_weight(GET_EQ(ch, j));
    }

  fclose(fp);
  return 1;

}


int reimb_load(struct char_data *ch, struct char_data *reimber, int force)
/* return values:
	0 - successful reimb
	1 - no reimb because of error or requires freimb
*/
{

  FILE *fl, *fl2;
  char fname[MAX_STRING_LENGTH];
  char fname2[]="tempreimb.reimb";
  int i;
  struct obj_file_elem object;
  struct obj_data *o;
  struct rent_info rent;
  extern time_t boot_time;

  if (!get_filename(GET_NAME(ch), fname, REIMB_FILE))
    return 1;
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: REIMB: READING OBJECT FILE %s (5)", fname);
      perror(buf1);
      sprintf(buf, "There was a fatal error opening %s's reimb file.\r\n", GET_NAME(ch));
      send_to_char(buf, reimber);
    }
    else {
      sprintf(buf, "%s has no reimb file.\r\n", GET_NAME(ch));
      send_to_char(buf, reimber);
    }
    return 1;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if((rent.time >= boot_time)&&(!force)) {
    fclose(fl);
    send_to_char("not reimbed: reimb creation time later than mud boot time\r\n", reimber);
    return 1;
  }

  if(!(fl2 = fopen(fname2, "wb"))) {
    fclose(fl);
    log("SYSERR: COULD NOT OPEN REIMB TEMP FILE");
    send_to_char("not reimbed: could not open temp file\r\n", reimber);
    return 1;
  }

  if(ch->carrying && (!force)) {
    fclose(fl);
    fclose(fl2);
    send_to_char("not reimbed: player is carrying equipment\r\n", reimber);
    return 1;
  }
  for(i=0; i<NUM_WEARS; i++) {
    if(GET_EQ(ch, i) && (!force)) {
      fclose(fl);
      fclose(fl2);
      send_to_char("not reimbed: player is carrying equipment\r\n", reimber);
      return 1;
    }
  }

  GET_GOLD(ch)+=rent.gold;

  rent.gold=0;
  fwrite(&rent, sizeof(struct rent_info), 1, fl2);

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      perror("Reading reimb file: reimb_load.");
      fclose(fl);
      fclose(fl2);
      unlink(fname2);
      return 1;
    }
    if (!feof(fl)) {
      if(!(o=Obj_from_store(object)))
        continue;
      if((Crash_is_unrentable(o))&&(!force)) {
        sprintf(buf, "%s [%d] not reimbed: !rent item", o->short_description, GET_OBJ_VNUM(o));
        send_to_char(buf, reimber);
        Obj_to_store(o, fl2);
        extract_obj(o);
      }
      else if(((GET_OBJ_VNUM(o)<=zone_table[real_zone(12)].top)&&(GET_OBJ_VNUM(o)>=zone_table[real_zone(12)].bottom)&&(GET_LEVEL(ch)<LVL_HERO))&&(!force)) {
        sprintf(buf, "%s [%d] not reimbed: zone 12 item", o->short_description, GET_OBJ_VNUM(o));
        send_to_char(buf, reimber);
        Obj_to_store(o, fl2);
        extract_obj(o);
      }
      else if(IS_OBJ_STAT(o, ITEM_DUPE)&&(!force)) {
        sprintf(buf, "%s [%d] not reimbed: would create a duped item", o->short_description, GET_OBJ_VNUM(o));
        send_to_char(buf, reimber);
        Obj_to_store(o, fl2);
        extract_obj(o);
      }
      else {
        o->worn_on=-1;
        obj_to_char(o, ch);
        if(GET_LEVEL(ch) >= LVL_HERO) {
          o->obj_flags.value[0]=0;
          o->obj_flags.value[1]=0;
          o->obj_flags.value[2]=0;
          o->obj_flags.value[3]=0;
          o->obj_flags.type_flag=13;
          o->obj_flags.cost=0;
          o->obj_flags.bitvector=0;
          o->obj_flags.immune=0;
          o->obj_flags.resist=0;
          o->obj_flags.weak=0;
          for(i=0; i<MAX_OBJ_AFFECT; i++) {
            o->affected[i].location=APPLY_NONE;
            o->affected[i].modifier=0;
          }
        }
        if(IS_OBJ_STAT(o, ITEM_DUPE)) {
          sprintf(buf, "WARNING: %s [%d] may create a duped item", o->short_description, GET_OBJ_VNUM(o));
          send_to_char(buf, reimber);
        }
        sprintf(buf, "  %s reimbing %s: %s [%d]", GET_NAME(reimber), GET_NAME(ch), o->short_description, GET_OBJ_VNUM(o));
        log(buf);
      }
    }
  }
  fclose(fl);
  fclose(fl2);
  rename(fname2, fname);
  Crash_crashsave(ch);
  return 0;
}

void Crash_listreimb(struct char_data * ch, char *name)
{
  FILE *fl;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  int overflow=0;
  struct obj_file_elem object;
  struct obj_data *obj;
  struct rent_info rent;

  if (!get_filename(name, fname, REIMB_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no reimb file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  strcat(buf, ctime(&rent.time));
  strcat(buf, "\r");
  sprintf(buf, "%s %ld coins\r\n", buf, rent.gold);
  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      return;
    }
    if (!feof(fl))
      if (real_object(object.item_number) > -1) {
	obj = read_object(object.item_number, VIRTUAL);
        if(strlen(buf) > MAX_STRING_LENGTH-120) {
          if(!overflow) {
            strcat(buf, "***OVERFLOW***\r\n");
            overflow=1;
          }
        }
        else {
          sprintf(buf, "%s [%5d] (%5dau) %-20s id:%ld%s\r\n", buf,
                  object.item_number, GET_OBJ_COST(obj),
                  obj->short_description, object.id,
                  (IS_SET(object.extra_flags, ITEM_DUPE)?" <DUPE>":""));
        }
	extract_obj(obj);
      }
  }
  page_string(ch->desc, buf, 1);

  fclose(fl);
}


ACMD(do_extract)
{
  FILE *fl, *fp;
  char name[MAX_INPUT_LENGTH];
  char number[MAX_INPUT_LENGTH];
  char fname[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct obj_data *o=NULL;
  struct rent_info rent;
  struct char_file_u tmp_store;
  struct char_data *cbuf=NULL;
  int found=0;
  long id;

  two_arguments(argument, name, number);

  if((!*name) || (!*number) || (!is_number(number))) {
    send_to_char("Usage: extract <player> <item_id>\r\n", ch);
    return;
  }

  CREATE(cbuf, struct char_data, 1);
  clear_char(cbuf);
  if (load_char(name, &tmp_store) > -1) {
    store_to_char(&tmp_store, cbuf);
    if(!IMM_AFF_IMM(ch, cbuf, TRUE)) {
      free_char(cbuf);
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    free_char(cbuf);
  }
  else {
    send_to_char("No such player.\r\n", ch);
    return;
  }

  if(get_player(name)) {
    send_to_char("You cannot extract an item from someone who is playing.\r\n", ch);
    return;
  }

  id=atol(number);

  if (!get_filename(name, fname, CRASH_FILE)) {
    send_to_char("Error getting rent file.\r\n", ch);
    return;
  }

  if (!(fl = fopen(fname, "rb"))) {
    if (errno != ENOENT)
      send_to_char("Error opening rent file.\r\n", ch);
    else {
      sprintf(buf, "%s has no rent file.\r\n", name);
      send_to_char(buf, ch);
    }
    return;
  }

  if(!(fp=fopen("extract.objs", "wb"))) {
    send_to_char("Could not create temporary file.\r\n", ch);
    fclose(fl);
    return;
  }

  if (!feof(fl)) {
    fread(&rent, sizeof(struct rent_info), 1, fl);
    fwrite(&rent, sizeof(struct rent_info), 1, fp);
  }

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      perror("Reading crash file: Crash_load.");
      fclose(fl);
      fclose(fp);
      return;
    }
    if (!feof(fl)) {
      if((object.id == id) && (!found) && (o=Obj_from_store(object))) {
        o->worn_on=-1;
        obj_to_char(o, ch);
        found=1;
        sprintf(buf, "(GC) %s extracted item %d, ID %ld, from %s", GET_NAME(ch), GET_OBJ_VNUM(o), o->id, name);
        mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
      }
      else {
        fwrite(&object, sizeof(struct obj_file_elem), 1, fp);
      }
    }
  }

  if(!o) {
    sprintf(buf, "%s doesn't have any item with that ID.\r\n", name);
    send_to_char(buf, ch);
  }
  else
    send_to_char(OK, ch);

  fclose(fl);
  fclose(fp);
  unlink(fname);
  rename("extract.objs", fname);
}
