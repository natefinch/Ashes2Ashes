#8450
>fight_prog  40~
mpat 8555 mpecho Drizzt says 'come to me'



>fight_prog  100~
mpkill $r


>greet_prog  70~
if class($n) == 5








|
#8451
>act_prog  Drizzt says 'come to me'~
mpgoto 8464





>fight_prog  20~
rescue drizzt

>rand_prog  2~
if inroom($i) ==8464










|
#8464
>give_prog  piece tarrasque carapace~
mpjunk carapace













|
$