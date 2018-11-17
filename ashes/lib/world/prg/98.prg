#9803
>fight_prog  10~
mpcast 127 'word of death' $r
~
>fight_prog  17~
mpecho $I shouts, 'This is for killing my love!'
mpecho Flames wash over you!
mpdamage 6d30+410 all
~
>fight_prog  5~
mpechoat $r $I stares at you and whispers a word that is too quiet to make out,
mpechoat $r yet echoes over and over in your mind...
mpechoaround $r $I stares at $r and whispers something that you can't make out.
mpset $r hit 0
mpdamage 22 $r
~
>fight_prog  20~
mpechoat $r $I picks you up and snaps your neck!!
mpecharound $r $I picks $r up, snaps $L neck, and tosses $K to the ground!
mpset $r hit 1
~
>fight_prog  22~
mpcast 127 'restore' $i
~
>fight_prog  20~
mpecho $I snaps $l fingers.
smirk
mpunaffect all
~
|
#9806
>fight_prog 10~
mpecho $I transforms into a giant snake and thrashes $l tail wildly!
mpdamage 10d15+110 all
~
>fight_prog 11~
mpecho Two shadowy copies of $I walk out of $k.
mpecho $I utters the words, 'ice storm'.
mpecho A shadow utters the words, 'flamestrike'.
mpecho A shadow utters the words, 'call lightning'.
mpecho There is a bright flash as all 3 attacks hit you at once.
mpdamage 1d50+200 all
mpecho When your vision clears, the shadows are gone.
~
>fight_prog 12~
mpecho $I charges up a blue ball of energy.
mpechoat $r $I fires a charged blast at you!
mpechoaround $r $I fires a charged blast at $r.
mpdamage 4d20+50 $r
~
>fight_prog 11~
mpecho $I charges up a blue ball of energy, which turns white.
mpechoat $r $I fires two charged blasts at you!
mpechoaround $r $I fires two charged blasts at $r.
if rand(50)
  mpecho The two charges merge.
  mpdamage 20d25+150 $r
else
  mpdamage 15d25+100 $r
endif
~
>fight_prog 8~
mpecho $I raises $l head and hisses loudly.
mpecho Several snakes slither up through the ground at your feet!
mpmload 9832
mpforce mprogsnake kill $n
if rand(70)
  mpmload 9832
  mpforce mprogsnake kill $r
endif
if rand(70)
  mpmload 9832
  mpforce mprogsnake kill $r
endif
if rand(70)
  mpmload 9832
  mpforce mprogsnake kill $n
endif
~
>fight_prog 9~
mpechoat $r $I turns toward you. As you meet $l gaze, $I's face
mpechoat $r transforms into that of a viper's. A sword appears in $l
mpechoat $r hands, and $j slashes at you!
mpechoaround $r $I transforms into a half-snake half-man and slashes
mpechoaround $r at $r with a sword that appears in $l hands!
mpdamage 2d50+400 $r
~
>fight_prog 9~
mpecho $I waves $l hands and the spells around you fall away.
mpunaffect all
~
>fight_prog 10~
mpecho $I charges up a blue ball of energy, which turns white.
mpecho $I starts to glow gold.
mpechoat $n $I fires a two charged blasts at you!
mpechoaround $n $I fires a two charged blasts at $r.
mpecho The two charges merge.
mpdamage 20d25+150 $n
if rand(50)
  mpechoat $r $I strikes at you with a saber!
  mpechoaround $r $I strikes at $r with a saber.
  mpdamage 3d20+70 $r
  if rand(30)
    mpechoat $r $I strikes at you with a saber!
    mpechoaround $r $I strikes at $r with a saber.
    mpdamage 3d20+70 $r
    if rand(10)
      mpechoat $r $I strikes at you with a saber!
      mpechoaround $r $I strikes at $r with a saber.
      mpdamage 3d20+70 $r
    endif
  endif
endif
~
>fight_prog 2~
mpechoat $r $I uppercuts you into a spiked ceiling!
mpechoaround $r $I uppercuts $r into a spiked ceiling.
say Fatality.
mpset $r hit 0
mpdamage 22 $r
~
>death_prog 100~
mpecho $I blows up!
mpdamage 5d100+250 all
~
|
#9810
>fight_prog  10~
mpechoat $r $I smothers you with a killer bearhug!
mpechoaround $r $I smothers $r with a killer bearhug!
mpset $r hit 1
~
>fight_prog  22~
bearhug $r
mpdamage 2d20+280 $r
~
>fight_prog  21~
mpecho $I grows to ten times $l size and crushes everyone
mpecho in an enormous bearhug!
mpdamage 1d100+350 all
~
|
#9824
>fight_prog  5~
mpecho $I calls out in a strange language and a dragon flies in to help $k!
mpmload 9827
mpforce dragon kill $n
~
>fight_prog  11~
mpechoat $n $I transforms into a brilliant gold dragon and slashes you!
mpechoaround $n $I transforms into a brilliant gold dragon and slashes $n!
mpdamage 4d25+250 $n
~
>fight_prog  17~
mpechoat $r $I transforms into a blazing red dragon and breathes fire on you!
mpechoaround $r $I transforms into a blazing red dragon and breathes fire on $r!
mpdamage 4d50+400 $r
~
>fight_prog  20~
mpecho $I transforms into a magnificent blue dragon and flaps $l wings,
mpecho pummeling you with gale force wind!
mpstun 1d3+0 all
~
>fight_prog  27~
mpecho $I transforms into a glistening green dragon and breathes gas!
mpdamage 2d50+350 all
~
|
#9850
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
mpecho archane words, and purple fire sweeps through the room!
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
mptransfer $r 9830
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
  mpecho Small archs of magic flash across $I's wounds and they close.
  mpheal 5d20+100 $i
endif
~
|
#9851
>fight_prog  30~
mpecho $I utters the words, 'dispel magic'.
grin
mpunaffect $i
~
>fight_prog  7~
mpecho $I utters the words, 'Mordenkainen's Disjunction'.
cackle
mpunaffect all
~
>fight_prog 15~
wave $n
mpecho $I utters the words, 'meteor swarm'.
mpscast 300 'fireball' $n
~
>fight_prog 10~
mpecho $I utters the words, 'time stop'.
mpstun 5 all
mpecho You feel queasy and the world begins to speed up around you...
~
>fight_prog 25~
mpscast 240 'armor' $i
mpecho $I utters the words, 'stoneskin'.
~
|
$
