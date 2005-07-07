/* $Id$ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
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


/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <glob.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "dynarray.h"
#include "hash_tools.h"
#include "parse.h"
#include "pathtools.h"
#include "pmkscan.h"
#include "premake.h"


/*#define PMKSCAN_DEBUG	1*/


/********************
 * global variables *
 ***********************************************************************/

scn_ext_t	 file_ext[] = {
		{"*.asm",	FILE_TYPE_ASM},
		{"*.C",		FILE_TYPE_C},
		{"*.c",		FILE_TYPE_C},
		{"*.cc",	FILE_TYPE_C},
		{"*.cpp",	FILE_TYPE_CXX},
		{"*.cxx",	FILE_TYPE_CXX},
		{"*.c++",	FILE_TYPE_CXX},
		{"*.H",		FILE_TYPE_C},
		{"*.h",		FILE_TYPE_C},
		{"*.hh",	FILE_TYPE_C},
		{"*.hpp",	FILE_TYPE_CXX},
		{"*.hxx",	FILE_TYPE_CXX},
		{"*.h++",	FILE_TYPE_CXX},
		{"*.S",		FILE_TYPE_ASM},
		{"*.s",		FILE_TYPE_ASM},
		{"*.l", 	FILE_TYPE_LEX},
		{"*.y", 	FILE_TYPE_YACC},
		{"*.1",		FILE_TYPE_MAN},
		{"*.2",		FILE_TYPE_MAN},
		{"*.3",		FILE_TYPE_MAN},
		{"*.4",		FILE_TYPE_MAN},
		{"*.5",		FILE_TYPE_MAN},
		{"*.6",		FILE_TYPE_MAN},
		{"*.7",		FILE_TYPE_MAN},
		{"*.8",		FILE_TYPE_MAN},
		{"*.9",		FILE_TYPE_MAN},
		{"*.xpm",	FILE_TYPE_IMG},
		{"*.jpg",	FILE_TYPE_IMG},
		{"*.jpeg",	FILE_TYPE_IMG},
		{"*.png",	FILE_TYPE_IMG},
		{"*.gif",	FILE_TYPE_IMG},
		{"*.html",	FILE_TYPE_HTML},
		{"*.htm",	FILE_TYPE_HTML},
		{"*.txt",	FILE_TYPE_TEXT},
		{"*.dat",	FILE_TYPE_DATA}
};
size_t	 nb_file_ext = sizeof(file_ext) / sizeof(scn_ext_t);


prskw	kw_pmkscan[] = {
	{"FUNCTIONS",	PSC_TOK_FUNC,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL},	/* XXX deprecated ? */
	{"INCLUDES",	PSC_TOK_INCL,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"PROCEDURES",	PSC_TOK_FUNC,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"TYPES",		PSC_TOK_TYPE,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL}
};
size_t	nbkwps = sizeof(kw_pmkscan) / sizeof(prskw);


/* pmkscan script parser options ***************************************/

/* common required options */
kw_t	req_dir[] = {
		{KW_OPT_DIR,	PO_STRING}
};

/* GEN_PMKFILE options */
kw_t	opt_genpmk[] = {
		{KW_OPT_DSC,	PO_LIST},
		{KW_OPT_REC,	PO_BOOL},
		{KW_OPT_UNI,	PO_BOOL}
};

kwopt_t	kw_genpmk = {
	req_dir,
	sizeof(req_dir) / sizeof(kw_t),
	opt_genpmk,
	sizeof(opt_genpmk) / sizeof(kw_t)
};

/* GEN_MAKEFILE options */
kw_t	opt_genmkf[] = {
		{KW_OPT_DSC,	PO_LIST},
		{KW_OPT_NAM,	PO_STRING},
		{KW_OPT_REC,	PO_BOOL},
		{KW_OPT_UNI,	PO_BOOL}
};

kwopt_t	kw_genmkf = {
	req_dir,
	sizeof(req_dir) / sizeof(kw_t),
	opt_genmkf,
	sizeof(opt_genmkf) / sizeof(kw_t)
};

/* GEN_ZONE options */
kw_t	opt_genzone[] = {
		{KW_OPT_DSC,	PO_LIST},
		{KW_OPT_MKF,	PO_BOOL},
		{KW_OPT_NAM,	PO_STRING},
		{KW_OPT_PMK,	PO_BOOL},
		{KW_OPT_REC,	PO_BOOL},
		{KW_OPT_UNI,	PO_BOOL}
};

kwopt_t	kw_genzone = {
	req_dir,
	sizeof(req_dir) / sizeof(kw_t),
	opt_genzone,
	sizeof(opt_genzone) / sizeof(kw_t)
};

prskw	kw_scanfile[] = {
	{"GEN_PMKFILE",		PSC_TOK_PMKF,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_genpmk},
	{"GEN_MAKEFILE",	PSC_TOK_MAKF,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_genmkf},
	{"GEN_ZONE",		PSC_TOK_ZONE,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_genzone}
};
size_t	nbkwsf = sizeof(kw_scanfile) / sizeof(prskw);

extern char	*optarg;
extern int	 optind;


/******************************
 * init and destroy functions *
 ***********************************************************************/

/********************
 * scan_node_init() *
 ***********************************************************************
 DESCR
	initialize scan node

 IN
	fname :	node file name

 OUT
	scan node structure
 ***********************************************************************/

