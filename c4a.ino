// Support for development boards

#include "c4a.h"

#define mySerial Serial // Teensy and Pico

#ifdef mySerial
    void serialInit() { while (!mySerial) ; }
    void emit(char c) { mySerial.print(c); }
    void zType(const char *s) { mySerial.print(s); }
    int qKey() { return mySerial.available(); }
    int key() { 
        while (!qKey()) {}
        return mySerial.read();
    }
#else
    void serialInit() { }
    void emit(char c) {}
    void zType(const char *s) {}
    int qKey() { return 0; }
    int key() { return 0; }
#endif

cell timer() { return micros(); }
void ttyMode(int isRaw) {}

// Cells are always 32-bit on dev boards (no 64-bit)
#define S1(x, y) (*(byte*)(x)=((y)&0xFF))
void store32(cell loc, cell val) {
    if (((cell)loc & 0x03) == 0) { *(cell*)loc = val; }
    else {
        S1(loc,val); S1(loc+1,val>>8); S1(loc+2,val>>16); S1(loc+3,val>>24);
    }
}

void store16(cell loc, cell val) {
    if (((cell)loc & 0x03) == 0) { *(short*)loc = (short)val; }
    else {
        S1(loc,val); S1(loc+1,val>>8);;
    }
}

#define F1(x, y) (*(byte*)(x)<<y)
cell fetch32(cell loc) {
    if (((cell)loc & 0x03) == 0) { return *(cell*)loc; }
    return F1(loc,0) | F1(loc+1,8) | F1(loc+2,16) | F1(loc+3,24);
}

cell fetch16(cell loc) {
    if (((cell)loc & 0x03) == 0) { return *(short*)loc; }
    return F1(loc,0) | F1(loc+1,8);
}

char *in, tib[128];
void setup() {
  serialInit();
  c4Init();
  outer(" .\" %nc4 - version \" .version cr");
  zType("Hello.");
  ok();
  in = (char*)0;
}

void idle() {
  // Fill this in as desired
}

void loop() {
  if (qKey() == 0) { idle(); return; }
  int c = key();
  if (!in) {
      in = tib;
      for (int i=0; i<128; i++) { tib[i] = 0; }
  }

  if (c==9) { c = 32; }
  if (c==EOL_CHAR) {
      *(in) = 0;
      emit(32); outer(tib);
      ok();
      in = 0;
  } else if ((c==8) || (c==127)) {
      if ((--in) < tib) { in = tib; }
      else { emit(8); emit(32); emit(8); }
  } else {
      if (btwi(c,32,126)) { *(in++) = c; emit(c); }
  }
}
