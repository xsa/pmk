# $Id$

CC?=		cc
INSTALL?=	install

CFLAGS?=
#CFLAGS+=	-DSYSCONFDIR=\"/etc/\"
PREFIX?=	/usr/local

PNAME=		pmk
DATADIR=	$(PREFIX)/share/$(PNAME)
SAMPLE=		$(PNAME)file.sample
CONFIG=		$(PNAME).conf.sample

SRCS=		pmk.c pmk.h
OBJS=		pmk.o

$(PNAME): $(OBJS)
	$(CC) -o $(PNAME) $(OBJS) $(CFLAGS)

all: $(PNAME)

install:
	$(INSTALL) -m 755 $(PNAME) $(PREFIX)/bin/
	$(INSTALL) -d $(DATADIR)
	$(INSTALL) -m 644 $(SAMPLE) $(DATADIR)
	$(INSTALL) -m 644 $(CONFIG) $(DATADIR)

clean:
	rm -f $(OBJS) $(PNAME)

deinstall:
	rm -f $(PREFIX)/bin/$(PNAME)
	rm -rf $(PREFIX)/share/$(PNAME)

