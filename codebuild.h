/* $Id$ */

/*
 * Copyright (c) 2005-2006 Damien Couderc
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


#ifndef _PMK_CODEBUILD_H_
#define _PMK_CODEBUILD_H_

#include "compat/pmk_stdbool.h"
#include "compat/pmk_stdio.h"

#include "dynarray.h"
#include "premake.h"


/*************
 * constants *
 **********************************************************************************************/

enum {
	LANG_UNKNOWN = 0,
	LANG_C,
	LANG_CXX
};

typedef unsigned int	lang_t;


#define LANG_NAME_LEN	64
#define COMP_NAME_LEN	64
#define PRE_NAME_LEN	64
#define CFLG_NAME_LEN	64
#define SHFLG_NAME_LEN	64

#define C_FILE_EXT		".c"
#define TEST_FILE_NAME	TMPDIR "/pmk_XXXXXXXX" C_FILE_EXT
#define BIN_TEST_NAME	TMPDIR "/pmk_XXXXXXXX_bin"


#define CODE_C_HDR		"#include <%s>\n"
#define CODE_C_BEG		"/* main procedure */\n" \
						"int main() {\n"
#define CODE_C_END		"return(0);\n" \
						"}\n"
#define CODE_C_DEF		"/* check define */\n" \
						"#ifndef %s\n" \
						"break_build_process();\n" \
						"#endif\n"
#define CODE_C_PROC		"/* check procedure */\n" \
						"void (*pmk_funcp)() = (void *) %s;\n"
#define CODE_C_VAR		"%s test_var;\n\n"
#define CODE_C_TYPE		"/* check type */\n" \
						"if (sizeof(test_var)) {\n" \
						"\treturn(0);\n" \
						"}\n"
#define CODE_C_MEMBER	"/* check structure member */\n" \
						"if (sizeof(test_var.%s)) {\n" \
						"\treturn(0);\n" \
						"}\n"


/**********************************
 * type and structure definitions *
 ***********************************************************************/

typedef struct {
	char	name[LANG_NAME_LEN],
			compiler[COMP_NAME_LEN],
			cflags[CFLG_NAME_LEN],
			slflags[CFLG_NAME_LEN];
	lang_t	lang;
} lgdata_t;

typedef struct {
	char		 srcfile[MAXPATHLEN],
				 binfile[MAXPATHLEN],
				 bldcmd[MAXPATHLEN],
				*header,				/* header filename */
				*library,				/* library name */
				*define,				/* macro name */
				*procedure,				/* procedure name */
				*type,					/* type name */
				*member,				/* type member name */
				*pathcomp,				/* compiler path */
				*flags,					/* compilation flags */
				*alt_cflags,			/* alternative compilation flags variable */
				*alt_libs,				/* alternative linker flags variable */
				*blog;					/* build log */
	dynary		*subhdrs;				/* header dependencies */
	lang_t		 lang;					/* language */
	lgdata_t	*pld;
} code_bld_t;


/**************
 * prototypes *
 ***********************************************************************/

void	 code_bld_init(code_bld_t *, char *);
bool	 set_language(code_bld_t *, char *);
char	*set_compiler(code_bld_t *, htable *t);
char	*get_lang_label(code_bld_t *);
char	*get_cflags_label(code_bld_t *);
char	*get_libs_label(code_bld_t *);
void	 code_logger(FILE *, FILE *, const char *, ...);
bool	 code_builder(code_bld_t *);
bool	 c_code_builder(code_bld_t *);
bool	 cmdline_builder(code_bld_t *, bool);
bool	 c_cmdline_builder(code_bld_t *, bool);
bool	 object_builder(code_bld_t *);
void	 cb_cleaner(code_bld_t *);

#endif /* _PMK_CODEBUILD_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
