#15149
>all_greet_prog  100~
if isnpc($n)
   mpechoar $n $n is smashed into a bloddy pulp as it is struck by a vehicle.
   mppurge $n
else
    if isimmort($n)
     mpechoat $n This transfers morts to 15103.  Mobprogs can't trans imms.
  else 
     mpechoar $n You see $n slump to the ground behind an obstruction, and disappear
     mpechoar $n from your sight.  There is a sudden wind, then silence.
     mpechoat $n As you stand on the city street, you suddenly feel a THUMP and you are
     mpechoat $n thrown by a sharp impact.  As you land, the world 
     mpechoat $n dissolves into the red light flashing in your eyes.  You suddenlt get the 
     mpechoat $n sensation of being taken elsewhere.  You feel stone beneath you, and as 
     mpechoat $n your vision clears, you see a hammer-headed humanoid with scrawny limbs, 
     mpechoat $n clutching an iron-heeled staff which hums with power.  "Mine!  All Mine! 
     mpechoat $n Droll summons!  Mine!" the scrawny creature screeches.  Suddenly, a deep 
     mpechoat $n omnious voice growls out "Back, Drool Rockworm.  This one is mine."  As 
     mpechoat $n the creature scuttles away, this voice says to you, "Before, others 
     mpechoat $n have disappointed me.  I know that you shall not.  Now, I, Lord Foul the 
     mpechoat $n Despiser, have returned, and theit days shall be numbered but four score 
     mpechoat $n and seven.  Tell them this, and despair with them."  And with that, 
     mpechoat $n the cavern dims around you, and again, you seem to be somewhere else...
     mptran $n 15199
     mpat 15199 mpforce $n sit
     mpat 15199 mptran $n 15103
     mpat 15103 mpechoar $n Suddenly, where before there was nothing, a body lies upon the rock.
     mpat 15103 mpechoar $n Dead or alive, you cannot tell.  Suddenly, as if a door was closed, there
     mpat 15103 mpechoar $n is a deep THUD, and then all is silence.
  endif
endif
~
|
$
