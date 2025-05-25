# c4a: a Forth system for PCs and Arduino, inspired by ColorForth and Tachyon

## ColorForth's influence on c4a
- c4a supports control characters (tags) in the whitespace that change the state.
- c4a has 4 states: INTERPRET, COMPILE, DEFINE, AND COMMENT.
- c4a also supports the standard state-change words.
- c4a has a built-in editor. It has a VI-like feel.
- - For details, see the [Editor documentation](Editor.md).
- c4a has 'a', 'b', and 't' words.
- c4a reads 'blocks.fth' into memory on load.
- - This is what it uses to edit and load blocks.
- - Use 'flush' to write it back to disk/flash.

## Tags
| Tag | Word | State | Description |
|:--  |:--   |:--    |:-- |
| $01 |  ]   |   1   | Compile |
| $02 |  :   |   2   | Define |
| $03 |  [   |   3   | Interpret/execute/immediate |
| $04 |      |   4   | Comment |
|     |  (   |   4   | Comment, save current state |
|     |  )   |       | End comment, restores saved state |

**NOTE**: In the DEFINE state, c4a changes the state to COMPILE after adding the next word. <br/>
**NOTE**: Unlike ColorForth, ';' compiles EXIT and then changes the state to INTERPRET. <br/>

## Tachyon's influence on c4a
- In c4a, a program is a sequence of WORD-CODEs. <br/>
- A WORD-CODE is a 16-bit unsigned number. <br/>
- Primitives are assigned numbers sequentially from 0 to [BYE]. <br/>
- If a WORD-CODE is less than or equal to [BYE], it is a primitive. <br/>
- If the top 4 bits are set, it is 13-bit unsigned literal, 0-$0FFF. <br/>
- If it is between 'BYE' and $F000, it is the code address of a word to execute. <br/>

## Development board considerations
- c4a is intended to be used with development boards via the Arduino IDE.
- However, it can be built for PCs as well for testing.
- I am not aware of 64-bit dev boards.
- So a **CELL** in c4a is 32-bits.
- C4A has many built-in primitives, for the following reasons:
- - Program flash is often quite large, so why not use it?
- - To give C4A more functionality out of the box.
- - C4A also runs faster because of it.

## Building c4a
 
### PCs - Windows and Linux, and probably others
- Windows: there is a c4a.sln file for Visual Studio
  - only the x86 target (32-bit) is supported
- Linux: there is a makefile
  - only the 32-bit configuration (-m32) is supported
- Others:
  - c4a is simple enough that it should be easy to migrate it to any platform

### Development boards via the Arduino IDE:
- I use the Arduino IDE v2.x
- There is a c4a.ino file
- File 'c4a.h' controls parameters for the target board
- Edit the section where `IS_BOARD` is defined to set the configuration for the board
- Use `#define FILE_NONE` to disable support for blocks and LittleFS
- For the RPI Pico:
  - Use the arduino-pico from earlephilhower (https://github.com/earlephilhower/arduino-pico)
  - The version must be 4.2.0 or later. Versions older than 4.0.0 do not support boards using
    the RP2350 microcontroller.
  - Use `#define FILE_PICO` to include support for LittleFS
- For the Teensy-4.x:
  - Use `#define FILE_TEENSY` to include support for LittleFS

## c4a memory areas
c4a provides a single memory area. See 'mem-sz' (MEM_SZ in c4a.h) for its size.
- It is broken into 3 areas: CODE, VARS, and DICTIONARY.
- The CODE area is an aray of WORD-CODEs starting at the beginning of the memory.
  - `here` is an offset into the CODE area.
  - The size of the CODE area is `code-sz`. See CODE_SZ in c4a.h.
  - **NOTE**: Use `wc@` and `wc!` to get and set 16-bit values in the code area.
  - **NOTE**: Use `cv@` and `cv!` to get and set 32-bit values in the code area.
  - **NOTE**: CODE slots 0-25 (`0 wc@ .. 25 wc@`) are reserved for c4a system values.
  - **NOTE**: CODE slots 26-(BYE) are unused by c4a.
  - **NOTE**: Using `wc@` and `wc!`, c4a provides storage space for many 16-bit variables.
  - **NOTE**: Using `cv@` and `cv!`, c4a provides storage space for many 32-bit variables.
  - **NOTE**: These are free for the user/application to use as desired.
- The VARS area is defined to begin at address `code-sz wc-sz * memory +`.
  - `vhere` is the absolute address of the first free byte the VARS area.
- The DICTIONARY is at the end of the memory. 'last' grows toward the beginning of the memory.
  - `last` is the address of the most recently created word.
  - A dictionary entry is [xt:2][flags:1][len:1][name:NAME_LEN][0:1]
  - The default NAME_LEN is 11 (see c4a.h), so 'de-sz' is 16.
- Use `->memory` to turn an offset into an address.

| WORD    | STACK | DESCRIPTION |
|:--      |:--    |:-- |
| memory  | (--A) | A: starting address of the c4a memory |
| mem-sz  | (--N) | N: size in BYTEs of the c4a memory |
| code-sz | (--N) | N: number of in WORD-CODE slots in the code area |
| dstk-sz | (--N) | N: size in CELLs of the DATA and RETURN stacks |
| tstk-sz | (--N) | N: size in CELLs of the A and T stacks |
| wc-sz   | (--N) | N: size in BYTEs of a WORD-CODE |
| de-sz   | (--N) | N: size in BYTEs of a dictionary entry |
| (dsp)   | (--N) | N: Address of the data stack pointer |
| (rsp)   | (--N) | N: Address of the return stack pointer |
| (lsp)   | (--N) | N: Address of the loop stack pointer |
| (asp)   | (--N) | N: Address of the A stack pointer |
| (bsp)   | (--N) | N: Address of the B stack pointer |
| (tsp)   | (--N) | N: Address of the T stack pointer |
| (here)  | (--N) | N: Address of the HERE variable |
| (last)  | (--N) | N: Address of the LAST variable |
| base    | (--N) | N: Address of the BASE variable |
| state   | (--N) | N: Address of the STATE variable |

## c4a Strings
Strings in c4a are NULL-terminated with no count byte. <br/>

## Format specifiers in `ftype` and `."`
Similar to the printf() function in C, c4a supports formatted output using '%'. <br/>
For example `: ascii dup dup dup ." char: %c, decimal: #%d, binary: %%%b, hex: $%x%n" ;`.

| Format | Stack | Description |
|:--     |:--    |:-- |
| %b     | (N--) | Print TOS in base 2. |
| %c     | (N--) | EMIT TOS. |
| %d     | (N--) | Print TOS in base 10. |
| %e     | (--)  | EMIT `escape` (#27). |
| %i     | (N--) | Print TOS in the current base. |
| %n     | (--)  | Print CR/LF (13/10). |
| %q     | (--)  | EMIT `"` (#34). |
| %s     | (A--) | Print TOS as a string (formatted). |
| %S     | (A--) | Print TOS as a string (unformatted). |
| %x     | (N--) | Print TOS in base 16. |
| %B     | (--)  | Change foreground to blue. |
| %G     | (--)  | Change foreground to green. |
| %P     | (--)  | Change foreground to purple. |
| %R     | (--)  | Change foreground to red. |
| %W     | (--)  | Change foreground to white. |
| %Y     | (--)  | Change foreground to yellow. |
| %[x]   | (--)  | EMIT [x]. |

## The editor
c4a has a built-in editor. It colorizes the code based on tags in the text. <br/>
For details, see the [Editor documentation](Editor.md).

## The A, B, and T stacks
c4a includes A, B, and T stacks. <br/>
These are somewhat similar to ColorForth's operations for a and b, but in c4a, they are stacks. <br/>
The size of the stacks is configurable (see `stk-sz`). <br/>
The words below are available for all 3 stacks. <br/>
Note that there are also additional words `r!` `r@+` and `r@-` for the return stack. <br/>

| WORD  | STACK | DESCRIPTION |
|:--    |:--    |:-- |
|  >a   | (N--) | Push N onto the A stack. |
|  a!   | (N--) | Set A to N. |
|  a@   | (--N) | N: value of A. |
|  a@+  | (--N) | N: value of A, then increment A. |
|  a@-  | (--N) | N: value of A, then decrement A. |
|  !a   | (N--) | Store N through A. |
|  !a+  | (N--) | Store N through A, then increment A. |
|  !a-  | (N--) | Store N through A, then decrement A. |
|  @a   | (--N) | Fetch N through A. |
|  @a+  | (--N) | Fetch N through A, then increment A. |
|  @a-  | (--N) | Fetch N through A, then decrement A. |
|  a>   | (--N) | Pop N from the A stack. |
| adrop | (--)  | Drop A-TOS |

## Temporary words
c4a provides 10 temporary words, 't0' .. 't9'.
- They are case-sensitive.
- - 't0' is a temporary word; 'T0' is NOT.
- They do not take valuable dictionary space.
- They can be used to improve factoring, or as variable or constant names.
- t0:t5 are normal words, t6:t9 are INLINE.

## Tasks
- c4a supports a simple cooperative multi-tasking system.
- The task system is not preemptive.
- Words `add-task (xt--n)`, `yield (--)`, `del-task (n--)` are provided.
- Each task has its own data stack, return stack, and loop stack.
- The A, B, and T stacks are shared by all tasks.

## c4a WORD-CODE primitives
NOTE: Since c4a is intended for dev boards, it has many more primitives than C4.</br>
      This is primarily because there is more program memory than RAM.</br>
      It provides more built-in functionality.</br>
      And the system is faster.</br>

The primitives in c4a:

| BASE        | STACK          | DESCRIPTION |
|:--          |:--             |:-- |
| (lit)       | (--WC)         | WC: WORD-CODE for LIT primitive |
| (jmp)       | (--WC)         | WC: WORD-CODE for JMP primitive |
| (jmpz)      | (--WC)         | WC: WORD-CODE for JMPZ primitive |
| (jmpnz)     | (--WC)         | WC: WORD-CODE for JMPNZ primitive |
| (njmpz)     | (--WC)         | WC: WORD-CODE for NJMPZ primitive |
| (njmpnz)    | (--WC)         | WC: WORD-CODE for NJMPNZ primitive |
| (exit)      | (--WC)         | WC: WORD-CODE for EXIT primitive |
| exit        | (--)           | EXIT word |
| dup         | (X--X X)       | Duplicate TOS (Top-Of-Stack) |
| ?dup        | (X--X | X X)   | Duplicate TOS if it is NON-zero |
| swap        | (X Y--Y X)     | Swap TOS and NOS (Next-On-Stack) |
| drop        | (N--)          | Drop TOS |
| 2dup        | (X Y--X Y X Y) | Duplicate TOS and NOS |
| 2drop       | (X Y--)        | Drop TOS and NOS |
| over        | (X Y--X Y X)   | Push NOS |
| nip         | (X Y--Y)       | Drop NOS |
| tuck        | (X Y--Y X Y)   | Perform SWAP, OVER |
| c@          | (A--C)         | C: the CHAR at absolute address A |
| w@          | (A--W)         | W: the WORD at absolute address A |
| @           | (A--N)         | N: the CELL at absolute address A |
| wc@         | (N--WC)        | WC: the WORD-CODE in CODE slot N |
| cv@         | (N--)          | Code-Variable: Fetch a 32-bit value from CODE slots N/N+1 |
| c!          | (C A--)        | Store CHAR C to absolute address A |
| w!          | (W A--)        | Store WORD W to absolute address A |
| !           | (N A--)        | Store CELL N to absolute address A |
| wc!         | (WC N--)       | Store WORD-CODE WC to CODE slot N |
| cv!         | (N--)          | Code-Variable: Store a 32-bit value to CODE slots N/N+1 |
| if          | (X--)          | Jump to 'then' if X == 0 (IMMEDIATE) |
| if0         | (X--)          | Jump to 'then' if X <> 0 (IMMEDIATE) |
| -if         | (X--X)         | Jump to 'then' if X == 0 (IMMEDIATE) |
| then        | (--)           | Target for 'if' branch (IMMEDIATE) |
| begin       | (--)           | Begin a loop (IMMEDIATE) |
| again       | (--)           | Jump to matching 'begin' (IMMEDIATE) |
| while       | (X--)          | Jump to matching 'begin' if X <> 0 (IMMEDIATE) |
| -while      | (X--X)         | Jump to matching 'begin' if X <> 0 (IMMEDIATE) |
| until       | (X--)          | Jump to matching 'begin' if X == 0 (IMMEDIATE) |

| MATH        | STACK       | DESCRIPTION |
|:--          |:--          |:-- |
| +           | (X Y--N)    | N: X + Y |
| -           | (X Y--N)    | N: X - Y |
| *           | (X Y--N)    | N: X * Y |
| */          | (N X Y--N') | N': (N * X) / Y - Scale N by X/Y |
| /           | (X Y--N)    | N: X / Y (integer division) |
| mod         | (X Y--M)    | M: X modulo Y |
| /mod        | (X Y--M Q)  | M: X modulo Y, Q: quotient of X / Y |
| <<          | (X Y--Z)    | Z: X left-shifted Y bits |
| >>          | (X Y--Z)    | Z: X right-shifted Y bits |
| 1+          | (X--Y)      | Increment TOS |
| 1-          | (X--Y)      | Decrement TOS |
| 2+          | (X--Y)      | Y: X + 2 |
| 2*          | (X--Y)      | Y: X * 2 |
| 2/          | (X--Y)      | Y: X / 2 |
| CELLS       | (X--Y)      | Y: X * CELL |
| CELL+       | (X--Y)      | Y: X + CELL |
| <           | (X Y--F)    | F: 1 if (X <  Y), else 0 |
| <=          | (X Y--F)    | F: 1 if (X <= Y), else 0 |
| =           | (X Y--F)    | F: 1 if (X == Y), else 0 |
| >=          | (X Y--F)    | F: 1 if (X >= Y), else 0 |
| >           | (X Y--F)    | F: 1 if (X >  Y), else 0 |
| 0=          | (N--F)      | F: 1 if (N == 0), else 0 |
| and         | (X Y--N)    | N: X AND Y |
| or          | (X Y--N)    | N: X OR  Y |
| xor         | (X Y--N)    | N: X XOR Y |
| com         | (X--Y)      | Y: X with all bits flipped (one's complement) |
| min         | (X Y--Z)    | Z: the minimum of (X and Y) |
| max         | (X Y--Z)    | Z: the maximum of (X and Y) |
| negate      | (X--Y)      | Y: -X |
| abs         | (X--Y)      | Y: the absolute value of X |

| MORE PRIMS  | STACK  | DESCRIPTION |
|:--          |:--     |:-- |
| for         | (N--)  | Begin FOR loop with bounds 0 and N-1. |
| i           | (--I)  | N: Current FOR loop index. |
| next        | (--)   | Increment I. If I >= N, exit, else start loop again. |
| unloop      | (--)   | Unwind the loop stack. **NOTE:** does NOT exit the loop. |
| >r          | (N--)  | Push N onto the return stack |
| r!          | (N--)  | Set R to N |
| r@          | (--N)  | N: copy of R |
| r@+         | (--N)  | N: copy of R, then increment it |
| r@-         | (--N)  | N: copy of R, then decrement it |
| r>          | (--N)  | Pop N from the return stack |
| rdrop       | (--)   | Drop R-TOS |
| >t          | (N--)  | Push N onto the T stack |
| t!          | (N--)  | Set T to N |
| t@          | (--N)  | N: copy of T |
| t@+         | (--N)  | N: copy of T, then increment T |
| t@-         | (--N)  | N: copy of T, then decrement T |
| t>          | (--N)  | Pop N from the T stack |
| tdrop       | (--)   | Drop T-TOS |
| >a          | (N--)  | Push N onto the A stack |
| a!          | (N--)  | Set A to N |
| a@          | (--N)  | N: copy of A |
| a@+         | (--N)  | N: copy of A, then increment A |
| a@-         | (--N)  | N: copy of A, then decrement A |
| @a          | (--C)  | Fetch BYTE C through A |
| @a+         | (--C)  | Fetch BYTE C through A, then increment A |
| @a-         | (--C)  | Fetch BYTE C through A, then decrement A |
| !a          | (C--)  | Store BYTE C through A |
| !a+         | (C--)  | Store BYTE C through A, then increment A |
| !a-         | (C--)  | Store BYTE C through A, then decrement A |
| a>          | (--N)  | Pop N from the A stack |
| adrop       | (--)   | Drop A-TOS |
| >b          | (N--)  | Push N onto the B stack |
| b!          | (N--)  | Set B to N |
| b@          | (--N)  | N: copy of B |
| b@+         | (--N)  | N: copy of B, then increment B |
| b@-         | (--N)  | N: copy of B, then decrement B |
| @b          | (--C)  | Fetch BYTE C through B |
| @b+         | (--C)  | Fetch BYTE C through B, then increment B |
| @b-         | (--C)  | Fetch BYTE C through B, then decrement B |
| !b          | (C--)  | Store BYTE C through B |
| !b+         | (C--)  | Store BYTE C through B, then increment B |
| !b-         | (C--)  | Store BYTE C through B, then decrement B |
| b>          | (--N)  | Pop N from the B stack |
| bdrop       | (--)   | Drop B-TOS |

| STRINGS     | STACK     | DESCRIPTION |
|:--          |:--        |:-- |
| s-len       | (S--N)    | N: Length of string S |
| s-cpy       | (D S--D)  | Copy string S to D |
| s-cat       | (D S--D)  | Concatenate string S to D |
| s-eq        | (D S--F)  | F: 1 if string S is equal to D (case sensitive) |
| s-eqi       | (D S--F)  | F: 1 if string S is equal to D (NOT case sensitive) |
| ltrim       | (S1--S2)  | S2: Trim whitespace from the beginning of S1 |
| rtrim       | (S--S)    | Trim whitespace from the end of S |
| fill        | (A N C--) | Fill N bytes from A with BYTE C |
| cmove       | (F T N--) | Copy N bytes from F to T - low to high |
| cmove>      | (F T N--) | Copy N bytes from F to T - high to low |
| lower       | (X--Y)    | Y: lower-case of X if 'A' <= X <= 'Z', else X |
| upper       | (X--Y)    | Y: upper-case of X if 'a' <= X <= 'a', else X |
| z"          | (--)      | -COMPILE: Create string S to next `"` (IMMEDIATE) |
|             | (--S)     | -RUN: push address S of string |
| ."          | (--)      | -COMPILE: execute `z"`, compile `ftype` (IMMEDIATE) |
|             | (--)      | -RUN: `ftype` on string |

| FILES       | STACK       | DESCRIPTION |
|:--          |:--          |:-- |
| fopen       | (NM MD--FH) | NM: File Name, MD: Mode, FH: File Handle (0 if error/not found) |
| fclose      | (FH--)      | FH: File Handle to close |
| fdelete     | (NM--)      | NM: File Name to delete |
| fread       | (A N FH--X) | A: Buffer, N: Size, FH: File Handle, X: num chars read |
| fwrite      | (A N FH--X) | A: Buffer, N: Size, FH: File Handle, X: num chars written |
| loaded?     | (XT A--)    | Stops current load if A <> 0 (see `find`) |
| load        | (N--)       | N: Block number to load |
| load-next   | (N--)       | Close the current block and load block N next |
| blocks      | (--)        | Dump block cache |
| block-addr  | (N--A)      | N: Block number, A: Address in cache |
| flush       | (--)        | Write RAM disk to flash/disk |
| edit        | (N--)       | N: Block number to edit |

| SYSTEM      | STACK    | DESCRIPTION |
|:--          |:--       |:-- |
| emit        | (C--)    | Output char C |
| key         | (--C)    | Read char C |
| ?key        | (--F)    | F: 1 if key available, else 0 |
| ;           | (--)     | Compile EXIT, set STATE=INTERPRET (IMMEDIATE) |
| ztype       | (S--)    | Print string at S (unformatted) |
| ftype       | (S--)    | Print string at S (formatted) |
| lit,        | (N--)    | Compile a push of number N |
| ,           | (WC--)   | WC: WORD-CODE to store at HERE, HERE += WC-SZ |
| v,          | (N--)    | N: CELL to store at VHERE, VHERE += CELL |
| vc,         | (B--)    | N: BYTE to store at VHERE, VHERE += 1 |
| const       | (N--)    | ADD-WORD, generate `LIT [N] EXIT` |
| var         | (N--)    | ADD-WORD, generate `LIT [VHERE] EXIT`, VHERE += N |
| val         | (--)     | ADD-WORD, generate `LIT 0 EXIT` |
| (val)       | (--)     | ADD-WORD, generate `LIT [HERE-4] EXIT` |
| next-wd     | (--L)    | L: length of the next word from the input stream |
| immediate   | (--)     | Mark the last created word as IMMEDIATE |
| inline      | (--)     | Mark the last created word as INLINE |
| outer       | (S--)    | Send string S to the c4a outer interpreter |
| addword     | (--)     | Add the next word to the dictionary |
| timer       | (--N)    | N: Current time |
| see X       | (--)     | Output the definition of word X |
| find        | (--XT A) | XT: Execution Token, A: Dict Entry address (0 0 if not found) |
| add-task    | (XT--N)  | XT: addr of code, N: task slot |
| del-task    | (N--)    | Delete task in slot N |
| .tasks      | (--)     | Print all tasks |
| yield       | (--)     | Yield to the next task |
| here        | (--)     | Current value of (HERE) |
| last        | (--)     | Current value of (LAST) |
| vhere       | (--)     | Current value of (VHERE) |
| words       | (--)     | Print list of words and primitives |
| words-n     | (N--)    | Print list of N most recently defined words |
| (.)         | (N--)    | Output N in the current BASE |
| .           | (N--)    | Output N in the current BASE, followed by a SPACE |
| .s          | (--)     | Output the stack in the current BASE |
| allot       | (N--)    | VHERE += N |
| bl          | (--N)    | N: 32 - the ascii value for a space |
| tab         | (--)     | EMIT a tab (ascii 9) |
| cr          | (--)     | EMIT CR/LF (13, 10) |
| space       | (--)     | EMIT a single space |

| PC ONLY     | STACK  | DESCRIPTION |
|:--          |:--     |:-- |
| system      | (S--)  | S: String to send to `system()` |
| bye         | (--)   | Exit c4a |

| DEV BOARD   | STACK   | DESCRIPTION |
|:--          |:--      |:-- |
| pin-input   | (P--)   | Open pin P for input |
| pin-output  | (P--)   | Open pin P for output |
| pin-pullup  | (P--)   | Open pin P for input with pullup |
| dpin@       | (P--N)  | N: value at digital pin P |
| dpin!       | (N P--) | Write N to digital pin P |
| apin@       | (P--N)  | N: value at analog pin P |
| apin!       | (N P--) | Write N to analog pin P |
| bye         | (--)    | No-op |

## c4a default words
Default/built-in words are defined in function `sys_load()` in file sys-load.cpp. <br/>
For details, or to add or change the default words, modify that function.
