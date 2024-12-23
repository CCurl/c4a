#ifndef __C4_H__
#define __C4_H__

#define VERSION   20241217
#define _SYS_LOAD_
#define EDITOR

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

#ifdef IS_PC
  #define MEM_SZ       32*1024 // A memory constrained system
  #define STK_SZ            32 // Data stack
  #define RSTK_SZ           32 // Return stack
  #define LSTK_SZ           30 // 10 nested loops (3 entries per loop)
  #define TSTK_SZ           32 // 'A' and 'T' stacks
  #define FSTK_SZ            8 // Files stack
  #define NAME_LEN          13 // To make dict-entry size 18 (13+1+1+1+2)
  #define CODE_SLOTS    0x0800 // $C000 and larger are inline numbers
  #define BLOCK_CACHE_SZ     8 // Each block is 1024 bytes
  #define BLOCK_MAX        255 // Maximum block
#define FILE_PC
#else
  #include <Arduino.h>
  #define MEM_SZ      400*1024 // These are for a RPi PICO 2 (2350)
  #define STK_SZ            64 // Data stack
  #define RSTK_SZ           64 // Return stack
  #define LSTK_SZ           45 // 15 nested loops (3 entries per loop)
  #define TSTK_SZ           64 // 'A' and 'T' stacks
  #define FSTK_SZ            8 // Files stack
  #define NAME_LEN          13 // To make dict-entry size 18 (13+1+1+1+2)
  #define CODE_SLOTS    0xC000 // $C000 and larger are inline numbers
  #define BLOCK_CACHE_SZ    16 // Each block is 1024 bytes
  #define BLOCK_MAX        255 // Maximum block
  // #define FILE_NONE
  #define FILE_PICO
  // #define FILE_TEENSY
#endif

#define btwi(n,l,h)   ((l<=n) && (n<=h))
#define _IMMED        1
#define _INLINE       2

#define CELL_T        int32_t
#define CELL_SZ       4
#define WC_T          uint16_t
#define WC_SZ         2
#define BLOCK_SZ      1024
#define NUM_BITS      0xC000
#define NUM_MASK      0x3FFF

enum { COMPILE=1, DEFINE=2, INTERP=3, COMMENT=4 };
enum { DSPA=0, RSPA, LSPA, TSPA, ASPA, HA, BA, SA, INSPA };

typedef CELL_T cell;
typedef WC_T wc_t;
typedef unsigned char byte;
typedef struct { wc_t xt; byte fl, ln; char nm[NAME_LEN+1]; } DE_T;
typedef struct { wc_t op; const char *name; byte fl; } PRIM_T;
typedef struct { uint16_t num, seq, flags; char data[BLOCK_SZ]; } CACHE_T;

// These are defined by c4.cpp
extern void c4Init();
extern void push(cell x);
extern cell pop();
extern void inPush(char *in);
extern char *inPop();
extern void strCpy(char *d, const char *s);
extern int  strEq(const char *d, const char *s);
extern int  strEqI(const char *d, const char *s);
extern int  strLen(const char *s);
extern int  lower(const char c);
extern void zTypeF(const char *fmt, ...);
extern int  changeState(int x);
extern void inner(wc_t start);
extern void outer(const char *src);
extern void outerF(const char *fmt, ...);
extern void storeWC(cell addr, wc_t val);
extern void ok();

// c4.cpp needs these to be defined
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

// Files
extern void fileInit();
extern void fileExit();
extern cell fileOpen(const char *name, const char *mode);
extern void fileClose(cell fh);
extern cell fileSize(cell fh);
extern void fileDelete(const char *name);
extern cell fileRead(char *buf, int sz, cell fh);
extern cell fileWrite(char *buf, int sz, cell fh);
extern cell fileSeek(cell fh, cell pos);
extern cell filePos(cell fh);
extern void fileLoad(const char *name);

// Blocks
extern cell block;
extern void blockInit();
extern char *blockAddr(cell blk);
extern void blockIsDirty(int blk);
extern void blockLoad(int blk);
extern void blockLoadNext(int blk);
extern void dumpCache();
extern void editBlock(cell blk);
extern void flushBlock(cell blk, CACHE_T *p, cell clear);
extern void flushBlocks(cell clear);

#endif //  __C4_H__
