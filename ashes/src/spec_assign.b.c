/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
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
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern struct room_data *world;
extern int top_of_world;
extern int mini_mud;
extern struct char_data *mob_proto;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

SPECIAL(postmaster);
SPECIAL(cityguard);
SPECIAL(guild);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(undead);
SPECIAL(cleric);
SPECIAL(breath_any);
SPECIAL(breath_fire);
SPECIAL(breath_acid);
SPECIAL(breath_frost);
SPECIAL(breath_gas);
SPECIAL(breath_lightning);
SPECIAL(spec_bash);
SPECIAL(spec_berserk);
SPECIAL(spec_kick);
SPECIAL(spec_warrior);

int(*spec_proc_table[MAX_SPECPROCS])(struct char_data *, void *, int, char *) =
{
    NULL,
    cityguard,
    postmaster,
    guild,
    fido,
    janitor,
    snake,
    thief,
    magic_user,
    undead,
    cleric,
    breath_any,
    breath_fire,
    breath_acid,
    breath_frost,
    breath_gas,
    breath_lightning,
    spec_bash,
    spec_berserk,
    spec_kick,
    spec_warrior,
    NULL
};

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  if (real_mobile(mob) >= 0) {
    mob_index[real_mobile(mob)].func = fname;
    SET_BIT(MOB_FLAGS(mob_proto+real_mobile(mob)), MOB_SPEC);
  }
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
	    mob);
    log(buf);
  }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
	    obj);
    log(buf);
  }
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
	    room);
    log(buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(guild_guard);
  SPECIAL(puff);
  SPECIAL(mayor);
  SPECIAL(retired_guildmaster);
  SPECIAL(statistician);
  SPECIAL(ali_rent_a_harem);
  SPECIAL(hakico_discount_fertilizer);
  SPECIAL(agraba_rajah);
  SPECIAL(agraba_iago);
  SPECIAL(agraba_abu);
  SPECIAL(agraba_aladdin);
  SPECIAL(agraba_jasmine);
  SPECIAL(agraba_jafar);
  SPECIAL(repair_shop);
  SPECIAL(well_of_mystery);
  SPECIAL(quest_shop);
  SPECIAL(mage_trainer);
  SPECIAL(cleric_trainer);
  SPECIAL(thief_trainer);
  SPECIAL(warrior_trainer);
  SPECIAL(paladin_trainer);
  SPECIAL(ranger_trainer);
  SPECIAL(antipaladin_trainer);
  SPECIAL(monk_trainer);
  SPECIAL(psionicist_trainer);
  SPECIAL(reroll);
  SPECIAL(teddy);
  SPECIAL(kender);
  SPECIAL(tandar);
  SPECIAL(crysania);
  SPECIAL(caramon);
  SPECIAL(tanis);
  SPECIAL(dragonlance_dragon);
  SPECIAL(tasslehoff);
  SPECIAL(chaos);
  SPECIAL(follow);
  SPECIAL(recharge_shop);
  SPECIAL(tour);

  ASSIGNMOB(1, puff);
  ASSIGNMOB(4, tour);

  /* Diablo
  ASSIGNMOB(2501, follow);
  ASSIGNMOB(2502, follow);
  ASSIGNMOB(2503, follow);
  ASSIGNMOB(2504, follow);
  ASSIGNMOB(2900, repair_shop);
  ASSIGNMOB(2902, recharge_shop);
*/

  ASSIGNMOB(3059, cityguard);
  ASSIGNMOB(3060, cityguard);
  ASSIGNMOB(3067, cityguard);
  ASSIGNMOB(3061, janitor);
  ASSIGNMOB(3062, fido);
  ASSIGNMOB(3066, fido);
  ASSIGNMOB(3068, janitor);
  ASSIGNMOB(3072, statistician);
  ASSIGNMOB(3005, receptionist);
  ASSIGNMOB(3095, cryogenicist);
  ASSIGNMOB(3010, postmaster);
  ASSIGNMOB(3073, repair_shop);
  ASSIGNMOB(3075, well_of_mystery);
  ASSIGNMOB(3076, quest_shop);

  ASSIGNMOB(3020, guild);
  ASSIGNMOB(3021, guild);
  ASSIGNMOB(3022, guild);
  ASSIGNMOB(3023, guild);
  ASSIGNMOB(3071, guild);
  ASSIGNMOB(3033, guild);  

  ASSIGNMOB(3024, guild_guard);
  ASSIGNMOB(3025, guild_guard);
  ASSIGNMOB(3026, guild_guard);
  ASSIGNMOB(3027, guild_guard);
  ASSIGNMOB(3070, guild_guard);
  ASSIGNMOB(3031, guild_guard);
  ASSIGNMOB(3032, guild_guard);
  ASSIGNMOB(3047, guild_guard);
  ASSIGNMOB(3099, guild_guard);
  ASSIGNMOB(3101, guild_guard);

  ASSIGNMOB(3007, reroll);

  /* multiclass trainers */
  ASSIGNMOB(3051, monk_trainer);
  ASSIGNMOB(3052, mage_trainer);
  ASSIGNMOB(3053, warrior_trainer);
  ASSIGNMOB(3054, cleric_trainer);
  ASSIGNMOB(3055, ranger_trainer);
  ASSIGNMOB(3056, thief_trainer);
  ASSIGNMOB(3057, paladin_trainer);
  ASSIGNMOB(3058, antipaladin_trainer);
  ASSIGNMOB(3084, psionicist_trainer);

  ASSIGNMOB(3143, mayor);

  /* Rome */
  ASSIGNMOB(12018, cityguard);
  ASSIGNMOB(12021, cityguard);
  ASSIGNMOB(12009, magic_user);
  ASSIGNMOB(12025, magic_user);

  ASSIGNMOB(12020, magic_user);
  ASSIGNMOB(12025, magic_user);
  ASSIGNMOB(12030, magic_user);
  ASSIGNMOB(12031, magic_user);
  ASSIGNMOB(12032, magic_user);
  ASSIGNMOB(12033, magic_user);

/*  ASSIGNMOB(6097, guild_guard); */

  /* Dragonlance
  ASSIGNMOB(17908, kender);
  ASSIGNMOB(17929, crysania);
  ASSIGNMOB(17930, tandar);
  ASSIGNMOB(17935, tasslehoff);
  ASSIGNMOB(17947, tanis);
  ASSIGNMOB(17948, caramon);
  ASSIGNMOB(17959, guild_guard);
  ASSIGNMOB(17960, guild_guard);
  ASSIGNMOB(17961, guild_guard);
  ASSIGNMOB(17964, guild_guard);
  ASSIGNMOB(17974, dragonlance_dragon);
  ASSIGNMOB(17975, dragonlance_dragon);
  ASSIGNMOB(17976, dragonlance_dragon);
  ASSIGNMOB(17977, dragonlance_dragon);
  ASSIGNMOB(17978, dragonlance_dragon);
  ASSIGNMOB(17979, dragonlance_dragon);
  ASSIGNMOB(17980, dragonlance_dragon);
  ASSIGNMOB(17981, dragonlance_dragon);
  ASSIGNMOB(17982, dragonlance_dragon);
  ASSIGNMOB(17983, dragonlance_dragon);
  ASSIGNMOB(17984, chaos);
*/
}



