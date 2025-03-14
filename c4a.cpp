#include "c4a.h"

#define STK(x)        tasks[curTask].stks[x].stk
#define SP(x)         tasks[curTask].stks[x].sp

#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define NCASE         goto next; case
#define BCASE         break; case

byte memory[MEM_SZ+1];
wc_t *code = (wc_t*)&memory[0];
cell here, base, state, inSp, last;
cell vhere, block, curTask;
cell *dstk, *rstk, *lstk;
cell dsp, rsp, lsp, asp, bsp, tsp;
cell astk[STK_SZ+1], bstk[STK_SZ+1], tstk[STK_SZ+1];
DE_T tmpWords[10];
char wd[32], *toIn, *inStk[FSTK_SZ+1];
TASK_T tasks[TASKS_SZ];

#define PRIMS_BASE \
	X(EXIT,    "exit",      0, if (0<rsp) { pc = (wc_t)rpop(); } else { return; } ) \
	X(DUP,     "dup",       0, t=TOS; push(t); ) \
	X(SWAP,    "swap",      0, t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",      0, pop(); ) \
	X(OVER,    "over",      0, t=NOS; push(t); ) \
	X(FET8,    "c@",        0, TOS = *(byte *)TOS; ) \
	X(FET16,   "w@",        0, TOS = fetch16(TOS); ) \
	X(FET32,   "@",         0, TOS = fetch32(TOS); ) \
	X(FETWC,   "wc@",       0, TOS = code[(wc_t)TOS]; ) \
	X(STO8,    "c!",        0, t=pop(); n=pop(); *(byte*)t=(byte)n; ) \
	X(STO16,   "w!",        0, t=pop(); n=pop(); store16(t, n); ) \
	X(STO32,   "!",         0, t=pop(); n=pop(); store32(t, n); ) \
	X(STOWC,   "wc!",       0, t=pop(); n=pop(); code[(wc_t)t] = (wc_t)n; ) \
	X(ADD,     "+",         0, t=pop(); TOS += t; ) \
	X(SUB,     "-",         0, t=pop(); TOS -= t; ) \
	X(MUL,     "*",         0, t=pop(); TOS *= t; ) \
	X(DIV,     "/",         0, t=pop(); TOS /= t; ) \
	X(SLMOD,   "/mod",      0, t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INCR,    "1+",        0, ++TOS; ) \
	X(DECR,    "1-",        0, --TOS; ) \
	X(LT,      "<",         0, t=pop(); TOS = (TOS < t); ) \
	X(EQ,      "=",         0, t=pop(); TOS = (TOS == t); ) \
	X(GT,      ">",         0, t=pop(); TOS = (TOS > t); ) \
	X(EQ0,     "0=",        0, TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",       0, t=pop(); TOS &= t; ) \
	X(OR,      "or",        0, t=pop(); TOS |= t; ) \
	X(XOR,     "xor",       0, t=pop(); TOS ^= t; ) \
	X(COM,     "com",       0, TOS = ~TOS; ) \
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
	X(TTO,     ">t",        0, t=pop(); if (tsp < STK_SZ) { tstk[++tsp]=t; }; ) \
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
	X(AFROM,   "a>",        0, push((0 < asp) ? astk[asp--] : 0); ) \
	X(ADROP,   "adrop",     0, if (0 < asp) { asp--; } ) \
	X(BTO,     ">b",        0, t=pop(); if (bsp < STK_SZ) { bstk[++bsp]=t; }; ) \
	X(BSET,    "b!",        0, bstk[bsp]=pop(); ) \
	X(BGET,    "b@",        0, push(bstk[bsp]); ) \
	X(BGETI,   "b@+",       0, push(bstk[bsp]++); ) \
	X(BGETD,   "b@-",       0, push(bstk[bsp]--); ) \
	X(BFROM,   "b>",        0, push((0 < bsp) ? astk[bsp--] : 0); ) \
	X(BDROP,   "bdrop",     0, if (0 < bsp) { bsp--; } ) \
	X(EMIT,    "emit",      0, emit((char)pop()); ) \
	X(KEY,     "key",       0, push(key()); ) \
	X(QKEY,    "?key",      0, push(qKey()); ) \
	X(SEMI,    ";",         1, comma(EXIT); state=INTERP; ) \
	X(LITC,    "lit,",      0, compileNum(pop()); ) \
	X(NEXTWD,  "next-wd",   0, push(nextWord()); ) \
	X(IMMED,   "immediate", 0, { DE_T *dp = (DE_T*)&memory[last]; dp->flg=_IMMED; } ) \
	X(INLINE,  "inline",    0, { DE_T *dp = (DE_T*)&memory[last]; dp->flg=_INLINE; } ) \
	X(OUTER,   "outer",     0, outer((char*)pop()); ) \
	X(ADDWORD, "addword",   0, addWord(0); ) \
	X(CLK,     "timer",     0, push(timer()); ) \
	X(SEE,     "see",       0, doSee(); ) \
	X(ZTYPE,   "ztype",     0, zType((char*)pop()); ) \
	X(FTYPE,   "ftype",     0, fType((char*)pop()); ) \
	X(SCPY,    "s-cpy",     0, t=pop(); strCpy((char*)TOS, (char*)t); ) \
	X(SEQ,     "s-eq",      0, t=pop(); TOS = strEq((char*)TOS, (char*)t); ) \
	X(SEQI,    "s-eqi",     0, t=pop(); TOS = strEqI((char*)TOS, (char*)t); ) \
	X(SLEN,    "s-len",     0, TOS = strLen((char*)TOS); ) \
	X(ZQUOTE,  "z\"",       1, quote(); ) \
	X(DOTQT,   ".\"",       1, quote(); (state==COMPILE) ? comma(FTYPE) : fType((char*)pop()); ) \
	X(FIND,    "find",      0, { DE_T *dp=findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(ADDTASK, "add-task",  0, TOS = addTask(TOS); ) \
	X(DELTASK, "del-task",  0, delTask(pop()); ) \
	X(YIELD,   "yield",     0, pc = nextTask(pc); )

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

#ifdef IS_PC
  #define PRIMS_SYSTEM \
	X(SYSTEM,  "system", 0, t=pop(); ttyMode(0); system((char*)t); ) \
	X(BYE,     "bye",    0, ttyMode(0); flushBlocks(0); exit(0); )
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

#define X(op, name, imm, cod) op,

enum _PRIM  {
	STOP, LIT, JMP, JMPZ, NJMPZ, JMPNZ, NJMPNZ, PRIMS_BASE PRIMS_FILE PRIMS_SYSTEM
};

#undef X
#define X(op, name, imm, code) { name, op, imm, 0 },

PRIM_T prims[] = { PRIMS_BASE PRIMS_FILE PRIMS_SYSTEM {0, 0, 0}};

void push(cell x) { if (dsp < STK_SZ) { dstk[++dsp] = x; } }
cell pop() { return (0<dsp) ? dstk[dsp--] : 0; }
void rpush(cell x) { if (rsp < STK_SZ) { rstk[++rsp] = x; } }
cell rpop() { return (0<rsp) ? rstk[rsp--] : 0; }
void inPush(char *in) { if (inSp < FSTK_SZ) { inStk[++inSp] = in; } }
char *inPop() { return (0 < inSp) ? inStk[inSp--] : 0; }
int  lower(const char c) { return btwi(c, 'A', 'Z') ? c + 32 : c; }
int  strLen(const char *s) { int l = 0; while (s[l]) { l++; } return l; }
void comma(cell x) { code[here++] = (wc_t)x; }
void commaCell(cell n) { store32((cell)&code[here], n); here += (CELL_SZ/WC_SZ); }
int  changeState(int x) { state = x; return x; }
void ok() { if (state==0) { state=INTERP; } zType((state==INTERP) ? " ok\r\n" : "... "); }

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
	last -= sizeof(DE_T);
	DE_T *dp = (DE_T*)&memory[last];
	dp->xt = here;
	dp->flg = 0;
	dp->len = len;
	strCpy(dp->nm, w);
	// zTypeF("\n-add:%d,%d,[%s],(%d)-\n", last, dp->len, dp->nm, dp->xt);
	return dp;
}

DE_T *findWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) { return &tmpWords[w[1]-'0']; }
	int len = strLen(w);
	DE_T *dp = (DE_T*)&memory[MEM_SZ];
	// Check primitives first
	while ((--dp)->xt < BYE) {
		if ((len == dp->len) && strEqI(dp->nm, w)) { return dp; }
	}
	// Now non-primitives
	dp = (DE_T*)&memory[last];
	while (BYE <= dp->xt) {
		if ((len == dp->len) && strEqI(dp->nm, w)) { return dp; }
		dp++;
	}
	return (DE_T*)0;
}

