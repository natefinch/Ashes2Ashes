/* ************************************************************************
*   File: comm.c                                        Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  All changes from stock Circle 3.0 are copyright (C) 1997 by Jesse Sterr*
************************************************************************ */

#define __COMM_C__

#include "conf.h"
#include "sysdep.h"


#ifdef CIRCLE_WINDOWS		/* Includes for Win32 */
#include <direct.h>
#include <mmsystem.h>
#else				/* Includes for UNIX */
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "class.h"

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

/* externs */
extern int restrict_mud;
extern int mini_mud;
extern int no_rent_check;
extern FILE *player_fl;
extern int DFLT_PORT;
extern char *DFLT_DIR;
extern int MAX_PLAYERS;
extern int MAX_DESCRIPTORS_AVAILABLE;

extern struct room_data *world;	/* In db.c */
extern int top_of_world;	/* In db.c */
extern struct time_info_data time_info;		/* In db.c */
extern char help[];
extern struct player_special_data dummy_mob;	/* dummy spec area for mobs	 */

/* local globals */
struct descriptor_data *descriptor_list = NULL;	/* master desc list */
struct char_data *to_free_list = NULL;
struct txt_block *bufpool = 0;	/* pool of large output buffers */
int buf_largecount = 0;		/* # of large buffers which exist */
int buf_overflows = 0;		/* # of overflows of output */
int buf_switches = 0;		/* # of switches from small to large buf */
int circle_shutdown = 0;	/* clean shutdown */
int circle_reboot = 0;		/* reboot the game after a shutdown */
int auto_reboot = 0;		/* Automaticaly reboot after 24 hrs */
int server_boot = 0;		/* Will shutdown the mud completely */
int no_specials = 0;		/* Suppress ass. of special routines */
int max_players = 0;		/* max descriptors available */
int tics = 0;			/* for extern checkpointing */
int scheck = 0;			/* for syntax checking mode */
int bottles = 99;		/* for drinking song */
int drunk = 0;			/* determine if bottles should be reset */
int singers = 0;		/* for drinking song, group mentality */
bool MOBTrigger = TRUE;		/* For MOBProg */
extern int nameserver_is_slow;	/* see config.c */
extern int auto_save;		/* see config.c */
extern int autosave_time;	/* see config.c */
struct timeval null_time;	/* zero-valued time structure */

/* functions in this file */
int get_from_q(struct txt_q *queue, char *dest, int *aliased);
void init_game(int port);
void signal_setup(void);
void game_loop(int mother_desc);
int init_socket(int port);
int new_descriptor(int s);
int get_max_players(void);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval a, struct timeval b);
struct timeval timeadd(struct timeval a, struct timeval b);
void flush_queues(struct descriptor_data *d);
void nonblock(socket_t s);
int perform_subst(struct descriptor_data *t, char *orig, char *subst);
int perform_alias(struct descriptor_data *d, char *orig);
void record_usage(void);
void make_prompt(struct descriptor_data *point);
void check_idle_passwords(void);
void heartbeat(int pulse);
void drunk_check(void);


/* extern fcnts */
void boot_db(void);
void boot_world(void);
void zone_update(void);
void affect_update(void);	/* In magic.c */
void update_spells(void);       /* In magic.c */
void point_update(void);	/* In limits.c */
void mobile_activity(void);
void ferocity_check(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void show_string(struct descriptor_data *d, char *input);
int isbanned(char *hostname);
void weather_and_time(int mode);
void mprog_act_trigger(char *buf, struct char_data *mob, struct char_data *ch, struct obj_data *obj, void *vo);
void room_procs(void);
void item_procs(void);
void update_auction(void);


/* *********************************************************************
*  main game loop and related stuff                                    *
********************************************************************* */

/* Windows doesn't have gettimeofday, so we'll simulate it. */
#ifdef CIRCLE_WINDOWS

void gettimeofday(struct timeval *t, struct timezone *dummy)
{
  DWORD millisec = GetTickCount();

  t->tv_sec = (int) (millisec / 1000);
  t->tv_usec = (millisec % 1000) * 1000;
}

#endif


int main(int argc, char **argv)
{
  int port;
  char buf[512];
  int pos = 1;
  char *dir;

  port = DFLT_PORT;
  dir = DFLT_DIR;

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
    case 'd':
      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else {
	log("Directory arg expected after option -d.");
	exit(1);
      }
      break;
    case 'm':
      mini_mud = 1;
      no_rent_check = 1;
      log("Running in minimized mode & with no rent check.");
      break;
    case 'c':
      scheck = 1;
      log("Syntax check mode enabled.");
      break;
    case 'q':
      no_rent_check = 1;
      log("Quick boot mode -- rent check supressed.");
      break;
    case 'r':
      restrict_mud = 1;
      log("Restricting game -- no new players allowed.");
      break;
    case 's':
      no_specials = 1;
      log("Suppressing assignment of special routines.");
      break;
    case 'a':
      auto_reboot=1;
      log("Auto-reboot enabled.");
      break;
    default:
      sprintf(buf, "SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
      log(buf);
      break;
    }
    pos++;
  }

  if (pos < argc) {
    if (!isdigit(*argv[pos])) {
      fprintf(stderr, "Usage: %s [-a] [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n", argv[0]);
      exit(1);
    } else if ((port = atoi(argv[pos])) <= 1024) {
      fprintf(stderr, "Illegal port number.\n");
      exit(1);
    }
  }
#ifdef CIRCLE_WINDOWS
  if (_chdir(dir) < 0) {
#else
  if (chdir(dir) < 0) {
#endif
    perror("Fatal error changing to data directory");
    exit(1);
  }
  sprintf(buf, "Using %s as data directory.", dir);
  log(buf);

  if (scheck) {
    boot_world();
    log("Done.");
    exit(0);
  } else {
    sprintf(buf, "Running game on port %d.", port);
    log(buf);
    init_game(port);
  }

  return 0;
}



/* Init sockets, run game, and cleanup sockets */
void init_game(int port)
{
  int mother_desc;

  srandom(time(0));

  log("Finding player limit.");
  max_players = get_max_players();

  log("Opening mother connection.");
  mother_desc = init_socket(port);

  boot_db();

#ifndef CIRCLE_WINDOWS
  log("Signal trapping.");
  signal_setup();
#endif

  log("Entering game loop.");

  game_loop(mother_desc);

  log("Closing all sockets.");
  while (descriptor_list)
    close_socket(descriptor_list);

  CLOSE_SOCKET(mother_desc);
  fclose(player_fl);

  if (circle_reboot) {
    log("Rebooting.");
    exit(52);			/* what's so great about HHGTTG, anyhow? */
  }
  log("Normal termination of game.");
}



/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
int init_socket(int port)
{
  int s, opt;
  struct sockaddr_in sa;

  /*
   * Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so ths point is (hopefully) moot.
   */

#ifdef CIRCLE_WINDOWS
  {
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(1, 1);

    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
      log("WinSock not available!\n");
      exit(1);
    }
    if ((wsaData.iMaxSockets - 4) < max_players) {
      max_players = wsaData.iMaxSockets - 4;
    }
    sprintf(buf, "Max players set to %d", max_players);
    log(buf);

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
      fprintf(stderr, "Error opening network connection: Winsock err #%d\n", WSAGetLastError());
      exit(1);
    }
  }
