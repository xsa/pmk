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
#include "pmkscan.h"
#include "premake.h"


/*#define PMKSCAN_DEBUG	1*/


/*****************
 global variables
************************************************************************/

char	*c_file_ext[] = {
		"c",
		"h"
};
size_t	 nb_c_file_ext = sizeof(c_file_ext) / sizeof(char *);

char	*cxx_file_ext[] = {
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
size_t	 nb_cxx_file_ext = sizeof(cxx_file_ext) / sizeof(char *);

scn_ext_t	 file_ext[] = {
		{"*.asm",	SRC_TYPE_ASM},
		{"*.C",		SRC_TYPE_C},
		{"*.c",		SRC_TYPE_C},
		{"*.cc",	SRC_TYPE_C},
		{"*.cpp",	SRC_TYPE_CXX},
		{"*.cxx",	SRC_TYPE_CXX},
		{"*.c++",	SRC_TYPE_CXX},
		{"*.H",		SRC_TYPE_C},
		{"*.h",		SRC_TYPE_C},
		{"*.hh",	SRC_TYPE_C},
		{"*.hpp",	SRC_TYPE_CXX},
		{"*.hxx",	SRC_TYPE_CXX},
		{"*.h++",	SRC_TYPE_CXX},
		{"*.S",		SRC_TYPE_ASM},
		{"*.s",		SRC_TYPE_ASM},
		{"*.l", 	SRC_TYPE_LEX},
		{"*.y", 	SRC_TYPE_YACC}
};
size_t	 nb_file_ext = sizeof(file_ext) / sizeof(scn_ext_t);

prskw	kw_pmkscan[] = {
		{"INCLUDES",	PSC_TOK_INCL, PRS_KW_CELL,	NULL},
		{"FUNCTIONS",	PSC_TOK_FUNC, PRS_KW_CELL,	NULL}
};

size_t	nbkwps = sizeof(kw_pmkscan) / sizeof(prskw);

extern char	*optarg;
extern int	 optind;


/***************************
 init and destroy functions
************************************************************************/

/*****************
 scan_node_init()

 DESCR
	initialize scan node

 IN
	fname :	node file name

 OUT
	scan node structure
************************************************************************/

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
	}

	/* return initialized structure or NULL */
	return(pnode);
}


/********************
 scan_node_destroy()

 DESCR
 	destroy scan node structure

 IN
	pnode :	scan node structure

 OUT
 	NONE
************************************************************************/

