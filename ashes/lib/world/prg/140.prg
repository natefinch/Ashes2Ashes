#14000
>death_prog  100~
mpecho Wolverine yells 'JEEEEAAAAANNNNNN!'
~
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  40~
mpheal 300 $i
mpecho Wolverine's wounds are healed.
~
>fight_prog  10~
mpheal 400 $i
mpecho Wolverine's wounds are healed!
~
>fight_prog  5~
mpheal 800 $i
mpecho Wolverine's wounds are COMPLETELY HEALED!
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14001
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  15~
mpecho Storm yells 'I call upon the forces of nature to assist me!
mpmload 14020
mpmload 14021
mpmload 14019
mpforce blizzard kill $r
mpforce thundercloud kill $r
mpforce tornado kill $r
~
>fight_prog  5~
mpecho Storm exclaims 'I call upon a blizzard to assist me!
mpmload 14021
mpforce blizzard kill $n
~
>fight_prog  5~
mpecho Storm exclaims 'I call upon a thundercloud to assist me!
mpmload 14019
mpforce thundercloud kill $n
~
>fight_prog  5~
mpecho Storm exclaims 'I call upon a tornado to assist me!
mpmload 14020
mpforce tornado kill $n
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14002
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  25~
mpechoaround $n Gambit throws a glowing card at $n!
mpechoat $n Gambit throws a glowing card at you!
mpechoat $n It explodes in your face!
mpdamage 127d8+127 $n
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14003
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  25~
mpechoaround $n $n is hit by Cyclop's laser blast!
mpechoat $n You are hit by Cyclop's laser blast!
mpdamage 127d8+127 $n
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14004
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpcast 127 'calm'
mpforce $n kill $r
mpechoat $n Jean forces you to kill!!  You must have BLOOD!
~
>fight_prog  16~
mpecho Jean focuses...
mpechoaround $n $n is brainfried by Jean!
mpechoat $n You feel your brain cook in your skull.
mpdamage 127d8+127 $n
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14005
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpechoaround $n Jubilee throws a blinding light at $n!
mpechoat $n You are hit and blinded!
mpcast 127 'calm'
mpcast 127 'blindness' $n
mpdamage 127d8+127 $n
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14006
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpechoat Beast leaps off the wall and lands on YOU!
mpechoaround Beast leaps off the wall and lands on $r!
mpkill $r
mpkill $r
mpkill $r
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14007
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
if isnpc($n)
mpecho Colossus grabs $n and crushes $m into nothingness!
mppurge $n
else
mpechoaround $n Colossus grabs $n by the neck and throws $m out of the room!
mpechoat $n Colossus grabs you by the neck and throws you out of the room!
mptrans $n 3001
endif
~
>fight_prog  8~
mpechoaround $n Colossus grabs $n by the neck, whips $m around the room, and heaves
mpechoaround $n $n through the ceiling!
mpechoat $n Colossus grabs you by the neck, whips you around the room and
mpechoat $n throws you through the ceiling!..
mpdamage 40d8+40 $n
mpechoat $n OUCH, that had to HURT!
mptrans $n 12020
~
>fight_prog  8~
mpechoaround $n Colossus grabs $n by the neck, whips $m around the room, and
mpechoaround $n crushes $n through the FLOOR!
mpechoat $n Colossus grabs you by the neck, whips you around the room and
mpechoat $n smooshes you through the floor!
mpdamage 40d8+40 $n
mpechoat $n Ouch, that had to HURT!
mptrans $n 2396
gossip Here Mahn-Tor, CATCH!
~
>fight_prog  8~
mpechoaround $n Colossus grabs $n by the leg, slaps $m up and down on the
mpechoaround $n floor, then whips $n through the wall!
mpechoat $n Colossus grabs you by the leg, whacks your head on the floor a
mpechoat $n few times, then launches your through the wall.
mpdamage 40d8+40 $n
mpechoat $n Ouch, that had to HURT!
mptrans $n 23773
gossip Here Raptors, it's FEEDING TIME!
~
>fight_prog  8~
mpechoaround $n Colossus grabs $n by the arm, slaps $m against the walls, then launches
mpechoaround $n $m through the ceiling!
mpechoat $n Colossus grabs you by the arm, smacks you against the wall a few
mpechoat $n times and tosses you through the ceiling.
mpdamage 40d8+40 $n
mpechoat $n Ouch, that had to HURT!
mptrans $n 12768
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14008
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpset $n hit 1
mpset $n mana 1
mpset $n move 1
mptrans $n 3001
mpechoaround $n Rogue touches $n on the shoulder and $e looks DRAINED.
mpechoat $n You are DRAINED TO DEATH!
mpat $n mpforce $n sleep
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14009
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpecho Nightcrawler focuses and poofs!
if rand(25)
  mpgoto 14057
else
  if rand(33)
    mpgoto 14058
  else
    flee
  endif
endif
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14010
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  4~
mpecho Xavier presses a button on his chair.
mpecho You see a laser create someone out of molten plastic!
mpmload 14003
mpforce cyclops rescue xavier
mpforce cyclops kill $n
~
>fight_prog  4~
mpecho Xavier yells 'Help, I am being attacked!
mpecho Xavier presses a button.
mpecho Something is created from a pool of molten metal.
mpmload 14007
mpforce colossus rescue xavier
mpforce colossus kill $n
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14011
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
look $n
mpechoaround $n Magneto focuses on $n.
mpechoat $n Magneto focuses on you, you feel your equipment being pulled
mpechoat $n from your body!
mpechoat $n Struggling to keep these titanic forces from robbing you of
mpechoat $n your equipment, you fumble wildly with your carried items.
mpforce $n drop all
mpforce $n remove all.charm
mpforce $n remove all.ring
mpforce $n remove all.plate
mpforce $n remove all.sceptre
mpechoaround $n $n has some equipment removed from $s body!
mpechoat $n You can feel charms, rings and so on being pulled from you!
mpforce $n give all.ring $i
mpforce $n give all.charm $i
mpforce $n give all.plate $i
mpforce $n give all.sceptre $i
mpechoaround $n $n has had $s equipment taken from $m!
mpechoat $n Magneto has taken your equipement from you!
cackle
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14012
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpecho Juggernaut says 'bah, yer a joke!'
mpechoat $n Juggernaut tells you 'For that yer gonna pay punk!'
mpechoaround $n Juggernaut charges $n and $n is sent flying through the wall!
mpechoat $n Juggernaut charges you and sends you flying through the wall!
mpdamage 60d8+60 $n
mpechoat $n Ouch, that wall hurt!
mptrans $n 14000
kill $r
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14014
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  20~
mpecho Sabertooth hollers, "Time to die punk!"
mpechoat $r Sabertooth rips your chest open with his claws!
mechoaround $r Sabertooth rips $r's chest open with his claws!
mpdamage 20d10+210 $r
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14016
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14017
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
#14018
>act_prog  'blindness'~
mpcast 50 'heal' $i
~
>fight_prog  100~
if isnpc($z)
  if number($z) < 14000
  or number($z) > 14099
    mppurge $z
  endif
endif
~
|
$