#else
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating socket");
    exit(1);
  }
#endif				/* CIRCLE_WINDOWS */

#if defined(SO_SNDBUF)
  opt = LARGE_BUFSIZE + GARBAGE_SPACE;
  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt SNDBUF");
    exit(1);
  }
#endif

#if defined(SO_REUSEADDR)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }
#endif

#if defined(SO_LINGER)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0) {
      perror("setsockopt LINGER");
      exit(1);
    }
  }
#endif

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("bind");
    CLOSE_SOCKET(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5);
  return s;
}


int get_max_players(void)
{
#if defined(CIRCLE_OS2) || defined(CIRCLE_WINDOWS)
  return MAX_PLAYERS;
#else

  int max_descs = 0;
  char *method;

/*
 * First, we'll try using getrlimit/setrlimit.  This will probably work
 * on most systems.
 */
#if defined (RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#if !defined(RLIMIT_NOFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
  {
    struct rlimit limit;

    /* find the limit of file descs */
    method = "rlimit";
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("calling getrlimit");
      exit(1);
    }
    /* set the current to the maximum */
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("calling setrlimit");
      exit(1);
    }
#ifdef RLIM_INFINITY
    if (limit.rlim_max == RLIM_INFINITY)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else
      max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#else
    max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
  }

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
  method = "OPEN_MAX";
  max_descs = OPEN_MAX;		/* Uh oh.. rlimit didn't work, but we have
				 * OPEN_MAX */
#elif defined (POSIX)
  /*
   * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
   * use the POSIX sysconf() function.  (See Stevens' _Advanced Programming
   * in the UNIX Environment_).
   */
  method = "POSIX sysconf";
  errno = 0;
  if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
    if (errno == 0)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else {
      perror("Error calling sysconf");
      exit(1);
    }
  }
#else
  /* if everything has failed, we'll just take a guess */
  max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
#endif

  /* now calculate max _players_ based on max descs */
  max_descs = MIN(MAX_PLAYERS, max_descs - NUM_RESERVED_DESCS);

  if (max_descs <= 0) {
    sprintf(buf, "Non-positive max player limit!  (Set at %d using %s).",
	    max_descs, method);
    log(buf);
    exit(1);
  }
  sprintf(buf, "Setting player limit to %d using %s.", max_descs, method);
  log(buf);
  return max_descs;
#endif				/* WINDOWS or OS2 */
}



/*
 * game_loop contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for accepting new
 * new connections, polling existing connections for input, dequeueing
 * output and sending it out to players, and calling "heartbeat" functions
 * such as mobile_activity().
 */
void game_loop(int mother_desc)
{
  fd_set input_set, output_set, exc_set, null_set;
  struct timeval last_time, before_sleep, opt_time, process_time, now, timeout;
  char comm[MAX_INPUT_LENGTH];
  struct descriptor_data *d, *next_d;
  int pulse = 0, missed_pulses, maxdesc, aliased;
  struct char_data *syschar;
  struct tm *tmp_time;
  time_t mytime;
  int day;
  extern time_t boot_time;
  extern struct char_data *character_list;

  ACMD(do_page);
  ACMD(do_shutdown);

  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  FD_ZERO(&null_set);

  gettimeofday(&last_time, (struct timezone *) 0);

  /* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. */
  while (!circle_shutdown) {

    /* Sleep if we don't have any connections */
    if ((descriptor_list == NULL)&&(auto_reboot<2)) {
      log("No connections.  Going to sleep.");
      FD_ZERO(&input_set);
      FD_SET(mother_desc, &input_set);
      if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
	if (errno == EINTR)
	  log("Waking up to process signal.");
	else
	  perror("Select coma");
      } else
	log("New connection.  Waking up.");
      gettimeofday(&last_time, (struct timezone *) 0);
    }
    /* Set up the input, output, and exception sets for select(). */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);

    maxdesc = mother_desc;
    for (d = descriptor_list; d; d = d->next) {
#ifndef CIRCLE_WINDOWS
      if (d->descriptor > maxdesc)
	maxdesc = d->descriptor;
#endif
      FD_SET(d->descriptor, &input_set);
      FD_SET(d->descriptor, &output_set);
      FD_SET(d->descriptor, &exc_set);
    }

    /*
     * At this point, we have completed all input, output and heartbeat
     * activity from the previous iteration, so we have to put ourselves
     * to sleep until the next 0.1 second tick.  The first step is to
     * calculate how long we took processing the previous iteration.
     */
    
    gettimeofday(&before_sleep, (struct timezone *) 0); /* current time */
    process_time = timediff(before_sleep, last_time);

    /*
     * If we were asleep for more than one pass, count missed pulses and sleep
     * until we're resynchronized with the next upcoming pulse.
     */
    if (process_time.tv_sec == 0 && process_time.tv_usec < OPT_USEC) {
      missed_pulses = 0;
    } else {
      missed_pulses = process_time.tv_sec * PASSES_PER_SEC;
      missed_pulses += process_time.tv_usec / OPT_USEC;
      process_time.tv_sec = 0;
      process_time.tv_usec = process_time.tv_usec % OPT_USEC;
    }

    /* Calculate the time we should wake up */
    last_time = timeadd(before_sleep, timediff(opt_time, process_time));

    /* Now keep sleeping until that time has come */
    gettimeofday(&now, (struct timezone *) 0);
    timeout = timediff(last_time, now);

    /* go to sleep */
    do {
#ifdef CIRCLE_WINDOWS
      Sleep(timeout.tv_sec * 1000 + timeout.tv_usec / 1000);
#else
      if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
	if (errno != EINTR) {
	  perror("Select sleep");
	  exit(1);
	}
      }
