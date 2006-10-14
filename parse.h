/* $Id$ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
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


#ifndef _PMK_PARSE_H_
#define _PMK_PARSE_H_

#include "compat/pmk_stdbool.h"
#include "pmk_obj.h"
#include "premake.h"
#include "prseng.h"


#define KW_SIZEOF(s)	sizeof(s) / sizeof(kw_t)


/*************
 * constants *
 ***********************************************************************/

#define PMK_CHAR_COMMENT		'#'
#define PMK_CHAR_COMMAND		'.'
#define PMK_CHAR_COMMAND_START	'{'
#define PMK_CHAR_COMMAND_END	'}'
#define PMK_CHAR_LABEL_START	'('
#define PMK_CHAR_LABEL_END		')'
#define PMK_CHAR_QUOTE_START	'"'
#define PMK_CHAR_QUOTE_END		PMK_CHAR_QUOTE_START
#define PMK_CHAR_LIST_START		'('
#define PMK_CHAR_LIST_END		')'
#define PMK_CHAR_LIST_SEP		','
#define PMK_CHAR_ASSIGN			'='
#define PMK_CHAR_ESCAPE			'\\'

#ifndef BOOL_STRING_TRUE
#define PMK_BOOL_TRUE		"TRUE"
#else
#define PMK_BOOL_TRUE		BOOL_STRING_TRUE
#endif

#ifndef BOOL_STRING_FALSE
#define PMK_BOOL_FALSE		"FALSE"
#else
#define PMK_BOOL_FALSE		BOOL_STRING_FALSE
#endif

#define PRS_PMKFILE_SEP		"="
#define PRS_PMKCONF_SEP		"=:"

#define PRS_ERR_UNKNOWN		"unknown error."
#define PRS_ERR_ALLOC		"memory allocation failed."
#define PRS_ERR_HASH		"hash add failure."
#define PRS_ERR_DYNARY		"dynary push failure."
#define PRS_ERR_OVERFLOW	"line too long."
#define PRS_ERR_SYNTAX		"syntax error."
#define PRS_ERR_TRAILING	"trailing garbage after value."
#define PRS_ERR_PRSFILL		"parsing buffer could not be filled."
#define PRS_ERR_INV_OPT		"invalid option '%s'."
#define PRS_ERR_TYP_OPT		"wrong type for option '%s'."
#define PRS_ERR_PRSENG		"parser engine error."


#define PRS_PMK_IDTF_STR	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
#define PRS_BOOL_IDTF_STR	"AEFLRSTU"	/* letters of TRUE and FALSE */


/* keyword types */
#define PRS_KW_UNKW	0
#define PRS_KW_NODE	1
#define PRS_KW_CELL	2
#define PRS_KW_ITEM	3

/* null token */
#define PRS_TOK_NULL	0

/* maximal number of options per command */
#define MAX_CMD_OPT		32

/* parsing buffer size */
#define PRS_BUF_LEN		1024


/************************
 * types and structures *
 ***********************************************************************/

typedef struct {
	char	*name;
	potype	 mask;
} kw_t;

/* keyword options */
typedef struct {
	kw_t	*req;
	size_t	 nbreq;
	kw_t	*opt;
	size_t	 nbopt;
} kwopt_t;

/* keyword structure */
typedef struct {
	char	*kw;		/* keyword string */
	int		 token,		/* associated token */
			 type,		/* type */
			 subtoken;	/* node specific subtoken */
	kwopt_t	*kwo;		/* keyword specific options */
} prskw;

/* parser option structure */
typedef struct {
	char	 key[OPT_NAME_LEN],	/* key name */
			 opchar;			/* operator */
	pmkobj	*value;				/* value */
} prsopt;

/* parser cell structure */
typedef struct s_prscell {
	int					 token,				/* item token id */
						 type;				/* item type */
	char				 label[LABEL_LEN];	/* command label */
	void				*data;				/* misc data */
	struct s_prscell	*next;				/* next item */
} prscell;

/* parser node structure */
typedef struct {
	int		 token;		/* node specific token */
	prscell	*first,		/* first item of this node */
			*last;		/* last item of this node */
} prsnode;

/* parser data structure */
typedef struct {
	int		 linenum;		/* current line */
	prsnode	*tree;			/* parser tree */
} prsdata;

/* misc data structure */
typedef struct {
	void	*data;			/* misc_data */
	htable	*phtkw;			/* command keywords */
	kwopt_t	*kwopts;		/* pointer to keyword def */
	size_t	 nbreq;			/* number of required options */
} miscdata_t;


/***********************
 * function prototypes *
 ***********************************************************************/

prsdata	*prsdata_init(void);
void	 prsdata_destroy(prsdata *);
prsnode	*prsnode_init(void);
void	 prsnode_add(prsnode *, prscell *);
void	 prsnode_destroy(prsnode *);
prscell	*prscell_init(int, int, int);
void	 prscell_destroy(prscell *);
prsopt	*prsopt_init(void);
prsopt	*prsopt_init_adv(char *, char, char *);
void	 prsopt_destroy(prsopt *);
htable	*keyword_hash(prskw [], int);
bool	 prs_skip_blank(prseng_t *);
bool	 prs_skip_comment(prseng_t *);
bool	 prs_skip_useless(prseng_t *);
bool	 parse_label(prseng_t *, char *, size_t);
bool	 parse_bool(prseng_t *, pmkobj *, size_t);
bool	 parse_quoted(prseng_t *, pmkobj *, size_t);
bool	 parse_list(prseng_t *, pmkobj *, size_t);
bool	 parse_key(prseng_t *, pmkobj *, size_t);
bool	 parse_data(prseng_t *, pmkobj *, size_t);
prscell	*parse_cell(char *, htable *);
prscell	*parse_cmd_header(prseng_t *, prsnode *pnode);
bool	 parse_opt(prseng_t *, prsopt *, char *);
bool	 parse_clopt(char *, prsopt *, char *);
bool	 parse_node(prsdata *, prseng_t *, prscell *);
bool	 parse_command(prsdata *, prseng_t *, prscell *);
kw_t	*check_opt_avl(char *, kw_t *, size_t);
bool	 check_opt_type(kw_t *, pmkobj *);
bool	 check_option(miscdata_t *, prsopt *);
bool	 process_block_opt(prseng_t *, prsnode *, prscell *);
bool	 parse_opt_block(prsdata *, prseng_t *, prscell *, bool);
bool	 parse_cmd_block(prsdata *, prseng_t *, prsnode *, bool);
bool	 parse_pmkfile(FILE *, prsdata *, prskw [], size_t);
bool	 process_opt(htable *, prsopt *);
bool	 parse_pmkconf(FILE *, htable *, char *, bool (*)(htable *, prsopt *));

#endif /* _PMK_PARSE_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
