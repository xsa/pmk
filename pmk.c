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
#include <err.h>
#include <sys/param.h>

#include "pmk.h"


int	cur_line = 0,
	err_line = 0;
char	err_msg[MAX_ERR_MSG_LEN] = "";


/*
	get a line from a file
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
*/

bool parse_cmd() {
}

/*
	parse the configuration file
	fd : file descriptor
*/

bool parse(FILE *fd) {
	char	buf[MAX_LINE_LEN],
		cmd[MAX_CMD_LEN];
	int	cmd_line = 0;
	bool	process = FALSE;

	while (getline(fd, buf, sizeof(buf)) == TRUE) {
		/* check first character */
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comments */
				printf("DEBUG COMMENT = %s\n", buf);
				break;

			case CHAR_COMMAND :
				if (process == FALSE) {
					/* XXX process cmd */

					printf("DEBUG COMMAND = %s\n", buf);
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
						snprintf(err_msg, sizeof(err_msg), "%s not found.", END_COMMAND);
						return(FALSE);
					}
				}
				break;

			default :
				if (process == FALSE) {
					err_line = cur_line;
					snprintf(err_msg, sizeof(err_msg), "syntax error");
					return(FALSE);
				}
				printf("COMMAND OPTION=%s\n", buf);
				break;
		}
	}

	if (process == TRUE) {
		err_line = cmd_line;
		snprintf(err_msg, sizeof(err_msg), "%s not found.", END_COMMAND);
		return(FALSE);
	}

	return(TRUE);
}


/*
	main
*/

int main(int argc, char *argv[]) {
	FILE	*fd,
		*cfd;
	char	cf[MAXPATHLEN];

	fd = fopen(PREMAKE_FILENAME, "r");
	if (fd == NULL) {
		warn("%s", PREMAKE_FILENAME);
		exit(1);
	}

	snprintf(cf, sizeof(cf), "%s/%s", SYSCONFDIR, PREMAKE_CONFIG);
	cfd = fopen(cf, "r");
	if (cfd == NULL) {
		printf("Error : %s not found in %s\n", PREMAKE_CONFIG, SYSCONFDIR);
	} else {
		fclose(cfd);
	}

	if (parse(fd) == FALSE) {
		printf("Error line %d : %s\n", err_line, err_msg);
		return(-1);
	}
	fclose(fd);

	return(0);
}
