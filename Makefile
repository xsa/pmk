# $Id$

CC?=		cc
INSTALL?=	install

CFLAGS?=
#CFLAGS+=	-Wall
#CFLAGS+=	-Werror
#CFLAGS+=	-ansi -pedantic

#optimize building stuff (gcc only)
#CFLAGS+=	-pipe

# Debug stuff
#CFLAGS+=	-DPMK_DEBUG
#CFLAGS+=	-DPMKSETUP_DEBUG

# Here you can change the default location of pmk.conf
#CFLAGS+=	-DSYSCONFDIR=\"/etc/\"

# Flag for platform testing checks
#CFLAGS+=	-DUSER_TEST

LDFLAGS?=

PREFIX?=	/usr/local
DEBUG?=		-g

PREMAKE=	pmk
SETUP=		pmksetup

BINDIR=		$(PREFIX)/bin/
SBINDIR=	$(PREFIX)/sbin/
DATADIR=	$(PREFIX)/share/$(PREMAKE)/
MANDIR=		$(PREFIX)/man/

SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

P_OBJS=		compat.o common.o hash.o func.o functool.o \
		dynarray.o autoconf.o pathtools.o pmk.o
S_OBJS=		$(SETUP).o common.o hash.o dynarray.o compat.o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(PREMAKE) $(SETUP)

config:
	@CC=$(CC) sh pmkcfg.sh
	@echo "OK" > config

$(PREMAKE): config $(P_OBJS)
	$(CC) -o $(PREMAKE) $(LDFLAGS) $(P_OBJS)

$(SETUP): config $(S_OBJS)
	$(CC) -o $(SETUP) $(LDFLAGS) $(S_OBJS)

install: all
	$(INSTALL) -d -m 755 $(BINDIR)
	$(INSTALL) -m 755 $(PREMAKE) $(BINDIR)$(PREMAKE)
	$(INSTALL) -d -m 755 $(SBINDIR)
	$(INSTALL) -m 755 $(SETUP) $(SBINDIR)$(SETUP)
	$(INSTALL) -d -m 755 $(DATADIR)
	$(INSTALL) -m 644 samples/$(SAMPLE) $(DATADIR)
	$(INSTALL) -m 644 samples/$(CONFIG) $(DATADIR)
	$(INSTALL) -d -m 755 $(MANDIR)/man1
	$(INSTALL) -m 444 man/$(PREMAKE).1 $(MANDIR)/man1/$(PREMAKE).1
	$(INSTALL) -d -m 755 $(MANDIR)/man5
	$(INSTALL) -m 444 man/$(PREMAKE).conf.5 $(MANDIR)/man5/$(PREMAKE).conf.5
	$(INSTALL) -m 444 man/$(PREMAKE)file.5 $(MANDIR)/man5/$(PREMAKE)file.5
	$(INSTALL) -d -m 755 $(MANDIR)/man8
	$(INSTALL) -m 444 man/$(SETUP).8 $(MANDIR)/man8/$(SETUP).8

clean:
	rm -f $(P_OBJS) $(S_OBJS) $(PREMAKE) $(SETUP) compat/compat.h config *.core

deinstall:
	rm -f $(BINDIR)$(PREMAKE)
	rm -f $(SBINDIR)$(SETUP)
	rm -rf $(DATADIR)
	rm -f $(PREFIX)/man/man1/$(PREMAKE).1
	rm -f $(PREFIX)/man/man8/$(SETUP).8
	rm -f $(PREFIX)/man/man5/$(PREMAKE).conf.5

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
	@echo "samples/subdir/Makefile.subdir.pmk"
	@echo "----------------------------------------"
	@cat samples/subdir/Makefile.subdir.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/config_sample.h.pmk"
	@echo "----------------------------------------"
	@cat samples/config_sample.h.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/ac_config.h"
	@echo "----------------------------------------"
	@cp samples/ac_config.h.sample samples/ac_config.h
	@cat samples/ac_config.h
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Running pmk"
	./pmk -b samples -f samples/pmkfile.sample -o samples/ovrfile.sample
	@echo ""
	@echo "-> Dumping generated files"
	@echo ""
	@echo "samples/Makefile.sample"
	@echo "----------------------------------------"
	@cat samples/Makefile.sample
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/subdir/Makefile.subdir"
	@echo "----------------------------------------"
	@cat samples/subdir/Makefile.subdir
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/config_sample.h"
	@echo "----------------------------------------"
	@cat samples/config_sample.h
	@echo "----------------------------------------"
	@echo ""
	@echo "samples/ac_config.h"
	@echo "----------------------------------------"
	@cat samples/ac_config.h
	@echo "----------------------------------------"
	@echo ""
	@echo "=> End of test"
	@echo ""

test_pmksetup: pmksetup
	@echo ""
	@echo "=> Testing pmksetup"
	@echo "Generating local pmk.conf."
	@echo "(need USER_TEST enabled)"
	@echo ""
	./pmksetup -V
	@echo ""
	@echo "=> End of test"
	@echo ""

test_clean:
	@echo ""
	@echo "=> Removing generated files"
	rm -f pmk.log pmk.conf
	rm -f samples/Makefile.sample samples/config_sample.h samples/ac_config.h
	rm -f samples/subdir/Makefile.subdir
	@echo ""
	@echo "=> End of cleaning."
	@echo ""

test_all: test_pmksetup test_pmk test_clean
