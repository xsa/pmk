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

#define EMPTY_OPT_VALUE ""

#include <sys/param.h>

#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"


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
		p = (char *)strchr(line, '\n');
		if (p != NULL) {
			/* remove trailing newline */
			*p= CHAR_EOS;
		}
		return(true);
	} else {
		/* reached eof or error */
		return(false);
	}
}

/*
	Parses a given line.
     
	line: line to parse
	linenum: line number
	opts: struct where where we store the key->value data   

	returns:  0 on success
		 -1 on failure
*/

int parse_conf_line(char *line, int linenum, cfg_opt *opts) {
	char	key[MAX_OPT_NAME_LEN],
		value[MAX_OPT_VALUE_LEN],
		c;
	int	i = 0, j = 0, k = 0,
		found_op = 0;


	while ((c = line[i]) != CHAR_EOS) {
		if (found_op == 0) {
			if ((c == CHAR_ASSIGN_UPDATE) || (c == CHAR_ASSIGN_STATIC)) {
				/* operator found, terminate key and copy key name in struct */
				found_op = 1;
				key[j] = CHAR_EOS;
				strlcpy(opts->key, key, MAX_OPT_NAME_LEN); /* XXX test ? */

				/* set operator in struct */
				opts->opchar = c;
			} else {
				key[j] = c;
				j++;
				if (j > MAX_OPT_NAME_LEN) {
					/* too small, cannot store key name */
					errorf_line(PREMAKE_CONFIG_PATH, linenum, "key name too long.");
					return(-1);
				}
			}
		} else {
			value[k] = c;
			k++;
			if (k > MAX_OPT_VALUE_LEN) {
				/* too small, cannot store key value */
				errorf_line(PREMAKE_CONFIG_PATH, linenum, "key value too long.");
				return(-1);
			}
		}
		i++;
	}
	if (found_op == 1) {
		/* line parsed without any error */
		value[k] = CHAR_EOS;
		strlcpy(opts->val, value, MAX_OPT_VALUE_LEN); /* XXX test ? */
		return(0);			
	} else {
		/* missing operator */
		errorf_line(PREMAKE_CONFIG_PATH, linenum, "operator not found.");
		return(-1);
	}
}

/*
	put env variable in an option

	env_name : variable name
	opt : option struct

	returns true if env_name exists
*/

bool env_to_opt(char *env_name, pmkcmdopt *opt) {
	char	*env_val;
	bool	rval;

	env_val = getenv(env_name);
	if (env_val == NULL) {
		/* env variable name not found */
		strlcpy(opt->value, EMPTY_OPT_VALUE, sizeof(EMPTY_OPT_VALUE)); /* XXX test ? */
		rval = false;
	} else {
		/* okay get it */
		strlcpy(opt->value, env_val, sizeof(env_val)); /* XXX test ? */
		rval = true;
		
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
	/* XXX 256 => berk berk berk ! */
	char	mfn[256] = "/tmp/pmk_mkf.XXXXXXXX",
		varstr[256];
	int	mfd = -1;
	bool	rval;

	mfd = mkstemp(mfn);
	if (mfd == -1) {
		/* name randomize failed */
		fprintf(stderr, "Failed to randomize filename\n");
		return(false);
	}

	mfp = fdopen(mfd, "w");
	if (mfp != NULL) {
		/* create a tmp makefile with the following format :
			test:
				@echo ${VARNAME}

		   /!\ should check content of VARNAME, could result
		   	in a security breach. XXX
		*/
		fprintf(mfp, "test:\n\t@echo ${%s}", varname);
		fclose(mfp);
	} else {
		fprintf(stderr, "Failed to open %s\n", mfn);
		return(false);
	}

	snprintf(varstr, 256, "/usr/bin/make -f %s", mfn);
	tfp = popen(varstr, "r");
	if (tfp != NULL) {
		/* catch output of make */
		get_line(tfp, result, rsize);
		fclose(tfp);

		rval = true;
	} else {
		fprintf(stderr, "Failed to exec %s\n", varstr);
		rval = false;
	}

	if (unlink(mfn) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", mfn);
	}

	return(rval);
}

/*
	split a string in a dynamic array

	str : string to split
	sep : separator
	da : dynamic array

	return true on success
*/

bool str_to_dynary(char *str, char sep, dynary *da) {
	char	buf[MAXPATHLEN];
	int	i = 0,
		j = 0;

	while (str[i] != CHAR_EOS) {
		if (str[i] == sep) {
			buf[j] = CHAR_EOS;
			if (da_push(da, buf) == false) {
				return(false);
			}
			j = 0;
		} else {
			buf[j] = str[i];
			j++;
		}
		i++;
	}
	buf[j] = CHAR_EOS;
	if (da_push(da, buf) == false) {
		return(false);
	}

	return(true);
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
	DIR		*dp;
	struct dirent	*de;
	int		i;
	char		*path;
	bool		found,
			exit;

	found = false;
	for (i = 0 ; (i < da_size(da)) && (found == false) ; i++) {
		path = da_idx(da, i);
		dp = opendir(path);
		if (dp == NULL) {
			errorf("could not open directory: %s", path);
			return(false);
		}
		exit = false;
		while (exit == false) {
			de = readdir(dp);
			if (de != NULL) {
				if (strncmp(de->d_name, fname, strlen(fname)) == 0) {
					if (snprintf(fpath, fplen, "%s/%s", path, fname) < fplen) {
						/* fpath set */
						found = true;
					}
					exit = true;
				}
			} else {
				/* no more entry */
				exit = true;
			}
		}
		closedir(dp);
	}

	return(found);
}


/*
	formated simple error

	fmt : format string followed by arguments
*/

void errorf(const char *fmt, ...) {
	va_list	plst;
	char	buf[256];

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
	va_list	plst;
	char	buf[256];

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
	va_list	plst;
	char	buf[256];

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
	va_list	plst;
	char	buf[256];

	va_start(plst, fmt);
	vsnprintf(buf, sizeof(buf), fmt, plst);
	va_end(plst);

	if (pmk_log_fp != NULL) {
		fprintf(pmk_log_fp, buf);
		fprintf(stdout, buf);
		return(true);
	} else {
		errorf("Unable to log.");
		return(false);
	}
}
