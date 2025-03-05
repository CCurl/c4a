block #004 - blocks
find blk-clr loaded?
t0(n--n)dup block-dirty ;
t1(n--a)block-addr ; inline
blk-clr(n--)t0 t1 block-sz 0 fill ;
blk-cpy(f t--)t0
    block-addr swap block-addr swap block-sz cmove ;
blk-mv(f t--)over t0 swap blk-cpy blk-clr ;








