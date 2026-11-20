#include "c4a.h"

#ifdef IS_PC

#ifdef IS_WINDOWS
  #include <windows.h>
  #include <conio.h>
  int qKey() { return _kbhit(); }
  int key() { return _getch(); }
  void ttyMode(int isRaw) {}
  void ms(cell x) { Sleep((DWORD)x); }
#endif

#ifdef IS_LINUX
  #include <termios.h>
  #include <unistd.h>
  #include <sys/time.h>
  
  void ttyMode(int isRaw) {
      static struct termios origt, rawt;
      static int curMode = -1;
      if (curMode == -1) {
          curMode = 0;
          tcgetattr( STDIN_FILENO, &origt);
          cfmakeraw(&rawt);
      }
      if (isRaw != curMode) {
          if (isRaw) {
              tcsetattr( STDIN_FILENO, TCSANOW, &rawt);
          } else {
              tcsetattr( STDIN_FILENO, TCSANOW, &origt);
          }
          curMode = isRaw;
      }
  }
  int qKey() {
      struct timeval tv;
      fd_set rdfs;
      ttyMode(1);
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      FD_ZERO(&rdfs);
      FD_SET(STDIN_FILENO, &rdfs);
      select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
      int x = FD_ISSET(STDIN_FILENO, &rdfs);
      return x;
  }
  int key() {
      ttyMode(1);
      int x = fgetc(stdin);
      return x;
  }
  void ms(cell x) {
	  if (x > 0) { usleep(x * 1000); }
  }
#endif // IS_LINUX

#ifdef FILE_PC
    // Support for files
    FILE *makeIn(cell fh) { return fh ? (FILE*)fh: stdin; }
    FILE *makeOut(cell fh) { return fh ? (FILE*)fh: stdout; }

    cell fileOpen(const char *name, const char *mode) { return (cell)fopen(name, mode); }
    void fileClose(cell fh) { fclose((FILE*)fh); }
    void fileDelete(const char *name) { remove(name); }
    cell fileRead(char *buf, int sz, cell fh) { return fread(buf, 1, sz, makeIn(fh)); }
    cell fileWrite(char *buf, int sz, cell fh) { return fwrite(buf, 1, sz, makeOut(fh)); }
    void fileInit() { blockInit(); }
#endif // FILE_PC

// C4A Required
cell outputFp = 0;
cell timer() { return (cell)clock(); }
void zType(const char *str) { fputs(str, outputFp ? (FILE*)outputFp : stdout); }
void emit(const char ch) { fputc(ch, outputFp ? (FILE*)outputFp : stdout); }
void store16(cell loc, cell val) { *(uint16_t*)loc = (uint16_t)val; }
void store32(cell loc, cell val) { *(uint32_t*)loc = (uint32_t)val; }
void storeCell(cell loc, cell val) { *(cell*)loc = (cell)val; }
cell fetch16(cell loc)   { return *(int16_t*)loc; }
cell fetch32(cell loc)   { return *(int32_t*)loc; }
cell fetchCell(cell loc) { return *(cell*)loc; }

// PC Specific
void readExecute() {
    char tib[128];
    ttyMode(0);
    if ((state == 0) || (state == COMMENT)) { changeState(INTERP); }
    if (state == INTERP) { zType(" ok\n"); }
    else if (state == COMPILE) { zType("... "); }
    else if (state == DEFINE) { zType("(define) "); }
	if (fgets(tib, sizeof(tib), stdin) == tib) { outer(tib); }
    else { exit(0); }
}

void loadArgument(const char *arg) {
    cell tmp = fileOpen((char *)arg, FL_READ);
    if (tmp) {
        char *buf = (char*)(vhere+15000);
        fileRead(buf, 15000, tmp);
        fileClose(tmp);
        outer(buf);
    }
}

int main(int argc, char *argv[]) {
	c4Init();
    if (argc > 1) { loadArgument(argv[1]); }
    else { outer("1 load"); }
    while (1) { readExecute(); };
	return 0;
}

#endif // IS_PC
