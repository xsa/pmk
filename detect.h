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
#include "parse.h"
#include "pmk.h"
#include "premake.h"

#ifndef DATADIR
/* for lint */
#define DATADIR	"./data"
#endif

#define PMKCOMP_DATA	DATADIR "/pmkcomp.dat"

#define PCC_TOK_ADDC	1

/* max number of compilers hash table */
#define MAX_COMP	32

#define CC_TFILE_EXT	".c"
#define CC_TEST_FILE	TMPDIR "/cc_testXXXXXXXX" CC_TFILE_EXT
#define CC_TEST_BIN	TMPDIR "/cc_test_bin"
#define CC_TEST_FORMAT	"%s -o %s %s >%s 2>&1"

#define DEF_VERSION	"#define CC_V\t%s"
#define DEF_NOVERSION	"#undef CC_V"

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
	"#define CC_ID\t\"%s\"\n" \
	"%s\n" \
	"#endif\n" \
	"\n"

/* footer of test code */
#define COMP_TEST_FOOTER \
	"/* unknown compiler found */\n" \
	"#ifndef CC_ID\n" \
	"#define CC_ID\t\"unknown\"\n" \
	"#endif\n" \
	"\n" \
	"int main() {\n" \
	"/* compiler id */\n" \
	"\tprintf(\"%%s\\n\", CC_ID);\n" \
	"/* compiler version */\n" \
	"#ifdef CC_V\n" \
	"\tprintf(\"%%d\\n\", CC_V);\n" \
	"#else\n" \
	"\tprintf(\"unknown\\n\");\n" \
	"#endif\n" \
	"\treturn(0);\n" \
	"}\n"


typedef struct {
	char	*c_id,
		*descr,
		*c_macro,
		*v_macro,
		*slflags;
} comp_cell;

typedef struct {
	char	*c_id,
		*version;
} comp_info;

typedef struct {
	htable	*cht;
} comp_data;


void		 compdata_destroy(comp_data *);
bool		 add_compiler(comp_data *, htable *);
comp_cell	*comp_get(comp_data *, char *c_id);
char		*comp_get_descr(comp_data *, char *);
comp_data	*parse_comp_file(char *);
bool		 gen_test_file(FILE *, comp_data *);
bool		 detect_compiler(char *, char *, comp_data *, comp_info *);

#endif /* _DETECT_H_ */