#endif /* CIRCLE_WINDOWS */
      gettimeofday(&now, (struct timezone *) 0);
      timeout = timediff(last_time, now);
    } while (timeout.tv_usec || timeout.tv_sec);

    /* poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("Select poll");
      return;
    }
    /* If there are new connections waiting, accept them. */
    if (FD_ISSET(mother_desc, &input_set))
      new_descriptor(mother_desc);

    /* kick out the freaky folks in the exception set */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &exc_set)) {
	FD_CLR(d->descriptor, &input_set);
	FD_CLR(d->descriptor, &output_set);
	close_socket(d);
      }
    }

    /* process descriptors with input pending */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &input_set))
	if (process_input(d) < 0) {
	  close_socket(d);
        }
    }

    /* process commands we just read from process_input */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;

      if (--(d->wait) <= 0) {
        d->wait=0;
        if(get_from_q(&d->input, comm, &aliased)) {
          if (d->character) {
            /* reset the idle timer & pull char back from void if necessary */
            d->character->char_specials.timer = 0;
            if (!d->connected && GET_WAS_IN(d->character) != NOWHERE) {
              if (d->character->in_room != NOWHERE)
                char_from_room(d->character);
              char_to_room(d->character, GET_WAS_IN(d->character));
              GET_WAS_IN(d->character) = NOWHERE;
              act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
            }
          }
          d->wait = 1;
          d->prompt_mode = 1;

          if (d->str)		/* writing boards, mail, etc.     */
            string_add(d, comm);
          else if (d->showstr_count)	/* reading something w/ pager     */
            show_string(d, comm);
          else if (d->connected != CON_PLAYING)	/* in menus, etc. */
            nanny(d, comm);
          else {			/* else: we're playing normally */
            if (aliased)		/* to prevent recursive aliases */
              d->prompt_mode = 0;
            else {
              if (perform_alias(d, comm))		/* run it through aliasing system */
                get_from_q(&d->input, comm, &aliased);
            }
            command_interpreter(d->character, comm);	/* send it to interpreter */
          }
        }
      }
    }

    /* send queued output out to the operating system (ultimately to user) */
    /* and give each descriptor an appropriate prompt */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if(d->prompt_mode) {
	make_prompt(d);
	d->prompt_mode = 2;
      }
      if (FD_ISSET(d->descriptor, &output_set) && *(d->output)) {
        if(d->prompt_mode==0)
          make_prompt(d);
	if (process_output(d) < 0) {
	  close_socket(d);
        }
      }
      d->prompt_mode = 0;
    }

    /* kick out folks in the CON_CLOSE state */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (STATE(d) == CON_CLOSE)
	close_socket(d);
    }

    /*
     * Now, we execute as many pulses as necessary--just one if we haven't
     * missed any pulses, or make up for lost time if we missed a few
     * pulses by sleeping for too long.
     */
    missed_pulses++;

    if (missed_pulses <= 0) {
      log("SYSERR: MISSED_PULSES IS NONPOSITIVE!!!");
      missed_pulses = 1;
    }

    /* If we missed more than 30 seconds worth of pulses, forget it */
    if (missed_pulses > (30 * PASSES_PER_SEC)) {
      log("Warning: Missed more than 30 seconds worth of pulses");
      missed_pulses = 30 * PASSES_PER_SEC;
    }

    /* Now execute the heartbeat functions */
    while (missed_pulses--) {
      heartbeat(++pulse);
    }

    /* Every min check for auto-reboot */
    if(!(pulse % (60 * PASSES_PER_SEC))) {
      mytime = time(0) - boot_time;
      day = mytime / 86400;
/*
      mytime=time(0);
      if((tmp_time=localtime(&mytime))->tm_wday==0) {
        if(tmp_time->tm_hour==12) {
          if((tmp_time->tm_min > 1) && (tmp_time->tm_min < 15)) {
            server_boot=1;
          }
        }
      }
*/
      if(((auto_reboot==1) && day) || (auto_reboot > 1) || server_boot) {
        if(server_boot)
          strcpy(buf1, "Shutdown for server reboot");
        else
          strcpy(buf1, "Auto-reboot");
        syschar=create_char();
        GET_LEVEL(syschar)=LVL_IMPL;
        GET_NAME(syschar)=str_dup("SYSTEM");
        syschar->player_specials=&dummy_mob;
        switch(auto_reboot++) {
        case 1:
          sprintf(buf, "all %s in 10 min, don't die.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 2:
          sprintf(buf, "all %s in 9 min, please quit soon.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 3:
          sprintf(buf, "all %s in 8 min, don't die.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 4:
          sprintf(buf, "all %s in 7 min, please quit soon.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 5:
          sprintf(buf, "all %s in 6 min, don't die.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 6:
          sprintf(buf, "all %s in 5 min, please quit soon.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 7:
          sprintf(buf, "all %s in 4 min, don't die.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 8:
          sprintf(buf, "all %s in 3 min, make sure you don't have too many items.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 9:
          sprintf(buf, "all %s in 2 min, better hurry.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        case 10:
          sprintf(buf, "all %s in 1 min, LAST CHANCE.", buf1);
          do_page(syschar, buf, 0, 0);
          break;
        default:
          do_shutdown(syschar, (server_boot?"die":"reboot"), 0, SCMD_SHUTDOWN);
          log("Auto-rebooting.");
          break;
        }
        character_list=syschar->next;
        syschar->player_specials=NULL;
        free_char(syschar);
      }
    }

    /* Roll pulse over after 30 min */
    if (pulse >= (30 * 60 * PASSES_PER_SEC))
      pulse = 0;

    /* Update tics for deadlock protection (UNIX only) */
    tics++;
  }

}


void heartbeat(int pulse)
{
  static int mins_since_crashsave = 0;
  static struct char_data *ch;

  if (!(pulse % PULSE_ZONE)) {
    zone_update();
  }

  if (!(pulse % (45 * PASSES_PER_SEC))) {	/* 45 seconds */
    check_idle_passwords();
  }

  if (!(pulse % PULSE_SPELL)) {
    update_spells();
  }

  if (!(pulse % PULSE_MOBILE)) {
    mobile_activity();
  }

  if (!(pulse % PULSE_VIOLENCE)) {
    perform_violence();
    ferocity_check();
    drunk_check();
  }

  if (!(pulse % PULSE_ROOM)) {
    room_procs();
  }

  if (!(pulse % PULSE_ITEM)) {
    item_procs();
  }

  if (!(pulse % PULSE_AUCTION)) {
    update_auction();
  }

  if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) {
    weather_and_time(1);
    affect_update();
    point_update();
    fflush(player_fl);
  }

  if (auto_save && !(pulse % (60 * PASSES_PER_SEC))) {	/* 1 minute */
    if (++mins_since_crashsave >= autosave_time) {
      mins_since_crashsave = 0;
      Crash_save_all();
    }
  }

  if (!(pulse % (15 * 60 * PASSES_PER_SEC))) {	/* 15 minutes */
    record_usage();
    if(drunk == -1) {
      drunk = 0;
      bottles = 99;
    }
    if(drunk == 1)
      drunk = -1;
  }

  while(to_free_list) {
    ch=to_free_list;
    to_free_list=ch->next;
    free_char(ch);
  }
}

void drunk_check(void)
{
  int i=0;
  struct descriptor_data *d;
  char song[MAX_INPUT_LENGTH];

  for(d=descriptor_list; d; d=d->next) {
    if(d->character && (!d->connected) && (!d->str)) {
      if((GET_COND(d->character, DRUNK) >= 20) || ((GET_COND(d->character, DRUNK) > (10 - (singers/30))) && (singers > 0))) {
        if(GET_COND(d->character, DRUNK) > number(1, 250)) {
          REMOVE_BIT(PRF_FLAGS(d->character), PRF_NOMUS);
          if(bottles > 1)
            sprintf(song, "music %d bottles of beer on the wall, %d bottles of beer,", bottles, bottles);
          else
            sprintf(song, "music 1 bottle of beer on the wall, 1 bottle of beer,");
          command_interpreter(d->character, song);
          bottles--;
          if(bottles > 0)
            sprintf(song, "music take one down, pass it around, %d bottle%s of beer on the wall", bottles, ((bottles>1)?"s":""));
          else {
            sprintf(song, "music take it down, restock the shelf! 99 bottles of beer on the wall");
            bottles=99;
            singers=i=0;
          }
          command_interpreter(d->character, song);
          drunk=1;
          singers+=3;
          i++;
        }
      }
    }
  }

  if(i)
    singers-=2*i;
  else
    singers-=2;
  if(singers < 0)
    singers=0;
  if(singers > 250)
    singers=250;
}


/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

/*
 *  new code to calculate time differences, which works on systems
 *  for which tv_usec is unsigned (and thus comparisons for something
 *  being < 0 fail).  Based on code submitted by ss@sirocco.cup.hp.com.
 */

/*
 * code to return the time difference between a and b (a-b).
 * always returns a nonnegative value (floors at 0).
 */
struct timeval timediff(struct timeval a, struct timeval b)
{
  struct timeval rslt;

  if (a.tv_sec < b.tv_sec)
    return null_time;
  else if (a.tv_sec == b.tv_sec) {
    if (a.tv_usec < b.tv_usec)
      return null_time;
    else {
      rslt.tv_sec = 0;
      rslt.tv_usec = a.tv_usec - b.tv_usec;
      return rslt;
    }
  } else {			/* a->tv_sec > b->tv_sec */
    rslt.tv_sec = a.tv_sec - b.tv_sec;
    if (a.tv_usec < b.tv_usec) {
      rslt.tv_usec = a.tv_usec + 1000000 - b.tv_usec;
      rslt.tv_sec--;
    } else
      rslt.tv_usec = a.tv_usec - b.tv_usec;
    return rslt;
  }
}

/* add 2 timevals */
struct timeval timeadd(struct timeval a, struct timeval b)
{
  struct timeval rslt;

  rslt.tv_sec = a.tv_sec + b.tv_sec;
  rslt.tv_usec = a.tv_usec + b.tv_usec;

  while (rslt.tv_usec >= 1000000) {
    rslt.tv_usec -= 1000000;
    rslt.tv_sec++;
  }

  return rslt;
}


void record_usage(void)
{
  int sockets_connected = 0, sockets_playing = 0;
  struct descriptor_data *d;
  char buf[256];

  for (d = descriptor_list; d; d = d->next) {
    sockets_connected++;
    if (!d->connected)
      sockets_playing++;
  }

  sprintf(buf, "nusage: %-3d sockets connected, %-3d sockets playing",
	  sockets_connected, sockets_playing);
  log(buf);

#ifdef RUSAGE
  {
    struct rusage ru;

    getrusage(0, &ru);
    sprintf(buf, "rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
	    ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
    log(buf);
  }
#endif

}



/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  SEND_TO_Q(off_string, d);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) TELOPT_NAOFFD,
    (char) TELOPT_NAOCRD,
    (char) 0,
  };

  SEND_TO_Q(on_string, d);
}


void make_prompt(struct descriptor_data *d)
{
  char prompt[MAX_INPUT_LENGTH];

  if(!d->character)
    return;

  if (d->str)
    send_to_char("] ", d->character);
  else if (d->showstr_count) {
    sprintf(prompt,
	    "\r\n[ Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d) ]",
	    d->showstr_page, d->showstr_count);
    send_to_char(prompt, d->character);
  } else if (!d->connected) {
    char cc[30], p2[MAX_INPUT_LENGTH], *lastc;
    int i, j=0, c=0, v[MAX_PROMPT_TOKENS], n;

    /* add the extra CRLF if the person isn't in compact mode */
    if (!PRF_FLAGGED(d->character, PRF_COMPACT)) {
      prompt[0]='\r';
      prompt[1]='\n';
      prompt[2]=0;
      j=2;
    }
    else {
      prompt[0]=0;
    }
    if(d->character->char_specials.prompt[0]=='*')
    {
      strcat(prompt,"<%d/%dH %d/%dM %d/%dV>");
      v[0]=d->character->points.hit;
      v[1]=d->character->points.max_hit;
      v[2]=d->character->points.mana;
      v[3]=d->character->points.max_mana;
      v[4]=d->character->points.move;
      v[5]=d->character->points.max_move;
    }
    else {
      lastc=(COLOR_LEV(d->character) >= C_SPR ? KNRM : KNUL);
      for(i=0; d->character->char_specials.prompt[i]; i++)
      {
        if((prompt[j++]=d->character->char_specials.prompt[i])=='%')
        {
          switch(d->character->char_specials.prompt[++i])
          {
            case 'a':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->char_specials.saved.alignment;
              break;
            }
            case 'A': {
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=compute_ac(d->character);
              break;
            }
            case 'b':{
              prompt[j-1]=0;
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (IS_GOOD(d->character)? KGRN :(IS_EVIL(d->character)? KRED :KYEL)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->char_specials.saved.alignment;
              break;
            }
            case 'c':
            case 'C':{
              switch(d->character->char_specials.prompt[++i]) {
              case '0':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KNRM : KNUL)));
                break;
              case '1':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KRED : KNUL)));
                break;
              case '2':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KGRN : KNUL)));
                break;
              case '3':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KYEL : KNUL)));
                break;
              case '4':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KBLU : KNUL)));
                break;
              case '5':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KMAG : KNUL)));
                break;
              case '6':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KCYN : KNUL)));
                break;
              case '7':
                strcpy(cc, (lastc=(COLOR_LEV(d->character) >= C_SPR ? KWHT : KNUL)));
                break;
              default:
                strcpy(cc, "!");
                break;
              }
              prompt[j-1]=0;
              strcat(prompt, cc);
              j+=strlen(cc)-1;            
              break;
            }
            case 'd':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->char_specials.spdrained;
              break;
            }
            case 'D':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->char_specials.spnocast;
              break;
            }
            case 'g':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)] = (int)GET_GOLD(d->character);
              break;
            }
            case 'h':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.hit;
              break;
            }
            case 'H':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.max_hit;
              break;
            }
            case 'i':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=GET_INVIS_LEV(d->character);
              break;
            }
            case 'j':{
              prompt[j-1]=0;
              n=(2*d->character->points.max_hit)/(d->character->points.hit?d->character->points.hit:1);
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (n>5 ? KRED :(n>2 ? KYEL :KGRN)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.hit;
              break;
            }
            case 'J':{
              prompt[j-1]=0;
              n=(2*d->character->points.max_hit)/(d->character->points.hit?d->character->points.hit:1);
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (n>5 ? KRED :(n>2 ? KYEL :KGRN)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.max_hit;
              break;
            }
            case 'm':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.mana;
              break;
            }
            case 'M':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.max_mana;
              break;
            }
            case 'n':{
              prompt[j-1]=0;
              n=(2*d->character->points.max_mana)/(d->character->points.mana?d->character->points.mana:1);
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (n>5 ? KRED :(n>2 ? KYEL :KGRN)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.mana;
              break;
            }
            case 'N':{
              prompt[j-1]=0;
              n=(2*d->character->points.max_mana)/(d->character->points.mana?d->character->points.mana:1);
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (n>5 ? KRED :(n>2 ? KYEL :KGRN)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.max_mana;
              break;
            }
            case 'r':{
              prompt[j-1]='\r';
              prompt[j++]='\n';
              break;
            }
            case 't':{
              if((FIGHTING(d->character))&&(FIGHTING(FIGHTING(d->character)))&&(FIGHTING(FIGHTING(d->character))!=d->character))
              {
                prompt[j++]='d';
                v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->char_specials.fighting->char_specials.fighting->points.hit;
              }
              else
                j--;
              break;
            }
            case 'v':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.move;
              break;
            }
            case 'V':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.max_move;
              break;
            }
            case 'w':{
              prompt[j-1]=0;
              n=(2*d->character->points.max_move)/(d->character->points.move?d->character->points.move:1);
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (n>5 ? KRED :(n>2 ? KYEL :KGRN)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.move;
              break;
            }
            case 'W':{
              prompt[j-1]=0;
              n=(2*d->character->points.max_move)/(d->character->points.move?d->character->points.move:1);
              strcpy(cc, (COLOR_LEV(d->character) >= C_SPR ? (n>5 ? KRED :(n>2 ? KYEL :KGRN)) : KNUL));
              strcat(cc, "%d");
              strcat(cc, lastc);
              strcat(prompt, cc);
              j+=strlen(cc)-1;
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=d->character->points.max_move;
              break;
            }
            case 'x':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=((GET_NUM_CLASSES(d->character)*exp_table[(int)GET_CLASS(d->character)][GET_LEVEL(d->character)+1])-GET_EXP(d->character));
              break;
            }
            case 'X':{
              prompt[j++]='d';
              v[(c<(MAX_PROMPT_TOKENS - 1)?c++:c)]=
                (100*((GET_NUM_CLASSES(d->character)*exp_table[(int)GET_CLASS(d->character)][GET_LEVEL(d->character)+1])-GET_EXP(d->character)))/
                ((GET_NUM_CLASSES(d->character)*exp_table[(int)GET_CLASS(d->character)][GET_LEVEL(d->character)+1])-(GET_NUM_CLASSES(d->character)*exp_table[(int)GET_CLASS(d->character)][GET_LEVEL(d->character)]));
              break;
            }
            case '%':{
              prompt[j++]='%';
              break;
            }
            case 0:{
              prompt[j-1]='!';
              i--;
              break;
            }
            default:{
              prompt[j-1]='!';
              break;
            }
          }
        }
      }
      prompt[j] = '\0';
    }
    strcat(prompt, (COLOR_LEV(d->character) >= C_SPR ? KNRM: KNUL));
    if(MAX_PROMPT_TOKENS != 18)
      send_to_char("Prompts need fixing\r\n", d->character);
    sprintf(p2, prompt, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17]);

    send_to_char(p2, d->character);
  }
}


