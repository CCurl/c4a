block #006 - some utility words

find dump loaded?

t0(num width--)>r <# r> 1- for # next #s #> ztype ;
.2(n--)#2 t0 ; .3(n--)#3 t0 ; .4(n--)#4 t0 ;
.NWB(num width base--)base @ >t base ! t0 t> base ! ;
.hex(n--)#2 #16 .NWB ; .hex4 (n--) #4 #16 .NWB ; .hex8#8 #16 .NWB ;
.bin(n--)#8  #2 .NWB ; .bin16(n--)#16  #2 .NWB ;
aemit(ch--)dup #32 < over #126 > orifdrop '.'thenemit ;
t0(addr--)>a $10 for @a+ aemit next adrop ;
dump(addr n--)swap >a 0 >t for
     t@+if0a@ ." %n%x: "then@a+ .hex space
     t@ $10 =if0 t! space space a@ $10 - t0then
   next adrop tdrop ;
align(a1--a2)#4 over #3 and - #3 and + ;
val seed  (val) t1  seed!(n--)t1 ! ; timer seed!
rand(--n)seed dup 13 << xor dup 17 >> xor dup 5 << xor dup seed! ;
rand-mod(n--m)rand abs swap mod ;

tt0 a! begin a@+ drop 10000 rand-mod while a@ . ;



