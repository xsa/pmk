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


/*
	XXX TODO :
		- error messages
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
	parse a command

	line : line to parse
	command : pmkcmd structure where to store the command and label
	pgd : global data structure (for pmkfile name)

	returns a boolean
*/

bool parse_cell(char *line, prscell *pcell) {
	bool	 cmd_found = false,
		 label_found = false;
	char	 buf[MAX_LABEL_NAME_LEN], /* should be big enough to contain command/label name */
		*pbf;
	int	 s,
		 so;

	s = MAX_CMD_LEN - 1;
	line++; /* ignore prefix character of the command string */
	pbf = buf;

	while (*line != CHAR_EOS && s != 0) {
		if (cmd_found == false) {
			if (*line != '(') {
				/* check uppercase */
				if (isalpha(*line) && islower(*line)) {
					/* my god, found a lowercase in command name ! */
					strlcpy(parse_err, "command should be in uppercase.", sizeof(parse_err));
					return(false);
				} else {
					/* good boy */
					*pbf = *line;
					pbf++;
				}
			} else {
				/* end of command name */
				*pbf = CHAR_EOS;

				so = sizeof(pcell->name);
				if (strlcpy(pcell->name, buf, so) >= so) {
					strlcpy(parse_err, "command too long.", sizeof(parse_err));
					return(false);
				}
				cmd_found = true;
				pbf = buf;
			}
		} else {
			if (*line != ')') {
				if (isalpha(*line) == 0 && *line != '_') {
					/* invalid character */
					strlcpy(parse_err, "invalid label name.", sizeof(parse_err));
					return(false);
					
				} else {
					/* needed to take care of quotation marks ? */
					*pbf = *line;
					pbf++;
				}
			} else {
				*pbf = CHAR_EOS;
				so = sizeof(pcell->label);
				if (strlcpy(pcell->label, buf, so) >= so) {
					strlcpy(parse_err, "label too long", sizeof(parse_err));
					return(false);
				}
				label_found = true;
				pbf = buf; /* useless :) */
			}
		}
		line++;
		s--;
	}

	if (cmd_found == false) {
		if (s != 0) {
			/* command without label */
			*pbf = CHAR_EOS;

			so = sizeof(pcell->name);
			if (strlcpy(pcell->name, buf, so) >= so) {
				strlcpy(parse_err, "command too long.", sizeof(parse_err));
				return(false);
			}

			/* the following should not need a check (label size > 3) */
			strlcpy(pcell->label, "", sizeof(pcell->label));
		} else {
			strlcpy(parse_err, "command too long.", sizeof(parse_err));
			return(false);
		}
	} else {
		if (label_found == true) {
			if (*line != CHAR_EOS) {
				/* some data remaining after parenthesis */
				strlcpy(parse_err, "trailing garbage after label.", sizeof(parse_err));
				return(false);
			}
		} else {
			/* ending parenthesis missing */
			strlcpy(parse_err, "label not terminated.", sizeof(parse_err));
			return(false);
		}
	}

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
	bool	f_key = false,
		f_oper = false,
		f_value = false,
		f_qmark = false;
	char	buf[MAXPATHLEN], /* XXX buf len ? */
		tkey[MAX_OPT_NAME_LEN],
		tval[MAX_OPT_VALUE_LEN];
	int	j = 0;

#ifdef DEBUG_PRS
	debugf("parsing opt line '%s'", line);
#endif
	do {
#ifdef DEBUG_PRS
		debugf("chr = '%c'", *line);
#endif
		if ((f_qmark == true) || (*line == CHAR_EOS)) {
#ifdef DEBUG_PRS
			debugf("qmark or EOS ('%c')", *line);
#endif
			switch (*line) {
				case '"' :
				case CHAR_EOS :
					buf[j] = CHAR_EOS;
					j++;
					f_qmark = false;

					if (f_key == false) {
#ifdef DEBUG_PRS
						debugf("key is '%s'", buf);
#endif
						/* set the key */
						if (strlcpy(tkey, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
							/* key value is too long */
							strlcpy(parse_err, "key value too long.", sizeof(parse_err));
							return(false);
						} else {
							f_key = true;
							j = 0;
						}
					} else {
						if (f_value == false) {
#ifdef DEBUG_PRS
							debugf("value is '%s'", buf);
#endif
							/* XXX TODO strdup should be enough here */
							if (strlcpy(tval, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
								/* key value is too long */
								strlcpy(parse_err, "key value too long.", sizeof(parse_err));
								return(false);
							} else {
								f_value = true;
								j = 0;
							}
						}
					}
					break;

				default :
					buf[j] = *line;
#ifdef DEBUG_PRS
					debugf("add chr '%c', j = %d", *line, j);
#endif
					j++;
					break;
			}
		} else {
			/* if not a blank character */
			if (isblank(*line) == 0) {
				switch (*line) {
					case PMK_KEY_CHAR :
						if (f_key == true) {
							f_oper = true;
						} else {
							if (j != 0) {
								buf[j] = CHAR_EOS;
								if (strlcpy(tkey, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
									/* key value is too long */
									strlcpy(parse_err, "key value too long.", sizeof(parse_err));
									return(false);
								} else {
									f_key = true;
									j = 0;
								}
							} else {
								strlcpy(parse_err, "operator without key.", sizeof(parse_err));
								return(false);
							}
						}
						break;

					case '"' :
						f_qmark = true;
						break;

					default :
						if ((isalpha(*line) != 0) || (*line == '_')) {
							buf[j] = *line;
							j++;
						} else {
							strlcpy(parse_err, "syntax error.", sizeof(parse_err));
							return(false);
						}
				}
			}
		}
	} while (*line++ != CHAR_EOS);

	if (f_value == true) {
#ifdef DEBUG_PRS
		debugf("recording '%s' with '%s'", tkey, tval);
#endif
		/* key name and value are ok */
		if (hash_add(ht, tkey, strdup(tval)) == HASH_ADD_FAIL) {
			errorf("hash failure.");
			return false;
		}
		return(true);
	} else {
		/* value not found */
		strlcpy(parse_err, "value not found.", sizeof(parse_err));
		return(false);
	}
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
					pcell = prscell_init(); /* XXX TODO check if null */
					if (pcell == NULL) {
						/* XXX error */
						return(false);
					}

					/* parse command and label */
					if (parse_cell(buf, pcell) == false) { /* XXX TODO should use prsdata */
						/* line number and error message already set */
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

			case CHAR_EOS :
				/* empty line */
				break;

			default :
				if (process == false) {
					errorf("line %d : syntax error.", cur_line);
					return(false);
				}

				if (parse_opt(buf, pcell->ht) == false) {
					/* line number and error message already set <= XXX FALSE !! TODO */
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