int findXT(wc_t xt) {
	cell cw = last;
	while (cw < MEM_SZ) {
		DE_T *dp = (DE_T*)&memory[cw];
		if (dp->xt == xt) { return cw; }
		cw += sizeof(DE_T);
	}
	return 0;
}

void doSee() {
	DE_T *dp = findWord(0), *lastWord = (DE_T*)&memory[last];
	if (!dp) { zTypeF("-nf:%s-", wd); return; }
	if (dp->xt <= BYE) { zTypeF("%s is a primitive (#%ld/$%lX).\r\n", wd, dp->xt, dp->xt); return; }
	cell x = (cell)dp-(cell)memory;
	int i = dp->xt, stop = (lastWord < dp) ? (dp-1)->xt : here;
	zTypeF("\r\n%04lX: %s (%04lX to %04lX)", (long)x, dp->nm, (long)dp->xt, (long)stop-1);
	while (i < stop) {
		long op = code[i++];
		zTypeF("\r\n%04X: %04X\t", i-1, op);
		if (op & NUM_BITS) { op &= NUM_MASK; zTypeF("num #%ld ($%lx)", op, op); continue; }
		x = code[i];
		switch (op) {
			case  STOP: zType("stop"); i++;
			BCASE LIT: x = fetch32((cell)&code[i]);
				zTypeF("lit #%zd ($%zX)", (size_t)x, (size_t)x);
				i += (CELL_SZ/WC_SZ);
			BCASE JMP:    zTypeF("jmp $%04lX", (long)x);             i++;
			BCASE JMPZ:   zTypeF("jmpz $%04lX (IF?)", (long)x);       i++;
			BCASE NJMPZ:  zTypeF("njmpz $%04lX (-IF?)", (long)x);     i++;
			BCASE JMPNZ:  zTypeF("jmpnz $%04lX (WHILE?)", (long)x);   i++; break;
			BCASE NJMPNZ: zTypeF("njmpnz $%04lX (-WHILE?)", (long)x); i++; break;
			default: x = findXT((wc_t)op); 
				zType(x ? ((DE_T*)&memory[x])->nm : "<unknown>");
		}
	}
}

