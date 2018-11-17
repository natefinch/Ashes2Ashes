/* ************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "mail.h"
#include "boards.h"

void show_string(struct descriptor_data *d, char *input);

char *string_fields[] =
{
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};


/* maximum length for text field x+1 */
int length[] =
{
  15,
  60,
  256,
  240,
  60
};


/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Adds removes all new lines from a string and inserts cr/lf after
   words to make lines as close to but not over 80 chars as possible */
void format_string(char **str)
{
  int i, j, need_space=0, spacing=0, length=0, prev=0, last_word=-1;
  char *temp_str, *formatted, *ptr;

  CREATE(temp_str, char, strlen(*str)+1);
  for(ptr=*str, i=0; *ptr; ptr++) {
    if((*ptr == '\r') || (*ptr == '\n')) {
      need_space=1;
    }
    else {
      if(need_space) {
        if(((i > 0) && (!isspace(temp_str[i-1]))) && (!isspace(*ptr)))
          temp_str[i++]=' ';
        need_space=0;
      }
      temp_str[i++]=*ptr;
    }
  }
  temp_str[i]=0;

  CREATE(formatted, char, 1.05*strlen(temp_str)+3);
  for(i=0, j=0; temp_str[i]; i++) {
    if(isspace(temp_str[i])) {
      if(!spacing) {
        if(last_word < 0)
          last_word=prev;
        for(; last_word < i; last_word++)
          formatted[j++]=temp_str[last_word];
      }
      spacing=1;
    }
    else {
      spacing=0;
    }
    length++;
    if(length >= 78) {
      if(last_word >= 0) {
        formatted[j++]='\r';
        formatted[j++]='\n';
        i=last_word;
        while(temp_str[i] && isspace(temp_str[i]))
          i++;
        i--;
      }
      else {
        for(last_word=prev; last_word <=i; last_word++)
          formatted[j++]=temp_str[last_word];
      }
      prev=i+1;
      length=0;
      last_word=-1;
    }
  }
  if(!spacing) {
    if(last_word < 0)
      last_word=prev;
    for(; last_word < i; last_word++)
      formatted[j++]=temp_str[last_word];
  }
  if(formatted[j-1] != '\n') {
    formatted[j++]='\r';
    formatted[j++]='\n';
  }
  formatted[j]=0;

  free(temp_str);
  free(*str);
  *str=str_dup(formatted);
  free(formatted);
}

void delete_tilda(char **str)
{
  char *temp_str=*str;
  int i, j;

  for(i=0, j=0; temp_str[i]; i++) {
    if(temp_str[i]!='~')
      temp_str[j++]=temp_str[i];
  }
  temp_str[j]=0;
  *str=str_dup(temp_str);
  free(temp_str);
}

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
  int terminator = 0, olc=1, cancel=0;
  struct mail_list *tmpmail;
  extern char *MENU;

  ACMD(do_olcmenu);

  /* determine if this is the terminal string, and truncate if so */
  /* changed to only accept '@' at the beginning of line - J. Elson 1/17/94 */

  delete_doubledollar(str);

  if((terminator = (*str == '@')))
    *str = '\0';
  else if((*str == '%')&&(!d->connected) && (PLR_FLAGGED(d->character, PLR_MAILING))) {
    terminator=1;
    cancel=1;
    *str = '\0';
  }

  str=fix_string(str);
  if (!(*d->str)) {
    if (strlen(str) > d->max_str) {
      send_to_char("String too long - Truncated.\r\n",
		   d->character);
      *(str + d->max_str) = '\0';
      terminator = 1;
    }
    CREATE(*d->str, char, strlen(str) + 3);
    strcpy(*d->str, str);
  } else {
    if (strlen(str) + strlen(*d->str) > d->max_str) {
      send_to_char("String too long.  Last line skipped.\r\n", d->character);
      terminator = 1;
    } else {
      if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) +
				       strlen(str) + 3))) {
	perror("string_add");
	exit(1);
      }
      strcat(*d->str, str);
    }
  }

  if (terminator) {
    if (!d->connected && (PLR_FLAGGED(d->character, PLR_MAILING))) {
      while(d->mail_to) {
        if(!cancel)
          store_mail(d->mail_to->to_who, GET_IDNUM(d->character), *d->str);
        tmpmail=d->mail_to;
        d->mail_to=tmpmail->next;
        free(tmpmail);
      }
      free(*d->str);
      free(d->str);
      if(cancel)
        SEND_TO_Q("Message canceled.\r\n", d);
      else
        SEND_TO_Q("Message sent!\r\n", d);
      if (!IS_NPC(d->character))
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
      olc=0;
    }

    if (d->mail_to && (d->mail_to->to_who >= BOARD_MAGIC)) {
      Board_save_board(d->mail_to->to_who - BOARD_MAGIC);
      while(d->mail_to) {
        tmpmail=d->mail_to;
        d->mail_to=tmpmail->next;
        free(tmpmail);
      }
      olc=0;
    }
    if (d->connected == CON_EXDESC) {
      SEND_TO_Q(MENU, d);
      d->connected = CON_MENU;
      olc=0;
    }
    if (d->connected == CON_DELMSG) {
      SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
                "ARE YOU ABSOLUTELY SURE?\r\n\r\n"
                "Please type \"yes\" to confirm: ", d);
      STATE(d) = CON_DELCNF2;
      olc=0;
    }
    if (!d->connected && d->character && !IS_NPC(d->character))
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
    if(olc && GET_OLC_MODE(d->character)) {
      delete_tilda(d->str);
      if(PRF_FLAGGED(d->character, PRF_FORMAT))
        format_string(d->str);
      d->str = NULL;
      do_olcmenu(d->character, "0", 0, 0);
    }
    else
      d->str = NULL;
  } else
    strcat(*d->str, "\r\n");
}



