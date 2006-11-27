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


#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"

#include "common.h"
#include "parse.h"
#include "prseng.h"


/*#define PRS_DEBUG	1*/


char	parse_err[MAX_ERR_MSG_LEN];


/******************
 * prsdata_init() *
 ***********************************************************************
 DESCR
	initialize parsing data structure

 IN
	NONE

 OUT
	prsdata structure
 ***********************************************************************/

prsdata *prsdata_init(void) {
	prsdata	*pdata;

	pdata = (prsdata *) malloc(sizeof(prsdata));
	if (pdata != NULL) {
		pdata->linenum = 0;
		pdata->tree = prsnode_init();
		if (pdata->tree == NULL) {
			/* tree init failed, clean allocated structure */
			prsdata_destroy(pdata);
			pdata = NULL;
		}
	}

	return(pdata);
}


/*********************
 * prsdata_destroy() *
 ***********************************************************************
 DESCR
	free space allocated by parsing data structure

 IN
	pdata:	structure to clean

 OUT
	NONE
 ***********************************************************************/

void prsdata_destroy(prsdata *pdata) {
#ifdef PRS_DEBUG
	debugf("prsdata_destroy() : cleaning tree");
#endif
	prsnode_destroy(pdata->tree);
#ifdef PRS_DEBUG
	debugf("prsdata_destroy() : cleaning structure");
#endif
	free(pdata);
#ifdef PRS_DEBUG
	debugf("prsdata_destroy() : exit");
#endif
}


/******************
 * prsnode_init() *
 ***********************************************************************
 DESCR
	initialize parsing node structure

 IN
	NONE

 OUT
	parsing node structure
 ***********************************************************************/

prsnode *prsnode_init(void) {
	prsnode	*pnode;

	pnode = (prsnode *) malloc(sizeof(prsnode));
	if (pnode != NULL) {
		pnode->first = NULL;
		pnode->last = NULL;
	}

	return(pnode);
}


/*****************
 * prsnode_add() *
 ***********************************************************************
 DESCR
	add a cell to a node

 IN
	pnode:	target node
	pcell:	cell to add

 OUT
	NONE
 ***********************************************************************/

void prsnode_add(prsnode *pnode, prscell *pcell) {
#ifdef PRS_DEBUG
	if (pnode == NULL) {
		debugf("prsnode_add() : pnode is NULL !");
	}

	if (pcell == NULL) {
		debugf("prsnode_add() : pcell is NULL !");
	}
#endif

	if (pnode->last != NULL) {
		/* linking to last item of node */
		pnode->last->next = pcell;
	} else {
		/* empty node, link as first */
		pnode->first = pcell;
	}
	pnode->last = pcell;
}


/*********************
 * prsnode_destroy() *
 ***********************************************************************
 DESCR
	free space allocated by parsing node structure

 IN
	pnode:	node structure to clean

 OUT
	NONE
 ***********************************************************************/

void prsnode_destroy(prsnode *pnode) {
	prscell	*p,
			*n;

	if (pnode != NULL) {
		p = pnode->first;

		while (p != NULL) {
			n = p;
			p = n->next;
			prscell_destroy(n);
		}

		free(pnode);
	}
}


/******************
 * prscell_init() *
 ***********************************************************************
 DESCR
	initialize parsing cell structure

 IN
	token:		cell token
	type:		cell type
	subtoken:	cell subtoken

 OUT
	parsing cell structure or NULL if failed
 ***********************************************************************/

prscell *prscell_init(int token, int type, int subtoken) {
	bool		 rval = false;
	htable_t	*pht;
	prscell		*pcell;
	prsnode		*pnode;
	prsopt		*popt;

	pcell = (prscell *) malloc(sizeof(prscell));

	if (pcell != NULL) {
		pcell->token = token;
		pcell->type = type;

		switch(type) {
			case PRS_KW_NODE :
				pnode = prsnode_init();
				if (pnode != NULL) {
					pnode->token = subtoken;
					pcell->data = pnode;
					rval = true;
				}
				break;

			case PRS_KW_CELL :
				pht = hash_create(MAX_CMD_OPT,
									false,
									(void *(*)(void *, void *, void *)) po_append,
									(void *(*)(void *)) po_dup,
									(void (*)(void *)) po_free);
				if (pht != NULL) {
					pcell->data = pht;
					rval = true;
				}
				break;

			case PRS_KW_ITEM :
				popt = (prsopt *) malloc(sizeof(prsopt));
				if (popt != NULL) {
					pcell->data = popt;
					rval = true;
				}
				break;

			default :
				break;
		}
	}

	if (rval == true) {
		pcell->next = NULL;
	} else {
		free(pcell);
		pcell = NULL;
	}

	return(pcell);
}


/*********************
 * prscell_destroy() *
 ***********************************************************************
 DESCR
	free space allocated for parsing cell structure

 IN
	pcell:	structure to clean

 OUT
	NONE
 ***********************************************************************/

void prscell_destroy(prscell *pcell) {
#ifdef PRS_DEBUG
	/*debugf("prscell_destroy().");*/
#endif
	if (pcell != NULL) {
		if (pcell->data != NULL) {
			switch(pcell->type) {
				case PRS_KW_NODE :
					prsnode_destroy(pcell->data);
					break;
				case PRS_KW_CELL :
					hash_destroy(pcell->data);
					break;
				default :
					break;
			}
		}
		free(pcell);
	}
}


/*****************
 * prsopt_init() *
 ***********************************************************************
 DESCR
	init a prsopt structure

 IN
	NONE

 OUT
	prsopt structure
  ***********************************************************************/

prsopt *prsopt_init(void) {
	prsopt	*ppo;

	ppo = (prsopt *) malloc(sizeof(prsopt));

	return(ppo);
}


