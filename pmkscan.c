/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
 * Copyright (c) 2004 Xavier Santolaria <xavier@santolaria.net>
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

/*#define PMKSCAN_DEBUG	1*/


/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"

#include <errno.h>
#include <glob.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "dirent.h"
#include "dynarray.h"
#include "hash_tools.h"
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

prskw	kw_pmkscan[] = {
		{"INCLUDES",	PSC_TOK_INCL, PRS_KW_CELL},
		{"FUNCTIONS",	PSC_TOK_FUNC, PRS_KW_CELL}
};

size_t	nbkwps = sizeof(kw_pmkscan) / sizeof(prskw);

extern char	*optarg;
extern int	 optind;


/*
	parse data from PMKSCAN_DATA file

	pdata : parsing data structure
	scandata : scanning data structure

	return : boolean (true on success)
*/

bool parse_data_file(prsdata *pdata, scandata *sdata) {
	FILE	*fd;
	bool	 rval;
	prscell	*pcell;

	fd = fopen(PMKSCAN_DATA, "r");
	if (fd == NULL) {
		errorf("cannot open '%s' : %s.",
			PMKSCAN_DATA, strerror(errno));
		return(false);
	}

	rval = parse_pmkfile(fd, pdata, kw_pmkscan, nbkwps);
	fclose(fd);

	if (rval == true) {
		pcell = pdata->tree->first;
		while (pcell != NULL) {
			switch(pcell->token) {
				case PSC_TOK_INCL :
					sdata->includes = pcell->data;
					break;
			
				case PSC_TOK_FUNC :
					sdata->functions = pcell->data;
					break;

				default :
					errorf("parsing of data file failed.");
					return(false);
					break;
			}

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
		strlcpy(idtf, (char *) (line + rm[1].rm_so), (size_t) (rm[1].rm_eo - rm[1].rm_so + 1));

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
	pht_md : misc data (ex. LANG)

	return : boolean
*/

bool idtf_check(char *idtf, htable *ht_fam, htable *phtgen, htable *pht_md) {
	char	 buf[TMP_BUF_LEN],
		*pval,
		*p;
	dynary	*da;
	int	 i;
	pmkobj	*po;
	potype	 pot;

	/* check if data exist for this identifier */
	po = hash_get(ht_fam, idtf);

	if (po != NULL) {
		/*
			check if the identifier has already been processed
			by looking if it has been added in phtgen
		*/
		if (hash_get(phtgen, idtf) == NULL) {
			/* not present in phtgen => processing */
			pot = po_get_type(po);
			switch (pot) {
				case PO_STRING :
					/* process a string */
					p = po_get_str(po); /* XXX check */
					pval = process_string(p, pht_md); /* XXX grmbl \n does not work */
#ifdef PMKSCAN_DEBUG
					debugf("Processed string '%s'", pval);
#endif

					/* record header data */
					hash_update(phtgen, idtf, po_mk_str(pval));
					free(pval);
					break;

				case PO_LIST :
					/* process a list */
					da = po_get_list(po); /* XXX also pval */
					strlcpy(buf, "", sizeof(buf));

					p = hash_get(pht_md, "LANG"); /* XXX check */

					for (i=0 ; i < da_usize(da) ; i++) {
						p = da_idx(da, i);
						pval = process_string(p, pht_md);
#ifdef PMKSCAN_DEBUG
						debugf("Processed string '%s'", pval);
#endif
						/*strlcat(buf, p, sizeof(buf));*/
						/*if (strncmp(p, "LANG=", 6) == 0) {   */
						/*        strlcat(buf, p, sizeof(buf));*/
						/*}                                    */
						strlcat(buf, pval, sizeof(buf));
						strlcat(buf, "\n", sizeof(buf));
					}

					hash_update(phtgen, idtf, po_mk_str(buf));
					break;

				default :
					errorf("type %u (key '%s') is not supported in idtf_check()", pot, idtf);
                                        return(false);
					break;
			}
		}
	}
        return(true);
}


/*
	parse a C or C++ language file

	filename : file to parse
	scandata : scanning data
	phtgen : ouput hash table

	return : boolean
*/

bool parse_c_file(char *filename, scandata *sdata, htable *phtgen, htable *pht_md) {
	FILE		*fp;
	char		 line[TMP_BUF_LEN], /* XXX better size of buffer ? */
			*p;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		errorf("cannot open '%s' : %s.", filename, strerror(errno));
		return(false);
	}

	/* main parsing */
	while (get_line(fp, line, sizeof(line)) == true) {
		/* check for include */
		p = regex_check("^#include[[:blank:]]+<([^>]+)>", line);
		if (p != NULL) {
#ifdef PMKSCAN_DEBUG
			debugf("Found header '%s'", p);
#endif

			idtf_check(p, sdata->includes, phtgen, pht_md);
		}

		/* check for function */
		p = regex_check("([[:alnum:]_]+)[[:blank:]]*\\(", line);
		if (p != NULL) {
#ifdef PMKSCAN_DEBUG
			debugf("Found function '%s'", p);
#endif

			idtf_check(p, sdata->functions, phtgen, pht_md);
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
	hkeys	*phk;
	int	 i;

	phk = hash_keys(pht);
	if (phk != NULL) {
		fp = fopen(ofile, "w");
		if (fp == NULL) {
			errorf("cannot open '%s' : %s.", ofile, strerror(errno));
			hash_free_hkeys(phk);
			return(false);
		}

		fprintf(fp, "# pmkfile generated by pmkscan\n\n");

		for(i = 0 ; i < phk->nkey ; i++) {
			value = po_get_str(hash_get(pht, phk->keys[i]));
			fprintf(fp, "%s\n", value);
		}

		hash_free_hkeys(phk);
		fclose(fp);

		printf("Saved as '%s'\n", ofile);
	} else {
		printf("No sources found, skipped.\n");
	}

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
#ifdef PMKSCAN_DEBUG
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

bool dir_explore(htable *pht, scandata *psd, char *path) {
	char	 buf[MAXPATHLEN];
	int	 i,
		 r;
	glob_t	 g;
	htable	*pht_misc;

        pht_misc = hash_init(256);
        if (pht_misc == NULL) {
                return(false);
        }

	/* globbing c files */
	for (i = 0 ; i < NB_C_FILE_EXT ; i++) {
		snprintf(buf, sizeof(buf), "%s/*.%s", path, c_file_ext[i]);
		if (i == 0) {
			r = glob(buf, GLOB_NOSORT, NULL, &g);
		} else {
			r = glob(buf, GLOB_NOSORT | GLOB_APPEND, NULL, &g);
		}
		/* XXX check result (r) */
	}

        /* set current language */
        hash_update_dup(pht_misc, "LANG", PMKSCAN_LANG_C); /* XXX check */

	/* parse selected files */
	for (i = 0 ; i < g.gl_pathc ; i++) {
		printf("\t'%s'\n", g.gl_pathv[i]);
		parse_c_file(g.gl_pathv[i], psd, pht, pht_misc);
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
		/* XXX check result (r) */
	}

        /* set current language */
        hash_update_dup(pht_misc, "LANG", PMKSCAN_LANG_CXX); /* XXX check */


	/* parse selected files */
	for (i = 0 ; i < g.gl_pathc ; i++) {
		printf("\t'%s'\n", g.gl_pathv[i]);
		parse_c_file(g.gl_pathv[i], psd, pht, pht_misc);
	}

	globfree(&g);

#ifdef PMKSCAN_DEBUG
	debugf("destroying pht_misc");
#endif
	hash_destroy(pht_misc);
	return(true);
}

/*
	pmkscan(1) usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmkscan [-hv] [path]\n");
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
		chr = getopt(argc, argv, "hv");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(EXIT_SUCCESS);
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					exit(EXIT_FAILURE);
					/* NOTREACHED */
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

	printf("PMKSCAN version %s\n\n", PREMAKE_VERSION);


	printf("Initializing data ... \n");
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("\ncannot intialize prsdata.");
		exit(EXIT_FAILURE);
	} else {
        	if (parse_data_file(pdata, &sd) == false) {
        		/* error message dysplayed by parse_data_file */
        		prsdata_destroy(pdata);
        		exit(EXIT_FAILURE);
        	}
	}
	
	pda = da_init();
	if (pda == NULL) {
		prsdata_destroy(pdata);
		errorf("init failed.");
		exit(EXIT_FAILURE);
	}

	pfdata = hash_init(256); /* XXX can do better :) */
	if (pfdata == NULL) {
		prsdata_destroy(pdata);
		da_destroy(pda);
		errorf("init failed.");
		exit(EXIT_FAILURE);
	}

	if (argc != 0) {
		/* use optional path */
		strlcpy(buf, argv[0], sizeof(buf));
	} else {
		strlcpy(buf, ".", sizeof(buf));
	}

	printf("Ok\n\n");

	dir_recurse(pda, buf);
#ifdef PMKSCAN_DEBUG
	debugf("dir_recurse finished.");
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

	printf("\nPMKSCAN finished.\n\n");

	hash_destroy(pfdata);
	da_destroy(pda);
	prsdata_destroy(pdata);

	return(0);
}
