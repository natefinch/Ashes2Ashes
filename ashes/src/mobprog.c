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
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
 ***************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

bool str_prefix(const char *, const char *);
char nuhl[ 1 ] = {0};
int MOBSay;

extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
extern char *command[];


/*
 * Local function prototypes
 */

char *	mprog_next_command	( char* clist );
int	mprog_seval		( char* lhs, char* opr, char* rhs );
int	mprog_veval		( int lhs, char* opr, int rhs );
int	mprog_do_ifchck		( char* ifchck, struct char_data* mob,
				       struct char_data* actor, struct obj_data* obj,
				       void* vo, struct char_data* rndm, 
				       struct char_data* rndmob );
char *	mprog_process_if	( char* ifchck, char* com_list, 
				       struct char_data* mob, struct char_data* actor,
				       struct obj_data* obj, void* vo,
				       struct char_data* rndm,
				       struct char_data* rndmob	 );
void	mprog_translate		( char ch, char* t, struct char_data* mob,
				       struct char_data* actor, struct obj_data* obj,
				       void* vo, struct char_data* rndm,
				       struct char_data* rndmob );
void	mprog_process_cmnd	( char* cmnd, struct char_data* mob, 
				       struct char_data* actor, struct obj_data* obj,
				       void* vo, struct char_data* rndm,
				       struct char_data* rndmob );
void	mprog_driver		( char* com_list, struct char_data* mob,
				       struct char_data* actor, struct obj_data* obj,
				       void* vo );
/***************************************************************************
 * Local function code and brief comments.
 */


/* Used to get sequential lines of a multi line string (separated by "\n\r")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *mprog_next_command( char *clist )
{

  char *pointer = clist;

  while ( *pointer != '\n' && *pointer != '\0' )
    pointer++;
/* Added by Jesse 2-19-97 to clean up strin handling */
  if ( *(pointer-1) == '\r' )
    *(pointer-1) = '\0';

  if ( *pointer == '\n' )
    *pointer++ = '\0';
  if ( *pointer == '\r' )
    *pointer++ = '\0';

  return ( pointer );

}

/* we need str_infix here because strstr is not case insensitive */

bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ((c0 = LOWER(astr[0])) == '\0')
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ ) {
        if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
            return FALSE;
    }

    return TRUE;
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
int mprog_seval( char *lhs, char *opr, char *rhs )
{

  if ( !str_cmp( opr, "==" ) )
    return ( !str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "!=" ) )
    return ( str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "/" ) )
    return ( !str_infix( rhs, lhs ) );
  if ( !str_cmp( opr, "!/" ) )
    return ( str_infix( rhs, lhs ) );

  bug2 ( "Improper MOBprog operator\n\r");
  return 0;

}

int mprog_veval( int lhs, char *opr, int rhs )
{

  if ( !str_cmp( opr, "==" ) )
    return ( lhs == rhs );
  if ( !str_cmp( opr, "!=" ) )
    return ( lhs != rhs );
  if ( !str_cmp( opr, ">" ) )
    return ( lhs > rhs );
  if ( !str_cmp( opr, "<" ) )
    return ( lhs < rhs );

/* this is fubared, should be as below
  if ( !str_cmp( opr, ">=" ) )
    return ( lhs <= rhs );
*/
  if ( !str_cmp( opr, "<=" ) )
    return ( lhs <= rhs );

  if ( !str_cmp( opr, ">=" ) )
    return ( lhs >= rhs );
  if ( !str_cmp( opr, "&" ) )
    return ( lhs & rhs );
  if ( !str_cmp( opr, "|" ) )
    return ( lhs | rhs );

  bug2 ( "Improper MOBprog operator\n\r");
  return 0;

}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
int mprog_do_ifchck( char *ifchck, struct char_data *mob, struct char_data *actor,
		     struct obj_data *obj, void *vo, struct char_data *rndm,
		     struct char_data *rndmob)
{

  char buf[ MAX_INPUT_LENGTH ];
  char arg[ MAX_INPUT_LENGTH ];
  char opr[ MAX_INPUT_LENGTH ];
  char val[ MAX_INPUT_LENGTH ];
  struct char_data *vict = (struct char_data *) vo;
  struct obj_data *v_obj = (struct obj_data  *) vo;
  char     *bufpt = buf;
  char     *argpt = arg;
  char     *oprpt = opr;
  char     *valpt = val;
  char     *point = ifchck;
  int       lhsvl;
  int       rhsvl;

  if ( *point == '\0' ) 
    {
      bug ( "Mob: %d null ifchck", (int)mob_index[mob->nr].virtual); 
      return -1;
    }   
  /* skip leading spaces */
  while ( *point == ' ' )
    point++;

  /* get whatever comes before the left paren.. ignore spaces */
  while ( *point != '(' ) 
    if ( *point == '\0' ) 
      {
	bug ( "Mob: %d ifchck syntax error", mob_index[mob->nr].virtual); 
	return -1;
      }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*bufpt++ = *point++; 

  *bufpt = '\0';
  point++;

  /* get whatever is in between the parens.. ignore spaces */
  while ( *point != ')' ) 
    if ( *point == '\0' ) 
      {
	bug ( "Mob: %d ifchck syntax error", mob_index[mob->nr].virtual); 
	return -1;
      }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*argpt++ = *point++; 

