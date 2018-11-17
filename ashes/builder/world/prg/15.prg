#1500
>fight_prog 10~
mpecho $I releases the full strength of the Night on the room.  Everything
mpecho goes black, and the room falls deadly quiet.  A beam of moonlight spot-
mpecho lights $I as you feel the terrible power of the Night wracking
mpecho your body.  You open your mouth but not even your screams can 
mpecho penetrate this darkness.
mpdamage 30d9+450 all
mpset all hunger 0
~
>fight_prog 15~
mpechoat $n $I allows you to taste the hunger of the darkness.
mpechoat $n Black shadows uncoil from around him, tendrils of deepest night
mpechoat $n penetrate your body and draw your soul into the darkness....
mpechoar $n Black shadows uncoil from around $I, tendrils of deepest
mpechoar $n night penetrate $n's body and draws $s soul into the darkness....
mpdamage 2d40+580 $n
mpset $n hunger 0
mpechoat $n You cry out in pain, and yet.. yearn to feel its touch again....
mpechoar $n $n cries out in pain.
~
>fight_prog 20~
mpechoat $r $I holds your gaze and draws you into the shadows cloaking him...
mpechoat $r You feel the death of sleep creeping up on you as your limbs grow
mpechoat $r heavy and you slowly sink into the depths of the darkness....
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 curse $r
mpscast 300 slow $r
mpscast 300 slow $r
mpset $r hunger 0
mpstun 3 $r
~
>death_prog 100~
mpecho You see the star around $I's neck grow dim, and as it darkens
mpecho it slowly disappears.  MidKnight laughs at your foolhardy attempt to
mpecho kill him and uses the Night to encompass himself in darkness, vanishing
mpecho without a trace.  The shadows where MidKnight once stood swirl into
mpecho a vortex then slowly fade away.
~
|
#1501
>fight_prog 10~
mpechoat $r $I points a finger at you and your chest feels as if it
mpechoat $r will explode! You feel all the energy in your body flow away.
mpechoaround $r $I points at $r. A beam of energy shoots out of $L
mpechoaround $r chest and twines around $I's hand.
mpset $r mana 0
mpset $r move 5
mpunaffect $r
mpdamage 10d10+0 $r
mpecho $I raises $l arms, and energy engulfs $k. $j utters a few
mpecho arcane words, and purple fire sweeps through the room!
mpdamage 5d50+150 all
~
>fight_prog 11~
mpcast 70 'implosion'
~
>fight_prog 19~
mpcast 140 'fireball' $n
~
>fight_prog 15~
mpcast 130 'color' $r
~
>fight_prog 9~
mpechoat $r $I stares at you and utters the words, 'wall of force'.
mpechoaround $r $I stares at $r and utters the words, 'wall of force'.
mpechoat $r You are thrown backwards and an invisible wall blocks you.
mpechoaround $r $r is thrown out of the room.
mptransfer $r 1508
mpstun 8 $r
~
>fight_prog 10~
if name($n) != $r
  mpechoat $r $I stares at you.
  mpechoaround $r $I stares at $r.
  mpecho $I says, 'Nothing is real but magic!'
  mpset $r hit 0
  mpset $r mana 1000
  mpset $r move 0
else
  mpecho $I says, 'Thanks for the spells, but I really don't need these.'
  mpunaffect $i
endif
~
>fight_prog 11~
mpecho $I glares around the room.
mpechoat $r $l's gaze stops on you.
mpechoaround $r $l's gaze stops on $r.
if class($r) == 0
  smile
else
  if class($r) == 1
    smile
  else
    if class($r) == 4
      smile
    else
      if class($r) == 6
        smile
      else
        mpecho $I shouts, 'NON-CASTER!'
        mpechoat $r $I hurls a blast of pure magic at you!
        mpechoaround $r $I hurls a blast of pure magic at $r.
        mpechoaround $r The blast hits $K and explodes!
        mpdamage 10d30+200 $r
        mpdamage 10d30+50 all
      endif
    endif
  endif
endif
~
>hitprcnt_prog 10~
if rand(30)
  mpecho Small arcs of magic flash across $I's wounds and they close.
  mpheal 5d20+100 $i
