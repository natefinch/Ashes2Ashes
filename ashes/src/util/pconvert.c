#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

#define PRAC_BONUS	0
#define PRAC_MUL	1
#define PRAC_DIV	2
#define MIN_PRAC	3
#define MAX_PRAC	4
#define XP_2		5
#define XP_MAX		6
#define XP_CURVE	7
#define THAC0_NUM	8
#define THAC0_DEN	9
#define MIN_HP_LEVEL	10
#define MAX_HP_LEVEL	11
#define MIN_MANA_LEVEL	12
#define MAX_MANA_LEVEL	13
#define MIN_MOVE_LEVEL	14
#define MAX_MOVE_LEVEL	15
#define HP_REGEN_BASE	16
#define HP_REGEN_DIV	17
#define MANA_REGEN_MUL	18
#define MANA_REGEN_BASE	19
#define MANA_REGEN_DIV	20
#define MOVE_REGEN_BASE	21
#define	MOVE_REGEN_DIV	22
#define SAVE_PARA_MIN	23
#define SAVE_PARA_MAX	24
#define SAVE_ROD_MIN	25
#define SAVE_ROD_MAX	26
#define SAVE_PETRI_MIN	27
#define SAVE_PETRI_MAX	28
#define SAVE_BREATH_MIN	29
#define SAVE_BREATH_MAX	30
#define SAVE_SPELL_MIN	31
#define SAVE_SPELL_MAX	32

#define NUM_LEVEL_PARAMS 33


int top_idnum=0;

long exp_table[NUM_CLASSES][LVL_IMPL + 1];

int level_params[NUM_LEVEL_PARAMS][NUM_CLASSES] = {
/* MAG    CLE    THE    WAR    PAL    RAN    APAL  MONK */
  {20,    20,    45,    50,    30,    40,    30,   40},   /* prac bonus (added to int bonus) */
  {5,     5,     5,     1,     5,     5,     5,    5},    /* prac multiplier */
  {14,    14,    19,    4,     16,    18,    16,   18},   /* prac divisor (when prac, get mul*(prac_bonus+int bonus)/div percent) */
  {1,     1,     0,     0,     1,     0,     1,    0},    /* min prac per level */
  {3,     3,     1,     1,     2,     1,     2,    1},    /* max prac per level */
  {1200,  1200,  1000,  1600,  1600,  1600,  1600, 2400}, /* level 2 xp */
  {104000000, 96000000, 93000000, 118000000, 164000000, 164000000, 164000000, 195000000}, /* level 101 xp */
  {28,    29,    27,    30,    26,    26,    26,   29},   /* xp curve (the lower the number, the more xp is piled up at the end) */
  {2,     3,     4,     5,     5,     5,     5,    5},    /* thac0 numerator per level */
  {2,     2,     2,     2,     2,     2,     2,    2},    /* thac0 denominator per level */
  {2,     2,     3,     5,     4,     4,     4,    3},    /* min hp per level */
  {4,     5,     7,     8,     7,     7,     7,    5},    /* max hp per level */
  {1,     1,     0,     0,     0,     0,     0,    0},    /* min mana per level */
  {5,     5,     0,     0,     3,     0,     3,    0},    /* max mana per level */
  {0,     0,     0,     0,     0,     1,     0,    0},    /* min move per level */
  {1,     1,     1,     1,     1,     2,     1,    2},    /* max move per level */
  {-4,    -4,    0,     0,     0,     0,     0,    -2},   /* hp regen base */
  {200,   200,   10,    10,    14,    14,    14,   16},   /* hp regen level divisor */
  {2,     2,     1,     1,     1,     1,     1,    1},    /* mana regen class multiplier */
  {0,     0,     0,     0,     0,     0,     0,    0},    /* mana regen base */
  {10,    10,    200,   200,   14,    200,   14,   200},  /* mana regen level divisor */
  {0,     0,     0,     0,     0,     0,     0,    0},    /* move regen base */
  {200,   200,   200,   200,   200,   14,    200,  24},   /* move regen level divisor */
  {95,    95,    95,    95,    95,    95,    95,   95},   /* save para level 1 */
  {28,    7,     26,    12,    12,    12,    12,   19},   /* save para level 100 */
  {95,    95,    95,    95,    95,    95,    95,   95},   /* save rod level 1 */
  {9,     24,    13,    22,    22,    22,    22,   14},   /* save rod level 100 */
  {95,    95,    95,    95,    95,    95,    95,   95},   /* save petri level 1 */
  {17,    12,    21,    27,    27,    27,    27,   24},   /* save petri level 100 */
  {95,    95,    95,    95,    95,    95,    95,   95},   /* save breath level 1 */
  {23,    27,    31,    17,    17,    17,    17,   14},   /* save breath level 100 */
  {95,    95,    95,    95,    95,    95,    95,   95},   /* save spell level 1 */
  {12,    22,    17,    27,    27,    27,    27,   19}    /* save spell level 100 */
};


