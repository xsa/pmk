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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"


/*
	get a line from a file and remove newline character

	fd : file descriptor
	line : buffer that will contain the line
	lsize : size of the buffer

	returns a boolean
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
	put env variable in an option
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
		tfn[256] = "/tmp/pmk_tst.XXXXXXXX",
		varstr[256];
	int	mfd = -1,
		tfd = -1;
	bool	rval;

	mfd = mkstemp(mfn);
	if (mfd == -1) {
		/* name randomize failed */
		fprintf(stderr, "Failed to randomize filename\n");
		return(FALSE);
	}

	tfd = mkstemp(tfn);
	if (tfd == -1) {
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
		fprintf(stderr, "Failed to open %s\n", tfn);
		return(FALSE);
	}

	tfp = fdopen(tfd, "r");
	if (tfp != NULL) {
		/* catch output of make */
		snprintf(varstr, 256, "/usr/bin/make -f %s > %s", mfn, tfn);
		system(varstr);

		get_line(tfp, result, rsize);
		fclose(tfp);

		rval = TRUE;
	} else {
		fprintf(stderr, "Failed to open %s\n", tfn);
		rval = FALSE;
	}

	if (unlink(mfn) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", mfn);
	}
	if (unlink(tfn) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", tfn);
	}

	return(rval);
}


/*
	simple error

	errmsg : error message
*/

void error(char *errmsg) {
	fprintf(stderr, "Error : %s\n", errmsg);
}

/*
	parse error

	filename : parsed file name
	line : line of error
	errmsg : error message
*/

void error_line(char *filename, int line, char *errmsg) {
	fprintf(stderr, "Error in '%s' line %d : %s\n", filename, line, errmsg);
}