scn_node_t *scan_node_init(char *fname) {
	char		*pstr;
	scn_node_t	*pnode;
	size_t		 len;

	/* allocate memory */
	pnode = (scn_node_t *) malloc(sizeof(scn_node_t));
	if (pnode != NULL) {
		/* set filename */
		pnode->fname = strdup(fname);
		if (pnode->fname == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* build prefix name */
		len = strlen(fname);
		pstr = &fname[len];
		while (pstr > fname) {
			if (*pstr == '.') {
				len = (size_t) (pstr - fname);
				break;
			} else {
				pstr--;
			}
		}

		/* get the size including null char */
		len++;
		pnode->prefix = (char *) malloc(len);
		if (pnode->prefix == NULL) {
			/* XXX msg err ? */
			return(NULL);
		}
		strlcpy(pnode->prefix, fname, len);

		/* init dynamic array of dependencies */
		pnode->system_inc = da_init();
		if (pnode->system_inc == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->local_inc = da_init();
		if (pnode->local_inc == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->func_calls = da_init();
		if (pnode->func_calls == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->func_decls = da_init();
		if (pnode->func_decls == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->type_idtfs = da_init();
		if (pnode->type_idtfs == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->src_deps = da_init();
		if (pnode->src_deps == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->obj_links = da_init();
		if (pnode->obj_links == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init XXX  */
		pnode->obj_deps = NULL;

		/* init dependency state */
		pnode->isdep = false; /* useful ? see scan_file() */

		/* init main() procedure flag */
		pnode->mainproc = false;

		/* init dependency score */
		pnode->score = 0;

		/* object pointer or null */
		pnode->obj_name = NULL;

		/* directory name */
		pnode->dname = NULL;
	}

	/* return initialized structure or NULL */
	return(pnode);
}


/***********************
 * scan_node_destroy() *
 ***********************************************************************
 DESCR
 	destroy scan node structure

 IN
	pnode :	scan node structure

 OUT
 	NONE
 ***********************************************************************/

void scan_node_destroy(scn_node_t *pnode) {
#ifdef PMKSCAN_DEBUG
	/*debugf("destroying node '%s'", pnode->fname);*/
#endif
	if (pnode->fname != NULL) {
		free(pnode->fname);
	}

	if (pnode->obj_name != NULL) {
		free(pnode->obj_name);
	}

	if (pnode->prefix != NULL) {
		free(pnode->prefix);
	}

	if (pnode->dname != NULL) {
		free(pnode->dname);
	}

	if (pnode->system_inc != NULL) {
		da_destroy(pnode->system_inc);
	}

	if (pnode->local_inc != NULL) {
		da_destroy(pnode->local_inc);
	}

	if (pnode->func_calls != NULL) {
		da_destroy(pnode->func_calls);
	}

	if (pnode->func_decls != NULL) {
		da_destroy(pnode->func_decls);
	}

	if (pnode->type_idtfs != NULL) {
		da_destroy(pnode->type_idtfs);
	}

	if (pnode->src_deps != NULL) {
		da_destroy(pnode->src_deps);
	}

	if (pnode->obj_links != NULL) {
		da_destroy(pnode->obj_links);
	}

	free(pnode);
}


/********************
 * scan_zone_init() *
 ***********************************************************************
 DESCR
	initialize scan zone structure

 IN
	nodes :	global nodes table

 OUT
	scan zone structure
 ***********************************************************************/

scn_zone_t *scan_zone_init(htable *nodes) {
	scn_zone_t	*pzone;
	size_t			i;

	pzone = (scn_zone_t *) malloc(sizeof(scn_zone_t));
	if (pzone != NULL) {
		/* set global nodes table */
		pzone->nodes = nodes;

		/* init boolean flags */
		pzone->recursive = false;
		pzone->unique = false;
		pzone->gen_pmk = false;
		pzone->gen_mkf = false;

		/* discard list */
		pzone->discard = NULL;

		/* init file type flags */
		for (i = 0 ; i < sizeof(pzone->found) ; i++) {
			pzone->found[i] = false;
		}

		/* init source flag */
		pzone->found_src = false;

		/* init zone tables */
		pzone->objects = hash_init(256); /* XXX can do better :) */
		pzone->targets = hash_init(256); /* XXX can do better :) */
		pzone->checks = hash_init(128); /* XXX can do better :) */

		/* init man pages dynary */
		pzone->manpgs = da_init();

		/* init data files dynary */
		pzone->datafiles = da_init();
	}

	return(pzone);
}


/***********************
 * scan_zone_destroy() *
 ***********************************************************************
 DESCR
	destroy scan zone structure

 IN
	pzone : scan zone structure

 OUT
	NONE
 ***********************************************************************/

void scan_zone_destroy(scn_zone_t *pzone) {
	if (pzone->objects != NULL) {
		hash_destroy(pzone->objects);
	}

	if (pzone->targets != NULL) {
		hash_destroy(pzone->targets);
	}

	if (pzone->checks != NULL) {
		hash_destroy(pzone->checks);
	}

	if (pzone->manpgs != NULL) {
		da_destroy(pzone->manpgs);
	}

	if (pzone->datafiles != NULL) {
		da_destroy(pzone->datafiles);
	}

	free(pzone);
}


/******************************
 * pmkfile specific functions *
 ***********************************************************************/

/*********************
 * parse_data_file() *
 ***********************************************************************
 DESCR
	parse data from PMKSCAN_DATA file

 IN
	pdata : parsing data structure
	scandata : scanning data structure

 OUT
	boolean (true on success)
 ***********************************************************************/

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

	sdata->includes = NULL;
	sdata->functions = NULL;
	sdata->types = NULL;

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

				case PSC_TOK_TYPE :
					sdata->types = pcell->data;
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


/****************
 * idtf_check() *
 ***********************************************************************
 DESCR
	check identifier and store relative data in datagen htable

 IN
	idtf : identifier string
	ht_fam : identifier family hash table
	phtgen : datagen hash table
	pht_md : misc data (ex. LANG)

 OUT
	boolean
 ***********************************************************************/

bool idtf_check(char *idtf, htable *ht_fam, htable *phtgen, htable *pht_md) {
	char			 buf[TMP_BUF_LEN],
					*pval,
					*p;
	dynary			*da;
	pmkobj			*po;
	potype			 pot;
	unsigned int	 i;

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
						/* process string for substitution */
					pval = process_string(p, pht_md); /* XXX grmbl \n does not work */
#ifdef PMKSCAN_DEBUG
					debugf("Processed string '%s'", pval);
#endif

					/* record header data */
					hash_update_dup(phtgen, idtf, pval); /* XXX check ? */
					free(pval);
					break;

				case PO_LIST :
					/* process a list */
					da = po_get_list(po); /* XXX also pval */
					strlcpy(buf, "", sizeof(buf)); /* no check */


					for (i=0 ; i < da_usize(da) ; i++) {
						p = da_idx(da, i);
						/* process string for substitution */
						pval = process_string(p, pht_md);
#ifdef PMKSCAN_DEBUG
						debugf("Processed string '%s'", pval);
#endif
						/* add line in buffer */
						strlcat(buf, pval, sizeof(buf)); /* no check */
						free(pval);

						/* append newline character */
						if (strlcat_b(buf, "\n", sizeof(buf)) == false)
							return(false);
					}

					hash_update_dup(phtgen, idtf, buf);
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


/****************
 * gen_checks() *
 ***********************************************************************
 DESCR
	build pmkfile using gathered data

 IN
	psz :	scanning zone data
	pht :	check component hash table
	psd :	parsing data structure
 OUT
	boolean
 ***********************************************************************/

bool gen_checks(scn_zone_t *psz, scandata *psd) {
	char			*pstr;
	hkeys			*phk;
	scn_node_t		*pn;
	unsigned int	 i,
					 j;
	htable			*pht_misc;

	pht_misc = hash_init(256); /* XXX */
	if (pht_misc == NULL) {
		return(false);
	}

	/* for each node */
	phk = hash_keys(psz->nodes);
	for(i = 0 ; i < phk->nkey ; i++) {
		pn = hash_get(psz->nodes, phk->keys[i]); /* no check needed */

		/* set current language */
		switch(pn->type) {
			case FILE_TYPE_C :
				hash_update_dup(pht_misc, "LANG", PMKSCAN_LANG_C); /* XXX check */
				break;

			case FILE_TYPE_CXX :
				hash_update_dup(pht_misc, "LANG", PMKSCAN_LANG_CXX); /* XXX check */
				break;

			default :
				hash_delete(pht_misc, "LANG");
		}

		if (psd->includes != NULL) {
			/* process system includes */
			for (j = 0 ; j < da_usize(pn->system_inc) ; j++) {
				pstr = (char *) da_idx(pn->system_inc, j);
				idtf_check(pstr, psd->includes, psz->checks, pht_misc);
			}
		}

		if (psd->functions != NULL) {
			/* process function calls */
			for (j = 0 ; j < da_usize(pn->func_calls) ; j++) {
				pstr = (char *) da_idx(pn->func_calls, j);
				idtf_check(pstr, psd->functions, psz->checks, pht_misc);
			}
		}

		if (psd->types != NULL) {
			/* process types definitions */
			for (j = 0 ; j < da_usize(pn->type_idtfs) ; j++) {
				pstr = (char *) da_idx(pn->type_idtfs, j);
				idtf_check(pstr, psd->types, psz->checks, pht_misc);
			}
		}
	}

#ifdef PMKSCAN_DEBUG
	debugf("destroying pht_misc");
#endif
	hash_destroy(pht_misc);

	return(true);
}


/*********************
 * scan_output_pmk() *
 ***********************************************************************
 DESCR
	build pmkfile using gathered data

 IN
	fname :	output file name
	psz :	scanning zone data
	psd :	parsing data structure
 OUT
	boolean
 ***********************************************************************/

/* XXX TODO : add comments, add default pmkfile stuff, manage when no checks exists */
bool scan_build_pmk(char *fname, scn_zone_t *psz, scandata *psd) {
	FILE			*fp;
	char			*value;
	hkeys			*phk;
	unsigned int	 i;

	phk = hash_keys(psz->checks);
	if (phk != NULL) {
		fp = fopen(fname, "w");
		if (fp == NULL) {
			errorf("cannot open '%s' : %s.", fname, strerror(errno));
			hash_free_hkeys(phk);
			return(false);
		}

		fprintf(fp, "# pmkfile generated by pmkscan\n\n");

		for(i = 0 ; i < phk->nkey ; i++) {
			value = hash_get(psz->checks, phk->keys[i]);
			fprintf(fp, "%s\n", value);
		}

		hash_free_hkeys(phk);
		fclose(fp);

		printf("Saved as '%s'\n", fname);
	} else {
		printf("No sources found, skipped.\n");
	}

	return(true);
}


/*******************************
 * makefile specific functions *
 ***********************************************************************/

/***************
 * find_deps() *
 ***********************************************************************
 DESCR
	check if function call list has at least one dependency with the
	function declaration list

 IN
	da_fc :		function call list
	da_fd :		function declaration list

 OUT
	boolean
 ***********************************************************************/

bool find_deps(dynary *da_fc, dynary *da_fd) {
	char			*str_fc,
					*str_fd;
	size_t			 siz;
	unsigned int	 i,
					 j;

	/* for each entry of the first dynary */
	for (i = 0 ; i < da_usize(da_fc) ; i++) {
		str_fc = da_idx(da_fc, i);
		siz = strlen(str_fc) + 1;

		/* compare with each entry of the second dynary */
		for (j = 0 ; j < da_usize(da_fd) ; j++) {
			str_fd = da_idx(da_fd, j);
			if (strncmp(str_fc, str_fd, siz) == 0) {
#ifdef PMKSCAN_DEBUG
				debugf("found common dep '%s'", str_fc);
#endif
				/* and return true if a common dependency is found */
				return(true);
			}
		}

	}

	/* no common stuff found */
	return(false);
}


/*****************
 * extract_dir() *
 ***********************************************************************
 DESCR
	extract the directory portion of the given path

 IN
	path :		original path
	dirbuf :	buffer to store the extracted directory
	blen :		buffer length

 OUT
	NONE
 ***********************************************************************/

void extract_dir(char *path, char *dirbuf, size_t blen) {
	char	 buffer[MAXPATHLEN],
			*p;

	/*
		work on a local copy due to implementations of dirname that do
		not preserve original string
	*/
	strlcpy(buffer, path, sizeof(buffer)); /* XXX check */

	/* get directory part */
	p = dirname(buffer);

	/* if the result start by "./" then skip it */
	if ((p[0] == '.') && (p[1] == '/')) {
		p = p + 2;
	}

	/* copy result in storage location */
	strlcpy(dirbuf, p, blen); /* XXX check */

	/* if directory is "." then set empty string */
	if (*dirbuf == '.') {
		*dirbuf = '\0';
	}
}


/****************
 * build_path() *
 ***********************************************************************
 DESCR
	build a path from given directory and filename

 IN
	dir :		directory string
	file :		file name string
	buffer :	buffer where to store the result
	blen :		buffer length

 OUT
	NONE
 ***********************************************************************/

void build_path(char *dir, char *file, char *buffer, size_t blen) {
	char	 tmp[MAXPATHLEN],
			 chk[MAXPATHLEN];

	if (*dir == '\0') {
		/* directory empty, store only file name */
		strlcpy(tmp, file, sizeof(tmp));
	} else {
		/* join directory and file names with the directory separator */
		snprintf(tmp, sizeof(tmp), "%s/%s", dir, file);
	}

	/* XXX */
	chkpath(tmp, chk);

	strlcpy(buffer, chk, blen);
}


/**********************
 * recurse_obj_deps() *
 ***********************************************************************
 DESCR
	gather recursively the local include dependencies

 IN
	nodes :		nodes hash table
	deps :		dynary structure where to store dependencies
	nodename :	node name to process recursively

 OUT
	boolean
 ***********************************************************************/

bool recurse_obj_deps(htable *nodes, dynary *deps, char *nodename) {
	char		 buf[MAXPATHLEN],
				 dir[MAXPATHLEN];
	scn_node_t	*pnode;
	size_t		 i;

	/* check if the node is already listed as target dependency */
	if (da_find(deps, nodename) == true) {
		/* yes, skip processing */
		return(true);
	}

	/* else add the new dependency */
	if (da_push(deps, strdup(nodename)) == false) {
		return(false);
	}

	/* get node structure */
	pnode = hash_get(nodes, nodename); /* should not fail */
	if (pnode == NULL) {
		/*debugf("ouargl2 '%s'", nodename); |+ XXX +|*/
		return(true);
	}

	/* get directory */
	extract_dir(nodename, dir, sizeof(dir));

	/* look for all the local dependencies of the current node */
	for (i = 0 ; i < da_usize(pnode->local_inc) ; i++) {
		/* build dependency name */
		build_path(dir, (char *)da_idx(pnode->local_inc, i), buf, sizeof(buf));

		/* ... and recurse it */
		if (recurse_obj_deps(nodes, deps, buf) == false) {
			return(false);
		}
	}

	return(true);
}


/*****************
 * gen_objects() *
 ***********************************************************************
 DESCR
	generate objects from the nodes

 IN
	psz :	scanning zone data

 OUT
	boolean
 ***********************************************************************/

bool gen_objects(scn_zone_t *psz) {
	char			 buf[MAXPATHLEN], /* XXX filename length */
					*pstr;
	hkeys			*phk;
	scn_node_t		*pnode,
					*pn;
	unsigned int	 i,
					 j;

	phk = hash_keys(psz->nodes);
	if (phk == NULL) {
		/* no objects to process */
		printf("No objects to generate.\n");
		return(true);
	}

	for(i = 0 ; i < phk->nkey ; i++) {
		pnode = hash_get(psz->nodes, phk->keys[i]); /* XXX check needed ??? (think no) */
#ifdef PMKSCAN_DEBUG
		debugf("score of '%s' = %d", phk->keys[i], pnode->score);
#endif

		/* if we got a node with a score of 0 then it should
			be an object */
		if (pnode->score == 0) {
			/* build object name */
			strlcpy(buf, pnode->prefix, sizeof(buf));
			strlcat(buf, OBJ_SUFFIX, sizeof(buf));

			/* add object reference */
			hash_update_dup(psz->objects, buf, pnode->fname); /* XXX check */

			/* set object name */
			pnode->obj_name = strdup(buf);

			/* generate object's source dependencies */
			recurse_obj_deps(psz->nodes, pnode->src_deps, pnode->fname);

			/* for each local include */
			for (j = 0 ; j < da_usize(pnode->local_inc) ; j++) {
				pstr = da_idx(pnode->local_inc, j);
				pn = hash_get(psz->nodes, pstr);

				if (pn == NULL) {
					/*debugf("ouargl '%s'", pstr); |+ XXX +|*/
					continue;
				}

				/* check for common function declarators */
				if (find_deps(pnode->func_decls, pn->func_decls) == true) {
#ifdef PMKSCAN_DEBUG
					debugf("adding object link '%s' dependency to node '%s'", pnode->obj_name, pn->fname);
#endif

					/* and set object link if common declarator is found */
					if (da_push(pn->obj_links,
								strdup(pnode->obj_name)) == false) {
						return(false);
					}
				}
			}

			/* extra stuff */
			if (pnode->type == FILE_TYPE_ASM) {
				/* try to find optionnal header file for assembly functions */
				for(j = 0 ; j < phk->nkey ; j++) {
					pn = hash_get(psz->nodes, phk->keys[j]);

					/* check for common function declarators */
					if (find_deps(pnode->func_decls, pn->func_decls) == true) {
#ifdef PMKSCAN_DEBUG
						debugf("adding object link '%s' dependency to node '%s'", pnode->obj_name, pn->fname);
#endif

						/* and set object link if common declarator is found */
						if (da_push(pn->obj_links,
									strdup(pnode->obj_name)) == false) {
							return(false);
						}
					}
				}
			}
		}
	}

	hash_free_hkeys(phk);

	return(true);
}


/**********************
 * recurse_src_deps() *
 ***********************************************************************
 DESCR
	generate targets from the objects

 IN
	psz :	scanning zone data
	deps :	dynary structure where to store object dependencies
	name :	node name to process

 OUT
	boolean
 ***********************************************************************/

bool recurse_src_deps(scn_zone_t *psz, dynary *deps, char *name) {
	char		*src,
				*odep;
	scn_node_t	*pnode,
				*pn;
	size_t		 i,
				 j;

	/* get node structure */
	pnode = hash_get(psz->nodes, name);
#ifdef PMKSCAN_DEBUG
	debugf("recurse_src_deps() : node '%s' START", pnode->fname);
#endif

	/* for each source dependency */
	for (i = 0 ; i < da_usize(pnode->src_deps) ; i++) {
		/* get the node structure */
		src = da_idx(pnode->src_deps, i);
		pn = hash_get(psz->nodes, src);

		if (pn == NULL) {
			/*debugf("ouargl3 '%s'", src); |+ XXX +|*/
			continue; /* XXX */
		}

#ifdef PMKSCAN_DEBUG
		debugf("recurse_src_deps() : node '%s' : src dep '%s' (%d)", pnode->fname, src, da_usize(pn->obj_links));
#endif

		/* check each object link */
		for (j = 0 ; j < da_usize(pn->obj_links) ; j++) {
			/* get object name */
			odep = da_idx(pn->obj_links, j);
#ifdef PMKSCAN_DEBUG
			debugf("recurse_src_deps() : node '%s' : obj link '%s'", pnode->fname, odep);
#endif

			/* check if already in the list */
			if (da_find(deps, odep) == false) {
				/* and add the object if not already present */
				if (da_push(deps, strdup(odep)) == false) {
					/* XXX err msg */
					return(false);
				}

#ifdef PMKSCAN_DEBUG
				debugf("recurse_src_deps() : node '%s' : adding '%s' in deps", pnode->fname, odep);
#endif

				/* recurse dependencies of this object */
				src = hash_get(psz->objects, odep);
#ifdef PMKSCAN_DEBUG
				debugf("recurse_src_deps() : node '%s' : => '%s'", pnode->fname, src);
#endif
				if (recurse_src_deps(psz, deps, src) == false) {
					/* recurse failed */
					return(false);
				}
			}
		}
	}
#ifdef PMKSCAN_DEBUG
	debugf("recurse_src_deps() : node '%s' END", pnode->fname);
#endif

	return(true);
}


/*****************
 * gen_targets() *
 ***********************************************************************
 DESCR
	generate targets from the objects

 IN
	psz :	scanning zone data

 OUT
	boolean
 ***********************************************************************/

bool gen_targets(scn_zone_t *psz) {
	char			*nodename;
	hkeys			*phk;
	scn_node_t		*pnode;
	unsigned int	 i;

	phk = hash_keys(psz->objects);
	if (phk == NULL) {
		/* no objects, skip */
		printf("No targets to generate.\n");
		return(true);
	}

	/* for each object */
	for(i = 0 ; i < phk->nkey ; i++) {
		/* get it's node structure */
		nodename = hash_get(psz->objects, phk->keys[i]);
		pnode = hash_get(psz->nodes, nodename); /* XXX check needed ??? (think no) */

		/* if main procedure has been found then it's a target */
		if (pnode->mainproc == true) {
			/* adding in the target list */
			if (hash_update_dup(psz->targets, pnode->prefix,
										pnode->fname) == HASH_ADD_FAIL) {
				return(false);
			}

			/* init object deps */
			pnode->obj_deps = da_init();
			if (pnode->obj_deps == NULL) {
				/* XXX err msg */
				return(false);
			}

#ifdef PMKSCAN_DEBUG
			debugf("START recurse_src_deps() for node '%s'", pnode->fname);
#endif
			/* recurse source deps to find object deps */
			if (recurse_src_deps(psz, pnode->obj_deps, pnode->fname) == false) {
				/* failed */
				return(false);
			}
#ifdef PMKSCAN_DEBUG
			debugf("END recurse_src_deps() for node '%s'\n", pnode->fname);
#endif
		}
	}

	return(false);
}


/*******************
 * fprintf_width() *
 ***********************************************************************
 DESCR
	print in a file in a formated width

 IN
	width :		width of a line
	offset :	actual column offset
	fp :		file stream
	str :		string to append

 OUT
	new offset

 NOTE
	NONE could support of a left limit for indentation
 ***********************************************************************/

size_t fprintf_width(size_t left, size_t width, size_t offset, FILE *fp, char *str) {
	unsigned int	 i,
					 m;
	size_t			 s,
					 t;

	/* compute new offset with the string length */
	s = strlen(str);
	t = offset + s;

	/* check if offset is greater than allowed width */
	if (t < width) {
		if (left != offset) {
			/* not the first append */
			fprintf(fp, " ");
			t++;
		}

		/* got enough space on the line */
		fprintf(fp, "%s", str);

		offset = t;
	} else {
		/* compute number of tabs for the left margin */
		m = (left / MKF_TAB_WIDTH);
		if ((left % MKF_TAB_WIDTH) != 0) {
			m++;
		}

		/* terminate current line */
		fprintf(fp, " \\\n");
		offset = 0;

		/* build left margin */
		for (i = 0 ; i < m ; i ++) {
			fprintf(fp, "\t");
			offset = offset + MKF_TAB_WIDTH;
		}

		/* print string */
		fprintf(fp, "%s", str);
		offset = offset + s;
	}

	return(offset);
}


/***********************
 * mkf_output_header() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_header(FILE *fp, scn_zone_t *psz) {
	char			 buf[256];
	time_t			 now;
	unsigned int	 i;

	/* generating date */
	now = time(NULL);
	strftime(buf, sizeof(buf), MKF_TIME_GEN, localtime(&now));

	/* set header */
	fprintf(fp, MKF_HEADER_GEN, buf);

	/* preprocessor */
	if (psz->found[FILE_TYPE_ASM] == true) {
		fprintf(fp, MKF_HEADER_CPP);
	}

	fprintf(fp, "\n# language specific\n");
	/* assembly stuff */
	if (psz->found[FILE_TYPE_ASM] == true) {
		fprintf(fp, MKF_HEADER_ASM);
	}
	/* C stuff */
	if (psz->found[FILE_TYPE_C] == true) {
		fprintf(fp, MKF_HEADER_C);
	}
	/* C++ stuff */
	if (psz->found[FILE_TYPE_CXX] == true) {
		fprintf(fp, MKF_HEADER_CXX);
	}
	/* lex stuff */
	if (psz->found[FILE_TYPE_LEX] == true) {
		fprintf(fp, MKF_HEADER_LEX);
	}
	/* yacc stuff */
	if (psz->found[FILE_TYPE_YACC] == true) {
		fprintf(fp, MKF_HEADER_YACC);
	}

	fprintf(fp, "\n# misc stuff\n");
	if (psz->found_src == true) {
		/* linker stuff */
		fprintf(fp, MKF_HEADER_LD);
	}
	/* misc stuff */
	fprintf(fp, MKF_HEADER_MISC);

	if (psz->found[FILE_TYPE_DATA] == true) {
		/* package data */
		fprintf(fp, MKF_HEADER_DATA);
	}

	/* directories */
	fprintf(fp, "# specific directories\n");
	fprintf(fp, MKF_HEADER_DIR);
	if (psz->found[FILE_TYPE_MAN] == true) {
		/* man pages directories */
		for (i = 0 ; i < 10 ; i++) {
			fprintf(fp, MKF_MANX_DIR, i, i);
		}
	}

	fprintf(fp, MKF_LINE_JUMP);

	if (psz->found_src == true) {
		/* suffixes */
		fprintf(fp, MKF_SUFFIXES);
	}

	/* assembly object build rule */
	if (psz->found[FILE_TYPE_ASM] == true) {
		fprintf(fp, MKF_BLD_ASM_OBJ);
	}

	/* C object build rule */
	if (psz->found[FILE_TYPE_C] == true) {
		fprintf(fp, MKF_BLD_C_OBJ);
	}

	/* C++ object build rule */
	if (psz->found[FILE_TYPE_CXX] == true) {
		fprintf(fp, MKF_BLD_CXX_OBJ);
	}

	/*  lex source build rule */
	if (psz->found[FILE_TYPE_LEX] == true) {
		fprintf(fp, MKF_BLD_LEX_SRC);
	}

	/*  yacc source build rule */
	if (psz->found[FILE_TYPE_YACC] == true) {
		fprintf(fp, MKF_BLD_YACC_SRC);
	}
}


/*********************
 * mkf_output_srcs() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_srcs(FILE *fp, scn_zone_t *psz) {
	char			 buf[256], /* XXX */
					*pstr;
	hkeys			*phk;
	scn_node_t		*pn;
	size_t			 ofst,
					 lm,
					 i,
					 j;

	/* XXX more comments */
	phk = hash_keys_sorted(psz->objects);
	if (phk == NULL) {
		/* nothing to do */
		return;
	}

	fprintf(fp, "\n# object dependency lists\n");
	for (i = 0 ; i < phk->nkey ; i++) {
		pstr = hash_get(psz->objects, phk->keys[i]);
		pn = hash_get(psz->nodes, pstr);

		/* object label */
		snprintf(buf, sizeof(buf), MKF_OBJECT_SRCS, pn->prefix);
		str_to_upper(buf, sizeof(buf), buf);
		fprintf(fp, buf);

		lm = strlen(buf);
		ofst = lm;

		/* append sources */
		for (j = 0 ; j < da_usize(pn->src_deps) ; j++) {
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp,
									(char *) da_idx(pn->src_deps, j));
		}

		fprintf(fp, MKF_TWICE_JUMP);
	}
	hash_free_hkeys(phk);
}


/*********************
 * mkf_output_objs() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_objs(FILE *fp, scn_zone_t *psz) {
	char			 buf[256], /* XXX */
					*pstr;
	hkeys			*phk;
	scn_node_t		*pn;
	size_t			 ofst,
					 lm,
					 i,
					 j;

	phk = hash_keys_sorted(psz->targets);
	if (phk != NULL) {
		/* generate target deps */
		fprintf(fp, "\n# target dependency lists\n");
		for (i = 0 ; i < phk->nkey ; i++) {
			pstr = hash_get(psz->targets, phk->keys[i]);
			pn = hash_get(psz->nodes, pstr);

			/* target label */
			snprintf(buf, sizeof(buf), MKF_TARGET_OBJS, pn->prefix);
			str_to_upper(buf, sizeof(buf), buf);
			fprintf(fp, buf);

			lm = strlen(buf);
			ofst = lm;

			/* append objects */
			for (j = 0 ; j < da_usize(pn->obj_deps) ; j++) {
				ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp,
										(char *) da_idx(pn->obj_deps, j));
			}

			fprintf(fp, MKF_TWICE_JUMP);
		}

		hash_free_hkeys(phk);
	}
}


/*************************
 * mkf_output_bld_trgs() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_bld_trgs(FILE *fp, scn_zone_t *psz) {
	char			 buf[256];
	hkeys			*phk;
	size_t			 ofst,
					 lm,
					 i;

	/* generate main building target list */
	fprintf(fp, MKF_TRGT_ALL_VAR);

	/* list of targets */
	phk = hash_keys_sorted(psz->targets);
	if (phk != NULL) {
		lm = strlen(MKF_TRGT_ALL_VAR);
		ofst = lm;
		for (i = 0 ; i < phk->nkey ; i++) {
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst,
												fp, phk->keys[i]);
		}
	}
	fprintf(fp, MKF_TWICE_JUMP);

	/* generate main cleaning target list */
	fprintf(fp, MKF_TRGT_CLEAN_VAR);

	/* list of targets */
	if (phk != NULL) {
		lm = strlen(MKF_TRGT_CLEAN_VAR);
		ofst = lm;
		for (i = 0 ; i < phk->nkey ; i++) {
			snprintf(buf, sizeof(buf), "%s_clean", phk->keys[i]);
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, buf);
		}
		hash_free_hkeys(phk);
	}
	fprintf(fp, MKF_TWICE_JUMP);

	/* main installing target list */
	fprintf(fp, MKF_TRGT_INST_VAR);

	/* main deinstalling target list */
	fprintf(fp, MKF_TRGT_DEINST_VAR);
}


/*************************
 * mkf_output_man_trgs() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_man_trgs(FILE *fp, scn_zone_t *psz) {
	char		 buf[256], /* XXX */
				*pstr;
	size_t		 ofst,
				 lm,
				 i,
				 j,
				 k;

	if (psz->found[FILE_TYPE_MAN] == false) {
		/* no man page, skip */
		return;
	}

	/* generate man page lists */
	for (i = 1 ; i < 10 ; i++) {
		snprintf(buf, sizeof(buf), MKF_FILE_MAN_VAR, (int) i);

		fprintf(fp, buf);

		lm = strlen(buf);
		ofst = lm;

		/* for each man page */
		for (j = 0 ; j < da_usize(psz->manpgs) ; j++) {
			/* get the last character */
			pstr = da_idx(psz->manpgs, j);
			k = strlen(pstr) - 1;
/*debugf("man page = '%s', mp_cat = %c, loop_cat = %d", pstr, pstr[k], i);*/

			/*
					if the numeric conversion of the character is equal
					to the current man page category
			*/
			if ((size_t) atoi(&pstr[k]) == i) {
					/* record it into the list */
					ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, pstr);
			}
		}

		fprintf(fp, MKF_TWICE_JUMP);
	}

	fprintf(fp, MKF_LINE_JUMP);
}


