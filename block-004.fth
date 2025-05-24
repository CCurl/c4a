block #004 - blocks

find blk-clr loaded?

blk-clr(n--)  block-addr block-sz 0 fill ;
blk-cpy(f t--)block-addr >r block-addr r> block-sz cmove ;
blk-mv (f t--)over swap blk-cpy blk-clr ;

















