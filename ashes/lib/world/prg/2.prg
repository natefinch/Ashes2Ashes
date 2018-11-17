#211
>fight_prog  50~
    mpcast 90 'fireball' $r
    if rand(35)
      mpcast 90 'fireball' $r
      if rand(10)
        mpcast 90 'fireball' $r
      endif
    endif
~
>fight_prog  9~
  mptrans kintaroclone
  mpecho Shang Tsung assumes the form of Kintaro!
  mpforce kintaroclone mpkill $n
  mpforce kintaroclone mptrans shang 290
~
>fight_prog  9~
  mptrans goroclone
  mpecho Shang Tsung assumes the form of Goro!
  mpforce goroclone mpkill $n
  mpforce goroclone mptrans shang 290
~
>fight_prog  9~
  mptrans raydenclone
  mpecho Shang Tsung assumes the form of Rayden!
  mpforce raydenclone mpkill $n
  mpforce raydenclone mptrans shang 290
~
>fight_prog  9~
  mptrans kitanaclone
  mpecho Shang Tsung assumes the form of Kitana!
  mpforce kitanaclone mpkill $n
  mpforce kitanaclone mptrans shang 290
~
>fight_prog  9~
  mptrans jaxclone
  mpecho Shang Tsung assumes the form of Jax!
  mpforce jaxclone mpkill $n
  mpforce jaxclone mptrans shang 290
~
>fight_prog  9~
  mptrans subclone
  mpecho Shang Tsung assumes the form of Sub-Zero!
  mpforce subclone mpkill $n
  mpforce subclone mptrans shang 290
~
>fight_prog  9~
  mptrans scorpionclone
  mpecho Shang Tsung assumes the form of Scorpion!
  mpforce scorpionclone mpkill $n
  mpforce scorpionclone mptrans shang 290
~
>fight_prog  9~
  mptrans reptileclone
  mpecho Shang Tsung assumes the form of Reptile!
  mpforce reptileclone mpkill $n
  mpforce reptileclone mptrans shang 290
~
>fight_prog  9~
  mptrans cageclone
  mpecho Shang Tsung assumes the form of Johhny Cage!
  mpforce cageclone mpkill $n
  mpforce cageclone mptrans shang 290
~
>fight_prog  9~
  mptrans barakaclone
  mpecho Shang Tsung assumes the form of Baraka!
  mpforce barakaclone mpkill $n
  mpforce barakaclone mptrans shang 290
~
>fight_prog  9~
  mptrans kungclone
  mpecho Shang Tsung assumes the form of Kung Lao!
  mpforce kungclone mpkill $n
  mpforce kungclone mptrans shang 290
~
>fight_prog  9~
  mptrans liuclone
  mpecho Shang Tsung assumes the form of Liu Kang!
  mpforce liuclone mpkill $n
  mpforce liuclone mptrans shang 290
~
>fight_prog  9~
  mptrans mileenaclone
  mpecho Shang Tsung assumes the form of Mileena!
  mpforce mileenaclone mpkill $n
  mpforce mileenaclone mptrans shang 290
~
>fight_prog  50~
  mpecho Shang Tsung vanishes!!!
  mpgoto 290
  mpechoat god DO_VANISH
~
>act_prog  100 RESUME~
  mpecho Shang Tsung attacks $r!
  mpkill $r
~ ~
|
#245
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#246
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#247
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#248
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#249
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#250
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#251
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#252
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#253
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#254
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#255
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#256
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#257
>fight_prog  25~
  mpecho Shang Tsung's form wavers!  
  mpecho Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
>death_prog  100~
  mpecho Shang Tsung's form wavers and appears unstable!
  mpecho As $i appears to die, Shang Tsung returns to his natural form!
  mptrans shang
  give all shang
  mpforce shang mpkill $n
  mpgoto 290
~
|
#258
>act_prog  100 DO_VANISH~
  mptrans shang 270
  mpat 270 mpecho Shang Tsung appears before you!
  mpat 270 mpechoat shang RESUME
~
|
$
