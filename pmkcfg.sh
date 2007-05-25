#!/bin/sh
# $Id$

# Copyright (c) 2003-2006 Damien Couderc
# Copyright (c) 2004 Martin Reindl
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    - Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    - Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
#    - Neither the name of the copyright holder(s) nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


#
# automatically detect and use posix shell
#

if test "$1" != "autodetected"; then
	printf "Autodetecting POSIX shell\n"

	posix_sh="/usr/bin/posix/sh"
	if test ! -x $posix_sh; then
		posix_sh="/usr/xpg4/bin/sh"
		if test ! -x "$posix_sh"; then
			posix_sh="/bin/sh"
		fi
	fi
	printf "Using %s %s %s\n\n" "$posix_sh" "$0" "$*"
	$posix_sh $0 "autodetected" "$@"
	exit $?
else
	# skip "autodetected"
	shift
fi


###########
# defines #
########################################################################

usermode=0
privsep_user="nobody"

if [ -z "$PREFIX" ]; then
	prefix="/usr/local"
else
	prefix=$PREFIX
fi

if [ -z "$SYSCONFDIR" ]; then
	sysdir="/etc"
else
	sysdir=$SYSCONFDIR
fi

sconf="$sysdir/pmk"

um_prfx='$(HOME)'

testbin="./cfgtest"
testfile="$testbin.c"
testmake="./maketest.mk"
testmkvar="./test_mkvar"
testobj="$testbin.o"

temporary="./pmkcfg.tmp"

tmpl_list="Makefile.pmk compat/config.h.pmk tests/Makefile.pmk"


#############
# functions	#
########################################################################

# display usage
usage() {
	echo "\
usage: $0 [-hup]

	-h		usage
	-p path		prefix override
	-u		set usermode
	-U		privilege separation user (default: nobody)
"
}

# remove template suffix
# result in filename variable
get_file_name() {
	basedir=`dirname $1`
	case $1 in
		*.in) sfx=".in";;
		*.pmk) sfx=".pmk";;
	esac
	basename=`basename $1 $sfx`
	filename="$basedir/$basename"
}

# create new files from templates
process_tmpl_list () {
	newlist=""
	for f in $*; do
		get_file_name $f
		newlist="$newlist $filename"
		cp $f $filename
	done
}

get_make_var() {
	mkvar="$1"
	printf "Getting '%s' value : " "$mkvar"

	cat > $testmake << EOF
all:
	@printf "%s\n" "\$($mkvar)" > $testmkvar
EOF

	make -f $testmake >/dev/null 2>&1

	rslt=`cat $testmkvar`

	rm $testmake $testmkvar

	printf "'%s'\n" "$rslt"
#	mkf_sed "$mkvar" "$rslt"
	make_var_value=$rslt
}

check_binary() {
	binary="$1"
	tname=`echo "$binary" | tr [a-z] [A-Z] | tr . _`
	tval=""

	printf "Checking binary '%s' : " "$binary"

	plst=`echo $PATH | sed "s/:/ /g"`

	r=1
	for d in $plst; do
		if test -x "$d/$binary"; then
			tval="$d/$binary"
			r=0
			break
		fi
	done

	if test "$r" -eq 0; then
		printf "yes\n"
		mkf_sed "$tname" "$tval"
	else
		printf "no\n"
	fi

	return $r
}

check_header() {
	header="$1"
	printf "Checking header '%s' : " "$header"

	cat > $testfile <<EOF
#include <stdio.h>
#include <$header>

int main() {
	return(0);
}
EOF

	if $CC -o $testbin $testfile >/dev/null 2>&1; then
		sed_define "def" "$header"
		printf "yes\n"
		r=0
	else
		sed_define "udef" "$header"
		printf "no\n"
		r=1
	fi
	rm -f $testfile $testbin

	return $r
}

check_header_function() {
	include="$1"
	function="$2"
	printf "Checking function '%s' in header '%s' : " "$function" "$include"

	cat > $testfile <<EOF
#include <stdio.h>
#include <$include>

void (*pmk_funcp)() = $function;

int main() {
	return(0);
}
EOF

	if $CC -o $testobj -c $testfile >/dev/null 2>&1; then
		sed_define "def" "$function"
		printf "yes\n"
		r=0
	else
		sed_define "udef" "$function"
		printf "no\n"
		r=1
	fi
	rm -f $testfile $testobj

	return $r
}

