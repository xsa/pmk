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
#define PMK_CHAR_ASSIGN		'='

#define PRS_ERR_HASH		"hash add failure."
#define PRS_ERR_SYNTAX		"syntax error."
#define PRS_ERR_TRAILING	"trailing garbage after value."
#define PRS_ERR_UNKNOWN		"unknown error."


typedef struct sprscell {
	char		 name[MAX_OPT_NAME_LEN],
			 label[MAX_LABEL_NAME_LEN];
	htable		*ht;
	struct sprscell	*next;
} prscell;

typedef struct {
	int	 linenum;
	prscell	*first,
		*last;
} prsdata;

prsdata	*prsdata_init(void);
void	 prsdata_destroy(prsdata *);
prscell	*prscell_init(void);
void	 prscell_destroy(prscell *);
char	*parse_quoted(char *, char *, size_t);
char	*parse_list(char *, char *, size_t);
char	*parse_word(char *, char *, size_t);
char	*skip_blank(char *pstr);
bool	 parse_cell(char *, prscell *);
bool	 parse(FILE *, prsdata *);
bool	 parse_opt(char *, htable *);

#endif /* _PMK_PARSE_H_ */