  *argpt = '\0';
  point++;

  /* check to see if there is an operator */
  while ( *point == ' ' )
    point++;
  if ( *point == '\0' ) 
    {
      *opr = '\0';
      *val = '\0';
    }   
  else /* there should be an operator and value, so get them */
    {
      while ( ( *point != ' ' ) && ( !isalnum( *point ) ) ) 
	if ( *point == '\0' ) 
	  {
	    bug ( "Mob: %d ifchck operator without value",
		 mob_index[mob->nr].virtual ); 
	    return -1;
	  }   
	else
	  *oprpt++ = *point++; 

      *oprpt = '\0';
 
      /* finished with operator, skip spaces and then get the value */
      while ( *point == ' ' )
	point++;
      for( ; ; )
	{
	  if ( ( *point != ' ' ) && ( *point == '\0' ) )
	    break;
	  else
	    *valpt++ = *point++; 
	}

      *valpt = '\0';
    }
  bufpt = buf;
  argpt = arg;
  oprpt = opr;
  valpt = val;

  if((*valpt) == '$') {
    mprog_translate(valpt[1], valpt, mob, actor, obj, vo, rndm, rndmob);
  }

  /* Ok... now buf contains the ifchck, arg contains the inside of the
   *  parentheses, opr contains an operator if one is present, and val
   *  has the value if an operator was present.
   *  So.. basically use if statements and run over all known ifchecks
   *  Once inside, use the argument and expand the lhs. Then if need be
   *  send the lhs,opr,rhs off to be evaluated.
   */

  if ( !str_cmp( buf, "rand" ) )
    {
      return ( number(1, 100) <= atoi(arg) );
    }

