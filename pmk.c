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


/* We include sys/param.h to get MAXPATHLEN.
   This is known to work under following operanding systems :
	OpenBSD
	FreeBSD
	NetBSD			(not verified)
	MACOSX
	Solaris			(not verified)
	SunOS			(not verified)
	HPUX			(not verified)
	AIX			(not verified)
	IRIX			(not verified)
	OSF1			(not verified)
	Ultrix			(not verified)
	Linux based systems	(not verified)
	DG-UX			(not verified)
	4.4BSD			(not verified)

   Some systems does not provide the same location :
   	Chorus			arpa/ftp.h


   Comments about this stuff is welcome. If your system is not
   supported then take contact with us to fix it.
*/
#ifdef MAXPATHLEN_
#	include <sys/param.h>
#endif


#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat/compat.h"
#include "pmk.h"
#include "func.h"
#include "common.h"

int	cur_line = 0,
	err_line = 0;
char	err_msg[MAX_ERR_MSG_LEN] = "";

/* keyword data */
htable		*khash;
cmdkw	functab[] = {
	{"DEFINE", pmk_define},
	{"CHECK_BINARY", pmk_check_binary},
	{"CHECK_INCLUDE", pmk_check_include},
	{"CHECK_LIB", pmk_check_lib}
};


/*
	process a command
*/

bool process_cmd(pmkcmd *cmd, htable *ht) {
	char	*aidx;
	int	idx;
	bool	rval = FALSE;
	
	aidx = hash_get(khash, cmd->name);
	if (aidx != NULL) {
		/* getting index of function in functab */
		idx = atoi(aidx);
		/* launching cmd function */
		rval = (*functab[idx].fnp)(cmd, ht);
	}

	return(rval);
}

/*
	check if the given string is a valid command

	cmdname : command to check

	returns TRUE is command is valid
*/

bool check_cmd(char *cmdname) {
	if (hash_get(khash, cmdname) == NULL) {
		err_line = cur_line;
		snprintf(err_msg, sizeof(err_msg), "Unknown command %s", cmdname);
		return(FALSE);
	}
	return(TRUE);
}

/*
	parse a command

	line : line to parse
	command : pmkcmd structure where to store the command and label

	returns a boolean
*/

bool parse_cmd(char *line, pmkcmd *command) {
	int	i,
		j;
	char	buf[MAX_LABEL_NAME_LEN];
		/* should be big enough to contain command/label name */
	bool	cmd_found = FALSE,
		label_found = FALSE;

	i = 1; /* ignore prefix character of the command string */
	j = 0;
	while (line[i] != '\0' && i < MAX_CMD_LEN) {
		if (cmd_found == FALSE) {
			if (line[i] != '(') {
				/* check uppercase */
				if (isalpha(line[i]) && islower(line[i])) {
					/* my god, found a lowercase in command name ! */
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "Command should be in uppercase");
					return(FALSE);
				} else {
					/* good boy */
					buf[j] = line[i];
					j++;
				}
			} else {
				/* end of command name */
				buf[j] = '\0';
				if (check_cmd(buf) == FALSE) {
					/* line number and error message already set */
					return(FALSE);
				}
				strncpy(command->name, buf, MAX_CMD_NAME_LEN);
				cmd_found = TRUE;
				j = 0;
			}
		} else {
			if (line[i] != ')') {
				if (isalpha(line[i]) == 0 && line[i] != '_') {
					/* invalid character */
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "Invalid label name");
					return(FALSE);
					
				} else {
					/* needed to take care of quotation marks ? */
					buf[j] = line[i];
					j++;
				}
			} else {
				buf[j] = '\0';
				strncpy(command->label, buf, MAX_LABEL_NAME_LEN);
				label_found = TRUE;
				j = 0; /* useless :) */
			}
		}
		i++;
	}

	if (cmd_found == FALSE) {
		/* command without label */
		buf[j] = '\0';
		if (check_cmd(buf) == FALSE) {
			/* line number and error message already set */
			return(FALSE);
		}
		strncpy(command->name, buf, MAX_CMD_NAME_LEN);
		strncpy(command->label, "", MAX_LABEL_NAME_LEN);
	} else {
		if (label_found == TRUE) {
			if (line[i] != '\0') {
				/* some data remaining after parenthesis */
				err_line = cur_line;
				snprintf(err_msg, sizeof(err_msg), "Trailing garbage after label");
				return(FALSE);
			}
		} else {
			/* ending parenthesis missing */
			err_line = cur_line;
			snprintf(err_msg, sizeof(err_msg), "Label not terminated");
			return(FALSE);
		}
	}

	return TRUE;
}

/*
	parse an option line

	line : option line

	returns a boolean
*/

