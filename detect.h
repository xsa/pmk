/* $Id$ */

/*
 * Copyright (c) 2003-2006 Damien Couderc
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
#include "hash_tools.h"
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
#define CC_TEST_BIN		TMPDIR "/cc_test_bin"
#define CC_TEST_FORMAT	"%s -o %s %s >%s 2>&1"

#define DEF_VERSION		"#define CC_V\t%s"
#define DEF_NOVERSION	"#undef CC_V"

#define SL_LDFLAG_VARNAME	"SLLDFLAGS"

#define SL_SYS_LABEL		"SLSYSNAME"


/****************
 * keyword data *
 ***********************************************************************/

/* ADD_COMPILER keyword options */
#define CC_KW_ID		"ID"
#define CC_KW_DESCR		"DESCR"
#define CC_KW_MACRO		"MACRO"
#define CC_KW_VERSION	"VERSION"
#define CC_KW_SLCFLAGS	"SLCFLAGS"
#define CC_KW_SLLDFLAGS	"SLLDFLAGS"

/* ADD_SYSTEM keyword options */
#define SYS_KW_NAME		"NAME"
#define SYS_KW_VERSION	"LIB_VERSION"
#define SYS_KW_SH_EXT	"SH_EXT"
#define SYS_KW_SH_NONE	"SH_LIBNAME_NONE"
#define SYS_KW_SH_VERS	"SH_LIBNAME_VERS"
#define SYS_KW_ST_EXT	"ST_EXT"
#define SYS_KW_ST_NONE	"ST_LIBNAME_NONE"
#define SYS_KW_ST_VERS	"ST_LIBNAME_VERS"

/* reserved variable name */
#define LIB_KW_MAJ	    "LIB_MAJOR"
#define LIB_KW_MIN	    "LIB_MINOR"
#define LIB_KW_NAME	    "LIB_NAME"
#define LIB_KW_SH_VAR	"SH_VAR_NAME"
#define LIB_KW_SH_NONE	SYS_KW_SH_NONE
#define LIB_KW_SH_VERS	SYS_KW_SH_VERS
#define LIB_KW_ST_VAR	"ST_VAR_NAME"
#define LIB_KW_ST_NONE	SYS_KW_ST_NONE
#define LIB_KW_ST_VERS	SYS_KW_ST_VERS


/************************
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


/***************************
 * compiler data structures *
 ***********************************************************************/

/* compiler cell */
typedef struct {
	char	*c_id,
			*descr,
			*c_macro,
			*v_macro,
			*slcflags,
			*slldflags;
} comp_prscell_t;

typedef struct {
	char	*c_id,
			*version;
} comp_info;

typedef struct {
	htable_t	*cht,	/* compiler data hash table */
				*sht;	/* system data hash table */
} comp_data;

/*********************
 * %TYPE comp_parse_t *
 ***********************************************************************
 * %DESCR compiler data storage for detection
 ***********************************************************************/
typedef struct {
	htable_t	*cht,	/* %FIELD cht:	compiler cell data hash table */
				*sht;	/* %FIELD sht:	system cell data hash table */
} comp_parse_t;

/*******************
 * %TYPE compiler_t *
 ***********************************************************************
 * %DESCR compiler profile containing data for shared lib support
 ***********************************************************************/
typedef struct {
	char	*c_id,		/* %FIELD c_id:			compiler identifier */
			*descr,		/* %FIELD descr:		compiler description */
			*c_macro,	/* %FIELD c_macro:		XXX */
			*v_macro,	/* %FIELD v_macro:		XXX */
			*slcflags,	/* %FIELD slcflags:		shared lib compiler flags */
			*slldflags,	/* %FIELD slldflags:	shared lib linker flags */
			*version;	/* %FIELD version:		detected version of the compiler */
	int		 lang;		/* %FIELD lang:			language identifier */
} compiler_t;

/********************
 * %TYPE comp_data_t *
 ***********************************************************************
 * %DESCR parsed compilers data
 ***********************************************************************/
typedef struct {
	compiler_t	*data;	/* %FIELD data:	array of data cells for detected compiler */
	size_t		 sz;	/* %FIELD sz:	number of data cells */
} comp_data_t;


/*********************
 * SECTION prototypes *
 ***********************************************************************/

bool			 init_compiler_data(comp_data_t	*, size_t);
void			 clean_compiler_cell(compiler_t *);
void			 clean_compiler_data(comp_data_t *);
void			 compcell_destroy(comp_prscell_t *);
comp_parse_t	*init_comp_parse(void);
void			 destroy_comp_parse(comp_parse_t *);
bool			 add_compiler(comp_parse_t *, htable_t *);
bool			 add_system(comp_parse_t *, htable_t *, char *);
comp_parse_t	*parse_comp_file(char *, char *);
bool			 gen_test_file(comp_parse_t *, char *, size_t);
bool			 comp_identify(char *, char *, compiler_t *, comp_parse_t *);
bool			 comp_detect(char *, char *, compiler_t *, comp_parse_t *, char *);

#endif /* _DETECT_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