  if ( !str_cmp( buf, "exist" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': if (  mob  )
                     return 1;
                  else return 0;
	case 'n': if ( actor )
 	             return 1;
	          else return 0;
	case 't': if ( vict )
                     return 1;
	          else return 0;
	case 'r': if ( rndm )
                     return 1;
	          else return 0;
        case 'z': if (rndmob)
    	             return 1;
                  else return 0;
	default:
	  bug ( "Mob: %d bad argument to 'exist'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "ispc" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return 0;
	case 'n': if ( actor )
 	             return ( !IS_NPC( actor ) );
	          else return -1;
	case 't': if ( vict )
                     return ( !IS_NPC( vict ) );
	          else return -1;
	case 'r': if ( rndm )
                     return ( !IS_NPC( rndm ) );
	          else return -1;
        case 'z': if (rndmob)
    	             return ( !IS_NPC(rndmob) );
                  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'ispc'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isnpc" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return 1;
	case 'n': if ( actor )
	             return IS_NPC( actor );
	          else return -1;
	case 't': if ( vict )
                     return IS_NPC( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_NPC( rndm );
	          else return -1;
	case 'z': if ( rndmob )
		     return IS_NPC( rndmob );
		  else return -1;
	default:
	  bug ("Mob: %d bad argument to 'isnpc'",
               mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isgood" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_GOOD( mob );
	case 'n': if ( actor )
	             return IS_GOOD( actor );
	          else return -1;
	case 't': if ( vict )
	             return IS_GOOD( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_GOOD( rndm );
	          else return -1;
	case 'z': if (rndmob)
	             return IS_GOOD(rndmob);
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isgood'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isevil" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_EVIL( mob );
	case 'n': if ( actor )
	             return IS_EVIL( actor );
	          else return -1;
	case 't': if ( vict )
	             return IS_EVIL( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_EVIL( rndm );
	          else return -1;
	case 'z': if (rndmob)
	             return IS_EVIL(rndmob);
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isevil'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isfight" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
/* 2-19-97 Added parenth around return because it looks wrong without - Jesse */
	case 'i': return(( FIGHTING(mob) ) ? 1 : 0);
	case 'n': if ( actor )
	             return(( FIGHTING(actor) ) ? 1 : 0);
	          else return -1;
	case 't': if ( vict )
	             return(( FIGHTING(vict) ) ? 1 : 0);
	          else return -1;
	case 'r': if ( rndm )
	             return(( FIGHTING(rndm) ) ? 1 : 0);
	          else return -1;
	case 'z': if (rndmob)
	 	     return(( FIGHTING(rndmob) ) ? 1 : 0);
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isfight'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isimmort" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( IS_NPC(mob) ? 0 : (GET_LEVEL( mob ) >= LVL_HERO) );
	case 'n': if ( actor )
	             return ( IS_NPC(actor) ? 0 : (GET_LEVEL( actor ) >= LVL_HERO) );
  	          else return -1;
	case 't': if ( vict )
	             return ( IS_NPC(vict) ? 0 : (GET_LEVEL( vict ) >= LVL_HERO) );
                  else return -1;
	case 'r': if ( rndm )
	             return ( IS_NPC(rndm) ? 0 : (GET_LEVEL( rndm ) >= LVL_HERO) );
                  else return -1;
        case 'z': if ( rndmob )
	             return ( IS_NPC(rndmob) ? 0 : (GET_LEVEL( rndmob ) >= LVL_HERO) );
                  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isimmort'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }


   if ( !str_cmp( buf, "cansee" ) ) {
      switch (arg[1]) {
       case 'i': return 1;
       case 'n': if (actor && mob)
	   return (CAN_SEE(mob, actor));
	 else return -1;
       case 't': if (vict && mob)
	   return (CAN_SEE(mob, vict));
	 else return -1;
       case 'r' : if (rndm && mob)
	   return (CAN_SEE(mob, rndm));
	 else return -1;
       case 'z' : if (rndm && mob)
	   return (CAN_SEE(mob, rndmob));
         else return -1;	
       default:
	 bug ("Mob: %d bad argument to 'cansee'",
	      mob_index[mob->nr].virtual);
	 return -1;
      }
   }
      
   if ( !str_cmp( buf, "ischarmed" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_AFFECTED( mob, AFF_CHARM );
	case 'n': if ( actor )
	             return IS_AFFECTED( actor, AFF_CHARM );
	          else return -1;
	case 't': if ( vict )
	             return IS_AFFECTED( vict, AFF_CHARM );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_AFFECTED( rndm, AFF_CHARM );
	          else return -1;
	case 'z': if (rndmob)
		     return IS_AFFECTED( rndmob, AFF_CHARM );
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'ischarmed'",
	       mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isfollow" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( mob->master != NULL
			  && mob->master->in_room == mob->in_room );
	case 'n': if ( actor )
	             return ( actor->master != NULL
			     && actor->master->in_room == actor->in_room );
	          else return -1;
	case 't': if ( vict )
	             return ( vict->master != NULL
			     && vict->master->in_room == vict->in_room );
	          else return -1;
	case 'r': if ( rndm )
	             return ( rndm->master != NULL
			     && rndm->master->in_room == rndm->in_room );
	          else return -1;
	case 'z': if (rndmob)
		     return ( rndmob->master != NULL
			     && rndmob->master->in_room == rndmob->in_room);
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isfollow'", 
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "isaffected" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
/* Jesse 2-19-97 - changed arg to val in atoi */
	case 'i': return ( AFF_FLAGS(mob) & atoll( val ) );
	case 'n': if ( actor )
	             return ( AFF_FLAGS(actor) & atoll( val ) );
	          else return -1;
	case 't': if ( vict )
	             return ( AFF_FLAGS(vict) & atoll( val ) );
	          else return -1;
	case 'r': if ( rndm )
	             return ( AFF_FLAGS(rndm) & atoll( val ) );
	          else return -1;
	case 'z': if (rndmob)
		     return ( AFF_FLAGS(rndmob) & atoll( val ) );
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'isaffected'",
	       mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "hitprcnt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = (int)((100L*mob->points.hit) / mob->points.max_hit);
	          rhsvl = atoi( val );
         	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = (int)((100L*actor->points.hit) / actor->points.max_hit);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = (int)((100L*vict->points.hit) / vict->points.max_hit);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = (int)((100L*rndm->points.hit) / rndm->points.max_hit);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'z': if (rndmob)
		  {
	            lhsvl = (int)((100L*rndmob->points.hit) / rndmob->points.max_hit);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'hitprcnt'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "inroom" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->in_room;
	          rhsvl = atoi(val);
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->in_room;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->in_room;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->in_room;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'z': if (rndmob)
		  {
		    lhsvl = rndmob->in_room;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
	          }
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'inroom'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "sex" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->player.sex;
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->player.sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->player.sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->player.sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'z': if ( rndmob )
	          {
		    lhsvl = rndmob->player.sex;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'sex'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "position" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GET_POS(mob);
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = GET_POS(actor);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = GET_POS(vict);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = GET_POS(rndm);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'z': if (rndmob)
	          {
	            lhsvl = GET_POS(rndmob);
		    rhsvl = atoi( val );
	            return mprog_veval( lhsvl, opr, rhsvl );
		  }
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'position'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "level" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GET_LEVEL( mob );
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = GET_LEVEL( actor );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = GET_LEVEL( vict );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = GET_LEVEL( rndm );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'z': if (rndmob)
		  {
	            lhsvl = GET_LEVEL( rndmob );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'level'",
                mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "class" ) )
    {
      rhsvl = atoi( val );
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': if(GET_CLASS_BITVECTOR(mob) & (1 << rhsvl))
                    return 1;
                  else
                    return 0;
                  break;
	case 'n': if ( actor )
	          {
                    if(GET_CLASS_BITVECTOR(actor) & (1 << rhsvl))
                      return 1;
                    else
                      return 0;
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
                    if(GET_CLASS_BITVECTOR(vict) & (1 << rhsvl))
                      return 1;
                    else
                      return 0;
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
                    if(GET_CLASS_BITVECTOR(rndm) & (1 << rhsvl))
                      return 1;
                    else
                      return 0;
		  }
	          else
		    return -1;
	case 'z': if ( rndmob )
	          {
                    if(GET_CLASS_BITVECTOR(rndmob) & (1 << rhsvl))
                      return 1;
                    else
                      return 0;
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'class'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "goldamt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->points.gold;
                  rhsvl = atoi( val );
                  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->points.gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->points.gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->points.gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'z': if (rndmob)
		  {
		    lhsvl = rndmob->points.gold;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
		  else return -1;
	default:
	  bug ( "Mob: %d bad argument to 'goldamt'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objtype" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->obj_flags.type_flag;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	         else
		   return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->obj_flags.type_flag;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objtype'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval0" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->obj_flags.value[0];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->obj_flags.value[0];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval0'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval1" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->obj_flags.value[1];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->obj_flags.value[1];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval1'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval2" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->obj_flags.value[2];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->obj_flags.value[2];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval2'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objval3" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->obj_flags.value[3];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj ) 
	          {
		    lhsvl = v_obj->obj_flags.value[3];
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'objval3'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "number" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
/* should check vnum, this is screwy
	case 'i': lhsvl = mob->points.gold;
*/
	case 'i': if IS_NPC( mob )
		  {
                    lhsvl = mob_index[mob->nr].virtual;
	            rhsvl = atoi( val );
	            return mprog_veval( lhsvl, opr, rhsvl );
                  }
                  return 0;
	case 'n': if ( actor )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = mob_index[actor->nr].virtual;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
                    return 0;
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    if IS_NPC( vict )
		    {
		      lhsvl = mob_index[vict->nr].virtual;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
                    return 0;
		  }
                  else
		    return -1;
	case 'r': if ( rndm )
	          {
		    if IS_NPC( rndm )
		    {
		      lhsvl = mob_index[rndm->nr].virtual;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
                    return 0;
		  }
	         else return -1;
	case 'z': if (rndmob)
    		  {
                    if IS_NPC( rndmob )
		    {
		      lhsvl = mob_index[rndmob->nr].virtual;
		      rhsvl = atoi( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
                    return 0;
                  }
	          else return -1;
	case 'o': if ( obj )
	          {
		    lhsvl = obj_index[obj->item_number].virtual;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = obj_index[v_obj->item_number].virtual;
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'number'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  if ( !str_cmp( buf, "name" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return mprog_seval( mob->player.name, opr, val );
	case 'n': if ( actor )
	            return mprog_seval( actor->player.name, opr, val );
	          else
		    return -1;
	case 't': if ( vict )
	            return mprog_seval( vict->player.name, opr, val );
	          else
		    return -1;
	case 'r': if ( rndm )
	            return mprog_seval( rndm->player.name, opr, val );
	          else
		    return -1;
	case 'z': if (rndmob)
		    return mprog_seval( rndmob->player.name, opr, val );
	          else
	            return -1;
	case 'o': if ( obj )
	            return mprog_seval( obj->name, opr, val );
	          else
		    return -1;
	case 'p': if ( v_obj )
	            return mprog_seval( v_obj->name, opr, val );
	          else
		    return -1;
	default:
	  bug ( "Mob: %d bad argument to 'name'", mob_index[mob->nr].virtual ); 
	  return -1;
	}
    }

  /* Ok... all the ifchcks are done, so if we didnt find ours then something
   * odd happened.  So report the bug and abort the MOBprogram (return error)
   */
  bug ( "Mob: %d unknown ifchck", mob_index[mob->nr].virtual ); 
  return -1;

}
/* Quite a long and arduous function, this guy handles the control
 * flow part of MOBprograms.  Basicially once the driver sees an
 * 'if' attention shifts to here.  While many syntax errors are
 * caught, some will still get through due to the handling of break
 * and errors in the same fashion.  The desire to break out of the
 * recursion without catastrophe in the event of a mis-parse was
 * believed to be high. Thus, if an error is found, it is bugged and
 * the parser acts as though a break were issued and just bails out
 * at that point. I havent tested all the possibilites, so I'm speaking
 * in theory, but it is 'guaranteed' to work on syntactically correct
 * MOBprograms, so if the mud crashes here, check the mob carefully!
 */
char *mprog_process_if( char *ifchck, char *com_list, struct char_data *mob,
		       struct char_data *actor, struct obj_data *obj, void *vo,
		       struct char_data *rndm, struct char_data *rndmob )
{

 char buf[ MAX_INPUT_LENGTH ];
 char *morebuf = '\0';
 char    *cmnd = '\0';
 int loopdone = FALSE;
 int     flag = FALSE;
 int  legal;

 *nuhl = '\0';

 /* check for trueness of the ifcheck */
 if ( ( legal = mprog_do_ifchck( ifchck, mob, actor, obj, vo, rndm, rndmob ) ) )
   if ( legal != -1 )
     flag = TRUE;
   else
     return nuhl;

 while( loopdone == FALSE ) /*scan over any existing or statements */
 {
     cmnd     = com_list;
     com_list = mprog_next_command( com_list );
     while ( *cmnd == ' ' )
       cmnd++;
     if ( *cmnd == '\0' )
     {
	 bug ( "Mob: %d no commands after IF/OR", mob_index[mob->nr].virtual ); 
	 return nuhl;
     }
     morebuf = one_argument( cmnd, buf );
     if ( !str_cmp( buf, "or" ) )
     {
	 if ( ( legal = mprog_do_ifchck( morebuf,mob,actor,obj,vo,rndm,rndmob ) ) )
	   if ( legal != -1 )
	     flag = TRUE;
	   else
	     return nuhl;
     }
     else
       loopdone = TRUE;
 }
 
 if ( flag )
   for ( ; ; ) /*ifcheck was true, do commands but ignore else to endif*/ 
   {
       if ( !str_cmp( buf, "if" ) )
       { 
	   com_list = 
mprog_process_if(morebuf,com_list,mob,actor,obj,vo,rndm,rndmob);
	   while ( *cmnd==' ' )
	     cmnd++;
	   if ( *com_list == '\0' )
	     return nuhl;
	   cmnd     = com_list;
	   com_list = mprog_next_command( com_list );
	   morebuf  = one_argument( cmnd,buf );
	   continue;
       }
       if ( !str_cmp( buf, "break" ) )
	 return nuhl;
       if ( !str_cmp( buf, "endif" ) )
	 return com_list; 
       if ( !str_cmp( buf, "else" ) ) 
       {
	   while ( str_cmp( buf, "endif" ) ) 
	   {
	       cmnd     = com_list;
	       com_list = mprog_next_command( com_list );
	       while ( *cmnd == ' ' )
		 cmnd++;
	       if ( *cmnd == '\0' )
	       {
		   bug ( "Mob: %d missing endif after else",
			mob_index[mob->nr].virtual );
		   return nuhl;
	       }
	       morebuf = one_argument( cmnd,buf );
	   }
	   return com_list; 
       }
       mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm,rndmob );
       cmnd     = com_list;
       com_list = mprog_next_command( com_list );
       while ( *cmnd == ' ' )
	 cmnd++;
       if ( *cmnd == '\0' )
       {
           bug ( "Mob: %d missing else or endif", mob_index[mob->nr].virtual ); 
           return nuhl;
       }
       morebuf = one_argument( cmnd, buf );
   }
 else /*false ifcheck, find else and do existing commands or quit at endif*/
   {
     while ( ( str_cmp( buf, "else" ) ) && ( str_cmp( buf, "endif" ) ) )
       {
	 cmnd     = com_list;
	 com_list = mprog_next_command( com_list );
	 while ( *cmnd == ' ' )
	   cmnd++;
	 if ( *cmnd == '\0' )
	   {
	     bug ( "Mob: %d missing an else or endif",
		  mob_index[mob->nr].virtual ); 
	     return nuhl;
	   }
	 morebuf = one_argument( cmnd, buf );
       }

     /* found either an else or an endif.. act accordingly */
     if ( !str_cmp( buf, "endif" ) )
       return com_list;
     cmnd     = com_list;
     com_list = mprog_next_command( com_list );
     while ( *cmnd == ' ' )
       cmnd++;
     if ( *cmnd == '\0' )
       { 
	 bug ( "Mob: %d missing endif", mob_index[mob->nr].virtual ); 
	 return nuhl;
       }
     morebuf = one_argument( cmnd, buf );
     
     for ( ; ; ) /*process the post-else commands until an endif is found.*/
       {
	 if ( !str_cmp( buf, "if" ) )
	   { 
	     com_list = mprog_process_if( morebuf, com_list, mob, actor,
					 obj, vo, rndm, rndmob );
	     while ( *cmnd == ' ' )
	       cmnd++;
	     if ( *com_list == '\0' )
	       return nuhl;
	     cmnd     = com_list;
	     com_list = mprog_next_command( com_list );
	     morebuf  = one_argument( cmnd,buf );
	     continue;
	   }
	 if ( !str_cmp( buf, "else" ) ) 
	   {
	     bug ( "Mob: %d found else in an else section",
		  mob_index[mob->nr].virtual ); 
	     return nuhl;
	   }
	 if ( !str_cmp( buf, "break" ) )
	   return nuhl;
	 if ( !str_cmp( buf, "endif" ) )
	   return com_list; 
	 mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm, rndmob );
	 cmnd     = com_list;
	 com_list = mprog_next_command( com_list );
	 while ( *cmnd == ' ' )
	   cmnd++;
	 if ( *cmnd == '\0' )
	   {
	     bug ( "Mob:%d missing endif in else section",
		  mob_index[mob->nr].virtual ); 
	     return nuhl;
	   }
	 morebuf = one_argument( cmnd, buf );
       }
   }
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
void mprog_translate( char ch, char *t, struct char_data *mob, struct char_data *actor,
                    struct obj_data *obj, void *vo, struct char_data *rndm,
		    struct char_data *rndmob )
{
 static char *he_she        [] = { "it",  "he",  "she" };
 static char *him_her       [] = { "it",  "him", "her" };
 static char *his_her       [] = { "its", "his", "her" };
 struct char_data   *vict             = (struct char_data *) vo;
 struct obj_data    *v_obj            = (struct obj_data  *) vo;

/*
 *t = '\0';
*/
 strcpy(t, "<NULL>");
 switch ( ch ) {
     case 'i':
         one_argument( mob->player.name, t );
      break;

     case 'I':
         strcpy( t, mob->player.short_descr );
      break;

     case 'n':
         if ( actor ) {
           if ( !IS_NPC( actor ) ) {
             strcpy(t, actor->player.name);
           } else
             one_argument( actor->player.name, t );
         }
      break;

     case 'N':
         if ( actor ) 
           if ( IS_NPC( actor ) )
             strcpy( t, actor->player.short_descr );
           else
           {
             strcpy( t, actor->player.name );
             strcat( t, " " );
             strcat( t, actor->player.title );
           }
	 break;

     case 't':
         if ( vict ) {
           if ( !IS_NPC( vict ) )
             strcpy(t, vict->player.name);
           else
             one_argument( vict->player.name, t );
         }
	 break;

     case 'T':
         if ( vict ) 
           if ( IS_NPC( vict ) )
             strcpy( t, vict->player.short_descr );
           else
           {
             strcpy( t, vict->player.name );
             strcat( t, " " );
             strcat( t, vict->player.title );
           }
	 break;
     
     case 'r':
         if ( rndm ) {
           if ( !IS_NPC( rndm ) )
             strcpy(t, rndm->player.name);
           else
             one_argument( rndm->player.name, t );
         }
      break;
     case 'z':
         if (rndmob) {
           one_argument( rndmob->player.name, t );
         }
      break;
     case 'R':
         if ( rndm ) 
           if ( IS_NPC( rndm ) )
             strcpy(t,rndm->player.short_descr);
           else
           {
             strcpy( t, rndm->player.name );
             strcat( t, " " );
             strcat( t, rndm->player.title );
           }
	 break;

     case 'Z':
         if ( rndmob ) 
           if ( IS_NPC( rndmob ) )
             strcpy(t,rndmob->player.short_descr);
           else
           {
             strcpy( t, rndmob->player.name );
             strcat( t, " " );
             strcat( t, rndmob->player.title );
           }
	 break;

     case 'e':
         if ( actor )
           strcpy( t, he_she[ actor->player.sex + 0] );
	 break;
  
     case 'm':
         if ( actor )
           strcpy( t, him_her[ actor->player.sex + 0] );
	 break;
  
     case 's':
         if ( actor )
           strcpy( t, his_her[ actor->player.sex + 0] );
	 break;
     
     case 'E':
         if ( vict )
           strcpy( t, he_she[ vict->player.sex + 0] );
	 break;
  
     case 'M':
         if ( vict )
           strcpy( t, him_her[ vict->player.sex + 0] );
	 break;
  
     case 'S':
         if ( vict )
           strcpy( t, his_her[ vict->player.sex + 0] ); 
	 break;

     case 'j':
	 strcpy( t, he_she[ mob->player.sex + 0] );
	 break;
  
     case 'k':
	 strcpy( t, him_her[ mob->player.sex + 0] );
	 break;
  
     case 'l':
	 strcpy( t, his_her[ mob->player.sex + 0] );
	 break;

     case 'J':
         if ( rndm )
           strcpy( t, he_she[ rndm->player.sex + 0] );
	 break;
  
     case 'K':
         if ( rndm )
           strcpy( t, him_her[ rndm->player.sex + 0] );
	 break;
  
     case 'L':
         if ( rndm )
           strcpy( t, his_her[ rndm->player.sex + 0] );
	 break;

     case 'u':
         if ( rndmob )
           strcpy( t, he_she[ rndmob->player.sex + 0] );
	 break;
  
     case 'v':
         if ( rndmob )
           strcpy( t, him_her[ rndmob->player.sex + 0] );
	 break;
  
     case 'w':
         if ( rndmob )
           strcpy( t, his_her[ rndmob->player.sex + 0] );
	 break;

     case 'o':
         if ( obj )
           one_argument( obj->name, t );
	 break;

     case 'O':
         if ( obj )
           strcpy( t, obj->short_description );
	 break;

     case 'p':
         if ( v_obj )
           one_argument( v_obj->name, t );
	 break;

     case 'P':
         if ( v_obj )
           strcpy( t, v_obj->short_description );
      break;

     case 'a':
         if ( obj ) 
          switch ( *( obj->name ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case 'A':
         if ( v_obj ) 
          switch ( *( v_obj->name ) )
	  {
            case 'a': case 'e': case 'i':
	    case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case '$':
         strcpy( t, "$" );
	 break;

     default:
         bug( "Mob: %d bad $var", mob_index[mob->nr].virtual );
	 break;
       }

 return;

}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void mprog_process_cmnd( char *cmnd, struct char_data *mob, struct char_data *actor,
			struct obj_data *obj, void *vo, struct char_data *rndm,
			struct char_data *rndmob )
{
  char buf[ MAX_INPUT_LENGTH ];
  char tmp[ MAX_INPUT_LENGTH ];
  char *str;
  char *i;
  char *point;

  point   = buf;
  str     = cmnd;

  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    str++;
    mprog_translate( *str, tmp, mob, actor, obj, vo, rndm, rndmob );
    i = tmp;
    ++str;
    while ( ( *point = *i ) != '\0' ) {
      ++point;
      ++i;
    }
  }
  *point = '\0';
  command_interpreter( mob, buf );

  return;

}

/* The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver ( char *com_list, struct char_data *mob, struct char_data *actor,
		   struct obj_data *obj, void *vo)
{

 char tmpcmndlst[ MAX_STRING_LENGTH ];
 char buf       [ MAX_INPUT_LENGTH ];
 char *morebuf;
 char *command_list;
 char *cmnd;
 struct char_data *rndm  = NULL;
 struct char_data *rndmob = NULL;   /* Maxx - for $m - random mob in room name */
 struct char_data *vch   = NULL;
 int        count = 0;
 int        count1 = 0;

 if IS_AFFECTED( mob, AFF_CHARM )
   return;

 /* get a random visable mortal player who is in the room with the mob */
 /* get a random visible mob who is in the room with the mob */
 for ( vch = world[mob->in_room].people; vch; vch = vch->next_in_room )
   if ( !IS_NPC( vch )
       &&  vch->player.level < LVL_HERO)
     {
       if ( number( 0, count ) == 0 )
	 rndm = vch;
       count++;
     } else {
       if(IS_NPC(vch)&&(mob!=vch)) {
         if ( number( 0, count1 ) == 0 )
           rndmob = vch;
         count1++;
       }
     }
  
 strcpy( tmpcmndlst, com_list );
 command_list = tmpcmndlst;
 cmnd         = command_list;
 command_list = mprog_next_command( command_list );
 while ( *cmnd == ' ' )
   cmnd++;
 while ( *cmnd != '\0' )
   {
     morebuf = one_argument( cmnd, buf );
     if ( !str_cmp( buf, "if" ) )
       command_list = mprog_process_if( morebuf, command_list, mob,
				       actor, obj, vo, rndm, rndmob );
     else
       mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm, rndmob );
     cmnd         = command_list;
     command_list = mprog_next_command( command_list );
     while ( *cmnd == ' ' )
       cmnd++;
   }

 return;

}

/***************************************************************************
 * Global function code and brief comments.
 */

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, struct char_data *mob, struct char_data *actor,
			  struct obj_data *obj, void *vo, int type )
{

  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *tdupl;
  char       *end;
  int         i, bk, go;

  for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
    if ( mprg->type & type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
        while(isspace(*list)) list++;
        i=strlen(list)-1;
        while(i&&isspace(list[i])) list[i--]=0;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
        while(isspace(*dupl)) dupl++;
        i=strlen(dupl)-1;
        while(i&&isspace(dupl[i])) dupl[i--]=0;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == 0 ) )
		{
                  if(type&SPEECH_PROG)
                    MOBSay=0;
		  mprog_driver( mprg->comlist, mob, actor, obj, vo );
                  MOBSay=1;
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
            tdupl=dupl;
            bk=0;
            go=1;
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) ) {
              dupl=tdupl;
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == 0 ) )
		  {
/*
		    mprog_driver( mprg->comlist, mob, actor, obj, vo );
*/
                    bk=1;
		    break;
		  }
		else
		  dupl = start+1;
              if(bk) {
                bk=0;
              }
              else {
                go=0;
                break;
              }
            }
            if(go) {
              if(type&SPEECH_PROG)
                MOBSay=0;
              mprog_driver( mprg->comlist, mob, actor, obj, vo );
              MOBSay=1;
            }
	  }
      }

  return;

}

void mprog_percent_check( struct char_data *mob, struct char_data *actor, struct obj_data *obj,
			 void *vo, int type)
{
 byte old_position;
 MPROG_DATA * mprg;

 for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
   if ( ( mprg->type & type )
       && ( number(1, 100) <= atoi( mprg->arglist ) ) )
     {
       if ( type == DEATH_PROG)
       {
          old_position = GET_POS(mob);
          GET_POS(mob) = POS_STANDING;
          mprog_driver( mprg->comlist, mob, actor, obj, vo );
          GET_POS(mob) = old_position;
       }
       else
          mprog_driver( mprg->comlist, mob, actor, obj, vo );

       if (( type != GREET_PROG) && (type != ALL_GREET_PROG ))
	 break;
     }

 return;

}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *buf, struct char_data *mob, struct char_data *ch,
		       struct obj_data *obj, void *vo)
{

  MPROG_ACT_LIST * tmp_act;

  if ( IS_NPC( mob )
      && ( mob_index[mob->nr].progtypes & ACT_PROG ) )
    {
      tmp_act = malloc( sizeof( MPROG_ACT_LIST ) );
      if ( mob->mob_specials.mpactnum > 0 )
	tmp_act->next = mob->mob_specials.mpact->next;
      else
	tmp_act->next = NULL;

      mob->mob_specials.mpact      = tmp_act;
      mob->mob_specials.mpact->buf = str_dup( buf );
      mob->mob_specials.mpact->ch  = ch; 
      mob->mob_specials.mpact->obj = obj; 
      mob->mob_specials.mpact->vo  = vo; 
      mob->mob_specials.mpactnum++;

    }
  return;

}

void mprog_bribe_trigger( struct char_data *mob, struct char_data *ch, int amount )
{

/*  char        buf[ MAX_STRING_LENGTH ]; */
  MPROG_DATA *mprg;
  struct obj_data   *obj;

  if((!mob) || (!ch))
    return;

  if ( IS_NPC( mob )
      && ( mob_index[mob->nr].progtypes & BRIBE_PROG ) )
    {

      obj = create_money(amount);
      obj_to_char( obj, mob );
      mob->points.gold -= amount;

      for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next ) {

	if ( ( mprg->type & BRIBE_PROG )
	    && ( amount >= atoi( mprg->arglist ) ) )
	  {
	    mprog_driver( mprg->comlist, mob, ch, obj, NULL );
	    break;
	  }
      }
    }
  
  return;

}

void mprog_death_trigger( struct char_data *mob, struct char_data *killer )
{

  if((!mob) || (!killer))
    return;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & DEATH_PROG ) )
   {
     mprog_percent_check( mob, killer, NULL, NULL, DEATH_PROG );
   }

 return;

}

void mprog_entry_trigger( struct char_data *mob )
{

  if((!mob))
    return;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & ENTRY_PROG ) )
   mprog_percent_check( mob, NULL, NULL, NULL, ENTRY_PROG );

 return;

}

void mprog_fight_trigger( struct char_data *mob, struct char_data *ch )
{

  if((!mob) || (!ch))
    return;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & FIGHT_PROG ) ) {
   mprog_percent_check( mob, ch, NULL, NULL, FIGHT_PROG );
 }

 return;

}

void mprog_give_trigger( struct char_data *mob, struct char_data *ch, struct obj_data *obj )
{

 char        buf[MAX_INPUT_LENGTH];
 MPROG_DATA *mprg;

  if((!mob) || (!ch) || (!obj))
    return;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & GIVE_PROG ) )
   for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
     {
       one_argument( mprg->arglist, buf );
       if ( ( mprg->type & GIVE_PROG )
	   && ( ( !str_infix( obj->name, mprg->arglist ) )
	       || ( !str_cmp( "all", buf ) ) ) )
	 {
	   mprog_driver( mprg->comlist, mob, ch, obj, NULL );
	   break;
	 }
     }

