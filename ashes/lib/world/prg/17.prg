#1701
>bribe_prog  500000~
  mpju gold




>bribe_prog  300000~
mpjunk gold









>bribe_prog  100000~
mpjunk gold









>bribe_prog  10000~
mpjunk gold










>bribe_prog  1~
cough



|
#1708
>greet_prog  100~
if ispc($n)







|
#1711
>fight_prog  20~
mpcast 45 'fireball' $r

|
#1712
>death_prog  100~
  mpol 1700



>fight_prog  10~
  mpecho A bloodguard warrior hears the commotion and joins the fray!



|
#1714
>fight_prog  20~
mpcast 50 'fireball' $r

|
#1715
>death_prog  100~
mpecho The battered and bruised body of Lord Glum begins to swell and expand.






|
#1717
>fight_prog  10~
mpecho A bloodguard warrior hears the commotion and joins the fray!



|
#1719
>fight_prog  20~
mpcast 75 'fireball' $r

>fight_prog  20~
mpcast 75 'sleep' $r

>fight_prog  5~
mpecho A priest of chaos hears the commotion and joins the fray!



|
#1720
>fight_prog  35~
  if ispc($n)
    say Blasphemer!  Get thee from my sight!
    mpechoat $n $i points at you and incants a spell.
    mpechoaround $n $i points at $n and incants a spell.
    mpechoaround $n $n disappears.
    mptransfer $n 3072
  else
    redirect $r
  endif
~
>greet_prog  10~
say Who dares enter my throne room without my leave???






|
$