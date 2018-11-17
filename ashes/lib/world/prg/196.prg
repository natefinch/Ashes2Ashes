#19600
>fight_prog 33~
mpcast 3 'magic missle' $n
~
|
#19601
>fight_prog 33~
mpcast 100 'cure light' $i
~
|
#19602
>fight_prog 25~
mpcast 30 'lightning bolt' $n
~
|
#19603
>fight_prog 50~
mpcast 100 'cure critic' $i
~
|
#19605
>fight_prog 33~
mpcast 3 'magic missle' $n
~
|
#19606
>fight_prog 33~
mpcast 100 'cure light' $i
~
|
#19608
>fight_prog 33~
mpcast 30 'fireball' $n
~
>rand_prog 20~
say To make a record of your journey is called Save.  You can make a Save at any time by typing "save".
~
>rand_prog 25~
say When you hear a new name, you'd better note its direction.  Same as maps, the upper part means north and lower south.  Left means west, and to the right is east.
~
>rand_prog 33~
say When it smells like Chocobos, use carrot.  Big-Chocobos will surely appear!
~
>rand_prog 50~
say Recklessness often proves to be damaging.  When you are losing a battle, don't hesitate to run away by typing "flee".
~
>rand_prog 100~
say Don't keep running away from your enemies!  You lose some experience when escaping from them.
~
|
#19614
>fight_prog 50~
mpcast 100 'cure critic' $i
~
|
#19615
>all_greet_prog 100~
mpunaffect $n
mpecho The crystal shimmers faintly.
~
|
#19616
>fight_prog 5~
mpecho Terra summons an esper to assist her!
mpecho Ragnarok appears!
mpechoat $r Ragnarok points his finger at you and you feel life slip from your limbs as
mpechoat $r you begin to transform into an inanimate object. At the last minute however,
mpechoat $r the divine power of Azuth rescues your spirit from non-life.
mpat 3072 mpecho $r appears in a white flash, and slumps to the ground.
mptrans $r 3072
mpat 3072 mpforce $r puke $r
mpat 3072 mpforce $r sit
mpat 3072 mpstun 10 $r
mpat 3072 mpechoat $r You are momentarily stunned by the experience.
if rand(50)
  mpecho Ragnarok points at $r and $J changes into a Mysidian souvenir mug!
  mpoload 19686
  mpdrop souvenir
else
  mpecho Ragnarok points at $r and $J changes into Yxylu's enchanted pot!
  mpoload 8404
  mpdrop enchanted
endif
~
>fight_prog 5~
mpecho Terra summons an esper to assist her!
mpecho Ragnarok appears!
mpechoat $r Ragnarok points his finger at you and you feel life slip from your limbs as
mpechoat $r you begin to transform into an inanimate object. At the last minute however,
mpechoat $r the divine power of Azuth rescues your spirit from non-life.
mpat 3072 mpecho $r appears in a white flash, and slumps to the ground.
mptrans $r 3072
mpat 3072 mpforce $r puke $r
mpat 3072 mpforce $r sit
mpat 3072 mpstun 10 $r
mpat 3072 mpechoat $r You are momentarily stunned by the experience.
if rand(50)
  mpecho Ragnarok points at $r and $J changes into a copy of Windows95!
  mpoload 20690
  mpdrop win95
else
  mpecho Ragnarok points at $r and $J changes into a Metallica T-shirt!
  mpoload 4186
  mpdrop metallica
endif
~
>fight_prog 11~
mpecho Terra summons an esper to assist her!
mpecho Tritoch appears!
mpechoat $n Tritoch unleashes his Tri-Elemental assault on you!
mpechoaround $n Tritoch unleashes his Tri-Elemental assault on $n!
mpscast 70 'fireball' $n
mpscast 90 'lightning bolt' $n
mpscast 115 'cone of cold' $n
~
>fight_prog 12~
mpecho Terra summons an esper to assist her!
mpecho Terrato appears!
mpecho Terrato causes the ground to heave and hurl!
mpscast 1000 'earthquake'
~
>fight_prog 14~
mpecho Terra summons an esper to assist her!
mpecho Seraphim appears!
mpecho Seraphim enfolds Terra in her arms, and a golden light flows into her.
mpheal 10d199+1000 $i
~
>fight_prog 16~
mpecho Terra summons an esper to assist her!
mpecho Bahamut appears!
mpecho A sun flare rages out of Bahamut's mouth!
mpdamage 600 all
mpat 19617 mpecho A great roar echoes from the top of the tower!
mpat 19617 mpecho Blue flames leap down the stairs, turning the room into an oven!
mpat 19617 mpdamage 50 all fire
~
>fight_prog 20~
mpecho Terra summons an esper to assist her!
mpecho Maduin appears!
mpechoat $n A swarm of magic flies from Maduin's fingers towards you!
mpechoaround $n A swarm of magic flies from Madiuin's fingers towards $n!
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
mpscast 25 'magic missile' $n
~
>fight_prog 1~
mpecho Terra summons Bahamut-Zero!!!
mpecho A terrifying shadow passes over the tower...
mpecho Suddenly a column of light as wide as the tower itself strikes from above,
mpecho rocking the Tower of Wishes to its foundation!  The pain is unbearable.
mpdamage 1000 all
mpdamage 600 $n
mpdamage 200 $r
mpat 19617 mpecho Suddenly a column of light as wide as the tower itself strikes from above,
mpat 19617 mpecho rocking the Tower of Wishes to its foundation!  The pain is excruciating.
mpat 19617 mpdamage 100 all
mpat 19616 mpecho A bright light is accompanyed by a great crashing noise from the north.
mpat 19615 mpecho A great crashing noise comes from the north.
mpat 19614 mpecho A terrifying shadow passes over Mysidia...
mpat 19614 mpecho Suddenly an incredible column of light strikes the Tower of Wishes!
mpat 19613 mpecho A great crashing noise comes from the northwest.
mpat 19612 mpecho A terrifying shadow passes over Mysidia...
mpat 19612 mpecho Suddenly an incredible column of light strikes the Tower of Wishes!
mpat 19611 mpecho A terrifying shadow passes over Mysidia...
mpat 19611 mpecho Suddenly an incredible column of light strikes the Tower of Wishes!
mpat 19601 mpecho A terrifying shadow passes over Mysidia...
mpat 19601 mpecho Suddenly an incredible column of light strikes the Tower of Wishes!
mpat 19607 mpecho A terrifying shadow passes over Mysidia...
mpat 19607 mpecho Suddenly an incredible column of light strikes the Tower of Wishes!
mpat 19602 mpecho A terrifying shadow passes over Mysidia...
mpat 19602 mpecho Suddenly an incredible column of light strikes the Tower of Wishes!
mpat 19606 mpecho A great crashing noise comes from the north.
mpat 19605 mpecho A great crashing noise comes from the north.
mpat 19604 mpecho A great crashing noise comes from the north.
mpat 19603 mpecho A great crashing noise comes from the north.
mpat 19609 mpecho A great crashing noise comes from the north.
mpat 19608 mpecho A great crashing noise comes from the north.
mpat 19610 mpecho A great crashing noise comes from the north.
~
|
#19618
>fight_prog 33~
mpcast 30 'fireball' $n
~
>fight_prog 50~
mpcast 30 'lightning bolt' $n
~
|
$