/*************************
 * mkf_output_man_trgs() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_data_trgs(FILE *fp, scn_zone_t *psz) {
	char		 buf[256], /* XXX */
				*pstr;
	size_t		 ofst,
				 lm,
				 i;

	/* data files */
	fprintf(fp, MKF_FILE_DATA_VAR);

	lm = strlen(buf);
	ofst = lm;

	/* for each man page */
	for (i = 0 ; i < da_usize(psz->datafiles) ; i++) {
		pstr = da_idx(psz->datafiles, i);

		/* record it into the list */
		ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, pstr);
	}

	fprintf(fp, MKF_TWICE_JUMP);
}


/**************************
 * mkf_output_obj_rules() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_obj_rules(FILE *fp, scn_zone_t *psz) {
	char			 buf[256], /* XXX */
					*pstr;
	hkeys			*phk;
	scn_node_t		*pn;
	size_t			 i;

	phk = hash_keys_sorted(psz->objects);
	if (phk == NULL) {
		/* nothing to do */
		return;
	}

	fprintf(fp, "\n# object rules\n");
	for (i = 0 ; i < phk->nkey ; i++) {
		pstr = hash_get(psz->objects, phk->keys[i]);
		pn = hash_get(psz->nodes, pstr);

		/* object label */
		str_to_upper(buf, sizeof(buf), pn->prefix);
		fprintf(fp, MKF_OBJECT_LABL, phk->keys[i], buf);

		fprintf(fp, MKF_TWICE_JUMP);
	}
	hash_free_hkeys(phk);
}


