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
#include <string.h>
#include <unistd.h>

#include "common.h"


/*
	get a line from a file and remove newline character

	fd : file descriptor
	line : buffer that will contain the line
	lsize : size of the buffer

	returns a boolean :
		- TRUE when a line is available
		- FALSE when error or eof occured
*/

bool get_line(FILE *fd, char *line, int lsize) {
	char	*p;

	if (fgets(line, lsize, fd) != NULL) {
		p = (char *)strchr(line, '\n');
		if (p != NULL) {
			/* remove trailing newline */
			*p= '\0';
		}
		return(TRUE);
	} else {
		/* reached eof or error */
		return(FALSE);
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


	while ((c = line[i]) != '\0') {
		if (found_op == 0) {
			if ((c == CHAR_ASSIGN_UPDATE) || (c == CHAR_ASSIGN_STATIC)) {
				/* operator found, terminate key and copy key name in struct */
				found_op = 1;
				key[j] = '\0';
				strncpy(opts->key, key, MAX_OPT_NAME_LEN);

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
		value[k] = '\0';
		strncpy(opts->val, value, MAX_OPT_VALUE_LEN);
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

	returns TRUE if env_name exists
*/

bool env_to_opt(char *env_name, pmkcmdopt *opt) {
	char	*env_val;
	bool	rval;

	env_val = getenv(env_name);
	if (env_val == NULL) {
		/* env variable name not found */
		strncpy(opt->value, EMPTY_OPT_VALUE, sizeof(EMPTY_OPT_VALUE));
		rval = FALSE;
	} else {
		/* okay get it */
		strncpy(opt->value, env_val, sizeof(env_val));
		rval = TRUE;
		
	}
	return(rval);
}

/*
	get variable content from make

	varname: name of the variable like for example CC
	result: storage of result
	rsize: result size

	return TRUE on success
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
		return(FALSE);
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
		return(FALSE);
	}

	snprintf(varstr, 256, "/usr/bin/make -f %s", mfn);
	tfp = popen(varstr, "r");
	if (tfp != NULL) {
		/* catch output of make */
		get_line(tfp, result, rsize);
		fclose(tfp);

		rval = TRUE;
	} else {
		fprintf(stderr, "Failed to exec %s\n", varstr);
		rval = FALSE;
	}

	if (unlink(mfn) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", mfn);
	}

	return(rval);
}


/*              
 * split strings using 'delimiter'
 * 
 *	str: string to split
 *	stpath: pointer to the struct where we'll store the paths 
 *	delimiter: string delimiter(s)
 *
 *	returns:  0 on success
 *		 -1 on failure
 */       
int strsplit(char *str, mpath *stpath, char *delimiter) {
	int	i = 0;
	char	*p, *last;

	for ((p = strtok_r(str, delimiter, &last)); p;
		(p = strtok_r(NULL, delimiter, &last)), i++) {
			if (i < MAXTOKENS -1)
				stpath->pathlst[i] = p;
			else
				return(-1);
	}
	stpath->pathnum = i;
	return(0);
}



/* 
 * find a file in a specified path;
 *
 *      stp : struct where we gather the path information from 
 *      file_name : name of the file to search
 *      file_path : storage of the full path if find
 *      fp_len : size of the storage, usually MAXPATHLEN
 *
 *	returns:  0 on success
 *		 -1 on failure
 */
int find_file(mpath *stp, char *file_name, char *file_path, int fp_len) { 
	DIR	*dirp;
	struct	dirent	*dp;
	int	i, s;
	char	**path;

	/* XXX add more checks later */
	s = stp->pathnum;
	path = stp->pathlst;

	for (i = 0; i < s; i++) {
		if (!(dirp = opendir(path[i]))) {
			errorf("could not open directory: %s", path[i]);
			return(-1);
		}
		while ((dp = readdir(dirp)) != NULL) {
			if (dp->d_ino == 0)
				continue;

			if ((strncmp(dp->d_name, file_name, fp_len) == 0) &&
				(snprintf(file_path, fp_len, "%s/%s",
					path[i], file_name) < fp_len)) {
						closedir(dirp);
						return(0);
			}
		}
		closedir(dirp);
	}
	return(-1);
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

	returns TRUE if opened
*/

bool pmk_log_open(char *logname) {
	if (pmk_log_fp != NULL) {
		errorf("%s already open.", logname);
		return(FALSE);
	}
	pmk_log_fp = fopen(logname, "w");
	if (pmk_log_fp == NULL) {
		errorf("while opening %s.", logname);
		return(FALSE);
	} else {
		return(TRUE);
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
		return(TRUE);
	} else {
		errorf("Unable to log.");
		return(FALSE);
	}
}