check_lib_function() {
	lib="$1"
	function="$2"
	printf "Checking function '%s' in library 'l%s' : " "$function" "$lib"

	cat > $testfile <<EOF
#include <stdio.h>

int $function();

int main() {
	printf("%p", $function);
	return(0);
}
EOF

	if $CC -o $testobj $testfile -l$lib >/dev/null 2>&1; then
		sed_define "def" "$function"
		printf "yes\n"
		r=0
	else
		sed_define "udef" "$function"
		printf "no\n"
		r=1
	fi
	rm -f $testfile $testobj

	return $r
}

check_type() {
	type="$1"
	printf "Checking type '%s' : " "$type"

	cat > $testfile <<EOF
#include <stdio.h>

int main() {
        if (($type *) 0)
                return(0);
        if (sizeof($type))
                return(0);
        return(0);
}
EOF

	if $CC -o $testbin $testfile >/dev/null 2>&1; then
		sed_define "def" "$type"
		printf "yes\n"
		r=0
	else
		sed_define "udef" "$type"
		printf "no\n"
		r=1
	fi
	rm -f $testfile $testbin

	return $r
}

check_type_header() {
	type="$1"
	header="$2"
	printf "Checking type '%s' in header '%s' : " "$type" "$header"

	cat > $testfile <<EOF
#include <stdio.h>
#include <$header>

int main() {
        if (($type *) 0)
                return(0);
        if (sizeof($type))
                return(0);
        return(0);
}
EOF

	if $CC -o $testbin $testfile >/dev/null 2>&1; then
		sed_define "def" "$type"
		printf "yes\n"
		r=0
	else
		sed_define "udef" "$type"
		printf "no\n"
		r=1
	fi
	rm -f $testfile $testbin

	return $r
}

sed_define() {
	sed_uc=`echo "$2" | tr [a-z] [A-Z] | tr . _ | tr " " _`
	case $1 in
		def)	sed_str="#define HAVE_$sed_uc 1";;
		udef)	sed_str="#undef HAVE_$sed_uc";;
	esac
	sed_tag="@DEF__$sed_uc@"

	for f in $file_list; do
		cp $f $temporary
		cat $temporary | sed "s/$sed_tag/$sed_str/" > $f
		rm -f $temporary
	done
}

mkf_sed() {
	sed_var="$1"
	sed_val="$2"

#printf "DEBUG: replace @%s@ by %s\n" "$sed_var" "$sed_val"

	for f in $file_list; do
		cp $f $temporary
		cat $temporary | sed "s,@$sed_var@,$sed_val," > $f
		rm -f $temporary
	done
}


########
# init #
########################################################################

# parse options
while getopts "hp:uU:" arg; do
	case $arg in
		h)	usage
			exit 1
			;;

		p)	printf "Setting prefix to '%s'\n" "$OPTARG"
			base="$OPTARG"
			;;

		u)	usermode=1
			;;

		U)	printf "Setting privsep user tp '%s'\n" "$OPTARG"
			privsep_user="$OPTARG"
			;;
	esac
done
shift `expr $OPTIND - 1`

# init templates
process_tmpl_list "$tmpl_list"
file_list=$newlist

if [ $usermode = 1 ]; then
	echo 'USERMODE ON.'

	mkf_sed 'USERMODE' '-DPMK_USERMODE'
	if [ -z "$base" ]; then
		mkf_sed 'PREFIX' "$um_prfx"
	else
		mkf_sed 'PREFIX' "$base"
	fi
	mkf_sed 'CONFDIR' '$(HOME)/.pmk'
	mkf_sed 'SBINDIR' '$(PREFIX)/bin'
	mkf_sed 'DATADIR' '$(CONFDIR)'
else
	echo "USERMODE OFF."

	mkf_sed 'USERMODE' ''
	if [ -z "$base" ]; then
		mkf_sed 'PREFIX' "$prefix"
	else
		mkf_sed 'PREFIX' "$base"
	fi
	mkf_sed 'CONFDIR' '$(SYSCONFDIR)/pmk'
	mkf_sed 'SBINDIR' '$(PREFIX)/sbin'
	mkf_sed 'DATADIR' '$(PREFIX)/share/$(PREMAKE)'
fi

mkf_sed 'BINDIR' '$(PREFIX)/bin'
mkf_sed 'MANDIR' '$(PREFIX)/man'
mkf_sed 'MAN1DIR' '$(MANDIR)/man1'
mkf_sed 'MAN5DIR' '$(MANDIR)/man5'
mkf_sed 'MAN8DIR' '$(MANDIR)/man8'
mkf_sed 'SYSCONFDIR' "$sysdir"
mkf_sed 'PRIVSEP_USER' "$privsep_user"
mkf_sed 'PACKAGE' "pmk"
mkf_sed 'VERSION' "0.9.3"
mkf_sed 'INSTALL' './pmkinstall'