/**************************
 * mkf_output_trg_rules() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
************************************************************************/

void mkf_output_trg_rules(FILE *fp, scn_zone_t *psz) {
	char			 buf[256], /* XXX */
					*pstr;
	hkeys			*phk;
	size_t			 i;

	phk = hash_keys_sorted(psz->targets);
	if (phk == NULL) {
		/* nothing to do */
		return;
	}

	if (phk != NULL) {
		/* generate targets */
		fprintf(fp, "\n# target rules\n");
		for (i = 0 ; i < phk->nkey ; i++) {
			pstr = phk->keys[i];

			/* uppercase string */
			str_to_upper(buf, sizeof(buf), pstr);

			/* build target */
			fprintf(fp, MKF_TARGET_LABL, pstr, buf, pstr, buf);

			/* clean target */
			fprintf(fp, MKF_TARGET_CLN, pstr, buf, pstr);
		}
		hash_free_hkeys(phk);
	}
}


/*************************
 * mkf_output_man_inst() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_man_inst(FILE *fp, scn_zone_t *psz) {
	unsigned int	 i;

	if (psz->found[FILE_TYPE_MAN] == false) {
		/* no man page, skip */
		return;
	}

	fprintf(fp, MKF_INST_MAN_H);
	/* manual pages to install/deinstall */
	for (i = 1 ; i <10 ; i++) {
		/* XXX TODO gather list of man categories */
		fprintf(fp, MKF_INST_MAN_B, i, i, i, i);
	}
	fprintf(fp, MKF_TWICE_JUMP);

	fprintf(fp, MKF_DEINST_MAN_H);
	/* manual pages to install/deinstall */
	for (i = 1 ; i <10 ; i++) {
		/* XXX TODO gather list of man categories */
		fprintf(fp, MKF_DEINST_MAN_B, i, i, i);
	}
	fprintf(fp, MKF_TWICE_JUMP);
}