/*********************
 * prsopt_init_adv() *
 ***********************************************************************
 DESCR
	init a prsopt structure with given aguments

 IN
	key:	parse option key name
	opchar:	operation character
	value:	parse option value

 OUT
	prsopt structure
 ***********************************************************************/

prsopt *prsopt_init_adv(char *key, char opchar, char *value) {
	prsopt	*ppo;

	ppo = (prsopt *) malloc(sizeof(prsopt));
	if (ppo == NULL) {
		return(NULL);
	}

	if (strlcpy_b(ppo->key, key, sizeof(ppo->key)) == false) {
		free(ppo);
		return (NULL); /* truncated */
	}

	ppo->opchar = opchar;

	if (value != NULL) {
		ppo->value = po_mk_str(value);
		if (ppo->value == NULL) {
			free(ppo);
			return(NULL); /* pmkobject failed */
		}
	} else {
		/* permit value = NULL */
		ppo->value = NULL;
	}

	return(ppo);
}


/********************
 * prsopt_destroy() *
 ***********************************************************************
 DESCR
	destroy a prsopt structure

 IN
	ppo:	prsopt structure to free

 OUT
	NONE
 ***********************************************************************/

void prsopt_destroy(prsopt *ppo) {
	if (ppo->value != NULL) {
		po_free(ppo->value);
	}
	free(ppo);
}


/******************
 * keyword_hash() *
 ***********************************************************************
 DESCR
	create hash table with keyword structures

 IN
	kwtab:	keyword table
	nbkw:	size of table

 OUT
	hash table
 ***********************************************************************/

htable_t *keyword_hash(prskw kwtab[], int nbkw) {
	htable_t	*phkw;
	int			 i;
	prskw		*pkw;

	phkw = hash_create(nbkw, false, NULL, (void *(*)(void *))strdup, free);
	if (phkw != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < nbkw ; i++) {
			pkw = (prskw *) malloc(sizeof(prskw));
			memmove(pkw, &kwtab[i], sizeof(prskw));
			if (hash_update(phkw, kwtab[i].kw,	/* no need to strdup */
					pkw) == false) {
				hash_destroy(phkw);
				errorf("failed to fill keywords hash table.");
				return(NULL);
			}
		}
	}

	return(phkw);
}


/********************
 * prs_skip_blank() *
 ***********************************************************************
 DESCR
	skip blank character(s)

 IN
	ppe:	parsing engine structure

 OUT
	new parsing cursor
 ***********************************************************************/

bool prs_skip_blank(prseng_t *ppe) {
	while (prseng_test_idtf_char(" \t",
				prseng_get_char(ppe)) == true) {
		if (prseng_next_char(ppe) == false) {
			return false;
		}
	}
	return true;
}


/**********************
 * prs_skip_comment() *
 ***********************************************************************
 DESCR
	skip a comment

 IN
	ppe:	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool prs_skip_comment(prseng_t *ppe) {
	bool	loop = true;
	char	c;

	/* check for end of line or end of file */
	while (loop == true) {
#ifdef PRS_DEBUG
		debugf("check if '%c' == CR.", prseng_get_char(ppe));
#endif

		c = prseng_get_char(ppe);

		/* reached end of line or buffer ? */
		if ((c == CHAR_CR) || (c == CHAR_EOS)) {
			loop = false;
		}

		/* next char */
		if (prseng_next_char(ppe) == false) {
			return false;
		}
	}

	return true;
}


/**********************
 * prs_skip_useless() *
 ***********************************************************************
 DESCR
	skip useless text (blanks, comments, empty lines)

 IN
	ppe:	parsing engine structure

 OUT
	NONE
 ***********************************************************************/

bool prs_skip_useless(prseng_t *ppe) {
	bool	 loop = true;

#ifdef PRS_DEBUG
	debugf("prs_skip_useless() : process start on line %d", ppe->linenum);
#endif
	while (loop == true) {
#ifdef PRS_DEBUG
		debugf("prs_skip_useless() : char = '%c'", prseng_get_char(ppe));
#endif
		switch(prseng_get_char(ppe)) {
			case ' ' :
			case '\t' :
			case '\n' :
				if (prseng_next_char(ppe) == false) {
					return false;
				}
				break;

			case PMK_CHAR_COMMENT :
				/* comment ? */
#ifdef PRS_DEBUG
				debugf("prs_skip_useless() : skip comment on line %d.", ppe->linenum);
#endif
				prs_skip_comment(ppe);
				break;

			default:
				loop = false;
		}
	}

#ifdef PRS_DEBUG
	debugf("prs_skip_useless() : process ended on line %d with char '%c'",
									ppe->linenum, prseng_get_char(ppe));
#endif
	return true;
}


/*****************
 * parse_label() *
 ***********************************************************************
 DESCR
	parse a label

 IN
	ppe:	parsing engine structure
	pbuf:	storage buffer
	size:	size of buffer

 OUT
	new parsing cursor or NULL
 ***********************************************************************/

bool parse_label(prseng_t *ppe, char *pbuf, size_t size) {
	char	c;

	c = prseng_get_char(ppe);
	if (c == '!') {
		*pbuf = c;
		pbuf++;
		size--;
		if (prseng_next_char(ppe) == false) {
			return false;
		}
	}

	return(prseng_get_idtf(ppe, pbuf, size, PRS_PMK_IDTF_STR));
}


/****************
 * parse_bool() *
 ***********************************************************************
 DESCR
	parse a bool value

 IN
	ppe:	parsing engine structure
	po:		storage pmk object
	size:	size of buffer

 OUT
	new parsing cursor or NULL
 ***********************************************************************/

