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


#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_string.h"

#include "common.h"
#include "hash.h"
#include "parse.h"


/*#define DEBUG_PRS	1*/

char	parse_err[MAX_ERR_MSG_LEN];


/*
	initialize parsing data structure

	return : prsdata structure
*/

prsdata *prsdata_init(void) {
	prsdata	*pdata;

	pdata = (prsdata *) malloc(sizeof(prsdata));
	if (pdata != NULL) {
		pdata->linenum = 0;
		pdata->tree = prsnode_init();
	}

	return(pdata);
}

/*
	free space allocated by parsing data structure

	pdata : structure to clean
*/

void prsdata_destroy(prsdata *pdata) {
	prsnode_destroy(pdata->tree);
	free(pdata);
}

/*
	initialize parsing engine structure

	return : prseng structure
*/

prseng *prseng_init(void) {
	prseng	*peng;

	peng = (prseng *) malloc(sizeof(prseng));
	if (peng != NULL) {
		peng->fp = NULL;
		peng->eof = false;
		peng->prscur = NULL;
		peng->phtkw = NULL;
		peng->linenum = 1;
		peng->offset = 0;
		peng->curnode = NULL;
	}

	return(peng);
}

/*
	free space allocated by parsing engine structure

	pdata : structure to clean
*/

void prseng_destroy(prseng *peng) {
	/* destroy keyword hash table */
	hash_destroy(peng->phtkw);
	/* free structure */
	free(peng);
}

/*
	initialize parsing node structure

	return : parsing node structure
*/

prsnode *prsnode_init(void) {
	prsnode	*pnode;

	pnode = (prsnode *) malloc(sizeof(prsnode));
	if (pnode != NULL) {
		pnode->first = NULL;
		pnode->last = NULL;
	}

	return(pnode);
}

/*
	add a cell to a node

	pnode : target node
	pcell : cell to add

	return : -
*/

void prsnode_add(prsnode *pnode, prscell *pcell) {
	if ((pnode != NULL) && (pcell != NULL)) {
		if (pnode->last != NULL) {
			/* linking to last item of node */
			pnode->last->next = pcell;
		} else {
			/* empty node, link as first */
			pnode->first = pcell;
		}
		pnode->last = pcell;
	}
}


/*
	free space allocated by parsing node structure

	pnode : node structure to clean
*/

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

/*
	initialize parsing cell structure

	return : parsing cell structure or NULL if failed
*/

