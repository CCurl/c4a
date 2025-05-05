block #004 - blocks

find blk-clr loaded?

t6(n--n)dup block-dirty ;
t7(n--a)block-addr ;
blk-clr(n--)t6 t7 block-sz 0 fill ;
blk-cpy(f t--)t6 t7 >r t7 r> block-sz cmove ;
blk-mv(f t--)over t6 swap blk-cpy blk-clr ;















