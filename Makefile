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
SETUP=		$(PREMAKE)setup
SCAN=		$(PREMAKE)scan

BINDIR=		$(PREFIX)/bin/
SBINDIR=	$(PREFIX)/sbin/
DATADIR=	$(PREFIX)/share/$(PREMAKE)/
MANDIR=		$(PREFIX)/man/

SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

P_OBJS=		${PREMAKE}.o compat.o common.o hash.o func.o functool.o \
		dynarray.o autoconf.o pathtools.o parse.o pmk_obj.o
S_OBJS=		$(SETUP).o common.o hash.o dynarray.o compat.o pmk_obj.o
SC_OBJS=	$(SCAN).o common.o compat.o dynarray.o parse.o hash.o pmk_obj.o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(PREMAKE) $(SETUP) $(SCAN)

# specific object files
$(SCAN).o:
	$(CC) $(CFLAGS) -DDATADIR=\"$(DATADIR)\" -c $(SCAN).c

config:
	@if ($(PREMAKE) -v >/dev/null 2>&1); then \
		echo "Configure using pmk."; \
		$(PREMAKE); \
	else \
		echo "Configure using pmkcfg.sh"; \
		CC=$(CC) sh pmkcfg.sh; \
	fi
	@echo "OK" > config

$(PREMAKE): config $(P_OBJS)
	$(CC) -o $(PREMAKE) $(LDFLAGS) $(P_OBJS)

$(SETUP): config $(S_OBJS)
	$(CC) -o $(SETUP) $(LDFLAGS) $(S_OBJS)

${SCAN}: config $(SC_OBJS)
	$(CC) -o $(SCAN) $(LDFLAGS) $(SC_OBJS)

install: all
	$(INSTALL) -d -m 755 $(BINDIR)
	$(INSTALL) -m 755 $(PREMAKE) $(BINDIR)$(PREMAKE)
	$(INSTALL) -m 755 $(SCAN) $(BINDIR)$(SCAN)
	$(INSTALL) -d -m 755 $(SBINDIR)
	$(INSTALL) -m 755 $(SETUP) $(SBINDIR)$(SETUP)
	$(INSTALL) -d -m 755 $(DATADIR)
	$(INSTALL) -m 644 samples/$(SAMPLE) $(DATADIR)
	$(INSTALL) -m 644 samples/$(CONFIG) $(DATADIR)
	$(INSTALL) -m 644 data/pmkscan.dat $(DATADIR)
	$(INSTALL) -d -m 755 $(MANDIR)/man1
	$(INSTALL) -m 444 man/$(PREMAKE).1 $(MANDIR)/man1/$(PREMAKE).1
	$(INSTALL) -d -m 755 $(MANDIR)/man5
	$(INSTALL) -m 444 man/$(PREMAKE).conf.5 $(MANDIR)/man5/$(PREMAKE).conf.5
	$(INSTALL) -m 444 man/$(PREMAKE)file.5 $(MANDIR)/man5/$(PREMAKE)file.5
	$(INSTALL) -d -m 755 $(MANDIR)/man8
	$(INSTALL) -m 444 man/$(SETUP).8 $(MANDIR)/man8/$(SETUP).8

$(PREMAKE)-clean:
	rm -f $(P_OBJS) $(PREMAKE)

$(SETUP)-clean:
	rm -f $(S_OBJS) $(SETUP)

$(SCAN)-clean:
	rm -f $(SC_OBJS) $(SCAN)

clean: $(PREMAKE)-clean $(SETUP)-clean $(SCAN)-clean
	rm -f compat/compat.h config *.core

deinstall:
	rm -f $(BINDIR)$(PREMAKE)
	rm -f $(BINDIR)$(SCAN)
	rm -f $(SBINDIR)$(SETUP)
	rm -rf $(DATADIR)
	rm -f $(PREFIX)/man/man1/$(PREMAKE).1
	rm -f $(PREFIX)/man/man8/$(SETUP).8
	rm -f $(PREFIX)/man/man5/$(PREMAKE).conf.5

test_$(PREMAKE): $(PREMAKE)
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
	./$(PREMAKE) -b samples -f samples/pmkfile.sample -o samples/ovrfile.sample
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

test_$(SETUP): $(SETUP)
	@echo ""
	@echo "=> Testing pmksetup"
	@echo "Generating local pmk.conf."
	@echo "(need USER_TEST enabled)"
	@echo ""
	./$(SETUP) -V
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(SCAN): $(SCAN)
	@echo ""
	@echo "=> Testing pmkscan"
	./$(SCAN)
	@echo ""
	@echo "Dumping pmkfile.scan"
	@echo "----------------------------------------"
	@cat pmkfile.scan
	@echo "----------------------------------------"
	@echo ""

test_clean:
	@echo ""
	@echo "=> Removing generated files"
	rm -f pmk.log pmk.conf
	rm -f samples/Makefile.sample samples/config_sample.h samples/ac_config.h
	rm -f samples/subdir/Makefile.subdir pmkfile.scan
	@echo ""
	@echo "=> End of cleaning."
	@echo ""

test_all: test_$(SETUP) test_$(PREMAKE) test_$(SCAN) test_clean
