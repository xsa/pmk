# $Id$

CC?=		cc

CFLAGS?=	-Wall
#CFLAGS=	-Werror
#CFLAGS=	-ansi -pedantic

LDFLAGS?=
DEBUG?=		-g

#LINT_TARGET=	da_test.c

all: hash dynary path parser mkvar comp

.c.o:
	$(CC) $(CFLAGS) -c $<

config:
	@if [ ! -f ../config ]; then \
		echo "Not configured, run \
		'cd ../ && make clean cfgum' first."; \
		exit 1; \
	fi

hash: config hash_test.o
	$(CC) -o hash_test $(LDFLAGS) hash_test.o

dynary: config da_test.o
	$(CC) -o da_test $(LDFLAGS) da_test.o

path: config path_test.o
	$(CC) -o path_test $(LDFLAGS) path_test.o

parser: config parser_test.o
	$(CC) -o parser_test $(LDFLAGS) parser_test.o

mkvar: config mkvar_test.o
	$(CC) -o mkvar_test $(LDFLAGS) mkvar_test.o

comp: config comp_test.o
	$(CC) -o comp_test $(LDFLAGS) comp_test.o

clean:
	rm -f *.o *_test *.core

#lint:
#.ifdef LINT_TARGET
#	lint $(LINT_TARGET)
#.else
#	@echo "LINT_TARGET not set."
#.endif
