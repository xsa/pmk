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
		- clean prsdata structure
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


char	parse_err[MAX_ERRMSG_LEN];


/*
	XXX
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
	XXX
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
	XXX
*/

void	prscell_destroy(prscell *pcell) {
	hash_destroy(pcell->ht);
	free(pcell);
}

/*
	parse a command XXX

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

				/* XXX don't check !! Will do later
				if (check_cmd(buf, pgd) == false) {
					return(false);
				}
				*/

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
			/* XXX TODO remove ?
			if (check_cmd(buf, pgd) == false) {
				return(false);
			}
			*/
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
	parse an option line XXX

	line : option line
	ht : hash table to store option
	pgd : global data structure (for pmkfile name)
	display : enable error messages if true

	returns a boolean
*/

bool parse_opt(char *line, htable *ht) {
	bool	 keyfound = false;
	char	 buf[MAXPATHLEN], /* XXX buf len ? */
		 tkey[MAX_OPT_NAME_LEN],
		 tval[MAX_OPT_VALUE_LEN];
	int	 i = 0,
		 j = 0;

	while (line[i] != CHAR_EOS && i < MAXPATHLEN) {
		if (keyfound == false) {
			if (line[i] == PMK_KEY_CHAR) {
				/* end of key name reached */
				buf[j] = CHAR_EOS;
				if (strlcpy(tkey, buf, MAX_OPT_NAME_LEN) >= MAX_OPT_NAME_LEN) {
					/* key name is too long */
					strlcpy(parse_err, "key name too long.", sizeof(parse_err));
					return(false);
				} else {
					keyfound = true;
					j = 0;
				}
			} else {
				/* XXX disable check until new new new engine ;)
				if ((isalpha(line[i]) == 0) && (line[i] != '_')) {
					*//* invalid character *//*
					strlcpy(parse_err, "malformed option.", sizeof(parse_err));
					return(false);
				} else {
				*/
					buf[j] = line[i];
					j++;
				/*
				}
				*/
			}
		} else {
			/* grabbing key value */
			buf[j] = line[i];
			j++;
		}
		i++;
	}
	buf[j] = CHAR_EOS; /* terminate buf */

	
	if (keyfound == false) {
			/* key name undefined */
			strlcpy(parse_err, "malformed option", sizeof(parse_err));
			return(false);
	} else {
		if (strlcpy(tval, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
			/* key value is too long */
			strlcpy(parse_err, "key value too long.", sizeof(parse_err));
			return(false);
		} else {
			/* key name and value are ok */
			if (hash_add(ht, tkey, strdup(tval)) == HASH_ADD_FAIL) {
				errorf("hash failure.");
				return false;
			}
			return(true);
		}
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

						/* XXX TODO add the cell in prsdata */
					} else {
						/* found another command before end of previous */
						prscell_destroy(pcell);
/* XXX						errorf_line(pgd->pmkfile, cur_line, "%s not found", PMK_END_COMMAND); */
						return(false);
					}
				}
				break;

			case CHAR_EOS :
				/* empty line */
				break;

			default :
				if (process == false) {
/* XXX					errorf_line(pgd->pmkfile, cur_line, "Syntax error"); */
					return(false);
				}

				if (parse_opt(buf, pcell->ht) == false) {
					/* line number and error message already set <= XXX FALSE !! TODO */
					prscell_destroy(pcell);
					return(false);
				}
				break;
		}
	}

	if (process == true) {
		/* found EOF before end of command */
		prscell_destroy(pcell);
/* XXX		errorf_line(pgd->pmkfile, cur_line, "%s not found", PMK_END_COMMAND); */
		return(false);
	}

	if (feof(fp) == 0) {
		/* error occured before EOF */
		prscell_destroy(pcell);
/* XXX		errorf_line(pgd->pmkfile, cur_line, "end of file not reached."); */
		return(false);
	}

	return(true);
}

