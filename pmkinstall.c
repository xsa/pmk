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
 *    - Neither the name of the copyright holder(s) nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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


#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "compat/pmk_sys_types.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "pathtools.h"
#include "pmkinstall.h"
#include "premake.h"


/* default mode */
#define DEFAULT_MODE	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

/* mode masks */
#define USR_MASK	S_IRWXU | S_ISUID		/* user */
#define GRP_MASK	S_IRWXG	| S_ISGID		/* group */
#define OTH_MASK	S_IRWXO				/* other */
#define FULL_MASK	USR_MASK | GRP_MASK | OTH_MASK	/* all */

/* perm masks */
#define R_PERM		S_IRUSR | S_IRGRP | S_IROTH		/* read */
#define W_PERM		S_IWUSR | S_IWGRP | S_IWOTH		/* write */
#define X_PERM		S_IXUSR | S_IXGRP | S_IXOTH		/* execute */
#define S_PERM		S_ISUID | S_ISGID			/* user/group ids */
#define FULL_PERM	R_PERM | W_PERM | X_PERM | S_PERM	/* all */


extern char	*optarg;
extern int	 optind;
extern int	 errno;


/*
	strip

	file : file to strip

	return : nothing at the moment :)

	NOTE: could use BIN_STRIP from pmk.conf
*/

void strip(char *file) {
	char	*s_path,
		 cmd[MAXPATHLEN];

	s_path = getenv(STRIP_ENV_NAME);
	if (s_path == NULL) {
		errorf("STRIP env variable not set, skipping.");
	} else {
		/* build the command */
		snprintf(cmd, sizeof(cmd), "%s %s", s_path, file);

		/* stripping */
		if (system(cmd) != EXIT_SUCCESS) {
			errorf("strip failed.");
		}
	}
}

/*
	convert symbolic value to octal

	mstr : symbolic value string
	pmode : resulting mode

	return : boolean

*/

bool symbolic_to_octal_mode(char *mstr, mode_t *pmode) {
	bool	 do_loop = true;
	char	*pstr,
		 op = CHAR_EOS;
	mode_t	 mask = 0,
		 perm = 0;

	pstr = mstr;

	/* who symbols */
	while (do_loop == true) {
/*debugf("char = '%c'", *pstr);*/
		switch (*pstr) {
			case 'a':
				mask = mask | FULL_MASK;
				break;

			case 'g':
				mask = mask | GRP_MASK;
				break;

			case 'o':
				mask = mask | OTH_MASK;
				break;

			case 'u':
				mask = mask | USR_MASK;
				break;

			default:
				/* separator found */
				op = *pstr;
				do_loop = false;
				break;
		}
		pstr++;
/*debugf("mask = %o", mask);*/
	}

	/* check if operator is valid */
	if ((op != '+') && (op != '-') && (op != '=')) {
		errorf("syntax error in symbolic mode '%s'", mstr);
		return(false);
	}

	/* perm symbols */
	do_loop = true;
	while (do_loop == true) {
		switch (*pstr) {
			case 'r':
				perm = perm | R_PERM;
				break;

			case 'w':
				perm = perm | W_PERM;
				break;

			case 'x':
				perm = perm | X_PERM;
				break;

			case 's':
				perm = perm | S_PERM;
				break;

			case CHAR_EOS:
				/* reached end of string */
				do_loop = false;
				break;

			default:
				/* unknow perm */
				errorf("unknow permission '%c'.", *pstr);
				return(false);
				break;
		}
		pstr++;
	}
/*debugf("perm = %o", perm);*/

	/* apply operator */
	switch (op) {
		case '+':
			if (perm == 0) {
				/* no perms given */
				*pmode = 0;
			} else {
				if (mask == 0) {
					/* mask not set, give perms to all */
					mask = FULL_MASK;
				}
				*pmode = mask & perm;
			}
			break;

		case '-':
			/* no perms */
			*pmode = 0;
			break;

		case '=':
			if (mask == 0) {
				*pmode = perm;
			} else {
				*pmode = 0;
			}
			break;
	}

	return(true);
}

/*
	check_mode

	mstr : mode string
	pmode : resulting mode

	return : boolean 

*/

bool check_mode(char *mstr, mode_t *pmode) {
	long	 mode = 0;
	mode_t	 mask;
	char	*ep;

	if (mstr == NULL)
		return(false);

	if (isdigit(*mstr) != 0) {
		/* octal value */
		mode = strtol(mstr, NULL, 8);
		if (*mstr == '\0' || *ep != '\0')
			return(false); /* not a number */
		if (errno == ERANGE && (mode == LONG_MIN || mode == LONG_MAX))
			return(false); /* invalid number */
		if (mode < 0 || mode > USHRT_MAX)
			return(false); /* invalid mode */
	} else {
		/* symbolic value */
		if (symbolic_to_octal_mode(mstr, (mode_t *)&mode) == false) {
			return(false);
		};
	}

	/* get umask */
	mask = umask(0);
	/* set original umask again */
	umask(mask);

	/* apply umask */
	*pmode = ((mode_t)mode) & (~mask);

	return(true);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmkinstall [-bcdghmostv] [path]\n");
	/* XXX to finish */
	exit(EXIT_FAILURE);
}

