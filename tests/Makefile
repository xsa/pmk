# $Id$

CC?=		cc

CFLAGS?=
CFLAGS+=	-Wall

LDFLAGS?=


all: hash dynary

.c.o:
	$(CC) $(CFLAGS) -c $<

hash: hash_test.o
	$(CC) -o hash_test $(LDFLAGS) hash_test.o

dynary: da_test.o
	$(CC) -o da_test $(LDFLAGS) da_test.o

clean:
	rm -f *.o *_test *.core

lint:
	lint hash_test.c
