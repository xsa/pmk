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


#ifndef _PMK_PARSE_H_
#define _PMK_PARSE_H_

#include "compat/pmk_stdbool.h"
#include "pmk_obj.h"

/*
#define LIST_SUPPORT	1
*/


#define PMK_CHAR_COMMENT	'#'
#define PMK_CHAR_COMMAND	'.'
#define PMK_CHAR_COMMAND_START	'{'
#define PMK_CHAR_COMMAND_END	'}'
#define PMK_CHAR_LABEL_START	'('
#define PMK_CHAR_LABEL_END	')'
#define PMK_CHAR_QUOTE_START	'"'
#define PMK_CHAR_QUOTE_END	PMK_CHAR_QUOTE_START
#define PMK_CHAR_LIST_START	'('
#define PMK_CHAR_LIST_END	')'
#define PMK_CHAR_LIST_SEP	','
#define PMK_CHAR_ASSIGN		'='
#define PMK_CHAR_ESCAPE		'\\'

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

#define PRS_ERR_ALLOC		"memory allocation failed."
#define PRS_ERR_HASH		"hash add failure."
#define PRS_ERR_DYNARY		"dynary push failure."
#define PRS_ERR_OVERFLOW	"line too long."
#define PRS_ERR_SYNTAX		"syntax error."
#define PRS_ERR_TRAILING	"trailing garbage after value."
#define PRS_ERR_UNKNOWN		"unknown error."

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
#define PRS_BUF_LEN            512

/* keyword structure */
typedef struct {
	char	*kw;		/* keyword string */
	int	 token,		/* associated token */
		 type,		/* type */
		 subtoken;	/* node specific subtoken */
} prskw;

typedef struct {
	char	 key[OPT_NAME_LEN],	/* key name */
		 opchar;		/* operator */
	pmkobj	*value;			/* value */
} prsopt;

typedef struct s_prscell {
	int			 token,			/* item token id */
				 type;			/* item type */
	char			 label[LABEL_LEN];	/* command label */
	void			*data;			/* misc data */
	struct s_prscell	*next;			/* next item */
} prscell;

typedef struct {
	int	 token;		/* node specific token */
	prscell	*first,		/* first item of this node */
		*last;		/* last item of this node */
} prsnode;

typedef struct {
	int	 linenum;		/* current line */
	prsnode	*tree;			/* parser tree */
} prsdata;

typedef struct {
        FILE	*fp;			/* file structure pointer */
	bool	 eof;			/* end of file flag */
	char	 prsbuf[PRS_BUF_LEN],	/* buffer window */
		*prscur;		/* parsing cursor */
	htable	*phtkw;			/* command keywords */
	int	 linenum;		/* current line */
	long	 offset;		/* offset of the buffer window */
} prseng;

prsdata	*prsdata_init(void);
void	 prsdata_destroy(prsdata *);
prseng	*prseng_init(void);
void	 prseng_destroy(prseng *);
prsnode	*prsnode_init(void);
void	 prsnode_destroy(prsnode *);
prscell	*prscell_init(int, int, int);
void	 prscell_destroy(prscell *);
prsopt	*prsopt_init(void);
prsopt	*prsopt_init_adv(char *, char, char *);
void	 prsopt_destroy(prsopt *);
bool	 prs_get_line(FILE *, char *, size_t);
bool	 prs_fill_buf(prseng *);
htable	*keyword_hash(prskw [], int);
char	*skip_blank(char *pstr);
void	 skip_useless(prseng *);
char	*parse_identifier(char *, char *, size_t);
char	*parse_obsolete(char *, char *, size_t);
char	*parse_bool(char *, pmkobj *, size_t);
char	*parse_quoted(char *, pmkobj *, size_t);
char	*parse_list(char *, pmkobj *, size_t);
char	*parse_key(char *, pmkobj *, size_t);
char	*parse_data(char *, pmkobj *, size_t);
prscell	*parse_cell(char *, htable *);
prscell	*parse_cmd_header(prseng *peng, prsnode *pnode);
bool	 parse_opt(char *, prsopt *, char *);
bool	 parse_opt_new(prseng *, prsopt *, char *);
bool	 parse_clopt(char *, prsopt *, char *);
bool	 parse_node(prsdata *, prseng *peng, prscell *);
bool	 parse_command(prsdata *, prseng *peng, prscell *);
bool	 parse_opt_block(prsdata *, prseng *, prscell *, bool);
bool	 parse_cmd_block(prsdata *, prseng *, prsnode *, bool);
bool	 parse_pmkfile(FILE *, prsdata *, prskw [], size_t);
bool	 process_opt(htable *, prsopt *);
bool	 parse_pmkconf(FILE *, htable *, char *, bool (*)(htable *, prsopt *));

#endif /* _PMK_PARSE_H_ */
