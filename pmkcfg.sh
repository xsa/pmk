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

testfile="cfgtest.c"
testbin="cfgtest"

template="compat/compat.h.in"
compat="compat/compat.h"
temporary="compat/compat.tmp"

def="#define"
udef="#undef"

#
# init
#

cp $template $compat


#
# strlcpy check
#

echo -n "Checking strlcpy : "

cat > $testfile <<EOF
#include <stdio.h>
#include <string.h>

#define str	"test string"

char	buf[256];

int main() {
	strlcpy(buf, str, sizeof(buf));
	return(0);
}
EOF

$CC -o $testbin $testfile

cp $compat $temporary

if [ -x "$testbin" ]; then
	sedstr="$def"
	msg="yes"
else
	sedstr="$udef"
	msg="no"
fi

cat $temporary | sed -e s/@DEF_STRLCPY@/$sedstr/ > $compat
echo "$msg"
rm -f $temporary $testfile $testbin


#
# stdbool.h check
#

echo -n "Checking stdbool.h : "

cat > $testfile <<EOF
#include <stdbool.h>
#include <stdio.h>

int main() {
	return(0);
}
EOF

$CC -o $testbin $testfile

cp $compat $temporary

if [ -x "$testbin" ]; then
	sedstr="$def"
	msg="yes"
else
	sedstr="$udef"
	msg="no"
fi

cat $temporary | sed -e s/@DEF_STDBOOL_H@/$sedstr/ > $compat
echo "$msg"
rm -f $temporary $testfile $testbin

#
# end
#