/*
	main
*/

int main(int argc, char *argv[]) {
	struct group	*pg = NULL;
	struct passwd	*pp = NULL;
	bool		 create_dir = false,
			 do_chown = false,
			 do_strip = false,
			 go_exit = false;
	char		*gstr = NULL,
			*ostr = NULL,
			*src,
			*dst,
			 dir[MAXPATHLEN];
	gid_t		 gid = (gid_t) -1;
	int		 chr;
	mode_t		 mode = DEFAULT_MODE;
	uid_t		 uid = (uid_t) -1;
	
	while (go_exit == false) {
		chr = getopt(argc, argv, "bcdg:hm:o:stv");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'b' :
					/* backup */
					/* XXX TODO */
					errorf("-b option has not been implemented yet");
					break;

				case 'c' :
					/* default behavior, do nothing (backwards compat.) */
					break;

				case 'd' :
					/* create directories */
					create_dir = true;
					break;

				case 'g' :
					/* specify group */
					gstr = optarg;
/*debugf("gstr = %s", gstr);*/
					break;

				case 'm' :
					/* specify mode */
					if (check_mode(optarg, &mode) == false)
						exit(EXIT_FAILURE);
/*debugf("mode = %o", mode);*/
					break;

				case 'o' :
					/* specifiy owner */
					ostr = optarg;
					break;

				case 's' :
					/* strip */
					do_strip = true;
					break;

				case 't' :
					/* autconf shit, only for compat */
					break;

				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(EXIT_SUCCESS);
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					break;
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

	if ((argc != 2) && (create_dir == false)) {
		/* XXX TODO  allow use of more than one source (<2)*/
		usage();
	}

	src = argv[0];
	dst = argv[1];

	/* check if an owner has been provided */
	if (ostr != NULL) {
		if (isdigit(*ostr) == 0) {
			/* user name */
			pp = getpwnam(ostr);
			if (pp == NULL) {
				errorf("invalid user name.");
				exit(EXIT_FAILURE);
			} else {
				uid = pp->pw_uid;
/*debugf("uid = %d", uid);*/
			}
		} else {
			/* uid */
			uid = (uid_t) strtoul(ostr, NULL, 10);
			/* XXX check for ERANGE ? */
/*debugf("uid = %d", uid);*/
		}
		do_chown = true;
	}

	if (gstr != NULL) {
		if (isdigit(*gstr) == 0) {
			/* group name */
			pg = getgrnam(gstr);
			if (pg == NULL) {
				errorf("invalid group name.");
				exit(EXIT_FAILURE);
			} else {
				gid = pg->gr_gid;
/*debugf("gid = %d", gid);*/
			}
		} else {
			/* gid */
			gid = (gid_t) strtoul(gstr, NULL, 10);
			/* XXX check for ERANGE ? */
/*debugf("gid = %d", gid);*/
		}
		do_chown = true;
	}

	if (create_dir == false) {
		/* copy file */
		if (fcopy(src, dst, mode) == false) {
			/* copy failed */
			exit(EXIT_FAILURE);
		}
	} else {
		/* create path */
/*debugf("create dir '%s'", src);*/
		if (*src == CHAR_SEP) {
			/* absolute path, copy */
			strlcpy(dir, src, sizeof(dir)); /* XXX check */
		} else {
			/* relative, getting current directory */
			if (getcwd(dir, sizeof(dir)) == NULL) {
				errorf("Unable to get current directory");
				exit(EXIT_FAILURE);
			}
			/* appending path */
			strlcat(dir, STR_SEP, sizeof(dir));
			strlcat(dir, src, sizeof(dir)); /* XXX check */
		}
/*debugf("dir = '%s'", dir);*/

		if (makepath(dir, S_IRWXU | S_IRWXG | S_IRWXO) == false) {
			errorf("cannot create directory.");
			exit(EXIT_FAILURE);
		}

		/* set dst for further operations */
		dst = dir;
	}

	if (do_strip == true) {
		strip(dst);
	}

	/* change owner and group */
	if (do_chown == true) {
/*debugf("doing chown('%s', %d, %d)", dst, uid, gid);*/
		if (chown(dst, uid, gid) != 0) {
			errorf("chown failed : %s.", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/* change perms (must follow chown that can change perms) */
	if (chmod(dst, mode) == -1) {
/*debugf("chmod('%s', %o)", dst, mode);*/
		errorf("chmod failed : %s.", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return(0);
}