prscell *prscell_init(int token, int type, int subtoken) {
	bool	 rval = false;
	htable	*pht;
	prscell	*pcell;
	prsnode	*pnode;
	prsopt	*popt;

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
				pht = hash_init_adv(MAX_CMD_OPT, (void *(*)(void *))po_dup, (void (*)(void *))po_free,
					(void *(*)(void *, void *, void *))po_append);
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

/*
	free space allocated for parsing cell structure

	pcell : structure to clean
*/

void prscell_destroy(prscell *pcell) {
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

/*
	init a prsopt structure
	
	return : prsopt structure
*/

prsopt *prsopt_init(void) {
	prsopt	*ppo;

	ppo = (prsopt *) malloc(sizeof(prsopt));

	return(ppo);
}

/*
	init a prsopt structure with given aguments
	
	return : prsopt structure
*/

prsopt *prsopt_init_adv(char *key, char opchar, char *value) {
	prsopt	*ppo;

	ppo = (prsopt *) malloc(sizeof(prsopt));
	if (ppo == NULL) {
		return(NULL);
	}

	if (strlcpy(ppo->key, key, sizeof(ppo->key)) >= sizeof(ppo->key)) {
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

/*
	destroy a prsopt structure
	
	ppo : prsopt structure to free
*/

void prsopt_destroy(prsopt *ppo) {
	if (ppo->value != NULL) {
		po_free(ppo->value);
	}
	free(ppo);
}

/*
	get a line from a file and pre-process it

	fp : file pointer
	line : buffer that will contain the line
	lsize : size of the buffer

	returns a boolean :
		- true when a line is available
		- false when error or eof occured
*/

bool prs_get_line(FILE *fp, char *line, size_t lsize) {
	char	*buf,
		*p,
		*rp;

	buf = (char *) malloc(lsize);
	if (buf == NULL)
		return(false);

	if (fgets(buf, lsize, fp) != NULL) {
		p = buf;

		/* skip blank characters */
		while (isblank(*p) != 0) {
			p++;
		}
		rp = p; /* save start of data */

		while (*p != CHAR_EOS) {
			if (*p == '\n') {
				/* remove trailing newline */
				*p= CHAR_EOS;
			}
			p++;
		}

		strlcpy(line, rp, lsize); /* no check needed */
		free(buf);

		return(true);
	} else {
		/* reached eof or error */
		return(false);
	}
}

/*
        fill parsing engine's buffer

        XXX

        return:
                - true: buffer filled
                - false: end of file or error
*/

bool prs_fill_buf(prseng *peng) {
	size_t	 s;

	if (peng->prscur == NULL) {
#ifdef DEBUG_PRS
		debugf("prs_fill_buf(): init offset.");
#endif
		/* initialise offset */
		peng->offset = 0;
	} else {
#ifdef DEBUG_PRS
		/*debugf("prs_fill_buf(): scurs = '%s'.", scurs);*/
#endif
		/* compute offset */
		peng->offset = peng->prscur - peng->prsbuf + peng->offset;
	}

#ifdef DEBUG_PRS
		debugf("prs_fill_buf(): offset = %d.", peng->offset);
#endif

	/* set the reading pointer at desired offset */
	fseek(peng->fp, peng->offset, SEEK_SET);

	/* fill buffer with size - 1 characters */
	s = fread((void *) peng->prsbuf, sizeof(char), (sizeof(peng->prsbuf ) - 1), peng->fp);
#ifdef DEBUG_PRS
	debugf("prs_fill_buf(): read %d chars", s);
#endif

	/* put EOS right after the last read char */
	peng->prsbuf[s] = CHAR_EOS;
#ifdef DEBUG_PRS
	debugf("prs_fill_buf(): set prsbuf[%d] to End Of String", s);
#endif

	/* rewind cursor to the beginning of the buffer */
	peng->prscur = (char *)peng->prsbuf;

	/* ferror ? */
	if (ferror(peng->fp) != 0) {
#ifdef DEBUG_PRS
		debugf("prs_fill_buf(): ferror XXX");
#endif
		return(false);
	}

	if (feof(peng->fp) != 0) {
		peng->eof = true;
#ifdef DEBUG_PRS
		debugf("prs_fill_buf(): set end of file flag");
#endif
	}

#ifdef DEBUG_PRS
	/*debugf("prs_fill_buf(): return = '%s'", peng->prscur);*/
#endif
	/* buffer filled, return pointer to the first character */
	return(true);
}

/*
	create hash table with keyword structures

	kwtab : keyword table
	nbkw : size of table

	return : hash table
*/

htable *keyword_hash(prskw kwtab[], int nbkw) {
	htable	*phkw;
	int	 i;
	prskw	*pkw;

	phkw = hash_init_adv(nbkw, (void *(*)(void *))strdup, free, NULL);
	if (phkw != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < nbkw ; i++) {
			pkw = (prskw *) malloc(sizeof(prskw));
			memmove(pkw, &kwtab[i], sizeof(prskw));
			if (hash_update(phkw, kwtab[i].kw, pkw) == HASH_ADD_FAIL) { /* no need to strdup */
				free(pkw);
				errorf("hash failure");
				exit(EXIT_FAILURE);
			}
		}
	}

	return(phkw);
}

/*
	skip blank character(s)

	pstr : current parsing cursor

	return : new parsing cursor
*/

char *skip_blank(char *pstr) {
	while (isblank(*pstr) != 0) {
		pstr++;
	}
	return(pstr);
}

/*
	skip useless text (blanks, comments, empty lines)

	pstr: starting pointer

	return: start of non useless text
*/

void skip_useless(prseng *peng) {
	bool	 loop = true;

#ifdef DEBUG_PRS
	debugf("skip_useless() : process start on line %d", peng->linenum);
#endif
	/*pstr = peng->prscur;*/
	while (loop == true) {
		/* skip blanck characters */
		while (isblank(*peng->prscur) != 0) {
			peng->prscur++;
		}

		switch (*peng->prscur) {
			case PMK_CHAR_COMMENT:
#ifdef DEBUG_PRS
				debugf("skip_useless() : skip comment on line %d.", peng->linenum);
				/*debugf("skip_useless() : comment : '%s'.", peng->prscur);*/
#endif
				/* comment is one line length,
				 * look for the next end of line */
				peng->prscur = strchr(peng->prscur, CHAR_CR);
				/* don't break to use following
				 * incrementation to skip end of line */

			case CHAR_CR:
#ifdef DEBUG_PRS
				debugf("skip_useless() : found new line at %d.", peng->linenum);
#endif
				peng->prscur++;
				peng->linenum++;
				break;

			default:
				/* found non useless char */
				loop = false;
		}
	}

	/*peng->prscur = pstr;*/
#ifdef DEBUG_PRS
	debugf("skip_useless() : process ended on line %d", peng->linenum);
#endif
}

/*
	get identifier 

	pstr : current parsing cursor
	pbuf : storage buffer
	size : size of buffer

	return : new parsing cursor or NULL
*/

char *parse_identifier(char *pstr, char *pbuf, size_t size) {
	while (((isalnum(*pstr) != 0) || (*pstr == '_')) && (size > 0)) {
		*pbuf = *pstr;
		pbuf++;
		pstr++;
		size--;
	}

	if (size == 0)
		return(NULL);

	*pbuf = CHAR_EOS;
	
	return(pstr);
}

/*
	get label

	pstr : current parsing cursor
	pbuf : storage buffer
	size : size of buffer

	return : new parsing cursor or NULL
*/

char *parse_label(char *pstr, char *pbuf, size_t size) {
	if (*pstr == '!') {
		*pbuf = *pstr;
		pbuf++;
		pstr++;
		size--;
	}

	return(parse_identifier(pstr, pbuf, size));
}

/*
	get bool value

	pstr : current parsing cursor
	po : storage pmk object
	size : size of buffer

	return : new parsing cursor or NULL
*/

char *parse_bool(char *pstr, pmkobj *po, size_t size) {
	bool	*pb;
	char	 buffer[6],
		*pbuf;
	size_t	 bsize;

	pb = (bool *) malloc(sizeof (bool));
	if (pb == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err));
		return(NULL);
	}

	bsize = sizeof(buffer);
	if (bsize < size)
		size = bsize;

	pbuf = buffer;

	while ((isalnum(*pstr) != 0) && (size > 0)) {
		*pbuf = *pstr;
		pbuf++;
		pstr++;
		size--;
	}

	if (size == 0) {
		free(pb);
		return(NULL);
	}

	*pbuf = CHAR_EOS;

	if (strncmp(buffer, PMK_BOOL_TRUE, sizeof(buffer)) == 0) {
		*pb = true;
	} else {
		if (strncmp(buffer, PMK_BOOL_FALSE, sizeof(buffer)) == 0) {
			*pb = false;
		} else {
			free(pb);
			return(NULL);
		}
	}

	po->type = PO_BOOL;
	po->data = pb;

	return(pstr);
}

/*
	get quoted string content

	pstr : current parsing cursor
	po : storage pmk object
	size : size of buffer

	return : new parsing cursor
*/

char *parse_quoted(char *pstr, pmkobj *po, size_t size) {
	char	*buffer,
		*pbuf;

	buffer = (char *) malloc(size);
	if (buffer == NULL) {
		return(NULL);
	}

	pbuf = buffer;
	while ((*pstr != PMK_CHAR_QUOTE_END) && (*pstr != CHAR_CR) && (*pstr != CHAR_EOS)) {
		/* found escape character */
		if (*pstr == PMK_CHAR_ESCAPE) {
			pstr++;
			if ((*pstr == CHAR_CR) || (*pstr == CHAR_EOS)) {
				/* misplaced escape character */
				strlcpy(parse_err, "trailing escape character.", sizeof(parse_err));
				return(NULL);
			}

			if (*pstr == PMK_CHAR_QUOTE_END) {
				size--;
			} else {
				pstr--;
			}
		}

		if (size > 1) {
			*pbuf = *pstr;
			pstr++;
			pbuf++;
			size--;
		} else {
			free(buffer);
			strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err));
			return(NULL);
		}
	}

	if (*pstr == PMK_CHAR_QUOTE_END) {
		/* found end of quoted string */
		*pbuf = CHAR_EOS;
		po->type = PO_STRING;
		/* use strdup to gain memory (but lose some cpu ;) */
		po->data = strdup(buffer);
		free(buffer);
		pstr++;
		return(pstr);
	} else {
		/* end of quoting not found */
		free(buffer);
		strlcpy(parse_err, "ending quote is missing.", sizeof(parse_err));
		return(NULL);
	}
}

