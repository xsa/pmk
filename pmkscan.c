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
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "dynarray.h"
#include "hash_tools.h"
#include "parse.h"
#include "pathtools.h"
#include "pmkscan.h"
#include "premake.h"
#include "tags.h"


/*#define PMKSCAN_DEBUG	1*/


/* file types **********************************************************/

scn_ext_t	 file_ext[] = {
		{"*.1",		FILE_TYPE_MAN1},
		{"*.2",		FILE_TYPE_MAN2},
		{"*.3",		FILE_TYPE_MAN3},
		{"*.4",		FILE_TYPE_MAN4},
		{"*.5",		FILE_TYPE_MAN5},
		{"*.6",		FILE_TYPE_MAN6},
		{"*.7",		FILE_TYPE_MAN7},
		{"*.8",		FILE_TYPE_MAN8},
		{"*.9",		FILE_TYPE_MAN9},
		{"*.asm",	FILE_TYPE_ASM},
		{"*.C",		FILE_TYPE_C},
		{"*.c",		FILE_TYPE_C},
		{"*.cc",	FILE_TYPE_C},
		{"*.cpp",	FILE_TYPE_CXX},
		{"*.cxx",	FILE_TYPE_CXX},
		{"*.c++",	FILE_TYPE_CXX},
		{"*.dat",	FILE_TYPE_DATA},
		{"*.gif",	FILE_TYPE_IMG},
		{"*.H",		FILE_TYPE_C},
		{"*.h",		FILE_TYPE_C},
		{"*.hh",	FILE_TYPE_C},
		{"*.hpp",	FILE_TYPE_CXX},
		{"*.html",	FILE_TYPE_HTML},
		{"*.htm",	FILE_TYPE_HTML},
		{"*.hxx",	FILE_TYPE_CXX},
		{"*.h++",	FILE_TYPE_CXX},
		{"*.in",	FILE_TYPE_TEMPL},
		{"*.jpg",	FILE_TYPE_IMG},
		{"*.jpeg",	FILE_TYPE_IMG},
		{"*.l", 	FILE_TYPE_LEX},
		{"*.pmk",	FILE_TYPE_TEMPL},
		{"*.png",	FILE_TYPE_IMG},
		{"*.S",		FILE_TYPE_ASM},
		{"*.s",		FILE_TYPE_ASM},
		{"*.txt",	FILE_TYPE_TEXT},
		{"*.xpm",	FILE_TYPE_IMG},
		{"*.y", 	FILE_TYPE_YACC}
};
size_t	 nb_file_ext = sizeof(file_ext) / sizeof(scn_ext_t);


/* pmkscan data parser options *****************************************/

/* common required options */
kw_t	req_name[] = {
	{KW_OPT_NAM,	PO_STRING}
};

/* ADD_HEADER options */
kw_t	opt_addhdr[] = {
	{KW_OPT_LIB,	PO_STRING},
	{KW_OPT_PRC,	PO_LIST},
	{KW_OPT_SUB,	PO_LIST}
};

kwopt_t	kw_addhdr = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_addhdr,
	sizeof(opt_addhdr) / sizeof(kw_t)
};

/* ADD_LIBRARY options */
kw_t	opt_addlib[] = {
	{KW_OPT_PRC,	PO_LIST}
};

kwopt_t	kw_addlib = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_addlib,
	sizeof(opt_addlib) / sizeof(kw_t)
};

/* ADD_TYPE options */
kw_t	opt_addtyp[] = {
	{KW_OPT_HDR,	PO_STRING},
	{KW_OPT_MBR,	PO_LIST}
};

kwopt_t	kw_addtyp = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_addtyp,
	sizeof(opt_addtyp) / sizeof(kw_t)
};

prskw	kw_pmkscan[] = {
	{KW_CMD_ADDHDR,	PSC_TOK_ADDHDR,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_addhdr},
	{KW_CMD_ADDLIB,	PSC_TOK_ADDLIB,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_addlib},
	{KW_CMD_ADDTYP,	PSC_TOK_ADDTYP,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_addtyp}
};
size_t	nbkwps = sizeof(kw_pmkscan) / sizeof(prskw);


/* pmkscan script parser options ***************************************/

/* common required options */
kw_t	req_dir[] = {
	{KW_OPT_DIR,	PO_STRING}
};

/* GEN_PMKFILE options */
kw_t	opt_genpmk[] = {
	{KW_OPT_ADVTAG,	PO_BOOL},
	{KW_OPT_CFGALT,	PO_STRING},
	{KW_OPT_DSC,	PO_LIST},
	{KW_OPT_EXTTAG,	PO_LIST},
	{KW_OPT_PMKALT,	PO_STRING},
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
	{KW_OPT_EXTMKF,	PO_STRING},
	{KW_OPT_EXTTAG,	PO_LIST},
	{KW_OPT_NAM,	PO_STRING},
	{KW_OPT_MKFALT,	PO_STRING},
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
	{KW_OPT_ADVTAG,	PO_BOOL},
	{KW_OPT_CFGALT,	PO_STRING},
	{KW_OPT_DSC,	PO_LIST},
	{KW_OPT_EXTMKF,	PO_STRING},
	{KW_OPT_EXTTAG,	PO_LIST},
	{KW_OPT_MKF,	PO_BOOL},
	{KW_OPT_NAM,	PO_STRING},
	{KW_OPT_MKFALT,	PO_STRING},
	{KW_OPT_PMK,	PO_BOOL},
	{KW_OPT_PMKALT,	PO_STRING},
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
	{KW_CMD_GENPF,		PSC_TOK_PMKF,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_genpmk},
	{KW_CMD_GENMF,		PSC_TOK_MAKF,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_genmkf},
	{KW_CMD_GENZN,		PSC_TOK_ZONE,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_genzone}
};
size_t	nbkwsf = sizeof(kw_scanfile) / sizeof(prskw);


/* global variables ****************************************************/

