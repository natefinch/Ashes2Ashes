#8600
>death_prog  100~
mpecho Elminster suddely appears out of thin air!







>entry_prog  100~
if isnpc($r)



>all_greet_prog ~
if isnpc($r)



>fight_prog  15~
mpecho $i suddenly charges you!!!!




>rand_prog  5~
mpasound You heard heavy, thudding footsteps approaching.

|
#8601
>fight_prog  100~
if rand(90)





|
#8602
>greet_prog  99~
	mpecho $i says 'Don't you now that the living aren't allowed



>fight_prog  50~
	mpecho $i let's out a demonic roar!






|
$