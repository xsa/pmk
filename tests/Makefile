# $Id$

CC?=		cc

CFLAGS?=
CFLAGS+=	-Wall

LDFLAGS?=


all: hash_test

.c.o:
	$(CC) $(CFLAGS) -c $<

hash_test: hash_test.o
	$(CC) -o hash_test $(LDFLAGS) hash_test.o

clean:
	rm -f hash_test.o hash_test *.core

lint:
	lint hash_test.c