bool parse_bool(prseng_t *ppe, pmkobj *po, size_t size) {
	bool	*pb;
	char	*pbuf,
			 buf[6];

	pb = (bool *) malloc(sizeof (bool));
	if (pb == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC,
				sizeof(parse_err)); /* no check */
		return false;
	}

	if (size > sizeof(buf))
		size = sizeof(buf);

	pbuf = buf;

	/* get bool identifier */
	if (prseng_get_idtf(ppe, buf, size, PRS_BOOL_IDTF_STR) == false) {
		strlcpy(parse_err, PRS_ERR_PRSENG, sizeof(parse_err)); /* no check */
		return false;
	}

	/* is it true value ? */
	if (strncmp(buf, PMK_BOOL_TRUE, sizeof(buf)) == 0) {
		*pb = true;
	} else {
		/* is it false value ? */
		if (strncmp(buf, PMK_BOOL_FALSE, sizeof(buf)) == 0) {
			*pb = false;
		} else {
			/* unknown value identifier */
			strlcpy(parse_err, "unknown boolean identifier.",
					sizeof(parse_err)); /* no check */
			free(pb);
			return false;
		}
	}

	po->type = PO_BOOL;
	po->data = pb;

	return true;
}


/******************
 * parse_quoted() *
 ***********************************************************************
 DESCR
	parse quoted string content

 IN
	ppe:	parsing engine structure
	po:		storage pmk object
	size:	size of buffer

 OUT
	new parsing cursor
 ***********************************************************************/

bool parse_quoted(prseng_t *ppe, pmkobj *po, size_t size) {
	bool	 escape = false,
			 loop = true;
	char	*buffer,
			*pbuf,
			 c;

	buffer = (char *) malloc(size);
	if (buffer == NULL) {
		return false;
	}

	pbuf = buffer;

	/* leave one char for end of line */
	size--;

	/* skip starting double quote */
	if (prseng_next_char(ppe) == false) {
		return false;
	}

	if (prseng_eof(ppe) == true) {
		free(buffer);
		strlcpy(parse_err, "ending quote is missing.", sizeof(parse_err)); /* no check */
		return false;
	}

	/* unset escape flag */
	escape = false;
	while (loop == true) {

		c = prseng_get_char(ppe);
		switch (c) {
			case CHAR_EOS :
				strlcpy(parse_err, "ending quote is missing.",
										sizeof(parse_err)); /* no check */
				return false;

			case PMK_CHAR_ESCAPE :
#ifdef PRS_DEBUG
				debugf("parse_quoted() : found escape char.");
#endif
				if (escape == true) {
					/* double escape */
					escape = false;
#ifdef PRS_DEBUG
					debugf("parse_quoted() : escape = false.");
#endif
				} else {
					/* found first escape, set flag */
					escape = true;
#ifdef PRS_DEBUG
					debugf("parse_quoted() : escape = true.");
#endif
				}
				break;

			case PMK_CHAR_QUOTE_END :
				/* if not escaped ... */
				if (escape == false) {
					/* ... found end quote */
					loop = false;
				} else {
					escape = false;
				}
				break;


			default :
				escape = false;
				break;
		}

		/* if not escape or ending quote */
		if ((escape == false) && (loop == true)) {
			/* copy character */
			*pbuf = c;
			pbuf++;
			size--;

			/* check remaining space */
			if (size < 0) {
				free(buffer);
				strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err)); /* no check */
				return false;
			}
		}

		if (prseng_next_char(ppe) == false) {
			free(buffer);
			return false;
		}
	}

	/* found end of quoted string */
	*pbuf = CHAR_EOS;
	po->type = PO_STRING;

	/* use strdup() to get rid of the extra space that could result
		of the allocation with the max size of buffer */
	po->data = strdup(buffer);

	/* deallocate the now useless buffer */
	free(buffer);

	if (po->data == NULL) {
		/* strdup() failed */
		errorf(ERRMSG_MEM);
		return false;
	}

	return true;
}


/****************
 * parse_list() *
 ***********************************************************************
 DESCR
	parse a list

 IN
	ppe:	parsing engine structure
	po:		storage pmk object
	size:	size of buffer

 OUT
	new parsing cursor
 ***********************************************************************/

bool parse_list(prseng_t *ppe, pmkobj *po, size_t size) {
	bool	 loop = true,
		 prev_data = false;
	char	*buffer,
		 c;
	dynary	*pda;
	pmkobj	 potmp;

	pda = da_init();
	if (pda == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err)); /* no check */
		return false;
	}

	/* skip starting list char */
	if (prseng_next_char(ppe) == false) {
		return false;
	}

	while (loop == true) {
		/* skip blank characters */
		if (prs_skip_blank(ppe) == false) {
			return false;
		}

		c = prseng_get_char(ppe);

		switch (c) {
			case PMK_CHAR_LIST_END :
#ifdef PRS_DEBUG
				debugf("found end of list.");
#endif
				loop = false;
				break;

			case PMK_CHAR_QUOTE_START :
				/* check if data was right before */
				if (prev_data == true) {
					/* yes => syntax error */
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
					return false;
				}

				/* found string data */
				if (parse_quoted(ppe, &potmp, size) == false) {
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
					return false;
				}

#ifdef PRS_DEBUG
				debugf("add '%s' into dynary (quoted)", potmp.data);
#endif

				if (da_push(pda, potmp.data) == false) {
					da_destroy(pda);
					return false;
				}

				/* set data flag */
				prev_data = true;

				/* already at next character, continue */
				continue;
				break;

			case PMK_CHAR_LIST_SEP :
				/* check if data was right before */
				if (prev_data == false) {
					/* no => syntax error */
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
					return false;
				}

#ifdef PRS_DEBUG
				debugf("found separator.");
#endif
				/* unset data flag */
				prev_data = false;
				break;

			case CHAR_CR :
				/* ignore end of line */
#ifdef PRS_DEBUG
				debugf("found end of line.");
#endif
				break;

			case CHAR_EOS :
				/* reached end of file */
				da_destroy(pda);
				strlcpy(parse_err, "end of list not found.",
												sizeof(parse_err)); /* no check */
				return false;
				break;

			default :
				/* check if data was right before */
				if (prev_data == true) {
					/* yes => syntax error */
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
					return false;
				}

				buffer = (char *) malloc(size);
				if (buffer == NULL) {
					strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err)); /* no check */
					return false;
				}

				if (prseng_get_idtf(ppe, buffer, size,
						PRS_PMK_IDTF_STR) == false) {
					da_destroy(pda);
					/* XXX better msg */
					strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err)); /* no check */
				}

				/* XXX useless ? */
				if (*buffer == CHAR_EOS) {
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
					return false;
				}

