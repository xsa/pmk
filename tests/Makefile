# $Id$

CC?=		cc
AS?=		as

CFLAGS?=	-Wall
#CFLAGS+=	-Werror
#CFLAGS+=	-ansi -pedantic

LDFLAGS?=
DEBUG?=		-g

#LINT_TARGET=	da_test.c

all: hash dynary path parser mkvar comp

.c.o:
	$(CC) $(CFLAGS) -c $<

config:
	@if [ ! -f ../compat/compat.h ]; then \
		echo "Not configured, run \
		'cd ../ && make clean config' first."; \
		exit 1; \
	fi

comp: config comp_test.o
	$(CC) -o comp_test $(LDFLAGS) comp_test.o

cpuid: ../detect_cpu_asm.o
	$(AS) -o detect_cpu_asm.o ../detect_cpu_asm.s
	$(CC) $(CFLAGS) -c cpuid_test.c
	$(CC) -o cpuid_test detect_cpu_asm.o cpuid_test.o

dynary: config da_test.o
	$(CC) -o da_test $(LDFLAGS) da_test.o

gencomp: config gen_comp_test.o
	$(CC) -o gen_comp_test $(LDFLAGS) gen_comp_test.o

hash: config hash_test.o
	$(CC) -o hash_test $(LDFLAGS) hash_test.o

mkvar: config mkvar_test.o
	$(CC) -o mkvar_test $(LDFLAGS) mkvar_test.o

parser: config parser_test.o
	$(CC) -o parser_test $(LDFLAGS) parser_test.o

path: config path_test.o
	$(CC) -o path_test $(LDFLAGS) path_test.o

pkgconfig: config pkgconfig_test.o
	$(CC) -o pkgconfig_test $(LDFLAGS) pkgconfig_test.o

clean:
	rm -f *.o *_test *.core

#lint:
#.ifdef LINT_TARGET
#	lint $(LINT_TARGET)
#.else
#	@echo "LINT_TARGET not set."
#.endif
