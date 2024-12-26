block #007 - some utility words

find dump loaded?
t0(num width--)>r <# r> 1- for # next #s #> ztype ;
.NWB(num width base--)base@ >t base! t0 t> base! ;
.hex  (n--) 2 #16 .NWB ; .hex4 (n--) 4 #16 .NWB ;
.bin  (n--) 8  #2 .NWB ; .bin16(n--)16  #2 .NWB ;

aemit(ch--)dup 32 < if drop '.' then emit ;
t0(addr--) $10 for dup c@ aemit 1+ next drop ;
dump(addr n--)swap >a 0 >t for
   t@+ if0 cr a@ .hex ':' emit space then @a+ .hex space
   t@ $10 = if 0 t! a@ $10 - space space t0 then
   next atdrop ;