/*********************
 * scan_output_mkf() *
 ***********************************************************************
 DESCR
	build makefile using gathered data

 IN
	fname :	file name
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

bool scan_build_mkf(char *fname, scn_zone_t *psz) {
	FILE			*fp;
	/*char			 buf[256];*/

	fp = fopen(fname, "w");
	if (fp == NULL) {
		/* XXX better err msg */
		errorf("unable to open '%s'.", fname);
		return(false);
	}

	/* generate header and definitions */
	mkf_output_header(fp, psz);

	/* generate object dependency lists */
	mkf_output_srcs(fp, psz);

	/* generate target dependency lists */
	mkf_output_objs(fp, psz);

	/* generate building and cleaning target list */
	mkf_output_bld_trgs(fp, psz);

	/* binaries to install/deinstall */
	fprintf(fp, MKF_FILE_BIN_VAR);
	fprintf(fp, MKF_FILE_SBIN_VAR);

	/* manual pages to install/deinstall */
	mkf_output_man_trgs(fp, psz);

	/* data files */
	mkf_output_data_trgs(fp, psz);

	fprintf(fp, "\n# generic targets\n");
	fprintf(fp, MKF_TARGET_ALL);
	fprintf(fp, MKF_TARGET_INST);
	fprintf(fp, MKF_INST_BIN);
	fprintf(fp, MKF_DEINST_BIN);

	mkf_output_man_inst(fp, psz);

	fprintf(fp, MKF_INST_DATA);
	fprintf(fp, MKF_DEINST_DATA);

	fprintf(fp, MKF_TWICE_JUMP);

	/* generate objects */
	mkf_output_obj_rules(fp, psz);

	/* generate targets */
	mkf_output_trg_rules(fp, psz);

	fclose(fp);

	return(true);
}


