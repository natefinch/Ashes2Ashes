#12813
>all_greet_prog  50~
 if ispc($n)
  if rand(30)
   say $n, did I mention that the Great Skeeve keeps a DRAGON for a pet??
  else
   if rand(30)
    say You know $n, the Great Skeeve has a PERVERT for a house guest!!!
   else
    say I warn you $n, you do NOT want to bother the Great Skeeve!!!
   endif
  endif
 else
  bow $n
 endif
~
|
#12831
>fight_prog  20~
 mpechoat $n $I grabs you by the neck and lifts you off the ground!!!
 mpechoaround $n $I grabs $n by the neck and lifts $m off the ground!!!
 goss That's PERVECT!!! Not PERVERT, $n!!!
~
|
#12832
>all_greet_prog  100~
if isnpc($n)
  say I don't need any more pets, I got Gleep here!!!
  point gleep
  mpecho The Great Skeeve surges with magikal energy as he points at $n.
  mpecho $n has been sent to another dimension, courtesy of Skeeve.
  mppurge $n
else
  mpechoat $n The Great Skeeve smiles as you enter the room.
endif
~
|
$