/*
	get list

	pstr : current parsing cursor
	po : storage pmk object
	size : size of buffer

	return : new parsing cursor
*/

char *parse_list(char *pstr, pmkobj *po, size_t size) {
	char	*buffer;
	dynary	*pda;
	pmkobj	 potmp;

	pda = da_init();

	while ((*pstr != PMK_CHAR_LIST_END) && (*pstr != CHAR_EOS)) {
		switch (*pstr) {
			case PMK_CHAR_QUOTE_START :
				pstr++;
				pstr = parse_quoted(pstr, &potmp, size);
				if (pstr == NULL) {
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
					return(NULL);
				}

#ifdef DEBUG_PRS
				debugf("add '%s' into dynary (quoted)", potmp.data);
#endif

				if (da_push(pda, potmp.data) == false) {
					da_destroy(pda);
					return(NULL);
				}

				switch (*pstr) {
					case PMK_CHAR_LIST_SEP :
					case PMK_CHAR_LIST_END :
					case CHAR_EOS :
						/* all ok */
						break;

					default :
						da_destroy(pda);
						strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
						return(NULL);
						break;
				}
				break;

			case PMK_CHAR_LIST_SEP :
				pstr++;
#ifdef DEBUG_PRS
				debugf("found separator");
#endif
				break;

			default :
				buffer = (char *) malloc(size);
				if (buffer == NULL) {
					strlcpy(parse_err, "arg", sizeof(parse_err));
					return(NULL);
				}

				pstr = parse_identifier(pstr, buffer, size);

				if (*buffer == CHAR_EOS) {
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
					return(NULL);
				}

				if (pstr == NULL) {
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err));
					return(NULL);
				}

#ifdef DEBUG_PRS
				debugf("add '%s' into dynary (simple).", buffer);
#endif

				if (da_push(pda, buffer) == false) {
					da_destroy(pda);
					return(NULL);
				}

				switch (*pstr) {
					case PMK_CHAR_LIST_SEP :
					case PMK_CHAR_LIST_END :
					case CHAR_EOS :
						/* all ok */
						break;

					default :
						da_destroy(pda);
						strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
						return(NULL);
						break;
				}
				break;
		}
	}

	if (*pstr == PMK_CHAR_LIST_END) {
		/* found end of list */
		po->type = PO_LIST;
		po->data = pda;
		pstr++;
		return(pstr);
	} else {
		/* end of list not found */
		da_destroy(pda);
		strlcpy(parse_err, "end of list not found.", sizeof(parse_err));
		return(NULL);
	}
}

