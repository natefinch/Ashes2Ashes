#8450
>fight_prog  40~
mpat 8555 mpecho Drizzt says 'come to me'
mpkill $r
mpkill $n
~
>fight_prog  100~
mpkill $r
mpkill $n
~
>greet_prog  70~
if class($n) == 5
	mpachoat $n $i looks at you intently, then smiles.
	mpechoaround $n $i looks at $n intently then smiles.
	mpecho $i reaches into his tunic and pulls out a carving of a
	mpecho unicorn's head...the symbol of Mielikki, godess of the
	mpecho forest and partoness of rangers.
	mpachoat $n You know you are in the presence of a fellow ranger.
endif
~
|
#8451
>act_prog  Drizzt says 'come to me'~
mpgoto 8464
mpecho Drizzt drops a small figurine on the ground and calls
mpecho out, 'Guenhwyvar'
mpecho A swirling mist rises up and a black panther emerges.
assist drizzt
~
>fight_prog  20~
rescue drizzt
~
>rand_prog  2~
if inroom($i) ==8464
	mpecho $i has spent too long on this plane, and must rest.
	mpecho Drizzt dismisses $i.
	mpecho $i dissolves into a black mist and vanishes.
	mpgoto 8555
	mppurge $i
else
	mpgoto 8555
	mppurge $i
endif
~
|
#8464
>give_prog  piece tarrasque carapace~
mpjunk carapace
mpoload 8601
mpecho $i eyes the crude carapace critically and throws it
mpecho into the forge. Then he takes a block of Mithril and
mpecho throws that in too.
mpecho $i works the bellows until the flames are white-hot,
mpecho then begins to work on his masterpiece, chanting out
mpecho prayers to Moradin to bless this most difficult of
mpecho substances to forge.
mpecho With divine aid, $i finishes his work in a matter of minutes.
give shield $n
say This'll stop anything short of Clanggedin's warhammer.
smile $n
~
|
$