#ifdef PRS_DEBUG
				debugf("add '%s' into dynary (simple).", buffer);
#endif

				if (da_push(pda, buffer) == false) {
					da_destroy(pda);
					return false;
				}

				/* set data flag */
				prev_data = true;
				break;
		}

		if (prseng_next_char(ppe) == false) {
			return false;
		}
	}

	/* check if data was right before */
	if (prev_data == false) {
		/* no => syntax error */
		da_destroy(pda);
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
#ifdef PRS_DEBUG
		debugf("no data before end of list.");
#endif
		return(NULL);
	}

	/* found end of list */
	po->type = PO_LIST;
	po->data = pda;

	return true;
}


/***************
 * parse_key() *
 ***********************************************************************
 DESCR
	parse key

 IN
	ppe:	parsing engine structure
	po:		storage pmk object
	size:	size of buffer

 OUT
	new parsing cursor or NULL
 ***********************************************************************/

bool parse_key(prseng_t *ppe, pmkobj *po, size_t size) {
	char	*buffer;

	po->type = PO_NULL;

#ifdef PRS_DEBUG
	debugf("parse_key() : cursor = '%.32s'", ppe->cur);
#endif

	if (prseng_get_char(ppe) == PMK_CHAR_QUOTE_START) {
#ifdef PRS_DEBUG
		debugf("parse_key() : found quoted string");
#endif
		/* found a quoted string */
		if (parse_quoted(ppe, po, size) == false) {
			return false;
		}
	} else {
#ifdef PRS_DEBUG
		debugf("parse_key() : found identifier");
#endif
		/* identifier */
		buffer = (char *) malloc(size);
		if (buffer == NULL) {
			strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err)); /* no check */
			return false;
		}

		if (prseng_get_idtf(ppe, buffer, size, PRS_PMK_IDTF_STR) == false) {
			free(buffer);
			/* XXX better msg ? */
			strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err)); /* no check */
			return false;
		}

		po->type = PO_STRING;
		po->data = buffer;
	}

	return true;
}


/****************
 * parse_data() *
 ***********************************************************************
 DESCR
	parse data

 IN
	ppe:	parsing engine structure
	po:		storage pmk object
	size:	size of buffer

 OUT
	boolean
 ***********************************************************************/

bool parse_data(prseng_t *ppe, pmkobj *po, size_t size) {
	po->type = PO_NULL;

#ifdef PRS_DEBUG
			debugf("parse_data(): first character = '%c'", prseng_get_char(ppe));
#endif
	switch (prseng_get_char(ppe)) {
		/* found a quoted string */
		case PMK_CHAR_QUOTE_START :
#ifdef PRS_DEBUG
			debugf("parse_data(): calling parse_quoted()");
#endif
			if (parse_quoted(ppe, po, size) == false) {
				return false;
			}
			break;

		/* found a list */
		case PMK_CHAR_LIST_START :
#ifdef PRS_DEBUG
			debugf("parse_data(): calling parse_list()");
#endif
			if (parse_list(ppe, po, size) == false) {
				return false;
			}
			break;

		default :
#ifdef PRS_DEBUG
			debugf("parse_data(): calling parse_bool()");
#endif
			if (parse_bool(ppe, po, size) == false) {
				return false;
			}
			break;
	}

	return true;
}


/**********************
 * parse_cmd_header() *
 ***********************************************************************
 DESCR
	parse command header

 IN
	ppe:	parser engine structure
	pnode:	parser node structure

 OUT
	parser cell structure on success or NULL
 ***********************************************************************/

