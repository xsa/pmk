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


#ifndef _PMK_FUNCTOOL_H_
#define _PMK_FUNCTOOL_H_

#include "common.h"
#include "hash_tools.h"
#include "pmk.h"
#include "premake.h"


/*************
 * constants *
 **********************************************************************************************/

#define LANG_NAME_LEN	64
#define COMP_NAME_LEN	64
#define PRE_NAME_LEN	64
#define CFLG_NAME_LEN	64
#define SHFLG_NAME_LEN	64

#define KW_OPT_DEPEND	"DEPEND"
#define KW_OPT_LANG	"LANG"
#define KW_OPT_REQUIRED	"REQUIRED"

#define CHAR_VERSION_SEPARATOR	'.'

#define C_FILE_EXT	".c"
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
	char	*header,
			*define,
			*procedure,
			*type,
			*member;
	dynary	*subhdrs;
} code_bld_t;

typedef struct {
	char	name[LANG_NAME_LEN],
		comp[COMP_NAME_LEN],
		pre[PRE_NAME_LEN],
		cflg[CFLG_NAME_LEN],
		slflg[CFLG_NAME_LEN];
} lgdata;


/**************
 * prototypes *
 ***********************************************************************/

bool	 check_bool_str(char *);
bool	 invert_bool(bool);
char	*bool_to_str(bool);
bool	 get_file_dir_path(char *, char *, char *, int);
char	*str_to_def(char *);
char	*build_def_name(char *);
bool	 record_def(htable *, char *, bool);
bool	 record_def_data(htable *, char *, char *);
bool	 process_def_list(htable *, dynary *);
bool	 record_have(htable *, char *, char *);
bool	 record_val(htable *, char *, char *);
bool	 label_set(htable *, char *, bool);
bool	 label_check(htable *, char *);
bool	 depend_check(htable *, pmkdata *);
bool	 require_check(htable *);
lgdata	*check_lang(char *);
lgdata	*check_lang_comp(char *);
lgdata	*get_lang(htable *, pmkdata *);
char	*get_comp_path(htable *, char *);
bool	 check_cfgt_data(pmkdata *);
bool	 process_required(pmkdata *, pmkcmd *, bool , char *, char *);
bool	 c_file_builder(char *, size_t, char *, ...);
void	 code_bld_init(code_bld_t *);
bool	 c_code_builder(char *, size_t, code_bld_t *);
bool	 c_object_build(char *, size_t, char *, char *, char *, char *, bool);

#endif /* _PMK_FUNCTOOL_H_ */

