/* $Id$ */

/*
 * Credits for patches :
 *	- Ted Unangst
 */

/*
 * Copyright (c) 2003 Damien Couderc
 * Copyright (c) 2003 Xavier Santolaria
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


#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "dynarray.h"
#include "premake.h"

/*#define	MKVAR_DEBUG	1*/


/*
	get a line from a file and remove newline character

	fd : file descriptor
	line : buffer that will contain the line
	lsize : size of the buffer

	returns a boolean :
		- true when a line is available
		- false when error or eof occured
*/

bool get_line(FILE *fd, char *line, int lsize) {
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

/*
	put env variable in an option

	env_name : variable name
	opt : option struct

	returns true if env_name exists
*/

bool env_to_opt(char *env_name, pmkcmdopt *opt) {
	bool	 rval;
	char	*env_val;
	int	 s;

	env_val = getenv(env_name);
	if (env_val == NULL) {
		/* env variable name not found */
		rval = false;
	} else {
		/* okay get it */
		s = sizeof(opt->value);
		if (strlcpy(opt->value, env_val, s) < s) {
			rval = true;
		} else {
			rval = false;
		}
		
	}
	return(rval);
}

/*
	get variable content from make

	varname: name of the variable like for example CC
	result: storage of result
	rsize: result size

	return true on success
*/

bool get_make_var(char *varname, char *result, int rsize) {
	FILE	*mfp,
		*tfp;
	bool	 rval;
	char	 mfn[MAXPATHLEN],
		 varstr[TMP_BUF_LEN];
	strlcpy(mfn, TMP_MK_FILE, sizeof(mfn));

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
		errorf("Failed to open %s : %s.", mfn, strerror(errno));
#endif
		return(false);
	}

	snprintf(varstr, sizeof(varstr), MKVAR_FMT_CMD, mfn);
	if (system(varstr) == 0) {
		tfp = fopen(MKVAR_FILE, "r");
		if (tfp != NULL) {
			/* catch output of make */
			if (get_line(tfp, result, rsize) == false) {
#ifdef MKVAR_DEBUG
				errorf("failed to get result from %s : %s.", varstr, strerror(errno));
#endif
				rval = false;
			}
			fclose(tfp);

			rval = true;
		} else {
#ifdef MKVAR_DEBUG
			errorf("failed to open %s : %s.", varstr, strerror(errno));
#endif
			rval = false;
		}
	} else {
#ifdef MKVAR_DEBUG
		errorf("failed to execute %s : %s.", varstr, strerror(errno));
#endif
		rval = false;
	}


	if (unlink(mfn) == -1) {
		/* cannot remove temporary file */
		errorf("Cannot remove temporary file: %s : %s", 
			mfn, strerror(errno));
	}

	if (unlink(MKVAR_FILE) == -1) {
		/* cannot remove temporary file */
#ifdef MKVAR_DEBUG
		errorf("Cannot remove temporary file: %s : %s", 
			mfn, strerror(errno));
#endif
	}

	return(rval);
}

/*
	split a string into a dynamic array (one separator)

	str : string to split
	sep : separator

	return : dynary or NULL 
*/

dynary *str_to_dynary(char *str, char sep) {
	static char	buf[2];

	snprintf(buf, sizeof(buf), "%c", sep); /* XXX check ! */
	return(str_to_dynary_adv(str, buf));
}

/*
	split a string into a dynamic array (list of separators)

	str : string to split
	sep : separators list

	return : dynary or NULL 
*/

