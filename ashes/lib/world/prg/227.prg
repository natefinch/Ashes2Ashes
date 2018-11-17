#22705
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22709
>fight_prog  30~
mpecho The Grey Seer waves his hand and summons forth a chaos spawn!!!
mpmload 22752
mpforce chaos kill $r
~
|
#22710
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22711
>fight_prog  30~
mpecho The Grey Seer waves his hand and summons forth a chaos spawn!!!
mpmload 22752
mpforce chaos kill $r
~
|
#22715
>death_prog  100~
rem key
give key albino
mpforce albino wear key
rem key
unlock door north
unlock door south
mpjunk key
mpjunk key
~
|
#22716
>all_greet_prog  100~
if isnpc($n)
  unlock gates
  open gates
  mpforce $n west
  close gates
  lock gates
else
  say Back to the holding rooms, scum-scum!!!
endif
~
|
#22718
>commandtrap_prog  east~
if level($n)<50
  say You are not experienced enough to brave the Circle of Assassins!!!
  say Come back after a few more levels...
else
  if level($n) == 50
    say The Circle of Assassins welcomes you...
    mpecho $n sets foot on the path to certain death.
    mptrans $n 22762
  else
    mptrans $n 22762
    tell $n You must enjoy watching morts die.
  endif
endif
~
|
#22722
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22723
>death_prog  100~
mpat 22873 mpmload 22723
~
|
#22724
>death_prog  100~
mpat 22875 mpmload 22724
~
|
#22725
>death_prog  100~
mpat 22874 mpmload 22725
~
|
#22729
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
>all_greet_prog  100~
if isimmort($n)
  bow $n
else
  mpecho Throt gets a wicked gleam in his eye as he points a finger at you.
  say Attack my pet-pets!!!
  mpforce rat kill $r
  mpforce 2.rat kill $r
endif
~
|
#22733
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22734
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22735
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22736
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22737
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22738
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22739
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22740
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22741
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22742
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22743
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22744
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22745
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
|
#22746
>greet_prog  100~
say What do you seek from me?~
>speech_prog  p blessing~
mpoload 22721
unlock chest
mpjun key
~
|
#22753
>death_prog  100~
rem key
give key albino
mpforce albino wear key
rem key
unlock door north
unlock door south
mpjunk key
mpjunk key
~
|
#22754
>death_prog  100~
rem key
give key albino
mpforce albino wear key
rem key
unlock door north
unlock door south
mpjunk key
mpjunk key
~
|
#22755
>death_prog  100~
rem key
give key albino
mpforce albino wear key
rem key
unlock door north
unlock door south
mpjunk key
mpjunk key
~
|
#22756
>death_prog  100~
rem key
give key albino
mpforce albino wear key
rem key
unlock door north
unlock door south
mpjunk key
mpjunk key
~
|
#22757
>death_prog  100~
rem key
give key albino
mpforce albino wear key
rem key
unlock door north
unlock door south
mpjunk key
mpjunk key
~
|
#22854
>fight_prog  20~
mpecho Suddenly you feel the very presence of evil in the room.
mpecho The Horned Rat appears out of thin air!!
mpecho The massive voice of the Horned Rat booms out,
mpecho I WILL NOT TOLERATE THESE ACTIONS AGAINST THOSE UPON WHOM
mpecho I HAVE BESTOWED MY BLESSING!!!
if rand(30)
  mpechoaround $n The Horned Rat grins evilly as he explodes a blast of
  mpechoaround $n pure warpfire in the face of $n!!!
  mpechoat $n The Horned Rat grins evilly as he explodes a blast of pure
  mpechoat warpfire in your face!!!
  if rand(20)
    mpechoaround $n The Horned Rat laughs maniacally as his second ball
    mpechoaround $n of warpfile explodes around $n!!!
    mpechoat $n The Horned Rat laughs maniacally as his second ball of
    mpechoat $n warpfire explodes around $n!!!
    if rand(10)
      mpechoaround $n The Horned Rat cackles as his third blast of
      mpechoaround $n warpfire decimates $n!!!
      mpechoat $n The Horned Rat cackles as his third blast of warpfire
      mpechoat $n decimates $n!!!
    endif
  endif
else
 if rand(20)
   mpechoat $n The Horned Rat waves his arms at you in a magical incantation.
   mpechoaround $n The Horned Rat waves his arms at $n in a magical chant.
   mpechoaround $n $n dissappears.
   mptransfer $n 3001
   mpat $n mpforce $n look
 else
   mpecho The Horned Rat waves his hands at $i in a magical incantation.
   mptrans hornedrat
   mpforce hornedrat mpcast 'restore' $i
   mpat 22899 mptrans hornedrat
 endif
endif
~
>give_prog  warptoken warp token~
mpoload 22721
unlock glass north
open glass north
north
mpjunk key
~
|
$
