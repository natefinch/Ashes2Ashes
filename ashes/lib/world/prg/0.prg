#5
>greet_prog  100~
  mpol 3010















































































    if class($n) == 7
      mpol 24
      mpol 25
      mpol 26
      give all $n
      mpfo $n hold coal
      mpfo $n wear all
    endif
  if class($n) == 8
    mpol 78
    mpol 86
    mpol 77
    mpol 76
    give all $n
    mpfo $n wear all
    mpfo $n wield scimitar
  endif
  tell $n I have given you your starting equipment.



>fight_prog  100~
  mpcast 50 'restore' $i




|
$