bool parse_opt(char *line, htable *ht) {
	char	buf[MAXPATHLEN],
		tkey[MAX_OPT_NAME_LEN],
		tval[MAX_OPT_VALUE_LEN];
	int	i = 0,
		j = 0;
	bool	keyfound = FALSE;

	while (line[i] != '\0' && i < MAXPATHLEN) {
		if (keyfound == FALSE) {
			if (line[i] == PMK_KEY_CHAR) {
				/* end of key name reached */
				buf[j] = '\0';
				if (strlcpy(tkey, buf, MAX_OPT_NAME_LEN) >= MAX_OPT_NAME_LEN) {
					/* key name is too long */
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "Key name is too long");
					return(FALSE);
				} else {
					keyfound = TRUE;
					j = 0;
				}
			} else {
				if (isalpha(line[i]) == 0) {
					/* invalid character */
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "Malformed option");
					return(FALSE);
				} else {
					buf[j] = line[i];
					j++;
				}
			}
		} else {
			/* grabbing key value */
			buf[j] = line[i];
			j++;
		}
		i++;
	}
	
	if (keyfound == FALSE) {
			/* key name undefined */
			err_line = cur_line;
			snprintf(err_msg, sizeof(err_msg), "Malformed option");
			return(FALSE);
	} else {
		if (strlcpy(tval, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
			/* key value is too long */
			err_line = cur_line;
			snprintf(err_msg, sizeof(err_msg), "Key value is too long");
			return(FALSE);
		} else {
			/* key name and value are ok */
			hash_add(ht, tkey, tval);
			return(TRUE);
		}
	}
}

/*
	parse the configuration file

	fd : file descriptor

	returns a boolean
*/

bool parse(FILE *fd) {
	char		buf[MAX_LINE_LEN];
	int		cmd_line = 0;
	bool		process = FALSE;
	pmkcmd		cmd;
	htable		*tabopts = NULL;

	while (get_line(fd, buf, sizeof(buf)) == TRUE) {
		/* update current line number */
		cur_line++;

		/* check first character */
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comments */
				/* XXX printf("DEBUG COMMENT = %s\n", buf); */
				break;

			case PMK_CHAR_COMMAND :
				if (process == FALSE) {
					/* parse command and label */
					if (parse_cmd(buf, &cmd) == FALSE) {
						/* line number and error message already set */
						return(FALSE);
					}

					cmd_line = cur_line;
					process = TRUE;
					tabopts = hash_init(MAX_CMD_OPT);
					if (tabopts == NULL) {
						err_line = cmd_line;
						snprintf(err_msg, sizeof(err_msg), "Cannot create hash table");
						return(FALSE);
					}
				} else {
					/* looking for end of command */
					if (strcmp(buf, PMK_END_COMMAND) == 0) {
						cmd_line = 0;
						process = FALSE;
						/* found */
						process_cmd(&cmd, tabopts);

						/* cmd processed, clean up */
						strncpy(cmd.name, "", MAX_CMD_NAME_LEN);
						strncpy(cmd.label, "", MAX_LABEL_NAME_LEN);
						hash_destroy(tabopts);
					} else {
						/* found another command before end of previous */
						hash_destroy(tabopts);
						err_line = cmd_line;
						snprintf(err_msg, sizeof(err_msg), "%s not found", PMK_END_COMMAND);
						return(FALSE);
					}
				}
				break;

			case '\0' :
				/* empty line */
				break;

			default :
				if (process == FALSE) {
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "Syntax error");
					return(FALSE);
				}

				if (parse_opt(buf, tabopts) == FALSE) {
					/* line number and error message already set */
					hash_destroy(tabopts);
					return(FALSE);
				}
				break;
		}
	}

	if (process == TRUE) {
		/* found EOF before end of command */
		hash_destroy(tabopts);
		err_line = cmd_line;
		snprintf(err_msg, sizeof(err_msg), "%s not found", PMK_END_COMMAND);
		return(FALSE);
	}

	return(TRUE);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmk\n");
}

/*
	main
*/

int main(int argc, char *argv[]) {
	FILE	*fd,
		*cfd,
		*lfd;
	char	cf[MAXPATHLEN];
	char	idxstr[4]; /* max 999 cmds, should be enough :) */
	int	rval = 0,
		i,
		s;


	if (argc != 1) {
		usage();
		exit(1);
	}

	/* open configuration file */
	snprintf(cf, sizeof(cf), "%s/%s", SYSCONFDIR, PREMAKE_CONFIG);
	cfd = fopen(cf, "r");
	if (cfd == NULL) {
		errorf("%s not found in %s.", PREMAKE_CONFIG, SYSCONFDIR);
		/* no pmksetup available so we ignore this error temporary ...
		return(-1);
		*/
	} else {
		fclose(cfd);
	}

	/* open pmk file */
	fd = fopen(PREMAKE_FILENAME, "r");
	if (fd == NULL) {
		errorf("while opening %s.", PREMAKE_FILENAME);
		exit(1);
	}

	/* open log file */
	lfd = fopen(PREMAKE_LOG, "w");
	if (lfd == NULL) {
		errorf("while opening %s.", PREMAKE_LOG);
		exit(1);
	}
	fprintf(lfd, "pmk version %s\n", PREMAKE_VERSION);

	fprintf(lfd, "Hashing pmk keywords ");
	s = sizeof(functab) / sizeof(cmdkw); /* compute number of keywords */
	khash = hash_init(s);
	if (khash != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < s ; i++) {
			snprintf(idxstr, 4, "%d", i);
			/* XXX
			printf("[DEBUG] add '%s' to keyword hash (%s)\n", functab[i].kw, idxstr);
			*/
			hash_add(khash, functab[i].kw, idxstr);
		}
	}
	/* print number of hashed command */
	fprintf(lfd, "(%d)\n", khash->count);

	if (parse(fd) == FALSE) {
		error_line(PREMAKE_FILENAME, err_line, err_msg);
		rval = -1;
	}

	fprintf(lfd, "End of log.\n");

	/* flush and close files */
	fflush(lfd);
	fclose(lfd);

	fflush(fd);
	fclose(fd);

	/* clear cmd hash */
	if (khash != NULL) {
		hash_destroy(khash);
	}

	return(rval);
}
