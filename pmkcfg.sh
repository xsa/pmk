# $Id$

# Copyright (c) 2003 Damien Couderc
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

template="compat/compat.h.in"
compat="compat/compat.h"
maketmpl="./Makefile.pmk"
makefile="./Makefile"
temporary="./pmkcfg.tmp"


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
		templ_sed "def" "$header"
		echo "yes"
	else
		templ_sed "udef" "$header"
		echo "no"
	fi
	rm -f $testfile $testbin
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
		templ_sed "def" "$function"
		echo "yes"
	else
		templ_sed "udef" "$function"
		echo "no"
	fi
	rm -f $testfile $testobj
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
		templ_sed "def" "$type"
		echo "yes"
	else
		templ_sed "udef" "$type"
		echo "no"
	fi
	rm -f $testfile $testbin
}

check_type_header() {
	type="$1"
	header="$2"
	printf "Checking type '%s' in header '%s': " "$type" "$header"

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
		templ_sed "def" "$type"
		echo "yes"
	else
		templ_sed "udef" "$type"
		echo "no"
	fi
	rm -f $testfile $testbin
}

templ_sed() {
	sed_uc=`echo "$2" | tr [a-z] [A-Z] | tr . _`
	case $1 in
		def)	sed_str="#define HAVE_$sed_uc 1";;
		udef)	sed_str="#undef HAVE_$sed_uc";;
	esac
	sed_tag="@DEF__$sed_uc@"

	cp $compat $temporary
	cat $temporary | sed "s/$sed_tag/$sed_str/" > $compat
	rm -f $temporary
}

mkf_sed() {
	sed_var="$1"
	sed_val="$2"

	cp $makefile $temporary
	cat $temporary | sed "s,@$sed_var@,$sed_val," > $makefile
	rm -f $temporary
}

#
# init
#

# parse options
while getopts 'hp:u' arg; do
	case $arg in
		p)
			echo "overriding prefix with '$OPTARG'"
			base=$OPTARG
			;;
		u)
			usermode=1
			;;
		h)
			usage
			exit 1
			;;
	esac
done

cp $template $compat
cp $maketmpl $makefile

if [ $usermode = 1 ]; then
	echo 'USERMODE ON.'

	mkf_sed 'USERMODE' '-DPMK_USERMODE'
	if [ -z "$base" ]; then
		mkf_sed 'BASE' "$um_prfx"
	else
		mkf_sed 'BASE' "$base"
	fi
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
	mkf_sed 'CONFDIR' '$(SYSCONFDIR)/pmk'
	mkf_sed 'BINDIR' '$(BASE)/bin'
	mkf_sed 'SBINDIR' '$(BASE)/sbin'
	mkf_sed 'DATADIR' '$(BASE)/share/$(PREMAKE)'
	mkf_sed 'MANDIR' '$(BASE)/man'
fi

mkf_sed 'SYSCONFDIR' "$sysdir"

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
# config finished, creates config
#

echo 'OK' > config

#
# end
#
