/* $Id$ */

/*
 * Copyright (c) 2003 Damien Couderc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    - Neither the name of the copyright holder(s) nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef _DETECT_H_
#define _DETECT_H_

#include "compat/pmk_string.h"
#include "pmk.h"
#include "premake.h"

#define CC_TFILE_EXT	".c"
#define CC_TEST_FILE	TMPDIR "/cc_testXXXXXXXX" CC_TFILE_EXT
#define CC_TEST_BIN	TMPDIR "/cc_test_bin"
#define CC_TEST_FORMAT	"%s -o %s %s >%s 2>&1"

/* compiler ids */
#define CI_UNKNOWN	0
#define CI_TENDRA	1
#define CI_GNUC		2
#define CI_SUNPRO_C	3
#define CI_SUNPRO_CXX	4
#define CI_COMPAQ_C	5
#define CI_COMPAQ_CXX	6
#define CI_HP_ANSI_C	7
#define CI_HP_ANSI_CXX	8
#define CI_IBM_XLC	9
#define CI_INTEL	10
#define CI_SGI_MPRO	11

/* compiler descr strings */
#define CD_UNKNOWN	"Unknown"
#define CD_TENDRA	"TenDRA"
#define CD_GNUC		"GNU gcc"
#define CD_SUNPRO_C	"Sun Workshop C"
#define CD_SUNPRO_CXX	"Sun Workshop C++"
#define CD_COMPAQ_C	"Compaq C"
#define CD_COMPAQ_CXX	"Compaq C++"
#define CD_HP_ANSI_C	"HP Ansi C"
#define CD_HP_ANSI_CXX	"HP Ansi C++"
#define CD_IBM_XLC	"IBM xlC"
#define CD_INTEL	"Intel"
#define CD_SGI_MPRO	"SGI MIPSpro"

/* shared libs compiler flags */

#define SCF_GNU		"-fPIC"
#define SCF_SYSV	"-Kpic"
#define SCF_HP		"+Z"


/* header of test code */
#define COMP_TEST_HEADER \
	"#include <stdio.h>\n\n"

/* descr macro c_id ver_macro */
#define COMP_TEST_FORMAT \
	"/* %s */\n" \
	"#ifdef %s\n" \
	"#define CC_ID\t%d\n" \
	"#define CC_V\t%s\n" \
	"#endif\n" \
	"\n"

/*  */
#define COMP_TEST_FOOTER \
	"/* unknown compiler found */\n" \
	"#ifndef CC_ID\n" \
	"#define CC_ID\t\"unknown\"\n" \
	"#endif\n" \
	"\n" \
	"/* no version */\n" \
	"#ifndef CC_V\n" \
	"#define CC_V\t0\n" \
	"#endif\n" \
	"\n" \
	"int main() {\n" \
	"/* compiler id */\n" \
	"\tprintf(\"%%d\\n\", CC_ID);\n" \
	"/* compiler version */\n" \
	"\tprintf(\"%%d\\n\", CC_V);\n" \
	"\treturn(0);\n" \
	"}\n"


typedef struct {
	unsigned int	 c_id;
	char		*descr,
			*c_macro,
			*v_macro;
} comp_cell;

typedef struct {
	char	descr[255],
		version[255];
	int	index;
} comp_data;


void	gen_test_file(FILE *);
int	cid_to_idx(unsigned int);
bool	detect_compiler(char *, char *, comp_data *);

#endif /* _DETECT_H_ */