void write_to_q(char *txt, struct txt_q *queue, int aliased)
{
  struct txt_block *new;

  CREATE(new, struct txt_block, 1);
  CREATE(new->text, char, strlen(txt) + 1);
  strcpy(new->text, txt);
  new->aliased = aliased;

  /* queue empty? */
  if (!queue->head) {
    new->next = NULL;
    queue->head = queue->tail = new;
  } else {
    queue->tail->next = new;
    queue->tail = new;
    new->next = NULL;
  }
}



int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
  struct txt_block *tmp;

  /* queue empty? */
  if (!queue->head)
    return 0;

  tmp = queue->head;
  strcpy(dest, queue->head->text);
  *aliased = queue->head->aliased;
  queue->head = queue->head->next;

  free(tmp->text);
  free(tmp);

  return 1;
}



/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
  int dummy;

  if (d->large_outbuf) {
    d->large_outbuf->next = bufpool;
    bufpool = d->large_outbuf;
  }
  while (get_from_q(&d->input, buf2, &dummy));
}

/* Add a new string to a player's output queue */
void write_to_output(const char *txt, struct descriptor_data *t)
{
  int size;

  size = strlen(txt);

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufptr < 0)
    return;

  /* if we have enough space, just write to buffer and that's it! */
  if (t->bufspace >= size) {
    strcpy(t->output + t->bufptr, txt);
    t->bufspace -= size;
    t->bufptr += size;
    return;
  }
  /*
   * If we're already using the large buffer, or if even the large buffer
   * is too small to handle this new text, chuck the text and switch to the
   * overflow state.
   */
  if (t->large_outbuf || ((size + strlen(t->output)) > LARGE_BUFSIZE)) {
    t->bufptr = -1;
    buf_overflows++;
    return;
  }
  buf_switches++;

  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {			/* else create a new one */
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output);	/* copy to big buffer */
  t->output = t->large_outbuf->text;	/* make big buffer primary */
  strcat(t->output, txt);	/* now add new text */

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - strlen(t->output);

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);
}



