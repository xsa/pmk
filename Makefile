# $Id$

CC?=		cc
DEBUG?=		-g
INSTALL=	install

SYSCONFDIR=	/etc

# base path for install
BASE=		/usr/local

# configuration file location
CONFDIR=	$(SYSCONFDIR)/pmk

BINDIR=		$(BASE)/bin
SBINDIR=	$(BASE)/sbin
DATADIR=	$(BASE)/share/$(PREMAKE)
MANDIR=		$(BASE)/man

CFGFLAGS=	-DSYSCONFDIR=\"$(SYSCONFDIR)\" -DCONFDIR=\"$(CONFDIR)\"

# Flag to enable pmk in user mode (check INSTALL file for details).
USERMODE=	

# Edit and use the following if needed :

# Warning options, uncomment as your need
#WARN=		-Wall
#WARN=		-Werror
#WARN=		-ansi -pedantic
#WARN=		-Wall -Werror -ansi -pedantic

# Debug stuff
#DBGFLAGS=	-DPMK_DEBUG
#DBGFLAGS=	-DPMKSETUP_DEBUG
#DBGFLAGS=	-DPMK_DEBUG -DPMKSETUP_DEBUG 

PMKCFLAGS= $(WARN) $(DBGFLAGS) $(CFGFLAGS) $(USERMODE) $(CFLAGS)

PREMAKE=	pmk
SETUP=		$(PREMAKE)setup
SCAN=		$(PREMAKE)scan

SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

P_OBJS=		autoconf.o common.o compat.o dynarray.o func.o functool.o \
		hash.o parse.o pmk_obj.o pathtools.o $(PREMAKE).o
S_OBJS=		dynarray.o common.o compat.o hash.o parse.o pmk_obj.o $(SETUP).o
SC_OBJS=	dynarray.o common.o compat.o hash.o parse.o pmk_obj.o $(SCAN).o

.c.o:
	$(CC) $(PMKCFLAGS) -c $<

# main target
all: $(PREMAKE) $(SETUP) $(SCAN)

# specific object files
$(SCAN).o:
	$(CC) $(PMKCFLAGS) -DDATADIR=\"$(DATADIR)\" -c $(SCAN).c

# for compatibility with 0.6 pmkfiles, will be removed later
parse.o:
	$(CC) $(PMKCFLAGS) -DPRS_OBSOLETE -c parse.c

cfgrm:
	@if ($(PREMAKE) -v >/dev/null 2>&1); then \
		echo 'Configure using pmk.'; \
		$(PREMAKE); \
	else \
		echo 'Configure using pmkcfg.sh'; \
		CC=$(CC) sh pmkcfg.sh; \
	fi
	@echo 'OK' > config
	@echo 'Done.'

cfgum:
	@if ($(PREMAKE) -v >/dev/null 2>&1); then \
		echo 'Configure using pmk.'; \
		$(PREMAKE) -e sw_usermode; \
	else \
		echo 'Configure using pmkcfg.sh'; \
		CC=$(CC) sh pmkcfg.sh usermode; \
	fi
	@echo 'OK' > config
	@echo 'Done.'

config:
	@echo 'Not configured, read INSTALL file.'
	@exit 1

$(PREMAKE): config $(P_OBJS)
	$(CC) -o $(PREMAKE) $(LDFLAGS) $(P_OBJS)

$(SETUP): config $(S_OBJS)
	$(CC) -o $(SETUP) $(LDFLAGS) $(S_OBJS)

${SCAN}: config $(SC_OBJS)
	$(CC) -o $(SCAN) $(LDFLAGS) $(SC_OBJS)

install: all
	$(INSTALL) -d -m 755 $(BINDIR)
	$(INSTALL) -m 755 $(PREMAKE) $(BINDIR)/$(PREMAKE)
	$(INSTALL) -m 755 $(SCAN) $(BINDIR)/$(SCAN)
	$(INSTALL) -d -m 755 $(SBINDIR)
	$(INSTALL) -m 755 $(SETUP) $(SBINDIR)/$(SETUP)
	$(INSTALL) -d -m 755 $(DATADIR)
	$(INSTALL) -m 644 samples/$(SAMPLE) $(DATADIR)
	$(INSTALL) -m 644 samples/$(CONFIG) $(DATADIR)
	$(INSTALL) -m 644 data/pmkscan.dat $(DATADIR)
	$(INSTALL) -d -m 755 $(MANDIR)/man1
	$(INSTALL) -m 444 man/$(PREMAKE).1 $(MANDIR)/man1/$(PREMAKE).1
	$(INSTALL) -m 444 man/$(SCAN).1 $(MANDIR)/man1/$(SCAN).1
	$(INSTALL) -d -m 755 $(MANDIR)/man5
	$(INSTALL) -m 444 man/$(PREMAKE).conf.5 $(MANDIR)/man5/$(PREMAKE).conf.5
	$(INSTALL) -m 444 man/$(PREMAKE)file.5 $(MANDIR)/man5/$(PREMAKE)file.5
	$(INSTALL) -d -m 755 $(MANDIR)/man8
	$(INSTALL) -m 444 man/$(SETUP).8 $(MANDIR)/man8/$(SETUP).8

deinstall:
	rm -f $(BINDIR)/$(PREMAKE)
	rm -f $(BINDIR)/$(SCAN)
	rm -f $(SBINDIR)/$(SETUP)
	rm -rf $(DATADIR)
	rm -f $(BASE)/man/man1/$(PREMAKE).1
	rm -f $(BASE)/man/man1/$(SCAN).1
	rm -f $(BASE)/man/man8/$(SETUP).8
	rm -f $(BASE)/man/man5/$(PREMAKE)file.5
	rm -f $(BASE)/man/man5/$(PREMAKE).conf.5

$(PREMAKE)-clean:
	rm -f $(P_OBJS) $(PREMAKE)

$(SETUP)-clean:
	rm -f $(S_OBJS) $(SETUP)

$(SCAN)-clean:
	rm -f $(SC_OBJS) $(SCAN)

clean: $(PREMAKE)-clean $(SETUP)-clean $(SCAN)-clean
	rm -f compat/compat.h config *.core

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
	./$(PREMAKE) -b samples -e use_gtk -f samples/pmkfile.sample -o samples/ovrfile.sample
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
	@echo "(need USERMODE enabled)"
	@echo ""
	./$(SETUP) -V
	@echo ""
	@echo "Dumping resulting pmk.conf"
	@echo "----------------------------------------"
	@cat $(CONFDIR)/pmk.conf
	@echo "----------------------------------------"
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
	rm -f samples/Makefile.sample samples/config_sample.h samples/ac_config.h
	rm -f samples/subdir/Makefile.subdir pmkfile.scan pmk.log
	@echo ""
	@echo "=> End of cleaning."
	@echo ""

test_all: all test_$(SETUP) test_$(PREMAKE) test_$(SCAN) test_clean
