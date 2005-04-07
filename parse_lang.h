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

/* C keywords */
#define RKW_C_BOOL	"_Bool"
#define RKW_C_CMPLX	"_Complex"
#define RKW_C_IMGNR	"_Imaginary"
#define RKW_C_AUTO	"auto"
#define RKW_C_BREAK	"break"
#define RKW_C_CASE	"case"
#define RKW_C_CHAR	"char"
#define RKW_C_CONST	"const"
#define RKW_C_CONTN	"continue"
#define RKW_C_DFLT	"default"
#define RKW_C_DO	"do"
#define RKW_C_DBL	"double"
#define RKW_C_ELSE	"else"
#define RKW_C_ENUM	"enum"
#define RKW_C_EXTRN	"extern"
#define RKW_C_FLOAT	"float"
#define RKW_C_FOR	"for"
#define RKW_C_GOTO	"goto"
#define RKW_C_IF	"if"
#define RKW_C_INLN	"inline"
#define RKW_C_INT	"int"
#define RKW_C_LONG	"long"
#define RKW_C_RGSTR	"register"
#define RKW_C_RSTCT	"restrict"
#define RKW_C_RTRN	"return"
#define RKW_C_SHORT	"short"
#define RKW_C_SGND	"signed"
#define RKW_C_SIZOF	"sizeof"
#define RKW_C_STTC	"static"
#define RKW_C_STRCT	"struct"
#define RKW_C_SWTCH	"switch"
#define RKW_C_TPDEF	"typedef"
#define RKW_C_UNION	"union"
#define RKW_C_USGND	"unsigned"
#define RKW_C_VOID	"void"
#define RKW_C_VLTL	"volatile"
#define RKW_C_WHILE	"while"


#define MAX_IDTF_LEN	64	/* maximum length of an identifier *//* XXX enough ??? */

#define PRS_C_IDTF_STR		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
#define PRS_C_IDTF_FNAME	PRS_C_IDTF_STR "./"

#define PRS_C_MISC_STR	"]){}<>&|!?:;,=+-*/%.^~\\"


/*********************
 types and structures
***********************************************************************/

typedef struct {
	bool	 strict;				/* stop on errors when true */
	void	 (*func_ppro)(char *, prseng_t *),	/* function called on preprocessor keyword */
		 (*func_proc)(char *, prseng_t *),	/* function called on procedure identifier */
		 (*func_type)(char *, prseng_t *),	/* function called on type identifier */
		*data;					/* data structure provided to the functions */
} prs_cmn_t;


/********************
 function prototypes
***********************************************************************/

bool	 prs_c_skip_to_char(prseng_t *, char);
bool	 prs_c_line_skip(prseng_t *);
bool	 prs_c_comment_skip(prseng_t *);
bool	 prs_c_squote_skip(prseng_t *);
bool	 prs_c_dquote_skip(prseng_t *);
void	 prs_c_skip(prseng_t *);
bool	 prs_c_prepro(prs_cmn_t *, prseng_t *);
bool	 prs_c_is_kw(char *);
bool	 prs_c_file(prs_cmn_t *, FILE *);