 return;

}

void mprog_greet_trigger( struct char_data *ch )
{

 struct char_data *vmob;

  if((!ch))
    return;

 for ( vmob = world[ch->in_room].people; vmob != NULL; vmob = vmob->next_in_room ) {
   if ( IS_NPC( vmob )
       && ch != vmob
       && CAN_SEE( vmob, ch )
       && ( FIGHTING(vmob) == NULL )
       && AWAKE( vmob )
       && ( mob_index[vmob->nr].progtypes & GREET_PROG) )
     mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
/* Jesse 2-22-97 - changed so that all greet progs go even if greet progs go */
   if ( IS_NPC( vmob )
       && ( FIGHTING(vmob) == NULL )
       && AWAKE( vmob )
       && ( mob_index[vmob->nr].progtypes & ALL_GREET_PROG ) )
     mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
  }

 return;

}

void mprog_hitprcnt_trigger( struct char_data *mob, struct char_data *ch)
{

 MPROG_DATA *mprg;

  if((!mob) || (!ch))
    return;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & HITPRCNT_PROG ) )
   for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
     if ( ( mprg->type & HITPRCNT_PROG )
	 && ( ( (100L*mob->points.hit) / mob->points.max_hit ) < atoi( mprg->arglist ) ) )
       {
	 mprog_driver( mprg->comlist, mob, ch, NULL, NULL );
	 break;
       }
 
 return;

}