/*
	parse key

	pstr : current parsing cursor
	po : storage pmk object
	size : size of buffer

	return : new parsing cursor or NULL
*/

char *parse_key(char *pstr, pmkobj *po, size_t size) {
	char	*buffer,
		*rptr;

	po->type = PO_NULL;

	switch (*pstr) {
		case PMK_CHAR_QUOTE_START :
			/* found a quoted string */
			pstr++;
			rptr = parse_quoted(pstr, po, size);
			break;

		default :
			/* identifier */
			buffer = (char *) malloc(size);
			if (buffer == NULL) {
				strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err));
				return(NULL);
			}

			pstr = parse_identifier(pstr, buffer, size);

			if (pstr == NULL) {
				free(buffer);
				strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err));
				return(NULL);
			}

			po->type = PO_STRING;
			po->data = buffer;

			rptr = pstr;
			break;
	}
	return(rptr);
}

/*
	parse data

	pstr : current parsing cursor
	po : storage pmk object
	size : size of buffer

	return : new parsing cursor or NULL
*/

char *parse_data(char *pstr, pmkobj *po, size_t size) {
	char	*rptr;

	po->type = PO_NULL;

	switch (*pstr) {
		/* found a quoted string */
		case PMK_CHAR_QUOTE_START :
			pstr++;
			rptr = parse_quoted(pstr, po, size);
			break;

		/* found a list */
		case PMK_CHAR_LIST_START :
			pstr++;
			rptr = parse_list(pstr, po, size);
			break;

		default :
			rptr = parse_bool(pstr, po, size);
			break;
	}
	return(rptr);
}

