#15800
>commandtrap_prog  north~
if level($n)<21






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