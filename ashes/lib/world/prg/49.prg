#4930
>fight_prog 100~
mpecho An alien says, "You have been chosen you must go."
mpechoat $n The claw moves above you and comes down and grasps you.
mpechoaround $n The claw moves above you and comed down and grasps $n.
mpechoat $n The claw grabs you and dumps you into the deposit.
mpechoat $n Before you can do anything you are picked up by Sid.
mpechoat $n Sid gives you a good look over and says, "Let's go home and play."
mpechoat $n Sid puts you in his backpack.
mpstrans $n 4925
~
|
#4946
>all_greet_prog 100~
if ispc($n)
  say Welcome to the party, here are some goodies. Go on in, and behave.
  mpcast 102 'sanctuary' $n
  mpoload 4924
  mpoload 4918
  mpoload 4919
  mpoload 4920
  put all.potion bag
  give bag $n
  junk bag
endif
~
>fight_prog 100~
mpcalm
~
|
#4949
>commandtrap_prog north~
if level($n)<10
  mptransfer $n 4900
  mpecho $n leaves north.
else
  say You can't enter this area. Only a true newbie can.
  say You have to be under level 10.
endif
~
|
#4950
>all_greet_prog 100~
if ispc($n)
mpcast 50 'heal' $n
endif
~
|
$
