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
#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"

#include "common.h"
#include "hash.h"
#include "parse.h"
#include "pmk.h"


/*
#define DEBUG_PRS	1
*/

/* for compatibility with 0.6, will be removed later */
#define PRS_OBSOLETE

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
	XXX TODO
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
					pcell->next = NULL;
					rval = true;
				} else {
					/* cannot init htable */
					free(pcell);
				}
				break;
			case PRS_KW_ITEM :
				pht = hash_init_adv(MAX_CMD_OPT, (void (*)(void *))po_free,
					(void *(*)(void *, void *, void *))po_append);
				if (pht != NULL) {
					pcell->data = pht;
					pcell->next = NULL;
					rval = true;
				} else {
					/* cannot init htable */
					free(pcell);
				}
				break;
			default :
				break;
		}
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
			case PRS_KW_ITEM :
				hash_destroy(pcell->data);
				break;
			default :
				break;
		}
		free(pcell);
	}
}

/*
	XXX TODO
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
	get quoted string content

	pstr : current parsing cursor
	buffer : storage buffer
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
	buffer : storage buffer
	size : size of buffer

	return : new parsing cursor

	NOTE: code is especially redundant as it will create pmk object later.

	XXX need more comments
*/

char *parse_list(char *pstr, pmkobj *po, size_t size) {
	char	*buffer,
		*pbuf;
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
					return(NULL);
				} else {
					pbuf = buffer;
				}

				while ((isalnum(*pstr) != 0) || (*pstr == '_')) {
					if (size > 1) {
						*pbuf = *pstr;
						pbuf++;
						pstr++;
						size--;
					} else {
						da_destroy(pda);
						free(buffer);
						strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err));
						return(NULL);
					}

				}

				if (pbuf == buffer) {
					da_destroy(pda);
					strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
					return(NULL);
				}

				*pbuf = CHAR_EOS;

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
	get word

	pstr : current parsing cursor
	buffer : storage buffer
	size : size of buffer

	return : new parsing cursor or NULL
*/

char *parse_word(char *pstr, pmkobj *po, size_t size) {
	char	*buffer;
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
			buffer = (char *) malloc(size);
			if (buffer == NULL)
				return(NULL);

			po->type = PO_STRING;
			po->data = buffer;

			while ((isalnum(*pstr) != 0) || (*pstr == '_')) {
				if (size > 1) {
					*buffer = *pstr;
					pstr++;
					buffer++;
					size--;
				} else {
					free(po->data);
					strlcpy(parse_err, PRS_ERR_OVERFLOW, sizeof(parse_err));
					return(NULL);
				}
			}

			*buffer = CHAR_EOS;
			rptr = pstr;
			break;
	}
	return(rptr);
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
	parse a command

	line : line to parse
	command : pmkcmd structure where to store the command and label
	pgd : global data structure (for pmkfile name)

	return : boolean
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
	ht : hash table to store option
	pgd : global data structure (for pmkfile name)
	display : enable error messages if true

	return : boolean
*/

bool parse_opt(char *line, prsopt *popt) {
	char	*pstr;
	pmkobj	 po;

#ifdef DEBUG_PRS
	debugf("line = '%s'", line);
#endif

	if (*line == PMK_CHAR_COMMENT) {
#ifdef DEBUG_PRS
		debugf("comment : '%s'", line);
#endif
		 /* comment is ok */
		return(true);
	}

	pstr = skip_blank(line);
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_UNKNOWN, sizeof(parse_err));
		return(false);
	}

	pstr = parse_word(pstr, &po, sizeof(popt->key));
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
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}

#ifdef DEBUG_PRS
	debugf("assign = '%c'", *pstr);
#endif

	if (*pstr != PMK_CHAR_ASSIGN) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	} else {
		pstr++;
	}

	pstr = skip_blank(pstr);
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}

	popt->value = (pmkobj *) malloc(sizeof(pmkobj));
	if (popt->value == NULL) {
		strlcpy(parse_err, "memory allocation failed.", sizeof(parse_err));
		return(false);
	}

	pstr = parse_word(pstr, popt->value, OPT_VALUE_LEN);
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
	parse the configuration file

	fd : file descriptor
	gdata : pmkdata struct

	return : boolean
*/

bool parse_pmkfile(FILE *fp, prsdata *pdata, prskw kwtab[], size_t size) {
	bool	 process = false;
	char	 buf[MAX_LINE_LEN];
	htable	*phkw;
	int	 cur_line = 0;
	prscell	*pcell = NULL;
	prsnode	*tnode;
	prsopt	 opt;

	tnode = prsnode_init();
	pdata->tree = tnode;

	phkw = keyword_hash(kwtab, size);

	while (get_line(fp, buf, sizeof(buf)) == true) {
		/* update current line number */
		cur_line++;

		/* check first character */
		switch (buf[0]) {
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
					if (parse_opt(buf, &opt) == false) {
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
								/* XXX  TODO
									create new cell
									set type to PRS_KW_UNKW
									link it in node
									NOTE : could set token instead of type

								pcell = prscell_init();
								if (pcell == NULL) {
									errorf("prscell init failed");
									return(false);
								}

								pcell->token = pkw->token;
								pcell->type = pkw->type;

								prsnode_add(tnode, pcell);					
								*/
								break;
							case PRS_KW_ITEM :
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
					pcell = parse_cell(buf, phkw);
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

