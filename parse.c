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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"

#include "common.h"
#include "hash.h"
#include "parse.h"
#include "pmk.h"

/*
#define DEBUG_PRS	1
*/

char	parse_err[MAX_ERRMSG_LEN];


/*
	initialize parsing data structure

	return : prsdata structure
*/

prsdata *prsdata_init(void) {
	prsdata	*pdata;

	pdata = (prsdata *) malloc(sizeof(prsdata));
	if (pdata != NULL) {
		pdata->first = NULL;
		pdata->last = NULL;
	}

	return(pdata);
}

/*
	free space allocated by parsing structure

	pdata : structure to clean
*/

void prsdata_destroy(prsdata *pdata) {
	prscell	*p,
		*n;

	p = pdata->first;
	while (p != NULL) {
		n = p;
		p = n->next;
		free(n);
	}
	free(pdata);
}

/*
	initialize parsing cell structure

	return : parsing cell structure
*/

prscell *prscell_init(void) {
	htable	*ht;
	prscell	*pcell;

	pcell = (prscell *) malloc(sizeof(prscell));
	if (pcell != NULL) {
		ht = hash_init(MAX_CMD_OPT);
		if (ht != NULL) {
			pcell->ht = ht;
			pcell->next = NULL;
		} else {
			/* cannot init htable */
			free(pcell);
			return(NULL);
		}
	}

	return(pcell);
}

/*
	free space allocated for parsing cell structure

	pcell : structure to clean
*/

void	prscell_destroy(prscell *pcell) {
	hash_destroy(pcell->ht);
	free(pcell);
}

/*
	get quoted string
*/

char *parse_quoted(char *pstr, char *buffer, size_t size) {
	while ((*pstr != PMK_CHAR_QUOTE_END) && (*pstr != CHAR_EOS)) {
		if (size > 1) {
			*buffer = *pstr;
			pstr++;
			buffer++;
			size--;
		} else {
			return(NULL);
		}
	}

	if (*pstr == PMK_CHAR_QUOTE_END) {
		/* found end of quoted string */
		*buffer = CHAR_EOS;
		pstr++;
		return(pstr);
	} else {
		/* end of quoting not found */
		return(NULL);
	}
}

/*
	get list

	NOTE: code is especially redundant as it will create pmk object later.
*/

char *parse_list(char *pstr, char *buffer, size_t size) {
	while ((*pstr != PMK_CHAR_LIST_END) && (*pstr != CHAR_EOS)) {
		if (size > 1) {
			*buffer = *pstr;
			pstr++;
			buffer++;
			size--;
		} else {
			return(NULL);
		}
	}

	if (*pstr == PMK_CHAR_LIST_END) {
		/* found end of list */
		*buffer = CHAR_EOS;
		pstr++;
		return(pstr);
	} else {
		/* end of list not found */
		return(NULL);
	}
}

/*
	get word XXX
*/