/* ******************************************************************
*  socket handling                                                  *
****************************************************************** */


int new_descriptor(int s)
{
  socket_t desc;
  int sockets_connected = 0;
  unsigned long addr;
  int i;
  static int last_desc = 0;	/* last descriptor number */
  struct descriptor_data *newd;
  struct sockaddr_in peer;
  struct hostent *from;
  extern char *greetings;

  /* accept the new connection */
  i = sizeof(peer);
  if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET) {
    perror("accept");
    return -1;
  }
  /* keep it from blocking */
  nonblock(desc);

  /* make sure we have room for it */
  for (newd = descriptor_list; newd; newd = newd->next)
    sockets_connected++;

  if (sockets_connected >= max_players) {
    write_to_descriptor(desc, "Sorry, Ashes to Ashes is full right now... try again later!  :-)\r\n");
    CLOSE_SOCKET(desc);
    return 0;
  }
  /* create a new descriptor */
  CREATE(newd, struct descriptor_data, 1);
  memset((char *) newd, 0, sizeof(struct descriptor_data));

  /* find the sitename */
  if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr,
				      sizeof(peer.sin_addr), AF_INET))) {

    /* resolution failed */
    if (!nameserver_is_slow)
      perror("gethostbyaddr");

    /* find the numeric site address */
    addr = ntohl(peer.sin_addr.s_addr);
    sprintf(newd->host, "%03u.%03u.%03u.%03u", (int) ((addr & 0xFF000000) >> 24),
     (int) ((addr & 0x00FF0000) >> 16), (int) ((addr & 0x0000FF00) >> 8),
	    (int) ((addr & 0x000000FF)));
  } else {
    strncpy(newd->host, from->h_name, HOST_LENGTH);
    *(newd->host + HOST_LENGTH) = '\0';
  }

  /* determine if the site is banned */
  if (isbanned(newd->host) == BAN_ALL) {
    CLOSE_SOCKET(desc);
    sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
    mudlog(buf2, CMP, LVL_ASST, TRUE);
    free(newd);
    return 0;
  }

  /* initialize descriptor data */
  newd->descriptor = desc;
  newd->connected = CON_GET_NAME;
  newd->idle_tics = 0;
  newd->wait = 1;
  newd->output = newd->small_outbuf;
  newd->bufspace = SMALL_BUFSIZE - 1;
  newd->next = descriptor_list;
  newd->login_time = time(0);

  if (++last_desc == 1000)
    last_desc = 1;
  newd->desc_num = last_desc;

  /* prepend to list */
  descriptor_list = newd;

  SEND_TO_Q(greetings, newd);
  SEND_TO_Q("By what name do you wish to be known? ", newd);

  return 0;
}



