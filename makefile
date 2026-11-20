srcfiles := $(shell find . -name "*.cpp")
incfiles := $(shell find . -name "*.h")

c4a: $(srcfiles) $(incfiles)
	$(CC) -O3 -m32 $(srcfiles) -DIS_LINUX=1 -o c4a

c4a64: $(srcfiles) $(incfiles)
	$(CC) -O3 -m64 $(srcfiles) -DIS_LINUX=1 -o c4a

clean:
	rm -f c4a

run: c4a
	./c4a

test: c4a
	./c4a hello.fth

bin: c4a
	cp -u -p c4a ~/bin/

disk: blocks.c4a
	cp -u -p blocks.c4a ~/bin/
