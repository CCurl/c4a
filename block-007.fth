block #007 - some simple benchmarks

find mil loaded?
mil(a--b)1000 dup * * ;
bm(n--)timer swap for next timer swap - . ;
bb(--)1000 mil bm ;

Utility words
t0(n w--)>r <# r> 1- for # next #s #> ztype ;
.NWB(num width base--)base@ >t base! t0 t> base! ;
.hex  (n--) 2 #16 .NWB ;
.hex4 (n--) 4 #16 .NWB ;
.bin  (n--) 8  #2 .NWB ;
.bin16(n--)16  #2 .NWB ;


