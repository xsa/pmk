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


#include <glob.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

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

	fd = fopen(PMKSCAN_DATA, "r");
	if (fd == NULL) {
		errorf("cannot open '%s'", PMKSCAN_DATA);
		return(false);
	}

	rval = parse(fd, pdata);
	fclose(fd);

	if (rval == true) {
		pcell = pdata->first;
		while (pcell != NULL) {
			if (strncmp(pcell->name, "INCLUDES", sizeof(pcell->name)) == 0) {
				sdata->includes = pcell->ht;
			}

			if (strncmp(pcell->name, "FUNCTIONS", sizeof(pcell->name)) == 0) {
				sdata->functions = pcell->ht;
			}

			/* XXX TODO missing other data */

			pcell = pcell->next;
		}
	} else {
		errorf("parsing of data file failed.");
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
			*pval,
			*p;
	regex_t		 re_inc;
	regmatch_t	 rm_inc[1];

	fp = fopen(filename, "r");
	if (fp == NULL) {
		errorf("cannot open %s.", filename);
		return(false);
	}

	/* main parsing */
	while (get_line(fp, line, sizeof(line)) == true) {
		/* init regexp */
		regcomp(&re_inc, "^#include[[:blank:]]+<([^>]+)>", REG_EXTENDED); /* XXX check */

		if (regexec(&re_inc, line, 2, rm_inc, 0) == 0) {
			/* found an include */
			*(line + rm_inc[1].rm_eo) = CHAR_EOS;

			/* copy header name */
			strlcpy(idtf, (char *) (line + rm_inc[1].rm_so), sizeof(idtf));

			/* check data for this header */
			pval = (char *)hash_get(sdata->includes, idtf);
			if (pval != NULL) {
				if ((char *)hash_get(phtgen, idtf) == NULL) {
#ifdef DEBUG
					printf("Setting header '%s'\n", idtf);
#endif

					/* XXX temporary */
					p = strdup(pval);
					pval = p;
					while (*p != CHAR_EOS) {
						if (*p == ',')
							*p = '\n';

						p++;
					}

					/* record header data */
					hash_add(phtgen, idtf, pval);
				}
			}
		}
	}

	/* clean regexps */
	regfree(&re_inc);

	fclose(fp);

	return(true);
}

/*
	XXX
*/

bool output_file(char *ofile, htable *pht) {
	FILE	*fp;
	char	*value;
	int	 i;
	hkeys	*phk;

	fp = fopen(ofile, "w");
	if (fp == NULL) {
		errorf("cannot open %s.", ofile);
		return(false);
	}

	phk = hash_keys(pht);

	fprintf(fp, "# pmkfile generated by pmkscan\n\n");

	for(i = 0 ; i < phk->nkey ; i++) {
		value = (char *)hash_get(pht, phk->keys[i]);

		fprintf(fp, "%s\n\n", value);
	}

	fclose(fp);
	hash_free_hkeys(phk);

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


	printf("Initializing data ... \n");
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

	printf("Generating scan result");
	output_file(PMKSCAN_OUTPUT, pfdata);
	printf("Saved as '%s'\n", PMKSCAN_OUTPUT);

	printf("PMKSCAN finished.\n\n");

	hash_destroy(pfdata);

	return(0);
}