prscell *parse_cmd_header(prseng_t *ppe, prsnode *pnode) {
	char		 name[CMD_LEN];
	miscdata_t	*pmd;
	prscell		*pcell;
	prskw		*pkw;

	if (prseng_get_idtf(ppe, name, sizeof(name),
										PRS_PMK_IDTF_STR) == false) {
		strlcpy(parse_err, "command parsing failed.", sizeof(parse_err)); /* no check */
		return(NULL);
	}
#ifdef PRS_DEBUG
	debugf("parse_cmd_header(): cmd name = '%s'", name);
#endif

	/* get misc data */
	pmd = (miscdata_t *) ppe->data;

	/* check validity of the command */
	pkw = hash_get(pmd->phtkw, name);
	if (pkw != NULL) {
		/* command ok, creating cell */
		pcell = prscell_init(pkw->token, pkw->type, pkw->subtoken);
		if (pcell == NULL) {
			strlcpy(parse_err, "pcell init failed.", sizeof(parse_err)); /* no check */
			return(NULL);
		}
#ifdef PRS_DEBUG
		debugf("parse_cmd_header(): cmd token = %d", pcell->token);
		debugf("parse_cmd_header(): cmd type = %d", pcell->type);
#endif

		/* check if options have to be checked */
		if (pkw->type == PRS_KW_CELL) {
			/* yes, keep pointer in prseng structure */
			pmd->kwopts = pkw->kwo;
			if (pmd->kwopts != NULL) {
				/* assign number of required opts */
				pmd->nbreq = pmd->kwopts->nbreq;
			} else {
				/* no required opts */
				pmd->nbreq = 0;
			}
		} else {
			pmd->kwopts = NULL;
			pmd->nbreq = 0;
		}
	} else {
		/* unknown command */
		snprintf(parse_err, sizeof(parse_err), "unknown command '%s'.", name);
		return(NULL);
	}

	/* look for a label */
	if (prseng_test_char(ppe, PMK_CHAR_LABEL_START)) {
		/* label delimiter found */
		if (prseng_next_char(ppe) == false) {
			return(NULL);
		}

		/* get label name */
		if (parse_label(ppe, pcell->label, sizeof(pcell->label)) == false) {
			strlcpy(parse_err, "label parsing failed.", sizeof(parse_err)); /* no check */
			prscell_destroy(pcell);
			return(NULL);
		}

#ifdef PRS_DEBUG
		debugf("parse_cmd_header(): cmd label = '%s'", pcell->label);
#endif

		/* check label's ending delimiter */
		if (prseng_test_char(ppe, PMK_CHAR_LABEL_END) == false) {
			/* label name must be immediately followed by closing delimiter */
			strlcpy(parse_err, "label parsing failed.", sizeof(parse_err)); /* no check */
			prscell_destroy(pcell);
			return(NULL);
		}

		if (prseng_next_char(ppe) == false) {
			return(NULL);
		}

#ifdef PRS_DEBUG
		debugf("parse_cmd_header(): parsed command '%s' with label '%s'", name, pcell->label);
	} else {
		debugf("parse_cmd_header(): parsed command '%s' with no label", name);
#endif
	}


	/* look for command block starting sequence */

	if (prseng_test_char(ppe, ' ') == false) {
		strlcpy(parse_err, PRS_ERR_UNKNOWN, sizeof(parse_err)); /* no check */
		prscell_destroy(pcell);
		return(NULL);
	}

	if (prseng_next_char(ppe) == false) {
		return(NULL);
	}

	if (prseng_test_char(ppe, PMK_CHAR_COMMAND_START) == false) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err)); /* no check */
		prscell_destroy(pcell);
		return(NULL);
	}

	if (prseng_next_char(ppe) == false) {
		return(NULL);
	}

	if (prseng_test_char(ppe, CHAR_CR) == false) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err)); /* no check */
		prscell_destroy(pcell);
		return(NULL);
	}

	/* add cell in the node */
	prsnode_add(pnode, pcell);

	return(pcell);
}


/***************
 * parse_opt() *
 ***********************************************************************
 DESCR
	parse an option

 IN
	ppe:	parsing engine structure
	popt:	option storage structure
	seplst:	string that contain all separator characters

 OUT
	boolean
 ***********************************************************************/

bool parse_opt(prseng_t *ppe, prsopt *popt, char *seplst) {
	char	 c;
	pmkobj	 po;

#ifdef PRS_DEBUG
	debugf("parse_opt() : line = '%.32s'", ppe->cur);
#endif

	if (parse_key(ppe, &po, sizeof(popt->key)) == false) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
		return false;
	}

	if (strlcpy_b(popt->key, po.data, sizeof(popt->key)) == false) {
		free(po.data);
		return false;
	}

	free(po.data);

#ifdef PRS_DEBUG
	debugf("parse_opt() : key = '%s'", popt->key);
#endif

	if (prs_skip_blank(ppe) == false) {
		return false;
	}

	/* check if character is in separator list */
	c = prseng_get_char(ppe);
	if (prseng_test_idtf_char(seplst, c) == false) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
		return false;
	} else {
		popt->opchar = c;
		if (prseng_next_char(ppe) == false) {
			return false;
		}
	}
#ifdef PRS_DEBUG
	debugf("parse_opt() : assign = '%c'", popt->opchar);
#endif

	if (prs_skip_blank(ppe) == false) {
		return false;
	}
	popt->value = (pmkobj *) malloc(sizeof(pmkobj));
	if (popt->value == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err)); /* no check */
		return false;
	}

	/* parse data */
	if (parse_data(ppe, popt->value, OPT_VALUE_LEN) == false) {
		po_free(popt->value);
		/*strlcpy(parse_err, PRS_ERR_SYNTAX,                        */
		/*                        sizeof(parse_err)); |+ no check +|*/
		return false;
	}

#ifdef PRS_DEBUG
	switch (popt->value->type) {
		case PO_BOOL :
			debugf("parse_opt() : value = *BOOL*");
			break;
		case PO_STRING :
			debugf("parse_opt() : value = '%s'", popt->value->data);
			break;
		case PO_LIST :
			debugf("parse_opt() : value = (*LIST*)");
			break;
		default :
			debugf("parse_opt() : value = !unknown type!");
			break;
	}
#endif

	return true;
}


/*****************
 * parse_clopt() *
 ***********************************************************************
 DESCR
	parse a command line option

 IN
	line:	option line
	popt:	storage structure
	seplst:	string that contain all separator characters

 OUT
	boolean
 ***********************************************************************/

bool parse_clopt(char *line, prsopt *popt, char *seplst) {
	char		 c;
	pmkobj		 po;
	prseng_t	*ppe;

#ifdef PRS_DEBUG
	debugf("parse_clopt() : line = '%s'", line);
#endif

	ppe = prseng_init_str(line, NULL);
	if (ppe == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err)); /* no check */
		return false;
	}

	if (parse_key(ppe, &po, sizeof(popt->key)) == false) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
		return false;
	} else {
		if (strlcpy_b(popt->key, po.data, sizeof(popt->key)) == false) {
			free(po.data);
			return false;
		}

		free(po.data);
	}

