#13000
>fight_prog  100~
mpechoat $n $I looks at you in disgust.
mpechoat $n $I gets some security guards to throw you out.
mptransfer $n 3072
~
>keyword_prog  sign up~
mpechoat $n $I sends you off to the tournament and wishes you luck.
mpechoat $n You're going to need it.
mpechoaround $n $n enters the tournament.
if level($n) > 35
 mptrans $n 13002
else
 mptrans $n 13052
endif
~
|
#13001
>commandtrap_prog  flee~
mpechoaround $n $I catches $n running away and steps down on $n's ankle.
mpechoat $n As you turn to flee, $I stomps on your ankle.
mpechoat $n You hear a sickening crunch and feel bones break.
mpdamage 20d20+50 $n
~
|
#13003
>fight_prog  10~
mpecho Your blood freezes as you hear someone's death cry.
mpecho The Khabal Ghoul gains strength from the death of a living being...
mpheal 100d100+100 $i
~
|
#13004
>fight_prog  70~
mpecho $I calls up gale-force winds to batter you.
mpdamage 100d3+0 all
~
|
#13009
>rand_prog  4~
mpecho $I swallows something.
drop all.corpse
mpjunk all
get all.corpse
burp
grin
~
|
#13015
>fight_prog  25~
mpecho $I laughs heartily.
mpecho $I says 'Ho ho ho midgets, you think you can stand in my way?'
~
|
#13016
>fight_prog  40~
mpechoat $r $I jumps on top of you and mauls you!!
mpechoaround $r $I pounces on $r and mauls $K!!
mpkill $r
mpkill $r
mpkill $r
mpkill $r
mpkill $r
mpkill $r
~
|
#13021
>death_prog  100~
mpunaffect $i
mpecho $I splits open, spilling the contents of it's stomach.
mpat 13046 mptransfer all 13012
mpat 13046 get all
drop all
mpat 13046 get all
drop all
mpat 13046 get all
drop all
mpat 13046 get all
drop all
~
>fight_prog  10~
mpecharound $n $I swallows $n whole!!
mpechoat $n $I swallows you whole!!
mptransfer $n 13046
~
>fight_prog  50~
mpat 13046 mpecho Digestive juices burn you.
mpat 13046 mpdamage 50 all
~
>rand_prog  50~
mpat 13046 mpecho Digestive juices burn you.
mpat 13046 mpdamage 50 all
~
|
#13025
>fight_prog  40~
mpechoat $n $I strikes at your source of mana!
mpset $n mana 0
~
|
#13027
>fight_prog  100~
flee
~
|
#13030
>fight_prog  20~
mpechoat $n $I drains all your life.
mpechoaround $n $I drains the life from $n.
mpset $n hp 0
~
|
#13038
>fight_prog  50~
if isevil($r)
 mpechoat $r $I punishes you for your evil ways.
 mpdamage 100d5+50 $r
endif
~
|
#13039
>fight_prog  30~
mpecho The damage you've done is redirected elsewhere.
mpheal 20d20+100 $i
~
>fight_prog  30~
mpheal 10d20+50 $i
~
|
#13042
>fight_prog  95~
mpecho $I alters time.
if rand(50)
 mptransfer $n 13002
else
 mptransfer $n 13040
endif
~
|
#13043
>fight_prog  30~
mpecho Sand warriors rise up from the desert sands to aid $I.
mpmload 13044
mpmload 13044
mpmload 13044
mpforce sand kill $n
mpforce 2.sand kill $n
mpforce 3.sand kill $n
~
|
#13054
>fight_prog  10~
mpecho $I makes some strange noises and gestures.
mpecho A confused-looking goblin appears in a puff of smoke.
mpmload 13051
mpforce raider kill $n
~
|
#13069
>fight_prog  30~
mpechoat $n $I points his finger at you and an explosion engulfs you.
mpechoaround $n $I points at $n, who explodes.
mpdamage 50 $n
~
>rand_prog  10~
say There are some who call me...Tim?
~
>rand_prog  10~
mpecho $I crooks two of his fingers to look like fangs.
say It has nasty, big, pointy teeth!
~
|
$
