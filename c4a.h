#ifndef __C4A_H__
#define __C4A_H__

#define VERSION   20250515
#define _SYS_LOAD_

#ifdef _MSC_VER
  #define _CRT_SECURE_NO_WARNINGS
  #define IS_WINDOWS 1
  #define IS_PC      1
#elif (defined __i386 || defined __x86_64 || defined IS_LINUX)
  #define IS_LINUX   1
  #define IS_PC      1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

// Change these for the system/board
// These work for the RPi Pico 2 and the Teensy 4.0
#define MEM_SZ      320*1024
#define CODE_SLOTS  NUM_BITS // Values larger are inline numbers
#define STK_SZ            31 // Data stack size
#define FSTK_SZ            8 // Files stack size
#define TASKS_SZ           8 // Number of tasks
#define NAME_LEN          11 // Size of dict-entry is (2+1+1+NAME_LEN+1)
#define BLOCK_MAX         49 // Max block
#define EOL_CHAR          13 // Carriage Return

// System defines
#define CELL_T           int32_t
#define CELL_SZ             4
#define WC_T             uint16_t
#define WC_SZ               2
#define NUM_BITS       0xF000
#define NUM_MASK       0x0FFF
#define NUM_LINES          24
#define NUM_COLS           80
#define BLOCK_SZ      (NUM_LINES*NUM_COLS)
#define STK_DATA            0
#define STK_RETN            1
#define STK_LSTK            2
#define TASK_MAX      (TASKS_SZ-1)
#define NUM_BLOCKS    (BLOCK_MAX+1)
#define _IMMED              1
#define _INLINE             2
#define btwi(n,l,h)   ((l<=n) && (n<=h))

#ifndef MAX
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

enum { COMPILE=1, DEFINE=2, INTERP=3, COMMENT=4 };
typedef CELL_T cell;
typedef WC_T wc_t;
typedef uint8_t byte;
typedef struct { wc_t xt; byte flg, len; char nm[NAME_LEN+1]; } DE_T;
typedef struct { wc_t op; byte flg, len; const char *name; } PRIM_T;
typedef struct { cell sp; cell stk[STK_SZ+1]; } STK_T;

// #define TASK_CYCLES   1000
typedef struct { STK_T stks[3]; wc_t pc, base; int status; } TASK_T;

#ifdef IS_PC
  #define FL_READ          "rb"
  #define FL_RW            "r+b"
  #define FL_WRITE         "wb"
  #define FL_APPEND        "ab"
  #define FILE_PC
#else
  #include <Arduino.h>
  #define IS_BOARD          1
  #define FL_READ          "r"
  #define FL_RW            "r+"
  #define FL_WRITE         "w"
  #define FL_APPEND        "a"
  // #define FILE_NONE
  // #define FILE_PICO
  #define FILE_TEENSY
#endif

// These are defined by c4a.cpp
extern void c4Init();
extern void push(cell x);
extern cell pop();
extern void inPush(char *in);
extern char *inPop();
extern void strCpy(char *d, const char *s);
extern int  strEq(const char *d, const char *s);
extern int  strEqI(const char *d, const char *s);
extern int  strLen(const char *s);
extern void fill(byte *dst, cell num, byte ch);
extern void cmove(byte *src, byte *dst, cell num);
extern int  lower(const char c);
extern void zTypeF(const char *fmt, ...);
extern void fType(const char *str);
extern int  changeState(int x);
extern void inner(wc_t start);
extern void outer(const char *src);
extern void outerF(const char *fmt, ...);
extern void ok();
extern cell block;

// c4a.cpp needs these to be defined
extern cell inputFp, outputFp;
extern cell fetch16(cell loc);
extern cell fetch32(cell loc);
extern void store16(cell loc, cell val);
extern void store32(cell loc, cell val);
extern void zType(const char *str);
extern void emit(const char ch);
extern void ttyMode(int isRaw);
extern int  key();
extern int  qKey();
extern cell timer();
extern void sys_load();
extern void Blue();
extern void Green();
extern void Purple();
extern void Red();
extern void White();
extern void Yellow();

#ifndef FILE_NONE
  extern void fileInit();
  extern cell fileOpen(const char *name, const char *mode);
  extern void fileClose(cell fh);
  extern void fileDelete(const char *name);
  extern cell fileRead(char *buf, int sz, cell fh);
  extern cell fileWrite(char *buf, int sz, cell fh);
  extern void fileLoad(const char *name);
  
  // ... and these - blocks
  extern void blockInit();
  extern char *blockAddr(wc_t blk);
  extern void blockLoad(wc_t blk);
  extern void blockLoadNext(wc_t blk);
  extern void editBlock(cell blk);
  extern void flushBlocks();
  #define EDITOR
#endif // FILE_NONE

#endif //  __C4A_H__
