# $Id$

CC?=		cc
DEBUG?=		-g

# On some platforms install is not BSD compatible
# so we use our own install program.
INSTALL=	pmkinstall

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

PMKCFLAGS= $(WARN) $(DBGFLAGS) $(CFGFLAGS) $(USERMODE) $(CFLAGS) \
		-DDATADIR=\"$(DATADIR)\"

PREMAKE=	pmk
SETUP=		$(PREMAKE)setup
SCAN=		$(PREMAKE)scan
INST=           $(PREMAKE)install

SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

SELFINST=	self-$(INST)

TEST_SAMPLE=	test_samples
TEST_INST=	test_pmkinstall
TEST_TARGET=	$(TEST_INST)/$(INST).test

P_OBJS=		autoconf.o common.o compat.o detect.o dynarray.o func.o \
		functool.o hash.o parse.o pmk_obj.o pathtools.o pkgconfig.o $(PREMAKE).o
S_OBJS=		common.o compat.o dynarray.o hash.o parse.o pmk_obj.o $(SETUP).o
SC_OBJS=	common.o compat.o dynarray.o hash.o parse.o pmk_obj.o $(SCAN).o
I_OBJS=		common.o compat.o dynarray.o pathtools.o $(INST).o

.c.o:
	$(CC) $(PMKCFLAGS) -c $<

# main target
all: $(PREMAKE) $(SETUP) $(SCAN) $(INST)

cfgrm:
	@if ($(PREMAKE) -v >/dev/null 2>&1); then \
		echo 'Configure using pmk.'; \
		$(PREMAKE); \
	else \
		echo 'Configure using pmkcfg.sh'; \
		CC=$(CC) sh pmkcfg.sh; \
	fi
	@echo 'OK' > cfgrm
	@rm -f cfgum
	@echo 'Done.'

cfgum:
	@if ($(PREMAKE) -v >/dev/null 2>&1); then \
		echo 'Configure using pmk.'; \
		$(PREMAKE) -e sw_usermode; \
	else \
		echo 'Configure using pmkcfg.sh'; \
		CC=$(CC) sh pmkcfg.sh -u; \
	fi
	@echo 'OK' > cfgum
	@rm -f cfgrm
	@echo 'Done.'

config: cfgrm
	@echo 'OK' > config

$(PREMAKE): config $(P_OBJS)
	$(CC) -o $(PREMAKE) $(LDFLAGS) $(P_OBJS)

$(SETUP): config $(S_OBJS)
	$(CC) -o $(SETUP) $(LDFLAGS) $(S_OBJS)

$(SCAN): config $(SC_OBJS)
	$(CC) -o $(SCAN) $(LDFLAGS) $(SC_OBJS)

$(INST): config $(I_OBJS)
	$(CC) -o $(INST) $(LDFLAGS) $(I_OBJS)

