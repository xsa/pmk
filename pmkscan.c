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


#include <stdio.h>
#include <glob.h>

#include "compat/pmk_string.h"
#include "pmkscan.h"
#include "premake.h"
#include "common.h"
#include "parse.h"


/*
	XXX
*/

void parse_data(prsdata *pdata) {
	FILE	*fd;

	fd = fopen("data/pmkscan.dat", "r"); /* XXX ARG !! HARDCODE !! */
	parse(fd, pdata); /* XXX check ? */
	fclose(fd);
}

/*
	parsefile
*/

bool parse_file(char *filename) {
	FILE	*fp;
	char	line[TMP_BUF_LEN]; /* XXX better size of buffer ? */

	fp = fopen(filename, "r");
	if (fp == NULL) {
		errorf("cannot open %s.", filename);
		return(false);
	}

	while (get_line(fp, line, sizeof(line)) == true) {
		if (strstr(line, "#include") != NULL) {
			printf("Found '%s'\n", line);
		}
	}

	return(true);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmkscan [path]\n");
}


/*
	main
*/

int main(int argc, char *argv[]) {
	int	 i;
	glob_t	 g;
	prsdata	*pdata;

	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot intialize prsdata.");
		exit(1);
	}

	parse_data(pdata);

	glob("*.c", GLOB_NOSORT, NULL, &g);

	for (i = 0 ; i < g.gl_matchc ; i++) {
		printf("[%d] '%s'\n", i, g.gl_pathv[i]);
		parse_file(g.gl_pathv[i]);
	}

	return(0);
}