int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((random() % (to - from + 1)) + from);
}

char *str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}

void class_init_exp_tables(void)
{
  int class, i, j;
  double e10_c, ugf, xmax_x10;
  long xp;

  double exp(double);

  for(class=0; class < NUM_CLASSES; class++) {

    if(!level_params[XP_CURVE][class])
      continue;

    exp_table[class][0]=0;
    exp_table[class][1]=1;
    exp_table[class][2]=level_params[XP_2][class];
    for(i=3; i<=10; i++)
      exp_table[class][i]=2*exp_table[class][i-1];

    e10_c = exp(10.0 / (double) level_params[XP_CURVE][class]);
    ugf = exp((double) (LVL_HERO) / (double) level_params[XP_CURVE][class]) - e10_c;
    xmax_x10 = ((double) level_params[XP_MAX][class] - (double) exp_table[class][10]);

    for(i=11; i < LVL_HERO; i++) {
      xp = (xmax_x10 * (exp((double) i / (double) level_params[XP_CURVE][class]) - e10_c) / ugf) + exp_table[class][10];
      xp /= 1000;
      xp *= 1000;
      exp_table[class][i]=xp;
    }

    for(j=LVL_IMPL; j>=i; j--) {
      exp_table[class][j] = 250000000L-(10000000L*(LVL_IMPL-j));
    }
  }
}

void clear_char(struct char_data * ch)
{
  struct player_special_data *ps;
  int i;

  ps=ch->player_specials;
  memset((char *) ch, 0, sizeof(struct char_data));
  ch->player_specials=ps;
  ch->in_room = NOWHERE;
  GET_PFILEPOS(ch) = -1;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch) = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->char_specials.vc = -1;
  ch->char_specials.vci = -1;
  ch->char_specials.vcm = FALSE;
  ch->char_specials.vcs = FALSE;
  GET_AC(ch) = 100;		/* Basic Armor */
  ch->player_specials->saved.olc_min1=ch->player_specials->saved.olc_min2=0;
  ch->player_specials->saved.olc_max1=ch->player_specials->saved.olc_max2=0;
  ch->player.hometown = 1;
  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
  for (i = 0; i < MAX_TONGUE; i++)
    GET_TALK(ch, i) = 0;
  /* make favors for sex */
  if (ch->player.sex == SEX_MALE) {
    ch->player.weight = number(120, 180);
    ch->player.height = number(160, 200);
  } else {
    ch->player.weight = number(100, 165);
    ch->player.height = number(150, 190);
  }
  ch->player_specials->saved.new_mana = ch->points.max_mana = 0;
  ch->points.mana = GET_MAX_MANA(ch);
  ch->player_specials->saved.new_hit = ch->points.hit = GET_MAX_HIT(ch) = 0;
  ch->player_specials->saved.new_move = ch->points.max_move = 0;
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.armor = 100;
  GET_IDNUM(ch) = ++top_idnum;
  for (i = 1; i <= MAX_SKILLS; i++) {
    SET_SKILL(ch, i, 0)
  }
  ch->char_specials.saved.affected_by = 0;
  for (i = 0; i < 5; i++)
    GET_SAVE(ch, i) = 0;
  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = 24;
  GET_LOADROOM(ch) = NOWHERE;
  for(i=0; i<MAX_REMEMBER; i++)
    GET_REMEMBER(ch, i)=NOWHERE;
}

