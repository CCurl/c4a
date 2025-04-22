#include "c4a.h"

#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define NCASE         goto next; case
#define BCASE         break; case
#define STK(x)        tasks[curTask].stks[x].stk
#define SP(x)         tasks[curTask].stks[x].sp

byte memory[MEM_SZ+1];
wc_t *code = (wc_t*)&memory[0];
cell here, base, state, inSp;
cell vhere, block, curTask;
cell *dstk, *rstk, *lstk;
cell dsp, rsp, lsp, asp, bsp, tsp;
cell astk[STK_SZ+1], bstk[STK_SZ+1], tstk[STK_SZ+1];
DE_T tmpWords[10], *last;
char wd[32], *toIn, *inStk[FSTK_SZ+1];
TASK_T tasks[TASKS_SZ];

#define PRIMS_BASE \
	X(EXIT,    "exit",      0, if (0<rsp) { pc = (wc_t)rpop(); } else { return; } ) \
	X(DUP,     "dup",       0, t=TOS; push(t); ) \
	X(SWAP,    "swap",      0, t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",      0, pop(); ) \
	X(DUP2,    "2dup",      0, t=TOS; n=NOS; push(n); push(t); ) \
	X(DROP2,   "2drop",     0, pop(); pop(); ) \
	X(OVER,    "over",      0, t=NOS; push(t); ) \
	X(NIP,     "nip",       0, t=pop(); TOS=t; ) \
	X(TUCK,    "tuck",      0, t=TOS; TOS=NOS; NOS=t; push(t); ) \
	X(FET8,    "c@",        0, TOS = *(byte *)TOS; ) \
	X(FET16,   "w@",        0, TOS = fetch16(TOS); ) \
	X(FET32,   "@",         0, TOS = fetch32(TOS); ) \
	X(FETWC,   "wc@",       0, TOS = code[(wc_t)TOS]; ) \
	X(FETCV,   "cv@",       0, TOS = fetch32((cell)&code[(wc_t)TOS]); ) \
	X(STO8,    "c!",        0, t=pop(); n=pop(); *(byte*)t=(byte)n; ) \
	X(STO16,   "w!",        0, t=pop(); n=pop(); store16(t, n); ) \
	X(STO32,   "!",         0, t=pop(); n=pop(); store32(t, n); ) \
	X(STOWC,   "wc!",       0, t=pop(); n=pop(); code[(wc_t)t] = (wc_t)n; ) \
	X(STOCV,   "cv!",       0, t=pop(); n=pop(); store32((cell)&code[(wc_t)t], n); ) \
	X(IF_C4,   "if",        1, comma(JMPZ);  push(here); comma(0); ) \
	X(IF0_C4,  "if0",       1, comma(JMPNZ); push(here); comma(0); ) \
	X(IFN_C4,  "-if",       1, comma(NJMPZ); push(here); comma(0); ) \
	X(THEN_C4, "then",      1, code[(wc_t)pop()] = (wc_t)here; ) \
	X(BEGIN,   "begin",     1, push(here); ) \
	X(AGAIN,   "again",     1, comma(JMP);    comma(pop()); ) \
	X(WHILE,   "while",     1, comma(JMPNZ);  comma(pop()); ) \
	X(WHILEN,  "-while",    1, comma(NJMPNZ); comma(pop()); ) \
	X(UNTIL,   "until",     1, comma(JMPZ);   comma(pop()); )

#define PRIMS_MATH \
	X(ADD,     "+",         0, t=pop(); TOS += t; ) \
	X(SUB,     "-",         0, t=pop(); TOS -= t; ) \
	X(MUL,     "*",         0, t=pop(); TOS *= t; ) \
	X(STARSL,  "*/",        0, t=pop(); n = pop(); TOS = (TOS*n)/t; ) \
	X(DIV,     "/",         0, t=pop(); TOS /= t; ) \
	X(MOD,     "mod",       0, t=pop(); TOS %= t; ) \
	X(SLMOD,   "/mod",      0, t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(LSHIFT,  "<<",        0, t=pop(); TOS = (TOS << t); ) \
	X(RSHIFT,  ">>",        0, t=pop(); TOS = (TOS >> t); ) \
	X(INCR,    "1+",        0, ++TOS; ) \
	X(DECR,    "1-",        0, --TOS; ) \
	X(INCR2,   "2+",        0, TOS += 2; ) \
	X(MULT2,   "2*",        0, TOS *= 2; ) \
	X(DIV2,    "2/",        0, TOS /= 2; ) \
	X(CELLS,   "cells",     0, TOS *= CELL_SZ; ) \
	X(CELLPL,  "cell+",     0, TOS += CELL_SZ; ) \
	X(LT,      "<",         0, t=pop(); TOS = (TOS < t); ) \
	X(LTEQ,    "<=",        0, t=pop(); TOS = (TOS <= t); ) \
	X(EQ,      "=",         0, t=pop(); TOS = (TOS == t); ) \
	X(GTEQ,    ">=",        0, t=pop(); TOS = (TOS >= t); ) \
	X(GT,      ">",         0, t=pop(); TOS = (TOS > t); ) \
	X(EQ0,     "0=",        0, TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",       0, t=pop(); TOS &= t; ) \
	X(OR,      "or",        0, t=pop(); TOS |= t; ) \
	X(XOR,     "xor",       0, t=pop(); TOS ^= t; ) \
	X(COM,     "com",       0, TOS = ~TOS; ) \
	X(MIN_C4,  "min",       0, t=pop(); if (t < TOS) { TOS = t; } ) \
	X(MAX_C4,  "max",       0, t=pop(); if (t > TOS) { TOS = t; } ) \
	X(NEGATE,  "negate",    0, TOS = -TOS; ) \
	X(ABS,     "abs",       0, if (TOS < 0) { TOS = -TOS; } )

#define PRIMS_OTHER \
	X(FOR,     "for",       0, lsp+=3; L2=pc; L0=0; L1=pop(); ) \
	X(INDEX,   "i",         0, push(L0); ) \
	X(NEXT,    "next",      0, if (++L0<L1) { pc=(wc_t)L2; } else { lsp=(lsp<3) ? 0 : lsp-3; } ) \
	X(UNLOOP,  "unloop",    0, lsp=(lsp<3) ? 0 : lsp-3; ) \
	X(TOR,     ">r",        0, rpush(pop()); ) \
	X(RSTO,    "r!",        0, rstk[rsp] = pop(); ) \
	X(RAT,     "r@",        0, push(rstk[rsp]); ) \
	X(RATI,    "r@+",       0, push(rstk[rsp]++); ) \
	X(RATD,    "r@-",       0, push(rstk[rsp]--); ) \
	X(RFROM,   "r>",        0, push(rpop()); ) \
	X(RDROP,   "rdrop",     0, rpop(); ) \
	X(TOT,     ">t",        0, t=pop(); if (tsp < STK_SZ) { tstk[++tsp]=t; }; ) \
	X(TSTO,    "t!",        0, tstk[tsp] = pop(); ) \
	X(TAT,     "t@",        0, push(tstk[tsp]); ) \
	X(TATI,    "t@+",       0, push(tstk[tsp]++); ) \
	X(TATD,    "t@-",       0, push(tstk[tsp]--); ) \
	X(TFROM,   "t>",        0, push((0 < tsp) ? tstk[tsp--] : 0); ) \
	X(TDROP,   "tdrop",     0, if (0 < tsp) { tsp--; } ) \
	X(TOA,     ">a",        0, t=pop(); if (asp < STK_SZ) { astk[++asp] = t; } ) \
	X(ASET,    "a!",        0, astk[asp]=pop(); ) \
	X(AGET,    "a@",        0, push(astk[asp]); ) \
	X(AGETI,   "a@+",       0, push(astk[asp]++); ) \
	X(AGETD,   "a@-",       0, push(astk[asp]--); ) \
	X(AFET,    "@a",        0, t=astk[asp];   push(*(byte*)t); ) \
	X(AFETI,   "@a+",       0, t=astk[asp]++; push(*(byte*)t); ) \
	X(AFETD,   "@a-",       0, t=astk[asp]--; push(*(byte*)t); ) \
	X(ASTO,    "!a",        0, t=astk[asp];   n=pop(); *(byte*)t=(byte)n; ) \
	X(ASTOI,   "!a+",       0, t=astk[asp]++; n=pop(); *(byte*)t=(byte)n; ) \
	X(ASTOD,   "!a-",       0, t=astk[asp]--; n=pop(); *(byte*)t=(byte)n; ) \
	X(AFROM,   "a>",        0, push((0 < asp) ? astk[asp--] : 0); ) \
	X(ADROP,   "adrop",     0, if (0 < asp) { asp--; } ) \
	X(TOB,     ">b",        0, t=pop(); if (bsp < STK_SZ) { bstk[++bsp]=t; }; ) \
	X(BSET,    "b!",        0, bstk[bsp]=pop(); ) \
	X(BGET,    "b@",        0, push(bstk[bsp]); ) \
	X(BGETI,   "b@+",       0, push(bstk[bsp]++); ) \
	X(BGETD,   "b@-",       0, push(bstk[bsp]--); ) \
	X(BFET,    "@b",        0, t=bstk[bsp];   push(*(byte*)t); ) \
	X(BFETI,   "@b+",       0, t=bstk[bsp]++; push(*(byte*)t); ) \
	X(BFETD,   "@b-",       0, t=bstk[bsp]--; push(*(byte*)t); ) \
	X(BSTO,    "!b",        0, t=bstk[bsp];   n=pop(); *(byte*)t=(byte)n; ) \
	X(BSTOI,   "!b+",       0, t=bstk[bsp]++; n=pop(); *(byte*)t=(byte)n; ) \
	X(BSTOD,   "!b-",       0, t=bstk[bsp]--; n=pop(); *(byte*)t=(byte)n; ) \
	X(BFROM,   "b>",        0, push((0 < bsp) ? astk[bsp--] : 0); ) \
	X(BDROP,   "bdrop",     0, if (0 < bsp) { bsp--; } ) \

#define PRIMS_STRING \
	X(SLEN,    "s-len",     0, TOS = strLen((char*)TOS); ) \
	X(SCPY,    "s-cpy",     0, t=pop(); strCpy((char*)TOS, (char*)t); ) \
	X(SCAT,    "s-cat",     0, t=pop(); strCat((char*)TOS, (char*)t); ) \
	X(SEQ,     "s-eq",      0, t=pop(); TOS = strEq((char*)TOS, (char*)t); ) \
	X(SEQI,    "s-eqi",     0, t=pop(); TOS = strEqI((char*)TOS, (char*)t); ) \
	X(LTRIM,   "s-ltrim",   0, TOS = (cell)lTrim((char *)TOS); ) \
	X(RTRIM,   "s-rtrim",   0, rTrim((char *)TOS); ) \
	X(FILL,    "fill",      0, t=pop(); n=pop(); fill((byte*)pop(), n, (byte)t); ) \
	X(CMOVE,   "cmove",     0, t=pop(); n=pop(); cmove((byte*)pop(), (byte *)n, t); ) \
	X(CMOVEL,  "cmove>",    0, t=pop(); n=pop(); cmovel((byte*)pop(), (byte *)n, t); ) \
	X(LOWER,   "lower",     0, TOS = lower((char)TOS); ) \
	X(UPPER,   "upper",     0, TOS = upper((char)TOS); ) \
	X(ZQUOTE,  "z\"",       1, quote(); ) \
	X(DOTQT,   ".\"",       1, quote(); (state==COMPILE) ? comma(FTYPE) : fType((char*)pop()); )

#ifndef FILE_NONE
#define PRIMS_FILE \
	X(FLOPEN,  "fopen",       0, t=pop(); n=pop(); push(fileOpen((char*)n, (char*)t)); ) \
	X(FLCLOSE, "fclose",      0, t=pop(); fileClose(t); ) \
	X(FLDEL,   "fdelete",     0, t=pop(); fileDelete((char*)t); ) \
	X(FLREAD,  "fread",       0, t=pop(); n=pop(); TOS = fileRead((char*)TOS, (int)n, t); ) \
	X(FLWRITE, "fwrite",      0, t=pop(); n=pop(); TOS = fileWrite((char*)TOS, (int)n, t); ) \
	X(FSEEK,   "fseek",       0, t=pop(); TOS = fileSeek(t, TOS); ) \
	X(FPOS ,   "fpos",        0, TOS = filePos(TOS); ) \
	X(FSIZE,   "fsize",       0, TOS = fileSize(TOS); ) \
	X(LOADED,  "loaded?",     0, t=pop(); pop(); if (t) { toIn = inPop(); } ) \
	X(LOAD,    "load",        0, t=pop(); blockLoad((int)t); ) \
	X(NXTBLK,  "load-next",   0, t=pop(); blockLoadNext((int)t); ) \
	X(BCACHE,  "blocks",      0, dumpCache(); ) \
	X(BADDR,   "block-addr",  0, t=pop(); push((cell)blockAddr(t)); ) \
	X(BDIRTY,  "block-dirty", 0, t=pop(); blockIsDirty((int)t); ) \
	X(BCLEAN,  "block-clean", 0, t=pop(); blockIsClean((int)t); ) \
	X(FLUSH,   "flush",       0, flushBlocks(pop()); ) \
	X(FLBLK,   "flush-block", 0, t=pop(); n=pop(); flushBlock(n, 0, t); ) \
	X(EDIT,    "edit",        0, t=pop(); editBlock(t); )
#else
    #define PRIMS_FILE
#endif // FILE_NONE

#define PRIMS_C4A \
	X(EMIT,    "emit",      0, emit((char)pop()); ) \
	X(KEY,     "key",       0, push(key()); ) \
	X(QKEY,    "?key",      0, push(qKey()); ) \
	X(SEMI,    ";",         1, comma(EXIT); state=INTERP; ) \
	X(ZTYPE,   "ztype",     0, zType((char*)pop()); ) \
	X(FTYPE,   "ftype",     0, fType((char*)pop()); ) \
	X(LITC,    "lit,",      0, compileNum(pop()); ) \
	X(COMMA,   ",",         0, comma(pop()); ) \
	X(CONST,   "const",     0, addWord(0); compileNum(pop()); comma(EXIT); ) \
	X(VAR,     "var",       0, addWord(0); compileNum(vhere); comma(EXIT); vhere += pop(); ) \
	X(VAL,     "val",       0, addWord(0); comma(LIT); commaCell(0); comma(EXIT); ) \
	X(PVAL,    "(val)",     0, addWord(0); comma(LIT); commaCell((cell)&code[here-4]); comma(EXIT); ) \
	X(NEXTWD,  "next-wd",   0, push(nextWord()); ) \
	X(IMMED,   "immediate", 0, last->flg =_IMMED; ) \
	X(INLINE,  "inline",    0, last->flg =_INLINE; ) \
	X(OUTER,   "outer",     0, outer((char*)pop()); ) \
	X(ADDWORD, "addword",   0, addWord(0); ) \
	X(CLK,     "timer",     0, push(timer()); ) \
	X(SEE,     "see",       0, doSee(); ) \
	X(FIND,    "find",      0, { DE_T *dp=findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(ADDTASK, "add-task",  0, TOS = addTask(TOS); ) \
	X(DELTASK, "del-task",  0, delTask(pop()); ) \
	X(TASKS,   ".tasks",    0, dumpTasks(); ) \
	X(YIELD,   "yield",     0, pc = nextTask(pc); ) \
	X(TOMEM,   "->memory",  0, TOS += (cell)memory; ) \
	X(HERE,    "here",      0, push(here); ) \
	X(LAST,    "last",      0, push((cell)last); ) \
	X(VHERE,   "vhere",     0, push(vhere); ) \
	X(WORDS,   "words",     0, words(999999); ) \
	X(WORDSN,  "words-n",   0, words(pop()); ) \
	X(DOTN,    "(.)",       0, iToA(pop(), base); ) \
	X(DOT,     ".",         0, iToA(pop(), base); emit(32); ) \
	X(DOTS,    ".s",        0, dotS(); ) \
	X(ALLOT,   "allot",     0, vhere += pop(); ) \
	X(BLANK,   "bl",        0, push(32); ) \
	X(TAB,     "tab",       0, emit(9); ) \
	X(CR,      "cr",        0, emit(13); emit(10); ) \
	X(SPACE,   "space",     0, emit(32); )

#ifdef IS_PC
  #define PRIMS_SYSTEM \
	X(SYSTEM,  "system",    0, t=pop(); ttyMode(0); system((char*)t); ) \
	X(BYE,     "bye",       0, ttyMode(0); flushBlocks(0); exit(0); )
#else // Must be a dev board ...
  #define PRIMS_SYSTEM \
	X(POPENI,  "pin-input",  0, pinMode(pop(), INPUT); ) \
	X(POPENO,  "pin-output", 0, pinMode(pop(), OUTPUT); ) \
	X(POPENU,  "pin-pullup", 0, pinMode(pop(), INPUT_PULLUP); ) \
	X(PREADD,  "dpin@",      0, TOS = digitalRead(TOS); ) \
	X(PWRITED, "dpin!",      0, t=pop(); n=pop(); digitalWrite(t, n); ) \
	X(PREADA,  "apin@",      0, TOS = analogRead(TOS); ) \
	X(PWRITEA, "apin!",      0, t=pop(); n=pop(); analogWrite(t, n); ) \
	X(BYE,     "bye",        0, ttyMode(0); )
#endif // IS_PC

#define PRIMS_ALL PRIMS_BASE PRIMS_MATH PRIMS_OTHER PRIMS_STRING PRIMS_FILE PRIMS_C4A PRIMS_SYSTEM

#define X(op, name, imm, cod) op,

enum _PRIM  {
	STOP, LIT, JMP, JMPZ, NJMPZ, JMPNZ, NJMPNZ, PRIMS_ALL
};

#undef X
#define X(op, name, flg, code) { op, flg, 0, name },

PRIM_T prims[] = { PRIMS_ALL {0, 0, 0}};

void push(cell x) { if (dsp < STK_SZ) { dstk[++dsp] = x; } }
cell pop() { return (0<dsp) ? dstk[dsp--] : 0; }
void rpush(cell x) { if (rsp < STK_SZ) { rstk[++rsp] = x; } }
cell rpop() { return (0<rsp) ? rstk[rsp--] : 0; }
void inPush(char *in) { if (inSp < FSTK_SZ) { inStk[++inSp] = in; } }
char *inPop() { return (0 < inSp) ? inStk[inSp--] : 0; }
void comma(cell x) { code[here++] = (wc_t)x; }
void commaCell(cell n) { store32((cell)&code[here], n); here += (CELL_SZ/WC_SZ); }
int  changeState(int x) { state = x; return x; }
void ok() { if (state==0) { state=INTERP; } zType((state==INTERP) ? " ok\r\n" : "... "); }
int  lower(const char c) { return btwi(c, 'A', 'Z') ? c + 32 : c; }
int  upper(const char c) { return btwi(c, 'a', 'z') ? c - 32 : c; }
int  strLen(const char *s) { int l = 0; while (s[l]) { l++; } return l; }
void fill(byte *dst, cell num, byte ch) { while (0 < num--) { *(dst++) = ch; } }
void cmove(byte *src, byte *dst, cell num) { while (0 < num--) { *(dst++) = *(src++); } }
void cmovel(byte *src, byte *dst, cell num) {
	dst += (num-1);
	src += (num-1);
	while (0 < num--) { *(dst--) = *(src--); }
}

int strEqI(const char *s, const char *d) {
	while (lower(*s) == lower(*d)) { if (*s == 0) { return 1; } s++; d++; }
	return 0;
}

int strEq(const char *s, const char *d) {
	while (*s == *d) { if (*s == 0) { return 1; } s++; d++; }
	return 0;
}
	
void strCpy(char *d, const char *s) {
	while (*s) { *(d++) = *(s++); }
	*(d) = 0;
}

void strCat(char *d, const char *s) {
	strCpy(d + strLen(d), s);
}

char *lTrim(char *dst) {
	while ((*dst) && (*dst < 33)) { ++dst; }
	return dst;
}

void rTrim(char *dst) {
	char *cp = dst + strLen(dst);
	while ((dst <= cp) && (*cp < 33)) { *(cp--) = 0; }
}

int getWord() {
	int len = 0, ch;
	while (btwi(*toIn, 1, 32)) {
		ch = *(toIn++);
		if (btwi(ch,COMPILE,COMMENT)) { state = ch; }
	}
	while (btwi(*toIn, 33, 126)) { wd[len++] = *(toIn++); }
	wd[len] = 0;
	return len;
}

int nextWord() {
	while (toIn) {
		int len = getWord();
		if (len) { return len; }
		toIn = (char*)inPop();
	}
	return 0;
}

int isTemp(const char *w) {
	return ((w[0]=='t') && btwi(w[1],'0','9') && (w[2]==0)) ? 1 : 0;
}

DE_T *addWord(char *w) {
	if (!w) { nextWord(); w = wd; }
	if (w[0] == 0) { return 0; }
	if (NAME_LEN < strLen(wd)) { zTypeF("-wd/len:%s-\n", wd); wd[NAME_LEN]=0; }
	if (isTemp(w)) {
		tmpWords[w[1]-'0'].xt = here;
		return &tmpWords[w[1]-'0'];
	}
	int len = strLen(w);
	if (NAME_LEN < len) {
		zTypeF("-trunc:[%s]-",wd);
		wd[NAME_LEN] = 0;
		len = NAME_LEN;
	}
	DE_T *dp = --last;
	dp->xt = here;
	dp->flg = 0;
	dp->len = len;
	strCpy(dp->nm, w);
	// zTypeF("\n-add:%d,%d,[%s],(%d)-\n", last, dp->len, dp->nm, dp->xt);
	return dp;
}

PRIM_T *findPrim(const char *w) {
	PRIM_T *dp = &prims[0];
	// Check primitives first
	while (dp->op) {
		if (strEqI(dp->name, w)) { return dp; }
		++dp;
	}
	return (PRIM_T*)0;
}

DE_T *findWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) { return &tmpWords[w[1]-'0']; }
	int len = strLen(w);
	// Check primitives first
	PRIM_T *p = findPrim(w);
	if (p) { return (DE_T*)p; }

	// Now non-primitives
	DE_T *dp = (DE_T*)&memory[MEM_SZ];
	dp = last;
	while (BYE <= dp->xt) {
		if ((len == dp->len) && strEqI(dp->nm, w)) { return dp; }
		dp++;
	}
	return (DE_T*)0;
}

cell wordOut(DE_T *dp, PRIM_T *pp, cell n) {
	if (0 < n) {
		if (n % 10 == 0) { zType("\r\n"); }
		else { emit(9); }
	}
	const char *nm = (dp) ? &dp->nm[0] : pp->name;
	zType(nm);
	if (7 < strLen(nm)) { ++n; }
	return n+1;
}

void words(cell count) {
	cell num = 0, n = 0;
	DE_T *dp = last, *stop = (DE_T*)&memory[MEM_SZ];
	while (dp < stop) {
		n = wordOut(dp, NULL, n);
		if (++num >= count) { return; }
		dp++;
	}
	// zType("\r\n** primitives **\r\n");
	PRIM_T *pp = &prims[0];
	while (pp->op) {
		n = wordOut(NULL, pp, n);
		if (++num >= count) { return; }
		pp++;
	}
	zTypeF("\t(%d words)", num);
}

const char *nameByXT(wc_t xt) {
	PRIM_T* pp = &prims[0];
	while (pp->op) {
		if (pp->op == xt) { return pp->name; }
		pp++;
	}
	DE_T *dp = last, *end = (DE_T*)&memory[MEM_SZ];
	while (dp < end) {
		if (dp->xt == xt) { return &dp->nm[0]; }
		dp++;
	}
	return "<unknown>";
}

void doSee() {
	DE_T *dp = findWord(0);
	if (!dp) { zTypeF("-nf:%s-", wd); return; }
	if (dp->xt <= BYE) { zTypeF("%s is a primitive (#%ld/$%lX).\r\n", wd, dp->xt, dp->xt); return; }
	cell x = (cell)dp;
	wc_t i = dp->xt, stop = (last < dp) ? (dp-1)->xt : here;
	zTypeF("\r\n%08lX: %s (%04lX to %04lX)", (long)x, dp->nm, (long)dp->xt, (long)stop-1);
	while (i < stop) {
		wc_t op = code[i++];
		zTypeF("\r\n%04X: %04X\t", i-1, op);
		if (op & NUM_BITS) { op &= NUM_MASK; zTypeF("num #%ld ($%lX)", op, op); continue; }
		x = code[i];
		switch (op) {
			case  STOP: zType("stop"); i++;
			BCASE LIT: x = fetch32((cell)&code[i]);
				zTypeF("lit #%zd ($%zX)", (size_t)x, (size_t)x);
				i += (CELL_SZ/WC_SZ);
			BCASE JMP:    zTypeF("jmp $%04lX", (long)x);              i++;
			BCASE JMPZ:   zTypeF("jmpz $%04lX (IF?)", (long)x);       i++;
			BCASE NJMPZ:  zTypeF("njmpz $%04lX (-IF?)", (long)x);     i++;
			BCASE JMPNZ:  zTypeF("jmpnz $%04lX (WHILE?)", (long)x);   i++;
			BCASE NJMPNZ: zTypeF("njmpnz $%04lX (-WHILE?)", (long)x); i++; break;
			default: x = (cell)nameByXT((wc_t)op); 
				zType((char*)x);
		}
	}
}

void iToA(cell n, cell b) {
	if (n < 0) { emit('-'); n = -n; }
	if (b <= n) { iToA(n/b, b); }
	n %= b; if (9 < n) { n += 7; }
	emit('0'+(char)n);
}

void dotS() {
	zType("( ");
	for (int i = 1; i <= dsp; i++) { iToA(dstk[i], base); emit(32); }
	zType(")");
}

void fType(const char *s) {
	while (*s) {
		char c = *(s++);
		if (c=='%') {
			c = *(s++);
			switch (c) {
				case  'B': Blue();
				BCASE 'G': Green();
				BCASE 'P': Purple();
				BCASE 'R': Red();
				BCASE 'W': White();
				BCASE 'Y': Yellow();
				BCASE 'b': iToA(pop(),2);
				BCASE 'c': emit((char)pop());
				BCASE 'd': iToA(pop(),10);
				BCASE 'e': emit(27);
				BCASE 'i': iToA(pop(),base);
				BCASE 'n': emit(13); emit(10);
				BCASE 'q': emit('"');
				BCASE 's': fType((char*)pop());
				BCASE 'S': zType((char*)pop());
				BCASE 'x': iToA(pop(),16); break;
				default: emit(c); break;
			}
		} else { emit(c); }
	}
}

void compileNum(cell num) {
	if (btwi(num, 0, NUM_MASK)) { comma((wc_t)(num | NUM_BITS)); }
	else { comma(LIT); commaCell(num); }
}

void quote() {
	char *vh = (char*)vhere;
	if (*toIn) { ++toIn; }
	while (*toIn) {
		if (*toIn == '"') { ++toIn; break; }
		*(vh++) = *(toIn++);
	}
	*(vh++) = 0; // NULL terminator
	push(vhere);
	if (state == COMPILE) {
		compileNum(pop());
		vhere = (cell)vh;
	}
}

cell addTask(cell xt) {
	for (int i = 1; i < TASKS_SZ; i++) {
		if (tasks[i].status == 0) { tasks[i].pc = xt; tasks[i].status = 1; return i; }
	}
	zType("-tasks-full-");
	return 0;
}

void delTask(cell taskNum) {
	if (taskNum < 1) { zType("-sys-task-"); return; }
	if (TASK_MAX < taskNum) { zType("-task-range-"); return; }
	tasks[taskNum].status = 0;
	curTask = 0;
}

void dumpTasks() {
	zType(" # PC   St DSP RSP LSP\n");
	zType("-- ---- -- --- --- ---\n");
	for (int i = 0; i < TASKS_SZ; i++) {
		TASK_T *t = &tasks[i];
		zTypeF("%2d %04X %2d", i, t->pc, t->status);
		zTypeF(" %3d %3d %3d", t->stks[0].sp, t->stks[1].sp, t->stks[2].sp);
		zType("\n");
	}
}

void setTask(cell tsk) {
	SP(STK_DATA) = dsp;
	SP(STK_RETN) = rsp;
	SP(STK_LSTK) = lsp;
	curTask = tsk;
	dsp = SP(STK_DATA); dstk = STK(STK_DATA);
	rsp = SP(STK_RETN); rstk = STK(STK_RETN);
	lsp = SP(STK_LSTK); lstk = STK(STK_LSTK);
}

wc_t nextTask(wc_t pc) {
	cell nt = 0;
	tasks[curTask].pc = pc;
	for (int i = curTask+1; i < TASKS_SZ; i++) { if (tasks[i].status == 1) { nt = i; break; } }
	setTask(nt);
	return tasks[curTask].pc;
}

#undef X
#define X(op, name, imm, code) NCASE op: code

void inner(wc_t start) {
	cell t, n;
	wc_t pc = start, wc;
next:
	wc = code[pc++];
	switch(wc) {
		case  STOP:   return;
		NCASE LIT:    push(fetch32((cell)&code[pc])); pc += (CELL_SZ/WC_SZ);
		NCASE JMP:    pc=code[pc];
		NCASE JMPZ:   if (pop()==0) { pc=code[pc]; } else { ++pc; }
		NCASE NJMPZ:  if (TOS==0) { pc=code[pc]; } else { ++pc; }
		NCASE JMPNZ:  if (pop()) { pc=code[pc]; } else { ++pc; }
		NCASE NJMPNZ: if (TOS) { pc=code[pc]; } else { ++pc; }
		PRIMS_ALL
		goto next; default:
			if ((wc & NUM_BITS) == NUM_BITS) { push(wc & NUM_MASK); goto next; }
			if (code[pc] != EXIT) { rpush(pc); }
			pc = wc;
			goto next;
	}
}

int isNum(const char *w, int b) {
	cell n=0, isNeg=0;
	if ((w[0]==39) && (w[2]==39) && (w[3]==0)) { push(w[1]); return 1; }
	if (w[0]=='%') { b= 2; ++w; }
	if (w[0]=='#') { b=10; ++w; }
	if (w[0]=='$') { b=16; ++w; }
	if ((b==10) && (w[0]=='-')) { isNeg=1; ++w; }
	if (w[0]==0) { return 0; }
	char c = lower(*(w++));
	while (c) {
		n = (n*b);
		if (btwi(c,'0','9') &&  btwi(c,'0','0'+b-1)) { n+=(c-'0'); }
		else if (btwi(c,'a','a'+b-11)) { n+=(c-'a'+10); }
		else return 0;
		c = lower(*(w++));
	}
	push(isNeg ? -n : n);
	return 1;
}

void executeWord(DE_T *de) {
	code[17] = de->xt;
	code[18] = STOP;
	inner(17);
}

void compileWord(DE_T *de) {
	if (de->flg & _IMMED) { executeWord(de); }
	else if (de->flg & _INLINE) {
		wc_t x = de->xt;
		do { comma(code[x++]); } while (code[x]!=EXIT);
	} else { comma(de->xt); }
}

int isStateChange(const char *wd) {
	static int prevState = INTERP;
	if (prevState == COMMENT) { prevState = INTERP; }
	if (strEq(wd,")")) { return changeState(prevState); }
	if (state == COMMENT) { return 0; }
	if (strEq(wd,":")) { return changeState(DEFINE); }
	if (strEq(wd,"[")) { return changeState(INTERP); }
	if (strEq(wd,"]")) { return changeState(COMPILE); }
	if (strEq(wd,"(")) { prevState=state; return changeState(COMMENT); }
	return 0;
}

void outer(const char *ln) {
	// zTypeF("-outer:%s-\n",ln);
	inPush(toIn);
	toIn = (char*)ln;
	while (nextWord()) {
		// zTypeF("-word:(%s,%d)-",wd,state);
		if (isStateChange(wd)) { continue; }
		if (state == COMMENT) { continue; }
		if (state == DEFINE) { addWord(wd); state = COMPILE; continue; }
		if (isNum(wd, base)) {
			if (state == COMPILE) { compileNum(pop()); }
			continue;
		}
		DE_T *de = findWord(wd);
		if (de) {
			if (state == COMPILE) { compileWord(de); }
			else { executeWord(de); }
			continue;
		}
		zTypeF("-%s?-", wd);
		if (inputFp) { zTypeF(" at\r\n\t%s", ln); }
		if (inputFp) { fileClose(inputFp); inputFp = 0; }
		while (toIn) { toIn = inPop(); }
		state = INTERP;
		return;
	}
	toIn = inPop();
}

void outerF(const char *fmt, ...) {
	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 128, fmt, args);
	va_end(args);
	outer(buf);
}

void zTypeF(const char *fmt, ...) {
	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 128, fmt, args);
	va_end(args);
	zType(buf);
}

void defineNum(const char *name, cell val) {
	DE_T *dp = addWord((char*)name);
	compileNum(val);
	if (btwi(val, 0, NUM_MASK)) { dp->flg=_INLINE; }
	comma(EXIT);
}

void baseSys() {
	//for (int i = 0; prims[i].name; i++) {
	//	DE_T *w = addWord((char*)prims[i].name);
	//	w->xt = prims[i].op;
	//	w->flg = prims[i].flg;
	//}

	defineNum("mem-sz",   MEM_SZ);
	defineNum("block-sz", BLOCK_SZ);
	defineNum("code-sz",  CODE_SLOTS);
	defineNum("de-sz",    sizeof(DE_T));
	defineNum("dstk-sz",  STK_SZ+1);
	defineNum("wc-sz",    WC_SZ);
	defineNum("tasks-sz", TASKS_SZ);
	defineNum("cur-task", (cell)&curTask);

	defineNum("(dsp)",   (cell)&dsp);
	defineNum("(rsp)",   (cell)&rsp);
	defineNum("(asp)",   (cell)&asp);
	defineNum("(bsp)",   (cell)&bsp);
	defineNum("(tsp)",   (cell)&tsp);
	defineNum("(lsp)",   (cell)&lsp);
	
	defineNum("dstk",    (cell)&dstk[0]);
	defineNum("rstk",    (cell)&rstk[0]);
	defineNum("astk",    (cell)&astk[0]);
	defineNum("bstk",    (cell)&bstk[0]);
	defineNum("tstk",    (cell)&tstk[0]);
	defineNum("lstk",    (cell)&lstk[0]);

	defineNum("memory",      (cell)&memory[0]);
	defineNum(">in",         (cell)&toIn);
	defineNum("wd",          (cell)&wd[0]);
	defineNum("block",       (cell)&block);
	defineNum("(vhere)",     (cell)&vhere);
	defineNum("(output-fp)", (cell)&outputFp);
	defineNum("(last)",      (cell)&last);

	defineNum("version",  VERSION);
	defineNum("(lit)",    LIT);
	defineNum("(jmp)",    JMP);
	defineNum("(jmpz)",   JMPZ);
	defineNum("(njmpz)",  NJMPZ);
	defineNum("(jmpnz)",  JMPNZ);
	defineNum("(njmpnz)", NJMPNZ);
	defineNum("(exit)",   EXIT);
	defineNum("(here)",   (cell)&here);
	defineNum("vars",     vhere);
	defineNum("base",     (cell)&base);
	defineNum("state",    (cell)&state);
	defineNum("cell",     CELL_SZ);
}

void c4Init() {
	code = (wc_t*)&memory[0];
	vhere = (cell)&code[CODE_SLOTS];
	fill((byte *)tasks, sizeof(tasks), 0);
	tasks[0].status = 1;
	setTask(0);
	here = BYE+1;
	last = (DE_T*)&memory[MEM_SZ];
	base = 10;
	state = INTERP;
	inSp = block = asp = bsp = tsp = 0;
	for (int i = 6; i <= 9; i++) { tmpWords[i].flg = _INLINE; }
	fileInit();
	baseSys();
	sys_load();
}
