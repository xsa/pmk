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
#include <regex.h>

#include "compat/pmk_string.h"
#include "common.h"
#include "parse.h"
#include "pmkscan.h"
#include "premake.h"


/*
	XXX
*/

bool parse_data(prsdata *pdata, scandata *sdata) {
	FILE	*fd;
	bool	 rval;
	prscell	*pcell;

	fd = fopen("data/pmkscan.dat", "r"); /* XXX ARG !! HARDCODE !! */
	if (fd == NULL)
		return(false);
	rval = parse(fd, pdata); /* XXX check ? */
	fclose(fd);

	if (rval == true) {
		pcell = pdata->first;
		while (pcell != NULL) {
			if (strncmp(pcell->name, "INCLUDES", sizeof(pcell->name)) == 0) {
				sdata->includes = pcell->ht;
			}

			pcell = pcell->next;
		}
	}

	return(rval);
}

/*
	parsefile
*/

bool parse_c_file(char *filename, scandata *sdata, htable *phtgen) {
	FILE		*fp;
	char		 line[TMP_BUF_LEN], /* XXX better size of buffer ? */
			 idtf[TMP_BUF_LEN],
			*pval;
	regex_t		 re_inc;
	regmatch_t	 rm_inc[1];

	fp = fopen(filename, "r");
	if (fp == NULL) {
		errorf("cannot open %s.", filename);
		return(false);
	}

	/* init regexps */
	regcomp(&re_inc, "^#include[[:blank:]]+<([^>]+)>", REG_EXTENDED); /* XXX check */

	/* main parsing */
	while (get_line(fp, line, sizeof(line)) == true) {
		if (regexec(&re_inc, line, 2, rm_inc, 0) == 0) {
			/* found a include */
			*(line + rm_inc[1].rm_eo) = CHAR_EOS;

			/* copy header name */
			strlcpy(idtf, (char *) (line + rm_inc[1].rm_so), sizeof(idtf));

			/* check data for this header */
			pval = hash_get(sdata->includes, idtf);
			if (pval != NULL) {
				if (hash_get(phtgen, idtf) == NULL) {
#ifdef DEBUG
					printf("Setting header '%s'\n", idtf);
#endif
					/* record header data */
					hash_add(phtgen, idtf, pval);
				}
			}
		}
	}

	/* clean regexps */
	regfree(&re_inc);

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
	int		 i;
	glob_t		 g;
	htable		*pfdata;
	prsdata		*pdata;
	scandata	 sd;

	printf("PMKSCAN version %s", PREMAKE_VERSION);
#ifdef DEBUG
	printf(" [SUB #%s] [SNAP #%s]", PREMAKE_SUBVER_PMKSCAN, PREMAKE_SNAP);
#endif
	printf("\n\n");


	printf("Initializing data ... ");
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("\ncannot intialize prsdata.");
		exit(1);
	}
	
	if (parse_data(pdata, &sd) == false) {
		/* XXX TODO error message */
		exit(1);
	}

	pfdata = hash_init(256); /* XXX can do better :) */

	printf("Ok\n\n");

	printf("Parsing C related files :\n");

	i = glob("*.c", GLOB_NOSORT, NULL, &g);
#ifdef DEBUG
	if (i == 0) {
		printf("Globbing of *.c files successful.\n");
	}
#endif

	i = glob("*.h", GLOB_NOSORT | GLOB_APPEND, NULL, &g);
#ifdef DEBUG
	if (i == 0) {
		printf("Globbing of *.h files successful.\n");
	}
#endif

	for (i = 0 ; i < g.gl_pathc ; i++) {
#ifdef DEBUG
		printf("[%d] '%s'\n", i, g.gl_pathv[i]);
#endif
		parse_c_file(g.gl_pathv[i], &sd, pfdata);
	}

	globfree(&g);
	printf("Parsing Ok\n\n");

	printf("PMKSCAN finished.\n\n");

	hash_destroy(pfdata);

	return(0);
}