/* log file descriptor */
FILE	*fp_log = NULL;

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
		pnode->sys_deps = da_init();
		if (pnode->sys_deps == NULL) {
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

		/* init object dependency list */
		pnode->obj_deps = NULL;

		/* init dependency state */
		pnode->isdep = false; /* XXX useful ? see scan_file() */

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
#endif /* PMKSCAN_DEBUG */
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

	if (pnode->sys_deps != NULL) {
		da_destroy(pnode->sys_deps);
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
		pzone->advtag = true;
		pzone->gen_pmk = false;
		pzone->gen_mkf = false;
		pzone->recursive = false;
		pzone->unique = true;

		/* init file names */
		pzone->cfg_name = PMKSCAN_CFGFILE;
		pzone->mkf_name = PMKSCAN_MKFILE;
		pzone->pmk_name = PMKSCAN_PMKFILE;
		pzone->ext_mkf = NULL;

		/* discard list */
		pzone->discard = NULL;

		/* extra tags */
		pzone->exttags = NULL;

		/* init file type flags */
		for (i = 0 ; i < sizeof(pzone->found) ; i++) {
			pzone->found[i] = false;
		}

		/* init source flag */
		pzone->found_src = false;

		/* init zone tables */
		pzone->objects = hash_init(256); /* XXX can do better :) */
		pzone->targets = hash_init(256); /* XXX can do better :) */
		pzone->h_checks = hash_init_adv(128, NULL, (void (*)(void *)) destroy_chk_cell, NULL); /* XXX can do better :) */
		pzone->l_checks = hash_init_adv(128, NULL, (void (*)(void *)) destroy_chk_cell, NULL); /* XXX can do better :) */
		pzone->t_checks = hash_init_adv(128, NULL, (void (*)(void *)) destroy_chk_cell, NULL); /* XXX can do better :) */

		/* init directory list to scan */
		pzone->dirlist = da_init();

		/* init man pages dynary */
		pzone->manpgs = da_init();

		/* init data files dynary */
		pzone->datafiles = da_init();

		/* init templates dynary */
		pzone->templates = da_init();

		/* init tags dynary */
		pzone->tags = da_init();
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

	if (pzone->h_checks != NULL) {
		hash_destroy(pzone->h_checks);
	}

	if (pzone->l_checks != NULL) {
		hash_destroy(pzone->l_checks);
	}

	if (pzone->t_checks != NULL) {
		hash_destroy(pzone->t_checks);
	}

	if (pzone->dirlist != NULL) {
		da_destroy(pzone->dirlist);
	}

	if (pzone->manpgs != NULL) {
		da_destroy(pzone->manpgs);
	}

	if (pzone->datafiles != NULL) {
		da_destroy(pzone->datafiles);
	}

	if (pzone->templates != NULL) {
		da_destroy(pzone->templates);
	}

	if (pzone->tags != NULL) {
		da_destroy(pzone->tags);
	}

	/* don't destroy discard and extra tags lists */

	free(pzone);
}


/******************************
 * pmkfile specific functions *
 ***********************************************************************/

/*******************
 * init_chk_cell() *
 ***********************************************************************
 DESCR
	initialize check cell

 IN
	NONE

 OUT
	pointer to new cell or NULL
 ***********************************************************************/

check_t *init_chk_cell(char *name) {
	check_t	*pchk;

	pchk = (check_t *) malloc(sizeof(check_t));
	if (pchk == NULL) {
		return(NULL);
	}

	/* set check name */
	pchk->name = strdup(name);
	if (pchk->name == NULL) {
		free(pchk);
		return(NULL);
	}

	/* init procedure list */
	pchk->procs = da_init();
	if (pchk->procs == NULL) {
		free(pchk->name);
		free(pchk);
		return(NULL);
	}

	/* init misc */
	pchk->header = NULL;
	pchk->library = NULL;
	pchk->member = NULL;
	pchk->subhdrs = NULL;

	return(pchk);
}


/**********************
 * destroy_chk_cell() *
 ***********************************************************************
 DESCR
	destroy allocated check cell

 IN
	pchk :	check cell structure

 OUT
	NONE
 ***********************************************************************/

void destroy_chk_cell(check_t *pchk) {
	free(pchk->name);
	da_destroy(pchk->procs);
	free(pchk);
}


/*****************
 * mk_chk_cell() *
 ***********************************************************************
 DESCR
	make check cell

 IN
	pht :	parser hash table
	token :	cell type token

 OUT
	pointer to new cell or NULL
 ***********************************************************************/

check_t *mk_chk_cell(htable *pht, int token) {
	check_t	*pchk;

	pchk = (check_t *) malloc(sizeof(check_t));
	if (pchk != NULL) {
		/* get the name */
		pchk->name = po_get_str(hash_get(pht, KW_OPT_NAM));
		if (pchk->name == NULL) {
			free(pchk);
			return(NULL);
		}

		/* init */
		pchk->procs = NULL;
		pchk->header = NULL;
		pchk->library = NULL;
		pchk->member = NULL;
		pchk->subhdrs = NULL;

		/* specific stuff */
		switch (token) {
			case PSC_TOK_ADDHDR :
				/* get procedure list */
				pchk->procs = po_get_list(hash_get(pht, KW_OPT_PRC));

				/* get eventual related library */
				pchk->library = po_get_str(hash_get(pht, KW_OPT_LIB));

				/* get eventual sub headers */
				pchk->subhdrs = po_get_list(hash_get(pht, KW_OPT_SUB));
				break;

			case PSC_TOK_ADDLIB :
				/* get procedure list */
				pchk->procs = po_get_list(hash_get(pht, KW_OPT_PRC));
				break;

			case PSC_TOK_ADDTYP :
				/* get eventual header */
				pchk->header = po_get_str(hash_get(pht, KW_OPT_HDR));

				/* get eventual header */
				pchk->member = po_get_str(hash_get(pht, KW_OPT_MBR));
				break;
		}
	}

	return(pchk);
}


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
	check_t	*pchk;
	prscell	*pcell;

	fd = fopen(PMKSCAN_DATA, "r");
	if (fd == NULL) {
		errorf("cannot open '%s' : %s.",
			PMKSCAN_DATA, strerror(errno));
		return(false);
	}

	/* init hash tables */
	sdata->headers = hash_init_adv(256, NULL, free, NULL);
	sdata->libraries = hash_init_adv(256, NULL, free, NULL);
	sdata->types = hash_init_adv(256, NULL, free, NULL);

	rval = parse_pmkfile(fd, pdata, kw_pmkscan, nbkwps);
	fclose(fd);

	if (rval == false) {
		errorf("parsing of data file failed.");
		return(rval);
	}

	pcell = pdata->tree->first;
	while (pcell != NULL) {
		switch(pcell->token) {
			case PSC_TOK_ADDHDR :
				pchk = mk_chk_cell(pcell->data, pcell->token);
				if (pchk == NULL) {
					/* XXX err msg ? */
					return(false);
				}

				hash_add(sdata->headers, pchk->name, pchk); /* XXX check */
				break;

			case PSC_TOK_ADDLIB :
				pchk = mk_chk_cell(pcell->data, pcell->token);
				if (pchk == NULL) {
					/* XXX err msg ? */
					return(false);
				}

				hash_add(sdata->libraries, pchk->name, pchk); /* XXX check */
				break;

			case PSC_TOK_ADDTYP :
				pchk = mk_chk_cell(pcell->data, pcell->token);
				if (pchk == NULL) {
					/* XXX err msg ? */
					return(false);
				}

				hash_add(sdata->types, pchk->name, pchk); /* XXX check */
				break;

			default :
				errorf("parsing of data file failed.");
				return(false);
				break;
		}

		pcell = pcell->next;
	}

	return(rval);
}


/*******************
 * conv_to_label() *
 ***********************************************************************
 DESCR
 	convert a string to a tag

 IN
	str :		string to convert

 OUT
	pointer to the tag buffer
 ***********************************************************************/

char *conv_to_label(ftype_t ltype, char *fmt, ...) {
	static char	 buffer[TMP_BUF_LEN];
	char		*pbuf;
	size_t		 s;
	va_list		 plst;

	/* language prefix */
	switch(ltype) {
		case FILE_TYPE_C :
			s = strlcpy(buffer, PMKSCAN_LABEL_C, sizeof(buffer));
			break;

		case FILE_TYPE_CXX :
			s = strlcpy(buffer, PMKSCAN_LABEL_CXX, sizeof(buffer));
			break;

		default :
			s = 0;
	}

	/* adjust pointer to end of language prefix */
	pbuf = buffer + s;
	s = sizeof(buffer) - s;

	/* produce formatted string in the buffer */
	va_start(plst, fmt);
	vsnprintf(pbuf, s, fmt, plst);
	va_end(plst);

	/* process the given string */
	while ((*pbuf != '\0') && (s > 1)) {
		/* check if we have an alphanumeric character */
		if (isalnum(*pbuf) == 0) {
			/* no, replace by an underscore */
			*pbuf = '_';
		} else {
			/* yes, convert to uppercase if needed */
			*pbuf = (char) tolower((int) *pbuf);
		}

		/* next character */
		s--;
		pbuf++;
	}

	/* end of string */
	*pbuf = '\0';

	return(buffer);
}


/**********************
 * recurse_sys_deps() *
 ***********************************************************************
 DESCR
	gather recursively the system include dependencies

 IN
	nodes :		nodes hash table
	deps :		dynary structure where to store dependencies
	nodename :	node name to process recursively

 OUT
	boolean
 ***********************************************************************/

bool recurse_sys_deps(htable *nodes, dynary *deps, char *nodename) {
	char		 dir[PATH_MAX],
				*pstr;
	scn_node_t	*pnode;
	size_t		 i;

	/* get node structure */
	pnode = hash_get(nodes, nodename); /* should not fail */
	if (pnode == NULL) {
#ifdef PMKSCAN_DEBUG
		debugf("recurse_sys_deps() : node '%s' missing.", nodename);
#endif /* PMKSCAN_DEBUG */
		return(true);
	}

	/* process all system headers linked to this node */
	for (i = 0 ; i < da_usize(pnode->system_inc) ; i++) {
		pstr = (char *) da_idx(pnode->system_inc, i);

		/* check if the system header is already listed as a dependency */
		if (da_find(deps, pstr) == false) {
			/* no, add it into the list */
			if (da_push(deps, strdup(pstr)) == false) {
				return(false);
			}
		}
	}

	/* get directory */
	extract_dir(nodename, dir, sizeof(dir));

	/* look for all the local dependencies of the current node */
	for (i = 0 ; i < da_usize(pnode->local_inc) ; i++) {
		/* and recurse */
		if (recurse_sys_deps(nodes, deps, (char *) da_idx(pnode->local_inc, i)) == false) {
			return(false);
		}
	}

	return(true);
}


/*****************
 * add_library() *
 ***********************************************************************
 DESCR
	generate a library check if a record exists for the given library

 IN
 	checks :	checks hash table
	library :	library to add
	psd :		scandata structure
	pn: 		node structure

 OUT
	boolean
 ***********************************************************************/

bool add_library(scn_zone_t *psz, char *library, scandata *psd, scn_node_t *pn) {
	char	*label,
			*pstr,
			*tag,
			 tmp[256]; /* XXX */
	check_t	*pchk,
			*pcrec;
	size_t	 i,
			 s;

	/* try to retrieve a check record for the given header */
	pcrec = hash_get(psd->libraries, library);
	if (pcrec == NULL) {
		/* no record, skipping */
		return(true);
	}

	label = conv_to_label(pn->type, "library_%s", library);
	pchk = hash_get(psz->l_checks, label);
	if (pchk == NULL) {
		/* add new check */
		pchk = init_chk_cell(library);
		if (pchk == NULL) {
			/* allocation failed */
			/* XXX err msg ? */
			return(false);
		}

		if (hash_update(psz->l_checks, label, pchk) == HASH_ADD_FAIL) {
			return(false);
		}

		/* set language */
		pchk->ftype = pn->type;

		/* add header tag */
		snprintf(tmp, sizeof(tmp), "lib%s", library);
		tag = gen_basic_tag_def(tmp);
		if (da_find(psz->tags, tag) == NULL) {
			da_push(psz->tags, strdup(tag));
		}
	}

	/* look for related procedures */
	if (pcrec->procs != NULL) {
		s = da_usize(pcrec->procs);
		for (i = 0 ; i < s ; i++) {
			pstr = da_idx(pcrec->procs, i);

			/* if procedure has been found in parsing */
			if (da_find(pn->func_calls, pstr) != NULL) {

				/* if it has not been added in the check yet */
				if (da_find(pchk->procs, pstr) == NULL) {
					/* add in the list of function to check */
					da_push(pchk->procs, strdup(pstr));

					psc_log(NULL, "Node '%s': found procedure '%s'\n", pn->fname, pstr);

					/* add header tag */
					tag = gen_basic_tag_def(pstr);
					if (da_find(psz->tags, tag) == NULL) {
						da_push(psz->tags, strdup(tag));
					}
				}
			}
		}
	}

	return(true);
}


