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


#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_string.h"

#include "common.h"
#include "hash.h"
#include "parse.h"
#include "pmk.h"


/*
#define DEBUG_PRS	1
*/

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
		pdata->tree = NULL;
	}

	return(pdata);
}

/*
	free space allocated by parsing structure

	pdata : structure to clean
*/

void prsdata_destroy(prsdata *pdata) {
	prsnode_destroy(pdata->tree);
	free(pdata);
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
		free(pcell);
	}
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
			bcopy(&kwtab[i], pkw, sizeof(prskw));
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
	while ((*pstr != PMK_CHAR_QUOTE_END) && (*pstr != CHAR_EOS)) {
		/* found escape character */
		if (*pstr == PMK_CHAR_ESCAPE) {
			pstr++;
			if (*pstr == CHAR_EOS) {
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
		/* XXX use strdup to gain memory ? */
		po->type = PO_STRING;
		po->data = buffer;
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
	parse a command

	line : line to parse
	phkw : keyword hash table 

	return : parsing cell structure
*/

prscell *parse_cell(char *line, htable *phkw) {
	char	 name[CMD_LEN],
		*pstr;
	prscell	*pcell;
	prskw	*pkw;

	pstr = parse_identifier(line, name, sizeof(name));
	if (pstr == NULL) {
		strlcpy(parse_err, "command parsing failed.", sizeof(parse_err));
		return(NULL);
	}
#ifdef DEBUG_PRS
	debugf("command = '%s'", name);
#endif

	/* set token */
	pkw = hash_get(phkw, name);
	if (pkw != NULL) {
		pcell = prscell_init(pkw->token, pkw->type, pkw->subtoken);
		if (pcell == NULL) {
			strlcpy(parse_err, "pcell init failed.", sizeof(parse_err));
			return(NULL);
		}
#ifdef DEBUG_PRS
		debugf("token = %d", pcell->token);
#endif
	} else {
		strlcpy(parse_err, "unknown command.", sizeof(parse_err));
		return(NULL);
	}

	if (*pstr == PMK_CHAR_LABEL_START) {
		pstr++;
		pstr = parse_label(pstr, pcell->label, sizeof(pcell->label));
		if (pstr == NULL) {
			strlcpy(parse_err, "label parsing failed.", sizeof(parse_err));
			return(NULL);
		}
#ifdef DEBUG_PRS
		debugf("label = '%s'", pcell->label);
#endif

		if (*pstr != PMK_CHAR_LABEL_END) {
			/* label name must be immediately followed by closing delimiter */
			strlcpy(parse_err, "label parsing failed.", sizeof(parse_err));
			return(NULL);
		} else {
			pstr ++;
		}
#ifdef DEBUG_PRS
	} else {
		strlcpy(pcell->label, "", sizeof(pcell->label));
#endif
	}

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

	if (*pstr != CHAR_EOS) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(NULL);
	}

#ifdef DEBUG_PRS
	debugf("command '%s' with label '%s'", name, pcell->label);
#endif
	/* everything is ok */
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

	pstr = skip_blank(pstr);

#ifdef DEBUG_PRS
	debugf("assign = '%c'", *pstr);
#endif

	/* check if character is in separator list */
	if (strchr(seplst, *pstr) == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		popt->opchar = *pstr;
		pstr++;
	}

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
			debugf("value = *BOOL*");
			break;
		case PO_STRING :
			debugf("value = '%s'", popt->value->data);
			break;
		case PO_LIST :
			debugf("value = (*LIST*)");
			break;
		default :
			debugf("value = !unknown type!");
			break;
	}
#endif

	if (*pstr != CHAR_EOS) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(false);
	}

	return(true);
}

/*
	parse node

	fp : file structure
	pdata : parsing data structure
	phkw : keyword hash table
	pcell : parsing cell structure

	return : boolean
*/