char *parse_word(char *pstr, char *buffer, size_t size) {
	char	*rptr;

	switch (*pstr) {
		case '"' :
			pstr++;
			rptr = parse_quoted(pstr, buffer, size);
			break;

		case '(' :
			pstr++;
			rptr = parse_list(pstr, buffer, size);
			break;

		default :
			while ((isalnum(*pstr) != 0) || (*pstr == '_')) {
				if (size > 1) {
					*buffer = *pstr;
					pstr++;
					buffer++;
					size--;
				} else {
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
	skip blank character(s)
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

	returns a boolean
*/

bool parse_cell(char *line, prscell *pcell) {
	char	*pstr;

	line++; /* skip leading dot */

	pstr = parse_word(line, pcell->name, sizeof(pcell->name));
	if (pstr == NULL) {
		strlcpy(parse_err, "command parsing failed.", sizeof(parse_err));
		return(false);
	}
#ifdef DEBUG_PRS
	debugf("command = '%s'", pcell->name);
#endif

	switch (*pstr) {
		case CHAR_EOS :
#ifdef DEBUG_PRS
			debugf("command '%s' with no label", pcell->name);
#endif
			/* command without label (old format) */
			return(true);
			break; /* yes i know */

		case PMK_CHAR_LABEL_START :
			/* found label starting delimiter */
			pstr++;
			break;

		case ' ' :
			/* command without label (new format) */
			pstr++;
			if (*pstr != PMK_CHAR_COMMAND_START) {
				strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
				return(false);
			}
			pstr++;
			if (*pstr != CHAR_EOS) {
				strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
				return(false);
			} else {
				/* format okay */
				return(true);
			}
			break;

		default :
			/* command must be immediately followed by starting label delimiter */
			strlcpy(parse_err, "starting label delimiter not found.", sizeof(parse_err));
			return(false);
	}

	pstr = parse_word(pstr, pcell->label, sizeof(pcell->label));
	if (pstr == NULL) {
		strlcpy(parse_err, "label parsing failed.", sizeof(parse_err));
		return(NULL);
	}
#ifdef DEBUG_PRS
	debugf("label = '%s'", pcell->label);
#endif

	if (*pstr != PMK_CHAR_LABEL_END) {
		/* label name must be immediately followed by closing delimiter */
		return(false);
	} else {
		pstr ++;
	}

	/* old format compatibility */
	if (*pstr == CHAR_EOS) {
#ifdef DEBUG_PRS
		debugf("command '%s' with label '%s'", pcell->name, pcell->label);
#endif
		/* everything is ok */
		return(true);
	}

	if (*pstr != ' ') {
		strlcpy(parse_err, PRS_ERR_UNKNOWN, sizeof(parse_err));
		return(false);
	} else {
		pstr++;
	}

	if (*pstr != PMK_CHAR_COMMAND_START) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(false);
	} else {
		pstr++;
	}

	if (*pstr != CHAR_EOS) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(false);
	}

#ifdef DEBUG_PRS
	debugf("command '%s' with label '%s'", pcell->name, pcell->label);
#endif
	/* everything is ok */
	return(true);
}

/*
	parse an option line

	line : option line
	ht : hash table to store option
	pgd : global data structure (for pmkfile name)
	display : enable error messages if true

	returns a boolean
*/

bool parse_opt(char *line, htable *ht) {
	char	 key[MAX_OPT_NAME_LEN],
		 value[MAX_OPT_VALUE_LEN],
		*pstr;

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

	pstr = parse_word(pstr, key, sizeof(key));
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}
#ifdef DEBUG_PRS
	debugf("key = '%s'", key);
#endif

	pstr = skip_blank(pstr);
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}

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

	pstr = parse_word(pstr, value, sizeof(value));
	if (pstr == NULL) {
		strlcpy(parse_err, PRS_ERR_SYNTAX, sizeof(parse_err));
		return(false);
	}
#ifdef DEBUG_PRS
	debugf("value = '%s'", value);
#endif

	if (*pstr != CHAR_EOS) {
		strlcpy(parse_err, PRS_ERR_TRAILING, sizeof(parse_err));
		return(false);
	}

#ifdef DEBUG_PRS
	debugf("recording '%s' with '%s'", key, value);
#endif
	/* key name and value are ok */
	if (hash_add(ht, key, strdup(value)) == HASH_ADD_FAIL) {
		strlcpy(parse_err, PRS_ERR_HASH, sizeof(parse_err));
		return(false);
	}

	return(true);
}

/*
	parse the configuration file

	fd : file descriptor
	gdata : pmkdata struct

	returns a boolean
*/

bool parse(FILE *fp, prsdata *pdata) {
	bool	 process = false;
	char	 buf[MAX_LINE_LEN];
	int	 cur_line = 0;
	prscell	*pcell = NULL;

	while (get_line(fp, buf, sizeof(buf)) == true) {
		/* update current line number */
		cur_line++;

		/* check first character */
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comments */
				/* printf("DEBUG COMMENT = %s\n", buf); */
				break;

			case PMK_CHAR_COMMAND :
				if (process == false) {
					pcell = prscell_init();
					if (pcell == NULL) {
						/* XXX TODO error */
						return(false);
					}

					/* parse command and label */
					if (parse_cell(buf, pcell) == false) { /* XXX TODO should use prsdata */
						errorf("line %d : %s", cur_line, parse_err);
#ifdef DEBUG_PRS
						debugf("parse_cell returned false");
#endif
						return(false);
					}

					process = true;
				} else {
					if (strcmp(buf, PMK_END_COMMAND) == 0) {
						/* found end of command */
						process = false;
						
						if (pdata->last != NULL) {
							pdata->last->next = pcell;
						} else {
							pdata->first = pcell;
						}
						pdata->last = pcell;

					} else {
						/* found another command before end of previous */
						prscell_destroy(pcell);
						errorf("line %d : %s not found", cur_line, PMK_END_COMMAND);
						return(false);
					}
				}
				break;
			case PMK_CHAR_COMMAND_END :
				if (process == true) {
					process = false;
						
					if (pdata->last != NULL) {
						pdata->last->next = pcell;
					} else {
						pdata->first = pcell;
					}
					pdata->last = pcell;
				} else {
					errorf("line %d : %s not found", cur_line, PMK_END_COMMAND);
					return(false);
				}
				break;

			case CHAR_EOS :
				/* empty line */
				break;

			default :
				if (process == false) {
					errorf("line %d : syntax error.", cur_line);
					return(false);
				}

				if (parse_opt(buf, pcell->ht) == false) {
					errorf("%s", parse_err);
#ifdef DEBUG_PRS
					debugf("parse_opt returned false");
#endif
					prscell_destroy(pcell);
					return(false);
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