/******************
 * check_header() *
 ***********************************************************************
 DESCR
	generate an header check if a record exists for the given header

 IN
 	checks :	checks hash table
	header :	header to process
	psd :		scandata structure
	pn: 		node structure

 OUT
	boolean

 TODO
	add check if LIBRARY is specified
 ***********************************************************************/

bool check_header(scn_zone_t *psz, char *header, scandata *psd, scn_node_t *pn) {
	char	*label,
			*pstr,
			*tag;
	check_t	*pchk,
			*pcrec;
	size_t	 i,
			 s;

	/* try to retrieve a check record for the given header */
	pcrec = hash_get(psd->headers, header);
	if (pcrec == NULL) {
		/* no record, skipping */
		return(true);
	}

	label = conv_to_label(pn->type, "header_%s", header);
	pchk = hash_get(psz->h_checks, label);
	if (pchk == NULL) {
		/* add new check */
		pchk = init_chk_cell(header);
		if (pchk == NULL) {
			/* allocation failed */
			/* XXX err msg ? */
			return(false);
		}

		if (hash_update(psz->h_checks, label, pchk) == HASH_ADD_FAIL) {
			return(false);
		}

		/* set language */
		pchk->ftype = pn->type;

		/* link to eventual sub headers */
		pchk->subhdrs = pcrec->subhdrs;

		/* add AC style header tag */
		tag = gen_basic_tag_def(header);
		if (da_find(psz->tags, tag) == NULL) {
			da_push(psz->tags, strdup(tag));
		}

		/* add header tag */
		tag = gen_tag_def(TAG_TYPE_HDR, header, NULL, NULL);
		if (da_find(psz->tags, tag) == NULL) {
			da_push(psz->tags, strdup(tag));
		}
	}

	/* look for related procedures */
	if (pcrec->procs != NULL) {
		s = da_usize(pcrec->procs);
		for (i = 0 ; i < s ; i++) {
			pstr = da_idx(pcrec->procs, i);

			/* if procedure has been found in parsing */
			if (da_find(pn->func_calls, pstr) != NULL) {

				/* if it has not been added in the check yet */
				if (da_find(pchk->procs, pstr) == NULL) {
					/* add in the list of function to check */
					da_push(pchk->procs, strdup(pstr));

					psc_log(NULL, "Node '%s': found procedure '%s'\n", pn->fname, pstr);

					/* add AC style header tag */
					tag = gen_basic_tag_def(pstr);
					if (da_find(psz->tags, tag) == NULL) {
						da_push(psz->tags, strdup(tag));
					}

					/* add header tag */
					tag = gen_tag_def(TAG_TYPE_HDR_PRC, header, pstr, NULL);
					if (da_find(psz->tags, tag) == NULL) {
						da_push(psz->tags, strdup(tag));
					}
				}
			}
		}
	}

	if (pcrec->library != NULL) {
		if (add_library(psz, pcrec->library, psd, pn) == false) {
			return(false);
		}
	}

	return(true);
}


/****************
 * check_type() *
 ***********************************************************************
 DESCR
	generate a type check if a record exists for the given type

 IN
 	checks :	checks hash table
	type :		type to process
	psd :		scandata structure
	pn: 		node structure

 OUT
	boolean

 TODO
	handle member ?
 ***********************************************************************/