/********************
 * common functions *
 ***********************************************************************/

/******************
 * str_to_upper() *
 ***********************************************************************
 DESCR
	store in the buffer the conversion in upper case of the string

 IN
	buf :	storage buffer
	siz :	size of buffer
	str :	string to convert

 OUT
	NONE
 ***********************************************************************/

void str_to_upper(char *buf, size_t siz, char *str) {
	while ((siz > 1) && (*str != '\0')) {
		*buf = toupper(*str);
		buf++;
		str++;
		siz--;
	}

	*buf = '\0';
}


/********************
 * check_file_ext() *
 ***********************************************************************
 DESCR
	check the file extension and return the supposed file type

 IN
	fname :	file name

 OUT
	file type
 ***********************************************************************/

ftype_t check_file_ext(char *fname) {
	int		 i;

	for (i = 0 ; i < (int) nb_file_ext ; i++) {
#ifdef PMKSCAN_DEBUG
		/*debugf("check '%s' extension with file '%s'", file_ext[i].ext, fname);*/
#endif
		/* check files that match known extension */
		if (fnmatch(file_ext[i].ext, fname, 0) != FNM_NOMATCH) {
			/* exit the loop */
			return(file_ext[i].type);
		}
	}

	/* unknown type */
	return(FILE_TYPE_UNKNOWN);
}


/*****************
 * regex_check() *
 ***********************************************************************
 DESCR
	check a line with regex pattern

 IN
	pattern : pattern to use
	line : string to check

 OUT
	return : found string or NULL
 ***********************************************************************/

char *regex_check(char *pattern, char *line) { /* XXX remove ? */
	static char	 idtf[TMP_BUF_LEN];
	regex_t		 re;
	regmatch_t	 rm[2];

	if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
		regfree(&re);
		return(NULL);
	}

	if (regexec(&re, line, 2, rm, 0) != 0) {
		return(NULL);
	}

	/* copy header name */
	strlcpy(idtf, (char *) (line + rm[1].rm_so),
			(size_t) (rm[1].rm_eo - rm[1].rm_so + 1)); /* no check ! */

	regfree(&re);

	return(idtf);
}


/******************
 * process_ppro() *
 ***********************************************************************
 DESCR
	called when a preprocessor directive is found

 IN
	data :	parsing data
	pstr :	directive identifier
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool process_ppro(void *data, char *pstr, prseng_t *ppe) {
	char		 iname[MAXPATHLEN],
				 c;
	scn_node_t	*pnode;
	scn_zone_t	*psz;

	psz = (scn_zone_t *) data;
	pnode = psz->pnode;

	if (strncmp(pstr, RKW_PP_INCL, strlen(pstr) + 1) == 0) {
		prs_c_skip(ppe); /* XXX check ? */

		c = prseng_get_char(ppe);

		prseng_next_char(ppe); /* XXX check */

		prseng_get_idtf(ppe, iname, sizeof(iname), PRS_C_IDTF_FNAME); /* XXX check ? */

		switch(c) {
			case '"' :
#ifdef PMKSCAN_DEBUG
				debugf("found local include '%s'", iname);
#endif
				/* add include in depedencies */
				if (da_push(pnode->local_inc, strdup(iname)) == false) {
					errorf("unable to add '%s' in local deps", iname);
					return(false);
				}

				break;

			case '<' :
#ifdef PMKSCAN_DEBUG
				debugf("found system include '%s'", iname);
#endif
				/* add include in depedencies */
				if (da_push(pnode->system_inc, strdup(iname)) == false) {
					errorf("unable to add '%s' in sys deps", iname);
					return(false);
				}
				break;

			default :
				return(false);
		}
	}

	prs_c_line_skip(ppe); /* XXX check ? */

	return(true);
}


/***********************
 * process_proc_call() *
 ***********************************************************************
 DESCR
	called when a procedure call is found

 IN
	data :	parsing data
	pstr :	function call identifier
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool process_proc_call(void *data, char *pstr, prseng_t *ppe) {
	scn_node_t	*pnode;
	scn_zone_t	*psz;

	psz = (scn_zone_t *) data;
	pnode = psz->pnode;

#ifdef PMKSCAN_DEBUG
	debugf("found procedure call of '%s'", pstr);
#endif
	/* add function in list */
	if (da_push(pnode->func_calls, strdup(pstr)) == false) {
		errorf("unable to add '%s' in function call list", pstr);
		return(false);
	}

	return(true);
}


/***********************
 * process_proc_decl() *
 ***********************************************************************
 DESCR
	called when a procedure declaration is found

 IN
	data :	parsing data
	pstr :	function declarator identifier
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool process_proc_decl(void *data, char *pstr, prseng_t *ppe) {
	scn_node_t	*pnode;
	scn_zone_t	*psz;

	psz = (scn_zone_t *) data;
	pnode = psz->pnode;

#ifdef PMKSCAN_DEBUG
	debugf("found procedure declaration of '%s'", pstr);
#endif
	/* add function in list */
	if (da_push(pnode->func_decls, strdup(pstr)) == false) {
		errorf("unable to add '%s' in function declaration list", pstr);
		return(false);
	}

	/* check for main procedure */
	if (strncmp(pstr, PSC_MAIN_C, strlen(pstr)) == 0) {
#ifdef PMKSCAN_DEBUG
		debugf("found main procedure '%s' in '%s'", pstr, pnode->fname);
#endif
		pnode->mainproc = true;
	}

	return(true);
}


/******************
 * process_type() *
 ***********************************************************************
 DESCR
	called when a type identifier is found

 IN
	data :	parsing data
	pstr :	type identifier
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool process_type(void *data, char *pstr, prseng_t *ppe) {
	scn_node_t	*pnode;
	scn_zone_t	*psz;

	psz = (scn_zone_t *) data;
	pnode = psz->pnode;

	/* add type in list */
	if (da_push(pnode->type_idtfs, strdup(pstr)) == false) {
		errorf("unable to add '%s' in type list", pstr);
		return(false);
	}

	return(true);
}


/****************
 * parse_file() *
 ***********************************************************************
 DESCR
	parse a file that has a known type

 IN
	pcmn :	common parser structure
	pnode :	scan node structure
	fname :	file to parse
	ft :	file type
	isdep : dependency flag

 OUT
	boolean
 ***********************************************************************/

bool parse_file(prs_cmn_t *pcmn, char *fname, ftype_t ft, bool isdep) {
	FILE			*fp;
	char			 buf[MAXPATHLEN],
					 dir[MAXPATHLEN];
	scn_node_t		*pnode;
	scn_zone_t		*psz;
	unsigned int	 i;

	/* get misc data */
	psz = (scn_zone_t *) pcmn->data;

	/* check if this node is already existing */
	pnode = hash_get(psz->nodes, fname);
	if (pnode == NULL) {
#ifdef PMKSCAN_DEBUG
		debugf("adding node for '%s'", fname); /* XXX */
#endif

		/* open file */
		fp = fopen(fname, "r");
		if (fp == NULL) {
			/*printf("Warning : cannot open '%s' : %s.\n", fname, strerror(errno));*/
			return(true);
		}

		/* create new node */
		pnode = scan_node_init(fname);
		if (pnode == NULL) {
			errorf("unable to initialize scan node");
			fclose(fp);
			return(false);
		}

		/* set curret node */
		psz->pnode = pnode;

		pnode->type = ft;

		switch (ft) {
			case FILE_TYPE_ASM :
				psz->found[FILE_TYPE_ASM] = true;
				psz->found_src = true;
				if (prs_asm_file(pcmn, fp) == false) {
					fclose(fp);
					return(false);
				}
				break;

			case FILE_TYPE_C :
			case FILE_TYPE_CXX :
				psz->found[FILE_TYPE_C] = true;
				psz->found_src = true;
				if (prs_c_file(pcmn, fp) == false) {
					fclose(fp);
					return(false);
				}
				break;
		}

		/* close file */
		fclose(fp);

		/* display  file as parsed */
		printf("P");
	}

	/* update dependency state */
	pnode->isdep = isdep;

	if (isdep == true) {
		/* update dependency score */
		pnode->score++;
#ifdef PMKSCAN_DEBUG
		debugf("score of '%s' = %d", pnode->fname, pnode->score);
#endif
	}

	/* add the node in the table of nodes */
	if (hash_add(psz->nodes, fname, pnode) == HASH_ADD_FAIL) {
		errorf("failed to add node '%s' in the hash table.", pnode->fname);
		scan_node_destroy(pnode);
		return(false);
	}

	/* get directory name */
	extract_dir(fname, dir, sizeof(dir));
	/* and store it for further use */
	pnode->dname = strdup(dir);

	for (i = 0 ; i < da_usize(pnode->local_inc) ; i++) {
		/* scan local include */
		build_path(dir, (char *)da_idx(pnode->local_inc, i), buf, sizeof(buf));

		if (scan_node_file(pcmn, buf, true) == false) {
			errorf("failed to scan file '%s'.", da_idx(pnode->local_inc, i));
			return(false);
		}
	}

	return(true);
}


