#include "c4a.h"

#ifndef _SYS_LOAD_
void sys_load() {
    fileLoad("bootstrap.fth");
}
#else
void sys_load() {
    #ifndef FILE_NONE
        outerF(R"(: fopen-r ( fn--fh )  z" %s" fopen ;)", FL_READ);
        outerF(R"(: fopen-w ( fn--fh )  z" %s" fopen ;)", FL_WRITE);
    #endif
    outer(R"(
: 0sp  0 (dsp) ! ;
: 0rsp 0 (rsp) ! ;
: create addword vhere lit, ;
: does> (jmp) , r> , ;
: hex $10 base ! ;  : binary %10 base ! ;  : decimal #10 base ! ;
: rot >r swap r> swap ;  : -rot swap >r swap r> ;
: btwi ( n l h--f ) >r over >  swap r> >  or 0= ;
: execute ( a-- ) >r ;
: !t- t@- c! ;  #40 var #buf
: <# ( n1--n2 ) #buf #39 + >t 0 !t- dup 0 < >a abs ;
: #. ( -- )     '.' !t- ;
: #n ( n-- )    dup 9 > if 7 + then '0' + !t- ;
: #  ( n1--n2 ) base @ /mod swap #n ;
: #s ( n--0 )   begin # -while ;
: #> ( --str )  drop a> if '-' !t- then t> 1+ ;
: .version version <# # # #. # # #. #s #> ztype ;
: [[ vhere >t here >t 1 state ! ;
: ]] (exit) , 0 state ! t@ (here) ! t> >r t> (vhere) ! ; immediate
mem-sz ->memory const dict-end
: fgl last w@  (here) ! last de-sz + (last) ! ;
marker
    )");
}
#endif // _SYS_LOAD_