install: all
	$(SUDO) $(INSTALL) -d -m 755 $(DESTDIR)$(BINDIR)
	$(SUDO) $(INSTALL) -m 755 $(PREMAKE) $(DESTDIR)$(BINDIR)/$(PREMAKE)
	$(SUDO) $(INSTALL) -m 755 $(SCAN) $(DESTDIR)$(BINDIR)/$(SCAN)
	$(SUDO) $(INSTALL) -m 755 $(INST) $(DESTDIR)$(BINDIR)/$(INST)
	$(SUDO) $(INSTALL) -d -m 755 $(DESTDIR)$(SBINDIR)
	$(SUDO) $(INSTALL) -m 755 $(SETUP) $(DESTDIR)$(SBINDIR)/$(SETUP)
	$(SUDO) $(INSTALL) -d -m 755 $(DESTDIR)$(DATADIR)
	$(SUDO) $(INSTALL) -m 644 samples/$(SAMPLE) $(DESTDIR)$(DATADIR)/$(SAMPLE)
	$(SUDO) $(INSTALL) -m 644 samples/$(CONFIG) $(DESTDIR)$(DATADIR)/$(CONFIG)
	$(SUDO) $(INSTALL) -m 644 data/pmkscan.dat $(DESTDIR)$(DATADIR)/pmkscan.dat
	$(SUDO) $(INSTALL) -m 644 data/pmkcomp.dat $(DESTDIR)$(DATADIR)/pmkcomp.dat
	$(SUDO) $(INSTALL) -m 644 data/pmkcfgtool.dat $(DESTDIR)$(DATADIR)/pmkcfgtool.dat
	$(SUDO) $(INSTALL) -d -m 755 $(DESTDIR)$(MANDIR)/man1
	$(SUDO) $(INSTALL) -m 444 man/$(PREMAKE).1 $(DESTDIR)$(MANDIR)/man1/$(PREMAKE).1
	$(SUDO) $(INSTALL) -m 444 man/$(SCAN).1 $(DESTDIR)$(MANDIR)/man1/$(SCAN).1
	$(SUDO) $(INSTALL) -m 444 man/$(INST).1 $(DESTDIR)$(MANDIR)/man1/$(INST).1
	$(SUDO) $(INSTALL) -d -m 755 $(DESTDIR)$(MANDIR)/man5
	$(SUDO) $(INSTALL) -m 444 man/$(PREMAKE).conf.5 $(DESTDIR)$(MANDIR)/man5/$(PREMAKE).conf.5
	$(SUDO) $(INSTALL) -m 444 man/$(PREMAKE)file.5 $(DESTDIR)$(MANDIR)/man5/$(PREMAKE)file.5
	$(SUDO) $(INSTALL) -d -m 755 $(DESTDIR)$(MANDIR)/man8
	$(SUDO) $(INSTALL) -m 444 man/$(SETUP).8 $(DESTDIR)$(MANDIR)/man8/$(SETUP).8

deinstall:
	$(SUDO) rm -f $(DESTDIR)$(BINDIR)/$(PREMAKE)
	$(SUDO) rm -f $(DESTDIR)$(BINDIR)/$(SCAN)
	$(SUDO) rm -f $(DESTDIR)$(BINDIR)/$(INST)
	$(SUDO) rm -f $(DESTDIR)$(SBINDIR)/$(SETUP)
	$(SUDO) rm -f $(DESTDIR)$(DATADIR)/$(SAMPLE)
	$(SUDO) rm -f $(DESTDIR)$(DATADIR)/$(CONFIG)
	$(SUDO) rm -f $(DESTDIR)$(DATADIR)/pmkscan.dat
	$(SUDO) rm -f $(DESTDIR)$(DATADIR)/pmkcomp.dat
	$(SUDO) rm -f $(DESTDIR)$(DATADIR)/pmkcfgtool.dat
	@if [ ! -f cfgum ]; then \
		$(SUDO) rm -rf $(DESTDIR)$(DATADIR); \
	fi
	$(SUDO) rm -f $(DESTDIR)$(MANDIR)/man1/$(PREMAKE).1
	$(SUDO) rm -f $(DESTDIR)$(MANDIR)/man1/$(SCAN).1
	$(SUDO) rm -f $(DESTDIR)$(MANDIR)/man1/$(INST).1
	$(SUDO) rm -f $(DESTDIR)$(MANDIR)/man8/$(SETUP).8
	$(SUDO) rm -f $(DESTDIR)$(MANDIR)/man5/$(PREMAKE)file.5
	$(SUDO) rm -f $(DESTDIR)$(MANDIR)/man5/$(PREMAKE).conf.5

$(PREMAKE)-clean:
	rm -f $(P_OBJS) $(PREMAKE)

$(SETUP)-clean:
	rm -f $(S_OBJS) $(SETUP)

$(SCAN)-clean:
	rm -f $(SC_OBJS) $(SCAN)

$(INST)-clean:
	rm -f $(I_OBJS) $(INST)

clean: $(PREMAKE)-clean $(SETUP)-clean $(SCAN)-clean $(INST)-clean
	rm -f compat/compat.h config cfgum cfgrm *.core

