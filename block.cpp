#include "c4a.h"

#ifndef FILE_NONE
// Support for blocks
#define BLOCK_FN1       "blocks.c4a"
#define BLOCK_FN2       "\\bin\\blocks.c4a"
#define BLOCK_FN3       "/home/chris/bin/blocks.c4a"
#define DISK_SZ         (BLOCK_SZ*NUM_BLOCKS)

static char blocks[DISK_SZ], blockFN[64];
extern char *toIn;

static cell tryOpen(const char *fn, cell *fileHandle) {
    *fileHandle = fileOpen(fn, FL_READ);
    if (*fileHandle) { strCpy(blockFN, fn); }
    return *fileHandle;
}

static cell deriveBlockFh() {
    cell fh = 0;
    if (tryOpen(BLOCK_FN1, &fh)) { return fh; }
    if (tryOpen(BLOCK_FN2, &fh)) { return fh; }
    if (tryOpen(BLOCK_FN3, &fh)) { return fh; }
    strCpy(blockFN, BLOCK_FN1);
    return 0;
}

void blockInit() {
	cell sz = 0, fh = deriveBlockFh();
    if (fh) {
        sz = fileRead(blocks, DISK_SZ, fh);
		fileClose(fh);
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
