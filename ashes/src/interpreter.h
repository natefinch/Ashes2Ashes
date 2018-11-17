/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

#define ACMD(name)  \
   void (name)(struct char_data *ch, char *argument, int cmd, int subcmd)

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) ((cmd<1)? FALSE : (!strcmp(cmd_name, cmd_info[cmd].command)))

/* necessary for CMD_IS macro */
#ifndef __INTERPRETER_C__
extern struct command_info cmd_info[];
#endif


void	command_interpreter(struct char_data *ch, char *argument);
int	search_block(char *arg, char **list, bool exact);
char	lower( char c );
char	*one_argument(char *argument, char *first_arg);
char	*one_word(char *argument, char *first_arg);
char	*any_one_arg(char *argument, char *first_arg);
char	*anyonearg(char *argument, char *first_arg);
char	*two_arguments(char *argument, char *first_arg, char *second_arg);
int	fill_word(char *argument);
void	half_chop(char *string, char *arg1, char *arg2);
void	nanny(struct descriptor_data *d, char *arg);
int	is_abbrev(char *arg1, char *arg2);
int	is_number(char *str);
int	find_command(char *command);
void	skip_spaces(char **string);
char	*delete_doubledollar(char *string);

/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)


struct command_info {
   char *command;
   byte minimum_position;
   void	(*command_pointer)
   (struct char_data *ch, char * argument, int cmd, int subcmd);
   sh_int minimum_level;
   int	subcmd;
   long grant; /* Grant bitvector, 0 if not grantable, otherwise bit(s) needed */
};

struct alias {
  char *alias;
  char *replacement;
  int type;
  struct alias *next;
};

#define ALIAS_SIMPLE	0
#define ALIAS_COMPLEX	1

#define ALIAS_SEP_CHAR	';'
#define ALIAS_VAR_CHAR	'$'
#define ALIAS_GLOB_CHAR	'*'

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH	1
#define SCMD_EAST	2
#define SCMD_SOUTH	3
#define SCMD_WEST	4
#define SCMD_UP		5
#define SCMD_DOWN	6
#define SCMD_NE		7
#define SCMD_NW		8
#define SCMD_SE		9
#define SCMD_SW		10

/* do_reimb */
#define SCMD_FORCE	1

/* do_backstab */
#define SCMD_DUAL	1

/* do_gen_ps */
#define SCMD_INFO       0
#define SCMD_HANDBOOK   1 
#define SCMD_CREDITS    2
#define SCMD_NEWS       3
#define SCMD_WIZLIST    4
#define SCMD_POLICIES   5
#define SCMD_VERSION    6
#define SCMD_IMMLIST    7
#define SCMD_MOTD	8
#define SCMD_IMOTD	9
#define SCMD_CLEAR	10
#define SCMD_WHOAMI	11

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL	4
#define SCMD_NOAUCTION	5
#define SCMD_DEAF	6
#define SCMD_NOGOSSIP	7
#define SCMD_NOGRATZ	8
#define SCMD_NOWIZ	9
#define SCMD_QUEST	10
#define SCMD_ROOMFLAGS	11
#define SCMD_NOREPEAT	12
#define SCMD_HOLYLIGHT	13
#define SCMD_SLOWNS	14
#define SCMD_AUTOEXIT	15
#define SCMD_NOMUS	16
#define SCMD_AFK	17
#define SCMD_MORTLOG	18
#define SCMD_NOASAY	19
#define SCMD_NOESP	20
#define SCMD_NOFRAN	21
#define SCMD_NOARENA	22
#define SCMD_AVTR	23
#define SCMD_COMBINE	24
#define SCMD_AUTOGOLD	25
#define SCMD_AUTOLOOT	26
#define SCMD_AUTOSPLIT	27
#define SCMD_NOMENU	28

/* do_wizutil */
#define SCMD_REROLL	0
#define SCMD_NOTITLE    1
#define SCMD_SQUELCH    2
#define SCMD_FREEZE	3
#define SCMD_THAW	4
#define SCMD_UNAFFECT	5

/* do_spec_com */
#define SCMD_WHISPER	0
#define SCMD_ASK	1

/* do_gen_com */
#define SCMD_HOLLER	0
#define SCMD_SHOUT	1
#define SCMD_GOSSIP	2
#define SCMD_AUCTION	3
#define SCMD_GRATZ	4
#define SCMD_MUS	5
#define SCMD_ASAY	6
#define SCMD_ESP	7
#define SCMD_FRENCH	8

/* do_zopen */
#define SCMD_CLOSE	1

/* do_load */
#define SCMD_FAKE	1

/* do_grep */
#define SCMD_IGREP	1

/* do_shutdown */
#define SCMD_SHUTDOW	0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUIT	1
#define SCMD_RQUIT	2
#define SCMD_QQUIT	3

/* do_date */
#define SCMD_DATE	0
#define SCMD_UPTIME	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2

/* do_look */
#define SCMD_LOOK	0
#define SCMD_READ	1

/* do_qcomm */
#define SCMD_QSAY	0
#define SCMD_QECHO	1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_poof */
#define SCMD_POOFIN	0
#define SCMD_POOFOUT	1

/* do_eat */
#define SCMD_EAT	0
#define SCMD_TASTE	1
#define SCMD_DRINK	2
#define SCMD_SIP	3

/* do_use */
#define SCMD_USE	0
#define SCMD_QUAFF	1
#define SCMD_RECITE	2

/* do_echo */
#define SCMD_ECHO	0
#define SCMD_EMOTE	1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4

/* do_walk */
#define SCMD_WALKIN	0
#define SCMD_WALKOUT	1

/* do_vc */
#define SCMD_JOIN	0
#define SCMD_DEPART	1
#define SCMD_SAY	2
#define SCMD_VEMOTE	3
#define SCMD_LIST	4
#define SCMD_INVITE	5
#define SCMD_VSQUELCH	6
#define SCMD_KICK	7
#define SCMD_SNOOP	8
#define SCMD_OWNER	9

/* do_zdelete */
#define SCMD_DELETE	1