void do_start(struct char_data * ch)
{
  int i;

  for(i=0; i<32; i++) {
    GET_CLASS_LEVEL(ch, i)=0;
  }
  GET_CLASS_LEVEL(ch, (int)GET_CLASS(ch)) = GET_LEVEL(ch);
  GET_NUM_CLASSES(ch) = 1;
  GET_CLASS_BITVECTOR(ch) = (1 << (int)GET_CLASS(ch));
  GET_ALIGNMENT(ch) = 0;
  GET_EXP(ch) = exp_table[(int)GET_CLASS(ch)][GET_LEVEL(ch)]+1;
  ch->player_specials->saved.extra_pracs=GET_PRACTICES(ch)=GET_LEVEL(ch)*level_params[MAX_PRAC][(int)GET_CLASS(ch)];
  ch->player_specials->saved.rerolling=0;
  ch->player_specials->saved.inherent_ac_apply=0;
  ch->player_specials->saved.reroll_level=0;
  ch->player_specials->saved.old_hit = GET_MAX_HIT(ch);
  ch->player_specials->saved.old_mana = GET_MAX_MANA(ch);
  ch->player_specials->saved.old_move = GET_MAX_MOVE(ch);
  ch->char_specials.saved.affected_by=0;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    break;

  case CLASS_CLERIC:
    break;

  case CLASS_THIEF:
    break;

  case CLASS_WARRIOR:
    break;

  case CLASS_PALADIN:
    SET_BIT(ch->char_specials.saved.affected_by, AFF_DETECT_ALIGN);
    break;

  case CLASS_RANGER:
    break;

  case CLASS_ANTIPALADIN:
    SET_BIT(ch->char_specials.saved.affected_by, AFF_DETECT_ALIGN);
    break;

  case CLASS_MONK:
    break;
  }

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;
}

void char_to_store(struct char_data * ch, struct char_file_u * st)
{
  int i;

  for (i = 0; i < MAX_AFFECT; i++) {
    st->affected[i].type = 0;	/* Zero signifies not used */
    st->affected[i].duration = 0;
    st->affected[i].modifier = 0;
    st->affected[i].location = 0;
    st->affected[i].bitvector = 0;
    st->affected[i].next = 0;
  }

  ch->aff_abils = ch->real_abils;
  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);
  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = GET_CLASS(ch);
  st->level = GET_LEVEL(ch);
  st->abilities = ch->real_abils;
  st->points = ch->points;
  st->char_specials_saved = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;
  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;
  *st->title = '\0';
  *st->description = '\0';
  /* Save walks */
  if (WALKIN(ch))
  {
    strncpy(st->walkin, WALKIN(ch), MAX_WALK_LENGTH);
    st->walkin[MAX_WALK_LENGTH]=0;
  }
  else
    *st->walkin = '\0';
  if (WALKOUT(ch))
  {
    strncpy(st->walkout, WALKOUT(ch), MAX_WALK_LENGTH);
    st->walkout[MAX_WALK_LENGTH]=0;
  }
  else
    *st->walkout = '\0';
  /* Save poofs */
  if (POOFIN(ch))
  {
    strncpy(st->poofin, POOFIN(ch), MAX_WALK_LENGTH);
    st->poofin[MAX_WALK_LENGTH]=0;
  }
  else
    *st->poofin = '\0';
  if (POOFOUT(ch))
  {
    strncpy(st->poofout, POOFOUT(ch), MAX_WALK_LENGTH);
    st->poofout[MAX_WALK_LENGTH]=0;
  }
  else
    *st->poofout = '\0';
  st->prompt[0] = '*';
  st->prompt[1] = '\0';
}