dynary *str_to_dynary_adv(char *str, char *seplst) {
	char	 buf[MAXPATHLEN],
		*pbuf;
	dynary	*da;
	int	 s;

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

/*
	find if one dir contain a given file in the specified list of path

	da : path list
	fname : file to search
	fpath : storage of the directory
	fplen : size of the storage

	returns true on success
*/

bool find_file_dir(dynary *da, char *fname, char *fpath, int fplen) {
	FILE		*fp;
	bool		 found = false;
	char		 tstr[MAXPATHLEN],
			*path;
	unsigned int	 i;

	for (i = 0 ; (i < da_usize(da)) && (found == false) ; i++) {
		path = da_idx(da, i);

		strlcpy(tstr, path, sizeof(tstr));
		strlcat(tstr, "/", sizeof(tstr));
		if (strlcat(tstr, fname, sizeof(tstr)) < sizeof(tstr)) {
			fp = fopen(tstr, "r");
			if (fp != NULL) {
				fclose(fp);
				if (strlcpy(fpath, path, fplen) < fplen) {
					/* fpath correctly set */
					found = true; /* XXX OPTIM return(true) ? */
				}
			}
		}

	}

	return(found); /* XXX OPTIM return(false); ? */
}

/*
	find a file in the specified list of path

	da : path list
	fname : file to search
	fpath : storage of the full path
	fplen : size of the storage

	returns true on success
*/

bool find_file(dynary *da, char *fname, char *fpath, int fplen) {
	bool	rval = false;

	if (find_file_dir(da, fname, fpath, fplen) == true) {
		strlcat(fpath, "/", fplen); /* no need to check here */
		if (strlcat(fpath, fname, fplen) < fplen) {
			/* fpath set correctly */
			rval = true;
		}
	}
	return(rval);
}


/*
	formated simple error

	fmt : format string followed by arguments
*/

void errorf(const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	fprintf(stderr, "Error : %s\n", buf);
}

/*
	formated parse error

	filename : parsed file name
	line : line of error
	fmt : format string followed by arguments
*/

void errorf_line(char *filename, int line, const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	fprintf(stderr, "Error in '%s' line %d : %s\n", filename, line, buf);
}

/*
	formated debug message

	fmt : format string followed by arguments
*/

void debugf(const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	fprintf(stdout, "!DEBUG! %s\n", buf);
}

/*
	open log file

	logname : log file name

	returns true if opened
*/

bool pmk_log_open(char *logname) {
	if (pmk_log_fp != NULL) {
		errorf("%s already open.", logname);
		return(false);
	}
	pmk_log_fp = fopen(logname, "w");
	if (pmk_log_fp == NULL) {
		errorf("while opening %s.", logname);
		return(false);
	} else {
		return(true);
	}
}

/*
	close log file
*/

void pmk_log_close(void) {
	fflush(pmk_log_fp);
	fclose(pmk_log_fp);
}

/*
	log formatted line

	fmt : format string
*/

bool pmk_log(const char *fmt, ...) {
	char	buf[TMP_BUF_LEN];
	va_list	plst;

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	if (pmk_log_fp != NULL) {
		fprintf(pmk_log_fp, buf);
		fprintf(stdout, buf);
		return(true);
	} else {
		errorf("unable to log.");
		return(false);
	}
}

/*
	copy a text file

	src_file : file to copy
	dst_file : filename of the copy

	returns true on success
*/

bool copy_text_file(char *src_file, char *dst_file) {
	FILE	*fp_src,
		*fp_dst;
	bool	 rval;
	char	 buf[TMP_BUF_LEN];

	fp_src = fopen(src_file, "r");
	if (fp_src == NULL) {
		return(false);
	}

	fp_dst = fopen(dst_file, "w");
	if (fp_dst == NULL) {
		fclose(fp_src);
		return(false);
	}

	while (get_line(fp_src, buf, sizeof(buf)) == 1) {
		fprintf(fp_dst, "%s\n", buf); /* should test fprintf */
	}

	fclose(fp_dst);

	if (feof(fp_src) == 0) {
		rval = false;
		/* copy failed to achieve, erase destination file */
		unlink(dst_file);
	} else {
		rval = true;
	}

	fclose(fp_src);

	return(rval);
}

/*
	copy file

	src : file to copy
	dst : destination file
	mode : destination perms

	return : boolean
*/

bool fcopy(char *src, char *dst, mode_t mode) {
	char		cbuf[S_BLKSIZE];
	bool		do_loop = true,
			rval = true;
	int		src_fd,
			dst_fd;
	ssize_t		rsz;

	/* try to open both source and destination files */
	src_fd = open(src, O_RDONLY, 0);
	if (src_fd == -1) {
		errorf("cannot open %s : %s.", src, strerror(errno));
		return(false);
	}
/*debugf("mode = %o", mode);*/
	dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (dst_fd == -1) {
		close(src_fd);
		errorf("cannot open %s : %s.", dst, strerror(errno));
		return(false);
	}

	while (do_loop == true) {
		/* reading data */
		rsz = read(src_fd, cbuf, sizeof(cbuf));
		switch(rsz) {
			case -1:
				/* read error */
				errorf("failed to read %s : %s.", src, strerror(errno));
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
					errorf("failed to write %s : %s.", dst, strerror(errno));
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

/*
	open temporary file
	
	tfile : template file name
	mode : file mode
	buf : buffer for the randomized file name
	bsize : buffer size

	return : file structure or NULL
*/

FILE *tmp_open(char *tfile, char *mode, char *buf, size_t bsize) {
	int	fd;

	/* copy file name in buf */
	strlcpy(buf, tfile, bsize);

	/* randomize file name */
	fd = mkstemp(buf);
	if (fd == -1) {
		return(NULL);
	}

	return(fdopen(fd, mode));
}

/*
	open temporary file with suffix
	
	tfile : template file name
	mode : file mode
	buf : buffer for the randomized file name
	bsize : buffer size
	slen : suffix length

	return : file structure or NULL
*/

FILE *tmps_open(char *tfile, char *mode, char *buf, size_t bsize, int slen) {
	int	fd;

	/* copy file name in buf */
	strlcpy(buf, tfile, bsize);

	/* randomize file name */
	fd = mkstemps(buf, slen);
	if (fd == -1) {
		return(NULL);
	}

	return(fdopen(fd, mode));
}

