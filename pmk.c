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
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat/compat.h"
#include "pmk.h"
#include "func.h"
#include "common.h"


char	pmkfile[MAXPATHLEN];
int	cur_line = 0;

/* keyword data */
htable		*keyhash;
cmdkw		functab[] = {
	{"DEFINE", pmk_define},
	{"TARGET", pmk_target},
	{"CHECK_BINARY", pmk_check_binary},
	{"CHECK_INCLUDE", pmk_check_include},
	{"CHECK_LIB", pmk_check_lib},
	{"CHECK_CONFIG", pmk_check_config}
};


/*
	process the target file to replace tags

	target : path of the target file

	returns TRUE on success
*/

bool process_template(char *template, htable *ht) {
	FILE	*tfd,
		*dfd;
	char	*path,
		*destfile,
		*dotidx,
		*value,
		final[MAXPATHLEN],
		lbuf[MAXPATHLEN],
		buf[MAXPATHLEN],
		tbuf[MAXPATHLEN];
	int	i,
		j,
		k;
	bool	replace;

	path = strdup(dirname(template));
	if (path == NULL) {
		errorf("Not enough memory !!");
		return(FALSE);
	}
	destfile = strdup(basename(template));
	if (destfile == NULL) {
		errorf("Not enough memory !!");
		return(FALSE);
	}

	/* remove suffix */
	dotidx = strrchr(destfile, '.');
	*dotidx = '\0';

	/* build destination file */
	snprintf(final, sizeof(final), "%s/%s", path, destfile);
	free(path);
	free(destfile);

	tfd = fopen(template, "r");
	if (tfd == NULL) {
		errorf("Cannot open %s.", template);
		return(FALSE);
	}

	dfd = fopen(final, "w");
	if (dfd == NULL) {
		fclose(tfd);
		errorf("Cannot open %s.", final);
		return(FALSE);
	}

	while (fgets(lbuf, sizeof(lbuf), tfd) != NULL) {
		i = 0;
		j = 0;
		k = 0;
		replace = FALSE;
		while (lbuf[i] != '\0') {
			if (replace == FALSE) {
				if (lbuf[i] == PMK_TAG_CHAR) {
					/* found begining of tag */
					replace = TRUE;
				} else {
					/* copy normal text */
					buf[j] = lbuf[i];
					j++;
				}
			} else {
				if (lbuf[i] == PMK_TAG_CHAR) {
					/* tag identified */
					replace = FALSE;
					tbuf[k] = '\0';

					value = hash_get(ht, tbuf);
					if (value == NULL) {
						/* not a valid tag */
						buf[j] = PMK_TAG_CHAR;
						j++;
						k = 0;
						while (tbuf[k] != '\0') {
							buf[j] = tbuf[k];
							j++;
							k++;
						}
						buf[j] = PMK_TAG_CHAR;
						j++;
					} else {
						/* replace with value */
						k = 0;
						while (value[k] != '\0') {
							buf[j] = value[k];
							j++;
							k++;
						}
					}
					k = 0;
				} else {
					/* continue getting tag name */
					tbuf[k] = lbuf[i];
					k++;
				}
			}
			i++;
		}
		if (replace == TRUE) {
			/* not a tag, copy tbuf in buf */
			buf[j] = PMK_TAG_CHAR;
			j++;
			tbuf[k] = '\0';
			k = 0;
			while (tbuf[k] != '\0') {
				buf[j] = tbuf[k];
				j++;
				k++;
			}
		}
		buf[j] = '\0';
		/* saving parsed line */
		fprintf(dfd, "%s", buf);
	}

	fclose(dfd);
	fclose(tfd);

	pmk_log("Created '%s'.\n", final);
	
	return(TRUE);
}

/*
	process a command
*/

bool process_cmd(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*aidx;
	int	idx;
	bool	rval = FALSE;
	
	aidx = hash_get(keyhash, cmd->name);
	if (aidx != NULL) {
		/* getting index of function in functab */
		idx = atoi(aidx);
		/* launching cmd function */
		rval = (*functab[idx].fnp)(cmd, ht, gdata);
	}

	return(rval);
}

/*
	check if the given string is a valid command

	cmdname : command to check

	returns TRUE is command is valid
*/

