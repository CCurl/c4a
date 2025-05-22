block #003 - strings

find s-scatn loaded?

100 var p1   100 var p2
s-trunc(str--str)0 over c! ;
s-end(str--end)dup s-len + ;
s-catc(dst c--dst)over s-end w! ;
s-scat(str dst--dst)swap s-cat ;
s-scatc(c dst--dst)swap s-catc ;
s-catn(dst n--dst)<# #s #> s-cat ;
s-scatn(n dst--dst)swap s-catn ;