bool parse_node(FILE *fp, prsdata *pdata, htable *phkw, prscell *pcell) {
	bool	 loop = true;
	char	 buf[MAX_LINE_LEN],
		*pbuf;
	prscell	*ncell;
	prsnode *pnode;
	prsopt	 opt,
		*nopt;

	pnode = pcell->data;

	if (pnode->token == PRS_TOK_NULL) {
		/* XXX BLAH BLAH */
		return(parse_line(fp, pdata, phkw, pnode));
	} else {
		while ((loop == true) && (prs_get_line(fp, buf, sizeof(buf)) == true)) {
			pbuf = buf;

			/* update current line number */
			pdata->linenum++;

			/* check first character */
			switch (*pbuf) {
				case CHAR_COMMENT :
				case CHAR_EOS :
					/* empty line */
					break;

				case PMK_CHAR_COMMAND_END :
					loop = false;
					break;

				default :
					if (parse_opt(pbuf, &opt, PRS_PMKFILE_SEP) == false) {
						errorf("line %d : %s", pdata->linenum, parse_err);
#ifdef DEBUG_PRS
						debugf("parse_opt returned false");
#endif
						prscell_destroy(pcell);
						return(false);
					} else {
#ifdef DEBUG_PRS
						debugf("recording '%s' key", opt.key);
#endif
						/* init item's pcell */
						ncell = prscell_init(pnode->token,
								PRS_KW_ITEM, PRS_TOK_NULL);
						if (ncell == NULL) {
							errorf("prscell init failed");
							return(false);
						}

						nopt = ncell->data;

						/* duplicate opt content in item */
						strlcpy(nopt->key, opt.key, sizeof(opt.key));
						nopt->value = opt.value;

						/* add item in cell node */
						prsnode_add(pnode, ncell);
					}
					break;
			}
		}
	}
	return(true);
}

/*
	parse command

	fp : file structure
	pdata : parsing data structure
	pcell : parsing cell structure 

	return : boolean
*/

bool parse_command(FILE *fp, prsdata *pdata, prscell *pcell) {
	bool	 loop = true;
	char	 buf[MAX_LINE_LEN],
		*pbuf;
	prsopt	 opt;

	while ((loop == true) && (prs_get_line(fp, buf, sizeof(buf)) == true)) {
		pbuf = buf;

		/* update current line number */
		pdata->linenum++;

		/* check first character */
		switch (*pbuf) {
			case CHAR_COMMENT :
			case CHAR_EOS :
				/* empty line */
				break;

			case PMK_CHAR_COMMAND_END :
				loop = false;
				break;

			default :
				if (parse_opt(pbuf, &opt, PRS_PMKFILE_SEP) == false) {
					errorf("line %d : %s", pdata->linenum, parse_err);
#ifdef DEBUG_PRS
					debugf("parse_opt returned false");
#endif
					prscell_destroy(pcell);
					return(false);
				} else {
#ifdef DEBUG_PRS
					debugf("recording '%s' key", opt.key);
#endif

					if (hash_update(pcell->data, opt.key, opt.value) == HASH_ADD_FAIL) { /* no need to strdup */
						strlcpy(parse_err, PRS_ERR_HASH, sizeof(parse_err));
						return(false);
					}
				}
				break;
		}
	}
	return(true);
}

/*
	parse line 

	fp : file structure
	pdata : parsing data structure
	phkw : keyword hash table
	pnode : parent node 

	return : boolean
*/

bool parse_line(FILE *fp, prsdata *pdata, htable *phkw, prsnode *pnode) {
	bool	 loop = true;
	char	 buf[MAX_LINE_LEN],
		*pbuf;
	prscell	*pcell = NULL;

	while ((loop == true) && (prs_get_line(fp, buf, sizeof(buf)) == true)) {

		/* update current line number */
		pdata->linenum++;

		pbuf = buf;

		switch(*pbuf) {
			case PMK_CHAR_COMMAND_END :
				loop = false;
				break;

		/* check first character, skip comment or empty line */
			case CHAR_COMMENT :
			case CHAR_EOS :
				break;

			default :
				/* parse command and label */
				pcell = parse_cell(pbuf, phkw);
				if (pcell == NULL) {
					errorf("line %d : %s", pdata->linenum, parse_err);
#ifdef DEBUG_PRS
					debugf("parse_cell returned NULL");
#endif
					return(false);
				}

#ifdef DEBUG_PRS
				debugf("type = %d", pcell->type);
#endif
			
				prsnode_add(pnode, pcell);

				switch (pcell->type) {
					case PRS_KW_NODE :
						parse_node(fp, pdata, phkw, pcell); /* XXX */
						break;
					case PRS_KW_CELL :
						parse_command(fp, pdata, pcell); /* XXX */
						break;
				}
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
	htable	*phkw;
	prsnode	*tnode;

	tnode = prsnode_init();
	pdata->tree = tnode;

	phkw = keyword_hash(kwtab, size);

	if (parse_line(fp, pdata, phkw, tnode) != true) {
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
		/* error occuered before EOF */
		errorf_line(PREMAKE_CONFIG_PATH, ln, "end of file not reached.");
		return(false);
	}

	return(true);
}

