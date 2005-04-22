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
		{"*.C",	SRC_TYPE_C},
		{"*.c",	SRC_TYPE_C},
		{"*.cc",	SRC_TYPE_C},
		{"*.cpp",	SRC_TYPE_CXX},
		{"*.cxx",	SRC_TYPE_CXX},
		{"*.c++",	SRC_TYPE_CXX},
		{"*.H",	SRC_TYPE_C},
		{"*.h",	SRC_TYPE_C},
		{"*.hh",	SRC_TYPE_C},
		{"*.hpp",	SRC_TYPE_CXX},
		{"*.hxx",	SRC_TYPE_CXX},
		{"*.h++",	SRC_TYPE_CXX},
		{"*.S",	SRC_TYPE_ASM},
		{"*.s",	SRC_TYPE_ASM}
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
		len++; /* get the size including null char */
		pnode->prefix = (char *) malloc(len);
		strlcpy(pnode->prefix, fname, len);

		/* init dynamic array of dependencies */
		pnode->s_deps = da_init();
		if (pnode->s_deps == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->l_deps = da_init();
		if (pnode->l_deps == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->fc_deps = da_init();
		if (pnode->fc_deps == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->fd_deps = da_init();
		if (pnode->fd_deps == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dynamic array of dependencies */
		pnode->t_deps = da_init();
		if (pnode->t_deps == NULL) {
			/* failed */
			scan_node_destroy(pnode);
			return(NULL);
		}

		/* init dependency state */
		pnode->isdep = false; /* useful ? see scan_file() */

		/* init main() procedure flag */
		pnode->mainproc = false;

		/* init dependency score */
		pnode->score = 0;

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

	if (pnode->s_deps != NULL) {
		da_destroy(pnode->s_deps);
	}

	if (pnode->l_deps != NULL) {
		da_destroy(pnode->l_deps);
	}

	if (pnode->fc_deps != NULL) {
		da_destroy(pnode->fc_deps);
	}

	if (pnode->fd_deps != NULL) {
		da_destroy(pnode->fd_deps);
	}

	if (pnode->t_deps != NULL) {
		da_destroy(pnode->t_deps);
	}

	free(pnode);
}


/*******************
 scan_object_init()

 DESCR
	initialize scan object

 IN
	oname : object name

 OUT
	scan object structure
************************************************************************/

scn_obj_t *scan_object_init(char *oname) {
	scn_obj_t	*pobj;

	pobj = (scn_obj_t *) malloc(sizeof(scn_obj_t));
	if (pobj != NULL) {
		/* init dynamic array of dependencies */
		pobj->deps = da_init();
		if (pobj->deps == NULL) {
			/* failed */
			scan_object_destroy(pobj);
			return(NULL);
		}

		pobj->name = strdup(oname);
		pobj->node = NULL;
		pobj->type = OBJ_TYPE_UNKNOWN;
		pobj->is_trgt = false;
	}

	return(pobj);
}


/**********************
 scan_object_destroy()

 DESCR
	destroy scan object structure

 IN
	pobj : scan object structure

 OUT
	NONE
************************************************************************/

void scan_object_destroy(scn_obj_t *pobj) {
	if (pobj->name != NULL) {
		free(pobj->name);
	}

	if (pobj->node != NULL) {
		free(pobj->node);
	}

	if (pobj->deps != NULL) {
		da_destroy(pobj->deps);
	}

	free(pobj);
}


/*******************
 scan_target_init()

 DESCR
	initialize scan target

 IN
	tname : target name

 OUT
	scan target structure
************************************************************************/

scn_trgt_t *scan_target_init(char *tname) {
	scn_trgt_t	*ptrgt;

	ptrgt = (scn_trgt_t *) malloc(sizeof(scn_trgt_t));
	if (ptrgt != NULL) {
		ptrgt->name = strdup(tname);

		/* init dynamic array of dependencies */
		ptrgt->deps = da_init();
		if (ptrgt->deps == NULL) {
			/* failed */
			scan_target_destroy(ptrgt);
			return(NULL);
		}
	}

	return(ptrgt);
}


/**********************
 scan_target_destroy()

 DESCR
	destroy scan target structure

 IN
	ptrgt : scan target structure

 OUT
	NONE
************************************************************************/

void scan_target_destroy(scn_trgt_t *ptrgt) {
	if (ptrgt->name != NULL) {
		free(ptrgt->name);
	}

	if (ptrgt->deps != NULL) {
		da_destroy(ptrgt->deps);
	}

	free(ptrgt);
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
		pglob->objects = hash_init_adv(512, NULL,
			(void (*)(void *))scan_object_destroy, NULL); /* XXX can do better :) */
		pglob->targets = hash_init_adv(256, NULL,
			(void (*)(void *))scan_target_destroy, NULL); /* XXX can do better :) */
		pglob->checks = hash_init(256); /* XXX can do better :) */

		pglob->is_asm = false;
		pglob->is_c = false;
		pglob->is_cxx = false;
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
		for (j = 0 ; j < da_usize(pn->s_deps) ; j++) {
			pstr = (char *) da_idx(pn->s_deps, j);
			idtf_check(pstr, psd->includes, psg->checks, pht_misc);
		}

		/* process function calls */
		for (j = 0 ; j < da_usize(pn->fc_deps) ; j++) {
			pstr = (char *) da_idx(pn->fc_deps, j);
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

	for (i = 0 ; i < da_usize(da_fc) ; i++) {
		str_fc = da_idx(da_fc, i);
		siz = strlen(str_fc) + 1;

		for (j = 0 ; j < da_usize(da_fd) ; j++) {
			str_fd = da_idx(da_fd, j);
			if (strncmp(str_fc, str_fd, siz) == 0) {
#ifdef PMKSCAN_DEBUG
#endif
				debugf("found common dep '%s'", str_fc);
				return(true);
			}
		}

	}

	return(false);
}


/*******************
 recurse_obj_deps()

 DESCR
	gather recursively the target dependencies

 IN
	objects :	scan object list
	t_deps :	object dependency list of the target
	oname :		object name being recursively checked

 OUT
	-
************************************************************************/

void recurse_obj_deps(htable *objects, dynary *t_deps, char *oname) {
	bool			 found;
	char			*odep;
	scn_obj_t		*pobj;
	size_t			 s;
	unsigned int	 i,
					 j;

	pobj = hash_get(objects, oname);

	/* for every node of the dependency list ... */
	for (i = 0 ; i < da_usize(pobj->deps) ; i++) {
		odep = da_idx(pobj->deps, i);
		s = strlen(odep);

		/* ... check if it is already listed as target dependency ...*/
		found = false;
		for (j = 0 ; j < da_usize(t_deps) ; j++) {
			if (strncmp(odep, da_idx(t_deps, j), s) == 0) {
				found = true;
				break;
			}
		}

		/* ... and if it's not then ... */
		if (found == false) {
			/* ... add the new dependency ... */
			da_push(t_deps, strdup(odep));

			/* ... and recurse it */
			recurse_obj_deps(objects, t_deps, odep);
		}
	}
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
	char			 buf[MAXPATHLEN]; /* XXX filename length */
	hkeys			*phk;
	scn_node_t		*pnode,
					*pn;
	scn_obj_t		*pobj;
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

			pobj = scan_object_init(buf);
			if (pobj == NULL) {
				errorf("line %d", __LINE__); /* XXX err */
				return(false);
			}

			/* record object */
			if (hash_add(psg->objects, buf, pobj) == HASH_ADD_FAIL) {
				errorf("line %d", __LINE__); /* XXX err */
				return(false);
			}

			/* node reference and copy type */
			pobj->pnode = pnode;
			pobj->type = pnode->type;

			/*
				check function call dependencies between current node
				and other nodes
			*/
			for(j = 0 ; j < phk->nkey ; j++) {
				if (i == j) {
					/* skip checking current node with itself */
					continue;
				}

				pn = hash_get(psg->nodes, phk->keys[j]); /* no check needed */
				if (pn->score != 0) {
					/* keep only object's main file */
					continue;
				}
				if (pn->mainproc == true) {
					/* keep only object's main file */
					continue;
				}

				if (find_deps(pnode->fc_deps, pn->fd_deps) == true) {

					/* build dependency object name */
					strlcpy(buf, pn->prefix, sizeof(buf));
					strlcat(buf, OBJ_SUFFIX, sizeof(buf));
#ifdef PMKSCAN_DEBUG
#endif
					debugf("adding '%s' dependency to node '%s'", buf, pobj->name);
					da_push(pobj->deps, strdup(buf)); /* XXX check ? */
				}
			}

			if (pnode->mainproc == true) {
#ifdef PMKSCAN_DEBUG
				debugf("node '%s' has main proc", phk->keys[i]);
#endif
				/* this objet is the main part of a target */
				pobj->is_trgt = true;
			}
		}
	}

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
	hkeys			*phk;
	scn_node_t		*pnode;
	scn_obj_t		*pobj;
	scn_trgt_t		*ptrgt;
	unsigned int	 i;

	phk = hash_keys(psg->objects);

	/*
		check dependencies between each objects
	*/
	for (i = 0 ; i < phk->nkey ; i++) {
#ifdef PMKSCAN_DEBUG
		debugf("scanning object '%s'", phk->keys[i]);
#endif
		pobj = hash_get(psg->objects, phk->keys[i]); /* no check needed */

		pnode = pobj->pnode;

		if (pobj->is_trgt == true) {
#ifdef PMKSCAN_DEBUG
			debugf("object '%s' is a target", pobj->name);
#endif
			/* create new target */
			ptrgt = scan_target_init(pnode->prefix);
			if (ptrgt == NULL) {
				/* XXX err */
				return(false);
			}

			/* record target */
			hash_add(psg->targets, pnode->prefix, ptrgt); /* XXX check */

			/* add main object in the deps list */
			da_push(ptrgt->deps, strdup(pobj->name)); /* XXX check ? */

			/* recursively generate dependencies starting from main object */
			recurse_obj_deps(psg->objects, ptrgt->deps, pobj->name);
		}
	}

	return(true);
}


/****************
 fprintf_width()

 DESCR

 IN

 OUT

************************************************************************/

size_t fprintf_width(size_t width, size_t offset, FILE *fp, char *str) {
	char	*fmt;
	size_t	 s;

	/* compute new offset with the string length */
	s = strlen(str);
	offset = offset + s;

	/* check if offset is greater than allowed width */
	if (offset < width) {
		/* got enough space on the line */
		fmt = " %s";
	} else {
		/* need to go to on the next line */
		fmt = " \\\n\t%s";
		/* update offset counting the 8 characters for the tabulation */
		offset = 8 + s;
	}

	/* print formated string */
	fprintf(fp, fmt, str);

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

void scan_build_mkf(char *fname, scn_glob_t *psg) {
	FILE			*fp;
	char			 buf[256],
					*pstr;
	hkeys			*phk_o,
					*phk_t;
	size_t			 ofst;
	scn_node_t		*pn;
	scn_obj_t		*po;
	scn_trgt_t		*pt;
	unsigned int	 i,
					 j;

	fp = fopen(fname, "w"); /* XXX check */

	fprintf(fp, "# makefile template generated by pmkscan\n");

	fprintf(fp, "\n# language specific\n");
	/* assembly stuff */
	if (psg->is_asm == true) {
		fprintf(fp, MKF_HEADER_ASM);
	}
	/* C stuff */
	if (psg->is_c == true) {
		fprintf(fp, MKF_HEADER_C);
	}
	/* C++ stuff */
	if (psg->is_cxx == true) {
		fprintf(fp, MKF_HEADER_CXX);
	}

	/* XXX */
	phk_o = hash_keys(psg->objects);
	/* generate object deps */
	fprintf(fp, "\n# object dependency lists\n");
	for(i = 0 ; i < phk_o->nkey ; i++) {
		pstr = phk_o->keys[i];
		po = hash_get(psg->objects, pstr);
		pn = po->pnode;

		/* object label */
		str_to_upper(buf, sizeof(buf), pn->prefix);
		fprintf(fp, MKF_OBJECT_SRCS, buf);

		/* append sources */
		ofst = strlen(buf) + 14; /* XXX */
		for (j = 0 ; j < da_usize(pn->l_deps) ; j++) {
			ofst = fprintf_width(MKF_OUTPUT_WIDTH, ofst, fp,
									(char *) da_idx(pn->l_deps, j));
		}
		fprintf_width(MKF_OUTPUT_WIDTH, ofst, fp, pn->fname);


		fprintf(fp, MKF_TWICE_JUMP);
	}

	/* XXX */
	phk_t = hash_keys(psg->targets);
	if (phk_t != NULL) {
		/* generate target deps */
		fprintf(fp, "\n# target dependency lists\n");
		for(i = 0 ; i < phk_t->nkey ; i++) {
			pstr = phk_t->keys[i];
			pt = hash_get(psg->targets, pstr);

			/* target label */
			str_to_upper(buf, sizeof(buf), pstr);
			fprintf(fp, MKF_TARGET_OBJS, buf);

			/* XXX TODO append objects */
			ofst = strlen(buf) + 14; /* XXX */
			for (j = 0 ; j < da_usize(pt->deps) ; j++) {
				ofst = fprintf_width(MKF_OUTPUT_WIDTH, ofst, fp,
										(char *) da_idx(pt->deps, j));
			}

			fprintf(fp, MKF_TWICE_JUMP);
		}
	}

	/* main target */
	fprintf(fp, "\n# main target\n");
	fprintf(fp, MKF_TARGET_ALL, "");
	/* XXX TODO list of targets */
	fprintf(fp, MKF_TWICE_JUMP);

	/* generate objects */
	fprintf(fp, "\n# objects\n");
	for(i = 0 ; i < phk_o->nkey ; i++) {
		pstr = phk_o->keys[i];
		po = hash_get(psg->objects, pstr);
		pn = po->pnode;

		/* object label */
		str_to_upper(buf, sizeof(buf), pn->prefix);
		fprintf(fp, MKF_OBJECT_LABL, pstr, buf);

		/* object build depending on the source type */
		/* XXX TODO
		fprintf(fp, MKF_BLD_ASM, XXX);
		fprintf(fp, MKF_BLD_C, XXX);
		fprintf(fp, MKF_BLD_CXX, XXX);
		*/

		fprintf(fp, MKF_TWICE_JUMP);
	}

	if (phk_t != NULL) {
		/* generate targets */
		fprintf(fp, "\n# targets\n");
		for(i = 0 ; i < phk_t->nkey ; i++) {
			pstr = phk_t->keys[i];

			/* build target */
			fprintf(fp, MKF_TARGET_LABL, pstr, pstr, pstr, pstr);

			/* clean target */
			fprintf(fp, MKF_TARGET_CLN, pstr, pstr, pstr);
		}
	}

	fprintf(fp, MKF_TARGET_INST);

	fclose(fp);
}


/*****************
 common functions
************************************************************************/

/***************
 str_to_upper()

 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
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
	XXX

 IN
	XXX

 OUT
	XXX
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

char *regex_check(char *pattern, char *line) {
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
	XXX

 IN
	XXX

 OUT
	XXX
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
				if (da_push(pnode->l_deps, strdup(iname)) == false) {
					errorf("unable to add '%s' in local deps", iname);
					return(false);
				}

				break;

			case '<' :
#ifdef PMKSCAN_DEBUG
				debugf("found system include '%s'", iname);
#endif
				/* add include in depedencies */
				if (da_push(pnode->s_deps, strdup(iname)) == false) {
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
	XXX

 IN
	XXX

 OUT
	XXX
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
	if (da_push(pnode->fc_deps, strdup(pstr)) == false) {
		errorf("unable to add '%s' in function call list", pstr);
		return(false);
	}

	return(true);
}


/********************
 process_proc_decl()

 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
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
	if (da_push(pnode->fd_deps, strdup(pstr)) == false) {
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
	XXX

 IN
	XXX

 OUT
	XXX
************************************************************************/

bool process_type(void *data, char *pstr, prseng_t *ppe) {
	scn_misc	*psm;
	scn_node_t	*pnode;

	psm = (scn_misc *) data;
	pnode = psm->pnode;

	/* add type in list */
	if (da_push(pnode->t_deps, strdup(pstr)) == false) {
		errorf("unable to add '%s' in type list", pstr);
		return(false);
	}

	return(true);
}


/*************
 parse_file()

 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
************************************************************************/

bool parse_file(prs_cmn_t *pcmn, scn_node_t *pnode, char *fname, ftype_t ft) {
	FILE			*fp;
	char			 buf[MAXPATHLEN],
					 dir[MAXPATHLEN];
	unsigned int	 i;
	scn_misc		*psm;

	/* open file */
	fp = fopen(fname, "r");
	if (fp == NULL) {
		errorf("cannot open '%s' : %s.", fname, strerror(errno));
		return(false);
	}

	/* init directory and file names */
	strlcpy(buf, fname, sizeof(buf));
	strlcpy(dir, dirname(buf), sizeof(dir));

	psm = (scn_misc *) pcmn->data;
	psm->pnode = pnode;

	switch (ft) {
		case SRC_TYPE_C :
			if (prs_c_file(pcmn, fp) == false) {
				return(false);
			}
			break;

#ifdef PMKSCAN_DEBUG
		default :
			debugf("cannot parse filetype %d", ft);
#endif
	}

	for (i = 0 ; i < da_usize(pnode->l_deps) ; i++) {
		/* scan local include */
		snprintf(buf, sizeof(buf), "%s/%s", dir, (char *)da_idx(pnode->l_deps, i));
		if (scan_node_file(pcmn, buf, true) == false) {
			errorf("failed to scan file '%s'.", da_idx(pnode->l_deps, i));
			return(false);
		}
	}

	/* close file */
	fclose(fp);

	return(true);
}


/*****************
 scan_node_file()

 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
************************************************************************/

bool scan_node_file(prs_cmn_t *pcmn, char *fname, bool isdep) {
	ftype_t		 ft;
	scn_node_t	*pnode;
	scn_misc	*psm;

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
		debugf("pnode->fname = '%s'", pnode->fname); /* XXX */
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

	return(true);
}


/***********
 scan_dir()

 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
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
		snprintf(buf, sizeof(buf), "%s/%s", dir, fname); /* XXX check ? */

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
	sm.nodes = psg->nodes;
	sm.pcmn = &pcmn;
	sm.dir = NULL;
	sm.pnode = NULL;

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
	scan_dir(&pcmn, buf, recursive);
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