#ifdef PRS_DEBUG
	debugf("key = '%s'", popt->key);
#endif

	/* check if character is in separator list */
	c = prseng_get_char(ppe);
	if (prseng_test_idtf_char(seplst, c) == false) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
		return false;
	} else {
		popt->opchar = c;
		if (prseng_next_char(ppe) == false) {
			return false;
		}
	}
#ifdef PRS_DEBUG
	debugf("assign = '%c'", popt->opchar);
#endif

	if (parse_data(ppe, &po, OPT_VALUE_LEN) == false) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
		return false;
	} else {
		if (po.type != PO_STRING) {
			strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err)); /* no check */
			return false;
		} else {
#ifdef PRS_DEBUG
			debugf("value = '%s'", po.data);
#endif

			popt->value = po_mk_str(po.data);
			free(po.data);
		}
	}

	return true;
}


/*******************
 * check_opt_avl() *
 ***********************************************************************
 DESCR
	check if the key is available in the given option array

 IN
	key:	key to search
	opts:	option array
	nbopts:	number of options

 OUT
	return the pointer to the kw_t structure else NULL
 ***********************************************************************/

kw_t *check_opt_avl(char *key, kw_t *opts, size_t nbopts) {
	int		i;
	size_t	s;

	s = strlen(key) + 1;

	for (i = 0 ; i < (int) nbopts ; i++) {
		if (strncmp(key, opts[i].name, s) == 0) {
			return(&opts[i]);
		}
	}

	return(NULL);
}


/********************
 * check_opt_type() *
 ***********************************************************************
 DESCR
	check if the option has a type allowed by the keyword mask

 IN
	pkw:	keyword structure
	po:		pmk object structure

 OUT
	boolean

 NOTE
	to disable type check set the mask to PO_NULL
 ***********************************************************************/

bool check_opt_type(kw_t *pkw, pmkobj *po) {
	if ((pkw->mask != PO_NULL) && ((pkw->mask & po->type) == 0)) {
		/*
			a type mask has been given but doesn't match
			with the option type
		*/
		return false;
	}

	/* the option type matches or the mask is set to PO_NULL */
	return true;
}


/******************
 * check_option() *
 ***********************************************************************
 DESCR
	check if the option name and type are valid

 IN
	pmd:	misc data structure
	ppo:	parse option structure

 OUT
	boolean
 ***********************************************************************/

bool check_option(miscdata_t *pmd, prsopt *ppo) {
	kw_t	*pkw;
	kwopt_t	*pkwo;

	pkwo = pmd->kwopts;
	if (pkwo == NULL) {
		/* nothing to check */
		return true;
	}

	if (pkwo->req != NULL) {
		/* check given required options */
		pkw = check_opt_avl(ppo->key, pkwo->req, pkwo->nbreq);
		if (pkw != NULL) {
			/* check option's type */
			if (check_opt_type(pkw, ppo->value) == false) {
				/* wrong option type */
				snprintf(parse_err, sizeof(parse_err),
											PRS_ERR_TYP_OPT, ppo->key);
				return false;
			}
			/* decrement required option counter */
			pmd->nbreq--;
			return true;
		}

		if (pkwo->opt == NULL) {
			/* allow any optional variable names */
			return true;
		}
	}

	if (pkwo->opt != NULL) {
		/* check given optional options */
		pkw = check_opt_avl(ppo->key, pkwo->opt, pkwo->nbopt);
		if (pkw != NULL) {
			/* check option's type */
			if (check_opt_type(pkw, ppo->value) == false) {
				/* wrong option type */
				snprintf(parse_err, sizeof(parse_err),
											PRS_ERR_TYP_OPT, ppo->key);
				return false;
			}
			return true;
		}
	}

	/* provided option is not expected */
	snprintf(parse_err, sizeof(parse_err), PRS_ERR_INV_OPT, ppo->key);
	return false;
}


/***********************
 * process_block_opt() *
 ***********************************************************************
 DESCR
	process a block option

 IN
	ppe:	parsing engine structure
	pnode:	parsing node structure
	pcell:	parent cell structure

 OUT
	boolean
 ***********************************************************************/

bool process_block_opt(prseng_t *ppe, prsnode *pnode, prscell *pcell) {
	prscell		*ncell;
	prsopt		*nopt,
			 opt;

#ifdef PRS_DEBUG
	debugf("process_block_opt() : calling parse_opt()");
#endif
	/* parse option */
	if (parse_opt(ppe, &opt, PRS_PMKFILE_SEP) == false) {
#ifdef PRS_DEBUG
		debugf("process_block_opt() : parse_opt() returned false");
#endif
		return false;
	}

#ifdef PRS_DEBUG
	debugf("process_block_opt() : recording '%s' key", opt.key);
#endif
	switch (pcell->type) {
		case PRS_KW_NODE :
			/*
				process a node
			*/

			/* init item's pcell */
			ncell = prscell_init(pnode->token, PRS_KW_ITEM, PRS_TOK_NULL);
			if (ncell == NULL) {
				errorf("process_block_opt() : prscell init failed");
				return false;
			}

			nopt = ncell->data;

			/* duplicate opt content in item */
			strlcpy(nopt->key, opt.key, sizeof(opt.key)); /* XXX check */
			nopt->value = opt.value;

			/* add item in cell node */
			prsnode_add(pnode, ncell);
			break;

		case PRS_KW_CELL :
			/*
				process a command
			*/

			/* check keyword option (NULL handled) */
			if (check_option(ppe->data, &opt) == false) {
				/* err msg done */
				return false;
			}

			/* verify if option is allowed */
			if (hash_get(pcell->data, opt.key) != NULL) {
				/* duplicate option */
				strlcpy(parse_err, "duplicate option.",
											sizeof(parse_err)); /* XXX */
				return false;
			}

			if (hash_update(pcell->data, opt.key, /* no need to strdup */
										opt.value) == false) {
				strlcpy(parse_err, PRS_ERR_HASH, sizeof(parse_err)); /* no check */
				return false;
			}
		break;
	}

	return true;
}


