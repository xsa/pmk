/* $Id$ */

/*
 * Copyright (c) 2005 Damien Couderc
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


#include "prseng.h"


/*#define DEBUG_PRSENG	1*/


/**********
 constants
***********************************************************************/

/* Preprocessor keywords */
#define RKW_PP_DEF	"define"
#define RKW_PP_ELIF	"elif"
#define RKW_PP_ELSE	"else"
#define RKW_PP_ENDF	"endif"
#define RKW_PP_ERR	"error"
#define RKW_PP_IF	"if"
#define RKW_PP_IFDF	"ifdef"
#define RKW_PP_IFND	"ifndef"
#define RKW_PP_INCL	"include"
#define RKW_PP_LINE	"line"
#define RKW_PP_PRGM	"pragma"
#define RKW_PP_UDEF	"undef"

/* assembler directives */
#define RKW_AS_TSEG	"text"
#define RKW_AS_DSEG	"data"

/* C keywords */
#define RKW_C_BOOL	"_Bool"			/* ISO C99 */
#define RKW_C_CMPLX	"_Complex"		/* ISO C?? *//* XXX */
#define RKW_C_IMGNR	"_Imaginary"	/* ISO C?? *//* XXX */
#define RKW_C_AUTO	"auto"			/* ISO C?? *//* XXX */
#define RKW_C_BREAK	"break"			/* ISO C90 */
#define RKW_C_CASE	"case"			/* ISO C90 */
#define RKW_C_CHAR	"char"			/* ISO C90 */
#define RKW_C_CONST	"const"			/* ISO C90 */
#define RKW_C_CONTN	"continue"		/* ISO C90 */
#define RKW_C_DFLT	"default"		/* ISO C90 */
#define RKW_C_DO	"do"			/* ISO C90 */
#define RKW_C_DBL	"double"		/* ISO C90 */
#define RKW_C_ELSE	"else"			/* ISO C90 */
#define RKW_C_ENUM	"enum"			/* ISO C90 */
#define RKW_C_EXTRN	"extern"		/* ISO C90 */
#define RKW_C_FLOAT	"float"			/* ISO C90 */
#define RKW_C_FOR	"for"			/* ISO C90 */
#define RKW_C_GOTO	"goto"			/* ISO C90 */
#define RKW_C_IF	"if"			/* ISO C90 */
#define RKW_C_INLN	"inline"		/* ISO C?? *//* XXX */
#define RKW_C_INT	"int"			/* ISO C90 */
#define RKW_C_LONG	"long"			/* ISO C90 */
#define RKW_C_RGSTR	"register"		/* ISO C90 */
#define RKW_C_RSTCT	"restrict"		/* ISO C99 *//* XXX */
#define RKW_C_RTRN	"return"		/* ISO C90 */
#define RKW_C_SHORT	"short"			/* ISO C90 */
#define RKW_C_SGND	"signed"		/* ISO C90 */
#define RKW_C_SIZOF	"sizeof"		/* ISO C90 */
#define RKW_C_STTC	"static"		/* ISO C90 */
#define RKW_C_STRCT	"struct"		/* ISO C90 */
#define RKW_C_SWTCH	"switch"		/* ISO C90 */
#define RKW_C_TPDEF	"typedef"		/* ISO C90 */
#define RKW_C_UNION	"union"			/* ISO C90 */
#define RKW_C_USGND	"unsigned"		/* ISO C90 */
#define RKW_C_VOID	"void"			/* ISO C90 */
#define RKW_C_VLTL	"volatile"		/* ISO C?? *//* XXX */
#define RKW_C_WHILE	"while"			/* ISO C90 */