/********************
 * scan_node_file() *
 ***********************************************************************
 DESCR
	scan a node file to extract useful data

 IN
	pcmn :	common parser structure
	fname :	file to scan
	isdep :	flag to notice if fname is a dependency or not

 OUT
	boolean
 ***********************************************************************/

bool scan_node_file(prs_cmn_t *pcmn, char *fname, bool isdep) {
	ftype_t			 ft;
	scn_zone_t		*psz;

#ifdef PMKSCAN_DEBUG
	debugf("fname = '%s'", fname);
#endif

	psz = (scn_zone_t *) pcmn->data;

	/* find a better fix or way to avoid to do that */
	if ((fname[0] == '.') && (fname[1] == '/')) {
		fname = fname + 2;
	}

	ft = check_file_ext(fname);
	switch (ft) {
		case FILE_TYPE_ASM :
		case FILE_TYPE_C :
		case FILE_TYPE_CXX :
			if (parse_file(pcmn, fname, ft, isdep) == false) {
				/* XXX err msg ? */
				return(false);
			}
			break;

		case FILE_TYPE_LEX :
			/* XXX TODO */
			break;

		case FILE_TYPE_YACC :
			/* XXX TODO */
			break;

		case FILE_TYPE_MAN :
			/* man pages will be processed later */
			psz->found[FILE_TYPE_MAN] = true;

			/* add man page in the list */
			da_push(psz->manpgs, strdup(fname)); /* XXX check ? */
#ifdef PMKSCAN_DEBUG
			debugf("added '%s' in psz->manpgs.", fname);
#endif
			break;

		case FILE_TYPE_DATA :
			/* data files will be processed later */
			psz->found[FILE_TYPE_DATA] = true;

			/* add data file in the list */
			da_push(psz->datafiles, strdup(fname)); /* XXX check ? */
#ifdef PMKSCAN_DEBUG
			debugf("added '%s' in psz->datafiles.", fname);
#endif
			break;

		default :
			/* skip unsupported file extension */

			/* display a file with unknown type */
			printf(".");
	}

	return(true);
}


/**************
 * scan_dir() *
 ***********************************************************************
 DESCR
	scan a directory to find and scan known file type

 IN
	pcmn :		common parser structure
	dir :		directory to scan
	recursive :	recursive flag

 OUT
	boolean
 ***********************************************************************/

bool scan_dir(prs_cmn_t *pcmn, char *dir, bool recursive) {
	struct dirent	*pde;
	struct stat		 tstat;
	DIR				*pd;
	char			 buf[MAXPATHLEN],
					*fname;
	dynary			*da;
	scn_zone_t		*psz;
	size_t			 i;

	psz = pcmn->data;

	if ((psz->discard != NULL) && (da_find(psz->discard, dir) == true)) {
		/* discard directory */
#ifdef PMKSCAN_DEBUG
		debugf("discarding '%s'", dir);
#endif
		return(true);
	}

	pd = opendir(dir);

	if (pd == NULL) {
		/* this is not a directory */
		return(false);
	}

	da = da_init();
	if (da == NULL) {
		/* XXX msg ? */
		closedir(pd);
		return(false);
	}

/*#ifdef PMKSCAN_DEBUG*/
	debugf("scanning directory '%s'", dir); /* XXX move to printf (or verbose) */
/*#endif*/

	/* check each directory's entries */
	while ((pde = readdir(pd)) && (pde != NULL)) {
		fname = pde->d_name;

#ifdef PMKSCAN_DEBUG
		debugf("checking entry '%s'", fname);
#endif

		if (*fname == '.') {
			/* skip every entries that starts with a dot */
#ifdef PMKSCAN_DEBUG
			debugf("skipping '%s'", fname);
#endif
			continue;
		}

		/* build full path */
		build_path(dir, fname, buf, sizeof(buf));
#ifdef PMKSCAN_DEBUG
		debugf("buf = '%s'", buf);
#endif

		if (stat(buf, &tstat) == -1) {
			continue;
		}

		/* if the entry is a directory ... */
		if ((tstat.st_mode & S_IFDIR) != 0) {
			/* ... then display a 'D' ... */
			printf("D");

			/* ... and if recursivity is enabled ... */
			if (recursive == true) {
				/* ... then display start of recurse ... */
				printf("[");

				/* ... then process the directory ... */
				/*scan_dir(pcmn, buf, recursive);*/
				da_push(da, strdup(buf));

				/* ... and then display end of recurse */
				printf("]");
			}

			/* go to next entry */
			continue;
		}

		if (scan_node_file(pcmn, buf, false) == false) {
#ifdef PMKSCAN_DEBUG
			debugf("failed"); /* XXX */
#endif
		}

		/* display a dot when an entry has been processed */
		printf(".");

	}
	closedir(pd);
	printf("\n");

	if (recursive == true) {
		for (i = 0 ; i < da_usize(da) ; i++) {
			scan_dir(pcmn, da_idx(da, i), true);
		}
	}
	da_destroy(da);

	return(true);
}


/******************
 * process_zone() *
 ***********************************************************************
 DESCR
	process a scanning zone

 IN
	pcmn :		common parser structure
	psd :		scanning data structure

 OUT
	boolean (true on success)
 ***********************************************************************/

bool process_zone(prs_cmn_t *pcmn, scandata *psd) {
	char		 fbuf[MAXPATHLEN];
	scn_zone_t	*psz;

	psz = pcmn->data;

	/* scanning directory */
	printf("Starting file parsing in '%s':\n", psz->directory);
	if (scan_dir(pcmn, psz->directory, psz->recursive) == false) {
		exit(EXIT_FAILURE);
	}
	printf("Parsing finished.\n\n");

	/* pmkfile stuff */
	if (psz->gen_pmk == true) {
		/* compare with known functions in pmkscan db */
		printf("Processing nodes for check generation ...\n");
		gen_checks(psz, psd);

		/* build fil name */
		build_path(psz->directory, PMKSCAN_PMKFILE, fbuf, sizeof(fbuf)); /* XXX check ? */

		/* generate pmkfile */
		printf("Generating %s ...\n", fbuf);
		scan_build_pmk(fbuf, psz, psd);
		printf("Ok\n\n");
	}

	/* makefile stuff */
	if (psz->gen_mkf == true) {
		/* scanning resulting nodes */
		printf("Processing nodes for object generation ...\n");
		gen_objects(psz);
		printf("Ok\n\n");

		/* scanning generated objects */
		printf("Processing objects for target generation ...\n");
		gen_targets(psz);
		printf("Ok\n\n");

		/* build fil name */
		build_path(psz->directory, PMKSCAN_MKFILE, fbuf, sizeof(fbuf)); /* XXX check ? */

		/* generate makefile */
		printf("Generating %s ...\n", fbuf);
		scan_build_mkf(fbuf, psz);
		printf("Ok\n\n");
	}

	return(true);
}


/******************
 * parse_script() *
 ***********************************************************************
 DESCR
	parse script file

 IN
	cfname :	configuration script filename
	pcmn :		common parser structure
	psd :		scanning data structure

 OUT
	boolean (true on success)
 ***********************************************************************/

