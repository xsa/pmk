# $Id$

CC?=		cc

CFLAGS?=
CFLAGS+=	-Wall
#CFLAGS+=	-Werror
#CFLAGS+=	-ansi -pedantic

LDFLAGS?=
DEBUG?=		-g

#LINT_TARGET=	da_test.c

all: hash dynary path parser mkvar

.c.o:
	$(CC) $(CFLAGS) -c $<

hash: hash_test.o
	$(CC) -o hash_test $(LDFLAGS) hash_test.o

dynary: da_test.o
	$(CC) -o da_test $(LDFLAGS) da_test.o

path: path_test.o
	$(CC) -o path_test $(LDFLAGS) path_test.o

parser: parser_test.o
	$(CC) -o parser_test $(LDFLAGS) parser_test.o

mkvar: mkvar_test.o
	$(CC) -o mkvar_test $(LDFLAGS) mkvar_test.o

clean:
	rm -f *.o *_test *.core

#lint:
#.ifdef LINT_TARGET
#	lint $(LINT_TARGET)
#.else
#	@echo "LINT_TARGET not set."
#.endif