void scan_node_destroy(scn_node_t *pnode) {
#ifdef PMKSCAN_DEBUG
	/*debugf("destroying node '%s'", pnode->fname);*/
#endif
	if (pnode->fname != NULL) {
		free(pnode->fname);
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


/*****************
 scan_glob_init()

 DESCR
	initialize global scan structure

 IN
	NONE

 OUT
	global scan structure
************************************************************************/

scn_glob_t *scan_glob_init(void) {
	scn_glob_t *pglob;

	pglob = (scn_glob_t *) malloc(sizeof(scn_glob_t));
	if (pglob != NULL) {
		pglob->nodes = hash_init_adv(512, NULL,
			(void (*)(void *))scan_node_destroy, NULL); /* XXX can do better :) */
		pglob->objects = hash_init(512); /* XXX can do better :) */
		pglob->targets = hash_init(512); /* XXX can do better :) */
		pglob->checks = hash_init(256); /* XXX can do better :) */

		pglob->found_asm = false;
		pglob->found_c = false;
		pglob->found_cxx = false;
		pglob->found_lex = false;
		pglob->found_yacc = false;
	}

	return(pglob);
}


/********************
 scan_glob_destroy()

 DESCR
	destroy global scan structure

 IN
	pglob : global scan structure

 OUT
	NONE
************************************************************************/

void scan_glob_destroy(scn_glob_t *pglob) {
	if (pglob->nodes != NULL) {
		hash_destroy(pglob->nodes);
	}

	if (pglob->objects != NULL) {
		hash_destroy(pglob->objects);
	}

	if (pglob->targets != NULL) {
		hash_destroy(pglob->targets);
	}

	if (pglob->checks != NULL) {
		hash_destroy(pglob->checks);
	}

	free(pglob);
}


/***************************
 pmkfile specific functions
************************************************************************/

/******************
 parse_data_file()

 DESCR
	parse data from PMKSCAN_DATA file

 IN
	pdata : parsing data structure
	scandata : scanning data structure

 OUT
	boolean (true on success)
************************************************************************/

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


/*************
 idtf_check()

 DESCR
	check identifier and store relative data in datagen htable

 IN
	idtf : identifier string
	ht_fam : identifier family hash table
	phtgen : datagen hash table
	pht_md : misc data (ex. LANG)

 OUT
	boolean
************************************************************************/

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


/*************
 gen_checks()

 DESCR
	build pmkfile using gathered data

 IN
	psg :	global scan structure
	pht :	check component hash table
	psd :	parsing data structure
 OUT
	boolean
************************************************************************/

bool gen_checks(scn_glob_t *psg, scandata *psd) {
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
	phk = hash_keys(psg->nodes);
	for(i = 0 ; i < phk->nkey ; i++) {
		pn = hash_get(psg->nodes, phk->keys[i]); /* no check needed */

		/* set current language */
		switch(pn->type) {
			case SRC_TYPE_C :
				hash_update_dup(pht_misc, "LANG", PMKSCAN_LANG_C); /* XXX check */
				break;

			case SRC_TYPE_CXX :
				hash_update_dup(pht_misc, "LANG", PMKSCAN_LANG_CXX); /* XXX check */
				break;

			default :
				hash_delete(pht_misc, "LANG");
		}

		/* process system includes */
		for (j = 0 ; j < da_usize(pn->system_inc) ; j++) {
			pstr = (char *) da_idx(pn->system_inc, j);
			idtf_check(pstr, psd->includes, psg->checks, pht_misc);
		}

		/* process function calls */
		for (j = 0 ; j < da_usize(pn->func_calls) ; j++) {
			pstr = (char *) da_idx(pn->func_calls, j);
			idtf_check(pstr, psd->functions, psg->checks, pht_misc);
		}
	}

#ifdef PMKSCAN_DEBUG
	debugf("destroying pht_misc");
#endif
	hash_destroy(pht_misc);

	return(true);
}


/******************
 scan_output_pmk()

 DESCR
	build pmkfile using gathered data

 IN
	fname :	output file name
	psg :	global scan structure
	psd :	parsing data structure
 OUT
	boolean
************************************************************************/

/* XXX TODO : add comments, ad default pmkfile stuff, manage when no checks exists */
bool scan_build_pmk(char *fname, scn_glob_t *psg, scandata *psd) {
	FILE			*fp;
	char			*value;
	hkeys			*phk;
	unsigned int	 i;

	phk = hash_keys(psg->checks);
	if (phk != NULL) {
		fp = fopen(fname, "w");
		if (fp == NULL) {
			errorf("cannot open '%s' : %s.", fname, strerror(errno));
			hash_free_hkeys(phk);
			return(false);
		}

		fprintf(fp, "# pmkfile generated by pmkscan\n\n");

		for(i = 0 ; i < phk->nkey ; i++) {
			value = hash_get(psg->checks, phk->keys[i]);
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


/****************************
 makefile specific functions
************************************************************************/

/************
 find_deps()

 DESCR
	check if function call list has at least one dependency with the
	function declaration list

 IN
	da_fc :		function call list
	da_fd :		function declaration list

 OUT
	boolean
************************************************************************/

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


/**********
 da_find()

 DESCR
	try to find an element in the given dynary

 IN
 	da :	dynary structure
 	str :	string to find

 OUT
 	boolean relative to the result of the search

 NOTE : this only work with dynaries intialised with da_init()
************************************************************************/

bool da_find(dynary *da, char *str) {
	bool		 rslt = false;
	size_t		 i,
				 s;

	/* compute size of string including the delimiter */
	s = strlen(str) + 1;

	/* for each item of the dynary */
	for (i = 0 ; i < da_usize(da) ; i++) {
		/* check if equal to the string */
		if (strncmp(str, da_idx(da, i), s) == 0) {
			/* and set the flag if true */
			rslt = true;
			break;
		}
	}

	return(rslt);
}


/**************
 extract_dir()

 DESCR
	extract the directory portion of the given path

 IN
	path :		original path
	dirbuf :	buffer to store the extracted directory
	blen :		buffer length

 OUT
	NONE
************************************************************************/

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


/*************
 build_path()

 DESCR
	build a path from given directory and filename

 IN
	dir :		directory string
	file :		file name string
	buffer :	buffer where to store the result
	blen :		buffer length

 OUT
	NONE
************************************************************************/

void build_path(char *dir, char *file, char *buffer, size_t blen) {
	if (*dir == '\0') {
		/* directory empty, store only file name */
		strlcpy(buffer, file, blen);
	} else {
		/* join directory and file names with the directory separator */
		snprintf(buffer, blen, "%s/%s", dir, file);
	}
}


/*******************
 recurse_obj_deps()

 DESCR
	gather recursively the local include dependencies

 IN
	nodes :		nodes hash table
	deps :		dynary structure where to store dependencies
	nodename :	node name to process recursively

 OUT
	boolean
************************************************************************/

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


/**************
 gen_objects()

 DESCR
	generate objects from the nodes

 IN
	psg : global scaning data

 OUT
	boolean
************************************************************************/

bool gen_objects(scn_glob_t *psg) {
	char			 buf[MAXPATHLEN], /* XXX filename length */
					*pstr;
	hkeys			*phk;
	scn_node_t		*pnode,
					*pn;
	unsigned int	 i,
					 j;

	phk = hash_keys(psg->nodes);
	for(i = 0 ; i < phk->nkey ; i++) {
		pnode = hash_get(psg->nodes, phk->keys[i]); /* XXX check needed ??? (think no) */
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
			hash_update_dup(psg->objects, buf, pnode->fname); /* XXX check */

			/* XXX */
			pnode->obj_name = strdup(buf);

			/* generate object's source dependencies */
			recurse_obj_deps(psg->nodes, pnode->src_deps, pnode->fname);

			/* for each local include */
			for (j = 0 ; j < da_usize(pnode->local_inc) ; j++) {
				pstr = da_idx(pnode->local_inc, j);
				pn = hash_get(psg->nodes, pstr);

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

	hash_free_hkeys(phk);

	return(true);
}


/*******************
 recurse_src_deps()

 DESCR
	generate targets from the objects

 IN
	psg :	global scanning data
	deps :	dynary structure where to store object dependencies
	name :	node name to process

 OUT
	boolean
************************************************************************/

bool recurse_src_deps(scn_glob_t *psg, dynary *deps, char *name) {
	char		*src,
				*odep;
	scn_node_t	*pnode,
				*pn;
	size_t		 i,
				 j;

	/* get node structure */
	pnode = hash_get(psg->nodes, name);
#ifdef PMKSCAN_DEBUG
	debugf("recurse_src_deps() : node '%s' START", pnode->fname);
#endif

	/* for each source dependency */
	for (i = 0 ; i < da_usize(pnode->src_deps) ; i++) {
		/* get the node structure */
		src = da_idx(pnode->src_deps, i);
		pn = hash_get(psg->nodes, src);
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
				src = hash_get(psg->objects, odep);
#ifdef PMKSCAN_DEBUG
				debugf("recurse_src_deps() : node '%s' : => '%s'", pnode->fname, src);
#endif
				if (recurse_src_deps(psg, deps, src) == false) {
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


/***************
 gen_targets()

 DESCR
	generate targets from the objects

 IN
	psg :	global scaning data

 OUT
	boolean
************************************************************************/

bool gen_targets(scn_glob_t *psg) {
	char			*nodename;
	hkeys			*phk;
	scn_node_t		*pnode;
	unsigned int	 i;

	phk = hash_keys(psg->objects);

	/* for each object */
	for(i = 0 ; i < phk->nkey ; i++) {
		/* get it's node structure */
		nodename = hash_get(psg->objects, phk->keys[i]);
		pnode = hash_get(psg->nodes, nodename); /* XXX check needed ??? (think no) */

		/* if main procedure has been found then it's a target */
		if (pnode->mainproc == true) {
			/* adding in the target list */
			if (hash_update_dup(psg->targets, pnode->prefix,
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
			if (recurse_src_deps(psg, pnode->obj_deps, pnode->fname) == false) {
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


/****************
 fprintf_width()

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
	- could support of a left limit for indentation
************************************************************************/

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


/******************
 scan_output_mkf()

 DESCR
	build makefile using gathered data

 IN
	fname :	file name
	psg :	global scaning data

 OUT
	-
************************************************************************/

bool scan_build_mkf(char *fname, scn_glob_t *psg) {
	FILE			*fp;
	char			 buf[256],
					*pstr;
	hkeys			*phk_o,
					*phk_t;
	size_t			 ofst,
					 lm;
	scn_node_t		*pn;
	unsigned int	 i,
					 j;

	fp = fopen(fname, "w");
	if (fp == NULL) {
		/* XXX err msg */
		return(false);
	}

	/* generating date */
	/*strftime(buf, sizeof(buf), MKF_TIME_GEN, localtime(time(NULL))); |+ XXX check +|*/
	snprintf(buf, sizeof(buf), "TODAY");

	/* set header */
	fprintf(fp, MKF_HEADER_GEN, buf);

	fprintf(fp, "\n# language specific\n");
	/* assembly stuff */
	if (psg->found_asm == true) {
		fprintf(fp, MKF_HEADER_ASM);
	}
	/* C stuff */
	if (psg->found_c == true) {
		fprintf(fp, MKF_HEADER_C);
	}
	/* C++ stuff */
	if (psg->found_cxx == true) {
		fprintf(fp, MKF_HEADER_CXX);
	}
	/* lex stuff */
	if (psg->found_lex == true) {
		fprintf(fp, MKF_HEADER_LEX);
	}
	/* yacc stuff */
	if (psg->found_yacc == true) {
		fprintf(fp, MKF_HEADER_YACC);
	}

	fprintf(fp, "\n# misc stuff\n");
	/* linker stuff */
	fprintf(fp, MKF_HEADER_LD);
	/* misc stuff */
	fprintf(fp, MKF_HEADER_MISC);

	/* suffixes */
	fprintf(fp, MKF_SUFFIXES);

	/* assembly object build rule */
	if (psg->found_asm == true) {
		fprintf(fp, MKF_BLD_ASM_OBJ);
	}

	/* C object build rule */
	if (psg->found_c == true) {
		fprintf(fp, MKF_BLD_C_OBJ);
	}

	/* C++ object build rule */
	if (psg->found_cxx == true) {
		fprintf(fp, MKF_BLD_CXX_OBJ);
	}

	/*  lex source build rule */
	if (psg->found_lex == true) {
		fprintf(fp, MKF_BLD_LEX_SRC);
	}

	/*  yacc source build rule */
	if (psg->found_yacc == true) {
		fprintf(fp, MKF_BLD_YACC_SRC);
	}

	/* generate object dependency lists */
	phk_o = hash_keys(psg->objects);
	/* generate object deps */
	fprintf(fp, "\n# object dependency lists\n");
	for (i = 0 ; i < phk_o->nkey ; i++) {
		pstr = hash_get(psg->objects, phk_o->keys[i]);
		pn = hash_get(psg->nodes, pstr);

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

	/* generate target dependency lists */
	phk_t = hash_keys(psg->targets);
	if (phk_t != NULL) {
		/* generate target deps */
		fprintf(fp, "\n# target dependency lists\n");
		for(i = 0 ; i < phk_t->nkey ; i++) {
			pstr = hash_get(psg->targets, phk_t->keys[i]);
			pn = hash_get(psg->nodes, pstr);

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
	}

	/* generate building target list */
	fprintf(fp, MKF_TRGT_ALL_VAR);
	/* list of targets */
	if (phk_t != NULL) {
		lm = strlen(MKF_TRGT_ALL_VAR);
		ofst = lm;
		for(i = 0 ; i < phk_t->nkey ; i++) {
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst,
												fp, phk_t->keys[i]);
		}
		fprintf(fp, MKF_TWICE_JUMP);
	}

	/* generate cleaning target list */
	fprintf(fp, MKF_TRGT_CLEAN_VAR);
	/* list of targets */
	if (phk_t != NULL) {
		lm = strlen(MKF_TRGT_CLEAN_VAR);
		ofst = lm;
		for(i = 0 ; i < phk_t->nkey ; i++) {
			snprintf(buf, sizeof(buf), "%s_clean", phk_t->keys[i]);
			ofst = fprintf_width(lm, MKF_OUTPUT_WIDTH, ofst, fp, buf);
		}
		fprintf(fp, MKF_TWICE_JUMP);
	}


	fprintf(fp, "\n# main targets\n");
	fprintf(fp, MKF_TARGET_ALL);
	fprintf(fp, MKF_TARGET_CLEAN);


	fprintf(fp, MKF_TWICE_JUMP);

	/* generate objects */
	fprintf(fp, "\n# object rules\n");
	for(i = 0 ; i < phk_o->nkey ; i++) {
		pstr = hash_get(psg->objects, phk_o->keys[i]);
		pn = hash_get(psg->nodes, pstr);

		/* object label */
		str_to_upper(buf, sizeof(buf), pn->prefix);
		fprintf(fp, MKF_OBJECT_LABL, phk_o->keys[i], buf);

		fprintf(fp, MKF_TWICE_JUMP);
	}

	if (phk_t != NULL) {
		/* generate targets */
		fprintf(fp, "\n# target rules\n");
		for(i = 0 ; i < phk_t->nkey ; i++) {
			pstr = phk_t->keys[i];

			/* uppercase string */
			str_to_upper(buf, sizeof(buf), pstr);

			/* build target */
			fprintf(fp, MKF_TARGET_LABL, pstr, buf, pstr, buf);

			/* clean target */
			fprintf(fp, MKF_TARGET_CLN, pstr, buf, pstr);
		}
	}

	fprintf(fp, MKF_TARGET_INST);

	hash_free_hkeys(phk_o);
	hash_free_hkeys(phk_t);

	fclose(fp);

	return(true);
}


/*****************
 common functions
************************************************************************/

/***************
 str_to_upper()

 DESCR
	store in the buffer the conversion in upper case of the string

 IN
	buf :	storage buffer
	siz :	size of buffer
	str :	string to convert

 OUT
	-
************************************************************************/

void str_to_upper(char *buf, size_t siz, char *str) {
	while ((siz > 1) && (*str != '\0')) {
		*buf = toupper(*str);
		buf++;
		str++;
		siz--;
	}

	*buf = '\0';
}


/*****************
 check_file_ext()

 DESCR
	check the file extension and return the supposed file type

 IN
	fname :	file name

 OUT
	file type
************************************************************************/

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
	return(SRC_TYPE_UNKNOWN);
}


/**************
 regex_check()

 DESCR
	check a line with regex pattern

 IN
	pattern : pattern to use
	line : string to check

 OUT
	return : found string or NULL
************************************************************************/

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


/***************
 process_ppro()

 DESCR
	called when a preprocessor directive is found

 IN
	data :	parsing data
	pstr :	directive identifier
	ppe :	parsing engine structure

 OUT
	boolean
************************************************************************/

bool process_ppro(void *data, char *pstr, prseng_t *ppe) {
	char		 iname[MAXPATHLEN],
				 c;
	scn_misc	*psm;
	scn_node_t	*pnode;

	psm = (scn_misc *) data;
	pnode = psm->pnode;

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


/********************
 process_proc_call()

 DESCR
	called when a procedure call is found

 IN
	data :	parsing data
	pstr :	function call identifier
	ppe :	parsing engine structure

 OUT
	boolean
************************************************************************/

bool process_proc_call(void *data, char *pstr, prseng_t *ppe) {
	scn_misc	*psm;
	scn_node_t	*pnode;

	psm = (scn_misc *) data;
	pnode = psm->pnode;

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


/********************
 process_proc_decl()

 DESCR
	called when a procedure declaration is found

 IN
	data :	parsing data
	pstr :	function declarator identifier
	ppe :	parsing engine structure

 OUT
	boolean
************************************************************************/

bool process_proc_decl(void *data, char *pstr, prseng_t *ppe) {
	scn_misc	*psm;
	scn_node_t	*pnode;

	psm = (scn_misc *) data;
	pnode = psm->pnode;

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


/***************
 process_type()

 DESCR
	called when a type identifier is found

 IN
	data :	parsing data
	pstr :	type identifier
	ppe :	parsing engine structure

 OUT
	boolean
************************************************************************/

bool process_type(void *data, char *pstr, prseng_t *ppe) {
	scn_misc	*psm;
	scn_node_t	*pnode;

	psm = (scn_misc *) data;
	pnode = psm->pnode;

	/* add type in list */
	if (da_push(pnode->type_idtfs, strdup(pstr)) == false) {
		errorf("unable to add '%s' in type list", pstr);
		return(false);
	}

	return(true);
}


/*************
 parse_file()

 DESCR
	parse a file that has a known type

 IN
	pcmn :	XXX
	pnode :	XXX
	fname :	XXX
	ft :	XXX

 OUT
	boolean
************************************************************************/

bool parse_file(prs_cmn_t *pcmn, scn_node_t *pnode, char *fname, ftype_t ft) {
	FILE			*fp;
	scn_misc		*psm;

	/* open file */
	fp = fopen(fname, "r");
	if (fp == NULL) {
		errorf("cannot open '%s' : %s.", fname, strerror(errno));
		return(false);
	}

	psm = (scn_misc *) pcmn->data;
	psm->pnode = pnode;

	switch (ft) {
		case SRC_TYPE_ASM :
			*psm->is_asm = true;
			/* XXX parsing */
			break;

		case SRC_TYPE_C :
			*psm->is_c = true;
			if (prs_c_file(pcmn, fp) == false) {
				return(false);
			}
			break;

		case SRC_TYPE_CXX :
			*psm->is_cxx = true;
			/* XXX parsing */
			break;

		case SRC_TYPE_LEX :
			*psm->is_lex = true;
			/* XXX parsing */
			break;

		case SRC_TYPE_YACC :
			*psm->is_yacc = true;
			/* XXX parsing */
			break;

#ifdef PMKSCAN_DEBUG
		default :
			debugf("cannot parse filetype %d", ft);
#endif
	}

	/* close file */
	fclose(fp);

	return(true);
}


/*****************
 scan_node_file()

 DESCR
	scan a node file to extract useful data

 IN
	pcmn :	XXX
	fname :	XXX
	isdep :	flag to notice if fname is a dependency or not

 OUT
	boolean
************************************************************************/

bool scan_node_file(prs_cmn_t *pcmn, char *fname, bool isdep) {
	char			 buf[MAXPATHLEN],
					 dir[MAXPATHLEN];
	ftype_t			 ft;
	scn_node_t		*pnode;
	scn_misc		*psm;
	unsigned int	 i;

#ifdef PMKSCAN_DEBUG
	debugf("fname = '%s'", fname);
#endif

	psm = (scn_misc *) pcmn->data;

	/* find a better fix or way to avoid to do that */
	if ((fname[0] == '.') && (fname[1] == '/')) {
		fname = fname + 2;
	}

	ft = check_file_ext(fname);
	if (ft == SRC_TYPE_UNKNOWN) {
		/* display a file with unknown type */
		printf(".");

		/* skip unsupported file extension */
		return(true);
	}

	/* check if this node is already existing */
	pnode = hash_get(psm->nodes, fname);
	if (pnode == NULL) {
#ifdef PMKSCAN_DEBUG
		debugf("adding node for '%s'", fname); /* XXX */
#endif
		/* not yet, create new node */
		pnode = scan_node_init(fname);
		if (pnode == NULL) {
			errorf("unable to initialize scan node");
			return(false);
		}

#ifdef PMKSCAN_DEBUG
		debugf("created node '%s'", fname); /* XXX */
#endif

		pnode->type = ft;

		if (parse_file(pcmn, pnode, fname, ft) == false) {
			/* XXX err msg ? */
			scan_node_destroy(pnode);
			return(false);
		}

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
	if (hash_add(psm->nodes, fname, pnode) == HASH_ADD_FAIL) {
		errorf("failed to add node '%s' in the hash table.", pnode->fname);
		scan_node_destroy(pnode);
		return(false);
	}

	/* get directory name */
	extract_dir(fname, dir, sizeof(dir));

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


/***********
 scan_dir()

 DESCR
	scan a directory to find and scan known file type

 IN
	pcmn :		XXX
	dir :		XXX
	recursive :	XXX

 OUT
	boolean
************************************************************************/

bool scan_dir(prs_cmn_t *pcmn, char *dir, bool recursive) {
	DIR				*pd;
	struct dirent	*pde;
	char			 buf[MAXPATHLEN],
					*fname;

	pd = opendir(dir);

	if (pd == NULL) {
		/* this is not a directory */
		return(false);
	}

#ifdef PMKSCAN_DEBUG
	debugf("scanning directory '%s'", dir);
#endif


	/* check each directory's entries */
	while ((pde = readdir(pd)) && (pde != NULL)) {
		fname = pde->d_name;

#ifdef PMKSCAN_DEBUG
		debugf("checking entry '%s'", fname);
#endif

		/* build full path */
		build_path(dir, fname, buf, sizeof(buf));

		/* if the entry is a directory ... */
		if (pde->d_type == DT_DIR) {

			/* ... then display a 'D' ... */
			printf("D");

			/* ... and if recursivity is enabled ... */
			if (recursive == true) {
				/* ... then display start of recurse ... */
				printf("[");

				/* ... then process the directory ... */
				scan_dir(pcmn, buf, recursive);

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

	return(true);
}


/********
 usage()

 DESCR
	pmkscan(1) usage

 IN
	NONE
 OUT
	NONE
************************************************************************/

void usage(void) {
	fprintf(stderr, "usage: pmkscan [-hv] [path]\n");
}


/*******
 main()

 DESCR
	main loop.
************************************************************************/

int main(int argc, char *argv[]) {
	bool		 go_exit = false,
				 recursive = false,
				 gen_mkf = false,
				 gen_pmk = false;
	char		 buf[MAXPATHLEN];
	int			 chr;
	prs_cmn_t	 pcmn;
	prsdata		*pdata = NULL; /* XXX into global struct ? */
	scandata	 sd;
	scn_glob_t	*psg;
	scn_misc	 sm;

	while (go_exit == false) {
		chr = getopt(argc, argv, "hmprv");
		switch (chr) {
			case -1 :
				go_exit = true;
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

	psg = scan_glob_init(); /* XXX check */
	if (psg == NULL) {
		/* XXX err msg */
		exit(EXIT_FAILURE);
	}

	/* init XXX */
	sm.pcmn = &pcmn;
	sm.nodes = psg->nodes;
	sm.is_asm = &psg->found_asm;
	sm.is_c = &psg->found_c;
	sm.is_cxx = &psg->found_cxx;
	sm.is_lex = &psg->found_lex;
	sm.is_yacc = &psg->found_yacc;

	/* init common parser structure */
	pcmn.func_ppro = &process_ppro;
	pcmn.func_proc = &process_proc_call;
	pcmn.func_decl = &process_proc_decl;
	pcmn.func_type = &process_type;
	pcmn.data = &sm;

	if (gen_pmk == true) {
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
		gen_checks(psg, &sd);

		printf("Generating %s ...\n", PMKSCAN_PMKFILE);
		scan_build_pmk(PMKSCAN_PMKFILE, psg, &sd);
		printf("Ok\n\n");
	}

	/* makefile stuff */
	if (gen_mkf == true) {
		/* scanning resulting nodes */
		printf("Processing nodes for object generation ...\n");
		gen_objects(psg);
		printf("Ok\n\n");

		/* scanning generated objects */
		printf("Processing objects for target generation ...\n");
		gen_targets(psg);
		printf("Ok\n\n");

		printf("Generating %s ...\n", PMKSCAN_MKFILE);
		scan_build_mkf(PMKSCAN_MKFILE, psg);
		printf("Ok\n\n");
	}

	/* cleaning */
	scan_glob_destroy(psg);
	if (gen_pmk == true) {
		prsdata_destroy(pdata);
	}

	printf("\nWork complete.\n\n");

	return(0);
}
