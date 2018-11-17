#500
>greet_prog  25~
   say Wait, your not suppost to be here.
~
>death_prog  100~
   say Dying for my country, there is no greater honor.
~
>rand_prog  25~
   say Die FOULISH Intruder!
~
|
#501
>greet_prog  25~
   say Ack, how did you get in here
~
>rand_prog  25~
   say Ok, guess I will kill you know.
~
>death_prog  100~
   say Dying for my country, their is no greater honor.                           
~
|
#504
>greet_prog  25~
   say How did you get in, ANSWER ME!!
~
>rand_prog  25~
   say NO Answer huh, ok time to DIE IDIOT!
~
>death_prog  100~
   say ACK, I don't want to die, please don't make me mommy!!
~
|
#505
>greet_prog  25~
   say Welcome my children to evil!!
~
>rand_prog  25~
   say Come stare deeply into my eyes to see your destruction!
~
>death_prog  100~
   say Freedom from the Knave Dragonsi, THANK YOU!!
~
|
#506
>greet_prog  25~
   say Welcome to death!                       
~
>rand_prog  25~
   say I am the find of secrets even in death
~
>death_prog  100~
   say Not possible a puny thing like you killed me!
~
|
#507
>greet_prog  15~
   say What do you want now? I don't have all day.
~
>rand_prog  15~
   say Come on Hurry up, you are wasting valuable time! 
~
>death_prog  100~
   say I am coming to join you old friend!
~
|
#508
>greet_prog  15~
   say Let me tell you future my friend.
~
>rand_prog  15~
   say I see grave danger ahead for you.
~
>death_prog  100~
   say Ack, why didn't I see this?
~
|
#509
>greet_prog  15~
   say HA, I can read your mind
~
>rand_prog  15~
   say Don't even think you couldn't kill me!
~
>death_prog  100~
   say HA, I have told my companions!! They will avenge me!!
~
|
#510
>greet_prog  15~
   say I am a total screw up.
~
>rand_prog  15~
   say I can't do anything right!
~
>death_prog  100~
   say Well at least I can do death right.
~ ~
|
#511
>greet_prog  25~
   say Come now, tell me the REAL truth.
~
>rand_prog  25~
   say Your lying to me, WHY?
~
>death_prog  100~
   say Ack, you weren't lying!!
~
|
#512
>greet_prog  25~
   say Welcome Comrade
~
>rand_prog  25~
   say You have seen to much, you must die
~
>death_prog  100~
   say Fool, do you know the consequences of my death??
~
|
#513
>greet_prog  25~
   say Welcome comrade
~
>rand_prog  25~
   say God I hope I never have to rule this place 
~
>death_prog  100~
   say Struck down in my prime, a CRUEL FATE!
~
|
#514
>greet_prog  25~
   say Welcome Comrade 
~
>rand_prog  25~
   say It will be mine, all of it, do you hear me???
~
>death_prog  100~
   say NO, I REFUSE to die, I am all powerful
~
|
#524
>rand_prog  25~
   say Ah, more things to vent my rage on!! 
   cackle
~
>death_prog  100~
   say NO, not by a puny WEAKLING like you!!
~
|
#525
>rand_prog  25~
   say Please I am begging you to help me escape.
~
>death_prog  100~
   say Finally freedom at last from Thibor!!
~
|
#526
>rand_prog  25~
   say AH foolish mortals for me to play with.
~
>death_prog  100~
   say ACK!!! My farther will avenge me!!!
~
|
#528
>rand_prog  25~
   say Foolish beings, I am not as easy as my brothers!!
~
>death_prog  100~
   say Even from death I will avenge my death!!
~
|
#530
>rand_prog  25~
   say Ah, did you know I built this?
~
>death_prog  100~
   say My creation will strike you down as my revenge!!
~
|
#531
>rand_prog  25~
   say Ah, test subjects!!
~
>death_prog  100~~
   say NO, I haven't finished my tests yet!!
~
|
#534
>fight_prog  20~
   say Take this you WEAKLING!
   mpcast 50 'fireball' $n
~
>death_prog  100~
mpecho Harry's body shrivels and vanishes!!!!!!
mpecho There is a horrid swirl around what is left of his corpse...
mpecho A shade has formed and is coming toward YOU!!!
mpmload 542
mpforce spirit kill $n
mpgoto 585
mpforce harry get all corpse
~
|
#538
>fight_prog  15~
   say Take this you WIMP!!  
   mpcast 50 'fireball' %n
~
>fight_prog  15~
   mpecho Lady Karen says 'HELP ME HARRY KEOGH!'
~
|
#539
>act_prog  Lady Karen says 'HELP ME HARRY KEOGH!'~
   assist karen
~
|
$