test_$(PREMAKE): $(PREMAKE)
	@echo ""
	@echo "=> Testing $(PREMAKE) with sample files"
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
	$(BINDIR)/$(PREMAKE) -l -b $(TEST_SAMPLE) -e use_gtk -f samples/pmkfile.sample -o samples/ovrfile.sample
	@echo ""
	@echo "-> Dumping generated files"
	@echo ""
	@echo "$(TEST_SAMPLE)/Makefile.sample"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/Makefile.sample
	@echo "----------------------------------------"
	@echo ""
	@echo "$(TEST_SAMPLE)/subdir/Makefile.subdir"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/subdir/Makefile.subdir
	@echo "----------------------------------------"
	@echo ""
	@echo "$(TEST_SAMPLE)/config_sample.h"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/config_sample.h
	@echo "----------------------------------------"
	@echo ""
	@echo "$(TEST_SAMPLE)/ac_config.h"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/ac_config.h
	@echo "----------------------------------------"
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(SETUP): $(SETUP)
	@echo ""
	@echo "=> Testing $(SETUP)"
	@echo "Generating local pmk.conf."
	@echo "(need USERMODE enabled)"
	@echo ""
	$(SBINDIR)/$(SETUP) -V
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
	@echo "=> Testing $(SCAN)"
	$(BINDIR)/$(SCAN)
	@echo ""
	@echo "Dumping pmkfile.scan"
	@echo "----------------------------------------"
	@cat pmkfile.scan
	@echo "----------------------------------------"
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(INST): $(INST)
	@echo ""
	@echo "=> Testing $(INST)"
	@echo ""
	@echo "-> Creating test directory"
	@echo ""
	$(BINDIR)/$(INST) -d -m 770 $(TEST_INST)
	@echo ""
	@echo "-> Checking test directory"
	@if (test -d "$(TEST_INST)"); then \
		echo ""; \
		echo "- - - - - - - - - - - - - - - - - - -" \
			"- - - - - - - - - - - - - - - - - -"; \
		echo ""; \
		ls -ld $(TEST_INST); \
		echo ""; \
		echo "- - - - - - - - - - - - - - - - - - -" \
			"- - - - - - - - - - - - - - - - - -"; \
		echo ""; \
		echo "Directory OK."; \
	else \
		echo ""; \
		echo "Failed."; \
		exit 1; \
	fi
	@echo ""
	@echo "-> Installing test data"
	@echo ""
	$(BINDIR)/$(INST) -m u+rw README $(TEST_TARGET)1
	$(BINDIR)/$(INST) -m ug+r README $(TEST_TARGET)2
	@echo ""
	@echo "-> Checking test file"
	@echo "";
	@echo "- - - - - - - - - - - - - - - - - - -" \
		"- - - - - - - - - - - - - - - - - -";
	@echo "";
	@for i in 1 2; do \
		if (test -f "$(TEST_TARGET)$$i"); then \
			ls -l $(TEST_TARGET)$$i; \
		else \
			echo "$(TEST_TARGET)$$i failed."; \
			exit 1; \
		fi \
	done
	@echo "";
	@echo "- - - - - - - - - - - - - - - - - - -" \
		"- - - - - - - - - - - - - - - - - -";
	@echo "";
	@echo ""
	@echo "=> End of test"
	@echo ""


test_clean:
	@echo ""
	@echo "=> Removing generated files"
	rm -rf $(TEST_SAMPLE)
	rm -rf $(TEST_TARGET)*
	rm -rf $(TEST_INST)
	rm -f samples/ac_config.h pmkfile.scan pmk*.log

	@echo ""
	@echo "=> End of cleaning."
	@echo ""

test_all: test_$(SETUP) test_$(PREMAKE) test_$(SCAN) test_$(INST) test_clean

test_pmk_only: test_$(SETUP) test_$(PREMAKE) test_clean

test: test_all
