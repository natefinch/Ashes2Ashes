/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "interpreter.h"
#include "comm.h"
#include "spells.h"

int mobat=0;
int mprog_disable=0;

extern struct index_data *mob_index;
extern struct room_data *world;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern char *spells[];
extern struct spell_info_type spell_info[];
extern char *npc_class_types[];

/*
 * Local functions.
 */

char *			mprog_type_to_name	( int type );

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */

/* external functions */
sh_int find_target_room(struct char_data * ch, char *rawroomstr);

ACMD(do_cast);

char *mprog_type_to_name( int type )
{
    switch ( type )
    {
    case IN_FILE_PROG:          return "in_file_prog";
    case ACT_PROG:              return "act_prog";
    case SPEECH_PROG:           return "speech_prog";
    case RAND_PROG:             return "rand_prog";
    case FIGHT_PROG:            return "fight_prog";
    case HITPRCNT_PROG:         return "hitprcnt_prog";
    case DEATH_PROG:            return "death_prog";
    case ENTRY_PROG:            return "entry_prog";
    case GREET_PROG:            return "greet_prog";
    case ALL_GREET_PROG:        return "all_greet_prog";
    case GIVE_PROG:             return "give_prog";
    case BRIBE_PROG:            return "bribe_prog";
    case COMMANDTRAP_PROG:     return "commandtrap_prog";
     case KEYWORD_PROG:        return "keyword_prog";
    default:                    return "ERROR_PROG";
    }
}

/* string prefix routine */

bool str_prefix(const char *astr, const char *bstr)
{
  if (!astr) {
    log("str_prefix: null astr.");
    return TRUE;
  }
  if (!bstr) {
    log("str_prefix: null bstr.");
    return TRUE;
  }
  for(; *astr; astr++, bstr++) {
    if(LOWER(*astr) != LOWER(*bstr)) return TRUE;
  }
  return FALSE;
}

ACMD(do_mpcast)
{
    char arg[MAX_INPUT_LENGTH];
    int level;
    int prev_level;
    
    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument(argument, arg);
    if (! *arg || !(level = atoi(arg)))
    {
	send_to_char( "Cast as what character level?\n\r", ch );
	return;
    }

    prev_level=GET_LEVEL(ch);
    GET_LEVEL(ch)=level;
    do_cast(ch, argument, 0, 0);
    GET_LEVEL(ch)=prev_level;
}

