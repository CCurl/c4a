block #007 - some simple benchmarks

mil1000 dup * * ;
bmtimer swap for next timer swap - . ;
bb1000 mil bm ;

.N(n w--)>r <# r> 1- for # next #s #> ztype ;
.hexbase@ swap hex 2 .N base! ;
.hex4base@ swap hex 4 .N base! ;
.binbase@ swap binary 8 .N base! ;
.bin16base@ swap binary 16 .N base! ;