/*********************
 * parse_opt_block() *
 ***********************************************************************
 DESCR
	parse a block of options

 IN
	pdata:		parsing data structure
	ppe:		parsing engine structure
	pcell:		parent cell structure
	chk_delim:	check delimiter switch

 OUT
	boolean
 ***********************************************************************/

bool parse_opt_block(prsdata *pdata, prseng_t *ppe, prscell *pcell,
													bool chk_delim) {
	bool		 loop = true;
	miscdata_t	*pmd;
	prsnode		*pnode;

	pnode = pcell->data;

#ifdef PRS_DEBUG
		debugf("parse_opt_block() : pcell->type = %u.", pcell->type);
		debugf("parse_opt_block() : pnode->token = %u.", pnode->token);
#endif
	if ((pcell->type == PRS_KW_NODE) && (pnode->token == PRS_TOK_NULL)) {
#ifdef PRS_DEBUG
		debugf("parse_opt_block() : found a node, calling parse_cmd_block().");
#endif
		/* found a block node (like for example IF command) => parse the block */
		return(parse_cmd_block(pdata, ppe, pnode, true));
	}

#ifdef PRS_DEBUG
	debugf("parse_opt_block() : parsing options.");
#endif
	while (loop == true) {
		/* skip useless text */
		if (prs_skip_useless(ppe) == false) {
			return false;
		}

		switch(prseng_get_char(ppe)) {
			case PMK_CHAR_COMMAND_END :
#ifdef PRS_DEBUG
				debugf("parse_opt_block() : found end of block character.");
#endif
				if (prseng_next_char(ppe) == false) {
					return false;
				}

				if (chk_delim == true) {
					/* delimiter found, exit from the loop */
					loop = false;

					/* check if all required options have been parsed */
					pmd = (miscdata_t *) ppe->data;
					if (pmd->nbreq != 0) {
						snprintf(parse_err, sizeof(parse_err), /* no check */
							"at least one required option is missing (%d).",
													(int) pmd->nbreq);
						return false;
					}
				} else {
					/* delimiter not expected */
					return false;
				}
				break;

			case CHAR_EOS :
				if (chk_delim == true) {
					/* expected delimiter not found */
					return false;
				} else {
					/* main loop, no delimiter was expected => ok */
					return true;
				}
				break;

			default :
				if (process_block_opt(ppe, pnode, pcell) == false) {
					/* error message already set */
					return false;
				}
				break;
		}
	}

	return true;
}


/*********************
 * parse_cmd_block() *
 ***********************************************************************
 DESCR
	parse a block of commands

 IN
	pdata:		parsing data structure
	ppe:		parsing engine structure
	pnode:		node structure
	chk_delim:	check delimiter switch

 OUT
	boolean
 ***********************************************************************/

bool parse_cmd_block(prsdata *pdata, prseng_t *ppe, prsnode *pnode,
													bool chk_delim) {
	bool	 loop = true;
	prscell	*pcell;

	/*
		start of block sequence does not exists or has already
		been parsed so don't look for it.
	*/

	while (loop == true) {
		/* skip useless text */
		if (prs_skip_useless(ppe) == false) {
			return false;
		}

#ifdef PRS_DEBUG
		debugf("parse_cmd_block() : checking character.");
#endif
		switch(prseng_get_char(ppe)) {
			case PMK_CHAR_COMMAND_END :
#ifdef PRS_DEBUG
				debugf("parse_cmd_block() : found end of command character.");
#endif

				if (prseng_next_char(ppe) == false) {
					return false;
				}

				if (chk_delim == true) {
					/* delimiter found, exit from the loop */
					loop = false;
				} else {
					/* delimiter not expected */
					return false;
				}
				break;

			case CHAR_EOS :
#ifdef PRS_DEBUG
				debugf("parse_cmd_block() : found end of string character.");
#endif
				if (chk_delim == true) {
					/* expected delimiter not found */
					return false;
				} else {
					/* main loop, no delimiter was expected => ok */
					return true;
				}
				break;

			default :
#ifdef PRS_DEBUG
				debugf("parse_cmd_block() : parsing command header.");
#endif
				pcell = parse_cmd_header(ppe, pnode);
				if (pcell == NULL) {
					/* parsing failed, message done */
					return false;
				}

#ifdef PRS_DEBUG
				debugf("parse_cmd_block() : parsing command body.");
#endif
				if (parse_opt_block(pdata, ppe, pcell, true) == false) {
#ifdef PRS_DEBUG
					debugf("parse_cmd_block() : command boby parsing has fail.");
#endif
					return false;
				}
		}
	}

	return true;
}


/*******************
 * parse_pmkfile() *
 ***********************************************************************
 DESCR
	parse pmkfile

 IN
	fp:		file descriptor
	pdata:	parsing data structure
	kwtab:	keyword table
	size:	size of keyword table

 OUT
	boolean
 ***********************************************************************/

