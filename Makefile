COMP    = gcc
CFLAGS  = -g -Wall
RM      = rm -f
BINNAME = final.bin
FILES   = main.c util.c cd.c pwd.c

default: build

build: $(FILES)
	$(COMP) $(CFLAGS) -o $(BINNAME) $(FILES)

clean:
	$(RM) $(BINNAME)
