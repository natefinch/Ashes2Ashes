#5
>greet_prog  100~
  mpol 3010
  mpol 3010
  mpol 3010
  mpol 3010
  mpol 3010
  mpol 3032
  put all.bread bag
  mpol 3104
  if class($n) == 0
    mpol 88
    mpol 86
    mpol 6002
    mpol 91
    mpol 82
    give all $n
    mpfo $n wield dirk
    mpfo $n wear all
  else
    if class($n) == 1
      mpol 85
      mpol 6002
      mpol 90
      mpol 81
      give all $n
      mpfo $n wield mace
      mpfo $n wear all
    else
      if class($n) == 2
        mpol 87
        mpol 89
        mpol 6002
        mpol 91
        mpol 83
        give all $n
        mpfo $n wield dagger
        mpfo $n wear all
      endif
    endif
  endif
  if class($n) == 3
    mpol 84
    mpol 6002
    mpol 90
    mpol 80
    give all $n
    mpfo $n wield spear
    mpfo $n wear all
  endif
  if class($n) == 4
    mpol 6002
    mpol 92
    mpol 93
    mpol 94
    mpol 95
    give all $n
    mpfo $n wield lance
    mpfo $n wear all
  endif
  if class($n) ==5
    mpol 12091
    mpol 12092
    mpol 12093
    mpol 12094   
    mpol 12095
    give all $n
    mpfo $n wield sword
    mpfo $n hold dagger
    mpfo $n wear all
    endif
  if class($n) == 6
    mpol 6002
    mpol 12096
    mpol 12097
    mpol 12098
    mpol 12099
    give all $n
    mpfo $n wield tormentor
    mpfo $n hold symbol
    mpfo $n wear all
  endif
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
  tell $n Please type HELP to learn the commands.
  tell $n Type DOWN to begin your adventures.
~
>fight_prog  100~
  mpcast 50 'restore' $i
  mptransfer $n 3001
  mpat $n mpfo $n look
  tell $n Don't EVER try to kill me again!!!
~
|
$
