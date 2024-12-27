block #005 - screen words
find cls loaded?
cur-on(--)." %e[?25h" ;       cur-off(--)." %e[?25l" ;
cur-block(--)." %e[2 q" ;     cur-bar(--)." %e[5 q" ;
->cr(r c--)." %e[%d;%dH" ;    ->rc(c r--) swap ->cr ;
cls(--)." %e[2J" 1 1 ->rc ;   clr-eol(--)." %e[0K" ;
color(bg fg--)." %e[%d;%dm" ; fg(fg--)." %e[38;5;%dm" ;

white255 fg ; red   203 fg ;
green 40 fg ; yellow226 fg ;
blue  63 fg ; purple201 fg ;
cyan 117 fg ; grey  250 fg ;

.color(c--c)dup dup fg ." color-%d%n" ;
colors(f t--)over - 1+ for .color 1+ next drop white ;

