#include "c4a.h"

#ifndef FILE_NONE
// Support for blocks
#define BLOCK_FN1       "blocks.c4a"
#define BLOCK_FN2       "\\bin\\blocks.c4a"
#define BLOCK_FN3       "/home/chris/bin/blocks.c4a"
#define DISK_SZ         (BLOCK_SZ*NUM_BLOCKS)

static cell blockFh = 0;
static char blocks[DISK_SZ], blockFN[64];
extern char *toIn;

static cell tryOpen(const char *fn, const char *mode) {
    blockFh = fileOpen(fn, mode);
    if (blockFh) { strCpy(blockFN, fn); }
    return blockFh;
}

static cell deriveBlockFN() {
    if (tryOpen(BLOCK_FN1, FL_READ)) { return 1; }
    if (tryOpen(BLOCK_FN2, FL_READ)) { return 1; }
    if (tryOpen(BLOCK_FN3, FL_READ)) { return 1; }
    strCpy(blockFN, BLOCK_FN1);
    return 0;
}

void blockInit() {
	cell sz = 0;
    if (deriveBlockFN()) {
        sz = fileRead(blocks, DISK_SZ, blockFh);
		fileClose(blockFh);
        blockFh = 0;
    }
}

void flushBlocks() {
    cell fh = fileOpen(blockFN, FL_WRITE);
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
