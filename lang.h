/* $Id$ */

/*
 * Copyright (c) 2006 Damien Couderc
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


#ifndef _PMK_LANG_H_
#define _PMK_LANG_H_

#include <sys/types.h>

#include "premake.h"

/*************
 * constants *
 **********************************************************************************************/

/*************************
 * define language tokens *
 **********************************************************************************************
 * %CONST LANG_UNKNOWN:	unknown language token
 * %CONST LANG_C:		C language token
 * %CONST LANG_CXX:		C++ language token
 * %CONST LANG_NUMBER:	number of supported languages
 **********************************************************************************************/
enum {
	LANG_UNKNOWN = -1,	/* unknown language */
	LANG_C,				/* C language */
	LANG_CXX,			/* C++ language */
	LANG_NUMBER			/* number of languages */
};

/* language labels */
#define LANG_LABEL_C			"C"		/* C language */
#define LANG_LABEL_CXX			"C++"	/* C++ language */

/* compiler labels */
#define COMP_LABEL_C			"CC"	/* C language */
#define COMP_LABEL_CXX			"CXX"	/* C++ language */

/* compiler flags labels */
#define CFLAGS_LABEL_C			"CFLAGS"	/* C language */
#define CFLAGS_LABEL_CXX		"CXXFLAGS"	/* C++ language */

/* linker flags labels */
#define LDFLAGS_LABEL_C			"CLDFLAGS"		/* C language */
#define LDFLAGS_LABEL_CXX		"CXXLDFLAGS"	/* C++ language */

/* shared lib compiler flags labels */
#define SLCFLAGS_LABEL_C		"SLCFLAGS"		/* C language */
#define SLCFLAGS_LABEL_CXX		"SLCXXFLAGS"	/* C++ language */

/* shared lib linker flags labels */
#define SLLDFLAGS_LABEL_C		"SLCLDFLAGS"	/* C language */
#define SLLDFLAGS_LABEL_CXX		"SLCXXLDFLAGS"	/* C++ language */

/* shared libs build targets */
#define MK_BLD_TARGET_C			"c_shared_libs"
#define MK_BLD_TARGET_CXX		"cxx_shared_libs"

/* shared libs clean targets */
#define MK_CLN_TARGET_C			"c_shared_libs_clean"
#define MK_CLN_TARGET_CXX		"cxx_shared_libs_clean"

/* shared libs install targets */
#define MK_INST_TARGET_C		"c_shared_libs_install"
#define MK_INST_TARGET_CXX		"cxx_shared_libs_install"

/* shared libs deinstall targets */
#define MK_DEINST_TARGET_C		"c_shared_libs_deinstall"
#define MK_DEINST_TARGET_CXX	"cxx_shared_libs_deinstall"

#define LANG_NAME_LEN	64
#define COMP_NAME_LEN	64
#define PRE_NAME_LEN	64
#define CFLG_NAME_LEN	64
#define SHFLG_NAME_LEN	64
#define MK_RULE_LEN		64


/**********************************
 * type and structure definitions *
 ***********************************************************************/

/* language cell */
typedef struct {
	char	name[LANG_NAME_LEN],			/* language label (ex. C) */
			compiler[COMP_NAME_LEN],		/* compiler label (ex. CC) */
			cflags[CFLG_NAME_LEN],			/* compiler flags label (ex. CFLAGS) */
			ldflags[CFLG_NAME_LEN],			/* linker flags label (ex. CLDFLAGS) */
			slcflags[CFLG_NAME_LEN],		/* shared lib compiler flags label (ex. SLCFLAGS) */
			slldflags[CFLG_NAME_LEN],		/* shared lib linker flags label (ex. SLCLDFLAGS) */
			mk_bld_rule[MK_RULE_LEN],		/* */
			mk_cln_rule[MK_RULE_LEN],		/* */
			mk_inst_rule[MK_RULE_LEN],		/* */
			mk_deinst_rule[MK_RULE_LEN];	/* */
	int		lang;							/* language token */
} lgdata_t;


/********************
 * global variables *
 **********************************************************************************************/

extern lgdata_t	lang_data[];
extern size_t	nb_lang_data;

#endif /* _PMK_LANG_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