int process_output(struct descriptor_data *t)
{
  static char i[LARGE_BUFSIZE + GARBAGE_SPACE];
  static char snoop_log[LARGE_BUFSIZE + 100];
  static int result;

  /* we may need this \r\n for later -- see below */
  strcpy(i, "\r\n");

  /* now, append the 'real' output */
  strcpy(i + 2, t->output);

  /* if we're in the overflow state, notify the user */
  if (t->bufptr < 0)
    strcat(i, "**OVERFLOW**");

  /*
   * now, send the output.  If this is an 'interruption', use the prepended
   * CRLF, otherwise send the straight output sans CRLF.
   */
  if (!t->prompt_mode)		/* && !t->connected) */
    result = write_to_descriptor(t->descriptor, i);
  else
    result = write_to_descriptor(t->descriptor, i + 2);

  /* handle snooping: prepend "% " and send to snooper */
  if (t->snoop_by) {
    SEND_TO_Q("% ", t->snoop_by);
    SEND_TO_Q(t->output, t->snoop_by);
    SEND_TO_Q("%%", t->snoop_by);
  }
  if ((t->character) && (PLR_FLAGGED(t->character, PLR_SNOOP))) {
    sprintf(snoop_log, "%sLOG %s", GET_NAME(t->character), t->output);
    log(snoop_log);
  }
  /*
   * if we were using a large buffer, put the large buffer on the buffer pool
   * and switch back to the small one
   */
  if (t->large_outbuf) {
    t->large_outbuf->next = bufpool;
    bufpool = t->large_outbuf;
    t->large_outbuf = NULL;
    t->output = t->small_outbuf;
  }
  /* reset total bufspace back to that of a small buffer */
  t->bufspace = SMALL_BUFSIZE - 1;
  t->bufptr = 0;
  *(t->output) = '\0';

  return result;
}



