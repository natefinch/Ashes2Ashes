#7303
>all_greet_prog  100~
if level($n)<6 
  if rand(20)
      mpcast 10 'cure s' $n
   else 
    if rand (20)
      mpcast 10 'armor' $n
   else 
   if rand (20)
    mpcast 10 'bless' $n
 else 
 if rand (20)
    mpcast 25 'aid' $n
 else 
 if rand (20)
    mpcast 25 'sanc' $n
 else 
    mpcast 10 'cure l' $n
 endif
 endif
 endif
 endif
 endif
endif
~
|
#7317
>all_greet_prog  100~
  tell $n Each Person MUST type north to enter this area.
~
>commandtrap_prog  north~
if level($n)<16
  mptransfer $n 7301
  mpecho $n enters the MudSchool.
  mpat $n say Here you go. Please ask a immort if you have any questions
  mpat $n say after you finish going thru the mudschool. You can only
  mpat $n say enter these halls until level 16. So make the most of this
  mpat $n say area while you can still enter it. 
  mpat $n say Thank you,
  mpat $n say The Ashes to Ashes Administration Staff.
else
  if isimmort($n)
    tell $n Goto 7301 if you want to watch over the newbies.
  else
    say You can't enter this area. Only a true newbie can.
    say You have to be UNDER level 16.
    say Thank you,
    say The Ashes to Ashes Administration Staff.
  endif
endif
~
|
$