bool check_type(scn_zone_t *psz, char *type, scandata *psd, scn_node_t *pn) {
	char	*label,
			*pstr;
	check_t	*pchk,
			*pcrec;

	label = conv_to_label(pn->type, "type_%s", type);
	if (hash_get(psz->t_checks, label) != NULL) {
		/* check already exists */
		return(true);
	}

	/* try to retrieve a check record for the given header */
	pcrec = hash_get(psd->types, type);
	if (pcrec == NULL) {
		/* no record */
		return(true);
	}

	/* add new check */
	pchk = init_chk_cell(type);
	if (pchk == NULL) {
		/* allocation failed */
		/* XXX err msg ? */
		return(false);
	}

	/* set language */
	pchk->ftype = pn->type;

	if (pcrec->header != NULL) {
		pchk->header = pcrec->header; /* XXX strdup ? */
	}

	if (hash_update(psz->t_checks, label, pchk) == HASH_ADD_FAIL) {
		return(false);
	}

	/* add AC style header tag */
	pstr = gen_basic_tag_def(type);
	if (da_find(psz->tags, pstr) == NULL) {
		da_push(psz->tags, strdup(pstr));
	}

	/* add header tag */
	pstr = gen_tag_def(TAG_TYPE_TYPE, type, NULL, NULL);
	if (da_find(psz->tags, pstr) == NULL) {
		da_push(psz->tags, strdup(pstr));
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

	/* for each node */
	phk = hash_keys(psz->nodes);
	if (phk == NULL) {
		return(true);
	}

	for(i = 0 ; i < phk->nkey ; i++) {
		pn = hash_get(psz->nodes, phk->keys[i]); /* no check needed */

		/* check types */
		if (psd->types != NULL) {
			/* process types */
			for (j = 0 ; j < da_usize(pn->type_idtfs) ; j++) {
				pstr = (char *) da_idx(pn->type_idtfs, j);

				if (check_type(psz, pstr, psd, pn) == false) {
					errorf("check_type() failed.");
					hash_free_hkeys(phk);
					return(false);
				}
			}
		}

		/* check headers */
		if (psd->headers != NULL) {
			/* generate system dependencies */
			recurse_sys_deps(psz->nodes, pn->sys_deps, pn->fname);

			/* process system dependencies */
			for (j = 0 ; j < da_usize(pn->sys_deps) ; j++) {
				pstr = (char *) da_idx(pn->sys_deps, j);

				if (check_header(psz, pstr, psd, pn) == false) {
					errorf("check_header() failed.");
					hash_free_hkeys(phk);
					return(false);
				}
			}
		}
	}
	hash_free_hkeys(phk);

	return(true);
}


/*********************
 * build_cmd_begin() *
 ***********************************************************************
 DESCR
	output body start of a command

 IN
	fp :	file pointer
	cmd :	command name
	label :	command label

 OUT
	-
 ***********************************************************************/

void build_cmd_begin(FILE *fp, char *cmd, char *label) {
	if (label == NULL) {
		fprintf(fp, PMKF_CMD_NOLABEL, cmd);
	} else {
		fprintf(fp, PMKF_CMD_LABEL, cmd, label);
	}
}


/*******************
 * build_cmd_end() *
 ***********************************************************************
 DESCR
	output body end of a command

 IN
	fp :	file pointer

 OUT
	-
 ***********************************************************************/

void build_cmd_end(FILE *fp) {
	fprintf(fp, PMKF_CMD_END);
}


/*******************
 * build_comment() *
 ***********************************************************************
 DESCR
	output a comment line at the main level (no indent)

 IN
	fp :	file pointer
	comment :	comment text

 OUT
	-
 ***********************************************************************/

void build_comment(FILE *fp, char *fmt, ...) {
	va_list	 plst;

	/* build comment */
	fprintf(fp, PMKF_COMMENT);

	/* format string */
	va_start(plst, fmt);
	vfprintf(fp, fmt, plst);
	va_end(plst);

	fprintf(fp, "\n");

}


/******************
 * build_quoted() *
 ***********************************************************************
 DESCR
	output a quoted string assignment

 IN
	fp :	file pointer
	vname :	variable name
	qstr :	quoted string content

 OUT
	-
 ***********************************************************************/

void build_quoted(FILE *fp, char *vname, char *qstr) {
	fprintf(fp, PMKF_VAR_QUOTED, vname, qstr);
}


/*******************
 * build_boolean() *
 ***********************************************************************
 DESCR
	output a boolean assignment

 IN
	fp :	file pointer
	vname :	variable name
	bval :	boolean value

 OUT
	-
 ***********************************************************************/

void build_boolean(FILE *fp, char *vname, bool bval) {
	char	*str;

	if (bval == true) {
		str = "TRUE";
	} else {
		str = "FALSE";
	}

	fprintf(fp, PMKF_VAR_BOOL, vname, str);
}


/****************
 * build_list() *
 ***********************************************************************
 DESCR
	output a list assignment (if the list is not empty)

 IN
	fp :	file pointer
	vname :	variable name
	list :	item list

 OUT
	-
 ***********************************************************************/

bool build_list(FILE *fp, char *vname, dynary *list) {
	size_t	 i,
			 s;

	if (list == NULL) {
		return(true);
	}

	/* get number of items */
	s = da_usize(list);

	/* if no items then leave */
	if (s == 0) {
		return(false);
	}

	fprintf(fp, PMKF_VAR_LIST_BEG, vname);

	/* process all items but the last */
	s--;
	for (i = 0 ; i < s ; i++) {
		fprintf(fp, PMKF_VAR_LIST_ITEM, (char *) da_idx(list, i));
	}

	/* process last item */
	fprintf(fp, PMKF_VAR_LIST_END, (char *) da_idx(list, s));

	return(true);
}


/**************
 * set_lang() *
 ***********************************************************************
 DESCR

 IN
	buf :	storage buffer
	siz :	size of buffer
	ltype :	language type

 OUT
	boolean
 ***********************************************************************/

bool set_lang(FILE *fp, ftype_t ltype) {
	char	*lang;

	/* set current language */
	switch(ltype) {
		case FILE_TYPE_C :
			lang = PMKSCAN_LANG_C;
			break;

		case FILE_TYPE_CXX :
			lang = PMKSCAN_LANG_CXX;
			break;

		default :
			lang = NULL;
	}

	if (lang != NULL) {
		/* output language name */
		build_quoted(fp, "LANG", lang);
	}

	return(true);
}


/******************
 * ouput_header() *
 ***********************************************************************
 DESCR
	XXX

 IN
 	checks :	checks hash table
	header :	header to process
	psd :		scandata structure
	pn: 		node structure

 OUT
	boolean
 ***********************************************************************/

bool output_header(htable *checks, char *cname, FILE *fp) {
	check_t	*pchk;

	pchk = hash_get(checks, cname);
	if (pchk == NULL) {
		return(false);
	}

	/* output comment */
	build_comment(fp, "check header %s", pchk->name);

	/* output pmk command */
	build_cmd_begin(fp, "CHECK_HEADER", cname);

	/* output type name */
	build_boolean(fp, "REQUIRED", false);

	/* output header name */
	build_quoted(fp, "NAME", pchk->name);

	/* output language */
	if (set_lang(fp, pchk->ftype) == false) {
		return(false);
	}

	/* output list (already handle empty list) */
	build_list(fp, "FUNCTION", pchk->procs);

	/* output list (already handle empty list) */
	build_list(fp, "SUBHDR", pchk->subhdrs);

	/* output end of command body */
	build_cmd_end(fp);

	fprintf(fp, "\n");

	return(true);
}


/*******************
 * ouput_library() *
 ***********************************************************************
 DESCR
	XXX

 IN
 	checks :	checks hash table
	header :	header to process
	psd :		scandata structure
	pn: 		node structure

 OUT
	boolean
 ***********************************************************************/

bool output_library(htable *checks, char *cname, FILE *fp) {
	check_t	*pchk;

	pchk = hash_get(checks, cname);
	if (pchk == NULL) {
		return(false);
	}

	/* output comment */
	build_comment(fp, "check library %s", pchk->name);

	/* output pmk command */
	build_cmd_begin(fp, "CHECK_LIB", cname);

	/* output type name */
	build_boolean(fp, "REQUIRED", false);

	/* output header name */
	build_quoted(fp, "NAME", pchk->name);

	/* output language */
	if (set_lang(fp, pchk->ftype) == false) {
		return(false);
	}

	/* output list (already handle empty list) */
	build_list(fp, "FUNCTION", pchk->procs);

	/* output end of command body */
	build_cmd_end(fp);

	fprintf(fp, "\n");

	return(true);
}


/*****************
 * output_type() *
 ***********************************************************************
 DESCR
	XXX

 IN
 	checks :	checks hash table
	type :		type to process
	psd :		scandata structure
	pn: 		node structure

 OUT
	boolean

 TODO
	handle member ?
 ***********************************************************************/


bool output_type(htable *checks, char *cname, FILE *fp) {
	check_t	*pchk;

	pchk = hash_get(checks, cname);
	if (pchk == NULL) {
		return(false);
	}

	/* output comment */
	build_comment(fp, "check type %s", pchk->name);

	/* output pmk command */
	build_cmd_begin(fp, "CHECK_TYPE", cname);

	/* output type name */
	build_boolean(fp, "REQUIRED", false);

	/* output type name */
	build_quoted(fp, "NAME", pchk->name);

	/* output language */
	if (set_lang(fp, pchk->ftype) == false) {
		return(false);
	}

	if (pchk->header != NULL) {
		/* output header name */
		build_quoted(fp, "HEADER", pchk->header);
	}

	/* output end of command body */
	build_cmd_end(fp);

	fprintf(fp, "\n");

	return(true);
}


/********************
 * scan_build_pmk() *
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

bool scan_build_pmk(scn_zone_t *psz) {
	FILE			*fp;
	char			 buf[MKF_OUTPUT_WIDTH * 2];
	hkeys			*phk;
	time_t			 now;
	unsigned int	 i;

	/* open output file */
	fp = fopen(psz->pmk_name, "w");
	if (fp == NULL) {
		errorf("cannot open '%s' : %s.", psz->pmk_name, strerror(errno));
		return(false);
	}

	/* header comment ******************/

	/* generating date */
	now = time(NULL);
	strftime(buf, sizeof(buf), STR_TIME_GEN, localtime(&now));

	/* output formatted string */
	build_comment(fp, PMKF_HDR_GEN, buf);

	/* settings command ****************/

	/* output command */
	build_cmd_begin(fp, "SETTINGS", NULL);

	/* template list comment */
	build_comment(fp, PMKF_TRGT_CMT);

	/* genere template list */
	if (build_list(fp, "TARGET", psz->templates) == false) {
		/* list is empty, produce comment */
		build_comment(fp, "TARGET = ()\t# TO EDIT");
	}

	/* end of command */
	build_cmd_end(fp);

	fprintf(fp, "\n");

	/* define command ******************/

	/* warning comment */
	build_comment(fp, PMKF_DEF_CMT);

	/* output command */
	build_cmd_begin(fp, "DEFINE", NULL);

	/* output default defines */
	fprintf(fp, PMKF_DEF_STD); /* (XXX TO IMPROVE)  */

	/* output needed man directories */
	if (psz->found[FILE_TYPE_MAN] == true) {
		/* man pages directories */
		for (i = 1 ; i < 10 ; i++) {
			/* check if current category is needed */
			if (psz->found[FILE_TYPE_MAN + i] == true) {
				fprintf(fp, PMKF_DEF_MAN, i, i);
			}
		}
	}

	/* extra tags */
	if (psz->exttags != NULL) {
		da_sort(psz->exttags);

		/* output each extra tag */
		for (i = 0 ; i < da_usize(psz->exttags) ; i++) {
			fprintf(fp, PMKF_DEF_TAG, (char *) da_idx(psz->exttags, i));
		}
	}

	/* end of command */
	build_cmd_end(fp);

	fprintf(fp, "\n");

	/* type checks *********************/
	phk = hash_keys_sorted(psz->t_checks);
	if (phk != NULL) {
		/* output every type checks */
		for(i = 0 ; i < phk->nkey ; i++) {
			output_type(psz->t_checks, phk->keys[i], fp); /* XXX check */
		}

		hash_free_hkeys(phk);
	}

	/* header checks *******************/
	phk = hash_keys_sorted(psz->h_checks);
	if (phk != NULL) {
		/* output every header checks */
		for(i = 0 ; i < phk->nkey ; i++) {
			output_header(psz->h_checks, phk->keys[i], fp); /* XXX check */
		}

		hash_free_hkeys(phk);
	}

	/* library checks ******************/
	phk = hash_keys_sorted(psz->l_checks);
	if (phk != NULL) {
		/* output every library checks */
		for(i = 0 ; i < phk->nkey ; i++) {
			output_library(psz->l_checks, phk->keys[i], fp); /* XXX check */
		}

		hash_free_hkeys(phk);
	}

	fclose(fp);
	psc_log("Saved as '%s'\n", NULL, psz->pmk_name);

	return(true);
}


/********************
 * scan_build_cfg() *
 ***********************************************************************
 DESCR
	build config file using gathered data

 IN
	fname :	output file name
	psz :	scanning zone data
	psd :	parsing data structure

 OUT
	boolean
 ***********************************************************************/

