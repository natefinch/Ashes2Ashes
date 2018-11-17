/* ************************************************************************
*   File: olc.h                                    Part of Ashes to Ashes *
*  Usage: defines for OLC and lists to specify what can be modified       *
*                                                                         *
*  All rights reserved.                                                   *
*                                                                         *
*  Copyright (C) 1997 by Jesse Sterr                                      *
*  Ashes to Ashes is based on CircleMUD, Copyright (C) 1993, 1994.        *
************************************************************************ */


/* OLC modes */
#define OLC_MEDIT	1
#define OLC_OEDIT	2
#define OLC_REDIT	3
#define OLC_IEDIT	4
#define OLC_PEDIT	5
#define OLC_ZEDIT	6


#define OLC_MENU_NUM 	-10000

/* zone command data structure */
struct zcmd_data {
  char command;
  int arg1;
  int arg2;
  int arg3;
  int arg4;
  int line;
  int prob;
  struct zcmd_data *prev;
  struct zcmd_data *next;
  struct zcmd_data *if_next;
  struct zcmd_data *else_next;
};


/* Lengths for strings made in olc */
#define OLC_ALIAS_LENGTH	80
#define OLC_SHORT_LENGTH	80
#define OLC_LONG_LENGTH		128
#define OLC_DESCRIPTION_LENGTH	1024


#ifdef __OLC_C__

const int olc_classes[] = {
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_sex[] = {
  1,
  1,
  1
};

const int olc_size[] = {
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_position[] = {
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  0,
  1,
  0
};

const  int olc_attack[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_spec[] =
{
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_mob_bits[] = {
  0,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_mobaff_bits[] =
{
  1,
  1,
  0,
  1,
  0,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0
};

const int olc_damtype[] = {
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_item_types[] = {
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_spells[] =
{
  0,

  /* SPELLS */
  1,			/* 1 */
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,		/* 10 */
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,		/* 20 */
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,		/* 30 */
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,			/* 40 */
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,		/* 50 */
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,		/* 60 */
  1,
  1,
  1,
  1,
  1,		/* 65 */
  1,
  1,
  1,
  1,
  1,		/* 70 */
  1,
  1,
  1,
  1,
  1,			/* 75 */
  0,
  1,
  0,
  1,
  0,			/* 80 */
  1,
  1,
  1,
  0,
  0,			/* 85 */
  0, 0, 0, 0, 0,	/* 90 */
  0, 0, 0, 0, 0,	/* 95 */
  0, 0, 0, 0, 0,	/* 100 */
  0, 0, 0, 0, 0,	/* 105 */
  0, 0, 0, 0, 0,	/* 110 */
  0, 0, 0, 0, 0,	/* 115 */
  0, 0, 0, 0, 0,	/* 120 */
  0, 0, 0, 0, 0,	/* 125 */
  0, 0, 0, 0, 0,	/* 130 */
  0, 0, 0, 0, 0,	/* 135 */
  0, 0, 0, 0, 0,	/* 140 */
  0, 0, 0, 0, 0,	/* 145 */
  0, 0, 0, 0, 0,	/* 150 */

  /* SKILLS */
  0,			/* 151 */
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,			/* 160 */
  0,
  0,
  0,
  0,
  0,			/* 165 */
  0,
  0,
  0,
  0,
  0,			/* 170 */
  0,
  0,
  0,
  0,
  0,			/* 175 */
  0,
  0,
  0,
  0,
  0,			/* 180 */
  0,
  0,
  0,
  0,
  0,			/* 185 */
  0, 0, 0, 0, 0,	/* 190 */
  0, 0, 0, 0, 0,	/* 195 */
  0, 0, 0, 0, 0,	/* 200 */
  0, 0, 0, 0, 0,	/* 205 */
  0, 0, 0, 0, 0,	/* 210 */
  0, 0, 0, 0, 0,	/* 215 */
  0, 0, 0, 0, 0,	/* 220 */
  0, 0, 0, 0, 0,	/* 225 */
  0, 0, 0, 0, 0,	/* 230 */
  0, 0, 0, 0, 0,	/* 235 */
  0, 0, 0, 0, 0,	/* 240 */
  0, 0, 0, 0, 0,	/* 245 */
  0, 0, 0, 0, 0,	/* 250 */

  /* OBJECT SPELLS AND NPC SPELLS/SKILLS */
  0,		/* 251 */
  0,
  0,
  0,
  0
};

const int olc_weapons[] = {
  1,
  0,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  0,
  1
};

const int olc_container_lock[] = {
  1,
  1,
  1,
  1
};

const int olc_drinks[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_wear[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_extra[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  1,
  1,
  1,
  1
};

const int olc_iedit_extra[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1
};

const int olc_objaff_bits[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  0,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  0,
  0,
  0
};

const int olc_apply[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_sector[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_room[] = {
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  0,
  1,
  0,
  1
};

const int olc_pc_classes[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_aff[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  0,
  0,
  0
};

const int olc_prf[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

const int olc_plr[] = {
  1,
  1,
  1,
  0,
  1,
  1,
  0,
  1,
  1,
  1,
  0,
  0,
  1,
  1,
  1,
  0,
  1
};

const int olc_grnt[] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

#endif