/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

ACMD(do_skillset)
{
  extern char *spells[];
  struct char_data *vict;
  char name[100], buf2[100], buf[100], help[MAX_STRING_LENGTH];
  int skill, value, i, qend;

  argument = one_argument(argument, name);

  if (!*name) {			/* no arguments. print an informative text */
    send_to_char("Syntax: skillset <name> '<skill>' <value>\r\n", ch);
    strcpy(help, "Skill being one of the following:\n\r");
    for (i = 0; *spells[i] != '\n'; i++) {
      if (*spells[i] == '!')
	continue;
      sprintf(help + strlen(help), "%18s", spells[i]);
      if (i % 4 == 3) {
	strcat(help, "\r\n");
	send_to_char(help, ch);
	*help = '\0';
      }
    }
    if (*help)
      send_to_char(help, ch);
    send_to_char("\n\r", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  skip_spaces(&argument);

  /* If there is no chars in argument */
  if (!*argument) {
    send_to_char("Skill name expected.\n\r", ch);
    return;
  }
  if (*argument != '\'') {
    send_to_char("Skill must be enclosed in: ''\n\r", ch);
    return;
  }
  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    send_to_char("Skill must be enclosed in: ''\n\r", ch);
    return;
  }
  strcpy(help, (argument + 1));
  help[qend - 1] = '\0';
  if ((skill = find_skill_num(help)) <= 0) {
    send_to_char("Unrecognized skill.\n\r", ch);
    return;
  }
  argument += qend + 1;		/* skip to next parameter */
  argument = one_argument(argument, buf);

  if (!*buf) {
    send_to_char("Learned value expected.\n\r", ch);
    return;
  }
  value = atoi(buf);
  if (value < 0) {
    send_to_char("Minimum value for learned is 0.\n\r", ch);
    return;
  }
  if (value > 100) {
    send_to_char("Max value for learned is 100.\n\r", ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char("You can't set NPC skills.\n\r", ch);
    return;
  }

  sprintf(buf2, "(GC) %s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict),
	  spells[skill], value);
  mudlog(buf2, BRF, GET_LEVEL(ch), TRUE);

  SET_SKILL(vict, skill, value);

  sprintf(buf2, "You change %s's %s to %d.\n\r", GET_NAME(vict),
	  spells[skill], value);
  send_to_char(buf2, ch);
}



/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

#define PAGE_LENGTH     22
#define PAGE_WIDTH      80

/* Traverse down the string until the begining of the next page has been
 * reached.  Return NULL if this is the last page of the string.
 */
char *next_page(char *str, int page_length)
{
  int col = 1, line = 1, spec_code = FALSE;

  for (;; str++) {
    /* If end of string, return NULL. */
    if (*str == '\0')
      return NULL;

    /* If we're at the start of the next page, return this fact. */
    else if (line > page_length)
      return str;

    /* Check for the begining of an ANSI color code block. */
    else if (*str == '\x1B' && !spec_code)
      spec_code = TRUE;

    /* Check for the end of an ANSI color code block. */
    else if (*str == 'm' && spec_code)
      spec_code = FALSE;

    /* Check for everything else. */
    else if (!spec_code) {
      /* Carriage return puts us in column one. */
      if (*str == '\r')
	col = 1;
      /* Newline puts us on the next line. */
      else if (*str == '\n')
	line++;

      /* We need to check here and see if we are over the page width,
       * and if so, compensate by going to the begining of the next line.
       */
      else if (col++ > PAGE_WIDTH) {
	col = 1;
	line++;
      }
    }
  }
}


/* Function that returns the number of pages in the string. */
int count_pages(char *str, int page_length)
{
  int pages;

  for (pages = 1; (str = next_page(str, page_length)); pages++);
  return pages;
}


/* This function assigns all the pointers for showstr_vector for the
 * page_string function, after showstr_vector has been allocated and
 * showstr_count set.
 */
void paginate_string(char *str, struct descriptor_data *d)
{
  int i, page_length=PAGE_LENGTH;

  if (d->showstr_count)
    *(d->showstr_vector) = str;

  if(d->character && (PAGE_LEN(d->character) > 0))
    page_length=PAGE_LEN(d->character);

  for (i = 1; i < d->showstr_count && str; i++)
    str = d->showstr_vector[i] = next_page(str, page_length);

  d->showstr_page = 0;
}


/* The call that gets the paging ball rolling... */
void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  int page_length=PAGE_LENGTH;

  if (!d)
    return;

  if (!str || !*str) {
    send_to_char("", d->character);
    return;
  }

  if(d->character && (PAGE_LEN(d->character) > 0))
    page_length=PAGE_LEN(d->character);

  CREATE(d->showstr_vector, char *, d->showstr_count = count_pages(str, page_length));

  if (keep_internal) {
    d->showstr_head = str_dup(str);
    paginate_string(d->showstr_head, d);
  } else
    paginate_string(str, d);

  show_string(d, "");
}


/* The call that displays the next page. */
void show_string(struct descriptor_data *d, char *input)
{
  char buffer[MAX_STRING_LENGTH];
  int diff;

  one_argument(input, buf);

  /* Q is for quit. :) */
  if (LOWER(*buf) == 'q') {
    free(d->showstr_vector);
    d->showstr_count = 0;
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = 0;
    }
    return;
  }
  /* R is for refresh, so back up one page internally so we can display
   * it again.
   */
  else if (LOWER(*buf) == 'r')
    d->showstr_page = MAX(0, d->showstr_page - 1);

  /* B is for back, so back up two pages internally so we can display the
   * correct page here.
   */
  else if (LOWER(*buf) == 'b')
    d->showstr_page = MAX(0, d->showstr_page - 2);

  /* Feature to 'goto' a page.  Just type the number of the page and you
   * are there!
   */
  else if (isdigit(*buf))
    d->showstr_page = MAX(0, MIN(atoi(buf) - 1, d->showstr_count - 1));

  else if (*buf) {
    send_to_char(
		  "Valid commands while paging are RETURN, Q, R, B, or a numeric value.\r\n",
		  d->character);
    return;
  }
  /* If we're displaying the last page, just send it to the character, and
   * then free up the space we used.
   */
  if (d->showstr_page + 1 >= d->showstr_count) {
    send_to_char(d->showstr_vector[d->showstr_page], d->character);
    free(d->showstr_vector);
    d->showstr_count = 0;
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = NULL;
    }
  }
  /* Or if we have more to show.... */
  else {
    strncpy(buffer, d->showstr_vector[d->showstr_page],
	    diff = ((int) d->showstr_vector[d->showstr_page + 1])
	    - ((int) d->showstr_vector[d->showstr_page]));
    buffer[diff] = '\0';
    send_to_char(buffer, d->character);
    d->showstr_page++;
  }
}