bool scan_build_cfg(scn_zone_t *psz) {
	FILE			*fp;
	char			 buf[MKF_OUTPUT_WIDTH * 2],
					*pstr;
	time_t			 now;
	unsigned int	 i;

	fp = fopen(psz->cfg_name, "w");
	if (fp == NULL) {
		errorf("unable to open file '%s' for writing.", psz->cfg_name);
		return(false);
	}

	/* generating date */
	now = time(NULL);
	strftime(buf, sizeof(buf), STR_TIME_GEN, localtime(&now));

	fprintf(fp, CFGF_HDR_GEN, buf);

	if (psz->exttags != NULL) {
		fprintf(fp, "/* extra tags */\n\n");
		for (i = 0 ; i < da_usize(psz->exttags) ; i++) {
			pstr = gen_basic_tag_def(da_idx(psz->exttags, i));
			fprintf(fp, "@%s@\n\n", pstr);
		}
	}

	fprintf(fp, "/* scanned tags */\n\n");
	for (i = 0 ; i < da_usize(psz->tags) ; i++) {
		fprintf(fp, "@%s@\n\n", (char *) da_idx(psz->tags, i));
	}

	fclose(fp);
	psc_log("Saved as '%s'\n", NULL, psz->cfg_name);

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
				psc_log(NULL, "\t\tFound common dependency '%s'\n", str_fc);

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
	char	 buffer[PATH_MAX],
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
	char	 tmp[PATH_MAX],
			 chk[PATH_MAX];

	if (*dir == '\0') {
		/* directory empty, store only file name */
		strlcpy(tmp, file, sizeof(tmp));
	} else {
		/* join directory and file names with the directory separator */
		snprintf(tmp, sizeof(tmp), "%s/%s", dir, file);
	}

	/* check resulting path */
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
	char		 dir[PATH_MAX];
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
#ifdef PMKSCAN_DEBUG
		debugf("recurse_obj_deps() : node '%s' missing.", nodename);
#endif /* PMKSCAN_DEBUG */
		return(true);
	}

	/* get directory */
	extract_dir(nodename, dir, sizeof(dir));

	/* look for all the local dependencies of the current node */
	for (i = 0 ; i < da_usize(pnode->local_inc) ; i++) {
		/* and recurse */
		if (recurse_obj_deps(nodes, deps, (char *)da_idx(pnode->local_inc, i)) == false) {
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
	char			 buf[PATH_MAX], /* XXX filename length */
					*pstr;
	hkeys			*phk;
	scn_node_t		*pnode,
					*pn;
	unsigned int	 i,
					 j;

	phk = hash_keys(psz->nodes);
	if (phk == NULL) {
		/* no objects to process */
		psc_log("No objects to generate.\n", NULL);
		return(true);
	}

	for(i = 0 ; i < phk->nkey ; i++) {
		pnode = hash_get(psz->nodes, phk->keys[i]); /* XXX check needed ??? (think no) */
#ifdef PMKSCAN_DEBUG
		debugf("score of '%s' = %d", phk->keys[i], pnode->score);
#endif /* PMKSCAN_DEBUG */

		/*
			if we got a node with a score of 0 then it should
			be an object
		*/
		if (pnode->score == 0) {
			/* build object name */
			strlcpy(buf, pnode->prefix, sizeof(buf));
			strlcat(buf, OBJ_SUFFIX, sizeof(buf));

			psc_log(NULL, "\tProcessing '%s'\n", buf);

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
#ifdef PMKSCAN_DEBUG
					debugf("gen_objects() : node '%s' missing.", pstr);
#endif /* PMKSCAN_DEBUG */
					continue;
				}

				/* check for common function declarators */
				if (find_deps(pnode->func_decls, pn->func_decls) == true) {
#ifdef PMKSCAN_DEBUG
					debugf("adding object link '%s' dependency to node '%s'", pnode->obj_name, pn->fname);
#endif /* PMKSCAN_DEBUG */

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
#endif /* PMKSCAN_DEBUG */

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
#endif /* PMKSCAN_DEBUG */

	/* for each source dependency */
	for (i = 0 ; i < da_usize(pnode->src_deps) ; i++) {
		/* get the node structure */
		src = da_idx(pnode->src_deps, i);
		pn = hash_get(psz->nodes, src);

		if (pn == NULL) {
#ifdef PMKSCAN_DEBUG
		debugf("recurse_src_deps() : source '%s' missing.", src);
#endif /* PMKSCAN_DEBUG */
			continue;
		}

#ifdef PMKSCAN_DEBUG
		debugf("recurse_src_deps() : node '%s' : src dep '%s' (%d)", pnode->fname, src, da_usize(pn->obj_links));
#endif /* PMKSCAN_DEBUG */

		/* check each object link */
		for (j = 0 ; j < da_usize(pn->obj_links) ; j++) {
			/* get object name */
			odep = da_idx(pn->obj_links, j);
#ifdef PMKSCAN_DEBUG
			debugf("recurse_src_deps() : node '%s' : obj link '%s'", pnode->fname, odep);
#endif /* PMKSCAN_DEBUG */

			/* check if already in the list */
			if (da_find(deps, odep) == false) {
				/* and add the object if not already present */
				if (da_push(deps, strdup(odep)) == false) {
					/* XXX err msg */
					return(false);
				}

#ifdef PMKSCAN_DEBUG
				debugf("recurse_src_deps() : node '%s' : adding '%s' in deps", pnode->fname, odep);
#endif /* PMKSCAN_DEBUG */

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
#endif /* PMKSCAN_DEBUG */

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
	char			 buf[PATH_MAX], /* XXX filename length */
					*nodename;
	hkeys			*phk;
	scn_node_t		*pnode;
	unsigned int	 i;

	phk = hash_keys(psz->objects);
	if (phk == NULL) {
		/* no objects, skip */
		psc_log("No targets to generate.\n", NULL);
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
				errorf("failed to init object dependencies dynary.");
				return(false);
			}

			/* build and store object name */
			strlcpy(buf, pnode->prefix, sizeof(buf));
			strlcat(buf, OBJ_SUFFIX, sizeof(buf));
			da_push(pnode->obj_deps, strdup(buf)); /* XXX check ? */

#ifdef PMKSCAN_DEBUG
			debugf("START recurse_src_deps() for node '%s'", pnode->fname);
#endif /* PMKSCAN_DEBUG */
			/* recurse source deps to find object deps */
			if (recurse_src_deps(psz, pnode->obj_deps, pnode->fname) == false) {
				/* failed */
				return(false);
			}
#ifdef PMKSCAN_DEBUG
			debugf("END recurse_src_deps() for node '%s'\n", pnode->fname);
#endif /* PMKSCAN_DEBUG */
		}
	}

	return(true);
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
	char			 buf[MKF_OUTPUT_WIDTH * 2],
					*pstr;
	time_t			 now;
	unsigned int	 i;

	/* generating date */
	now = time(NULL);
	strftime(buf, sizeof(buf), STR_TIME_GEN, localtime(&now));

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

	/* package data */
	fprintf(fp, MKF_HEADER_DATA);

	/* extra tags */
	if (psz->exttags != NULL) {
		fprintf(fp, "# extra tags\n");

		da_sort(psz->exttags);

		/* output each extra tag */
		for (i = 0 ; i < da_usize(psz->exttags) ; i++) {
			pstr = (char *) da_idx(psz->exttags, i);
			fprintf(fp, MKF_VARIABLE, pstr, pstr);
		}

		fprintf(fp, "\n");
	}

	/* directories */
	fprintf(fp, "# specific directories\n");
	fprintf(fp, MKF_HEADER_DIR);
	if (psz->found[FILE_TYPE_MAN] == true) {
		/* main man pages directory */
		fprintf(fp, MKF_MAN_DIR);

		/* man pages directories */
		for (i = 1 ; i < 10 ; i++) {
			/* check if current category is needed */
			if (psz->found[FILE_TYPE_MAN + i] == true) {
				fprintf(fp, MKF_MANX_DIR, i, i);
			}
		}
	}

	/* system configuration directory */
	fprintf(fp, MKF_SYSCONF_DIR);

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

	fprintf(fp, MKF_LINE_JUMP);
}


/***********************
 * mkf_output_recurs() *
 ***********************************************************************
 DESCR
	XXX

 IN
	fp :	file pointer
	psz :	scanning zone data

 OUT
	NONE
 ***********************************************************************/

void mkf_output_recurs(FILE *fp, scn_zone_t *psz) {
	char			*pstr;
	size_t			 ofst,
					 lm,
					 i,
					 s;

	/*
		generate the list of scanned directories
	*/
	s = da_usize(psz->dirlist);
	if (s > 0) {
		/* get directory list */
		fprintf(fp, "#\n# directory list\n#\n");
		fprintf(fp, MKF_SDIR_LIST);

		da_sort(psz->dirlist);

		lm = strlen(MKF_SDIR_LIST);
		ofst = lm;
		for (i = 0 ; i < s ; i++) {
			/* get directory */
			pstr = da_idx(psz->dirlist, i);

			/* add to the list */
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, pstr);
		}

		fprintf(fp, MKF_TWICE_JUMP);
	}

	/*
		generate the list of template files
	*/
	s = da_usize(psz->templates);
	if (s > 0) {
		/* get list of template files */
		fprintf(fp, "#\n# list of generated files\n#\n");
		fprintf(fp, MKF_TEMPLATES);

		da_sort(psz->templates);

		lm = strlen(MKF_TEMPLATES);
		ofst = lm;
		for (i = 0 ; i < s ; i++) {
			/* get template file name */
			pstr = da_idx(psz->templates, i);

			/* add to the list */
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, pstr);
		}

		fprintf(fp, MKF_TWICE_JUMP);
	}

	/*
		generate the list of template generated files
	*/
	s = da_usize(psz->templates);
	if (s > 0) {
		/* get list of generated files */
		fprintf(fp, "#\n# list of generated files\n#\n");
		fprintf(fp, MKF_GEN_FILES);

		da_sort(psz->templates);

		lm = strlen(MKF_GEN_FILES);
		ofst = lm;
		for (i = 0 ; i < s ; i++) {
			/* generate file name from template */
			pstr = gen_from_tmpl(da_idx(psz->templates, i));

			/* add result to the list */
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, pstr);
		}

		fprintf(fp, MKF_TWICE_JUMP);
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
	char			 buf[MKF_OUTPUT_WIDTH * 2],
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

	fprintf(fp, "#\n# object dependency lists\n#\n");
	for (i = 0 ; i < phk->nkey ; i++) {
		pstr = hash_get(psz->objects, phk->keys[i]);
		pn = hash_get(psz->nodes, pstr);

		/* object label */
		snprintf(buf, sizeof(buf), MKF_OBJECT_SRCS, pn->prefix);
		str_to_upper(buf, sizeof(buf), buf);
		fprintf(fp, buf);

		lm = strlen(buf);
		ofst = lm;

		da_sort(pn->src_deps);

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
	char			 buf[MKF_OUTPUT_WIDTH * 2],
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
		fprintf(fp, "#\n# target dependency lists\n#\n");
		for (i = 0 ; i < phk->nkey ; i++) {
			pstr = hash_get(psz->targets, phk->keys[i]);
			pn = hash_get(psz->nodes, pstr);

			/* target label */
			snprintf(buf, sizeof(buf), MKF_TARGET_OBJS, pn->prefix);
			str_to_upper(buf, sizeof(buf), buf);
			fprintf(fp, buf);

			lm = strlen(buf);
			ofst = lm;

			da_sort(pn->obj_deps);

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
	char			 buf[MKF_OUTPUT_WIDTH * 2];
	hkeys			*phk;
	size_t			 ofst,
					 lm,
					 i;

	fprintf(fp, "#\n# target lists\n#\n");

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
	char		 buf[MKF_OUTPUT_WIDTH * 2],
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
		/* if category has at least one file */
		if (psz->found[FILE_TYPE_MAN + i] == true) {
			/* output category list macro */
			snprintf(buf, sizeof(buf), MKF_FILE_MAN_VAR, (int) i);
			fprintf(fp, buf);

			lm = strlen(buf);
			ofst = lm;

			da_sort(psz->manpgs);

			/* for each man page */
			for (j = 0 ; j < da_usize(psz->manpgs) ; j++) {
				/* get the last character */
				pstr = da_idx(psz->manpgs, j);
				k = strlen(pstr) - 1;

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
	}
}


/**************************
 * mkf_output_data_trgs() *
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
	char		 buf[MKF_OUTPUT_WIDTH * 2],
				*pstr;
	size_t		 ofst,
				 lm,
				 i;

	if (psz->found[FILE_TYPE_DATA] == false) {
		/* no data files */
		return;
	}

	/* data files */
	fprintf(fp, MKF_FILE_DATA_VAR);

	lm = strlen(buf);
	ofst = lm;

	da_sort(psz->datafiles);

	/* for each data file */
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
	char			 buf[MKF_OUTPUT_WIDTH * 2],
					*pstr;
	hkeys			*phk;
	scn_node_t		*pn;
	size_t			 i;

	phk = hash_keys_sorted(psz->objects);
	if (phk == NULL) {
		/* nothing to do */
		return;
	}

	fprintf(fp, "#\n# object rules\n#\n");
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
	char			 buf[MKF_OUTPUT_WIDTH * 2],
					*pstr,
					*pname;
	hkeys			*phk;
	scn_node_t		*pn;
	size_t			 i;

	phk = hash_keys_sorted(psz->targets);
	if (phk == NULL) {
		/* nothing to do */
		return;
	}

	if (phk != NULL) {
		/* generate targets */
		fprintf(fp, "\n#\n# target rules\n#\n");
		for (i = 0 ; i < phk->nkey ; i++) {
			pstr = phk->keys[i];

			/* uppercase string */
			str_to_upper(buf, sizeof(buf), pstr);

			/* build target */
			fprintf(fp, MKF_TARGET_LABL, pstr, buf);
			pname = hash_get(psz->targets, phk->keys[i]);
			pn = hash_get(psz->nodes, pname);
			if (pn != NULL) {
				/* XXX */
				switch (pn->type) {
					/*case FILE_TYPE_ASM : XXX */

					case FILE_TYPE_C :
						fprintf(fp, MKF_TARGET_C, buf);
						break;

					case FILE_TYPE_CXX :
						fprintf(fp, MKF_TARGET_CXX, buf);
						break;

					default :
						fprintf(fp, MKF_TARGET_DEF, buf);
				}
			}

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
	for (i = 1 ; i < 10 ; i++) {
		/* if category has at least one file */
		if (psz->found[FILE_TYPE_MAN + i] == true) {
			/* gather list of man categories */
			fprintf(fp, MKF_INST_MAN_B, i, i, i, i, i);
		}
	}
	fprintf(fp, MKF_TWICE_JUMP);

	fprintf(fp, MKF_DEINST_MAN_H);
	/* manual pages to install/deinstall */
	for (i = 1 ; i < 10 ; i++) {
		/* if category has at least one file */
		if (psz->found[FILE_TYPE_MAN + i] == true) {
			/* gather list of man categories */
			fprintf(fp, MKF_DEINST_MAN_B, i, i, i, i);
		}
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

bool scan_build_mkf(scn_zone_t *psz) {
	FILE	*fp,
			*fp_ext;
	bool	 ferr = false;
	char	 buf[512];
	size_t	 len;

	fp = fopen(psz->mkf_name, "w");
	if (fp == NULL) {
		errorf("unable to open file '%s' for writing.", psz->mkf_name);
		return(false);
	}

	/* generate header and definitions */
	mkf_output_header(fp, psz);

	/* generate recursive data */
	mkf_output_recurs(fp, psz);

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

	fprintf(fp, "\n#\n# generic targets\n#\n");
	fprintf(fp, MKF_TARGET_ALL);
/*	fprintf(fp, MKF_TARGET_CFG); XXX TO ENABLE: auto config update target */
	fprintf(fp, MKF_TARGET_INST);
	fprintf(fp, MKF_INST_BIN);
	fprintf(fp, MKF_DEINST_BIN);

	mkf_output_man_inst(fp, psz);

	if (psz->found[FILE_TYPE_DATA] == true) {
		fprintf(fp, MKF_INST_DATA);
		fprintf(fp, MKF_DEINST_DATA);
	}

	fprintf(fp, MKF_DIST_CLEAN);

	/* recursive targets */
	fprintf(fp, MKF_RECURS_TRGT);

	if (psz->unique == true) {
		fprintf(fp, MKF_LINE_JUMP);
	} else {
		/* recursive targets wrapper */
		fprintf(fp, MKF_RECURS_PROC);
	}

	/* generate objects */
	mkf_output_obj_rules(fp, psz);

	/* generate targets */
	mkf_output_trg_rules(fp, psz);

	/* extra to append */
	if (psz->ext_mkf != NULL) {
		fp_ext = fopen(psz->ext_mkf, "r");
		if (fp_ext == NULL) {
			errorf("unable to open file '%s' for reading.", psz->ext_mkf);
			return(false);
		}

		/* append extra content to the template */
		while ((feof(fp_ext) == 0) && (ferr == false)) {
			len = fread(buf, sizeof(char), sizeof(buf), fp_ext);
			if (ferror(fp_ext) != 0) {
				ferr = true;
			} else {
				fwrite(buf, sizeof(char), len, fp);
				if (ferror(fp) != 0) {
					ferr = true;
				}
			}
		}

		fclose(fp_ext);
	}

	fclose(fp);

	/* append failed */
	if (ferr == true) {
		errorf("unable to append '%s' content into template.", psz->ext_mkf);
		return(false);
	}

	psc_log("Saved as '%s'\n", NULL, psz->mkf_name);

	return(true);
}


/********************
 * common functions *
 ***********************************************************************/

/**************
 * psc_log () *
 ***********************************************************************
 DESCR
	log on standard output and log file if enabled

 IN
	fmt :	standard output format string
	fmtl :	log file format string
	... :	common parameters

 OUT
	NONE
 ***********************************************************************/

void psc_log(char *fmt, char *fmtl, ...) {
	va_list	 vlst,
			 vlstl;

	va_start(vlst, fmtl);
	va_copy(vlstl, vlst);

	if (fmt != NULL) {
		vprintf(fmt, vlst);
	}

	/* if log is enabled */
	if (fp_log != NULL) {

		/* if a log format is not provided take the first */
		if (fmtl == NULL) {
			fmtl = fmt;
		}

		if (fmtl != NULL) {
			vfprintf(fp_log, fmtl, vlstl);
		}
	}

	va_end(vlst);
}


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
		if ((*str == '.') || (*str == '/')) {
			/*
				replaces dot and slashes by underscores
				ex: if we got "../" then replace by "___"
			*/
			*buf = '_';
		} else {
			/* else copy character in uppercase */
			*buf = toupper(*str);
		}
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
#endif /* PMKSCAN_DEBUG */
		/* check files that match known extension */
		if (fnmatch(file_ext[i].ext, fname, 0) != FNM_NOMATCH) {
			/* exit the loop */
			return(file_ext[i].type);
		}
	}

	/* unknown type */
	return(FILE_TYPE_UNKNOWN);
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
	char		 iname[PATH_MAX],
				 buf[PATH_MAX],
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
				debugf("process_ppro() : found local include '%s'", iname);
#endif /* PMKSCAN_DEBUG */

				/* build relative path of the include */
				build_path(pnode->dname, iname, buf, sizeof(buf));

#ifdef PMKSCAN_DEBUG
				debugf("process_ppro() : adding include path '%s'", buf);
#endif /* PMKSCAN_DEBUG */

				/* add include in depedencies */
				if (da_push(pnode->local_inc, strdup(buf)) == false) {
					errorf("unable to add '%s' in local deps", buf);
					return(false);
				}

				break;

			case '<' :
#ifdef PMKSCAN_DEBUG
				debugf("process_ppro() : found system include '%s'", iname);
#endif /* PMKSCAN_DEBUG */
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
	debugf("process_proc_call() : found procedure call of '%s'", pstr);
#endif /* PMKSCAN_DEBUG */

	/* add function in list */
	if (da_push(pnode->func_calls, strdup(pstr)) == false) {
		errorf("unable to add '%s' in function call list", pstr);
		return(false);
	}

	psc_log(NULL, "\t\tFound procedure call '%s'.\n", pstr);

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
	debugf("process_proc_decl() : found procedure declaration of '%s'", pstr);
#endif /* PMKSCAN_DEBUG */

	/* add function in list */
	if (da_push(pnode->func_decls, strdup(pstr)) == false) {
		errorf("unable to add '%s' in function declaration list", pstr);
		return(false);
	}

	/* check for main procedure */
	if (strncmp(pstr, PSC_MAIN_C, strlen(pstr)) == 0) {
#ifdef PMKSCAN_DEBUG
		debugf("process_proc_decl() : found main procedure '%s' in '%s'", pstr, pnode->fname);
#endif /* PMKSCAN_DEBUG */
		pnode->mainproc = true;
	}

	psc_log(NULL, "\t\tFound procedure declaration '%s'.\n", pstr);

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

	psc_log(NULL, "\t\tFound type '%s'.\n", pstr);

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
	char			*ptr,
					 dir[PATH_MAX],
					 idir[PATH_MAX];
	scn_node_t		*pnode;
	scn_zone_t		*psz;
	unsigned int	 i;

	/* get misc data */
	psz = (scn_zone_t *) pcmn->data;

	/* check if this node is already existing */
	pnode = hash_get(psz->nodes, fname);
	if (pnode == NULL) {
#ifdef PMKSCAN_DEBUG
		debugf("parse_file() : adding node for '%s'", fname); /* XXX */
#endif /* PMKSCAN_DEBUG */

		/* open file */
		fp = fopen(fname, "r");
		if (fp == NULL) {
			errorf("Warning : cannot open '%s' : %s.\n", fname, strerror(errno));
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

		/* get directory name */
		extract_dir(fname, dir, sizeof(dir));
		/* and store it for further use */
		pnode->dname = strdup(dir);

		pnode->type = ft;

		switch (ft) {
			case FILE_TYPE_ASM :
				psz->found[FILE_TYPE_ASM] = true;
				psz->found_src = true;
				psc_log(NULL, "\tStart parsing of assembly file '%s'\n", fname);
				if (prs_asm_file(pcmn, fp) == false) {
					fclose(fp);
					return(false);
				}

				/* display parsed assembly file */
				psc_log("a", "\tParsed assembly file '%s'\n", fname);
				break;

			case FILE_TYPE_C :
			case FILE_TYPE_CXX :
				psz->found_src = true;
				if (ft == FILE_TYPE_C) {
					psz->found[FILE_TYPE_C] = true;
					psc_log(NULL, "\tStart parsing of C file '%s'\n", fname);
				} else {
					psz->found[FILE_TYPE_CXX] = true;
					psc_log(NULL, "\tStart parsing of C++ file '%s'\n", fname);
				}

				if (prs_c_file(pcmn, fp) == false) {
					fclose(fp);
					return(false);
				}

				if (ft == FILE_TYPE_C) {
					/* display parsed c file */
					psc_log("c", "\tParsed C file '%s'\n", fname);
				} else {
					/* display parsed c++ file */
					psc_log("C", "\tParsed C++ file '%s'\n", fname);
				}
				break;
		}

		/* close file */
		fclose(fp);
	}

	/* update dependency state */
	pnode->isdep = isdep;

	if (isdep == true) {
		/* update dependency score */
		pnode->score++;
#ifdef PMKSCAN_DEBUG
		debugf("parse_file() : score of '%s' = %d", pnode->fname, pnode->score);
#endif /* PMKSCAN_DEBUG */
	}

	/* add the node in the table of nodes */
	if (hash_add(psz->nodes, fname, pnode) == HASH_ADD_FAIL) {
		errorf("failed to add node '%s' in the hash table.", pnode->fname);
		scan_node_destroy(pnode);
		return(false);
	}

	for (i = 0 ; i < da_usize(pnode->local_inc) ; i++) {
		ptr = (char *) da_idx(pnode->local_inc, i);
#ifdef PMKSCAN_DEBUG
		debugf("parse_file() : dir = '%s', inc = '%s'", dir, ptr);
#endif /* PMKSCAN_DEBUG */

		/* scan local include */
		if (scan_node_file(pcmn, ptr, true) == false) {
			errorf("failed to scan file '%s'.", ptr);
			return(false);
		}

		/* add include directory in list */
		extract_dir(ptr, idir, sizeof(idir));
#ifdef PMKSCAN_DEBUG
		debugf("parse_file() : include extracted dir = '%s'", idir);
#endif /* PMKSCAN_DEBUG */

		/* if the header directory is not yet in the scan list */
		if ((*idir != '\0') && (da_find(psz->dirlist, idir) == false)) {
#ifdef PMKSCAN_DEBUG
			debugf("parse_file() : adding '%s' in directory list to scan", idir);
#endif /* PMKSCAN_DEBUG */
			/* add in zone directory list for further process */
			da_push(psz->dirlist, strdup(idir));
			/* and in the list of directorie to be scanned */
			da_push(psz->dirscan, strdup(idir));
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
	debugf("scan_node_file() : fname = '%s'", fname);
#endif /* PMKSCAN_DEBUG */

	psz = (scn_zone_t *) pcmn->data;

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
			psc_log(".", "\tFound Lex file '%s' (unsupported yet)\n", fname);
			break;

		case FILE_TYPE_YACC :
			/* XXX TODO */
			psc_log(".", "\tFound Yacc file '%s' (unsupported yet)\n", fname);
			break;

		case FILE_TYPE_MAN1 :
		case FILE_TYPE_MAN2 :
		case FILE_TYPE_MAN3 :
		case FILE_TYPE_MAN4 :
		case FILE_TYPE_MAN5 :
		case FILE_TYPE_MAN6 :
		case FILE_TYPE_MAN7 :
		case FILE_TYPE_MAN8 :
		case FILE_TYPE_MAN9 :
			/* man pages will be processed later */
			psz->found[FILE_TYPE_MAN] = true;
			psz->found[ft] = true;

			/* add man page in the list */
			da_push(psz->manpgs, strdup(fname)); /* XXX check ? */
#ifdef PMKSCAN_DEBUG
			debugf("scan_node_file() : added '%s' in psz->manpgs.", fname);
#endif /* PMKSCAN_DEBUG */

			/* display man page file as recorded */
			psc_log("m", "\tRecorded manual page '%s'\n", fname);
			break;

		case FILE_TYPE_DATA :
			/* data files will be processed later */
			psz->found[FILE_TYPE_DATA] = true;

			/* add data file in the list */
			da_push(psz->datafiles, strdup(fname)); /* XXX check ? */
#ifdef PMKSCAN_DEBUG
			debugf("scan_node_file() : added '%s' in psz->datafiles.", fname);
#endif /* PMKSCAN_DEBUG */

			/* display data file as recorded */
			psc_log("d", "\tRecorded data file '%s'\n", fname);
			break;

		case FILE_TYPE_TEMPL :
			/* data files will be processed later */
			psz->found[FILE_TYPE_TEMPL] = true;

			/* add template file in the list */
			da_push(psz->templates, strdup(fname)); /* XXX check ? */

			/* display data file as recorded */
			psc_log("t", "\tRecorded template file '%s'\n", fname);
			break;

		default :
			/* skip unsupported file extension */

			/* display a file with unknown type */
			psc_log(".", "\tFound file '%s' with unknown type\n", fname);
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
	char			 buf[PATH_MAX],
					*fname;
	dynary			*hdr_dir,
					*to_scan = NULL;
	scn_zone_t		*psz;
	size_t			 i;

	psz = pcmn->data;

	if ((psz->discard != NULL) && (da_find(psz->discard, dir) == true)) {
		/* discard directory */
		psc_log("Discarding directory '%s'\n", NULL, dir);
		return(true);
	}

	pd = opendir(dir);
	if (pd == NULL) {
		/* this is not a directory */
		return(false);
	}

	hdr_dir = da_init();
	if (hdr_dir == NULL) {
		/* XXX msg ? */
		closedir(pd);
		return(false);
	}

	if (recursive == true) {
		to_scan = da_init();
		if (to_scan == NULL) {
			/* XXX msg ? */
			da_destroy(hdr_dir);
			closedir(pd);
			return(false);
		}
	}

	/* set pointer to dynary of directories to scan */
	psz->dirscan = hdr_dir;

	psc_log("Scanning directory '%s'\n", NULL, dir);
	psc_log("[", "[\n");

	/* check each directory's entries */
	while ((pde = readdir(pd)) && (pde != NULL)) {
		fname = pde->d_name;

#ifdef PMKSCAN_DEBUG
		debugf("scan_dir() : checking entry '%s'", fname);
#endif /* PMKSCAN_DEBUG */

		if (*fname == '.') {
			/* skip every entries that starts with a dot */
#ifdef PMKSCAN_DEBUG
			debugf("scan_dir() : skipping '%s'", fname);
#endif /* PMKSCAN_DEBUG */
			continue;
		}

		/* build full path */
		build_path(dir, fname, buf, sizeof(buf));

#ifdef PMKSCAN_DEBUG
		debugf("scan_dir() : built path = '%s'", buf);
#endif /* PMKSCAN_DEBUG */

		if (stat(buf, &tstat) == -1) {
			continue;
		}

		/* if the entry is a directory */
		if ((tstat.st_mode & S_IFDIR) != 0) {
			/* and if recursivity is enabled */
			if (recursive == true) {
				/* then process the directory */
				if (da_find(psz->dirlist, buf) == false) {
					da_push(psz->dirlist, strdup(buf));
					da_push(to_scan, strdup(buf));
				}
			}

			/* go to next entry */
			continue;
		}

		if (scan_node_file(pcmn, buf, false) == false) {
			/* display scan failure for the file */
			psc_log("!", "Failed to scan '%s'", fname);
		}
	}

	closedir(pd);
	psc_log("]\n", "]\n\n");

	/* scan directories of relative headers (no recurse) */
	for (i = 0 ; i < da_usize(hdr_dir) ; i++) {
		scan_dir(pcmn, da_idx(hdr_dir, i), false);
	}

	da_destroy(hdr_dir);

	if (recursive == true) {
		/* recurse sub directories and directory found in headers */
		for (i = 0 ; i < da_usize(to_scan) ; i++) {
			scan_dir(pcmn, da_idx(to_scan, i), true);
		}

		da_destroy(to_scan);
	}

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
	bool		 frslt,
				 rslt;
	char		 sdir[PATH_MAX];
	scn_zone_t	*psz;

	psz = pcmn->data;

	/* save current working directory */
	if (getcwd(sdir, sizeof(sdir)) == NULL) {
		errorf("cannot get current working directory.");
		return(false);
	}

	/* change to zone directory */
	if (chdir(psz->directory) != 0) {
		errorf("cannot set working directory to '%s'.", psz->directory);
		return(false);
	}

	/* scanning directory */
	psc_log("Starting file parsing in '%s':\n", NULL, psz->directory);
	da_push(psz->dirlist, strdup("."));
	frslt = scan_dir(pcmn, ".", psz->recursive);
	if (frslt == false) {
		psc_log("Parsing failed.\n\n", NULL);
	} else {
		psc_log("Parsing finished.\n\n", NULL);

		/* pmkfile stuff */
		if (psz->gen_pmk == true) {
			/* compare with known functions in pmkscan db */
			psc_log("Processing nodes for check generation ...\n", NULL);
			rslt = gen_checks(psz, psd);
			if (rslt == false) {
				psc_log("Failed\n\n", NULL);
			} else {
				psc_log("Ok\n\n", NULL);

				/* generate pmkfile */
				psc_log("Generating %s ...\n", NULL, psz->cfg_name);
				rslt = scan_build_pmk(psz);
				if (rslt == true) {
					psc_log("Ok\n\n", NULL);
				} else {
					psc_log("Failed\n\n", NULL);
				}

				/* generate config file */
				psc_log("Generating %s ...\n", NULL, psz->cfg_name);
				if (scan_build_cfg(psz) == true ) {
					psc_log("Ok\n\n", NULL);
				} else {
					psc_log("Failed\n\n", NULL);
				}
			}

			if (rslt == false) {
				frslt = false;
			}
		}

		/* makefile stuff */
		if (psz->gen_mkf == true) {
			/* scanning resulting nodes */
			psc_log("Processing nodes for object generation ...\n", NULL);
			rslt = gen_objects(psz);
			if (rslt == false) {
				psc_log("Failed\n\n", NULL);
			} else {
				psc_log("Ok\n\n", NULL);

				/* scanning generated objects */
				psc_log("Processing objects for target generation ...\n", NULL);
				rslt = gen_targets(psz);
				if (rslt == false) {
					psc_log("Failed\n\n", NULL);
				} else {
					psc_log("Ok\n\n", NULL);

					/* generate makefile */
					psc_log("Generating %s ...\n", NULL, psz->mkf_name);
					if (scan_build_mkf(psz) == true ) {
						psc_log("Ok\n\n", NULL);
					} else {
						psc_log("Failed\n\n", NULL);
					}
				}
			}

			if (rslt == false) {
				frslt = false;
			}
		}
	}

	/* come back to previously saved directory */
	if (chdir(sdir) != 0) {
		errorf("cannot set working directory back to '%s'.", sdir);
		return(false);
	}

	return(frslt);
}


/***********************
 * parse_zone_opts() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool parse_zone_opts(htable *pht, scn_zone_t *psz) {
	char		*pstr;
	pmkobj		*ppo;

	/* get pmkfile switch */
	ppo = hash_get(pht, KW_OPT_PMK);
	if (ppo != NULL) {
		psz->gen_pmk = po_get_bool(ppo);

		if (psz->gen_pmk == true) {
			/* alternate name for config file template */
			ppo = hash_get(pht, KW_OPT_CFGALT);
			if (ppo != NULL) {
				pstr = po_get_str(ppo);
				if (pstr != NULL) {
					/* set config file name name */
					psz->cfg_name = pstr;
				} else {
					return(false);
				}
			}

			/* alternate name for pmkfile template */
			ppo = hash_get(pht, KW_OPT_PMKALT);
			if (ppo != NULL) {
				pstr = po_get_str(ppo);
				if (pstr != NULL) {
					/* set config file name name */
					psz->pmk_name = pstr;
				} else {
					return(false);
				}
			}
		}
	}

	/* get makefile switch */
	ppo = hash_get(pht, KW_OPT_MKF);
	if (ppo != NULL) {
		psz->gen_mkf = po_get_bool(ppo);

		if (psz->gen_mkf == true) {
			/* alternate name for makefile template */
			ppo = hash_get(pht, KW_OPT_NAM); /* check old option */
			if (ppo != NULL) {
				/* obsolete option message */
				fprintf(stderr, "Warning: %s is obsolete, use %s instead.\n", KW_OPT_NAM, KW_OPT_MKFALT);
			} else {
				ppo = hash_get(pht, KW_OPT_MKFALT); /* check new option */
			}
			if (ppo != NULL) {
				pstr = po_get_str(ppo);
				if (pstr != NULL) {
					/* set makefile name */
					psz->mkf_name = po_get_str(ppo);
				} else {
					return(false);
				}
			}

			/* extra to append to makefile template */
			ppo = hash_get(pht, KW_OPT_EXTMKF);
			if (ppo != NULL) {
				pstr = po_get_str(ppo);
				if (pstr != NULL) {
					/* set config file name name */
					psz->ext_mkf = pstr;
				} else {
					return(false);
				}
			}
		}
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
	bool		 frslt = true;
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
			case PSC_TOK_MAKF :
			case PSC_TOK_ZONE :
				if (parse_zone_opts(pcell->data, psz) == false) {
					errorf("parsing of script file failed."); /* XXX better error message ? */
					return(false);
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

		/* get extra tags list */
		ppo = hash_get(pcell->data, KW_OPT_EXTTAG);
		if (ppo != NULL) {
			psz->exttags = po_get_list(ppo);
		}

		/* process current zone */
		if (process_zone(pcmn, psd) == false) {
			frslt = false;
		}

		scan_zone_destroy(psz);
		hash_destroy(tnodes);

		pcell = pcell->next;
	}

	prsdata_destroy(pdata);

	return(frslt);
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
				 /*unique_file = false,*/
				 recursive = false,
				 gen_mkf = false,
				 gen_pmk = false;
	char		 buf[PATH_MAX],
				*scfile = NULL;
	htable		*tnodes;
	int			 chr;
	prs_cmn_t	 pcmn;
	prsdata		*pdata = NULL; /* XXX into global struct ? */
	scandata	 sd;
	scn_zone_t	*psz;

	while (go_exit == false) {
		chr = getopt(argc, argv, "f:hlmpruv");
		switch (chr) {
			case -1 :
				go_exit = true;
				break;

			case 'f' :
				/* use script file */
				use_script = true;
				scfile = optarg;
				break;

			case 'l' :
				fp_log = fopen("pmkscan.log", "w");
				if (fp_log == NULL) {
					/* XXX err msg */
					exit(EXIT_FAILURE);
				}
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
				/* enable recursive scanning */
				recursive = true;
				break;

			case 'u' :
				/* use unique file (with recursive mode) */
				/* unique_file = true; *//* XXX TODO */
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

	psc_log("PMKSCAN version %s\n\n", NULL, PREMAKE_VERSION);


	psc_log("Initializing data ... \n", NULL);

	/* init common parser structure */
	pcmn.func_ppro = &process_ppro;
	pcmn.func_proc = &process_proc_call;
	pcmn.func_decl = &process_proc_decl;
	pcmn.func_type = &process_type;
	pcmn.data = NULL; /* will be updated later */

	/* if generation options and scanfile are not set */
	if ((gen_pmk == false) && (gen_mkf == false) && (use_script == false)) {
		/* look for default config file */
		use_script = true;
		scfile = PMKSCAN_CONFIG;
	}

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
		psc_log("Using scanning script '%s'.\n", NULL, scfile);

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
	psc_log("Ok\n\n", NULL);

	/* scanning directory */
	psc_log("Starting file parsing :\n", NULL);
	if (scan_dir(&pcmn, buf, recursive) == false) {
		exit(EXIT_FAILURE);
	}
	psc_log("Parsing finished.\n\n", NULL);

	/* pmkfile stuff */
	if (gen_pmk == true) {
		psc_log("Processing nodes for check generation ...\n", NULL);
		gen_checks(psz, &sd); /* XXX check */

		psc_log("Generating %s ...\n", NULL, PMKSCAN_PMKFILE);
		scan_build_pmk(psz); /* XXX check */
		psc_log("Ok\n\n", NULL);

		psc_log("Generating %s ...\n", NULL, PMKSCAN_CFGFILE);
		scan_build_cfg(psz); /* XXX check */
		psc_log("Ok\n\n", NULL);
	}

	/* makefile stuff */
	if (gen_mkf == true) {
		/* scanning resulting nodes */
		psc_log("Processing nodes for object generation ...\n", NULL);
		gen_objects(psz); /* XXX check */
		psc_log("Ok\n\n", NULL);

		/* scanning generated objects */
		psc_log("Processing objects for target generation ...\n", NULL);
		gen_targets(psz); /* XXX check */
		psc_log("Ok\n\n", NULL);

		psc_log("Generating %s ...\n", NULL, PMKSCAN_MKFILE);
		scan_build_mkf(psz); /* XXX check */
		psc_log("Ok\n\n", NULL);
	}

	/* cleaning */
	scan_zone_destroy(psz);
	hash_destroy(tnodes);
	if (gen_pmk == true) {
		prsdata_destroy(pdata);
	}

	psc_log("\nWork complete.\n\n", NULL);

	if (fp_log != NULL) {
		fclose(fp_log);
	}

	return(0);
}