/*
        parse command header

        XXX
*/

prscell *parse_cmd_header(prseng *peng, prsnode *pnode) {
	char	 name[CMD_LEN],
		*pstr;
	prscell	*pcell;
	prskw	*pkw;

	pstr = parse_identifier(peng->prsbuf, name, sizeof(name));
	if (pstr == NULL) {
		strlcpy(parse_err, "command parsing failed.", sizeof(parse_err));
		return(NULL);
	}
#ifdef DEBUG_PRS
	debugf("parse_cmd_header(): cmd name = '%s'", name);
#endif

	/* check validity of the command */
	pkw = hash_get(peng->phtkw, name);
	if (pkw != NULL) {
		/* command ok, creating cell */
		pcell = prscell_init(pkw->token, pkw->type, pkw->subtoken);
		if (pcell == NULL) {
			strlcpy(parse_err, "pcell init failed.", sizeof(parse_err));
			return(NULL);
		}
#ifdef DEBUG_PRS
		debugf("parse_cmd_header(): cmd token = %d", pcell->token);
		debugf("parse_cmd_header(): cmd type = %d", pcell->type);
#endif
	} else {
		/* unknown command */
		strlcpy(parse_err, "unknown command.", sizeof(parse_err));
		return(NULL);
	}

	/* look for a label */
	if (*pstr == PMK_CHAR_LABEL_START) {
		/* label delimiter found */
		pstr++;
		/* get label name */
		pstr = parse_label(pstr, pcell->label, sizeof(pcell->label));
		if (pstr == NULL) {
			strlcpy(parse_err, "label parsing failed.", sizeof(parse_err));
			return(NULL);
		}
#ifdef DEBUG_PRS
		debugf("parse_cmd_header(): cmd label = '%s'", pcell->label);
#endif

		/* check label's ending delimiter */
		if (*pstr != PMK_CHAR_LABEL_END) {
			/* label name must be immediately followed by closing delimiter */
			strlcpy(parse_err, "label parsing failed.", sizeof(parse_err));
			return(NULL);
		} else {
			pstr ++;
		}
#ifdef DEBUG_PRS
		debugf("parse_cmd_header(): parsed command '%s' with label '%s'", name, pcell->label);
	} else {
		debugf("parse_cmd_header(): parsed command '%s' with no label", name);
#endif
	}


	/* look for command block starting sequence */

	if (*pstr != ' ') {
		strlcpy(parse_err, PRS_ERR_UNKNOWN, sizeof(parse_err));
		return(NULL);
	} else {
		pstr++;
	}

	if (*pstr != PMK_CHAR_COMMAND_START) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(NULL);
	} else {
		pstr++;
	}

	if (*pstr != CHAR_CR) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(NULL);
	}

	/* add cell in the node */
	prsnode_add(pnode, pcell); /* XXX check */
	
	peng->prscur = pstr;

	return(pcell);
}

/*
	parse an option line

	line : option line
	popt : storage structure 
	seplst : string that contain all separator characters

	return : boolean
*/

bool parse_opt(char *line, prsopt *popt, char *seplst) {
	char	*pstr;
	pmkobj	 po;

#ifdef DEBUG_PRS
	debugf("parse_opt() : line = '%s'", line);
#endif

	pstr = parse_key(line, &po, sizeof(popt->key));
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		strlcpy(popt->key, po.data, sizeof(popt->key));
		free(po.data);
	}

