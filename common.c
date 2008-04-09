/* $Id$ */

/*
 * Copyright (c) 2003-2007 Damien Couderc
 * Copyright (c) 2003-2004 Xavier Santolaria <xavier@santolaria.net>
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


/*
 * Credits for patches :
 *	- Ted Unangst
 */

#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pwd.h>

#include "compat/pmk_string.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_unistd.h"
#include "common.h"

/*#define	MKVAR_DEBUG	1*/

FILE	*pmk_log_fp;


/**************
 * get_line() *
 ***********************************************************************
 DESCR
	get a line from a file and remove newline character

 IN
	fd : file descriptor
	line : buffer that will contain the line
	lsize : size of the buffer

 OUT
	returns a boolean :
		- true when a line is available
		- false when error or eof occured
 ***********************************************************************/

bool get_line(FILE *fd, char *line, size_t lsize) {
	char	*p;

	if (fgets(line, lsize, fd) != NULL) {
		p = line;
		while (*p != CHAR_EOS) {
			if (*p == '\n') {
				/* remove trailing newline */
				*p= CHAR_EOS;
			}
			p++;
		}
		return(true);
	} else {
		/* reached eof or error */
		return(false);
	}
}


/****************
 * env_to_opt() *
 ***********************************************************************
 DESCR
	put env variable in an option

 IN
	env_name :	variable name
	opt :		option struct

 OUT
	returns true if env_name exists
 ***********************************************************************/

bool env_to_opt(char *env_name, pmkcmdopt *opt) {
	bool	 rval;
	char	*env_val;

	env_val = getenv(env_name);
	if (env_val == NULL) {
		/* env variable name not found */
		rval = false;
	} else {
		/* okay get it */
		if (strlcpy_b(opt->value, env_val,
					sizeof(opt->value)) == false) {
			rval = true;
		} else {
			rval = false;
		}

	}
	return(rval);
}


/******************
 * get_make_var() *
 ***********************************************************************
 DESCR
	get variable content from make

 IN
	varname:	name of the variable like for example CC
	result:		storage of result
	rsize:		result size

 OUT
	boolean
 ***********************************************************************/

bool get_make_var(char *varname, char *result, size_t rsize) {
	FILE	*mfp,
			*tfp;
	bool	 rval;
	char	 mfn[MAXPATHLEN],
			 varstr[TMP_BUF_LEN];

	if (strlcpy_b(mfn, TMP_MK_FILE, sizeof(mfn)) == false) {
		errorf("failed to create temporary file");
		return(false);
	}

	mfp = tmps_open(TMP_MK_FILE, "w", mfn, sizeof(mfn), strlen(MK_FILE_EXT));
	if (mfp != NULL) {
		/* create a tmp makefile with the following format :
			all:
				@printf ${VARNAME}

		   /!\ should check content of VARNAME, could result
		   	in a security breach.
		*/
		fprintf(mfp, MKVAR_FMT_MK, varname, MKVAR_FILE);
		fclose(mfp);
#ifdef MKVAR_DEBUG
debugf(MKVAR_FMT_MK, varname, MKVAR_FILE);
#endif
	} else {
#ifdef MKVAR_DEBUG
		errorf("failed to open '%s' : %s.", mfn, strerror(errno));
#endif
		return(false);
	}

	if (snprintf_b(varstr, sizeof(varstr), MKVAR_FMT_CMD, mfn) == false) {
		return(false);
	}

	if (system(varstr) == 0) {
		tfp = fopen(MKVAR_FILE, "r");
		if (tfp != NULL) {
			/* catch output of make */
			if (get_line(tfp, result, rsize) == false) {
#ifdef MKVAR_DEBUG
				errorf("failed to get result from '%s' : %s.", varstr, strerror(errno));
#endif
				rval = false;
			}
			fclose(tfp);

			rval = true;
		} else {
#ifdef MKVAR_DEBUG
			errorf("failed to open '%s' : %s.", varstr, strerror(errno));
#endif
			rval = false;
		}
	} else {
#ifdef MKVAR_DEBUG
		errorf("failed to execute '%s' : %s.", varstr, strerror(errno));
#endif
		rval = false;
	}


	if (unlink(mfn) == -1) {
		/* cannot remove temporary file */
		errorf("cannot remove temporary file: '%s' : %s.", mfn, strerror(errno));
	}

	if (unlink(MKVAR_FILE) == -1) {
		/* cannot remove temporary file */
#ifdef MKVAR_DEBUG
		errorf("cannot remove temporary file: '%s' : %s.", mfn, strerror(errno));
#endif
	}

	return(rval);
}


