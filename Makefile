COMP    = gcc
CFLAGS  = -g -Wall
RM      = rm -f
BINNAME = final.bin
FILES   = main.c util.c cd.c pwd.c quit.c ls.c

default: build

run: build
	./$(BINNAME)

build: $(FILES)
	$(COMP) $(CFLAGS) -o $(BINNAME) $(FILES)

clean:
	$(RM) $(BINNAME)
