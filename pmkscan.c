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

/*#define PMKSCAN_DEBUG	1*/


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
	char	*pstr;
	scn_node_t	*pnode;
	size_t	 len;

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
		pnode->f_deps = da_init();
		if (pnode->f_deps == NULL) {
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
debugf("destroying node '%s'", pnode->fname);
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

	if (pnode->f_deps != NULL) {
		da_destroy(pnode->f_deps);
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
	char		 buf[TMP_BUF_LEN],
			*pval,
			*p;
	dynary		*da;
	pmkobj		*po;
	potype		 pot;
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
	char		*pstr;
	hkeys		*phk;
	scn_node_t	*pn;
	unsigned int	 i,
			 j;
	htable	*pht_misc;

	pht_misc = hash_init(256); /* XXX */
	if (pht_misc == NULL) {
		return(false);
	}

	phk = hash_keys(psg->nodes);
	for(i = 0 ; i < phk->nkey ; i++) {
		pn = hash_get(psg->nodes, phk->keys[i]); /* XXX check needed ??? (think no) */

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
		for (j = 0 ; j < da_usize(pn->f_deps) ; j++) {
			pstr = (char *) da_idx(pn->f_deps, j);
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
	FILE		*fp;
	char		*value;
	hkeys		*phk;
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
	XXX

 IN
	XXX

 OUT
	XXX
************************************************************************/

bool find_deps(dynary *da_a, dynary *da_b) {
	char		*str_a,
			*str_b;
	size_t		 siz;
	unsigned int	 i,
			 j;

	for (i = 0 ; i < da_usize(da_a) ; i++) {
		str_a = da_idx(da_a, i);
		siz = strlen(str_a) + 1;

		for (j = 0 ; j < da_usize(da_b) ; j++) {
			str_b = da_idx(da_b, j);
			if (strncmp(str_a, str_b, siz) == 0) {
				return(true);
			}
		}

	}

	return(false);
}


/**************
 gen_objects()

 DESCR
	generate objects from the nodes

 IN
	XXX

 OUT
	XXX
************************************************************************/

void gen_objects(scn_glob_t *psg) {
	char		 buf[MAXPATHLEN]; /* XXX filename length */
	hkeys		*phk;
	scn_node_t	*pnode;
	scn_obj_t	*pobj;
	unsigned int	 i;

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
				/* XXX err */
				errorf("line %d", __LINE__);
				return;
			}

			/* record object */
			if (hash_add(psg->objects, buf, pobj) == HASH_ADD_FAIL) {
				/* XXX err */
				errorf("line %d", __LINE__);
				return;
			}

			pobj->pnode = pnode;

			/* XXX obj type ? */

			if (pnode->mainproc == true) {
/*debugf("node '%s' has main proc", phk->keys[i]);*/
				/* this objet is the main part of a target */
				pobj->is_trgt = true;
			}
		}

	}
}


/***************
 gen_targets()

 DESCR
	generate targets from the objects

 IN
	XXX

 OUT
	XXX
************************************************************************/

void gen_targets(scn_glob_t *psg) {
	hkeys		*phk;
	scn_node_t	*pnode,
			*pn;
	scn_obj_t	*pobj,
			*po;
	scn_trgt_t	*ptrgt;
	unsigned int	 i,
			 j;

	phk = hash_keys(psg->objects);
	for(i = 0 ; i < phk->nkey ; i++) {
/*debugf("scanning object '%s'", phk->keys[i]); |+ XXX +|*/
		pobj = hash_get(psg->objects, phk->keys[i]); /* XXX check needed ??? (think no) */
/*debugf("object is '%s'", pobj->name); |+ XXX +|*/

		pnode = pobj->pnode;

		if (pobj->is_trgt != false) {
/*debugf("object '%s' is a target", pobj->name); |+ XXX +|*/
			/* XXX create and store new target */
			ptrgt = scan_target_init(pnode->prefix);
			if (ptrgt == NULL) {
				/* XXX err */
				return;
			}

			/* record target */
			hash_add(psg->targets, pnode->prefix, ptrgt); /* XXX check */

			/* add main object in the deps list */
			da_push(ptrgt->deps, strdup(pobj->name)); /* XXX check ? */

			/*
				link the target with every objects that have at least one
				common local dep but which are not a target.

				XXX not good !!!!
			*/
			for(j = 0 ; j < phk->nkey ; j++) {
				po = hash_get(psg->objects, phk->keys[j]); /* XXX check needed ??? (think no) */

				if (po->is_trgt == true) {
					continue;
				}

				pn = po->pnode;

				if (find_deps(pnode->l_deps, pn->l_deps) == true) {
					da_push(ptrgt->deps, strdup(po->name)); /* XXX check ? */
				}
			}
		}
	}
}


/******************
 scan_output_mkf()

 DESCR
	build makefile using gathered data

 IN
	XXX

 OUT
	XXX
************************************************************************/

void scan_build_mkf(char *fname, scn_glob_t *psg) {
	FILE		*fp;
	char		 buf[256],
			*pstr;
	hkeys		*phk_o,
			*phk_t;
	scn_node_t	*pn;
	scn_obj_t	*po;
	scn_trgt_t	*pt;
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
	phk_t = hash_keys(psg->targets);

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
		for (j = 0 ; j < da_usize(pn->l_deps) ; j++) {
			fprintf(fp, "%s ", (char *) da_idx(pn->l_deps, j));
		}
		fprintf(fp, "%s", pn->fname);


		fprintf(fp, MKF_TWICE_JUMP);
	}

	/* generate target deps */
	fprintf(fp, "\n# target dependency lists\n");
	for(i = 0 ; i < phk_t->nkey ; i++) {
		pstr = phk_t->keys[i];
		pt = hash_get(psg->targets, pstr);

		/* target label */
		str_to_upper(buf, sizeof(buf), pstr);
		fprintf(fp, MKF_TARGET_OBJS, buf);

		/* XXX TODO append objects */
		for (j = 0 ; j < da_usize(pt->deps) ; j++) {
			fprintf(fp, "%s ", (char *) da_idx(pt->deps, j));
		}

		fprintf(fp, MKF_TWICE_JUMP);
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

	/* generate targets */
	fprintf(fp, "\n# targets\n");
	for(i = 0 ; i < phk_t->nkey ; i++) {
		pstr = phk_t->keys[i];

		/* build target */
		fprintf(fp, MKF_TARGET_LABL, pstr, pstr, pstr, pstr);

		/* clean target */
		fprintf(fp, MKF_TARGET_CLN, pstr, pstr, pstr);
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
	debugf("check '%s' extension with file '%s'", file_ext[i].ext, fname);
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


/*************
 parse_file()

 DESCR
	XXX

 IN
	XXX

 OUT
	XXX

 NOTE: XXX XXX
	This function is actually C file oriented and should be improved to
	call the appropriate parser of the file type (parse_c_file,
	parse_asm_file, ...). Those specific parser should also be more
	efficient than running regex on lines ...
************************************************************************/

bool parse_file(htable *pht, scn_node_t *pnode, char *fname) {
	FILE	*fp;
	char		 line[TMP_BUF_LEN],
			 buf[MAXPATHLEN],
			 dir[MAXPATHLEN],
			*pc;

	/* open file */
	fp = fopen(fname, "r");
	if (fp == NULL) {
		errorf("cannot open '%s' : %s.", fname, strerror(errno));
		return(false);
	}

	/* init directory and file names */
	strlcpy(buf, fname, sizeof(buf));
	strlcpy(dir, dirname(buf), sizeof(dir));

	/* gathering local includes */
	while (get_line(fp, line, sizeof(line)) == true) {
		/* seek system include */
		pc = regex_check("^#include[[:blank:]]+<([^>]+)>", line);
		if (pc != NULL) {
#ifdef PMKSCAN_DEBUG
			debugf("found system include '%s'", pc);
#endif
			/* add include in depedencies */
			if (da_push(pnode->s_deps, strdup(pc)) == false) {
				errorf("unable to add '%s' in sys deps", pc);
				return(false);
			}
		}

		/* seek local include */
		pc = regex_check("^#include[[:blank:]]+\"([^\"]+)\"", line);
		if (pc != NULL) {
#ifdef PMKSCAN_DEBUG
			debugf("found local include '%s' in file '%s'", pc, fname);
#endif
			/* add include in depedencies */
			if (da_push(pnode->l_deps, strdup(pc)) == false) {
				errorf("unable to add '%s' in local deps", pc);
				return(false);
			}

			/* scan local include */
			snprintf(buf, sizeof(buf), "%s/%s", dir, pc);
			if (scan_node_file(pht, buf, true) == false) {
				errorf("failed to scan file '%s'.", pc);
				return(false);
			}
		}

		/* check for function */
		pc = regex_check("([[:alnum:]_]+)[[:blank:]]*\\(", line);
		if (pc != NULL) {
#ifdef PMKSCAN_DEBUG
			debugf("found function '%s'", pc);
#endif

			/* add function in list */
			if (da_push(pnode->f_deps, strdup(pc)) == false) {
				errorf("unable to add '%s' in function deps", pc);
				return(false);
			}

			/* check for main procedure */
			if (strncmp(pc, PSC_MAIN_C, strlen(pc)) == 0) {
#ifdef PMKSCAN_DEBUG
				debugf("found main procedure '%s' in '%s'", pc, fname);
#endif
				pnode->mainproc = true;
			}
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

bool scan_node_file(htable *pht, char *fname, bool isdep) {
	ftype_t		 ft;
	scn_node_t	*pnode;

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
	pnode = hash_get(pht, fname);
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

		if (parse_file(pht, pnode, fname) == false) {
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
	if (hash_add(pht, fname, pnode) == HASH_ADD_FAIL) {
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

bool scan_dir(htable *nodes, char *dir, bool recursive) {
	DIR		*pd;
	struct dirent	*pde;
	char		 buf[MAXPATHLEN],
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
				scan_dir(nodes, buf, recursive);

				/* ... and then display end of recurse */
				printf("]");
			}

			/* go to next entry */
			continue;
		}

		if (scan_node_file(nodes, buf, false) == false) {
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
	int		 chr;
	prsdata		*pdata = NULL; /* XXX into global struct ? */
	scandata	 sd;
	scn_glob_t	*psg;

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
	scan_dir(psg->nodes, buf, recursive);
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
