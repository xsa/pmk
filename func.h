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

#ifndef _PMK_FUNC_H_
#define _PMK_FUNC_H_

#include "compat/pmk_stdbool.h"
#include "pmk.h"
#include "parse.h"

/*************
 * constants *
 ***********************************************************************/

/* node tokens */
#define PMK_TOK_SETNGS	1
#define PMK_TOK_DEFINE	2
#define PMK_TOK_SWITCH	3
#define PMK_TOK_IFCOND	4
#define PMK_TOK_ELCOND	5

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
/* lib stuff */
#define PMK_TOK_BLDLIB	30
#define PMK_TOK_BLDSLN	31


/* option keywords */
/* common */
#define KW_OPT_NAME			"NAME"
#define KW_OPT_DEFS			"DEFS"
#define KW_OPT_FUNCTION		"FUNCTION"
#define KW_OPT_HEADER		"HEADER"
#define KW_OPT_CFLAGS		"CFLAGS"
#define KW_OPT_LANG			"LANG"
#define KW_OPT_LIBS			"LIBS"
#define KW_OPT_MACRO		"MACRO"
#define KW_OPT_MAJOR		"MAJOR"
#define	KW_OPT_MEMBER		"MEMBER"
#define KW_OPT_MINOR		"MINOR"
#define KW_OPT_MODULE		"MODULE"
#define KW_OPT_SUBHDR		"SUBHDR"
#define KW_OPT_VERSION		"VERSION"
#define KW_OPT_VALUE		"VALUE"
#define KW_OPT_VARIABLE		"VARIABLE"
/* SETTINGS */
#define KW_SETNGS_GLANG		"LANG"
#define KW_SETNGS_TARGET	"TARGET"
#define KW_SETNGS_ACCOMP	"AC_COMPAT"
#define KW_SETNGS_ACCOMP	"AC_COMPAT"
#define KW_SETNGS_CCDTCT	"DETECT"
/* library name building */
#define KW_SL_VERSION		"VERSION"
#define KW_SL_STATIC		"STATIC"
#define KW_SL_SHARED		"SHARED"
/* obsolete stuff *//* XXX OBOSLETE */
#define KW_SL_VERS_NONE		"VERSION_NONE"
#define KW_SL_VERS_MAJ		"VERSION_MAJ"
#define KW_SL_VERS_FULL		"VERSION_FULL"


#define DEFINE_DEFAULT		"1"


/* error messages */
#define ERR_MSG_CC_CMD		"failed to build compiler command line."


/**********************************
 * type and structure definitions *
 ***********************************************************************/

/* structures */
typedef struct {
	char	kw[CMD_LEN];
	bool	(*fnp)(pmkcmd *, htable_t *, pmkdata *);
} cmdkw;


/**************
 * prototypes *
 ***********************************************************************/

bool	func_wrapper(prscell *, pmkdata *);
bool	process_node(prsnode *, pmkdata *);
bool	pmk_define(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_target(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_ac_compat(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_settings(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_ifcond(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_elcond(pmkcmd *, prsnode *, pmkdata *);
bool	pmk_switches(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_binary(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_header(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_lib(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_config(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_pkg_config(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_type(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_check_variable(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_build_lib_name(pmkcmd *, htable_t *, pmkdata *);
bool	pmk_build_shlib_name(pmkcmd *, htable_t *, pmkdata *);

bool	pmk_set_parameter(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_accompat(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_glang(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_target(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_setparam_detect(pmkcmd *, prsopt *, pmkdata *);
bool	pmk_set_variable(pmkcmd *, prsopt *, pmkdata *);

#endif /* _PMK_FUNC_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
