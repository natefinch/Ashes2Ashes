/* ************************************************************************
*                                                                         *
*  Copyright (C) 1997 by Jesse Sterr                                      *
*                                                                         *
************************************************************************* */
/* Things needed for classes */

/* Class flags */
#define MU_F	(1<<CLASS_MAGIC_USER)
#define CL_F	(1<<CLASS_CLERIC)
#define TH_F	(1<<CLASS_THIEF)
#define WA_F	(1<<CLASS_WARRIOR)
#define PA_F	(1<<CLASS_PALADIN)
#define RA_F	(1<<CLASS_RANGER)
#define AP_F	(1<<CLASS_ANTIPALADIN)
#define MK_F	(1<<CLASS_MONK)
#define PS_F	(1<<CLASS_PSIONICIST)

#define SPELL_LEARNED_LEVEL	95
#define SKILL_LEARNED_LEVEL	85

#define SPELL	0
#define SKILL	1

/* Index values for level_params */
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

int class_get_thac0(struct char_data *ch);
int class_get_save(struct char_data *ch, int save_type);

/* psi_stats.abil values */
#define PSI_STR 1
#define PSI_INT 2
#define PSI_WIS 3
#define PSI_DEX 4
#define PSI_CON 5
#define PSI_CHA 6

struct psi_stat_chart {
  int abil;
  int mod;
  int hp;
  int move;
  int mana;
  int (*func) (struct char_data *ch, char *argument, int psi_power, int power_score, int roll);
};

#ifndef __CLASS_C_
extern char *class_abbrevs[];
extern int level_params[NUM_LEVEL_PARAMS][NUM_CLASSES];
extern long exp_table[NUM_CLASSES][LVL_IMPL + 1];
extern int MULTICLASS_SKILL_LEVELS[NUM_CLASSES+1];
#endif
