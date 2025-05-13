#include "c4a.h"

#ifndef FILE_NONE
// Support for blocks
#define BLOCK_DIRTY     0x01
#define BLOCK_CLEAN     0xFE
#define BLOCK_FREE      0xFFFF
#define BLOCK_FN        "blocks.fth"

char blocks[BLOCK_SZ*(BLOCK_MAX+1)];
static uint16_t seq;
extern char *toIn;

void blockInit() {
	cell fn = fileOpen(BLOCK_FN, FL_READ);
    if (fn) {
        fileRead(blocks, sizeof(blocks), fn);
		fileClose(fn);
    }
}

void flushBlocks() {
    cell fn = fileOpen(BLOCK_FN, FL_WRITE);
    if (fn) {
        fileWrite(blocks, sizeof(blocks), fn);
        fileClose(fn);
    }
}

static void prepForLoad() { toIn[BLOCK_SZ-1]=0; changeState(INTERP); }
char *blockAddr(wc_t blk) { return (char*)&blocks[MIN(blk, BLOCK_MAX)*BLOCK_SZ]; }
void blockLoadNext(wc_t blk) { toIn=blockAddr(blk); prepForLoad(); }
void blockLoad(wc_t blk) { inPush(toIn); blockLoadNext(blk); }

#endif // FILE_NONE