void iToA(cell n, cell b) {
	if (n<0) { emit('-'); n = -n; }
	if (b<=n) { iToA(n/b, b); }
	n %= b; if (9<n) { n += 7; }
	emit('0'+(char)n);
}

void fType(const char *s) {
	while (*s) {
		char c = *(s++);
		if (c=='%') {
			c = *(s++);
			switch (c) {
				case  'b': iToA(pop(),2);
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
	char *vh=(char*)vhere;
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

wc_t nextTask(wc_t pc) {
	cell nt = 0;
	for (int i = curTask+1; i < TASKS_SZ; i++) { if (tasks[i].status == 1) { nt = i; break; } }
	for (int i = 0; i < curTask; i++) { if (tasks[i].status == 1) { nt = i; break; } }
	tasks[curTask].pc = pc;
	SP(STK_DATA) = dsp; SP(STK_RETN) = rsp; SP(STK_LSTK) = lsp;
	curTask = nt;
	dsp = SP(STK_DATA); dstk = STK(STK_DATA);
	rsp = SP(STK_RETN); rstk = STK(STK_RETN);
	lsp = SP(STK_LSTK); lstk = STK(STK_LSTK);
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
		PRIMS_BASE PRIMS_FILE PRIMS_SYSTEM
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
	if (isNeg) { n = -n; }
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
	for (int i = 0; prims[i].name; i++) {
		DE_T *w = addWord((char*)prims[i].name);
		w->xt = prims[i].op;
		w->flg = prims[i].fl;
	}

	defineNum("mem-sz",   MEM_SZ);
	defineNum("block-sz", BLOCK_SZ);
	defineNum("code-sz",  CODE_SLOTS);
	defineNum("de-sz",    sizeof(DE_T));
	defineNum("dstk-sz",  STK_SZ+1);
	defineNum("wc-sz",    WC_SZ);
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
	for (int i = 0; i < TASKS_SZ; i++) {
		tasks[i].status = 0;
		for (int j = 0; j < 6; j++) { tasks[i].stks[j].sp = 0; }
	}
	tasks[0].status = 1;
	nextTask(0);
	here = BYE+1;
	last = MEM_SZ;
	base = 10;
	state = INTERP;
	inSp = block = asp = bsp = tsp = 0;
	for (int i = 6; i <= 9; i++) { tmpWords[i].flg = _INLINE; }
	fileInit();
	baseSys();
	sys_load();
}
