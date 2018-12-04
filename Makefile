COMP    = gcc
CFLAGS  = -g -Wall -std=c11
RM      = rm -f
BINNAME = final.bin
FILES   = main.c util.c cd.c pwd.c quit.c ls.c mkdir.c ialloc_balloc.c link.c

default: build

run: build
	./$(BINNAME)

build: $(FILES)
	$(COMP) $(CFLAGS) -o $(BINNAME) $(FILES)

clean:
	$(RM) $(BINNAME)
