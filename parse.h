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


#ifndef _PMK_PARSE_H_
#define _PMK_PARSE_H_

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

#define PRS_ERR_HASH		"hash add failure."
#define PRS_ERR_DYNARY		"dynary push failure."
#define PRS_ERR_OVERFLOW	"line too long."
#define PRS_ERR_SYNTAX		"syntax error."
#define PRS_ERR_TRAILING	"trailing garbage after value."
#define PRS_ERR_UNKNOWN		"unknown error."

/* keyword types */
#define PRS_KW_UNKW	0
#define PRS_KW_NODE	1
#define PRS_KW_ITEM	2

/* null token */
#define PRS_TOK_NULL	0

/* keyword structure */
typedef struct {
	char	*kw;		/* keyword string */
	int	 token,		/* associated token */
		 type,		/* type */
		 subtoken;	/* node specific subtoken */
} prskw;

typedef struct {
	char	 key[OPT_NAME_LEN];
	pmkobj	*value;
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
	int	 linenum;
	prsnode	*tree;
} prsdata;


prsdata	*prsdata_init(void);
void	 prsdata_destroy(prsdata *);
prsnode	*prsnode_init(void);
void	 prsnode_destroy(prsnode *);
prscell	*prscell_init(int, int, int);
void	 prscell_destroy(prscell *);
htable	*keyword_hash(prskw [], int);
char	*parse_quoted(char *, pmkobj *, size_t);
char	*parse_list(char *, pmkobj *, size_t);
char	*parse_word(char *, pmkobj *, size_t);
char	*parse_identifier(char *, char *, size_t);
char	*skip_blank(char *pstr);
prscell	*parse_cell(char *, htable *);
bool	 parse_pmkfile(FILE *, prsdata *, prskw [], size_t);
bool	 parse_opt(char *, prsopt *);

#endif /* _PMK_PARSE_H_ */
