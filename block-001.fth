boot block

3 load  4 load  5 load  7 load

rbforget 1 load ;       edblock @ edit ;
lsz" ls -l" system ;    lgz" lazygit" system ;
devz" ccc dev" system ;

[[ tasks-sz 1- for i 1+ del-task next ]]
heretask1." -task1-" yield task1 ;add-task drop
heretask2." -task2-" yield task2 ;add-task drop

." c4a - %Gv" .version white
7 block !

 cr .tasks








