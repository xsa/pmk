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
				"int main() {\n" \
				"printf(\"%%p\", %s);\n" \
				"return(0);}"

#define LIB_TEST_CODE	"#include <stdio.h>\n" \
			"int main() {return(0);}"

#define LIB_FUNC_TEST_CODE	"#include <stdio.h>\n" \
				"int %s();\n" \
				"int main() {\n" \
				"printf(\"%%p\", %s);\n" \
				"return(0);}"

#define TYPE_TEST_CODE	"#include <stdio.h>\n" \
			"int main() {\n" \
			"if ((%s *) 0) {return(0);}\n" \
			"if (sizeof(%s)) {return(0);}\n" \
		        "return(0);}"


#define TEST_FILE_NAME	"test.c"
#define BIN_TEST_NAME	"test_bin"

/* node tokens */
#define PMK_TOK_SETNGS	1
#define PMK_TOK_DEFINE	2
#define PMK_TOK_SWITCH	3
#define PMK_TOK_TARGET	4 /* will be obsolete by SETTINGS */
#define PMK_TOK_ACCOMP	5 /* will be obsolete by SETTINGS */

/* special setting tokens */
#define PMK_TOK_SETVAR	9 /* set variable */
#define PMK_TOK_SETPRM	10 /* set parameter */

/* item command tokens */
#define PMK_TOK_CHKBIN	17
#define PMK_TOK_CHKINC	18
#define PMK_TOK_CHKLIB	19
#define PMK_TOK_CHKCFG	20
#define PMK_TOK_CHKPKG	21
#define PMK_TOK_CHKTYP	22
#define PMK_TOK_CHKVAR	23

/*
#define KW_SETNGS_GLANG		"LANG"
#define KW_SETNGS_TARGET	"TARGET"
#define KW_SETNGS_ACCOMP	"ACCOMP"
*/


typedef struct {
	char	kw[CMD_LEN];
	bool	(*fnp)(pmkcmd *, htable *, pmkdata *);
} cmdkw;

bool	func_wrapper(prscell *, pmkdata *);
bool	process_node(prsnode *, pmkdata *);
bool	pmk_define(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_target(pmkcmd *, htable *, pmkdata *);
bool	pmk_ac_compat(pmkcmd *, htable *, pmkdata *);
bool	pmk_switches(pmkcmd *, htable *, pmkdata *);
bool	pmk_settings(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_check_binary(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_include(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_lib(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_config(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_pkg_config(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_type(pmkcmd *, htable *, pmkdata *);
bool	pmk_check_variable(pmkcmd *, htable *, pmkdata *);
bool	pmk_set_parameter(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_set_variable(pmkcmd *, prsopt *, pmkdata *);

#endif /* _PMK_FUNC_H_ */