int write_to_descriptor(socket_t desc, char *txt)
{
  int total, bytes_written;

  total = strlen(txt);

  do {
#ifdef CIRCLE_WINDOWS
    if ((bytes_written = send(desc, txt, total, 0)) < 0) {
      if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
    if ((bytes_written = write(desc, txt, total)) < 0) {
#ifdef EWOULDBLOCK
      if (errno == EWOULDBLOCK)
	errno = EAGAIN;
#endif /* EWOULDBLOCK */
      if (errno == EAGAIN)
#endif /* CIRCLE_WINDOWS */
	log("process_output: socket write would block, about to close");
      else
	perror("Write to socket");
      return -1;
    } else {
      txt += bytes_written;
      total -= bytes_written;
    }
  } while (total > 0);

  return 0;
}


/*
 * ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 */
int process_input(struct descriptor_data *t)
{
  int buf_length, bytes_read, space_left, failed_subst;
  char *ptr, *read_point, *write_point, *nl_pos = NULL;
  static char tmp[MAX_INPUT_LENGTH + 8];
  static char snoop_log[MAX_INPUT_LENGTH + 100];

  /* first, find the point where we left off reading data */
  buf_length = strlen(t->inbuf);
  read_point = t->inbuf + buf_length;
  space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

  do {
    if (space_left <= 0) {
      log("process_input: about to close connection: input overflow");
      return -1;
    }
#ifdef CIRCLE_WINDOWS
    if ((bytes_read = recv(t->descriptor, read_point, space_left, 0)) < 0) {
      if (WSAGetLastError() != WSAEWOULDBLOCK) {
#else
    if ((bytes_read = read(t->descriptor, read_point, space_left)) < 0) {
#ifdef EWOULDBLOCK
      if (errno == EWOULDBLOCK)
	errno = EAGAIN;
#endif /* EWOULDBLOCK */
      if (errno != EAGAIN) {
#endif /* CIRCLE_WINDOWS */
	perror("process_input: about to lose connection");
	return -1;		/* some error condition was encountered on
				 * read */
      } else
	return 0;		/* the read would have blocked: just means no
				 * data there but everything's okay */
    } else if (bytes_read == 0) {
      log("EOF on socket read (connection broken by peer)");
      return -1;
    }
    /* at this point, we know we got some data from the read */

    *(read_point + bytes_read) = '\0';	/* terminate the string */

    /* search for a newline in the data we just read */
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;

    read_point += bytes_read;
    space_left -= bytes_read;

/*
 * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
 * causing the MUD to hang when it encounters input not terminated by a
 * newline.  This was causing hangs at the Password: prompt, for example.
 * I attempt to compensate by always returning after the _first_ read, instead
 * of looping forever until a read returns -1.  This simulates non-blocking
 * I/O because the result is we never call read unless we know from select()
 * that data is ready (process_input is only called if select indicates that
 * this descriptor is in the read set).  JE 2/23/95.
 */
#if !defined(POSIX_NONBLOCK_BROKEN)
  } while (nl_pos == NULL);
#else
  } while (0);

  if (nl_pos == NULL)
    return 0;
#endif /* POSIX_NONBLOCK_BROKEN */

  /*
   * okay, at this point we have at least one newline in the string; now we
   * can copy the formatted data to a new array for further processing.
   */

  read_point = t->inbuf;

  while (nl_pos != NULL) {
    write_point = tmp;
    space_left = MAX_INPUT_LENGTH - 1;

    for (ptr = read_point; (space_left > 0) && (ptr < nl_pos); ptr++) {
      if (*ptr == '\b') {	/* handle backspacing */
	if (write_point > tmp) {
	  if (*(--write_point) == '$') {
	    write_point--;
	    space_left += 2;
	  } else
	    space_left++;
	}
      } else if (isascii(*ptr) && isprint(*ptr)) {
	if ((*(write_point++) = *ptr) == '$') {		/* copy one character */
	  *(write_point++) = '$';	/* if it's a $, double it */
	  space_left -= 2;
	} else
	  space_left--;
      }
    }

    *write_point = '\0';

    if ((space_left <= 0) && (ptr < nl_pos)) {
      char buffer[MAX_INPUT_LENGTH + 64];

      sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
      if (write_to_descriptor(t->descriptor, buffer) < 0)
	return -1;
    }
    if (t->snoop_by) {
      SEND_TO_Q("% ", t->snoop_by);
      SEND_TO_Q(tmp, t->snoop_by);
      SEND_TO_Q("\r\n", t->snoop_by);
    }
    if ((t->character) && (PLR_FLAGGED(t->character, PLR_SNOOP))) {
      sprintf(snoop_log, "%sLOG %s", GET_NAME(t->character), tmp);
      log(snoop_log);
    }
    failed_subst = 0;

    if (*tmp == '!')
      strcpy(tmp, t->last_input);
    else if (*tmp == '^') {
      if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
	strcpy(t->last_input, tmp);
    } else
      strcpy(t->last_input, tmp);

    if (!failed_subst)
      write_to_q(tmp, &t->input, 0);

    /* find the end of this line */
    while (ISNEWL(*nl_pos))
      nl_pos++;

    /* see if there's another newline in the input buffer */
    read_point = ptr = nl_pos;
    for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;
  }

  /* now move the rest of the buffer up to the beginning for the next pass */
  write_point = t->inbuf;
  while (*read_point)
    *(write_point++) = *(read_point++);
  *write_point = '\0';

  return 1;
}



/*
 * perform substitution for the '^..^' csh-esque syntax
 * orig is the orig string (i.e. the one being modified.
 * subst contains the substition string, i.e. "^telm^tell"
 */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
  char new[MAX_INPUT_LENGTH + 5];

  char *first, *second, *strpos;

  /*
   * first is the position of the beginning of the first string (the one
   * to be replaced
   */
  first = subst + 1;

  /* now find the second '^' */
  if (!(second = strchr(first, '^'))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* terminate "first" at the position of the '^' and make 'second' point
   * to the beginning of the second string */
  *(second++) = '\0';

  /* now, see if the contents of the first string appear in the original */
  if (!(strpos = strstr(orig, first))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* now, we construct the new string for output. */

  /* first, everything in the original, up to the string to be replaced */
  strncpy(new, orig, (strpos - orig));
  new[(strpos - orig)] = '\0';

  /* now, the replacement string */
  strncat(new, second, (MAX_INPUT_LENGTH - strlen(new) - 1));

  /* now, if there's anything left in the original after the string to
   * replaced, copy that too. */
  if (((strpos - orig) + strlen(first)) < strlen(orig))
    strncat(new, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(new) - 1));

  /* terminate the string in case of an overflow from strncat */
  new[MAX_INPUT_LENGTH - 1] = '\0';
  strcpy(subst, new);

  return 0;
}



void close_socket(struct descriptor_data *d)
{
  char buf[128];
  char fname[MAX_STRING_LENGTH];
  struct descriptor_data *temp;
  struct mail_list *tmpmail;
  long target_idnum = -1;
  int i;

  ACMD(do_vcdepart);

  CLOSE_SOCKET(d->descriptor);
  flush_queues(d);

  /* Forget snooping */
  if (d->snooping) {
    sprintf(buf, "(GC) %s stopped snooping %s.", (d->character&&GET_NAME(d->character)?GET_NAME(d->character):""), (d->snooping->character&&GET_NAME(d->snooping->character)?GET_NAME(d->snooping->character):""));
    log(buf);
    d->snooping->snoop_by = NULL;
  }

  if (d->snoop_by) {
    sprintf(buf, "(GC) %s stopped snooping %s.", (d->snoop_by->character&&GET_NAME(d->snoop_by->character)?GET_NAME(d->snoop_by->character):""), (d->character&&GET_NAME(d->character)?GET_NAME(d->character):""));
    log(buf);
    SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
    d->snoop_by->snooping = NULL;
  }

  if (d->character) {
    if(GET_NAME(d->character))
      strcpy(buf, GET_NAME(d->character));
    else
      strcpy(buf, "no");
    buf[5]=0;
    if(!str_cmp(buf, "guest")) {
      sprintf(buf, "Losing %s.", GET_NAME(d->character));
      mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
      SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
      save_char(d->character, NOWHERE);
      Crash_delete_file(GET_NAME(d->character));
      alias_delete(GET_NAME(d->character));
      if (get_filename(GET_NAME(d->character), fname, SAVEMAIL_FILE))
        unlink(fname);
    }
    target_idnum = GET_IDNUM(d->character);
    if (d->connected == CON_PLAYING) {
      if (d->str)
        if(!(*d->str))
          (*d->str)=str_dup("");
      d->character->desc = NULL;
      if(GET_NAME(d->character))
        strcpy(buf, GET_NAME(d->character));
      else
        strcpy(buf, "no");
      buf[5]=0;
      if(!str_cmp(buf, "guest")) {
        act("Losing $n.", TRUE, d->character, 0, 0, TO_ROOM);
        do_vcdepart(d->character, "", 0, 0);
        extract_char(d->character);
      }
      else {
        save_char(d->character, NOWHERE);
        act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
        sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
        mudlog(buf, NRM, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
      }
    } else {
      sprintf(buf, "Losing player: %s.",
	      GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
      mudlog(buf, CMP, MAX(LVL_HERO, GET_INVIS_LEV(d->character)), TRUE);
      for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(d->character, i))
          obj_to_char(unequip_char(d->character, i), d->character);
      while(d->character->carrying)
        extract_obj(d->character->carrying);
      free_char(d->character);
    }
  } else
    mudlog("Losing descriptor without char.", CMP, LVL_HERO, TRUE);

  /* JE 2/22/95 -- part of my unending quest to make switch stable */
  if (d->original && d->original->desc)
    d->original->desc = NULL;

  REMOVE_FROM_LIST(d, descriptor_list, next);

  if (d->showstr_head)
    free(d->showstr_head);
  if (d->showstr_count)
    free(d->showstr_vector);

  while(d->mail_to) {
    tmpmail=d->mail_to;
    d->mail_to=tmpmail->next;
    free(tmpmail);
  }

  free(d);
}



void check_idle_passwords(void)
{
  struct descriptor_data *d, *next_d;

  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME)
      continue;
    if (!d->idle_tics) {
      d->idle_tics++;
      continue;
    } else {
      echo_on(d);
      SEND_TO_Q("\r\nTimed out... goodbye.\r\n", d);
      STATE(d) = CON_CLOSE;
    }
  }
}



/*
 * I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.)
 */

#ifdef CIRCLE_WINDOWS

void nonblock(socket_t s)
{
  long val;

  val = 1;
  ioctlsocket(s, FIONBIO, &val);
}

#else

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(socket_t s)
{
  int flags;

  flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
    perror("Fatal error executing nonblock (comm.c)");
    exit(1);
  }
}


/* ******************************************************************
*  signal-handling functions (formerly signals.c)                   *
****************************************************************** */


RETSIGTYPE checkpointing()
{
  if (!tics) {
    log("SYSERR: CHECKPOINT shutdown: tics not updated");
    abort();
  } else
    tics = 0;
}


RETSIGTYPE reread_wizlists()
{
  void reboot_wizlists(void);

  mudlog("Signal received - rereading wizlists.", CMP, LVL_IMMORT, TRUE);
  reboot_wizlists();
}


RETSIGTYPE unrestrict_game()
{
  extern struct ban_list_element *ban_list;
  extern int num_invalid;

  mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
	 BRF, LVL_IMMORT, TRUE);
  ban_list = NULL;
  restrict_mud = 0;
  num_invalid = 0;
}