/******************
 * str_to_ulong() *
 ***********************************************************************
 DESCR
	convert a string into an unsigned long

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool str_to_ulong(char *str, int base, unsigned long *value) {
	char	*ep;

	*value = strtoul(str, &ep, base);
	if (*str == '\0' || *ep != '\0') {
		return(false); /* not a number */
	}

	if ((errno == ERANGE) && (*value == ULONG_MAX)) {
		return(false); /* out of range */
	}

	return(true);
}


/*******************
 * str_to_dynary() *
 ***********************************************************************
 DESCR
	split a string into a dynamic array (one separator)

 IN
	str :	string to split
	sep :	separator

 OUT
	dynary or NULL

 NOTE
	move to dynary.c ?
 ***********************************************************************/

dynary *str_to_dynary(char *str, char sep) {
	static char	buf[2];

	snprintf(buf, sizeof(buf), "%c", sep); /* should not overflow */
	return(str_to_dynary_adv(str, buf));
}


/***********************
 * str_to_dynary_adv() *
 ***********************************************************************
 DESCR
	split a string into a dynamic array (list of separators)

 IN
	str :	string to split
	sep :	separators list

 OUT
	dynary or NULL

 NOTE
	move to dynary.c ?
 ***********************************************************************/

dynary *str_to_dynary_adv(char *str, char *seplst) {
	char	 buf[MAXPATHLEN],
			*pbuf;
	dynary	*da;
	int		 s;

	if (str == NULL) {
		/* protect against NULL */
		return(NULL);
	}

	/* init dynary */
	da = da_init();
	if (da == NULL) {
		return(NULL);
	}

	s = sizeof(buf);
	pbuf = buf;

	while (*str != CHAR_EOS) {
		/* check if character is in separator list */
		if (strchr(seplst, *str) != NULL) {
			*pbuf = CHAR_EOS;
			if (da_push(da, strdup(buf)) == false) {
				da_destroy(da);
				return(NULL);
			}
			pbuf = buf;
			s = sizeof(buf);
		} else {
			*pbuf = *str;
			pbuf++;
			s--;
			if (s == 0) {
				/* not enough space in buffer */
				da_destroy(da);
				return(NULL);
			}
		}
		str++;
	}

	*pbuf = CHAR_EOS;
	if (da_push(da, strdup(buf)) == false) {
		da_destroy(da);
		return(NULL);
	}

	return(da);
}


/*******************
 * find_file_dir() *
 ***********************************************************************
 DESCR
	find if one dir contain a given file in the specified list of path

 IN
	da :	path list
	fname :	file to search
	fpath :	storage of the directory
	fplen :	size of the storage

 OUT
	boolean
 ***********************************************************************/

bool find_file_dir(dynary *da, char *fname, char *fpath, size_t fplen) {
	char			 tstr[MAXPATHLEN],
					*path;
	unsigned int	 i;

	for (i = 0 ; i < da_usize(da) ; i++) {
		path = da_idx(da, i);

		strlcpy(tstr, path, sizeof(tstr)); /* no check */
		strlcat(tstr, "/", sizeof(tstr)); /* no check */
		if (strlcat_b(tstr, fname, sizeof(tstr)) == true) {
			if (access(tstr, F_OK) == 0) { /* no race condition here */
				if (strlcpy_b(fpath, path, fplen) == true) {
					/* fpath correctly set */
					return(true);
				} else {
					errorf("strlcpy failed in find_file_dir().");
					return(false);
				}
			}
		}

	}

	/* not found */
	return(false);
}


/***************
 * find_file() *
 ***********************************************************************
 DESCR
	find a file in the specified list of path

 IN
	da :	path list
	fname :	file to search
	fpath :	storage of the full path
	fplen :	size of the storage

 OUT
	boolean
 ***********************************************************************/