#ifdef DEBUG_PRS
	debugf("parse_opt() : key = '%s'", popt->key);
#endif

	pstr = skip_blank(pstr);

	/* check if character is in separator list */
	if (strchr(seplst, *pstr) == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		popt->opchar = *pstr;
		pstr++;
	}
#ifdef DEBUG_PRS
	debugf("parse_opt() : assign = '%c'", popt->opchar);
#endif

	pstr = skip_blank(pstr);

	popt->value = (pmkobj *) malloc(sizeof(pmkobj));
	if (popt->value == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err));
		return(false);
	}

	pstr = parse_data(pstr, popt->value, OPT_VALUE_LEN);
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}

#ifdef DEBUG_PRS
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

	if (*pstr != CHAR_EOS) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(false);
	}

	return(true);
}

bool parse_opt_new(prseng *peng, prsopt *popt, char *seplst) {
	char	*pstr;
	pmkobj	 po;

#ifdef DEBUG_PRS
	/*debugf("parse_opt_new() : line = '%s'", peng->prscur);*/
#endif

	pstr = parse_key(peng->prscur, &po, sizeof(popt->key));
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		strlcpy(popt->key, po.data, sizeof(popt->key));
		free(po.data);
	}

#ifdef DEBUG_PRS
	debugf("parse_opt_new() : key = '%s'", popt->key);
#endif

	pstr = skip_blank(pstr);

	/* check if character is in separator list */
	if (strchr(seplst, *pstr) == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		popt->opchar = *pstr;
		pstr++;
	}
#ifdef DEBUG_PRS
	debugf("parse_opt_new() : assign = '%c'", popt->opchar);
#endif

	pstr = skip_blank(pstr);

	popt->value = (pmkobj *) malloc(sizeof(pmkobj));
	if (popt->value == NULL) {
		strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err));
		return(false);
	}

	pstr = parse_data(pstr, popt->value, OPT_VALUE_LEN);
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}

#ifdef DEBUG_PRS
	switch (popt->value->type) {
		case PO_BOOL :
			debugf("parse_opt_new() : value = *BOOL*");
			break;
		case PO_STRING :
			debugf("parse_opt_new() : value = '%s'", popt->value->data);
			break;
		case PO_LIST :
			debugf("parse_opt_new() : value = (*LIST*)");
			break;
		default :
			debugf("parse_opt_new() : value = !unknown type!");
			break;
	}
#endif

	/*if (*pstr != CHAR_EOS) {                                        */
	/*        strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));*/
	/*        return(false);                                          */
	/*}                                                               */

	/* update cursor */
	peng->prscur = pstr;

	return(true);
}

/*
	parse a command line option

	line : option line
	popt : storage structure 
	seplst : string that contain all separator characters

	return : boolean
*/

bool parse_clopt(char *line, prsopt *popt, char *seplst) {
	char	*pstr;
	pmkobj	 po;

#ifdef DEBUG_PRS
	debugf("line = '%s'", line);
#endif

	pstr = parse_key(line, &po, sizeof(popt->key));
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		strlcpy(popt->key, po.data, sizeof(popt->key));
		free(po.data);
	}

#ifdef DEBUG_PRS
	debugf("key = '%s'", popt->key);
#endif

	/* check if character is in separator list */
	if (strchr(seplst, *pstr) == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		popt->opchar = *pstr;
		pstr++;
	}
#ifdef DEBUG_PRS
	debugf("assign = '%c'", popt->opchar);
#endif


	popt->value = po_mk_str(pstr);
#ifdef DEBUG_PRS
	debugf("value = '%s'", popt->value);
#endif

	return(true);
}

/*
        parse a block of options

        XXX
*/

