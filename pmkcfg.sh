#!/bin/sh
# $Id$

# Copyright (c) 2003-2004 Damien Couderc
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
	$posix_sh $0 "autodetected" "$*"
	exit $?
else
	# skip "autodetected"
	shift
fi


#
# defines
#

usermode=0

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
testobj="$testbin.o"

temporary="./pmkcfg.tmp"

tmpl_list="Makefile.pmk compat/compat.h.in tests/Makefile.pmk"

#
# functions
#

# display usage
usage() {
	echo "\
usage: $0 [-hup]

	-h		usage
	-u		set usermode
	-p path		prefix override
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
		echo "yes"
	else
		echo "no"
	fi
	mkf_sed "$tname" "$tval"

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
		echo "yes"
		r=0
	else
		sed_define "udef" "$header"
		echo "no"
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
		echo "yes"
		r=0
	else
		sed_define "udef" "$function"
		echo "no"
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
		echo "yes"
		r=0
	else
		sed_define "udef" "$function"
		echo "no"
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
		echo "yes"
		r=0
	else
		sed_define "udef" "$type"
		echo "no"
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
		echo "yes"
		r=0
	else
		sed_define "udef" "$type"
		echo "no"
		r=1
	fi
	rm -f $testfile $testbin

	return $r
}

sed_define() {
	sed_uc=`echo "$2" | tr [a-z] [A-Z] | tr . _`
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

	for f in $file_list; do
		cp $f $temporary
		cat $temporary | sed "s,@$sed_var@,$sed_val," > $f
		rm -f $temporary
	done
}


#
# init
#

# parse options
while getopts "hp:u" arg; do
	case $arg in
		p)	echo "Setting prefix to '$OPTARG'"
			base="$OPTARG"
			;;

		u)	usermode=1
			;;

		h)	usage
			exit 1
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
		mkf_sed 'BASE' "$um_prfx"
	else
		mkf_sed 'BASE' "$base"
	fi
	mkf_sed 'MKTARGET' 'user'
	mkf_sed 'CONFDIR' '$(HOME)/.pmk'
	mkf_sed 'BINDIR' '$(BASE)/bin'
	mkf_sed 'SBINDIR' '$(BASE)/bin'
	mkf_sed 'DATADIR' '$(CONFDIR)'
	mkf_sed 'MANDIR' '$(BASE)/man'
else
	echo "USERMODE OFF."

	mkf_sed 'USERMODE' ''
	if [ -z "$base" ]; then
		mkf_sed 'BASE' "$prefix"
	else
		mkf_sed 'BASE' "$base"
	fi
	mkf_sed 'MKTARGET' 'global'
	mkf_sed 'CONFDIR' '$(SYSCONFDIR)/pmk'
	mkf_sed 'BINDIR' '$(BASE)/bin'
	mkf_sed 'SBINDIR' '$(BASE)/sbin'
	mkf_sed 'DATADIR' '$(BASE)/share/$(PREMAKE)'
	mkf_sed 'MANDIR' '$(BASE)/man'
fi

mkf_sed 'SYSCONFDIR' "$sysdir"

#
# cc check
#


if [ -z "$CC" ]; then
	if ! check_binary cc; then
		printf "Unable to find C compiler"
		exit 0
	fi
else
	printf "CC defined, skipping C compiler check.\n"
	mkf_sed 'CC' "$CC"
fi

#
# cpp check
#

if ! check_binary cpp; then
	printf "Unable to find C preprocessor"
	exit 0
fi

#
# as check
#

if ! check_binary as; then
	printf "Unable to find assembler"
	exit 0
fi

#
# strlcpy check
#

check_header_function string.h strlcpy

#
# strlcat check
#

check_header_function string.h strlcat

#
# _Bool type check
#

check_type _Bool

#
# blkcnt_t type check
#

check_type_header blkcnt_t sys/types.h

#
# stdbool.h check
#

check_header stdbool.h

#
# libgen.h check
#

check_header libgen.h

#
# isblank check
#

check_header_function ctype.h isblank

#
# mkstemps check
#

check_header_function unistd.h mkstemps

#
# dirname check
#

if check_lib_function gen dirname; then
	mkf_sed 'LGEN_FLAGS' "-lgen"
else
	mkf_sed 'LGEN_FLAGS' ""
fi

#
# end
#
