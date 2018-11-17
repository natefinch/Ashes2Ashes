#11902
>fight_prog 25~
mpecho $I waves $l hands in the air and a great blast of fire washes over you!
mpdamage 250d5+25 all
~
>fight_prog 100~
if class($r) == 0
or class($r) == 1
or class($r) == 4
or class($r) == 6
  mpechoat $r $I laughs at your puny spells. With a mighty backhand $j
  mpechoat $r sends you flying into the wall. You feel scorched and crushed.
  mpechoaround $r $I laughs at $r and sends $K flying into the wall with a mighty backhand.
  mpunaffect $r
  mpdamage 12d50+100 $r
endif
~
|
$