bool find_file(dynary *da, char *fname, char *fpath, size_t fplen) {
	bool	rval = false;

	if (find_file_dir(da, fname, fpath, fplen) == true) {
		strlcat(fpath, "/", fplen); /* no check */
		if (strlcat_b(fpath, fname, fplen) == true) {
			/* fpath set correctly */
			rval = true;
		}
	}
	return(rval);
}


/*******************
 * get_file_path() *
 ***********************************************************************
 DESCR
	get path of given filename in given path list

 IN
	filename :	name of the file to look for
	path :		string of the list of path
	storage :	storage for resulting path
	size :		size of storage

 OUT
	returns true if filename is found in one of the list's path
 ***********************************************************************/

bool get_file_path(char *filename, char *path, char *storage, size_t size) {
	bool	 rval = false;
	dynary	*bplst;

	/* fill dynary with path */
	bplst= str_to_dynary(path, PATH_STR_DELIMITER);
	if (bplst == NULL) {
		errorf("failed to put a path into a dynamic array.");
		return(false);
	}

	/* try to locate binary */
	if (find_file(bplst, filename, storage, size) == true) {
		rval = true;
	} else {
		rval = false;
	}

	da_destroy(bplst);
	return(rval);
}


/************
 * errorf() *
 ***********************************************************************
 DESCR
	formated simple error

 IN
	fmt :	format string followed by arguments

 OUT
	NONE
 ***********************************************************************/

void errorf(const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	fprintf(stderr, "Error : %s\n", buf);
}


/*****************
 * errorf_line() *
 ***********************************************************************
 DESCR
	formated parse error

 IN
	filename :	parsed file name
	line :		line of error
	fmt :		format string followed by arguments

 OUT
	NONE
 ***********************************************************************/

void errorf_line(char *filename, int line, const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	fprintf(stderr, "Error in '%s' line %d : %s\n", filename, line, buf);
}


/************
 * debugf() *
 ***********************************************************************
 DESCR
	formated debug message

 IN
	fmt :	format string followed by arguments

 OUT
	NONE
 ***********************************************************************/

void debugf(const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	fprintf(stdout, "!DEBUG! %s\n", buf);
	fflush(stdout);
}


/******************
 * pmk_log_open() *
 ***********************************************************************
 DESCR
	open log file

 IN
	logname :	log file name

 OUT
	returns true if opened
 ***********************************************************************/

bool pmk_log_open(char *logname) {
	if (pmk_log_fp != NULL) {
		errorf("'%s' is already open.", logname);
		return(false);
	}
	pmk_log_fp = fopen(logname, "w");
	if (pmk_log_fp == NULL) {
		errorf("while opening '%s'.", logname);
		return(false);
	} else {
		return(true);
	}
}


/*******************
 * pmk_log_close() *
 ***********************************************************************
 DESCR
	close log file

 IN
	NONE

 OUT
	NONE
 ***********************************************************************/

void pmk_log_close(void) {
	fflush(pmk_log_fp);
	fclose(pmk_log_fp);
}


/*************
 * pmk_log() *
 ***********************************************************************
 DESCR
	log formatted line

 IN
	fmt :	format string

 OUT
	boolean
 ***********************************************************************/

bool pmk_log(const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	if (pmk_log_fp != NULL) {
		fprintf(pmk_log_fp, buf);
		fflush(pmk_log_fp);
		fprintf(stdout, buf);
		fflush(stdout);
		return(true);
	} else {
		errorf("unable to log.");
		return(false);
	}
}


/***********
 * fcopy() *
 ***********************************************************************
 DESCR
	copy file

 IN
	src :	file to copy
	dst :	destination file
	mode :	destination perms

 OUT
	boolean
 ***********************************************************************/

