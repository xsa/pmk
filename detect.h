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

#define COMPILER_TEST	\
	"#include <stdio.h>\n" \
	"/* GNU gcc */\n" \
	"#ifdef __GNUC__\n" \
	"#define CC_ID\t\"gnu_cc\"\n" \
	"#define CC_V\t__GNUC__\n" \
	"#define CC_VMIN\t__GNUC_MINOR__\n" \
	"#endif\n" \
	"\n" \
	"/* TenDRA */\n" \
	"#ifdef __TenDRA__\n" \
	"#define CC_ID\t\"tendra_cc\"\n" \
	"#endif\n" \
	"\n" \
	"/* Sun workshop C compiler */\n" \
	"#ifdef __SUNPRO_C\n" \
	"#define CC_ID\t\"sun_cc\"\n" \
	"#define CC_V\t__SUNPRO_C\n" \
	"#endif\n" \
	"\n" \
	"/* Sun workshop C++ compiler */\n" \
	"#ifdef __SUNPRO_CC\n" \
	"#define CC_ID\t\"sun_cxx\"\n" \
	"#define CC_V\t__SUNPRO_CC\n" \
	"#endif\n" \
	"\n" \
	"/* Compaq c */\n" \
	"#ifdef __DECC\n" \
	"#define CC_ID\t\"compaq_cc\"\n" \
	"#define CC_V\t__DEC_VER\n" \
	"#endif\n" \
	"\n" \
	"/* Compaq c++ */\n" \
	"#ifdef __DECCXX\n" \
	"#define CC_ID\t\"compaq_cxx\"\n" \
	"#define CC_V\t__DECXX_VER\n" \
	"#endif\n" \
	"\n" \
	"/* HP ansi C */\n" \
	"#ifdef __HP_cc\n" \
	"#define CC_ID\t\"hp_cc\"\n" \
	"#endif\n" \
	"\n" \
	"#ifdef __HP_aCC\n" \
	"#define CC_ID\t\"hp_cxx\"\n" \
	"#define CC_V\t__HP_aCC\n" \
	"#endif\n" \
	"\n" \
	"/* IBM xlC */\n" \
	"#ifdef __xlC__\n" \
	"#define CC_ID\t\"ibm_cc\"\n" \
	"#define CC_V\t__xlC__\n" \
	"#endif\n" \
	"\n" \
	"/* __IBMC__ */\n" \
	"\n" \
	"/* intel compiler */\n" \
	"#ifdef __INTEL_COMPILER\n" \
	"#define CC_ID\t\"intel_cc\"\n" \
	"#define CC_V\t__INTEL_COMPILER\n" \
	"#endif\n" \
	"\n" \
	"/* sgi compiler */\n" \
	"#ifdef __sgi\n" \
	"#define CC_ID\t\"sgi_cc\"\n" \
	"#define CC_V\t__COMPILER_VERSION\n" \
	"#endif\n" \
	"\n" \
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
	"\tprintf(\"%%s %%d\", CC_ID, CC_V);\n" \
	"#ifdef CC_VMIN\n" \
	"\tprintf(\".%%d\", CC_VMIN);\n" \
	"#endif\n" \
	"\tprintf(\"\\n\");\n" \
	"\treturn(0);\n" \
	"}\n"


bool	detect_compiler(char *, pmkdata *);

#endif /* _DETECT_H_ */
