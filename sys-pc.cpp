#include "c4a.h"

#ifdef IS_PC

#ifdef IS_WINDOWS
  #include <conio.h>
  int qKey() { return _kbhit(); }
  int key() { return _getch(); }
  void ttyMode(int isRaw) {}
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
#endif // IS_LINUX

cell timer() { return (cell)clock(); }
void zType(const char *str) { fputs(str, outputFp ? (FILE*)outputFp : stdout); }
void emit(const char ch) { fputc(ch, outputFp ? (FILE*)outputFp : stdout); }
void store16(cell loc, cell val) { *(uint16_t*)loc = (uint16_t)val; }
void store32(cell loc, cell val) { *(uint32_t*)loc = (uint32_t)val; }
cell fetch16(cell loc) {return *(int16_t*)loc; }
cell fetch32(cell loc) {return *(int32_t*)loc; }

// REP - Read/Execute/Print (no Loop)
void REP() {
	FILE *inFp = (FILE*)inputFp;
	if (inFp == 0) { inFp = stdin; }
    char tib[128];
    ttyMode(0);
    ok();
	if (fgets(tib, sizeof(tib), inFp) == tib) { outer(tib); }
    else { exit(0); }
}

void loadArgument(const char *arg) {
    char fn[32];
    strCpy(fn, arg);
    cell tmp = fileOpen(fn, FL_READ);
    if (tmp) { inputFp = tmp; }
}

int main(int argc, char *argv[]) {
	c4Init();
    if (argc > 1) { loadArgument(argv[1]); }
    else { outer("1 load"); }
    while (1) { REP(); };
	return 0;
}

#endif // IS_PC
