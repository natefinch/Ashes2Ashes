#8600
>death_prog  100~
mpecho Elminster suddely appears out of thin air!
mpecho Elminster frowns and says, 'Hmm, a tarrasque. The
mpecho only way to kill one permenantly is to WISH it dead.
mpecho Since none of you adventuring types can do that sort
mpecho of thing, it falls to 'ole Elminster to put things
mpecho right.'
mpecho Elminster utters the words, 'wish', and then vanishes.
~
>entry_prog  100~
if isnpc($r)
	kill $r
endif
~
>all_greet_prog ~
if isnpc($r)
	kill $r
endif
~
>fight_prog  15~
mpecho $i suddenly charges you!!!!
berserk
berserk
mpasound The ground thunders beneath your feet!
~
>rand_prog  5~
mpasound You heard heavy, thudding footsteps approaching.
~
|
#8601
>fight_prog  100~
if rand(90)
        mpcast 127 'fireball' $r
else
	mpcast 127 'slow' $r
endif
~
|
#8602
>greet_prog  99~
	mpecho $i says 'Don't you now that the living aren't allowed
	mpecho in Hades? Well, we'll fix that though.'
	mpkill $n
~
>fight_prog  50~
	mpecho $i let's out a demonic roar!
	mpechoaround $n $i plunges his hand into the chest of $n and
	mpechoaround $n rips out $m very soul.
	mpechoat $n $i plunges his hand into your chest and rips out
	mpechoat $n your SOUL!!
	mpcast 127 'chill touch' $n
~
|
$