endif
~
|
#1502
>fight_prog 25~
mpecho $I spreads $l wings and flames a bright red!
mpheal 250 $i
~
>fight_prog 33~
mpechoat $r $I flaps $l wings at you, engulfing you in flames!
mpechoar $r $I flaps $l wings at $r, engulfing $K in flames!
mpdamage 5d75+350 $r
~
>fight_prog 20~
mpechoat $r $I covers you in ashes and strips you of your protection.
mpechoar $r $I covers $r in ashes, stripping $K of $L protection.
mpunaffect $r
~
>death_prog 100~
mpecho $I bursts into flames as $j dies.
mpecho A phoenix rises from the ashes!
mpunaffect $i
mpat 1535 mpmload 1514
mpat 1535 remove all
mpat 1535 give all progphoenix
mpat 1535 mpstrans progphoenix 1506
mpforce progphoenix mpkill $n
~
|
#1503
>fight_prog 10~
mpecho There's a loud BANG and then all settles.
mpdamage 50d4+50 all
mpcalm
mpkill $n
~
>fight_prog 22~
if class($r) == 1
  say That'll be enough of you.
  mpechoar $r The earth molds around you holding you motionless!
  mpechoar $r The earth molds around $r holding $K motionless.
  mpstunn 1d4+1 $r
else
  say I never liked your kind.
  mpechoat $r $I waves $l hand and the Earth throws you into the air,
  mpechoat $r sending you slamming back down into the ground!
  mpechoar $r $I waves $l hand and the Earth throws $r into the air,
  mpechoar $r sending $K slamming back down into the ground.
  mpdamage 30d10+75 $r
endif
~
>fight_prog 21~
mpechoat $r $I pulls a small box out of his pocket and smashes you into it!
mpechoar $r $I pulls a small box out of his pocket and smashes $r into it.
mpkill $r
mpkill $r
mpkill $r
mpkill $r
mpkill $r
~
>fight_prog 27~
mpechoat $r $I conjures up a large stone club and bonks you over the head!
mpechoar $r $I conjures up a large stone club and bonks $r over the head!
mpforce $r gossip Quit Bonking Me Tindalos!  I'm Telling Mommy!
mpdamage 15d15+30 $r
~
>death_prog 20~
mpecho $I sends you back in time!
mptrans all 3072
~
|
#1504
>fight_prog 11~
mpechoat $r $I knocks the stuffing out of you!
mpechoar $r $I knocks the stuffing out of $r.
mpforce $r remove all
mpdamage 10d20+150 $r
~
>fight_prog 18~
mpecho Teddy Looks at you with love and decides it's a "group hug" moment.
mpdamage 15d20+250 all
~
>fight_prog 22~
mpechoat $n Teddy looks at you, decides you need some Extra Lovin',
mpechoat $n and bearhugs you.
mpechoar $n Teddy looks at $n, decides $e needs some Extra Lovin',
mpechoar $n and bearhugs $m.
mpdamage 10d50+300 $n
~
>fight_prog 37~
bearhug $r
mpecho You hear ribs cracking.
mpdamage 2d100+300 $r
mpstun 1d3+1 $r
~
|
#1505
>fight_prog 15~
mpecho $I waves $l arms as if clearing a board and sends you flying!
mpdamage 100d5+25 all
~
>fight_prog 82~
if class($r) == 0
or class($r) == 1
or class($r) == 4
or class($r) == 6
  mpechoat $r $I laughs at your tiny protection spells. With a stone fist $j
  mpechoat $r pounds you into the ground.
  mpechoaround $r $I laughs at $r and pounds $K into the ground with a stone fist.
  mpunaffect $r
  mpdamage 10d35+50 $r
endif
~
|
#1508
>fight_prog 10~
moechoat $r $I pulls out a black walnut staff and cracks you upside the head!
moechoar $r $I pulls out a black walnut staff and cracks $r upside the head.
mpdamage 10d79+200 $r
~
>fight_prog 10~
wave $r
if class($r) == 1
  mpechoar $r $r disappears.
  mptrans $r 3072
else
  mpechoar $r $r shimmers briefly.
  mpechoat $r You feel less protected.
  mpunaf $r
endif
~
>fight_prog 17~
mpecho $I stomps on the ground with the force of an earthquake.
mpdamage 30d25+200 all
~
|
$
