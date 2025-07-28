#include "c4a.h"

#ifndef FILE_NONE
// Support for blocks
#define BLOCK_FN        "blocks.fth"
#define DISK_SZ         (BLOCK_SZ*NUM_BLOCKS)


char blocks[DISK_SZ];
extern char *toIn;

void blockInit() {
	cell sz = 0, fh = fileOpen(BLOCK_FN, FL_READ);
    if (fh) {
        sz = fileRead(blocks, DISK_SZ, fh);
		fileClose(fh);
    }
    // if (sz != DISK_SZ) { flushBlocks(); }
}

void flushBlocks() {
    cell fh = fileOpen(BLOCK_FN, FL_WRITE);
    if (fh) {
        fileWrite(blocks, DISK_SZ, fh);
        fileClose(fh);
    }
}

static void prepForLoad() { toIn[BLOCK_SZ-1]=0; changeState(INTERP); }
char *blockAddr(wc_t blk) { return (char*)&blocks[MIN(blk, BLOCK_MAX)*BLOCK_SZ]; }
void blockLoadNext(wc_t blk) { toIn=blockAddr(blk); prepForLoad(); }
void blockLoad(wc_t blk) { inPush(toIn); blockLoadNext(blk); }

#endif // FILE_NONE