bool check_cmd(char *cmdname) {
	if (hash_get(keyhash, cmdname) == NULL) {
		errorf_line(pmkfile, cur_line, "Unknown command %s", cmdname);
		return(FALSE);
	} else {
		return(TRUE);
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
					errorf_line(pmkfile, cur_line, "Command should be in uppercase");
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
					errorf_line(pmkfile, cur_line, "Invalid label name");
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
				errorf_line(pmkfile, cur_line, "Trailing garbage after label");
				return(FALSE);
			}
		} else {
			/* ending parenthesis missing */
			errorf_line(pmkfile, cur_line, "Label not terminated");
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
					errorf_line(pmkfile, cur_line, "Key name is too long");
					return(FALSE);
				} else {
					keyfound = TRUE;
					j = 0;
				}
			} else {
				if (isalpha(line[i]) == 0) {
					/* invalid character */
					errorf_line(pmkfile, cur_line, "Malformed option");
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
	buf[j] = '\0'; /* terminate buf */

	
	if (keyfound == FALSE) {
			/* key name undefined */
			errorf_line(pmkfile, cur_line, "Malformed option");
			return(FALSE);
	} else {
		if (strlcpy(tval, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
			/* key value is too long */
			errorf_line(pmkfile, cur_line, "Key value is too long");
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

bool parse(FILE *fd, pmkdata *gdata) {
	char		buf[MAX_LINE_LEN];
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

					process = TRUE;
					tabopts = hash_init(MAX_CMD_OPT);
					if (tabopts == NULL) {
						errorf_line(pmkfile, cur_line, "Cannot create hash table");
						return(FALSE);
					}
				} else {
					if (strcmp(buf, PMK_END_COMMAND) == 0) {
						/* found end of command */
						process = FALSE;
						if (process_cmd(&cmd, tabopts, gdata) == FALSE) {
							/* command processing failed */
							hash_destroy(tabopts);
							return(FALSE);
						}

						/* cmd processed, clean up */
						strncpy(cmd.name, "", MAX_CMD_NAME_LEN);
						strncpy(cmd.label, "", MAX_LABEL_NAME_LEN);
						hash_destroy(tabopts);
					} else {
						/* found another command before end of previous */
						hash_destroy(tabopts);
						errorf_line(pmkfile, cur_line, "%s not found", PMK_END_COMMAND);
						return(FALSE);
					}
				}
				break;

			case '\0' :
				/* empty line */
				break;

			default :
				if (process == FALSE) {
					errorf_line(pmkfile, cur_line, "Syntax error");
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
		errorf_line(pmkfile, cur_line, "%s not found", PMK_END_COMMAND);
		return(FALSE);
	}

	if (feof(fd) == 0) {
		/* error occuered before EOF */
		hash_destroy(tabopts);
		errorf_line(pmkfile, cur_line, "end of file not reached.");
		return(FALSE);
	}

	return(TRUE);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmk [-vh] [-f file] [options]\n");
}

/*
	main
*/

int main(int argc, char *argv[]) {
	FILE	*fd,
		*cfd;
	char	cf[MAXPATHLEN];
	char	idxstr[4]; /* max 999 cmds, should be enough :) */
	int	rval = 0,
		i,
		s,
		chr;
	bool	go_exit = FALSE,
		pmkfile_set = FALSE;
	pmkdata	gdata;
	dynary	*da;

	while (go_exit == FALSE) {
		chr = getopt(argc, argv, "f:hv");
		if (chr == -1) {
			go_exit = TRUE;
		} else {
			switch (chr) {
				case 'f' :
					/* filename */
					if (strlcpy(pmkfile, optarg, sizeof(pmkfile)) >= sizeof(pmkfile)) {
						errorf("Cannot use file argument");
						exit(1);
					} else {
						pmkfile_set = TRUE;
					}
					break;
				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(0);
					break;
				case 'h' :
				case '?' :
				default :
					usage();
					exit(1);
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

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

	if (argc != 0) {
		/* XXX parse optional arguments that override pmk.conf */
		/* as for example PREFIX=/opt/ */
	}

	/* open pmk file */
	if (pmkfile_set == FALSE) {
		strlcpy(pmkfile, PREMAKE_FILENAME, sizeof(pmkfile)); /* should not fail */
	}
	fd = fopen(pmkfile, "r");
	if (fd == NULL) {
		errorf("while opening %s.", pmkfile);
		exit(1);
	}

	/* open log file */
	if (pmk_log_open(PREMAKE_LOG) == FALSE) {
		exit(1);
	}

	pmk_log("PreMaKe version %s\n", PREMAKE_VERSION);
	pmk_log("Hashing pmk keywords ");
	s = sizeof(functab) / sizeof(cmdkw); /* compute number of keywords */
	keyhash = hash_init(s);
	if (keyhash != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < s ; i++) {
			snprintf(idxstr, 4, "%d", i);
			hash_add(keyhash, functab[i].kw, idxstr);
		}
	}
	/* print number of hashed command */
	pmk_log("(%d)\n", keyhash->count);

	gdata.htab = hash_init(MAX_DATA_KEY);

	if (parse(fd, &gdata) == FALSE) {
		/* an error occured while parsing */
		rval = 1;
	} else {
		pmk_log("\n");

		da = gdata.tlist;
		for (i=0 ; i < da_size(da) ; i++) {
			process_template(da_idx(da, i), gdata.htab);
		}
		
		da_destroy(da);

		pmk_log("\n");
		pmk_log("End of log.\n");
	}

	/* flush and close files */
	pmk_log_close();

	fflush(fd);
	fclose(fd);

	/* clear cmd hash */
	if (keyhash != NULL) {
		hash_destroy(keyhash);
	}

	return(rval);
}
