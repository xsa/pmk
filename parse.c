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

/* for compatibility with 0.6, will be removed later */
/* #define PRS_OBSOLETE */

char	parse_err[MAX_ERRMSG_LEN];


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

	p = pnode->first;
	while (p != NULL) {
		n = p;
		p = n->next;
		prscell_destroy(n);
	}
	free(pnode);
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
				pht = hash_init_adv(MAX_CMD_OPT, (void (*)(void *))po_free,
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
	create hash table with keyword structures

	kwtab : keyword table
	nbkw : size of table

	return : hash table
*/

htable *keyword_hash(prskw kwtab[], int nbkw) {
	htable	*phkw;
	int	 i;
	prskw	*pkw;

	phkw = hash_init_adv(nbkw, free, NULL);
	if (phkw != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < nbkw ; i++) {
			pkw = (prskw *) malloc(sizeof(prskw));
			bcopy(&kwtab[i], pkw, sizeof(prskw));
			if (hash_add(phkw, kwtab[i].kw, pkw) == HASH_ADD_FAIL) {
				free(pkw);
				errorf("hash failure");
				exit(1);
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
	parse obsolete
	for pmk 0.6 compatibility
*/

char *parse_obsolete(char *pstr, char *pbuf, size_t size) {
	while ((*pstr != CHAR_EOS) && (*pstr != ' ') && (size > 0)) {
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
			size--;
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
#ifdef PRS_OBSOLETE
	char	*buffer,
		*rptr;
#else
	char	*rptr;
#endif

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
#ifdef PRS_OBSOLETE
			buffer = (char *) malloc(size);
			if (buffer == NULL) {
				strlcpy(parse_err, PRS_ERR_ALLOC, sizeof(parse_err));
				return(NULL);
			}

			rptr = parse_bool(pstr, po, size);
			if (rptr != NULL) {
				return(rptr);
			}

			pstr = parse_obsolete(pstr, buffer, size);

			if (pstr == NULL) {
				free(buffer);
				strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err));
				return(NULL);
			}

			po->type = PO_STRING;
			po->data = buffer;

			rptr = pstr;
#else
			rptr = parse_bool(pstr, po, size);
#endif
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
#ifdef PRS_OBSOLETE
		*pname, /* temporary to skip leading dot */
#endif
		*pstr;
#ifdef PRS_OBSOLETE
	int	 s;
#endif
	prscell	*pcell;
	prskw	*pkw;

#ifdef PRS_OBSOLETE
	pname = name;
	s = sizeof(name);

	if (*line == PMK_CHAR_COMMAND) {
		/* skip leading dot */
		*pname = *line;
		pname++;
		line++;
		s--;
	}

	pstr = parse_identifier(line, pname, s);
#else
	pstr = parse_identifier(line, name, sizeof(name));
#endif
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
		pstr = parse_identifier(pstr, pcell->label, sizeof(pcell->label));
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
		strlcpy(pcell->label, "none", sizeof(pcell->label));
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
		case PO_STRING :
			debugf("value = '%s'", popt->value->data);
			break;
		case PO_LIST :
			debugf("value = (*LIST*)");
			break;
		default :
			debugf("value = !unknow type!");
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
	parse pmkfile

	fd : file descriptor
	pdata : parsing data structure
	kwtab : keyword table
	size : size of keyword table

	return : boolean
*/

bool parse_pmkfile(FILE *fp, prsdata *pdata, prskw kwtab[], size_t size) {
	bool	 process = false;
	char	 buf[MAX_LINE_LEN],
		*pbuf;
	htable	*phkw;
	int	 cur_line = 0;
	prscell	*pcell = NULL,
		*ncell;
	prsnode	*tnode,
		*pnode;
	prsopt	 opt,
		*nopt;

	tnode = prsnode_init();
	pdata->tree = tnode;

	phkw = keyword_hash(kwtab, size);

	while (get_line(fp, buf, sizeof(buf)) == true) {
		/* update current line number */
		cur_line++;

		pbuf = skip_blank(buf);

		/* check first character */
		switch (*pbuf) {
			case CHAR_COMMENT :
				/* ignore comments */
				/* printf("DEBUG COMMENT = %s\n", buf); */
				break;

			case PMK_CHAR_COMMAND_END :
				if (process == true) {
					process = false;
					prsnode_add(tnode, pcell);					
#ifdef DEBUG_PRS
					debugf("end of command");
#endif
				} else {
					errorf("line %d : %s not found", cur_line, PMK_END_COMMAND);
					return(false);
				}
				break;

			case CHAR_EOS :
				/* empty line */
				break;

			default :
				if (process == true) {
					if (parse_opt(pbuf, &opt, PRS_PMKFILE_SEP) == false) {
						errorf("line %d : %s", cur_line, parse_err);
#ifdef DEBUG_PRS
						debugf("parse_opt returned false");
#endif
						prscell_destroy(pcell);
						return(false);
					} else {
#ifdef DEBUG_PRS
						debugf("recording '%s' key", opt.key);
#endif
						/* key name and value are ok */
						switch(pcell->type) {
							case PRS_KW_NODE :
								pnode = pcell->data;

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
								break;

							case PRS_KW_CELL :
								if (hash_add(pcell->data, opt.key, opt.value) == HASH_ADD_FAIL) {
									strlcpy(parse_err, PRS_ERR_HASH, sizeof(parse_err));
									return(false);
								}
								break;

							default :
								strlcpy(parse_err, "unknow type.", sizeof(parse_err));
								return(false);
								break;
						}
					}
				} else {
					/* parse command and label */
					pcell = parse_cell(pbuf, phkw);
					if (pcell == NULL) {
						errorf("line %d : %s", cur_line, parse_err);
#ifdef DEBUG_PRS
						debugf("parse_cell returned NULL");
#endif
						return(false);
					}

/*					debugf("type = %d", pcell->type); / XXX */

					process = true;
				}

				break;
		}
	}

	if (process == true) {
		/* found EOF before end of command */
		prscell_destroy(pcell);
		errorf("line %d : %s not found", cur_line, PMK_END_COMMAND);
		return(false);
	}

	if (feof(fp) == 0) {
		/* error occured before EOF */
		prscell_destroy(pcell);
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

