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


#include <sys/types.h>
#include <glob.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "common.h"
#include "dirent.h"
#include "dynarray.h"
#include "parse.h"
#include "pmkscan.h"
#include "premake.h"


#define NB_C_FILE_EXT 2
char *c_file_ext[NB_C_FILE_EXT] = {
	"c",
	"h"
};

#define NB_CXX_FILE_EXT 10
char *cxx_file_ext[NB_CXX_FILE_EXT] = {
	"C",
	"cxx",
	"cpp",
	"cc",
	"c++",
	"H",
	"hxx",
	"hpp",
	"hh",
	"h++"
};

/*
	parse data from PMKSCAN_DATA file

	pdata : parsing data structure
	scandata : scanning data structure

	return : boolean (true on success)
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
	check a line with regex pattern

	pattern : pattern to use
	line : string to check

	return : found string or NULL
*/

char *regex_check(char *pattern, char *line) {
	static char	 idtf[TMP_BUF_LEN];
	char		*rval;
	regex_t		 re;
	regmatch_t	 rm[2];

	if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
		regfree(&re);
		return(NULL);
	}

	if (regexec(&re, line, 2, rm, 0) == 0) {
		/* copy header name */
		strlcpy(idtf, (char *) (line + rm[1].rm_so), rm[1].rm_eo - rm[1].rm_so + 1);

		rval =idtf;
	} else {
		rval = NULL;
	}

	regfree(&re);

	return(rval);
}

/*
	check identifier and store relative data in datagen htable

	idtf : identifier string
	ht_fam : identifier family hash table
	phtgen : datagen hash table

	return : -
*/

void idtf_check(char *idtf, htable *ht_fam, htable *phtgen, char *langstr) {
	char	 buf[TMP_BUF_LEN],
		*pval,
		*p;
	dynary	*da;
	int	 i;
	pmkobj	*po;

	/* check data for this identifier */
	po = hash_get(ht_fam, idtf);

	if (po != NULL) {
		/* check if it has been already added in phtgen */
		if (hash_get(phtgen, idtf) == NULL) {
			switch (po_get_type(po)) {
				case PO_STRING :
					/* XXX TODO temporary,  */
					pval = strdup(po_get_str(po));
					p = pval;
					while (*p != CHAR_EOS) {
						if (*p == ',')
							*p = '\n';
		
						p++;
					}
		
					/* record header data */
					hash_add(phtgen, idtf, po_mk_str(pval));
					free(pval);
					break;

				case PO_LIST :
					da = po_get_list(po); /* XXX also pval */
					strlcpy(buf, "", sizeof(buf));

					for (i=0 ; i < da_usize(da) ; i++) {
						p = da_idx(da, i);
						strlcat(buf, p, sizeof(buf));
						if (strncmp(p, "LANG=", 6) == 0) {
							strlcat(buf, langstr, sizeof(buf));
						}
						strlcat(buf, "\n", sizeof(buf));
					}

					hash_add(phtgen, idtf, po_mk_str(buf));
					break;

				default :
					debugf("DOH !!"); /* XXX temporary */
					break;
			}
		}
	}
}


/*
	parse a C or C++ language file

	filename : file to parse
	scandata : scanning data
	phtgen : ouput hash table

	return : boolean
*/

bool parse_c_file(char *filename, scandata *sdata, htable *phtgen, char *langstr) {
	FILE		*fp;
	char		 line[TMP_BUF_LEN], /* XXX better size of buffer ? */
			*p;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		errorf("cannot open %s.", filename);
		return(false);
	}

	/* main parsing */
	while (get_line(fp, line, sizeof(line)) == true) {
		/* check for include */
		p = regex_check("^#include[[:blank:]]+<([^>]+)>", line);
		if (p != NULL) {
#ifdef DEBUG
			printf("Found header '%s'\n", p);
#endif

			idtf_check(p, sdata->includes, phtgen, langstr);
		}

		/* check for function */
		p = regex_check("([[:alnum:]_]+)[[:blank:]]*\\(", line);
		if (p != NULL) {
#ifdef DEBUG
			printf("Found function '%s'\n", p);
#endif

			idtf_check(p, sdata->functions, phtgen, langstr);
		}
	}

	fclose(fp);

	return(true);
}

/*
	generate a pmkfile with results of the scan

	ofile : file name
	pht : output hash table

	return : -
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
		value = (char *)po_get_data(hash_get(pht, phk->keys[i]));

		fprintf(fp, "%s\n", value);
	}

	fclose(fp);
	hash_free_hkeys(phk);

	return(true);
}

/*
	find directories recursively

	pda : dynarray to store directories
	path : starting path

	return : -
*/

