# $Id$

CC?=		cc
INSTALL?=	install

CFLAGS?=
#CFLAGS+=	-Wall
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

P_OBJS=		common.o hash.o func.o pmk.o dynarray.o
S_OBJS=		$(SETUP).o common.o hash.o dynarray.o

all: config $(PREMAKE) $(SETUP)

.c.o:
	$(CC) $(CFLAGS) -c $<

config:
	@CC=$(CC) sh pmkcfg.sh

$(PREMAKE): $(P_OBJS)
	$(CC) -o $(PREMAKE) $(LDFLAGS) $(P_OBJS)

$(SETUP): $(S_OBJS)
	$(CC) -o $(SETUP) $(LDFLAGS) $(S_OBJS)

install: pmk pmksetup
	$(INSTALL) -m 755 $(PREMAKE) $(PREFIX)/bin/$(PREMAKE)
	$(INSTALL) -m 755 $(SETUP) $(PREFIX)/sbin/$(SETUP)
	$(INSTALL) -d $(DATADIR)
	$(INSTALL) -m 644 samples/$(SAMPLE) $(DATADIR)
	$(INSTALL) -m 644 samples/$(CONFIG) $(DATADIR)
	$(INSTALL) -m 444 $(PREMAKE).1 $(PREFIX)/man/man1/$(PREMAKE).1
	$(INSTALL) -m 444 $(SETUP).8 $(PREFIX)/man/man8/$(SETUP).8

clean:
	rm -f $(P_OBJS) $(S_OBJS) $(PREMAKE) $(SETUP) compat/compat.h *.core

deinstall:
	rm -f $(PREFIX)/bin/$(PREMAKE)
	rm -f $(PREFIX)/sbin/$(SETUP)
	rm -rf $(PREFIX)/share/$(PREMAKE)
	rm -f $(PREFIX)/man/man1/$(PREMAKE).1
	rm -f $(PREFIX)/man/man8/$(SETUP).8

test_pmk: pmk
	@echo ""
	@echo "=> Testing pmk with sample files"
	@echo ""
	@echo "-> Dumping target files"
	@echo ""
	@echo "samples/Makefile.sample.pmk"
	@echo "----------------------------------------"
	@cat samples/Makefile.sample.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/Makefile.sample.pmk"
	@echo "----------------------------------------"
	@cat samples/Makefile.samplebis.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Running pmk"
	./pmk -f samples/pmkfile.sample
	@echo ""
	@echo "-> Dumping generated files"
	@echo ""
	@echo "samples/Makefile.sample"
	@echo "----------------------------------------"
	@cat samples/Makefile.sample
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/Makefile.samplebis"
	@echo "----------------------------------------"
	@cat samples/Makefile.samplebis
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Removing generated files"
	rm -f samples/Makefile.sample samples/Makefile.samplebis pmk.log
	@echo ""
	@echo "=> End of test"
	@echo ""

test_all: test_pmk clean
