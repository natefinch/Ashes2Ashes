#1701
>bribe_prog  500000~
  mpju gold
  mpol 1706
  mpat 1706 unlock gate
  mpju key
~
>bribe_prog  300000~
mpjunk gold
if level($n)>=40
   cough
   twiddle
else
   mpoload 1706
   mpat 1706 unlock gate
   mpjunk key
endif
~
>bribe_prog  100000~
mpjunk gold
if level($n)>=30
   cough
   twiddle
else
   mpoload 1706
   mpat 1706 unlock gate
   mpjunk key
endif
~
>bribe_prog  10000~
mpjunk gold
if level($n)>=15
   cough
   twiddle
   puke
else
   mpoload 1706
   mpat 1706 unlock gate
   mpjunk key
endif
~
>bribe_prog  1~
cough
twiddle
mpjunk gold
~
|
#1708
>greet_prog  100~
if ispc($n)
  if isimmort($n)
    bow $n
  else
    mpkill $n
  endif
endif
~
|
#1711
>fight_prog  20~
mpcast 45 'fireball' $r
~
|
#1712
>death_prog  100~
  mpol 1700
  mpat 1762 unlock door
  mpju key
~
>fight_prog  10~
  mpecho A bloodguard warrior hears the commotion and joins the fray!
  mpmload 1709
  mpforce bloodguard kill $r
~
|
#1714
>fight_prog  20~
mpcast 50 'fireball' $r
~
|
#1715
>death_prog  100~
mpecho The battered and bruised body of Lord Glum begins to swell and expand.
mpecho Suddenly, it explodes in a fine red mist covering you in blood. In
mpecho place of Lord Glum is a hideous demon who grins and attacks!!!
mpmload 1716
mpforce demon kill $n
mpgoto 1763
~
|
#1717
>fight_prog  10~
mpecho A bloodguard warrior hears the commotion and joins the fray!
mpmload 1709
mpforce bloodguard kill $r
~
|
#1719
>fight_prog  20~
mpcast 75 'fireball' $r
~
>fight_prog  20~
mpcast 75 'sleep' $r
~
>fight_prog  5~
mpecho A priest of chaos hears the commotion and joins the fray!
mpmload 1718
mpforce priest kill $r
~
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
if isimmort($n)
   say Oh, never mind
else
   mpkill $n
endif
~
|
$
