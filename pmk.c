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


/* include needed for MAXPATHLEN */
#ifdef __OpenBSD__
#	include <sys/param.h>
#endif
#ifdef __FreeBSD__
#	include <sys/param.h>
#endif
#ifdef __NetBSD__
/* not tested */
#	include <sys/param.h>
#endif
#ifdef MACOSX
#	include <sys/param.h>
#endif
#ifdef SOLARIS
/* not tested */
#	include <sys/param.h>
#endif
#ifdef SUNOS
#	include <sys/param.h>
#endif
#ifdef hpux
/* not tested */
#	include <sys/param.h>
#endif
#ifdef AIX
/* not tested */
#	include <sys/param.h>
#endif
#ifdef IRIX
/* not tested */
#	include <sys/param.h>
#endif
#ifdef OSF1
/* not tested */
#	include <sys/param.h>
#endif
#ifdef __osf__
/* not tested */
#	include <sys/param.h>
#endif
#ifdef ULTRIX
/* not tested */
#	include <sys/param.h>
#endif
#ifdef linux
/* not tested */
#	include <sys/param.h>
#endif

/* sys/param.h also valid for :
	dg-ux
	4.4BSD
*/

/* imcompatible systems :
	chorus		arpa/ftp.h
*/


#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "premake.h"
#include "pmk.h"
#include "hash.h"
#include "func.h"

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
	get a line from a file

	fd : file descriptor
	line : buffer that will contain the line
	lsize : size of the buffer

	returns a boolean
*/

bool getline(FILE *fd, char *line, int lsize) {
	char	*p;

	if (fgets(line, lsize, fd) != NULL) {
		/* update current line number */
		cur_line++;

		p = (char *)strchr(line, '\n');
		if (p != NULL) {
			/* remove trailing newline */
			*p= '\0';
		}
		return TRUE;
	} else {
		/* XXX test eof ? */
		return FALSE;
	}
}

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
				/* needed to take care of quotation marks ? */
				buf[j] = line[i];
				j++;
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
	char	buf[16],
		tkey[MAX_OPT_NAME_LEN],
		tval[MAX_OPT_VALUE_LEN];

	snprintf(buf, sizeof(buf), "%%%i[^=]=%%%i[^$]", MAX_OPT_NAME_LEN, MAX_OPT_VALUE_LEN);
	if (sscanf(line, buf, tkey, tval) != 2) {
			err_line = cur_line;
			snprintf(err_msg, sizeof(err_msg), "Malformed option");
			return(FALSE);
	} else {
		hash_add(ht, tkey, tval);

		return(TRUE);
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

	while (getline(fd, buf, sizeof(buf)) == TRUE) {
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
					tabopts = hash_init(MAX_CMD_OPT); /* XXX is NULL ? */
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

	/* open pmk file */
	fd = fopen(PREMAKE_FILENAME, "r");
	if (fd == NULL) {
		warn("%s", PREMAKE_FILENAME);
		exit(1);
	}

	/* open log file */
	lfd = fopen(PREMAKE_LOG, "w");
	if (lfd == NULL) {
		warn("%s", PREMAKE_LOG);
		exit(1);
	}
	fprintf(lfd, "pmk version %s\n", PREMAKE_VERSION);

	/* open configuration file */
	snprintf(cf, sizeof(cf), "%s/%s", SYSCONFDIR, PREMAKE_CONFIG);
	cfd = fopen(cf, "r");
	if (cfd == NULL) {
		printf("Error : %s not found in %s.\n", PREMAKE_CONFIG, SYSCONFDIR);
		/* no pmksetup available so we ignore this error temporary ...
		return(-1);
		*/
	} else {
		fclose(cfd);
	}

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
		printf("Error line %d : %s.\n", err_line, err_msg);
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
