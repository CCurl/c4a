block #000                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     boot block                                                                                                                     3 load  5 load  7 load                                                                                                         rbforget 1 load ;       edblock @ edit ;                    lsz" ls -l" system ;    lgz" lazygit" system ;                                                                              ." c4a - " green .version white                                2 block !                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      - these are for executing from the editor -                                                                                     ls                                                              lg                                                              0 flush                                                         blocks                                                          z" ccc dev" system                                              z" git pull" system                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            block #003 - strings                                           find fill loaded?                                              p1vhere $100 + ;   p2p1 $100 + ;                            fill(to num char--)swap ?dup                                    if>t swap t> for over over c! 1+ nextthen2drop ;         cmove(src dst n--)>r >t >a                                      r> ?dupiffor @a+ !t+ nextthenatdrop ;                   cmove>(src dst n--)>r r@ + >t  r@ + >a                          r> ?dupif1+ for @a- !t- nextthenatdrop ;                s-end(str--end)dup s-len + ;                                 s-cat(dst src--dst)over s-end swap s-cpy drop ;              s-catc(dst c--dst)over s-end w! ;  s-scatcswap s-catc ;    s-catn(dst n--dst)<# #s #> s-cat ; s-scatnswap s-catn ;                                                                                                                                                                                                    block #004 - doc for string words                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              block #005 - screen words                                      find cls loaded?                                               cur-on(--)." %e[?25h" ;       cur-off(--)." %e[?25l" ;    cur-block(--)." %e[2 q" ;     cur-bar(--)." %e[5 q" ;     ->cr(r c--)." %e[%d;%dH" ;    ->rc(c r--) swap ->cr ;     cls(--)." %e[2J" 1 1 ->rc ;   clr-eol(--)." %e[0K" ;      color(bg fg--)." %e[%d;%dm" ; fg(fg--)." %e[38;5;%dm" ;                                                                   white255 fg ;   red   203 fg ;                              green 40 fg ;   yellow226 fg ;                              blue  63 fg ;   purple201 fg ;                              cyan 117 fg ;   grey  250 fg ;                                                                                              .color(c--c)dup dup fg ." color-%d%n" ;                      colors(f t--)over - 1+ for .color 1+ next drop white ;                                                                       block #006 - doc for screen words                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              block #007 - some simple benchmarks                                                                                            mil1000 dup * * ;                                             bmtimer swap for next timer swap - . ;                        bb1000 mil bm ;                                                                                                               .N(n w--)>r <# r> 1- for # next #s #> ztype ;                .hexbase@ swap hex 2 .N base! ;                               .hex4base@ swap hex 4 .N base! ;                              .binbase@ swap binary 8 .N base! ;                            .bin16base@ swap binary 16 .N base! ;                                                                                                                                                                                                                                                                                                                                                         block #008 - doc for benchmarks                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                