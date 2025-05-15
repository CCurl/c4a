boot block

3 load  4 load  5 load  6 load

rb forget 1 load ;       ed  block @ edit ;         ed!block ! ;
ls z" ls -l" system ;    lg  z" lazygit" system ;   flflush ;
pwdz" pwd" system ;      sys(a--)dup ." %n%G%S%W%n" system ;
devz" ccc dev" sys ;     ndevz" ccc ndev" sys ;
pl(a--)p1 z" cd " s-cpy s-scat z"  && git pull -p" s-cat sys ;
g1z" \code\mine\c4a" pl ;     g2z" \code\sia" pl ;
g3z" \code\360DB" pl ;        g4z" \code\bwServiceDefinitions" pl ;
g5z" \code\Service" pl ;      g6z" \code\bwPluginModules" pl ;
g7z" \code\NgComponents" pl ;
pull-allg1 g2 g3 g4 g5 g6 g7 ." %YDone.%W" ;

[[ tasks-sz 1- for i 1+ del-task next ]]
heretask1." -task1-" yield task1 ;add-task drop
heretask2." -task2-" yield task2 ;add-task drop

." c4a - %Gv" .version white
1 block !    cr .tasks