bool parse_pmkfile(FILE *fp, prsdata *pdata, prskw kwtab[], size_t size) {
	bool		 rslt = true;
	miscdata_t	 md;
	prseng_t	*ppe;

	ppe = prseng_init(fp, &md);
	if (ppe == NULL) {
		/* cannot init engine structure */
		errorf("init of parsing engine structure failed.");
		return false;
	}

	/* init hash table of command keywords */
	md.phtkw = keyword_hash(kwtab, size);
	if (md.phtkw == NULL) {
		/* message done */
		rslt = false;
	}

	if ((rslt == true) && (parse_cmd_block(pdata, ppe, pdata->tree,
													false) == false)) {
		/* display error message */
		errorf("line %d : %s", ppe->linenum, parse_err);
		rslt = false;
	}

	if ((rslt == true) && (feof(fp) == 0)){
		/* error occured before EOF */
		errorf("end of file not reached.");
		rslt = false;
	}

	hash_destroy(md.phtkw);
	prseng_destroy(ppe);
	return(rslt);
}


/*****************
 * process_opt() *
 ***********************************************************************
 DESCR
	process option line of configuration file

 IN
	pht:	storage hash table
	popt:	option structure to record

 OUT
	boolean
 ***********************************************************************/

bool process_opt(htable_t *pht, prsopt *popt) {
	if ((popt->opchar != CHAR_COMMENT) && (popt->opchar != CHAR_EOS)) {
		/* add options that are not comment neither blank lines */
		if (hash_update_dup(pht, popt->key,
							po_get_str(popt->value)) == false) {
			errorf("hash failure.");
			return false;
		}
	}
	return true;
}


/*******************
 * parse_pmkconf() *
 ***********************************************************************
 DESCR
	parse configuration file

 IN
	fp:		file to parse
	pht:	data used by processing function
	seplst:	list of separators
	func:	processing function

 OUT
	boolean
 ***********************************************************************/

bool parse_pmkconf(FILE *fp, htable_t *pht, char *seplst,
									bool (*func)(htable_t *, prsopt *)) {
	bool		 loop;
	char		*pbuf,
				 c,
				 buf[MAX_LINE_LEN];
	prseng_t	*ppe;
	prsopt		 opt;
	size_t		 s;

	ppe = prseng_init(fp, NULL);
	if (ppe == NULL) {
		/* cannot init engine structure */
		errorf("init of parsing engine structure failed.");
		return false;
	}

	while (prseng_eof(ppe) == false) {
		switch (prseng_get_char(ppe)) {
			case CHAR_COMMENT :
				/* comment */
				strlcpy(opt.key, "comment", sizeof(opt.key)); /* no check */
				opt.opchar = CHAR_COMMENT;

				/* check for end of line or end of file */
				pbuf = buf;
				s = sizeof(buf);
				loop = true;
				while (loop == true) {
#ifdef PRS_DEBUG
					debugf("check if '%c' == CR.", prseng_get_char(ppe));
#endif

					c = prseng_get_char(ppe);

					/* reached end of line or buffer ? */
					if ((c == CHAR_CR) || (c == CHAR_EOS)) {
						loop = false;
						c = CHAR_EOS;
					}

					*pbuf = c;
					pbuf++;
					s--;

					/* check remaining space */
					if (s < 0) {
						strlcpy(parse_err, PRS_ERR_OVERFLOW,
												sizeof(parse_err)); /* no check */
						prseng_destroy(ppe);
						return false;
					}

					/* next char */
					if (prseng_next_char(ppe) == false) {
						prseng_destroy(ppe);
						return false;
					}
				}

#ifdef PRS_DEBUG
				debugf("parse_pmkconf() : found comment '%.32s'.", buf);
#endif
				/* copy comment */
				opt.value = po_mk_str(buf);

#ifdef PRS_DEBUG
				debugf("parse_pmkconf() : cursor after comment '%.32s'.", ppe->cur);
#endif

				/* process option */
				if (func(pht, &opt) == false) {
					errorf("line %d : processing failed", ppe->linenum);
					return false;
				}
				break;

			case CHAR_CR :
				/* empty line */
				strlcpy(opt.key, "", sizeof(opt.key)); /* no check */
				opt.opchar = CHAR_EOS;
				opt.value = po_mk_str("");
#ifdef PRS_DEBUG
				debugf("parse_pmkconf() : found empty line.");
#endif

				/* next char */
				if (prseng_next_char(ppe) == false) {
					prseng_destroy(ppe);
					return false;
				}

				/* process option */
				if (func(pht, &opt) == false) {
					errorf("line %d : processing failed", ppe->linenum);
					return false;
				}
				break;

			case CHAR_EOS :
				/* end of file reached */
				loop = false;
				break;

			default :
				if (parse_opt(ppe, &opt, seplst) == true) {
#ifdef PRS_DEBUG
					strlcpy(buf, po_get_str(opt.value), sizeof(buf)); /* no check */
					debugf("parse_pmkconf(): opt line to parse = '%.32s'.", buf);
#endif
					/* parse ok */
					if (func(pht, &opt) == false) {
						errorf("line %d : processing failed", ppe->linenum);
						return false;
					}
				} else {
#ifdef PRS_DEBUG
					debugf("parse_pmkconf(): incorrect line = '%.32s'.", ppe->cur);
#endif
					/* incorrect line */
					errorf("line %d : %s", ppe->linenum, parse_err);
					return false;
				}

				/* skip end of line */
				if (prseng_test_char(ppe, CHAR_CR) == true) {
					/* next char */
					if (prseng_next_char(ppe) == false) {
						prseng_destroy(ppe);
						return false;
					}
				}

#ifdef PRS_DEBUG
				debugf("parse_pmkconf(): line after opt parse = '%.32s'.", ppe->cur);
#endif

				break;
		}
	}

	if (prseng_eof(ppe) == false) {
		/* error occured before EOF */
		errorf("line %d : %.32s", ppe->linenum, "end of file not reached.");
		return false;
	}

	return true;
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