void main(int argc, char *argv[])
{
  FILE *oldf, *newf;
  long plr_num;
  struct char_data ch;
  struct char_file_u st;
  unsigned char b1, b2, b3, b4;
  char temp[256];
  int i;

  if(argc != 3) {
    printf("Usage: %s <roi_file> <wod_pfile>", argv[0]);
    return;
  }

  if(!(oldf=fopen(argv[1], "rb"))) {
    printf("Error opening file %s", argv[1]);
    return;
  }

  if(!(newf=fopen(argv[2], "wb"))) {
    printf("Error opening file %s", argv[2]);
    return;
  }

  class_init_exp_tables();

  CREATE(ch.player_specials, struct player_special_data, 1);

  WALKIN(&ch)=str_dup("enters");
  WALKOUT(&ch)=str_dup("leaves");
  POOFIN(&ch)=str_dup("appears with a loud bang.");
  POOFOUT(&ch)=str_dup("vanishes in a bright flash.");
  ch.char_specials.prompt=str_dup("*");
  ch.player_specials->saved.wimp_level=0;
  ch.player_specials->saved.freeze_level=0;
  ch.player_specials->saved.invis_level=0;
  ch.player_specials->saved.pref=0;
  ch.player_specials->saved.inherent_ac_apply=0;
  ch.player_specials->saved.age_add=0;
  ch.player_specials->saved.num_rerolls=0;
  ch.player_specials->saved.grants=0;
  ch.player_specials->saved.reroll_level=0;

  for(plr_num=0; 1; plr_num++) {
    clear_char(&ch);

    fseek(oldf, 1944*plr_num, SEEK_SET);
    fread(&GET_SEX(&ch), 1, 1, oldf);
    fread(&GET_CLASS(&ch), 1, 1, oldf);
    fread(&GET_LEVEL(&ch), 1, 1, oldf);

    GET_CLASS(&ch)-=1;

    if((GET_LEVEL(&ch) > 50) || (GET_LEVEL(&ch) < 1))
      continue;
    GET_LEVEL(&ch) *= 2;

    fseek(oldf, (1944*plr_num)+339, SEEK_SET);
    fread(&b1, 1, 1, oldf);
    ch.real_abils.str=b1;
    fread(&b1, 1, 1, oldf);
    ch.real_abils.str_add=b1;
    fread(&b1, 1, 1, oldf);
    ch.real_abils.intel=b1;
    fread(&b1, 1, 1, oldf);
    ch.real_abils.wis=b1;
    fread(&b1, 1, 1, oldf);
    ch.real_abils.dex=b1;
    fread(&b1, 1, 1, oldf);
    ch.real_abils.con=b1;
    fread(&b1, 1, 1, oldf);
    ch.real_abils.cha=13;

    fseek(oldf, (1944*plr_num)+350, SEEK_SET);
    fread(&b1, 1, 1, oldf);
    fread(&b2, 1, 1, oldf);
    GET_MAX_MANA(&ch)=((int)b1*256)+b2;

    fseek(oldf, (1944*plr_num)+354, SEEK_SET);
    fread(&b1, 1, 1, oldf);
    fread(&b2, 1, 1, oldf);
    GET_MAX_HIT(&ch)=((int)b1*256)+b2;

    fseek(oldf, (1944*plr_num)+358, SEEK_SET);
    fread(&b1, 1, 1, oldf);
    fread(&b2, 1, 1, oldf);
    GET_MAX_MOVE(&ch)=((int)b1*256)+b2;

    fseek(oldf, (1944*plr_num)+364, SEEK_SET);
    fread(&b1, 1, 1, oldf);
    fread(&b2, 1, 1, oldf);
    fread(&b3, 1, 1, oldf);
    fread(&b4, 1, 1, oldf);
    GET_GOLD(&ch)=(b1*256L*256L*256L)+(b2*256L*256L)+(b3*256L)+b4;
    fread(&b1, 1, 1, oldf);
    fread(&b2, 1, 1, oldf);
    fread(&b3, 1, 1, oldf);
    fread(&b4, 1, 1, oldf);
    GET_BANK_GOLD(&ch)=(b1*256L*256L*256L)+(b2*256L*256L)+(b3*256L)+b4;

    do_start(&ch);

    char_to_store(&ch, &st);
    st.player_specials_saved.load_room = NOWHERE;

    fseek(oldf, (1944*plr_num)+1880, SEEK_SET);
    for(i=0; i<31; i++) {
      fread(&b1, 1, 1, oldf);
      st.host[i]=b1;
    }

    fseek(oldf, (1944*plr_num)+1911, SEEK_SET);
    for(i=0; i<20; i++) {
      fread(&b1, 1, 1, oldf);
      st.name[i]=b1;
    }

    fseek(oldf, (1944*plr_num)+1931, SEEK_SET);
    for(i=0; i<11; i++) {
      fread(&b1, 1, 1, oldf);
      st.pwd[i]=b1;
    }

    fwrite(&st, sizeof(struct char_file_u), 1, newf);

    fread(temp, 1, 255, oldf);
    if(feof(oldf))
      break;
  }

  fclose(newf);
  fclose(oldf);
  return;
}