RETSIGTYPE hupsig()
{
  log("Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
  exit(0);			/* perhaps something more elegant should
				 * substituted */
}


/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 *
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 *
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
 */

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc *my_signal(int signo, sigfunc * func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif

  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
#endif				/* NeXT */


void signal_setup(void)
{
#ifndef CIRCLE_OS2
  struct itimerval itime;
  struct timeval interval;
#endif

  /* user signal 1: reread wizlists.  Used by autowiz system. */
  my_signal(SIGUSR1, reread_wizlists);

  /*
   * user signal 2: unrestrict game.  Used for emergencies if you lock
   * yourself out of the MUD somehow.  (Duh...)
   */
  my_signal(SIGUSR2, unrestrict_game);

  /*
   * set up the deadlock-protection so that the MUD aborts itself if it gets
   * caught in an infinite loop for more than 3 minutes.  Doesn't work with
   * OS/2.
   */
#ifndef CIRCLE_OS2
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  my_signal(SIGVTALRM, checkpointing);
#endif

  /* just to be on the safe side: */
  my_signal(SIGHUP, hupsig);
  my_signal(SIGINT, hupsig);
  my_signal(SIGTERM, hupsig);
  my_signal(SIGPIPE, SIG_IGN);
  my_signal(SIGALRM, SIG_IGN);

#ifdef CIRCLE_OS2
#if defined(SIGABRT)
  my_signal(SIGABRT, hupsig);
#endif
#if defined(SIGFPE)
  my_signal(SIGFPE, hupsig);
#endif
#if defined(SIGILL)
  my_signal(SIGILL, hupsig);
#endif
#if defined(SIGSEGV)
  my_signal(SIGSEGV, hupsig);
#endif
#endif				/* CIRCLE_OS2 */

}

#endif				/* CIRCLE_WINDOWS */


/* ****************************************************************
*       Public routines for system-to-player-communication        *
**************************************************************** */

void send_to_char(char *messg, struct char_data *ch)
{
  if (ch->desc && messg)
    SEND_TO_Q(messg, ch->desc);
}


void send_to_all(char *messg)
{
  struct descriptor_data *i;

  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	SEND_TO_Q(messg, i);
}


void send_to_outdoor(char *messg)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && AWAKE(i->character) &&
	OUTSIDE(i->character) && (!PLR_FLAGGED(i->character, PLR_WRITING | PLR_MAILING | PLR_BUILDING)))
      SEND_TO_Q(messg, i);
}


void send_to_weather(char *messg, int sect)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && AWAKE(i->character) &&
	OUTSIDE(i->character) && (!PLR_FLAGGED(i->character, PLR_WRITING | PLR_MAILING | PLR_BUILDING)) &&
        (i->character->in_room!=NOWHERE) && (world[i->character->in_room].sector_type == sect))
      SEND_TO_Q(messg, i);
}


void send_to_room(char *messg, int room)
{
  struct char_data *i;

  if (messg)
    for (i = world[room].people; i; i = i->next_in_room)
      if (i->desc)
	SEND_TO_Q(messg, i->desc);
}



char *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) i = ACTNULL; else i = (expression);


/* higher-level communication: the act() function */
void perform_act(char *orig, struct char_data *ch, struct obj_data *obj,
		 void *vict_obj, struct char_data *to)
{
  register char *i = NULL, *buf;
  static char lbuf[MAX_STRING_LENGTH];

  buf = lbuf;

  for (;;) {
    if (*orig == '$') {
      switch (*(++orig)) {
      case 'n':
	i = PERS(ch, to);
	break;
      case 'N':
	CHECK_NULL(vict_obj, PERS((struct char_data *) vict_obj, to));
	break;
      case 'm':
	i = HMHR(ch);
	break;
      case 'M':
	CHECK_NULL(vict_obj, HMHR((struct char_data *) vict_obj));
	break;
      case 's':
	i = HSHR(ch);
	break;
      case 'S':
	CHECK_NULL(vict_obj, HSHR((struct char_data *) vict_obj));
	break;
      case 'e':
	i = HSSH(ch);
	break;
      case 'E':
	CHECK_NULL(vict_obj, HSSH((struct char_data *) vict_obj));
	break;
      case 'o':
	CHECK_NULL(obj, OBJN(obj, to));
	break;
      case 'O':
	CHECK_NULL(vict_obj, OBJN((struct obj_data *) vict_obj, to));
	break;
      case 'p':
	CHECK_NULL(obj, OBJS(obj, to));
	break;
      case 'P':
	CHECK_NULL(vict_obj, OBJS((struct obj_data *) vict_obj, to));
	break;
      case 'a':
	CHECK_NULL(obj, SANA(obj));
	break;
      case 'A':
	CHECK_NULL(vict_obj, SANA((struct obj_data *) vict_obj));
	break;
      case 'T':
	CHECK_NULL(vict_obj, (char *) vict_obj);
	break;
      case 'F':
	CHECK_NULL(vict_obj, fname((char *) vict_obj));
	break;
      case '$':
	i = "$";
	break;
      default:
	log("SYSERR: Illegal $-code to act():");
	strcpy(buf1, "SYSERR: ");
	strcat(buf1, orig);
	log(buf1);
	break;
      }
      while ((*buf = *(i++)))
	buf++;
      orig++;
    } else if (!(*(buf++) = *(orig++)))
      break;
  }
  *(--buf) = '\r';
  *(++buf) = '\n';
  *(++buf) = '\0';
  SEND_TO_Q(CAP(lbuf), to->desc);
  if(MOBTrigger)
    mprog_act_trigger(lbuf, to, ch, obj, vict_obj);}
#define SENDOK(ch) ((ch)->desc && (AWAKE(ch) || sleep) && \
		    !PLR_FLAGGED((ch), PLR_WRITING | PLR_MAILING | PLR_BUILDING))
void act(char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type)
{
  struct char_data *to;
  static int sleep;
  if (!str || !*str) {
    MOBTrigger=TRUE;
    return;
  }
  /*
   * Warning: the following TO_SLEEP code is a hack.
   * 
   * I wanted to be able to tell act to deliver a message regardless of sleep
   * without adding an additional argument.  TO_SLEEP is 128 (a single bit
   * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
   * command.  It's not legal to combine TO_x's with each other otherwise.
   */
  /* check if TO_SLEEP is there, and remove it if it is. */
  if ((sleep = (type & TO_SLEEP)))
    type &= ~TO_SLEEP;
  if (type == TO_CHAR) {
    if (ch && SENDOK(ch))
      perform_act(str, ch, obj, vict_obj, ch);
    MOBTrigger=TRUE;
    return;
  }
  if (type == TO_VICT) {
    if ((to = (struct char_data *) vict_obj) && SENDOK(to))
      perform_act(str, ch, obj, vict_obj, to);
    MOBTrigger=TRUE;
    return;
  }
  if (type == TO_OBJ) {
    if ((to = (struct char_data *) obj) && SENDOK(to))
      perform_act(str, ch, obj, vict_obj, to);
    MOBTrigger=TRUE;
    return;
  }
  /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */
  if (ch && ch->in_room != NOWHERE)
    to = world[ch->in_room].people;
  else if (obj && obj->in_room != NOWHERE)
    to = world[obj->in_room].people;
  else {
    MOBTrigger=TRUE;
    sprintf(buf, "SYSERR: no valid target to act(\"%s\")!", str);
    log(buf);
    return;
  }
  for (; to; to = to->next_in_room)
    if (SENDOK(to) && !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
	(to != ch) && (type == TO_ROOM || (to != vict_obj)))
      perform_act(str, ch, obj, vict_obj, to);
  MOBTrigger=TRUE;
}
