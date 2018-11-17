#22610
>commandtrap_prog  north~
mpechoat $n $i humiliates you, and blocks your way.
mpechoaround $n $i humiliates $n, and blocks $s way.
~
|
#22611
>commandtrap_prog  north~
mpechoat $n $i humiliates you, and blocks your way.
mpechoaround $n $i humiliates $n, and blocks $s way.
~
|
#22639
>rand_prog  25~
   mpecho Jasmine looks out at the stars, dreamily humming "A Whole New World".
~
|
#22647
>rand_prog  25~
   mpecho Aladdin stares off into the sky, day-dreaming about Jasmine.
~
|
#22648
>rand_prog  25~
   if rand(60)
      mpecho Abu chatters a long string of something - you feel scolded.
   else
      mpecho Abu tips his hat happily to you.
   endif
~
>rand_prog  25~
   if rand(40)
      if isfollow($i)
         mpecho Aladdin pats Abu lightly on the head, smiling.
      endif
   endif
~
|
#22652
>rand_prog  25~
   if rand(75)
      mpecho Rajah purrrs contentedly.
   else
      mpecho Rajah suddenly deafens you with an ear-splitting ROAR.
   endif
~
>rand_prog  25~
   if rand(40)
      if isfollow($i)
         mpecho Jasmine reaches down and scratches behind Rajah's ears.
      endif
   endif
~
|
#22661
>rand_prog  25~
   mpecho Jafar mutters something about ruling the world.
~
>rand_prog  100~
if isfight($i)
if rand(20)
mpcast 50 'vanish' jafar
endif
endif
~
|
#22662
>rand_prog  25~
   if rand(60)
      mpecho Iago grumbles something under his breath.
   else
      mpecho Iago squawks loudly, hurting your ears.
   endif
~
>rand_prog  25~
   if rand(40)
      if isfollow($i)
         mpecho Jafar slaps Iago forcefully, resulting in flying feathers and some angry sputtering.
      endif
   endif
~
|
$
