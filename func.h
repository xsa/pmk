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

#ifndef _PMK_FUNC_H_
#define _PMK_FUNC_H_

#include "pmk.h"
#include "parse.h"

/* format string for include check */
#define INC_TEST_CODE	"#include <stdio.h>\n" \
			"#include <%s>\n" \
			"int main() {return(0);}"

#define INC_FUNC_TEST_CODE	"#include <stdio.h>\n" \
				"#include <%s>\n" \
				"void (*pmk_funcp)() = %s;\n" \
				"int main() {\n" \
				"return(0);}"

#define LIB_TEST_CODE	"#include <stdio.h>\n" \
			"int main() {return(0);}"

/* args: func_name func_name */
#define LIB_FUNC_TEST_CODE	"#include <stdio.h>\n" \
				"int %s();\n" \
				"int main() {\n" \
				"printf(\"%%p\", %s);\n" \
				"return(0);}"

#define TYPE_TEST_CODE	"#include <stdio.h>\n" \
			"int main() {\n" \
			"%s test_type;\n" \
			"if (sizeof(test_type)) {return(0);}\n" \
		        "return(0);}"

#define TYPE_INC_TEST_CODE	"#include <stdio.h>\n" \
				"#include <%s>\n" \
				"int main() {\n" \
				"%s test_type;\n" \
				"if (sizeof(test_type)) {return(0);}\n" \
			        "return(0);}"

/* args: struct_name struct_name member_name*/
#define TYPE_MEMBER_TEST_CODE	"#include <stdio.h>\n" \
				"int main() {\n" \
				"%s test_struct;\n" \
				"if (sizeof(test_struct.%s)) {return(0);}\n" \
			        "return(0);}"

/* args: header struct_name struct_name member_name*/
#define TYPE_INC_MEMBER_TEST_CODE	"#include <stdio.h>\n" \
					"#include <%s>\n" \
					"int main() {\n" \
					"%s test_struct;\n" \
					"if (sizeof(test_struct.%s)) {return(0);}\n" \
				        "return(0);}"

/* compiler command format string */
				/* compiler cflags binary source log */
#define HEADER_CC_FORMAT	"%s %s -o %s %s >>%s 2>&1"
				/* compiler cflags objfile source log */
#define HEADER_FUNC_CC_FORMAT	"%s %s -o %s -c %s >>%s 2>&1"
				/* compiler libflags binary testlib source log */
#define LIB_CC_FORMAT		"%s %s -o %s -l%s %s >>%s 2>&1"
				/* compiler binary source log */
#define TYPE_CC_FORMAT		"%s -o %s %s >>%s 2>&1"

/* file names */
#define C_FILE_EXT	".c"
#define TEST_FILE_NAME	TMPDIR "/pmk_XXXXXXXX" C_FILE_EXT
#define BIN_TEST_NAME	TMPDIR "/pmk_XXXXXXXX_bin"

/* node tokens */
#define PMK_TOK_SETNGS	1
#define PMK_TOK_DEFINE	2
#define PMK_TOK_SWITCH	3
#define PMK_TOK_IFCOND	4

/* special setting tokens */
#define PMK_TOK_SETVAR	10 /* set variable */
#define PMK_TOK_SETPRM	11 /* set parameter */

/* item command tokens */
/* checks */
#define PMK_TOK_CHKBIN	20
#define PMK_TOK_CHKINC	21
#define PMK_TOK_CHKLIB	22
#define PMK_TOK_CHKCFG	23
#define PMK_TOK_CHKPKG	24
#define PMK_TOK_CHKTYP	25
#define PMK_TOK_CHKVAR	26
/* shared lib stuff */
#define PMK_TOK_BLDSLN	30


#define KW_SETNGS_GLANG		"LANG"
#define KW_SETNGS_TARGET	"TARGET"
#define KW_SETNGS_ACCOMP	"AC_COMPAT"
#define KW_SETNGS_ACCOMP	"AC_COMPAT"
#define KW_SETNGS_CCDTCT	"DETECT"

#define KW_SL_VERS_NONE		"VERSION_NONE"
#define KW_SL_VERS_MAJ		"VERSION_MAJ"
#define KW_SL_VERS_FULL		"VERSION_FULL"


/* structures */
typedef struct {
	char	kw[CMD_LEN];
	bool	(*fnp)(pmkcmd *, htable *, pmkdata *);
} cmdkw;

bool	func_wrapper(prscell *, pmkdata *);
bool	process_node(prsnode *, pmkdata *);
bool	pmk_define(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_target(pmkcmd *, htable *, pmkdata *);
bool	pmk_ac_compat(pmkcmd *, htable *, pmkdata *);
bool	pmk_settings(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_ifcond(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_switches(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_binary(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_header(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_lib(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_config(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_pkg_config(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_type(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_variable(pmkcmd *, htable *, pmkdata *);
bool	pmk_build_shlib_name(pmkcmd *, htable *, pmkdata *);

bool	pmk_set_parameter(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_accompat(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_glang(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_target(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_detect(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_set_variable(pmkcmd *, prsopt *, pmkdata *);

#endif /* _PMK_FUNC_H_ */