void dir_recurse(dynary *pda, char *path) {
	DIR		*pd;
	struct dirent	*pde;
	char		 buf[MAXPATHLEN];

	pd = opendir(path);
	if (pd != NULL) {
		/* this is a directory, save it */
#ifdef DEBUG
		debugf("Add directory '%s' into list.", path);
#endif
		da_push(pda, strdup(path));

		/* check directory entries one by one */
		do {
			pde = readdir(pd);
			if (pde != NULL) {
				/* avoid entries starting by '.' */
				if (pde->d_name[0] != '.') {
					snprintf(buf, sizeof(buf), "%s/%s", path, pde->d_name);
					/* try to recurse the resulting path */
					dir_recurse(pda, buf);
				}
			}
		} while (pde != NULL);
		closedir(pd);
	}
}

/*
	launch scan on selected files of a directory

	pht : output hash table
	psd : scanned data
	path : directory to scan

	return : -
*/

void dir_explore(htable *pht, scandata *psd, char *path) {
	char	buf[MAXPATHLEN];
	int	i,
		r;
	glob_t	g;

	/* globbing c files */
	for (i = 0 ; i < NB_C_FILE_EXT ; i++) {
		snprintf(buf, sizeof(buf), "%s/*.%s", path, c_file_ext[i]);
		if (i == 0) {
			r = glob(buf, GLOB_NOSORT, NULL, &g);
		} else {
			r = glob(buf, GLOB_NOSORT | GLOB_APPEND, NULL, &g);
		}
	}

	/* parse selected files */
	for (i = 0 ; i < g.gl_pathc ; i++) {
		printf("\t'%s'\n", g.gl_pathv[i]);
		parse_c_file(g.gl_pathv[i], psd, pht, "\"C\"");
	}

	globfree(&g);

	/* globbing c++ files */
	for (i = 0 ; i < NB_CXX_FILE_EXT ; i++) {
		snprintf(buf, sizeof(buf), "%s/*.%s", path, cxx_file_ext[i]);
		if (i == 0) {
			r = glob(buf, GLOB_NOSORT, NULL, &g);
		} else {
			r = glob(buf, GLOB_NOSORT | GLOB_APPEND, NULL, &g);
		}
	}

	/* parse selected files */
	for (i = 0 ; i < g.gl_pathc ; i++) {
		printf("\t'%s'\n", g.gl_pathv[i]);
		parse_c_file(g.gl_pathv[i], psd, pht, "\"C++\"");
	}

	globfree(&g);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmkscan [-vh] [path]\n");
}


/*
	main
*/

int main(int argc, char *argv[]) {
	bool		 go_exit = false;
	char		 buf[MAXPATHLEN],
			*p;
	dynary		*pda;
	htable		*pfdata;
	int		 chr;
	prsdata		*pdata;
	scandata	 sd;

	while (go_exit == false) {
		chr = getopt(argc, argv, "b:f:ho:v");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(0);
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					exit(1);
					break;
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

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
	
	pda = da_init();
	if (pda == NULL) {
		prsdata_destroy(pdata);
		errorf("init failed.");
		exit(1);
	}

	pfdata = hash_init(256); /* XXX can do better :) */
	if (pfdata == NULL) {
		prsdata_destroy(pdata);
		da_destroy(pda);
		errorf("init failed.");
		exit(1);
	}

	if (parse_data(pdata, &sd) == false) {
		/* XXX TODO error message */
		exit(1);
	}

	if (argc != 0) {
		/* use optional path */
		strlcpy(buf, argv[0], sizeof(buf));
	} else {
		strlcpy(buf, ".", sizeof(buf));
	}

	printf("Ok\n\n");

	dir_recurse(pda, buf);
#ifdef DEBUG
	printf("dir_recurse finished.\n");
#endif

	printf("Start parsing files :\n");
	do {
		p = da_pop(pda);
		if (p != NULL) {
			dir_explore(pfdata, &sd, p);
			free(p);
		}
	} while (p != NULL);
	printf("Parsing Ok.\n\n");

	printf("Generating scan result ...\n");
	output_file(PMKSCAN_OUTPUT, pfdata);
	printf("Saved as '%s'\n", PMKSCAN_OUTPUT);

	printf("PMKSCAN finished.\n\n");

	hash_destroy(pfdata);
	da_destroy(pda);
	prsdata_destroy(pdata);

	return(0);
}