ACMD(do_mpscast)
{
    char arg[MAX_INPUT_LENGTH];
    int level;
    int prev_level;
    
    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument(argument, arg);
    if (! *arg || !(level = atoi(arg)))
    {
	send_to_char( "Cast as what character level?\n\r", ch );
	return;
    }

    prev_level=GET_LEVEL(ch);
    GET_LEVEL(ch)=level;
    do_cast(ch, argument, 0, 1);
    GET_LEVEL(ch)=prev_level;
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MOBprograms which are set.
 */

ACMD(do_mpstat)
{
    char buf[2*MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    struct char_data  *victim;

    if ( IS_AFFECTED(ch, AFF_CHARM))
      return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "MobProg stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room_vis( ch, arg ) ) == NULL ) {
        if ( ( victim = get_char_vis( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }

    if ( !IS_NPC( victim ) )
    {
	send_to_char( "Only Mobiles can have Programs!\n\r", ch);
	return;
    }

    if ( !( mob_index[victim->nr].progtypes ) )
    {
	send_to_char( "That Mobile has no Programs set.\n\r", ch);
	return;
    }

    sprintf( buf, "Name: %s.  Vnum: %d.\n\r",
	victim->player.name, mob_index[victim->nr].virtual );

    sprintf( buf1, "Short description: %s.\n\rLong  description: %s",
	    victim->player.short_descr,
	    victim->player.long_descr[0] != '\0' ?
	    victim->player.long_descr : "(none).\n\r" );
    strcat(buf, buf1);

    sprintf( buf1, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \n\r",
	victim->points.hit,         victim->points.max_hit,
	victim->points.mana,        victim->points.max_mana,
	victim->points.move,        victim->points.max_move );
    strcat(buf, buf1);

    sprintf( buf1,
	"Lv: %d, Class: %s, Align: %d, AC: %d, Gold: %ld, Exp: %ld.\n\r",
	GET_LEVEL(victim), npc_class_types[(int)GET_CLASS(victim)], GET_ALIGNMENT(victim),
	compute_ac(victim), GET_GOLD(victim), GET_EXP(victim));
    strcat(buf, buf1);

    for ( mprg = mob_index[victim->nr].mobprogs; mprg != NULL;
	 mprg = mprg->next )
    {
      sprintf( buf1, ">%s %s\n\r%s\n\r",
	      mprog_type_to_name( mprg->type ),
	      mprg->arglist,
	      mprg->comlist );
      strcat(buf, buf1);
    }

    page_string(ch->desc, buf, TRUE);

    return;
}

/* prints the argument to all the rooms aroud the mobile */
ACMD(do_mpasound)
{

  sh_int was_in_room;
  int door;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if (argument[0] == '\0' )
    {
        bug( "Mpasound - No argument: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }
    skip_spaces(&argument);

    was_in_room = ch->in_room;

    for(door=0; door<NUM_OF_DIRS; door++)
    {
      if (CAN_GO(ch, door)) {
        ch->in_room = world[was_in_room].dir_option[door]->to_room;
        MOBTrigger  = FALSE;
        act( argument, FALSE, ch, NULL, NULL, TO_ROOM );
        ch->in_room=was_in_room;
      }
    }

  ch->in_room = was_in_room;
  return;

}

/* lets the mobile kill any player or mobile without murder*/
ACMD(do_mpkill)
{
    struct descriptor_data *d;
    char      arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	bug( "MpKill - no argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                hit( ch, d->character, -1);
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL )
    {
	bug( "MpKill - Victim not in room: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if ( victim == ch )
    {
	bug( "MpKill - Bad victim to attack: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	bug( "MpKill - Charmed mob attacking master: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if((GET_LEVEL(victim)>=LVL_HERO) && (!IS_NPC(victim)) && (!PRF_FLAGGED(victim, PRF_AVTR)))
        return;

    hit( ch, victim, -1);
    return;
}

/* Jesse 2-20-97 - added #d#+# format to mpdamage */
ACMD(do_mpdamage)
{
    struct descriptor_data *d;
    char      arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;
    int dam, fighting, type=TYPE_SILENT;
    int	amount=-1, dice_type=1, bonus=0;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument=one_argument( argument, arg );

    if ((!*arg) || (*arg < '0') || (*arg > '9'))
    {
	bug( "MpDamage - bad damage argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    sscanf(arg, "%dd%d+%d", &amount, &dice_type, &bonus);
    if(amount<0) {
	bug( "MpDamage - bad damage argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    argument=one_argument(argument, arg);
    one_argument(argument, buf1);
    if(*buf1) {
      if(!strn_cmp("fire", buf1, strlen(buf1)))
        type=TYPE_SFIRE;
      else if(!strn_cmp("ice", buf1, strlen(buf1)))
        type=TYPE_SICE;
      else if(!strn_cmp("energy", buf1, strlen(buf1)))
        type=TYPE_SENERGY;
      else if(!strn_cmp("blunt", buf1, strlen(buf1)))
        type=TYPE_SBLUNT;
      else if(!strn_cmp("slash", buf1, strlen(buf1)))
        type=TYPE_SSLASH;
      else if(!strn_cmp("pierce", buf1, strlen(buf1)))
        type=TYPE_SPIERCE;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                dam=dice(amount, dice_type)+bonus;
                if(FIGHTING(d->character))
                  fighting=1;
                else
                  fighting=0;
                damage( ch, d->character, dam, type);
                if((!fighting) && mobat)
                  stop_fighting(d->character);
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL )
    {
	bug( "MpDamage - Victim not in room: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if ( victim == ch )
    {
	bug( "MpDamage - Bad victim to attack: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	bug( "MpDamage - Charmed mob attacking master: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    dam=dice(amount, dice_type)+bonus;
    if(FIGHTING(victim))
      fighting=1;
    else
      fighting=0;
    damage( ch, victim, dam, type);
    if((!fighting) && mobat)
      stop_fighting(victim);
    return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy 
   items using all.xxxxx or just plain all of them */
ACMD(do_mpjunk)
{
    char      arg[ MAX_INPUT_LENGTH ];
    int pos;
    struct obj_data *obj;
    struct obj_data *obj_next;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) ) {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0') {
        bug( "Mpjunk - No argument: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) ) {
      if ((obj=get_object_in_equip(arg,ch->equipment,&pos))!= NULL) {
	unequip_char( ch, pos);
	extract_obj( obj );
	return;
      }
      if ((obj = get_obj_in_list(arg, ch->carrying)) != NULL )
        extract_obj( obj );
      return;
    } else {
      for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
        obj_next = obj->next_content;
        if ( arg[3] == '\0' || isexact(arg+4, obj->name ) ) {
          extract_obj(obj);
        }
      }
      while((obj=get_object_in_equip(arg,ch->equipment,&pos))!=NULL){
          unequip_char(ch, pos);
          extract_obj(obj);
      }   
    }
    return;
}

/* prints the message to everyone in the room other than the mob and victim */
ACMD(do_mpechoaround)
{
  char       arg[ MAX_INPUT_LENGTH ];
  struct char_data *victim;
  char *p;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
       send_to_char( "Huh?\n\r", ch );
       return;
    }

    p=one_argument( argument, arg );
    while(isspace(*p)) p++; /* skip over leading space */

    if ( arg[0] == '\0' )
    {
       bug( "Mpechoaround - No argument:  vnum %d.", mob_index[ch->nr].virtual );
       return;
    }

    if ( !( victim=get_char_room(arg, ch->in_room) ) )
    {
        bug( "Mpechoaround - victim does not exist: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    MOBTrigger  = FALSE;
    act(p, FALSE, ch, NULL, victim, TO_NOTVICT );
    return;
}

/* prints the message to only the victim */
ACMD(do_mpechoat)
{
  char       arg[ MAX_INPUT_LENGTH ];
  struct char_data *victim;
  char *p;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
       send_to_char( "Huh?\n\r", ch );
       return;
    }

    p = one_argument( argument, arg );
    while(isspace(*p)) p++; /* skip over leading space */

    if ( arg[0] == '\0')
    {
       bug( "Mpechoat - No argument:  vnum %d.",
	   mob_index[ch->nr].virtual );
       return;
    }

    if ( !( victim = get_char_room(arg, ch->in_room) ) )
    {
        bug( "Mpechoat - victim does not exist: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if(ch==victim)
        MOBTrigger  = FALSE;
    act( p,FALSE,  ch, NULL, victim, TO_VICT );
    return;
}

/* prints the message to the room at large */
ACMD(do_mpecho)
{
    char *p;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC(ch) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if (ch->desc) 
	return;

    if ( argument[0] == '\0' )
    {
        bug( "Mpecho - called w/o argument: vnum %d.",
	    mob_index[ch->nr].virtual );
        return;
    }
    p = argument;
    while(isspace(*p)) p++;

    MOBTrigger  = FALSE;
    act(p,FALSE,  ch, NULL, NULL, TO_ROOM );
    return;

}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */
ACMD(do_mpmload)
{
    char arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	bug( "Mpmload - Bad vnum as arg: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    if (real_mobile(atoi(arg)) < 0)
    {
	bug( "Mpmload - Bad mob vnum: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    victim = read_mobile( atoi(arg), VIRTUAL);
    char_to_room( victim, ch->in_room );
    return;
}

ACMD(do_mpoload)
{
    char arg1[ MAX_INPUT_LENGTH ];
    struct obj_data *obj;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        bug( "Mpoload - Bad syntax: vnum %d.",
	    mob_index[ch->nr].virtual );
        return;
    }
 
    if (real_object(atoi(arg1)) < 0)
    {
	bug( "Mpoload - Bad vnum arg: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    obj = read_object( atoi(arg1), VIRTUAL);
    if ( CAN_WEAR(obj, ITEM_WEAR_TAKE) ) {
	obj_to_char( obj, ch );
    } else {
	obj_to_room( obj, ch->in_room );
    }

    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */
ACMD(do_mppurge)
{
    char       arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;
    struct obj_data  *obj;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        struct char_data *vnext;
        struct obj_data  *obj_next;

	for ( victim = world[ch->in_room].people; victim != NULL; victim = vnext )
	{
	  vnext = victim->next_in_room;
	  if ( IS_NPC( victim ) && victim != ch )
	    extract_char( victim);
	}

	for ( obj = world[ch->in_room].contents; obj != NULL; obj = obj_next )
	{
	  obj_next = obj->next_content;
	  extract_obj( obj );
	}

	return;
    }

    if( ( victim = get_char_room(arg, ch->in_room) ) ) {
      if ( !IS_NPC( victim ) ) {
        bug( "Mppurge - Purging a PC: vnum %d.", mob_index[ch->nr].virtual );
        return;
      }
      extract_char( victim);        
    }
    else {
      if ( ( obj = get_obj_in_list(arg, ch->carrying) ) ) {
        extract_obj( obj );
      }
      else {
        if((obj=get_obj_in_list(arg, world[ch->in_room].contents))) {
           extract_obj(obj);
        }
        else
           bug("Mppurge - Bad argument: vnum %d.",mob_index[ch->nr].virtual);
      }
    }

    return;
}


/* lets the mobile goto any location it wishes that is not private */
ACMD(do_mpgoto)
{
    char             arg[ MAX_INPUT_LENGTH ];
    sh_int location;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "Mpgoto - No argument: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    if ( ( location = find_target_room( ch, arg ) ) < 0 )
    {
	bug( "Mpgoto - No such location: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    if ( FIGHTING(ch) != NULL )
	stop_fighting( ch);

    char_from_room( ch );
    char_to_room( ch, location );

    return;
}

/* lets the mobile do a command at another location. Very useful */
ACMD(do_mpat)
{
    char             arg[ MAX_INPUT_LENGTH ];
    sh_int location;
    sh_int original;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
 
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpat - Bad argument: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    if ( ( location = find_target_room( ch, arg ) ) < 0)
    {
	bug( "Mpat - No such location: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    /*ch->desc->max_str = MAX_INPUT_LENGTH;*/
    mobat=1;
    command_interpreter( ch, argument );
    mobat=0;

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    if(ch->in_room == location) {
      char_from_room(ch);
      char_to_room( ch, original );
    }

    return;
}
 
/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */
ACMD(do_mptransfer)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    sh_int location;
    struct descriptor_data *d;
    struct char_data       *victim;

    ACMD(do_look);

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	bug( "Mptransfer - Bad syntax: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_target_room( ch, arg2 ) ) < 0)
	{
	    bug( "Mptransfer - No such location: vnum %d.",
	        mob_index[ch->nr].virtual );
	    return;
	}

	if ( IS_SET(world[location].room_flags, ROOM_PRIVATE) )
	{
	    bug( "Mptransfer - Private room: vnum %d.",
		mob_index[ch->nr].virtual );
	    return;
	}
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                if ( FIGHTING(d->character) != NULL )
                    stop_fighting(d->character);

                char_from_room( d->character );
                char_to_room( d->character, location );
                do_look (d->character, "\0", 0, 0);
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg1, ch->in_room) ) == NULL )
    {
	bug( "Mptransfer - No such person: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if ( victim->in_room == 0 )
    {
	bug( "Mptransfer - Victim in Limbo: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if((GET_LEVEL(victim) >= LVL_HERO) && (!IS_NPC(victim)) && (!PRF_FLAGGED(victim, PRF_AVTR)))
        return;

    if ( FIGHTING(victim) != NULL )
	stop_fighting( victim);

    char_from_room( victim );
    char_to_room( victim, location );
    do_look (victim, "\0", 0, 0);

    return;
}

ACMD(do_mpstransfer)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    sh_int location;
    struct descriptor_data *d;
    struct char_data       *victim;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	bug( "Mpstransfer - Bad syntax: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_target_room( ch, arg2 ) ) < 0)
	{
	    bug( "Mpstransfer - No such location: vnum %d.",
	        mob_index[ch->nr].virtual );
	    return;
	}

	if ( IS_SET(world[location].room_flags, ROOM_PRIVATE) )
	{
	    bug( "Mpstransfer - Private room: vnum %d.",
		mob_index[ch->nr].virtual );
	    return;
	}
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                if ( FIGHTING(d->character) != NULL )
                    stop_fighting(d->character);

                char_from_room( d->character );
                char_to_room( d->character, location );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg1, ch->in_room) ) == NULL )
    {
	bug( "Mpstransfer - No such person: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if ( victim->in_room == 0 )
    {
	bug( "Mpstransfer - Victim in Limbo: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if((GET_LEVEL(victim) >= LVL_HERO) && (!IS_NPC(victim)) && (!PRF_FLAGGED(victim, PRF_AVTR)))
        return;

    if ( FIGHTING(victim) != NULL )
	stop_fighting( victim);

    char_from_room( victim );
    char_to_room( victim, location );

    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */
ACMD(do_mpforce)
{
    char arg[ MAX_INPUT_LENGTH ];

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpforce - Bad syntax: vnum %d.", mob_index[ch->nr].virtual );
	return;
    }

    if ( !str_cmp( arg, "all" ) ) {
        struct descriptor_data *i;
        struct char_data *vch;

	for ( i = descriptor_list; i ; i = i->next) {
          if(i->character != ch && !i->connected &&
             i->character->in_room == ch->in_room) {
            vch = i->character;
            if((GET_LEVEL(vch) < LVL_HERO) || PRF_FLAGGED(vch, PRF_AVTR)) {
		command_interpreter( vch, argument );
	    }
	  }
        }
    } else {
	struct char_data *victim;

	if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL ) {
	    bug( "Mpforce - No such victim: vnum %d.",
	  	mob_index[ch->nr].virtual );
	    return;
	}

	if ( victim == ch ) {
	    bug( "Mpforce - Forcing oneself: vnum %d.",
	    	mob_index[ch->nr].virtual );
	    return;
	}

        if((IS_NPC(victim)) || (GET_LEVEL(victim)<LVL_HERO)|| PRF_FLAGGED(victim, PRF_AVTR)) {
          command_interpreter( victim, argument );
        }
    }

    return;
}

ACMD(do_mpset)
{
    struct descriptor_data *d;
    char arg[ MAX_INPUT_LENGTH ], varg[ MAX_INPUT_LENGTH ];
    int value;
    struct char_data *victim;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "all" ) )
    {
        argument = one_argument( argument, arg );
        if(!*arg) {
            bug( "Mpset - Invalid set field: vnum %d.",
                mob_index[ch->nr].virtual );
            return;
        }
        one_argument( argument, varg);
        if(!varg[0]) {
            bug( "Mpset - Missing value: vnum %d.",
                mob_index[ch->nr].virtual );
            return;
        }
        value=atoi(varg);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                victim=d->character;
                if(!str_cmp(arg, "hit")) {
                    if((value>1000)||(value<0)) {
                        bug( "Mpset - Invalid hit value: vnum %d.",
                            mob_index[ch->nr].virtual );
                    }
                    GET_HIT(victim)=MAX(MIN(value, 1000), 0);
                } else if(!str_cmp(arg, "mana")) {
                    if((value>1000)||(value<0)) {
                        bug( "Mpset - Invalid mana value: vnum %d.",
                            mob_index[ch->nr].virtual );
                    }
                    GET_MANA(victim)=MAX(MIN(value, 1000), 0);
                } else if(!str_cmp(arg, "move")) {
                    if((value>250)||(value<0)) {
                        bug( "Mpset - Invalid move value: vnum %d.",
                            mob_index[ch->nr].virtual );
                    }
                    GET_MOVE(victim)=MAX(MIN(value, 250), 0);
                } else if(!str_cmp(arg, "thirst")) {
                    if((value>24)||(value<0)) {
                       bug( "Mpset - Invalid thirst value: vnum %d.",
                            mob_index[ch->nr].virtual );
                    }
                    GET_COND(victim, 2)=MAX(MIN(value, 24), 0);
                } else if(!str_cmp(arg, "hunger")) {
                    if((value>24)||(value<0)) {
                        bug( "Mpset - Invalid hunger value: vnum %d.",
                            mob_index[ch->nr].virtual );
                    }
                    GET_COND(victim, 1)=MAX(MIN(value, 24), 0);
                }
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL ) {
        bug( "Mpset - No such victim: vnum %d.",
            mob_index[ch->nr].virtual );
        return;
    }

    if (IS_NPC(victim) || (GET_LEVEL(victim)<LVL_HERO) || PRF_FLAGGED(victim, PRF_AVTR)) {
        argument = one_argument( argument, arg );
        if(!*arg) {
            bug( "Mpset - Invalid set field: vnum %d.",
                mob_index[ch->nr].virtual );
            return;
        }
        one_argument( argument, varg);
        if(!varg[0]) {
            bug( "Mpset - Missing value: vnum %d.",
                mob_index[ch->nr].virtual );
            return;
        }
        value=atoi(varg);
        if(!str_cmp(arg, "hit")) {
            if((value>1000)||(value<0)) {
                bug( "Mpset - Invalid hit value: vnum %d.",
                    mob_index[ch->nr].virtual );
            }
            GET_HIT(victim)=MAX(MIN(value, 1000), 0);
        } else if(!str_cmp(arg, "mana")) {
            if((value>1000)||(value<0)) {
                bug( "Mpset - Invalid mana value: vnum %d.",
                    mob_index[ch->nr].virtual );
            }
            GET_MANA(victim)=MAX(MIN(value, 1000), 0);
        } else if(!str_cmp(arg, "move")) {
            if((value>250)||(value<0)) {
                bug( "Mpset - Invalid move value: vnum %d.",
                    mob_index[ch->nr].virtual );
            }
            GET_MOVE(victim)=MAX(MIN(value, 250), 0);
        } else if(!str_cmp(arg, "thirst")) {
            if((value>24)||(value<0)) {
                bug( "Mpset - Invalid thirst value: vnum %d.",
                    mob_index[ch->nr].virtual );
            }
            GET_COND(victim, 2)=MAX(MIN(value, 24), 0);
        } else if(!str_cmp(arg, "hunger")) {
            if((value>24)||(value<0)) {
                bug( "Mpset - Invalid hunger value: vnum %d.",
                    mob_index[ch->nr].virtual );
            }
            GET_COND(victim, 1)=MAX(MIN(value, 24), 0);
        }
    }

    return;
}

ACMD(do_mpheal)
{
    struct descriptor_data *d;
    char      arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;
    int heal;
    int	amount=-1, dice_type=1, bonus=0;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument=one_argument( argument, arg );

    if ((!*arg) || (*arg < '0') || (*arg > '9'))
    {
	bug( "MpHeal - bad heal argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    sscanf(arg, "%dd%d+%d", &amount, &dice_type, &bonus);
    if(amount<0) {
	bug( "MpHeal - bad heal argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    argument=one_argument(argument, arg);

    if ( !str_cmp( arg, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                victim=d->character;
                heal=dice(amount, dice_type)+bonus;
                GET_HIT(victim)=MIN(GET_MAX_HIT(victim), GET_HIT(victim)+heal);
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL )
    {
	bug( "MpHeal - Victim not in room: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    heal=dice(amount, dice_type)+bonus;

    GET_HIT(victim)=MIN(GET_MAX_HIT(victim), GET_HIT(victim)+heal);
    return;
}

ACMD(do_mpunaffect)
{
    struct descriptor_data *d;
    char      arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;
    struct affected_type *af, *af_next;
    struct spcontinuous *s;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument=one_argument(argument, arg);

    if ( !str_cmp( arg, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                victim=d->character;
                for(af=victim->affected; af; af=af_next) {
                  af_next=af->next;
                  if((af->type < PSI_START) || (af->type > PSI_END))
                    affect_remove(victim, victim->affected);
                }
                for(s=victim->char_specials.spcont; s; s=s->next) {
                  s->sptimer=0;
                }
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL )
    {
	bug( "MpUnaffect - Victim not in room: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if(IS_NPC(victim) || (GET_LEVEL(victim)<LVL_HERO) || PRF_FLAGGED(victim, PRF_AVTR)) {
        for(af=victim->affected; af; af=af_next) {
          af_next=af->next;
          if((af->type < PSI_START) || (af->type > PSI_END))
            affect_remove(victim, victim->affected);
        }
        for(s=victim->char_specials.spcont; s; s=s->next) {
          s->sptimer=0;
        }
    }

    return;
}

ACMD(do_mpcalm)
{
    struct char_data *i;
    char      arg[ MAX_INPUT_LENGTH ];

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    if(argument) {
        skip_spaces(&argument);
        if(*argument) {
            argument=one_argument(argument, arg);
            if ( ( i = get_char_room(arg, ch->in_room) ) == NULL )
            {
                bug( "MpCalm - Victim not in room: vnum %d.",
                      mob_index[ch->nr].virtual );
            }
            else {
                if(i==ch) {
                  if(FIGHTING(i))
                      stop_fighting(i);
                }
                else {
                  if(FIGHTING(i)==ch)
                      stop_fighting(i);
                }
            }
            return;
        }
    }

    for(i=world[ch->in_room].people; i; i=i->next_in_room) {
        if(FIGHTING(i)==ch)
            stop_fighting(i);
    }
    stop_fighting(ch);

    return;
}

ACMD(do_mpstun)
{
    struct descriptor_data *d;
    char      arg[ MAX_INPUT_LENGTH ];
    struct char_data *victim;
    int rounds;
    int	amount=-1, dice_type=1, bonus=0;

    if ( IS_AFFECTED(ch, AFF_CHARM))
	return;

    if(mprog_disable)
      return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument=one_argument(argument, arg);

    if ((!*arg) || (*arg < '0') || (*arg > '9'))
    {
	bug( "MpStun - bad stun argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    sscanf(arg, "%dd%d+%d", &amount, &dice_type, &bonus);
    if(amount<0) {
	bug( "MpStun - bad stun argument: vnum %d.",
		mob_index[ch->nr].virtual );
	return;
    }

    argument=one_argument(argument, arg);

    if ( !str_cmp( arg, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room == ch->in_room
            &&   ((GET_LEVEL(d->character) < LVL_HERO) || PRF_FLAGGED(d->character, PRF_AVTR)) )
	    {
                rounds=dice(amount, dice_type)+bonus;
                GET_STUN(d->character) = rounds;
                WAIT_STATE(d->character, PULSE_VIOLENCE * rounds);
	    }
	}
	return;
    }

    if ( ( victim = get_char_room(arg, ch->in_room) ) == NULL )
    {
	bug( "MpStun - Victim not in room: vnum %d.",
	    mob_index[ch->nr].virtual );
	return;
    }

    if(IS_NPC(victim) || (GET_LEVEL(victim)<LVL_HERO) || PRF_FLAGGED(victim, PRF_AVTR)) {
       rounds=dice(amount, dice_type)+bonus;
       GET_STUN(victim) = rounds;
       WAIT_STATE(victim, PULSE_VIOLENCE * rounds);
    }

    return;
}

ACMD(do_mpdrop)
{
  struct obj_data *obj;

  if(AFF_FLAGGED(ch, AFF_CHARM))
    return;

  if(mprog_disable)
    return;

  if(!IS_NPC(ch)) {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if(ch->in_room==NOWHERE)
    return;

  one_argument(argument, arg);

  if(!str_cmp(arg, "all")) {
    while((obj=ch->carrying)) {
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
    }
  }
  else {
    if((obj=get_obj_in_list(arg, ch->carrying))) {
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
    }
    else {
      bug("MpDrop - Item does not exist: vnum %d.", mob_index[ch->nr].virtual);
    }
  }
}