bool parse_opt_block(prsdata *pdata, prseng *peng, prscell *pcell, bool chk_delim) {
	bool	 loop = true;
	prscell	*ncell;
	prsnode	*pnode;
	prsopt	 opt,
		*nopt;

	/*pstr = peng->prscur; |+ XXX should take beginning of buffer ? useless ? +|*/

	pnode = pcell->data;

	if ((pcell->type == PRS_KW_NODE) && (pnode->token == PRS_TOK_NULL)) {
		/* found a block node (like for example IF command) => parse the block */
		return(parse_cmd_block(pdata, peng, pnode, false));
	} else {
		while (loop == true) {
			/* skip useless text */
			skip_useless(peng);
			/* update buffer window */
			prs_fill_buf(peng); /* XXX check */

			switch(*peng->prscur) {
				case PMK_CHAR_COMMAND_END :
#ifdef DEBUG_PRS
					debugf("parse_opt_block() : found end of block character.");
#endif
					peng->prscur++;

					if (chk_delim == true) {
						/* XXX should return(false) instead ?*/
						loop = false;
					} else {
						/* XXX should not found that ? */
						return(false);
					}
					break;

				case CHAR_EOS :
					if (chk_delim == true) {
						/* delimiter not found */
						return(false);
					} else {
						/* main loop, no delimiter */
						return(true);
					}
					break;

				default :
#ifdef DEBUG_PRS
					debugf("parse_opt_block() : calling parse_opt_new()");
#endif
					/* XXX XXX parse_opt does not update frame pointer */
					if (parse_opt_new(peng, &opt, PRS_PMKFILE_SEP) == false) {
						errorf("line %d : %s", pdata->linenum, parse_err);
#ifdef DEBUG_PRS
						debugf("parse_opt_block() : parse_opt_new() returned false");
#endif
						prscell_destroy(pcell);
						return(false);
					} else {
#ifdef DEBUG_PRS
						debugf("parse_opt_block() : recording '%s' key", opt.key);
#endif
						switch (pcell->type) {
							case PRS_KW_NODE :
								/* process a node */
								/* init item's pcell */
								ncell = prscell_init(pnode->token,
									PRS_KW_ITEM, PRS_TOK_NULL);
								if (ncell == NULL) {
									errorf("parse_opt_block() : prscell init failed");
									return(false);
								}

								nopt = ncell->data;

								/* duplicate opt content in item */
								strlcpy(nopt->key, opt.key, sizeof(opt.key));
								nopt->value = opt.value;

								/* add item in cell node */
								prsnode_add(pnode, ncell);
								break;

							case PRS_KW_CELL :
								/* process a command */
								if (hash_update(pcell->data, opt.key, opt.value) == HASH_ADD_FAIL) { /* no need to strdup */
									strlcpy(parse_err, PRS_ERR_HASH, sizeof(parse_err));
									return(false);
								}
								break;
						}

					}
					break;
			}
		}
	}

	return(true);
}

/*
        parse a block of commands

        XXX
*/

bool parse_cmd_block(prsdata *pdata, prseng *peng, prsnode *pnode, bool chk_delim) {
	bool	 loop = true;
	prscell	*pcell;

	/*pstr = peng->prscur; |+ XXX should take beginning of buffer ? useless ? +|*/

	if (chk_delim == true) {
#ifdef DEBUG_PRS
		debugf("parse_cmd_block() : checking beginning of the block sequence.");
#endif
		/* check the start sequence of the command block */
		if (*peng->prscur != ' ') {
			strlcpy(parse_err, PRS_ERR_UNKNOWN, sizeof(parse_err));
			return(false);
		} else {
			peng->prscur++;
		}

		if (*peng->prscur != PMK_CHAR_COMMAND_START) {
			strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
			return(false);
		} else {
			peng->prscur++;
		}

		if (*peng->prscur != CHAR_CR) {
			strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
			return(false);
		} else {
			peng->prscur++;
		}
#ifdef DEBUG_PRS
		debugf("parse_cmd_block() : found beginning of the block sequence.");
#endif
	}

	/*peng->prscur = pstr;*/

	while (loop == true) {
		/* skip useless text */
		skip_useless(peng);
		/* update buffer window */
		prs_fill_buf(peng);

#ifdef DEBUG_PRS
		debugf("parse_cmd_block() : checking character.");
#endif
		switch(*peng->prscur) {
			case PMK_CHAR_COMMAND_END :
#ifdef DEBUG_PRS
				debugf("parse_cmd_block() : found end of command character.");
#endif

				peng->prscur++;

				if (chk_delim == true) {
					/* XXX should return(false) instead ?*/
					loop = false;
				} else {
					/* XXX should not found that ? */
					return(false);
				}
				break;

			case CHAR_EOS :
#ifdef DEBUG_PRS
				debugf("parse_cmd_block() : found end of string character.");
#endif
				if (chk_delim == true) {
					/* delimiter not found */
					return(false);
				} else {
					/* main loop, no delimiter */
					return(true);
				}
				break;

			default :
				if (chk_delim == true) {
					/* return() XXX TODO not NULL */
				}

#ifdef DEBUG_PRS
				debugf("parse_cmd_block() : parsing command header.");
#endif
				pcell = parse_cmd_header(peng, pnode);
				if (pcell == NULL) {
					/* XXX error msg ? */
					return(false);
				}

#ifdef DEBUG_PRS
				debugf("parse_cmd_block() : parsing command boby.");
#endif
				parse_opt_block(pdata, peng, pcell, true);

		}

	}

	return(true);
}

