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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compat/pmk_libgen.h" /* basename, dirname */
#include "compat/pmk_string.h" /* strlcpy */
#include "pmk.h"
#include "func.h"
#include "common.h"


extern char	*optarg;
extern int	optind;

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
	read configuration file

	ht : hash table that will contain data

	returns true on success
*/

bool read_conf(htable *ht) {
	FILE	*fp;
	char	buf[MAX_LINE_LEN];
	int	ln = 0;
	cfg_opt	co;

	fp = fopen(PREMAKE_CONFIG_PATH, "r");
	if (fp == NULL) {
		errorf("Cannot open %s.", PREMAKE_CONFIG_PATH);
		return(false);
	}

	while (get_line(fp, buf, sizeof(buf)) == true) {
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comment */
				break;

			case CHAR_EOS :
				/* ignore empty line */
				break;

			default :
				if (parse_conf_line(buf, ln, &co) == 0) {
					/* parse ok */
					hash_add(ht, co.key, co.val); /* XXX test ?*/
				} else {
					/* incorrect line */
					return(false);
				}
				break;
		}
		ln++; /* increment line number */
	}

	if (feof(fp) == 0) {
		/* error occuered before EOF */
		errorf_line(PREMAKE_CONFIG_PATH, cur_line, "end of file not reached.");
		return(false);
	}

	fclose(fp);

	return(true);
}

/*
	process the target file to replace tags

	target : path of the target file
	ht : global data

	returns true on success
*/

bool process_template(char *template, htable *ht) {
	FILE	*tfd,
		*dfd;
	char	*path,
		*destfile,
		*dup_p,
		*dup_d,
		*plb,
		*pbf,
		*ptbf,
		*ptmp,
		final[MAXPATHLEN],
		lbuf[MAXPATHLEN],
		buf[MAXPATHLEN],
		tbuf[MAXPATHLEN];
	bool	replace;

	dup_p = strdup(template);
	path = dirname(dup_p);
	if (path == NULL) {
		errorf("not enough memory !!");
		return(false);
	}
	dup_d = strdup(template);
	destfile = basename(dup_d);
	if (destfile == NULL) {
		errorf("not enough memory !!");
		return(false);
	}

	/* remove suffix */
	ptmp = strrchr(destfile, '.');
	if (ptmp != NULL) {
		*ptmp = CHAR_EOS;
	} else {
		errorf("error while creating name for template %s", destfile);
		return(false);
	}

	/* build destination file */
	snprintf(final, sizeof(final), "%s/%s", path, destfile);
	free(dup_p);
	free(dup_d);

	tfd = fopen(template, "r");
	if (tfd == NULL) {
		errorf("cannot open %s.", template);
		return(false);
	}

	dfd = fopen(final, "w");
	if (dfd == NULL) {
		fclose(tfd);
		errorf("cannot open %s.", final);
		return(false);
	}

	while (fgets(lbuf, sizeof(lbuf), tfd) != NULL) {
		plb = lbuf;
		pbf = buf;
		ptbf = tbuf;
		replace = false;
		while (*plb != CHAR_EOS) {
			if (replace == false) {
				if (*plb == PMK_TAG_CHAR) {
					/* found begining of tag */
					replace = true;
				} else {
					/* copy normal text */
					*pbf = *plb;
					pbf++;
				}
			} else {
				if (*plb == PMK_TAG_CHAR) {
					/* tag identified */
					replace = false;
					*ptbf = CHAR_EOS;

					ptmp = hash_get(ht, tbuf);
					if (ptmp == NULL) {
						/* not a valid tag, put it back */
						*pbf = PMK_TAG_CHAR;
						pbf++;

						ptmp = tbuf;
						while (*ptmp != CHAR_EOS) {
							*pbf = *ptmp;
							pbf++;
							ptmp++;
						}
						*pbf = PMK_TAG_CHAR;
						pbf++;
					} else {
						/* replace with value */
						while (*ptmp != CHAR_EOS) {
							*pbf = *ptmp;
							pbf++;
							ptmp++;
						}
					}
					ptbf = tbuf;
				} else {
					/* continue getting tag name */
					*ptbf = *plb;
					ptbf++;
				}
			}
			plb++;
		}
		if (replace == true) {
			/* not a tag, copy tbuf in buf */
			*pbf = PMK_TAG_CHAR;
			pbf++;
			*ptbf = CHAR_EOS;
			ptmp = tbuf;
			while (*ptmp != CHAR_EOS) {
				*pbf = *ptmp;
				pbf++;
				ptmp++;
			}
		}
		*pbf = CHAR_EOS;
		/* saving parsed line */
		fprintf(dfd, "%s", buf);
	}

	fclose(dfd);
	fclose(tfd);

	pmk_log("Created '%s'.\n", final);
	
	return(true);
}

/*
	process a command

	cmd : structure of the command to be processed
	ht : command options
	gdata : global data

	returns true on success
*/

