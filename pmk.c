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


#include <sys/param.h>
#include <err.h>
#include <stdio.h>

#include "pmk.h"


int	cur_line = 0,
	err_line = 0;
char	err_msg[MAX_ERR_MSG_LEN] = "";


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
	parse a command

	line : line to parse
	command : pmkcmd structure where to store the command and label

	returns a boolean
*/

bool parse_cmd(char *line, pmkcmd *command) {
	int	i,
		j;
	char	buf[MAX_LABEL_LEN];
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
				strncpy(command->label, buf, MAX_CMD_NAME_LEN);
				label_found = TRUE;
				j = 0; /* useless :) */
			}
		}
		i++;
	}

	if (cmd_found == FALSE) {
		/* command without label */
		buf[j] = '\0';
		strncpy(command->name, buf, MAX_CMD_NAME_LEN);
		strncpy(command->label, "", MAX_LABEL_LEN);
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

bool parse_opt(char *line, pmkcmdopt *pair) {
	char	buf[16];

	snprintf(buf, sizeof(buf), "%%%i[^=]=%%%i[^$]", MAX_OPT_NAME_LEN, MAX_OPT_VALUE_LEN);
	if (sscanf(line, buf, pair->name, pair->value) != 2) {
			err_line = cur_line;
			snprintf(err_msg, sizeof(err_msg), "Malformed option");
			return(FALSE);
	}

	return(TRUE);
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
	pmkcmdopt	opt;

	while (getline(fd, buf, sizeof(buf)) == TRUE) {
		/* check first character */
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comments */
				/* XXX printf("DEBUG COMMENT = %s\n", buf); */
				break;

			case CHAR_COMMAND :
				if (process == FALSE) {
					/* parse command and label */
					if (parse_cmd(buf, &cmd) == FALSE) {
						/* line number and error message already set */
						return(FALSE);
					}

					cmd_line = cur_line;
					process = TRUE;
				} else {
					/* looking for end of command */
					if (strcmp(buf, END_COMMAND) == 0) {
						cmd_line = 0;
						process = FALSE;
						/* found */
					} else {
						/* found another command before end of previous */
						err_line = cmd_line;
						snprintf(err_msg, sizeof(err_msg), "%s not found", END_COMMAND);
						return(FALSE);
					}
				}
				break;

			default :
				if (process == FALSE) {
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "Syntax error");
					return(FALSE);
				}

				/* XXX actually just parse option without adding it to cmd */
				if (parse_opt(buf, &opt) == FALSE) {
					/* line number and error message already set */
					return(FALSE);
				}
				break;
		}
	}

	if (process == TRUE) {
		err_line = cmd_line;
		snprintf(err_msg, sizeof(err_msg), "%s not found", END_COMMAND);
		return(FALSE);
	}

	return(TRUE);
}

/*
	usage
*/

void usage(void) {
	/* does nothing yet ... */
	/* ... or maybe just makes xsa happy ;) */
}

/*
	main
*/

int main(int argc, char *argv[]) {
	FILE	*fd,
		*cfd,
		*lfd;
	char	cf[MAXPATHLEN];

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

	if (parse(fd) == FALSE) {
		printf("Error line %d : %s.\n", err_line, err_msg);
		return(-1);
	}

	fprintf(lfd, "End of log.\n");

	/* flush and close files */
	fflush(lfd);
	fclose(lfd);

	fflush(fd);
	fclose(fd);

	return(0);
}
