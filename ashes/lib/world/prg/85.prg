#8500
>death_prog  100~
mpecho $i grimaces in pain from what are obviously mortal wounds,
mpecho and says, 'Well, Mystra, ye've been telling about that
mpecho afterlife of yers for nigh on a thousand years now. I guess
mpecho it's about time I had a look at it.'
~
>act_prog  p utters the words, 'yufzbarr'~
mpechoaround $n $i raises and eyebrow and smirks at $n.
mpachoat $n $i raises an eyebrow at you and smirks.
say So, it's fireballs you want?
say This is how we used to cast those in Myth Drannor.
mpcast 127 'fireball' $n
tell $n Now, wasn't mine much nicer?
~
>fight_prog  70~
mpcast 127 'fireball' $n
~
>fight_prog  100~
if rand(50)
	mpecho $i says 'Younglings these days...always so quick with
	mpecho swords and slaying spells, trying to kill their elders
	mpecho and their betters.'
else
	mpecho says 'Bah! Doesn't anyone these days have any patience?
	mpecho It's always "help me kill this" or "Elminster loaded that,"
	mpecho when in just a few hundred years they could earn the same
	mpecho amount of power without all the bloodletting.'
endif
~
|
#8550
>greet_prog  100~
if rand(50)
	mpecho $i grins as you enter.
	mpecho $i says 'All visitors to my temple must shed a drop of
	mpecho blood...but don't worry, this won't hurt a bit.
	backstab $n
	mpachoat $n $i tells you 'I lied. I do that, you see.'
else
	mpachoat $n $i laughs in your face.
	mpechoaround $n $i laughs in $n's face.
	mpecho $i says 'Only an idiot would barge in on the Lord of Murder
	mpecho and expect to live!'
	mpecho $i adds '...and seeing as you're expecting to die, I'd hate
	mpecho to dissapoint you.'
	backstab $n
endif
~
>fight_prog  15~
	mpecho $i disappears suddenly.
 	mpechoaround $n $i reappears behind $n!
	mpat 8555 force cyric mpat $n backstab $n
~
|
#8561
>death_prog  100~
mpforce all.eyestalk give all beholder
mppurge charmpersoneye
mppurge charmmonstereye
mppurge sleepeye
mppurge telekinesiseye
mppurge stoneeye
mppurge disintegrateeye
mppurge feareye
mppurge sloweye
mppurge causewoundseye
mppurge deathrayeye
~
|
#8562
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r Th beholder starts to control your body!!
	mpechoat Fighting to regain control, you drop all your equipment.
	mpforce $r drop all
else
	mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8563
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r The beholder takes control of your mind!!
	mpechoat $r It forces you to surrender.
	mpforce $r remove all
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8564
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r Your eyelids start drooping and you get sleepy.
	mpcast 100 'sleep' $r
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8565
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r You feel a mysterious force lifting you off your feet.
	hurl $r
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8566
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r Your feet feel rooted to the ground.
	mpforce $r wimpy 0
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8567
>death_prog  100~
junk all
mppurge $i
~
|
#8568
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
        mpechoat $r Inexplicable fear enters your heart.
        mpforce $r flee
        mpat $r mpechoat $r You coward!!	
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8569
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r The world around you begins to speed up.
	mpcast 100 'slow' $r
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8570
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r You flesh starts ripping open!!
	mpcast 50 'harm' $r
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
#8571
>death_prog  100~
junk all
mppurge $i
~
>fight_prog  30~
mpechoat $r One of the beholder's eyestalks swivels toward you.
mpechoaround $r One of the beholder's eyestalks swivels toward $r.
if rand(50)
	mpechoat $r You feel your life-force being snuffed out!
	mpcast 50 'word of death' $r
else
        mpechoat $r You manage to resist the eye's influence.
endif
~
|
$