#############
# get flags #
########################################################################

#
# gathering CFLAGS
#

if [ -z "$CFLAGS" ]; then
	get_make_var CFLAGS
	CFLAGS=$make_var_value
else
	printf "CFLAGS defined, skipping detection.\n"
fi

#
# gathering CLDFLAGS
#

if [ -z "$CLDFLAGS" ]; then
	get_make_var CLDFLAGS
	CLDFLAGS=$make_var_value
else
	printf "CLDFLAGS defined, skipping detection.\n"
fi


##################
# check binaries #
########################################################################

#
# cc check
#

if [ -z "$CC" ]; then
	# check first for C99 then C89 compilers
	if check_binary cc; then
		CC="cc"
	elif check_binary c99; then
		CC="c99"
	elif check_binary c89; then
		CC="c89"
	else
		printf "Unable to find C compiler.\n"
		exit 1
	fi
else
	printf "CC defined, skipping C compiler check.\n"
fi

mkf_sed 'CC' "$CC"

#
# cpp check
#
# 'cc -E' is the C99 standard for preprocessing C-language source files.
# See: http://www.opengroup.org/onlinepubs/009695399/utilities/c99.html
#
# However, the results are unspecified for assembler source files.
# Gcc has its own conventions for preprocessing assembler files, only the cpp
# binary preprocesses assembler files at any time. In gcc, 'cc -E' only
# processes assembler files which end in '.S'. To work around we first look
# for the cpp binary and fall back to 'cc -E' if not successful.

if [ -z "$CPP" ]; then
	if check_binary cpp; then
		CPP="cpp"
	else
		printf "Using '$CC -E'.\n"
		CPP="$CC -E"
	fi
else
	printf "CPP defined, skipping C preprocessor check.\n"
fi

mkf_sed 'CPP' "$CPP"

#
# as check
#

if [ -z "$AS" ]; then
	if ! check_binary as; then
		printf "Unable to find assembler.\n"
		exit 1
	fi
else
	printf "AS defined, skipping assembler.\n"
fi

mkf_sed 'AS' "$AS"


###############
# check types #
########################################################################

#
# _Bool type check
#

check_type _Bool

#
# blkcnt_t type check
#

check_type_header blkcnt_t sys/types.h

#
# long long type check
#

check_type "long long"

#
# unsigned long long type check
#

check_type "unsigned long long"

#
# long double type check
#

check_type "long double"

#
# intmax_t type check
#

check_type_header intmax_t stdint.h

#
# ptrdiff_t type check
#

check_type_header ptrdiff_t stddef.h

#
# wchar_t type check
#

check_type_header wchar_t wchar.h

#
# wint_t type check
#

check_type_header wint_t wchar.h


#################
# check headers #
########################################################################

#
# stdbool.h check
#

check_header stdbool.h

#
# libgen.h check
#

check_header libgen.h


###################
# check functions #
########################################################################

#
# strlcpy() check
#

check_header_function string.h strlcpy

#
# strlcat() check
#

check_header_function string.h strlcat

#
# strdup() check
#

check_header_function string.h strdup

#
# vsnprintf() check
#

check_header_function stdio.h vsnprintf

#
# snprintf() check
#

check_header_function stdio.h snprintf

#
# isblank() check
#

check_header_function ctype.h isblank

#
# mkstemps() check
#

check_header_function unistd.h mkstemps

#
# seteuid() check
#

if check_header_function unistd.h seteuid; then
	PS_FLAGS=""
else
	PS_FLAGS="-DWITHOUT_FORK"
fi

#
# dirname() check
#

if check_lib_function gen dirname; then
	LGEN_FLAGS="-lgen"
else
	LGEN_FLAGS=""
fi

#
# basename() check
#

check_lib_function gen basename

#
# isnan() check
#

if check_lib_function m isnan; then
	LM_FLAGS="-lm"
else
	LM_FLAGS=""
fi


#####################
# compilation flags #
########################################################################

mkf_sed 'configure_input' "Generated by pmkcfg.sh"
mkf_sed 'CFLAGS' "$CFLAGS $PS_FLAGS"
mkf_sed 'CLDFLAGS' "$CLDFLAGS"
mkf_sed 'LIBS' "$LGEN_FLAGS $LM_FLAGS"


#
# end
#

exit 0

