#22529
>rand_prog  20~
   mpecho A bad-tempered camel spits in your face.
~
|
#22552
>commandtrap_prog  list~
if cansee($n)
tell $n Sorry, I'm all out of rugs..But I've got a nice Joint instead!
else
say I don't deal with invisible people.
endif
~
>commandtrap_prog  buy~
if cansee($n)
tell $n Type "maryjane" for a J...too many Cops around to do it this way!
else
say I don't deal with invisibile people.
endif
~
>keyword_prog  maryjane~
if cansee($n)
MPOLOAD 22560
give joint $n
tell $n This one's on me bud!
else
say I don't deal with invisibile people.
endif
~
|
#22584
>commandtrap_prog  up~
if class($n) == 5
 mpechoat $n $i smiles at you and steps aside to let you through.
 mpechoaround $n $i smiles at $n and steps aside to let $m through.
 mptransfer $n 22583
 mpat $n mpforce $n look
else
 mpechoat $n $i humiliates you and blocks your way.
 mpechoaround $n $i humiliates $n and blocks $s way.
endif
~
|
#22586
>commandtrap_prog  up~
if class($n) == 7
 mpechoat $n $i smiles at you and steps aside to let you through.
 mpechoaround $n $i smiles at $n and steps aside to let $m through.
 mptransfer $n 22585
 mpat $n mpforce $n look
else
 mpechoat $n $i humiliates you and blocks your way.
 mpechoaround $n $i humiliates $n and blocks $s way.
endif
~
|
#22587
>commandtrap_prog  up~
if class($n) == 3
 mpechoat $n $i smiles at you and steps aside to let you through.
 mpechoaround $n $i smiles at $n and steps aside to let $m through.
 mptransfer $n 22586
 mpat $n mpforce $n look
else
 mpechoat $n $i humiliates you and blocks your way.
 mpechoaround $n $i humiliates $n and blocks $s way.
endif
~
|
#22588
>commandtrap_prog  up~
if class($n) == 2
 mpechoat $n $i smiles at you and steps aside to let you through.
 mpechoaround $n $i smiles at $n and steps aside to let $m through.
 mptransfer $n 22587
 mpat $n mpforce $n look
else
 mpechoat $n $i humiliates you and blocks your way.
 mpechoaround $n $i humiliates $n and blocks $s way.
endif
~
|
#22589
>commandtrap_prog  up~
if class($n) == 1
 mpechoat $n $i smiles at you and steps aside to let you through.
 mpechoaround $n $i smiles at $n and steps aside to let $m through.
 mptransfer $n 22588
 mpat $n mpforce $n look
else
 mpechoat $n $i humiliates you and blocks your way.
 mpechoaround $n $i humiliates $n and blocks $s way.
endif
~
|
#22590
>commandtrap_prog  up~
if class($n) == 4
 mpechoat $n $i smiles at you and steps aside to let you through.
 mpechoaround $n $i smiles at $n and steps aside to let $m through.
 mptransfer $n 22589
 mpat $n mpforce $n look
else
 mpechoat $n $i humiliates you and blocks your way.
 mpechoaround $n $i humiliates $n and blocks $s way.
endif
~
|
$