bool parse_script(char *cfname, prs_cmn_t *pcmn, scandata *psd) {
	FILE		*fd;
	char		*pdir;
	htable		*tnodes;
	pmkobj		*ppo;
	prscell		*pcell;
	prsdata		*pdata;
	scn_zone_t	*psz;

	fd = fopen(cfname, "r");
	if (fd == NULL) {
		errorf("cannot open '%s' : %s.", cfname, strerror(errno));
		return(false);
	}

	/* initialise parsing data structure */
	pdata = prsdata_init();

	if (parse_pmkfile(fd, pdata, kw_scanfile, nbkwsf) == false) {
		fclose(fd);
		prsdata_destroy(pdata);
		errorf("parsing of script file failed.");
		return(false);
	}

	pcell = pdata->tree->first;
	while (pcell != NULL) {
		/* init of nodes table */
		tnodes = hash_init_adv(512, NULL,
				(void (*)(void *))scan_node_destroy, NULL);

		/* init zone structure */
		psz = scan_zone_init(tnodes);
		if (psz == NULL) {
			/* XXX err msg */
			prsdata_destroy(pdata);
			hash_destroy(tnodes);
			exit(EXIT_FAILURE);
		}
		pcmn->data = psz;

		switch(pcell->token) {
			case PSC_TOK_PMKF :
				/* generate pmkfile only */
				psz->gen_pmk = true;
				break;

			case PSC_TOK_MAKF :
				/* generate makefile only */
				psz->gen_mkf = true;

				/* optional name for makefile template */
				ppo = hash_get(pcell->data, KW_OPT_NAM);
				if (ppo != NULL) {
					/* set makefile name */
					psz->mkf_name = po_get_str(ppo);
				}
				break;

			case PSC_TOK_ZONE :
				/* mixed zone */

				/* get pmkfile switch */
				ppo = hash_get(pcell->data, KW_OPT_PMK);
				if (ppo != NULL) {
					psz->gen_pmk = po_get_bool(ppo);
				}

				/* get makefile switch */
				ppo = hash_get(pcell->data, KW_OPT_MKF);
				if (ppo != NULL) {
					psz->gen_mkf = po_get_bool(ppo);

					/* optional name for makefile template */
					ppo = hash_get(pcell->data, KW_OPT_NAM);
					if (ppo != NULL) {
						/* set makefile name */
						psz->mkf_name = po_get_str(ppo);
					}
				}
				break;

			default :
				scan_zone_destroy(psz);
				hash_destroy(tnodes);
				prsdata_destroy(pdata);
				errorf("parsing of script file failed.");
				return(false);
				break;
		}

		/* get base directory (REQUIRED) */
		pdir = po_get_str(hash_get(pcell->data, KW_OPT_DIR));
		psz->directory = strdup(pdir);

		/* get discard list */
		ppo = hash_get(pcell->data, KW_OPT_DSC);
		if (ppo != NULL) {
			psz->discard = po_get_list(ppo);
		}

		/* get recursivity switch (OPTIONAL, false by default) */
		ppo = hash_get(pcell->data, KW_OPT_REC);
		if (ppo != NULL) {
			psz->recursive = po_get_bool(ppo);
		}

		/* get unique file switch (OPTIONAL, false by default) */
		ppo = hash_get(pcell->data, KW_OPT_UNI);
		if (ppo != NULL) {
			psz->unique = po_get_bool(ppo);
		}

		/* process current zone */
		process_zone(pcmn, psd);

		scan_zone_destroy(psz);
		hash_destroy(tnodes);

		pcell = pcell->next;
	}

	prsdata_destroy(pdata);

	return(true);
}


/***********
 * usage() *
 ***********************************************************************
 DESCR
	pmkscan(1) usage

 IN
	NONE

 OUT
	NONE
 ***********************************************************************/

void usage(void) {
	fprintf(stderr, "usage: pmkscan [-f file] [-hmpuv] [path]\n");
}


/**********
 * main() *
 ***********************************************************************
 DESCR
	main loop.
 ***********************************************************************/

int main(int argc, char *argv[]) {
	bool		 go_exit = false,
				 use_script = false,
				 unique_file = false,
				 recursive = false,
				 gen_mkf = false,
				 gen_pmk = false;
	char		 buf[MAXPATHLEN],
				*scfile = NULL;
	htable		*tnodes;
	int			 chr;
	prs_cmn_t	 pcmn;
	prsdata		*pdata = NULL; /* XXX into global struct ? */
	scandata	 sd;
	scn_zone_t	*psz;

	while (go_exit == false) {
		chr = getopt(argc, argv, "f:hmpruv");
		switch (chr) {
			case -1 :
				go_exit = true;
				break;

			case 'f' :
				/* use script file */
				use_script = true;
				scfile = optarg;
				break;

			case 'm' :
				/* enable makefile generation */
				gen_mkf = true;
				break;

			case 'p' :
				/* enable pmkfile generation */
				gen_pmk = true;
				break;

			case 'r' :
				/* enable pmkfile generation */
				recursive = true;
				fprintf(stdout, "recursive option disabled until better support\n");
				break;

			case 'u' :
				/* use unique file (with recursive mode) */
				unique_file = true;
				break;

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

	argc = argc - optind;
	argv = argv + optind;

	printf("PMKSCAN version %s\n\n", PREMAKE_VERSION);


	printf("Initializing data ... \n");

	/* init common parser structure */
	pcmn.func_ppro = &process_ppro;
	pcmn.func_proc = &process_proc_call;
	pcmn.func_decl = &process_proc_decl;
	pcmn.func_type = &process_type;
	pcmn.data = NULL; /* will be updated later */

	if ((gen_pmk == true) || (use_script == true)) {
		/* initialise parsing data structure */
		pdata = prsdata_init();
		if (pdata == NULL) {
			errorf("\ncannot initialize prsdata.");
			exit(EXIT_FAILURE);
		} else {
			if (parse_data_file(pdata, &sd) == false) {
				/* error message displayed by parse_data_file */
				prsdata_destroy(pdata);
				exit(EXIT_FAILURE);
			}
		}
	}

	/* if a script file has been provided */
	if (use_script == true) {
		printf("Using scanning script '%s'.\n", scfile);

		if (parse_script(scfile, &pcmn, &sd) == false) {
			exit(EXIT_FAILURE);
		}

		prsdata_destroy(pdata);

		exit(EXIT_SUCCESS);
	}

	/* init of nodes table */
	tnodes = hash_init_adv(512, NULL,
			(void (*)(void *))scan_node_destroy, NULL);

	/* init zone structure */
	psz = scan_zone_init(tnodes);
	if (psz == NULL) {
		/* XXX err msg */
		exit(EXIT_FAILURE);
	}
	pcmn.data = psz;

	if (argc != 0) {
		/* use optional path */
		if (strlcpy_b(buf, argv[0], sizeof(buf)) == false) {
			errorf("failed to set buffer.");
			exit(EXIT_FAILURE);
		}
	} else {
		strlcpy(buf, ".", sizeof(buf)); /* should not fail */
	}

	/* end of init */
	printf("Ok\n\n");

	/* scanning directory */
	printf("Starting file parsing :\n");
	if (scan_dir(&pcmn, buf, recursive) == false) {
		exit(EXIT_FAILURE);
	}
	printf("Parsing finished.\n\n");

	/* pmkfile stuff */
	if (gen_pmk == true) {
		printf("Processing nodes for check generation ...\n");
		gen_checks(psz, &sd);

		printf("Generating %s ...\n", PMKSCAN_PMKFILE);
		scan_build_pmk(PMKSCAN_PMKFILE, psz, &sd);
		printf("Ok\n\n");
	}

	/* makefile stuff */
	if (gen_mkf == true) {
		/* scanning resulting nodes */
		printf("Processing nodes for object generation ...\n");
		gen_objects(psz);
		printf("Ok\n\n");

		/* scanning generated objects */
		printf("Processing objects for target generation ...\n");
		gen_targets(psz);
		printf("Ok\n\n");

		printf("Generating %s ...\n", PMKSCAN_MKFILE);
		scan_build_mkf(PMKSCAN_MKFILE, psz);
		printf("Ok\n\n");
	}

	/* cleaning */
	scan_zone_destroy(psz);
	hash_destroy(tnodes);
	if (gen_pmk == true) {
		prsdata_destroy(pdata);
	}

	printf("\nWork complete.\n\n");

	return(0);
}