/*
	parse pmkfile

	fd : file descriptor
	pdata : parsing data structure
	kwtab : keyword table
	size : size of keyword table

	return : boolean
*/

bool parse_pmkfile(FILE *fp, prsdata *pdata, prskw kwtab[], size_t size) {
	prseng	*peng;

	peng = prseng_init();
	if (peng == NULL) {
		/* cannot init engine structure */
		errorf("init of parsing engine structure failed.");
		return(false);
	} else {
		/* set file pointer */
		peng->fp = fp;
		/* init hash table of command keywords */
		peng->phtkw = keyword_hash(kwtab, size);
		/*|+ set current node +|      */
		/*peng->curnode = pdata->tree;*/
	}

	/* update buffer window */
	prs_fill_buf(peng); /* XXX check */

	/*if (parse_line(fp, pdata, phkw, tnode) != true) {*/
	if (parse_cmd_block(pdata, peng, pdata->tree, false) != true) {
		/* display error message */
		errorf("line %d : %s", peng->linenum, parse_err);
		return(false);
	}

	if (feof(fp) == 0) {
		/* error occured before EOF */
		errorf("end of file not reached.");
		return(false);
	}

	return(true);
}

/*
	process option line of configuration file

	pht : storage hash table
	popt : option structure to record

	return : boolean
*/

bool process_opt(htable *pht, prsopt *popt) {
	if ((popt->opchar != CHAR_COMMENT) && (popt->opchar != CHAR_EOS)) {
		/* add options that are not comment neither blank lines */
		if (hash_update_dup(pht, popt->key, po_get_str(popt->value)) == HASH_ADD_FAIL) {
			errorf("hash failure.");
			return(false);
		}
	}
	return(true);
}

/*
	parse configuration file

	fp : file to parse 
	pht : data used by processing function
	seplst : list of separators
	func : processing function

	return : boolean
*/

bool parse_pmkconf(FILE *fp, htable *pht, char *seplst, bool (*func)(htable *, prsopt *)) {
	char	 buf[MAX_LINE_LEN];
	int	 ln = 0;
	prsopt	 opt;

	while (get_line(fp, buf, sizeof(buf)) == true) {
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* comment */
				strlcpy(opt.key, "comment", sizeof(opt.key)); /* don't check */
				opt.opchar = CHAR_COMMENT;
				opt.value = po_mk_str(buf);
				func(pht, &opt);
				break;

			case CHAR_EOS :
				/* empty line */
				strlcpy(opt.key, "", sizeof(opt.key)); /* don't check */
				opt.opchar = CHAR_EOS;
				opt.value = po_mk_str(buf);
				func(pht, &opt);
				break;

			default :
				if (parse_opt(buf, &opt, seplst) == true) {
					/* parse ok */
					if (func(pht, &opt) == false) {
						errorf("line %d : processing failed", ln);
						return(false);
					}
				} else {
					/* incorrect line */
					errorf("line %d : %s", ln, parse_err);
					return(false);
				}
				break;
		}
		ln++; /* increment line number */
	}

	if (feof(fp) == 0) {
		/* error occured before EOF */
		errorf("line %d : %s", ln, "end of file not reached.");
		return(false);
	}

	return(true);
}