void mprog_random_trigger( struct char_data *mob )
{

  if((!mob))
    return;

  if ( mob_index[mob->nr].progtypes & RAND_PROG)
    mprog_percent_check(mob,NULL,NULL,NULL,RAND_PROG);

  return;

}

void mprog_speech_trigger( char *txt, struct char_data *mob )
{

  struct char_data *vmob;

  if((!mob))
    return;

  for ( vmob = world[mob->in_room].people; vmob != NULL; vmob = vmob->next_in_room )
    if ( IS_NPC( vmob ) && ( mob_index[vmob->nr].progtypes & SPEECH_PROG ) )
      mprog_wordlist_check( txt, vmob, mob, NULL, NULL, SPEECH_PROG );
  
  return;

}

int mprog_commandtrap_trigger(int cmdnum, struct char_data* mob, struct char_data* ch)
{
   int cmd=cmdnum;
   char *ptr;
   MPROG_DATA *mprog;

  if((!mob) || (!ch))
    return 0;

   if (IS_NPC(mob) && mob_index[mob->nr].progtypes & COMMANDTRAP_PROG)
   for ( mprog = mob_index[mob->nr].mobprogs; mprog != NULL; mprog = mprog->next )
     if ( mprog->type & COMMANDTRAP_PROG ) {
       ptr=mprog->arglist;
       skip_spaces(&ptr);
       if (CMD_IS(ptr))
       {
	 mprog_driver( mprog->comlist, mob, ch, NULL, NULL );
	 return 1;
       }
     }

     return 0;
}

int mprog_keyword_trigger(char *cmdline, struct char_data* mob, struct char_data* ch)
{
   MPROG_DATA *mprog;
   char *buf1=buf;

  if((!mob) || (!ch))
    return 0;
   
   if (IS_NPC(mob) && mob_index[mob->nr].progtypes & KEYWORD_PROG)
   for (mprog = mob_index[mob->nr].mobprogs; mprog != NULL; mprog = mprog->next) {
      strcpy(buf, mprog->arglist);
      skip_spaces(&buf1);
        if ((mprog->type & KEYWORD_PROG) && !str_cmp(cmdline, buf1)) {
	   mprog_driver(mprog->comlist, mob, ch, NULL, NULL);
	   return 1;
	}
   }
   return 0;
}
      
	
