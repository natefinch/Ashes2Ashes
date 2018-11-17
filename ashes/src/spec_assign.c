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

  void assign_kings_castle(void);

  assign_kings_castle();

  ASSIGNMOB(1, puff);

  /* WAMPHRI */
  ASSIGNMOB(534, magic_user);
  ASSIGNMOB(538, magic_user);

  /* SHIRE */
  ASSIGNMOB(1100, magic_user);
  ASSIGNMOB(1131, receptionist);

  /* HIGH TOWER OF SORCERY */
  ASSIGNMOB(1300, undead);
  ASSIGNMOB(1301, cleric);
  ASSIGNMOB(1304, magic_user);
  ASSIGNMOB(1306, magic_user);
  ASSIGNMOB(1307, magic_user);
  ASSIGNMOB(1308, magic_user);
  ASSIGNMOB(1309, magic_user);
  ASSIGNMOB(1310, magic_user);
  ASSIGNMOB(1311, thief);
  ASSIGNMOB(1312, undead);
  ASSIGNMOB(1313, undead);
  ASSIGNMOB(1314, magic_user);
  ASSIGNMOB(1315, magic_user);
  ASSIGNMOB(1316, magic_user);
  ASSIGNMOB(1317, magic_user);
  ASSIGNMOB(1318, magic_user);
  ASSIGNMOB(1320, magic_user);
  ASSIGNMOB(1321, magic_user);
  ASSIGNMOB(1322, magic_user);
  ASSIGNMOB(1323, magic_user);
  ASSIGNMOB(1324, magic_user);
  ASSIGNMOB(1325, magic_user);
  ASSIGNMOB(1326, magic_user);
  ASSIGNMOB(1327, magic_user);
  ASSIGNMOB(1328, magic_user);
  ASSIGNMOB(1329, magic_user);
  ASSIGNMOB(1330, magic_user);
  ASSIGNMOB(1331, magic_user);
  ASSIGNMOB(1332, magic_user);
  ASSIGNMOB(1333, magic_user);
  ASSIGNMOB(1334, magic_user);
  ASSIGNMOB(1336, magic_user);
  ASSIGNMOB(1337, magic_user);
  ASSIGNMOB(1338, magic_user);
  ASSIGNMOB(1340, magic_user);
  ASSIGNMOB(1341, magic_user);
  ASSIGNMOB(1348, magic_user);
  ASSIGNMOB(1349, magic_user);
  ASSIGNMOB(1350, undead);
  ASSIGNMOB(1351, undead);
  ASSIGNMOB(1352, magic_user);
  ASSIGNMOB(1353, magic_user);
  ASSIGNMOB(1354, magic_user);
  ASSIGNMOB(1355, magic_user);
  ASSIGNMOB(1356, magic_user);
  ASSIGNMOB(1357, magic_user);
  ASSIGNMOB(1358, magic_user);
  ASSIGNMOB(1359, magic_user);
  ASSIGNMOB(1360, magic_user);
  ASSIGNMOB(1361, magic_user);
  ASSIGNMOB(1362, magic_user);
  ASSIGNMOB(1363, magic_user);
  ASSIGNMOB(1364, magic_user);
  ASSIGNMOB(1365, magic_user);

  /* God Museum */
  ASSIGNMOB(1504, teddy);

  /* DRACONIA */
  ASSIGNMOB(2200, breath_any);
  ASSIGNMOB(2203, magic_user);
  ASSIGNMOB(2204, cleric);
  ASSIGNMOB(2205, breath_any);
  ASSIGNMOB(2220, breath_any);
  ASSIGNMOB(2221, breath_fire);
  ASSIGNMOB(2222, breath_acid);
  ASSIGNMOB(2223, breath_frost);
  ASSIGNMOB(2225, breath_gas);
  ASSIGNMOB(2226, breath_gas);
  ASSIGNMOB(2227, thief);
  ASSIGNMOB(2240, undead);
  ASSIGNMOB(2241, breath_gas);
  ASSIGNMOB(2242, thief);
  ASSIGNMOB(2243, breath_any);

  /* Diablo */
  ASSIGNMOB(2501, follow);
  ASSIGNMOB(2502, follow);
  ASSIGNMOB(2503, follow);
  ASSIGNMOB(2504, follow);
  ASSIGNMOB(2900, repair_shop);
  ASSIGNMOB(2902, recharge_shop);

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

  /* MORIA */
  ASSIGNMOB(4000, snake);
  ASSIGNMOB(4001, snake);
  ASSIGNMOB(4053, snake);

  ASSIGNMOB(4103, thief);
  ASSIGNMOB(4100, magic_user);
  ASSIGNMOB(4102, snake);

  /* DROW and THALOS */
  ASSIGNMOB(5005, guild_guard);
  ASSIGNMOB(5014, magic_user);
  ASSIGNMOB(5010, magic_user);
  ASSIGNMOB(5004, magic_user);
  ASSIGNMOB(5103, magic_user);
  ASSIGNMOB(5104, magic_user);
  ASSIGNMOB(5107, magic_user);
  ASSIGNMOB(5108, magic_user);
  ASSIGNMOB(5192, statistician);
  ASSIGNMOB(5200, magic_user);
  ASSIGNMOB(5201, magic_user);

  ASSIGNMOB(6097, guild_guard);

  /* FOREST */
  ASSIGNMOB(6113, snake);

  ASSIGNMOB(6115, magic_user);
  ASSIGNMOB(6112, magic_user);
  ASSIGNMOB(6114, magic_user);
  ASSIGNMOB(6116, magic_user);

  /* DWARVEN KINGDOM */
  ASSIGNMOB(6502, magic_user);
  ASSIGNMOB(6516, magic_user);
  ASSIGNMOB(6509, magic_user);
  ASSIGNMOB(6500, cityguard);

  /* SEWERS */
  ASSIGNMOB(7006, snake);
  ASSIGNMOB(7009, magic_user);

  /* GALAXY */
  ASSIGNMOB(9301, thief);
  ASSIGNMOB(9313, breath_fire);
  ASSIGNMOB(9314, breath_fire);
  ASSIGNMOB(9315, cleric);
  ASSIGNMOB(9316, cleric);
  ASSIGNMOB(9317, cleric);
  ASSIGNMOB(9318, cleric);
  ASSIGNMOB(9319, cleric);
  ASSIGNMOB(9320, cleric);
  ASSIGNMOB(9321, cleric);
  ASSIGNMOB(9322, cleric);
  ASSIGNMOB(9323, cleric);
  ASSIGNMOB(9324, cleric);
  ASSIGNMOB(9325, cleric);
  ASSIGNMOB(9326, cleric);
  ASSIGNMOB(9327, cleric);
  ASSIGNMOB(9328, cleric);
  ASSIGNMOB(9329, cleric);
  ASSIGNMOB(9330, magic_user);
  ASSIGNMOB(9331, magic_user);
  ASSIGNMOB(9332, breath_acid);

  /* ROME */
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

  /*Astral Plane*/
  ASSIGNMOB(12202, magic_user);
  ASSIGNMOB(12205, thief);
  ASSIGNMOB(12210, magic_user);
  ASSIGNMOB(12211, magic_user);
  ASSIGNMOB(12212, magic_user);
  ASSIGNMOB(12213, magic_user);
  ASSIGNMOB(12213, undead);
  ASSIGNMOB(12214, undead);
  ASSIGNMOB(12215, undead);
  ASSIGNMOB(12217, magic_user);
  ASSIGNMOB(12218, breath_fire);
  ASSIGNMOB(12218, magic_user);
  ASSIGNMOB(12219, breath_fire);

  ASSIGNMOB(13120, retired_guildmaster);
  ASSIGNMOB(13121, retired_guildmaster);
  ASSIGNMOB(13122, retired_guildmaster);
  ASSIGNMOB(13123, retired_guildmaster);
  ASSIGNMOB(13124, retired_guildmaster);

  /* Land of Lag */
  ASSIGNMOB(17213, guild_guard);

  /* Dragonlance */
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

  /* NEW SPARTA */
  ASSIGNMOB(21068, guild_guard);
  ASSIGNMOB(21069, guild_guard);
  ASSIGNMOB(21070, guild_guard);
  ASSIGNMOB(21071, guild_guard);

  ASSIGNMOB(21072, guild);
  ASSIGNMOB(21073, guild);
  ASSIGNMOB(21074, guild);
  ASSIGNMOB(21075, guild);

  ASSIGNMOB(21084, janitor);
  ASSIGNMOB(21085, fido);
  ASSIGNMOB(21057, magic_user);
  ASSIGNMOB(21054, magic_user);
  ASSIGNMOB(21011, magic_user);
  ASSIGNMOB(21003, cityguard);

  ASSIGNMOB(21078, receptionist);

  /* AGRABA CITY */
  ASSIGNMOB(22558, postmaster);
  ASSIGNMOB(22573, receptionist);
  ASSIGNMOB(22574, cryogenicist);
  ASSIGNMOB(22554, ali_rent_a_harem);
  ASSIGNMOB(22579, hakico_discount_fertilizer);
  ASSIGNMOB(22559, guild); /* clerics guild */
  ASSIGNMOB(22550, guild); /* thieves guild */
  ASSIGNMOB(22563, guild); /* mages guild */
  ASSIGNMOB(22565, guild); /* warriors guild */
  ASSIGNMOB(22583, guild); /* paladins guild */
  ASSIGNMOB(22585, guild); /* anti-paladins guild */
   
  /* AGRABA PALACE */
  ASSIGNMOB(22652, agraba_rajah);
  ASSIGNMOB(22662, agraba_iago);
  ASSIGNMOB(22648, agraba_abu);
  ASSIGNMOB(22647, agraba_aladdin);
  ASSIGNMOB(22639, agraba_jasmine);
  ASSIGNMOB(22661, agraba_jafar);
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

  /* Diablo */
  ASSIGNOBJ(2914, evaporating_potions);
  ASSIGNOBJ(2915, evaporating_potions);
  ASSIGNOBJ(2916, mana_potions_low);
  ASSIGNOBJ(2917, mana_potions_high);
  ASSIGNOBJ(2918, evaporating_potions);
  ASSIGNOBJ(2919, diablo_recall);
  ASSIGNOBJ(2937, mana_potions_low);
  ASSIGNOBJ(2938, mana_potions_high);

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

  ASSIGNOBJ(6025, gen_board);	/* social board */

  ASSIGNOBJ(3034, bank);	/* atm */

  /* Toy story */
  ASSIGNOBJ(4900, evaporating_newbie_eq);
  ASSIGNOBJ(4901, evaporating_newbie_eq);
  ASSIGNOBJ(4902, evaporating_newbie_eq);
  ASSIGNOBJ(4903, evaporating_newbie_eq);
  ASSIGNOBJ(4904, evaporating_newbie_eq);
  ASSIGNOBJ(4905, evaporating_newbie_eq);
  ASSIGNOBJ(4906, evaporating_newbie_eq);
  ASSIGNOBJ(4907, evaporating_newbie_eq);
  ASSIGNOBJ(4908, evaporating_newbie_eq);
  ASSIGNOBJ(4909, evaporating_newbie_eq);
  ASSIGNOBJ(4910, evaporating_newbie_eq);
  ASSIGNOBJ(4911, evaporating_newbie_eq);
  ASSIGNOBJ(4912, evaporating_newbie_eq);
  ASSIGNOBJ(4913, evaporating_newbie_eq);
  ASSIGNOBJ(4914, evaporating_newbie_eq);
  ASSIGNOBJ(4915, evaporating_newbie_eq);
  ASSIGNOBJ(4916, evaporating_newbie_eq);
  ASSIGNOBJ(4917, evaporating_newbie_eq);
  ASSIGNOBJ(4921, evaporating_newbie_eq);
  ASSIGNOBJ(4922, evaporating_newbie_eq);
  ASSIGNOBJ(4923, evaporating_newbie_eq);

  /* Dragonlance */
  ASSIGNOBJ(17920, crysania_medallion);
  ASSIGNOBJ(17986, pure_chaos);
  ASSIGNOBJ(17989, kitiara_sword);
  ASSIGNOBJ(17990, staff_of_magius);
  ASSIGNOBJ(17991, kitiara_stick);
  ASSIGNOBJ(17992, rabbitslayer);
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
  SPECIAL(shuttle_station);
  SPECIAL(raistlin_ghost);
  SPECIAL(waywreth_forest);
  SPECIAL(wight_and_beard);
  SPECIAL(slow_decay);
  SPECIAL(museum_tour1);
  SPECIAL(museum_tour2);
  SPECIAL(museum_tour3);

  ASSIGNROOM(0, dump);

  ASSIGNROOM(1530, museum_tour1);
  ASSIGNROOM(1520, museum_tour2);
  ASSIGNROOM(1526, museum_tour2);
  ASSIGNROOM(1531, museum_tour2);
  ASSIGNROOM(1512, museum_tour3);

  ASSIGNROOM(3030, dump);
  ASSIGNROOM(3031, pet_shops);
  ASSIGNROOM(3063, slow_decay);

  ASSIGNROOM(3699, poppy_field);
  ASSIGNROOM(3757, poppy_field);
  ASSIGNROOM(3758, poppy_field);
  ASSIGNROOM(3759, poppy_field);
/*  ASSIGNROOM(3677, falling_down); */
  ASSIGNROOM(3701, falling_down);
  ASSIGNROOM(3702, falling_down);
  ASSIGNROOM(3703, falling_down);
  ASSIGNROOM(3704, falling_down);

  ASSIGNROOM(17207, lag_trap);
  ASSIGNROOM(17279, pet_shops);

  /* Dragonlance */
  ASSIGNROOM(18016, wight_and_beard);
  ASSIGNROOM(18057, raistlin_ghost);
  ASSIGNROOM(18190, waywreth_forest);
  ASSIGNROOM(18192, waywreth_forest);
  ASSIGNROOM(18193, waywreth_forest);
  ASSIGNROOM(18195, waywreth_forest);
  ASSIGNROOM(18196, waywreth_forest);
  ASSIGNROOM(18198, waywreth_forest);

  ASSIGNROOM(21116, pet_shops);
  ASSIGNROOM(22580, pet_shops);
  ASSIGNROOM(22535, dump);

  ASSIGNROOM(24508, shuttle_station);
  ASSIGNROOM(24511, shuttle_station);

  if (dts_are_dumps)
    for (i = 0; i <= top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	world[i].func = dump;
}