/* assign special procedures to objects */
void assign_objects(void)
{
  SPECIAL(bank);
  SPECIAL(gen_board);
  SPECIAL(evaporating_newbie_eq);
  SPECIAL(crysania_medallion);
  SPECIAL(pure_chaos);
  SPECIAL(staff_of_magius);
  SPECIAL(kitiara_sword);
  SPECIAL(kitiara_stick);
  SPECIAL(rabbitslayer);
  SPECIAL(evaporating_potions);
  SPECIAL(mana_potions_low);
  SPECIAL(mana_potions_high);
  SPECIAL(diablo_recall);

  /* Diablo
  ASSIGNOBJ(2914, evaporating_potions);
  ASSIGNOBJ(2915, evaporating_potions);
  ASSIGNOBJ(2916, mana_potions_low);
  ASSIGNOBJ(2917, mana_potions_high);
  ASSIGNOBJ(2918, evaporating_potions);
  ASSIGNOBJ(2919, diablo_recall);
  ASSIGNOBJ(2937, mana_potions_low);
  ASSIGNOBJ(2938, mana_potions_high);
*/

  ASSIGNOBJ(3006, gen_board);	/*equipment board */
  ASSIGNOBJ(3090, gen_board);	/* building board */
  ASSIGNOBJ(3091, gen_board);	/* coding board */
  ASSIGNOBJ(3093, gen_board);	/* STAFF+ board */
  ASSIGNOBJ(3094, gen_board);	/* social board */
  ASSIGNOBJ(3095, gen_board);	/* social board */
  ASSIGNOBJ(3096, gen_board);	/* social board */
  ASSIGNOBJ(3097, gen_board);	/* freeze board */
  ASSIGNOBJ(3098, gen_board);	/* immortal board */
  ASSIGNOBJ(3099, gen_board);	/* mortal board */

  ASSIGNOBJ(3034, bank);	/* atm */

  /* Dragonlance
  ASSIGNOBJ(17920, crysania_medallion);
  ASSIGNOBJ(17986, pure_chaos);
  ASSIGNOBJ(17989, kitiara_sword);
  ASSIGNOBJ(17990, staff_of_magius);
  ASSIGNOBJ(17991, kitiara_stick);
  ASSIGNOBJ(17992, rabbitslayer);
*/
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  extern int dts_are_dumps;
  int i;

  SPECIAL(dump);
  SPECIAL(falling_down);
  SPECIAL(lag_trap);
  SPECIAL(pet_shops);
  SPECIAL(poppy_field);
  SPECIAL(push_altar);
  SPECIAL(shuttle_station);
  SPECIAL(raistlin_ghost);
  SPECIAL(waywreth_forest);
  SPECIAL(wight_and_beard);

  ASSIGNROOM(0, dump);

  ASSIGNROOM(3030, dump);
  ASSIGNROOM(3031, pet_shops);

  /* Dragonlance
  ASSIGNROOM(18016, wight_and_beard);
  ASSIGNROOM(18057, raistlin_ghost);
  ASSIGNROOM(18190, waywreth_forest);
  ASSIGNROOM(18192, waywreth_forest);
  ASSIGNROOM(18193, waywreth_forest);
  ASSIGNROOM(18195, waywreth_forest);
  ASSIGNROOM(18196, waywreth_forest);
  ASSIGNROOM(18198, waywreth_forest);
*/

  if (dts_are_dumps)
    for (i = 0; i <= top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	world[i].func = dump;
}
