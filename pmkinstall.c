/* $Id$ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
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
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_stdbool.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "pathtools.h"
#include "pmkinstall.h"
#include "premake.h"

/*#define INST_DEBUG	1*/


extern char	*optarg;
extern int	 optind;
#ifndef errno
extern int	 errno;
#endif

char		*bsfx = DEFAULT_BACKUP_SFX; /* backup extension */


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
		if (snprintf_b(cmd, sizeof(cmd), "%s %s", s_path, file) == false) {
			errorf("failed to build strip command, skipping.");
		} else {
			/* stripping */
			if (system(cmd) != EXIT_SUCCESS) {
				errorf("strip failed.");
			}
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
	process owner string

	pstr: owner string
	puid: storage of uid

	returns: true on success else false
*/

bool process_owner(char *pstr, uid_t *puid) {
	struct passwd	*pp = NULL;
	unsigned long	 ul;

	if (isdigit(*pstr) == 0) {
		/* user name */
		pp = getpwnam(pstr);
		if (pp == NULL) {
			errorf("invalid user name.");
			return(false);
		} else {
			*puid = pp->pw_uid;
		}
	} else {
		/* uid */
		if (str_to_ulong(pstr, 10, &ul) == false) {
			errorf("cannot get numerical value of '%s'.", pstr);
			return(false);
		}
		*puid = (uid_t) ul;
	}

	return(true);
}

/*
	process owner string

	pstr: owner string
	puid: storage of uid

	returns: true on success else false
*/

bool process_group(char *pstr, gid_t *pgid) {
	struct group	*pg = NULL;
	unsigned long	 ul;

	if (isdigit(*pstr) == 0) {
		/* group name */
		pg = getgrnam(pstr);
		if (pg == NULL) {
			errorf("invalid group name.");
			return(false);
		}
		*pgid = pg->gr_gid;
	} else {
		/* gid */
		if (str_to_ulong(pstr, 10, &ul) == false) {
			errorf("cannot get numerical value of '%s'.", pstr);
			return(false);
		}
		*pgid = (gid_t) ul;
	}

	return(true);
}

/*
	create directory

	psrc: source directory
	buffer: destination buffer
	bsize: buffer size

	returns: true on success else false
*/

bool create_directory(char *psrc, char *buffer, size_t bsize) {

#ifdef INST_DEBUG
	debugf("create dir '%s'", psrc);
#endif

	/* create path */
	if (*psrc == CHAR_SEP) {
	/* absolute path, copy */
		if (strlcpy_b(buffer, psrc, bsize) == false) {
			errorf("overflow in directory creation (abs. path).");
			return(false);
		}
	} else {
		/* relative, getting current directory */
		if (getcwd(buffer, bsize) == NULL) {
			errorf("unable to get current directory");
			return(false);
		}
		/* appending path */
		strlcat(buffer, STR_SEP, bsize); /* no check */
		if (strlcat_b(buffer, psrc, bsize) == false) {
			errorf("overflow in directory creation (rel. path).");
			return(false);
		}
	}
#ifdef INST_DEBUG
	debugf("dir = '%s'", buffer);
#endif

	if (makepath(buffer, S_IRWXU | S_IRWXG | S_IRWXO) == false) {
		errorf("cannot create directory.");
		return(false);
	}

	return(true);
}

/*
	build destination target

	psrc: source file
	pdst: destination directory
	buffer: destination buffer
	bsize: buffer size

	returns: true on success else false
*/

bool build_destination(char *psrc, char *pdst, char *buffer, size_t bsize) {
	char	*pstr;

	strlcpy(buffer, pdst, bsize); /* no check */
	if (strlcat_b(buffer, STR_SEP, bsize) == false) {
		errorf("overflow detected in destination.");
		return(false);
	}

	pstr = basename(psrc);
	if (pstr == NULL) {
		errorf("unable to get basename of source.");
		return(false);
	}

	if (strlcat_b(buffer, pstr, bsize) == false) {
		errorf("overflow detected in destination.");
		return(false);
	}

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
	struct stat	 sb;
	bool		 create_dir = false,
			 do_backup = false,
			 do_chown = false,
			 do_strip = false,
			 go_exit = false;
	char		*gstr = NULL,
			*ostr = NULL,
			*src,
			*dst,
			 dir[MAXPATHLEN],
			 backup[MAXPATHLEN];
	gid_t		 gid = (gid_t) -1;
	int		 chr;
	mode_t		 mode = DEFAULT_MODE,
			 tmode;
	uid_t		 uid = (uid_t) -1;
	unsigned int	 src_idx,
			 last_idx;

	while (go_exit == false) {
		chr = getopt(argc, argv, "B:bcdg:hm:o:stv");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'B' :
					bsfx = optarg;
#ifdef INST_DEBUG
					debugf("backup suffix = %s", bsfx);
#endif
					break;

				case 'b' :
					/* backup */
					do_backup = true;
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
		if (process_owner(ostr, &uid) == false) {
			/* error message already done */
			exit(EXIT_FAILURE);
		}
#ifdef INST_DEBUG
		debugf("uid = %d", uid);
#endif
		do_chown = true;
	}

	/* if a group has been provided */
	if (gstr != NULL) {
		if (process_group(gstr, &gid) == false) {
			/* error message already done */
			exit(EXIT_FAILURE);
		}
#ifdef INST_DEBUG
		debugf("gid = %d", gid);
#endif
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
			/* create directory */
			if (create_directory(src, dir, sizeof(dir)) == false) {
				/* error message already done */
				exit(EXIT_FAILURE);
			}

			/* set dst for further operations */
			dst = dir;
		} else {
			/* install file	*/

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
					/* not a directory */
					unlink(dst);
				} else {
					/* build destination */
					if (build_destination(src, dst, dir,
							sizeof(dir)) == false) {
						/* error message already done */
						exit(EXIT_FAILURE);
					}

					/* set dst for further operations */
					dst = dir;
				}
			}


			/* backup, check dst again as it could have been modified */
			if ((do_backup == true) && (stat(dst, &sb) == 0)) {
#ifdef INST_DEBUG
				debugf("backup of '%s'", dst);
#endif
				if (snprintf_b(backup, sizeof(backup),
						"%s%s", dst, bsfx) == false) {
					errorf("failed to generate backup name for '%s': %s.",
							dst, strerror(errno));
					/* XXX exit ? */
				}

				if (rename(dst, backup) != 0) {
					errorf("failed to backup %s: %s.",
							dst, strerror(errno));
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
