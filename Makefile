# $Id$

CC?=		cc
INSTALL?=	install

CFLAGS?=
CFLAGS+=	-Wall
#CFLAGS+=	-Werror
#CFLAGS+=	-ansi -pedantic

# Debug stuff
#CFLAGS+=	-DPMK_DEBUG
CFLAGS+=	-DPMKSETUP_DEBUG

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
S_OBJS=		$(SETUP).o common.o hash.o

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
	$(INSTALL) -m 444 $(PREMAKE).1 $(PREFIX)/man/man1/
	$(INSTALL) -m 444 $(SETUP).8 $(PREFIX)/man/man8/

clean:
	rm -f $(P_OBJS) $(S_OBJS) $(PREMAKE) $(SETUP) *.core

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
	@echo "-> Dumping target file"
	@echo "----------------------------------------"
	@cat samples/Makefile.sample.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Running pmk"
	./pmk -f samples/pmkfile.sample
	@echo ""
	@echo "-> Dumping generated file"
	@echo "----------------------------------------"
	@cat samples/Makefile.sample
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Removing generated files"
	rm -f samples/Makefile.sample pmk.log
	@echo ""
	@echo "=> End of test"
	@echo ""

test_pmksetup: pmksetup
	@echo ""
	@echo "=>Test pmksetup with sample file"
	@echo "\t(PMKSETUP_DEBUG must be set)"
	@echo ""
	@echo "-> Dumping original configuration file"
	@echo "----------------------------------------"
	@cat samples/pmk.conf.sample
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Running pmksetup"
	./pmksetup
	@echo ""
	@echo "-> Dumping temporary file"
	@echo "----------------------------------------"
	@cat /tmp/pmk.notrandom
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Removing temporary file"
	rm -f /tmp/pmk.notrandom
	@echo ""
	@echo "=> End of test"
	@echo ""

test_all: test_pmk test_pmksetup clean
