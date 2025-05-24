#include "c4a.h"

#ifdef FILE_PC
// Support for files

cell inputFp, outputFp;
extern char *toIn;

FILE *makeIn(cell fh) { return fh ? (FILE*)fh: stdin; }
FILE *makeOut(cell fh) { return fh ? (FILE*)fh: stdout; }

cell fileOpen(const char *name, const char *mode) { return (cell)fopen(name, mode); }
void fileClose(cell fh) { fclose((FILE*)fh); }
void fileDelete(const char *name) { remove(name); }
cell fileRead(char *buf, int sz, cell fh) { return fread(buf, 1, sz, makeIn(fh)); }
cell fileWrite(char *buf, int sz, cell fh) { return fwrite(buf, 1, sz, makeOut(fh)); }
void fileInit() { blockInit(); }

#endif // FILE_PC
