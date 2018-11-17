#15800
>commandtrap_prog  north~
if level($n)<21
  mptransfer $n 15801
  mpecho The Forest Ranger tips his hat to $n and welcomes $n 
  mpecho into the forest with a cheery "g'day!"
  tell $n Welcome to the forest!  Feel free to look around,
  tell $n but remember that once you reach level 21 you won't
  tell $n be able to come back.
else
    if isimmort($n)
      tell $n If you want to watch over the newbies, goto 15801.
    else
      mpecho The Forest Ranger blocks your way, and reminds you that only
      mpecho characters under level 21 may pass.
    endif  
endif
~
|
$