bool fcopy(char *src, char *dst, mode_t mode) {
	char		cbuf[S_BLKSIZE];
	bool		do_loop = true,
				rval = true;
	int			src_fd,
				dst_fd;
	ssize_t		rsz;

	/* try to open both source and destination files */
	src_fd = open(src, O_RDONLY, 0);
	if (src_fd == -1) {
		errorf("cannot open '%s' : %s.", src, strerror(errno));
		return(false);
	}
/*debugf("mode = %o", mode);*/
	dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (dst_fd == -1) {
		close(src_fd);
		errorf("cannot open '%s' : %s.", dst, strerror(errno));
		return(false);
	}

	while (do_loop == true) {
		/* reading data */
		rsz = read(src_fd, cbuf, sizeof(cbuf));
		switch(rsz) {
			case -1:
				/* read error */
				errorf("failed to read '%s' : %s.", src, strerror(errno));
				do_loop = false;
				rval = false;
				break;

			case 0:
				/* no more data to copy */
				do_loop = false;
				break;

			default:
				/* data read, trying to write */
				if (write(dst_fd, cbuf, rsz) != rsz) {
					/* write failed */
					errorf("failed to write '%s' : %s.", dst, strerror(errno));
					do_loop = false;
					rval = false;
				}
				break;
		}
	}

	close(src_fd);
	close(dst_fd);

	return(rval);
}


/**************
 * tmp_open() *
 ***********************************************************************
 DESCR
	open temporary file

 IN
	tfile :	template file name
	mode :	file mode
	buf :	buffer for the randomized file name
	bsize :	buffer size

 OUT
	file structure or NULL
 ***********************************************************************/

FILE *tmp_open(char *tfile, char *mode, char *buf, size_t bsize) {
	int	fd;

	/* copy file name in buf */
	if (strlcpy_b(buf, tfile, bsize) == false)
		return(NULL);

	/* randomize file name */
	fd = mkstemp(buf);
	if (fd == -1) {
		return(NULL);
	}

	return(fdopen(fd, mode));
}


/**************
 * tmps_open() *
 ***********************************************************************
 DESCR
	open temporary file with suffix

 IN
	tfile :	template file name
	mode :	file mode
	buf :	buffer for the randomized file name
	bsize :	buffer size
	slen :	suffix length

 OUT
	file structure or NULL
 ***********************************************************************/

FILE *tmps_open(char *tfile, char *mode, char *buf, size_t bsize, size_t slen) {
	int	fd;

	/* copy file name in buf */
	if (strlcpy_b(buf, tfile, bsize) == false)
		return(NULL);

	/* randomize file name */
	fd = mkstemps(buf, slen);
	if (fd == -1) {
		return(NULL);
	}

	return(fdopen(fd, mode));
}

/*************
 * get_home() *
 ***********************************************************************
 DESCR
	get the home directory path

 IN
	NONE

 OUT
	path or NULL
 ***********************************************************************/

char *get_home(void) {
	struct passwd	*ppw;

	ppw = getpwuid(geteuid());
	if (ppw == NULL) {
#ifdef DEBUG_COMMON
		debugf("failed to get passwd entry");
#endif
		return NULL;
	}

	return ppw->pw_dir;
}

/**********************
 * get_pmk_conf_path() *
 ***********************************************************************
 DESCR
	get the path to pmk.conf by looking first in ~/.pmk/, else in SYSCONFDIR

 IN
	NONE

 OUT
	Configuration file path or NULL
 ***********************************************************************/

bool get_pmk_conf_path(char *buffer, size_t bsize) {
	char	*home;

	home = get_home();
	if (home == NULL) {
		return false;
	}

	if (snprintf_b(buffer, bsize, PMK_CFG_HOME_FMT, home) == false) {
#ifdef DEBUG_COMMON
		debugf("failed to get build home path for pmk config file");
#endif
		return false;
	}

	/*
	 * check if the config file exists in the home directory
	 * NOTE: no race condition here for access(), BUT
	 * BE CAREFUL if changes are to be made.
	 */
	if (access(buffer, F_OK) != 0) { /* see above */
		/* the file doesn't exists in HOME */
#ifdef DEBUG_COMMON
		debugf("failed to access '%s'", buffer);
#endif

		if (snprintf_b(buffer, bsize, PMK_CFG_SYS_FMT) == false) {
#ifdef DEBUG_COMMON
			debugf("failed to get build system path for pmk config file");
#endif
			return false;
		}

		if (access(buffer, F_OK) != 0) { /* see above */
			/* the file doesn't exists in SYSCONFDIR */
#ifdef DEBUG_COMMON
			debugf("failed to access '%s'", buffer);
#endif
			return false;
		}
	}

	/* pmk.conf has been found */
	return true;
}


/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
