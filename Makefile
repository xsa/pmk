# $Id$

CC?=		cc
INSTALL?=	install

CFLAGS?=
CFLAGS+=	-Wall
#CFLAGS+=	-Werror
#CFLAGS+=	-ansi -pedantic

# Debug stuff
#CFLAGS+=	-DPMK_DEBUG
#CFLAGS+=	-DPMKSETUP_DEBUG

# Here you can change the default location of pmk.conf
#CFLAGS+=	-DSYSCONFDIR=\"/etc/\"

LDFLAGS?=

PREFIX?=	/usr/local
DEBUG?=		-g

PREMAKE=	pmk
SETUP=		pmksetup

DATADIR=	$(PREFIX)/share/$(PREMAKE)
SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

P_OBJS=		common.o hash.o func.o pmk.o
S_OBJS=		$(SETUP).o common.o hash.o readconf.o

all: $(PREMAKE) $(SETUP)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(PREMAKE): $(P_OBJS)
	$(CC) -o $(PREMAKE) $(LDFLAGS) $(P_OBJS)

$(SETUP): $(S_OBJS)
	$(CC) -o $(SETUP) $(LDFLAGS) $(S_OBJS)

install: pmk pmksetup
	$(INSTALL) -m 755 $(PREMAKE) $(PREFIX)/bin/
	$(INSTALL) -m 755 $(SETUP) $(PREFIX)/sbin/
	$(INSTALL) -d $(DATADIR)
	$(INSTALL) -m 644 samples/$(SAMPLE) $(DATADIR)
	$(INSTALL) -m 644 samples/$(CONFIG) $(DATADIR)

clean:
	rm -f $(P_OBJS) $(S_OBJS) $(PREMAKE) $(SETUP) *.core

deinstall:
	rm -f $(PREFIX)/bin/$(PREMAKE)
	rm -f $(PREFIX)/sbin/$(SETUP)
	rm -rf $(PREFIX)/share/$(PREMAKE)

test_pmk: pmk
	@echo "Testing pmk with sample file"
	pmk -f samples/pmkfile.sample