bool process_cmd(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*aidx;
	int	idx;
	bool	rval = false;
	
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

	returns true is command is valid
*/

bool check_cmd(char *cmdname) {
	if (hash_get(keyhash, cmdname) == NULL) {
		errorf_line(pmkfile, cur_line, "Unknown command %s", cmdname);
		return(false);
	} else {
		return(true);
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
	bool	cmd_found = false,
		label_found = false;

	i = 1; /* ignore prefix character of the command string */
	j = 0;
	while (line[i] != CHAR_EOS && i < MAX_CMD_LEN) {
		if (cmd_found == false) {
			if (line[i] != '(') {
				/* check uppercase */
				if (isalpha(line[i]) && islower(line[i])) {
					/* my god, found a lowercase in command name ! */
					errorf_line(pmkfile, cur_line, "Command should be in uppercase");
					return(false);
				} else {
					/* good boy */
					buf[j] = line[i];
					j++;
				}
			} else {
				/* end of command name */
				buf[j] = CHAR_EOS;
				if (check_cmd(buf) == false) {
					/* line number and error message already set */
					return(false);
				}
				strlcpy(command->name, buf, MAX_CMD_NAME_LEN);
				cmd_found = true;
				j = 0;
			}
		} else {
			if (line[i] != ')') {
				if (isalpha(line[i]) == 0 && line[i] != '_') {
					/* invalid character */
					errorf_line(pmkfile, cur_line, "Invalid label name");
					return(false);
					
				} else {
					/* needed to take care of quotation marks ? */
					buf[j] = line[i];
					j++;
				}
			} else {
				buf[j] = CHAR_EOS;
				strlcpy(command->label, buf, MAX_LABEL_NAME_LEN);
				label_found = true;
				j = 0; /* useless :) */
			}
		}
		i++;
	}

	if (cmd_found == false) {
		/* command without label */
		buf[j] = CHAR_EOS;
		if (check_cmd(buf) == false) {
			/* line number and error message already set */
			return(false);
		}
		strlcpy(command->name, buf, MAX_CMD_NAME_LEN);
		strlcpy(command->label, "", MAX_LABEL_NAME_LEN);
	} else {
		if (label_found == true) {
			if (line[i] != CHAR_EOS) {
				/* some data remaining after parenthesis */
				errorf_line(pmkfile, cur_line, "Trailing garbage after label");
				return(false);
			}
		} else {
			/* ending parenthesis missing */
			errorf_line(pmkfile, cur_line, "Label not terminated");
			return(false);
		}
	}

	return true;
}

/*
	parse an option line

	line : option line
	ht : hash table to store option
	display : enable error messages if true

	returns a boolean
*/

bool parse_opt(char *line, htable *ht, bool display) {
	char	buf[MAXPATHLEN],
		tkey[MAX_OPT_NAME_LEN],
		tval[MAX_OPT_VALUE_LEN];
	int	i = 0,
		j = 0;
	bool	keyfound = false;

	while (line[i] != CHAR_EOS && i < MAXPATHLEN) {
		if (keyfound == false) {
			if (line[i] == PMK_KEY_CHAR) {
				/* end of key name reached */
				buf[j] = CHAR_EOS;
				if (strlcpy(tkey, buf, MAX_OPT_NAME_LEN) >= MAX_OPT_NAME_LEN) {
					/* key name is too long */
					if (display == true)
						errorf_line(pmkfile, cur_line, "Key name is too long");
					return(false);
				} else {
					keyfound = true;
					j = 0;
				}
			} else {
				if ((isalpha(line[i]) == 0) && (line[i] != '_')) {
					/* invalid character */
					if (display == true)
						errorf_line(pmkfile, cur_line, "Malformed option");
					return(false);
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
	buf[j] = CHAR_EOS; /* terminate buf */

	
	if (keyfound == false) {
			/* key name undefined */
			if (display == true)
				errorf_line(pmkfile, cur_line, "Malformed option");
			return(false);
	} else {
		if (strlcpy(tval, buf, MAX_OPT_VALUE_LEN) >= MAX_OPT_VALUE_LEN) {
			/* key value is too long */
			if (display == true)
				errorf_line(pmkfile, cur_line, "Key value is too long");
			return(false);
		} else {
			/* key name and value are ok */
			hash_add(ht, tkey, tval); /* XXX test */
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

bool parse(FILE *fp, pmkdata *gdata) {
	char		buf[MAX_LINE_LEN];
	bool		process = false;
	pmkcmd		cmd;
	htable		*tabopts = NULL;

	while (get_line(fp, buf, sizeof(buf)) == true) {
		/* update current line number */
		cur_line++;

		/* check first character */
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comments */
				/* XXX printf("DEBUG COMMENT = %s\n", buf); */
				break;

			case PMK_CHAR_COMMAND :
				if (process == false) {
					/* parse command and label */
					if (parse_cmd(buf, &cmd) == false) {
						/* line number and error message already set */
						return(false);
					}

					process = true;
					tabopts = hash_init(MAX_CMD_OPT);
					if (tabopts == NULL) {
						errorf_line(pmkfile, cur_line, "Cannot create hash table");
						return(false);
					}
				} else {
					if (strcmp(buf, PMK_END_COMMAND) == 0) {
						/* found end of command */
						process = false;
						if (process_cmd(&cmd, tabopts, gdata) == false) {
							/* command processing failed */
							hash_destroy(tabopts);
							return(false);
						}

						/* cmd processed, clean up */
						strlcpy(cmd.name, "", MAX_CMD_NAME_LEN);
						strlcpy(cmd.label, "", MAX_LABEL_NAME_LEN);
						hash_destroy(tabopts);
					} else {
						/* found another command before end of previous */
						hash_destroy(tabopts);
						errorf_line(pmkfile, cur_line, "%s not found", PMK_END_COMMAND);
						return(false);
					}
				}
				break;

			case CHAR_EOS :
				/* empty line */
				break;

			default :
				if (process == false) {
					errorf_line(pmkfile, cur_line, "Syntax error");
					return(false);
				}

				if (parse_opt(buf, tabopts, true) == false) {
					/* line number and error message already set */
					hash_destroy(tabopts);
					return(false);
				}
				break;
		}
	}

	if (process == true) {
		/* found EOF before end of command */
		hash_destroy(tabopts);
		errorf_line(pmkfile, cur_line, "%s not found", PMK_END_COMMAND);
		return(false);
	}

	if (feof(fp) == 0) {
		/* error occuered before EOF */
		hash_destroy(tabopts);
		errorf_line(pmkfile, cur_line, "end of file not reached.");
		return(false);
	}

	return(true);
}

/*
	process command line values

	val : array of defines
	nbval : size of the array
	ht : storage for parsed values

	returns true on success
*/

bool parse_cmdline(char **val, int nbval, htable *ht) {
	int	i;
	bool	rval = true;

	for (i = 0 ; (i < nbval) && (rval == true) ; i++) {
		/* parse option */
		rval = parse_opt(val[i], ht, false);
	}

	return(rval);
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
	FILE	*fd;
	char	idxstr[4]; /* max 999 cmds, should be enough :) */
	int	rval = 0,
		nbpd,
		nbcd,
		i,
		s,
		chr;
	bool	go_exit = false,
		pmkfile_set = false;
	pmkdata	gdata;
	dynary	*da;

	while (go_exit == false) {
		chr = getopt(argc, argv, "f:hv");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'f' :
					/* filename */
					if (strlcpy(pmkfile, optarg, sizeof(pmkfile)) >= sizeof(pmkfile)) {
						errorf("Cannot use file argument");
						exit(1);
					} else {
						pmkfile_set = true;
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

	/* initialise global data has htable */
	gdata.htab = hash_init(MAX_DATA_KEY);

	if (read_conf(gdata.htab) == true) {
		nbpd = hash_nbkey(gdata.htab);
	} else {
		/* configuration file not found */
		hash_destroy(gdata.htab);
		return(1);
	}

	if (argc != 0) {
		/* parse optional arguments that override pmk.conf */
		if (parse_cmdline(argv, argc, gdata.htab) == true) {
			nbcd = argc;
		} else {
			errorf("incorrect optional arguments");
			return(1);
		}
	} else {
		nbcd = 0;
	}

	/* open pmk file */
	if (pmkfile_set == false) {
		strlcpy(pmkfile, PREMAKE_FILENAME, sizeof(pmkfile)); /* should not fail */
	}
	fd = fopen(pmkfile, "r");
	if (fd == NULL) {
		errorf("while opening %s.", pmkfile);
		exit(1);
	}

	/* open log file */
	if (pmk_log_open(PREMAKE_LOG) == false) {
		errorf("while opening %s.", PREMAKE_LOG);
		exit(1);
	}

	pmk_log("PreMaKe version %s\n\n", PREMAKE_VERSION);
	s = sizeof(functab) / sizeof(cmdkw); /* compute number of keywords */
	keyhash = hash_init(s);
	if (keyhash != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < s ; i++) {
			snprintf(idxstr, 4, "%d", i);
			hash_add(keyhash, functab[i].kw, idxstr); /* XXX test ? */
		}
	}
	/* print number of hashed command */
	pmk_log("Hashed %d pmk keywords.\n", keyhash->count);

	pmk_log("Loaded %d predefinined variables.\n", nbpd);
	pmk_log("Loaded %d overrided variables.\n", nbcd);
	pmk_log("Total : %d variables.\n\n", hash_nbkey(gdata.htab));

	pmk_log("Processing '%s' :\n", pmkfile);
	if (parse(fd, &gdata) == false) {
		/* an error occured while parsing */
		rval = 1;
	} else {
		pmk_log("\nProcess templates :\n");

		da = gdata.tlist;
		for (i = 0 ; (i < da_size(da)) && (rval == 0) ; i++) {
			if (process_template(da_idx(da, i), gdata.htab) == false) {
				/* failure while processing template */
				rval = 1;
			}
		}
		
		da_destroy(da);

		pmk_log("\nEnd of log\n");
	}

	/* flush and close files */
	pmk_log_close();

	fflush(fd);
	fclose(fd);

	/* clear cmd hash */
	if (keyhash != NULL) {
		hash_destroy(keyhash);
	}

	/* clean global data */
	hash_destroy(gdata.htab);

	return(rval);
}
