/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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
/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "pathtools.h"
#include "pmkinstall.h"
#include "premake.h"

/*#define INST_DEBUG	1*/

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
#ifndef errno
extern int	 errno;
#endif

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
		errorf("%s env variable not set, skipping.", STRIP_ENV_NAME);
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
#ifdef INST_DEBUG
debugf("char = '%c'", *pstr);
#endif
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
#ifdef INST_DEBUG
debugf("mask = %o", mask);
#endif
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
				/* unknown perm */
				errorf("unknown permission '%c'.", *pstr);
				return(false);
				break;
		}
		pstr++;
	}
#ifdef INST_DEBUG
debugf("perm = %o", perm);
#endif

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
	unsigned long	 mode = 0;

	if (mstr == NULL)
		return(false);

	if (isdigit(*mstr) != 0) {
		/* octal value */
		if (str_to_ulong(mstr, 8, &mode) == false) {
			return(false); /* unable to get numerical value */
		}
	} else {
		/* symbolic value */
		if (symbolic_to_octal_mode(mstr, (mode_t *)&mode) == false) {
			return(false);
		};
	}

	/* set mode */
	*pmode = (mode_t) mode;

	return(true);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmkinstall [-cs] [-g group] [-m mode] "
		"[-o owner] file1 file2\n");
	fprintf(stderr, "       pmkinstall [-cs] [-g group] [-m mode] "
		"[-o owner] file1 ... fileN directory\n");
	fprintf(stderr, "       pmkinstall -d [-g group] [-m mode] "
		"[-o owner] directory ...\n");
	fprintf(stderr, "       pmkinstall -v\n");
	fprintf(stderr, "       pmkinstall -h\n");

	exit(EXIT_FAILURE);
}

/*
	main
*/

int main(int argc, char *argv[]) {
	struct group	*pg = NULL;
	struct passwd	*pp = NULL;
	struct stat	 sb;
	bool		 create_dir = false,
			 do_chown = false,
			 do_strip = false,
			 go_exit = false;
	char		*gstr = NULL,
			*ostr = NULL,
			*src,
			*dst,
			*pstr,
			 dir[MAXPATHLEN];
	gid_t		 gid = (gid_t) -1;
	int		 chr;
	mode_t		 mode = DEFAULT_MODE,
			 tmode;
	uid_t		 uid = (uid_t) -1;
	unsigned int	 src_idx,
			 last_idx;
	unsigned long	 ul;
	
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
#ifdef INST_DEBUG
debugf("gstr = %s", gstr);
#endif
					break;

				case 'm' :
					/* specify mode */
					if (check_mode(optarg, &mode) == false)
						exit(EXIT_FAILURE);
#ifdef INST_DEBUG
debugf("mode = %o", mode);
#endif
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

	/* only directory creation allow 1 parameter */
	if ((argc < 2) && (create_dir == false)) {
		/* not enough parameters */
		usage();
	}

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
#ifdef INST_DEBUG
debugf("uid = %d", uid);
#endif
			}
		} else {
			/* uid */
			if (str_to_ulong(ostr, 10, &ul) == false) {
				errorf("cannot get numerical value of '%s'.", ostr);
				exit(EXIT_FAILURE);
			}
			uid = (uid_t) ul;
#ifdef INST_DEBUG
debugf("uid = %d", uid);
#endif
		}
		do_chown = true;
	}

	/* if a group has been provided */
	if (gstr != NULL) {
		if (isdigit(*gstr) == 0) {
			/* group name */
			pg = getgrnam(gstr);
			if (pg == NULL) {
				errorf("invalid group name.");
				exit(EXIT_FAILURE);
			} else {
				gid = pg->gr_gid;
#ifdef INST_DEBUG
debugf("gid = %d", gid);
#endif
			}
		} else {
			/* gid */
			if (str_to_ulong(gstr, 10, &ul) == false) {
				errorf("cannot get numerical value of '%s'.", gstr);
				exit(EXIT_FAILURE);
			}
			gid = (gid_t) ul;
#ifdef INST_DEBUG
debugf("gid = %d", gid);
#endif
		}
		do_chown = true;
	}

	/* set last_idx respectively */
	if (create_dir == true) {
		last_idx = argc;
	} else {
		last_idx = argc - 1;
	}

	/* process each source */
	for (src_idx = 0 ; src_idx < last_idx ; src_idx++) {
		src = argv[src_idx];

		if (create_dir == true) {
			/*
				directory creation
			*/

#ifdef INST_DEBUG
debugf("create dir '%s'", src);
#endif

			/* create path */
			if (*src == CHAR_SEP) {
			/* absolute path, copy */
				if (strlcpy(dir, src, sizeof(dir)) >= sizeof(dir)) {
					errorf("overflow detected in directory creation (abs. path).");
					exit(EXIT_FAILURE);
				}
			} else {
				/* relative, getting current directory */
				if (getcwd(dir, sizeof(dir)) == NULL) {
					errorf("unable to get current directory");
					exit(EXIT_FAILURE);
				}
				/* appending path */
				strlcat(dir, STR_SEP, sizeof(dir));
				if (strlcat(dir, src, sizeof(dir)) >= sizeof(dir)) {
					errorf("overflow detected in directory creation (rel. path).");
					exit(EXIT_FAILURE);
				}
			}
#ifdef INST_DEBUG
debugf("dir = '%s'", dir);
#endif

			if (makepath(dir, S_IRWXU | S_IRWXG | S_IRWXO) == false) {
				errorf("cannot create directory.");
				exit(EXIT_FAILURE);
			}

			/* set dst for further operations */
			dst = dir;
		} else {
			/*
				install file
			*/

#ifdef INST_DEBUG
debugf("process install of '%s'", src);
#endif

			/*  set dst */
			dst = argv[last_idx];

#ifdef INST_DEBUG
debugf("initial dst = '%s'", dst);
#endif

			/* check if destination is a directory */
			if (stat(dst, &sb) == 0) {
				/* many checks to do (is a directory, etc ...) */
				tmode = sb.st_mode & S_IFDIR;
				if (tmode == 0) {
					/* not a directory, XXX backup ? */
					unlink(dst);
				} else {
					strlcpy(dir, dst, sizeof(dir));
					strlcat(dir, STR_SEP, sizeof(dir));

					pstr = basename(src);
					if (pstr == NULL) {
						errorf("unable to get basename of source.");
						exit(EXIT_FAILURE);
					}

					if (strlcat(dir, pstr, sizeof(dir)) >= sizeof(dir)) {
						errorf("overflow detected in destination.");
						exit(EXIT_FAILURE);
					}
					dst = dir;
				}
			}

#ifdef INST_DEBUG
debugf("copy to '%s'", dst);
#endif
			/* copy file */
			if (fcopy(src, dst, mode) == false) {
				/* copy failed, error message already displayed */
				exit(EXIT_FAILURE);
			}

			/* strip binary if asked */
			if (do_strip == true) {
				strip(dst);
			}
		}

		/* change owner and group */
		if (do_chown == true) {
#ifdef INST_DEBUG
debugf("doing chown('%s', %d, %d)", dst, uid, gid);
#endif
			if (chown(dst, uid, gid) != 0) {
				errorf("chown failed : %s.", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		/* change perms (must follow chown that can change perms) */
		if (chmod(dst, mode) == -1) {
#ifdef INST_DEBUG
debugf("chmod('%s', %o)", dst, mode);
#endif
			errorf("chmod failed : %s.", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	return(0);
}
