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

// Cell size is set by CELL64 in c4a.h (32-bit by default)
#define F1(x, y) (*(byte*)(x)<<y)
#define S1(x, y) (*(byte*)(x)=((y)&0xFF))

cell fetch16(cell loc) {
    if ((loc & 0x03) == 0) { return *(short*)loc; }
    return F1(loc,0) | F1(loc+1,8);
}

void store16(cell loc, cell val) {
    if ((loc & 0x03) == 0) { *(short*)loc = (short)val; }
    else {
        S1(loc,val); S1(loc+1,val>>8);;
    }
}

cell fetchCell(cell loc) {
    cell val;
    memcpy(&val, (void*)loc, CELL_SZ);
    return val;
}

void storeCell(cell loc, cell val) {
    memcpy((void*)loc, &val, CELL_SZ);
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
