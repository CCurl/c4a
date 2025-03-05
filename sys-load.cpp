#include "c4a.h"

#ifndef _SYS_LOAD_
void sys_load() {
    fileLoad("bootstrap.fth");
}
#else
void sys_load() {
    outer("( Comments are free/built-in )");
    outer(": ->memory memory + ;");
    outer(": here  (here)  @ ;");
    outer(": last  (last)  @ ;");
    outer(": base@ base    @ ;");
    outer(": base! base    ! ;");
    outer(": vhere (vhere) @ ;");
    outer(": allot vhere + (vhere) ! ;");
    outer(": 0sp  0 (dsp)  ! ;");
    outer(": 0rsp 0 (rsp)  ! ;");
    outer(": , here  dup 1+ (here) ! wc! ;");
    outer(": v, vhere dup cell + (vhere) ! ! ;");
    outer(": vc, vhere dup 1+ (vhere) ! c! ;");

    outer(": const  addword lit, (exit) , ;");
    outer(": var    vhere const allot ;");
    outer(": create addword vhere lit, ;");
    outer(": does> (jmp) , r> , ;");

    outer(": begin here ; immediate");
    outer(": again (jmp)   , , ; immediate");
    outer(": while (jmpnz) , , ; immediate");
    outer(": until (jmpz)  , , ; immediate");
    outer(": -while (njmpnz) , , ; immediate");
    outer(": -until (njmpz)  , , ; immediate");

    outer(": -if (njmpz) , here 0 , ; immediate");
    outer(": if  (jmpz)  , here 0 , ; immediate");
    outer(": if0 (jmpnz) , here 0 , ; immediate");
    outer(": else (jmp) , here swap 0 , here swap wc! ; immediate");
    outer(": then here swap wc! ; immediate");

    outer(": hex     $10 base! ;   : binary  %10 base! ;");
    outer(": decimal #10 base! ;   : ?dup -if dup then ;");
    outer(": rot >r swap r> swap ; : -rot swap >r swap r> ;");
    outer(": nip swap drop ;  : tuck swap over ;");
    outer(": 2dup over over ; : 2drop drop drop ;");
    outer(": 0< 0 < ;         : 0> 0 > ;");
    outer(": <= > 0= ;        : >= < 0= ;    : <> = 0= ;");
    outer(": 2+ 1+ 1+ ;       : 2* dup + ;   : 2/ 2 / ;");
    outer(": cells cell * ;   : chars ;      : cell+ cell + ;");
    outer(": min ( a b--c ) 2dup > if swap then drop ;");
    outer(": max ( a b--c ) 2dup < if swap then drop ;");
    outer(": btwi ( n l h--f ) >r over >  swap r> >  or 0= ;");
    outer(": negate com 1+ ;");
    outer(": abs  dup 0< if negate then ;");
    outer(": mod /mod drop ;");
    outer(": +! tuck  @ + swap ! ;");
    outer(": execute ( a-- ) >r ;");

    outer(": @a  a@  c@ ;    : !a  a@  c! ;");
    outer(": @a+ a@+ c@ ;    : !a+ a@+ c! ;");
    outer(": @a- a@- c@ ;    : !a- a@- c! ;");
    outer(": a+  a@+ drop ;  : a-  a@- drop ;");
    outer(": atdrop adrop tdrop ;");
    outer(": @t  t@  c@ ;    : !t  t@  c! ;");
    outer(": @t+ t@+ c@ ;    : !t+ t@+ c! ;");
    outer(": @t- t@- c@ ;    : !t- t@- c! ;");
    outer(": t+  t@+ drop ;  : t-  t@- drop ;");

    outer(": bl 32 ; inline");
    outer(": space bl emit ;");
    outer(": cr 13 emit 10 emit ;");
    outer(": tab 9 emit ;");

    outer("40 var #buf");
    outer(": <# ( n1--n2 ) #buf 39 + >t 0 !t- dup 0 < >a abs ;");
    outer(": #c ( c-- )    !t- ; inline");
    outer(": #. ( -- )     '.' #c ;");
    outer(": #n ( n-- )    dup 9 > if 7 + then '0' + #c ;");
    outer(": #  ( n1--n2 ) base@ /mod swap #n ;");
    outer(": #s ( n-- )    begin # -while ;");
    outer(": #> ( --str )  drop a> if '-' #c then t> 1+ ;");
    outer(": (.) ( n-- ) <# #s #> ztype ;");
    outer(": .   ( n-- ) (.) space ;");

    outer(": .version version <# # # #. # # #. #s #> ztype ;");

    outer(": ? ( addr-- ) @ . ;");
    outer(": .s '(' emit space (dsp) @ 1- ?dup");
    outer("    if for i 1+ cells dstk + @ . next then ')' emit ;");

    outer(": [[ vhere >t here >t 1 state ! ;");
    outer(": ]] (exit) , 0 state wc! t@ (here) ! t> >r t> (vhere) ! ; immediate");

    outer("mem-sz 1- ->memory const dict-end");
    outer(": ->xt     @ ;");
    outer(": ->flags  wc-sz + c@ ;");
    outer(": ->len    wc-sz + 1+ c@ ;");
    outer(": ->name   wc-sz + 2+ ;");

    outer(": words last ->memory >a 0 >t 0 >r");
    outer("    begin");
    outer("      a@ ->name ztype r@ 1+ r!");
    outer("      a@ ->len dup 7 > t@ + t! 14 > t@ + t!");
    outer("      t@+ 9 > if cr 0 t! else tab then");
    outer("      a@ de-sz + a! a@ dict-end <");
    outer("    while tdrop adrop r> .\"  (%d words)\" ;");
    outer(": words-n ( n-- )  0 >a last ->memory swap for");
    outer("          dup ->name ztype tab a@+ 9 > if cr 0 a! then de-sz +");
    outer("      next drop adrop ;");

    outer("cell var t0  cell var t1  cell var t2");
    outer(": marker here t0 ! last t2 ! vhere t2 ! ;");
    outer(": forget t0 @ (here) ! t1 @ (last) ! t2 @ (vhere) ! ;");
    outer(": fgl last dup de-sz + (last) ! ->memory ->xt (here) ! ;");

#ifdef FILE_PC
    outer(": fopen-r ( fn--fh )  z\" rb\" fopen ;");
    outer(": fopen-w ( fn--fh )  z\" wb\" fopen ;");
#endif // FILE_PC
#ifdef FILE_PICO
    outer(": fopen-r ( fn--fh )  z\" r\" fopen ;");
    outer(": fopen-w ( fn--fh )  z\" w\" fopen ;");
#endif // FILE_PICO
#ifdef FILE_TEENSY
    outer(": fopen-r ( fn--fh )  z\" r\" fopen ;");
    outer(": fopen-w ( fn--fh )  z\" w\" fopen ;");
#endif // FILE_TEENSY

    outer("marker");
}
#endif // _SYS_LOAD_
