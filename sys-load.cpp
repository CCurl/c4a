#include "c4a.h"

#ifndef _SYS_LOAD_
void sys_load() {
    fileLoad("bootstrap.fth");
}
#else
void sys_load() {
    outer(": 0sp  0 (dsp) ! ;");
    outer(": 0rsp 0 (rsp) ! ;");
    outer(": create addword vhere lit, ;");
    outer(": does> (jmp) , r> , ;");
    outer(": hex $10 base ! ;  : binary %10 base ! ;  : decimal #10 base ! ;");
    outer(": rot >r swap r> swap ;  : -rot swap >r swap r> ;");
    outer(": btwi ( n l h--f ) >r over >  swap r> >  or 0= ;");
    outer(": +! tuck  @ + swap ! ;");
    outer(": execute ( a-- ) >r ;");
    outer(": !t- t@- c! ;   #40 var #buf");
    outer(": <# ( n1--n2 ) #buf #39 + >t 0 !t- dup 0 < >a abs ;");
    outer(": #. ( -- )     '.' !t- ;");
    outer(": #n ( n-- )    dup 9 > if 7 + then '0' + !t- ;");
    outer(": #  ( n1--n2 ) base @ /mod swap #n ;");
    outer(": #s ( n-- )    begin # -while ;");
    outer(": #> ( --str )  drop a> if '-' !t- then t> 1+ ;");
    outer(": .version version <# # # #. # # #. #s #> ztype ;");
    outer(": [[ vhere >t here >t 1 state ! ;");
    outer(": ]] (exit) , 0 state ! t@ (here) ! t> >r t> (vhere) ! ; immediate");
    outer("mem-sz ->memory const dict-end");
    #ifndef FILE_NONE
        outerF(": fopen-r ( fn--fh )  z\" %s\" fopen ;", FL_READ);
        outerF(": fopen-w ( fn--fh )  z\" %s\" fopen ;", FL_WRITE);
    #endif
    outer(": fgl last de-sz + (last) ! last w@  (here) ! ;");
    outer("marker");
}
#endif // _SYS_LOAD_