/* C++ extra keywords */
#define RKW_CXX_AND		"and"				/* ISO C++98 */
#define RKW_CXX_ANDEQ	"and_eq"			/* ISO C++98 */
#define RKW_CXX_ASM		"asm"				/* ISO C++98 */
#define RKW_CXX_AUTO	"auto"				/* ISO C++98 */
#define RKW_CXX_BITAND	"bitand"			/* ISO C++98 */
#define RKW_CXX_BITOR	"bitor"				/* ISO C++98 */
#define RKW_CXX_BOOL	"bool"				/* ISO C++98 */
#define RKW_CXX_BREAK	"break"				/* ISO C++98 */
#define RKW_CXX_CASE	"case"				/* ISO C++98 */
#define RKW_CXX_CATCH	"catch"				/* ISO C++98 */
#define RKW_CXX_CHAR	"char"				/* ISO C++98 */
#define RKW_CXX_CLASS	"class"				/* ISO C++98 */
#define RKW_CXX_COMPL	"compl"				/* ISO C++98 */
#define RKW_CXX_CONST	"const"				/* ISO C++98 */
#define RKW_CXX_CNSTCST	"const_cast"		/* ISO C++98 */
#define RKW_CXX_CONTN	"continue"			/* ISO C++98 */
#define RKW_CXX_DFLT	"default"			/* ISO C++98 */
#define RKW_CXX_DELETE	"delete"			/* ISO C++98 */
#define RKW_CXX_DO		"do"				/* ISO C++98 */
#define RKW_CXX_DBL		"double"			/* ISO C++98 */
#define RKW_CXX_DYNCAST	"dynamic_cast"		/* ISO C++98 */
#define RKW_CXX_ELSE	"else"				/* ISO C++98 */
#define RKW_CXX_ENUM	"enum"				/* ISO C++98 */
#define RKW_CXX_EXPLI	"explicit"			/* ISO C++98 */
#define RKW_CXX_EXPORT	"export"			/* ISO C++98 */
#define RKW_CXX_EXTRN	"extern"			/* ISO C++98 */
#define RKW_CXX_FALSE	"false"				/* ISO C++98 */
#define RKW_CXX_FLOAT	"float"				/* ISO C++98 */
#define RKW_CXX_FOR		"for"				/* ISO C++98 */
#define RKW_CXX_FRIEND	"friend"			/* ISO C++98 */
#define RKW_CXX_GOTO	"goto"				/* ISO C++98 */
#define RKW_CXX_IF		"if"				/* ISO C++98 */
#define RKW_CXX_INLN	"inline"			/* ISO C++98 */
#define RKW_CXX_INT		"int"				/* ISO C++98 */
#define RKW_CXX_LONG	"long"				/* ISO C++98 */
#define RKW_CXX_MUTABL	"mutable"			/* ISO C++98 */
#define RKW_CXX_NSPC	"namespace"			/* ISO C++98 */
#define RKW_CXX_NEW		"new"				/* ISO C++98 */
#define RKW_CXX_NOT		"not"				/* ISO C++98 */
#define RKW_CXX_NOTEQ	"not_eq"			/* ISO C++98 */
#define RKW_CXX_OPER	"operator"			/* ISO C++98 */
#define RKW_CXX_OR		"or"				/* ISO C++98 */
#define RKW_CXX_OREQ	"or_eq"				/* ISO C++98 */
#define RKW_CXX_PRIV	"private"			/* ISO C++98 */
#define RKW_CXX_PROT	"protected"			/* ISO C++98 */
#define RKW_CXX_PUBLIC	"public"			/* ISO C++98 */
#define RKW_CXX_RGSTR	"register"			/* ISO C++98 */
#define RKW_CXX_RINTCST	"reinterpret_cast"	/* ISO C++98 */
#define RKW_CXX_RTRN	"return"			/* ISO C++98 */
#define RKW_CXX_SHORT	"short"				/* ISO C++98 */
#define RKW_CXX_SGND	"signed"			/* ISO C++98 */
#define RKW_CXX_SIZOF	"sizeof"			/* ISO C++98 */
#define RKW_CXX_STTC	"static"			/* ISO C++98 */
#define RKW_CXX_STCCST	"static_cast"		/* ISO C++98 */
#define RKW_CXX_STRCT	"struct"			/* ISO C++98 */
#define RKW_CXX_SWTCH	"switch"			/* ISO C++98 */
#define RKW_CXX_TMPLT	"template"			/* ISO C++98 */
#define RKW_CXX_THIS	"this"				/* ISO C++98 */
#define RKW_CXX_THROW	"throw"				/* ISO C++98 */
#define RKW_CXX_TRUE	"true"				/* ISO C++98 */
#define RKW_CXX_TRY		"try"				/* ISO C++98 */
#define RKW_CXX_TYPEDEF	"typedef"			/* ISO C++98 */
#define RKW_CXX_TYPEID	"typeid"			/* ISO C++98 */
#define RKW_CXX_TYPENAM	"typename"			/* ISO C++98 */
#define RKW_CXX_UNION	"union"				/* ISO C++98 */
#define RKW_CXX_USGND	"unsigned"			/* ISO C++98 */
#define RKW_CXX_USING	"using"				/* ISO C++98 */
#define RKW_CXX_VIRT	"virtual"			/* ISO C++98 */
#define RKW_CXX_VOID	"void"				/* ISO C++98 */
#define RKW_CXX_VLTL	"volatile"			/* ISO C++98 */
#define RKW_CXX_WCHART	"wchar_t"			/* ISO C++98 */
#define RKW_CXX_WHILE	"while"				/* ISO C++98 */
#define RKW_CXX_XOR		"xor"				/* ISO C++98 */
#define RKW_CXX_XOREQ	"xor_req"			/* ISO C++98 */

#define MAX_IDTF_LEN	64	/* maximum length of an identifier *//* XXX enough ??? */

enum {
	SEG_TYPE_UNKNW = 0,
	SEG_TYPE_TEXT,
	SEG_TYPE_DATA
};

#define PRS_C_IDTF_STR		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
#define PRS_C_IDTF_FNAME	PRS_C_IDTF_STR "./"

#define PRS_C_MISC_STR	"]){}<>&|!?:;,=+-*/%.^~\\"


/*********************
 types and structures
***********************************************************************/

typedef struct {
	bool	 strict,	/* stop on errors when true */
			 (*func_ppro)(void *, char *, prseng_t *),	/* function to call on preprocessor keyword */
			 (*func_proc)(void *, char *, prseng_t *),	/* function called on procedure identifier */
			 (*func_decl)(void *, char *, prseng_t *),	/* function called on procedure declaration */
			 (*func_type)(void *, char *, prseng_t *);	/* function called on type identifier */
	void	*data;		/* data structure provided to the functions */
} prs_cmn_t;


/********************
 function prototypes
***********************************************************************/

bool	 prs_asm_file(prs_cmn_t *, FILE *);
bool	 prs_c_skip_to_char(prseng_t *, char);
bool	 prs_c_line_skip(prseng_t *);
bool	 prs_c_comment_skip(prseng_t *);
bool	 prs_c_squote_skip(prseng_t *);
bool	 prs_c_dquote_skip(prseng_t *);
bool	 prs_c_skip(prseng_t *);
bool	 prs_c_prepro(prs_cmn_t *, prseng_t *);
bool	 prs_c_is_kw(char *, char **, size_t);
bool	 prs_c_file(prs_cmn_t *, FILE *);
bool	 prs_cxx_file(prs_cmn_t *, FILE *);
bool	 prs_c_common(prs_cmn_t *, FILE *, char **, char **);

