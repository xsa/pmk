/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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
#include "premake.h"

#ifndef DATADIR
/* for lint */
#define DATADIR	"/DATADIR_not_defined"
#endif


/*************
 * constants *
 ***********************************************************************/

#define PMKCOMP_DATA	DATADIR "/pmkcomp.dat"

#define PCC_TOK_ADDC	1
#define PCC_TOK_ADDS	2

/* max number of compilers hash table */
#define MAX_COMP	32

/* max number of OSes in hash table */
#define MAX_OS		128

#define CC_TFILE_EXT	".c"
#define CC_TEST_FILE	TMPDIR "/cc_testXXXXXXXX" CC_TFILE_EXT
#define CC_TEST_BIN	TMPDIR "/cc_test_bin"
#define CC_TEST_FORMAT	"%s -o %s %s >%s 2>&1"

#define DEF_VERSION	"#define CC_V\t%s"
#define DEF_NOVERSION	"#undef CC_V"

#define SL_LDFLAG_VARNAME	"SLLDFLAGS"


/****************
 * keyword data *
 ***********************************************************************/

/* ADD_COMPILER keyword options */
#define CC_KW_ID	"ID"
#define CC_KW_DESCR	"DESCR"
#define CC_KW_MACRO	"MACRO"
#define CC_KW_VERSION	"VERSION"
#define CC_KW_SLCFLAGS	"SLCFLAGS"
#define CC_KW_SLLDFLAGS	"SLLDFLAGS"

/* reserved variable name */
#define SL_KW_LIB_VNONE	"SL_LIBNAME"
#define SL_KW_LIB_VMAJ	"SL_LIBNAME_VMAJ"
#define SL_KW_LIB_VFULL	"SL_LIBNAME_VFULL"
#define SL_KW_LIB_NAME	"SL_NAME"
#define SL_KW_LIB_MAJ	"SL_MAJOR"
#define SL_KW_LIB_MIN	"SL_MINOR"


/*************************
 * code of various tests *
 ***********************************************************************/

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


/****************************
 * compiler data structures *
 ***********************************************************************/

typedef struct {
	char	*c_id,
		*descr,
		*c_macro,
		*v_macro,
		*slcflags,
		*slldflags;
} comp_cell;

typedef struct {
	char	*c_id,
		*version;
} comp_info;

typedef struct {
	htable	*cht,
		*sht;
} comp_data;


/***********************
 * function prototypes *
 ***********************************************************************/

comp_data	*compdata_init(size_t, size_t);
void		 compdata_destroy(comp_data *);
void		 compcell_destroy(comp_cell *);
bool		 add_compiler(comp_data *, htable *);
bool		 add_system(comp_data *, htable *, char *);
comp_cell	*comp_get(comp_data *, char *c_id);
char		*comp_get_descr(comp_data *, char *);
comp_data	*parse_comp_file_adv(char *, htable *);
comp_data	*parse_comp_file(char *);
bool		 gen_test_file(FILE *, comp_data *);
bool		 detect_compiler(char *, char *, comp_data *, comp_info *);

#endif /* _DETECT_H_ */

