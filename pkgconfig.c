/* $Id$ */

/*
 * Copyright (c) 2003-2006 Damien Couderc
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
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "common.h"
#include "hash.h"
#include "hash_tools.h"
#include "parse.h"
#include "pkgconfig.h"
#include "premake.h"


/*#define PKGCFG_DEBUG	1*/

/*
 * env PKG_CONFIG_PATH => list of colon separated path
 * env PKG_CONFIG_LIBDIR => replace default location (aka prefix/lib/pkgconfig)
 */

/**********
 constants
************************************************************************/

#define PKGCFG_HTABLE_SIZE	32
#define PKGCFG_TOK_ADDCT	1

#define PKGCFG_KW_NAME	0
#define PKGCFG_KW_DESCR	1
#define PKGCFG_KW_VERS	2
#define PKGCFG_KW_REQS	3
#define PKGCFG_KW_LIBS	4
#define PKGCFG_KW_CFLGS	5
#define PKGCFG_KW_CFLCT	6

/**********
 variables
************************************************************************/

/* pc file keywords */
pkgkw	kw_pkg[] = {
		{"Name",	PKGCFG_KW_NAME},
		{"Description",	PKGCFG_KW_DESCR},
		{"Version",	PKGCFG_KW_VERS},
		{"Requires",	PKGCFG_KW_REQS},
		{"Libs",	PKGCFG_KW_LIBS},
		{"Cflags",	PKGCFG_KW_CFLGS},
		{"CFlags",	PKGCFG_KW_CFLGS},
		{"Conflicts",	PKGCFG_KW_CFLCT}
};
size_t	nb_pkgkw = sizeof(kw_pkg) / sizeof(pkgkw);


/**********
 functions
************************************************************************/

/***************
 pkgcell_init()

 DESCR
	initialize pkgcell structure

 IN
	NONE

 OUT
	pkgcell structure or NULL
************************************************************************/

pkgcell *pkgcell_init(void) {
	pkgcell *ppc;

	ppc = (pkgcell *) malloc(sizeof(pkgcell));
	if (ppc == NULL) {
		errorf("cannot initialize pkgcell.");
		return(NULL);
	}

	ppc->variables = hash_create_simple(PKGCFG_HTABLE_SIZE);
	if (ppc->variables == NULL) {
		free(ppc);
		errorf("cannot initialize pkgcell hash table.");
		return(NULL);
	}

	ppc->cflags = NULL;
	ppc->libs = NULL;
	ppc->requires = NULL;

	return(ppc);
}


/******************
 pkgcell_destroy()

 DESCR
	free pkgcell structure

 IN
	ppc : pkgcell structure

 OUT
	NONE
************************************************************************/

void pkgcell_destroy(pkgcell *ppc) {
	if (ppc->name != NULL)
		free(ppc->name);

	if (ppc->descr != NULL)
		free(ppc->descr);

	if (ppc->version != NULL)
		free(ppc->version);

	if (ppc->requires != NULL)
		free(ppc->requires);

	da_destroy(ppc->cflags);
	da_destroy(ppc->libs);
	hash_destroy(ppc->variables);

	free(ppc);
}


/***************
 pkgdata_init()

 DESCR
	initialize pkgdata structure

 IN
	NONE

 OUT
	pkgdata structure or NULL
************************************************************************/

pkgdata *pkgdata_init(void) {
	pkgdata	*ppd;

	ppd = (pkgdata *) malloc(sizeof(pkgdata));
	if (ppd == NULL)
		return(NULL);

	/* init pc files hash table */
	ppd->files = hash_create_simple(PKGCFG_HT_SIZE);
	if (ppd->files == NULL) {
		free(ppd);
		return(NULL);
	}

	/* init package cells hash table */
	ppd->cells = hash_create(PKGCFG_HT_SIZE, false, NULL, NULL, (void (*)(void *))pkgcell_destroy);
	if (ppd->cells == NULL) {
		hash_destroy(ppd->files);
		free(ppd);
		return(NULL);
	}

	/* init recursed modules hash table */
	ppd->mods = da_init();
	if (ppd->mods == NULL) {
		hash_destroy(ppd->files);
		hash_destroy(ppd->cells);
		free(ppd);
		return(NULL);
	}

	return(ppd);
}


/******************
 pkgdata_destroy()

 DESCR
	free pkgdata structure

 IN
	ppd : pkgdata structure

 OUT
	NONE
************************************************************************/

void pkgdata_destroy(pkgdata *ppd) {
#ifdef PKGCFG_DEBUG
	debugf("clean pc files hash");
#endif
	hash_destroy(ppd->files);

#ifdef PKGCFG_DEBUG
	debugf("clean cells hash");
#endif
	hash_destroy(ppd->cells);

#ifdef PKGCFG_DEBUG
	debugf("clean modules list");
#endif
	da_destroy(ppd->mods);

#ifdef PKGCFG_DEBUG
	debugf("clean structure");
#endif
	free(ppd);
}


/*************
 skip_blank()

 DESCR
	skip blank character(s)

 IN
	pstr: current parsing cursor

 OUT
	new parsing cursor
***********************************************************************/

char *skip_blank(char *pstr) {
	while (isblank(*pstr) != 0) {
		pstr++;
	}
	return(pstr);
}


/***********
 scan_dir()

 DESCR
	scan given directory for pc files

 IN
	dir : directory to scan
	ppd : pkgdata structure

 OUT
	boolean
************************************************************************/

bool scan_dir(char *dir, pkgdata *ppd) {
	struct dirent	*pde;
	DIR				*pdir;
	char			*pstr,
					 buf[MAXPATHLEN],
					 fpath[MAXPATHLEN];
	size_t			 l;

	/* open directory */
	pdir = opendir(dir);
	if (pdir == NULL) {
		errorf("cannot open '%s' directory : %s.",
			dir, strerror(errno));
		return(false);
	}

	/* parse directory */
	do {
		pde = readdir(pdir);
		if (pde != NULL) {
			/* got an entry */
			pstr = pde->d_name;
			l = strlen(pstr);
			/* check if it's a .pc file */
			if ((pstr[l-3] == '.') && (pstr[l-2] == 'p')
						&& (pstr[l-1] == 'c')) {

				/* build pkg name */
				if (strlcpy_b(buf, pstr,
						sizeof(buf)) == false) {
					/* XXX err msg ? */
					closedir(pdir);
					return(false);
				}
				buf[l-3] = CHAR_EOS;

				/* build full path of pc file */
				strlcpy(fpath, dir, sizeof(fpath)); /* no check */
				strlcat(fpath, "/", sizeof(fpath)); /* no check */
				if (strlcat_b(fpath, pstr, sizeof(fpath)) == false) {
					/* XXX err msg ? */
					closedir(pdir);
					return(false);
				}

				hash_update_dup(ppd->files, buf, fpath);
				/* XXX detect if the package has already been detected ? */

#ifdef PKGCFG_DEBUG
				debugf("add module '%s' with file '%s'", buf, pstr);
#endif
			}
		}
	} while (pde != NULL);

	closedir(pdir);

	return(true);
}


/**************
 pkg_collect()

 DESCR
	collect packages data

 IN
	pkglibdir : default packages data directory
	ppd : pkgdata structure

 OUT
	return : boolean
************************************************************************/

bool pkg_collect(char *pkglibdir, pkgdata *ppd) {
	char			*pstr;
	dynary			*ppath,
					*ppcpath;
	unsigned int	 i;

	pstr = getenv(PMKCFG_ENV_PATH);
	if (pstr != NULL) {
		/* also scan path provided in env variable */
		ppath = str_to_dynary(pstr, PKGCFG_CHAR_PATH_SEP);

		for (i = 0 ; i < da_usize(ppath) ; i++) {
			pstr = da_idx(ppath, i);
			if (scan_dir(pstr, ppd) == false) {
				da_destroy(ppath);
				return(false);
			}
		}

		da_destroy(ppath);
	}

	/* check if pkg-config library directory has been set in env variable */
	pstr = getenv(PMKCFG_ENV_LIBDIR);
	if (pstr == NULL) {
		/* env variable is not set, getting pmk.conf variable */
		ppcpath = str_to_dynary(pkglibdir, PKGCFG_CHAR_PATH_SEP);
	} else {
		/* else use env variable */
		ppcpath = str_to_dynary(pstr, PKGCFG_CHAR_PATH_SEP);
	}

	/* scan all path for pc files */
	for (i = 0 ; i < da_usize(ppcpath) ; i++) {
		/* get directory */
		pstr = da_idx(ppcpath, i);

		/* scan directory */
		if (scan_dir(pstr, ppd) == false) {
			return(false);
		}
	}

	return(true);
}


/****************
 parse_keyword()

 DESCR
	parse keyword and set data

 IN
	ppc : pkgcell structure
	kword : keyword
	value : value to assign

 OUT
	boolean
************************************************************************/

bool parse_keyword(pkgcell *ppc, char *kword, char *value) {
	unsigned int	i;

	for (i = 0 ; i < nb_pkgkw ; i ++) {
		if (strncmp(kword, kw_pkg[i].kw_name, sizeof(kword)) == 0) {
			/* assgin keyword */
			switch (kw_pkg[i].kw_id) {
				case PKGCFG_KW_NAME :
					ppc->name = strdup(value);
					if (ppc->name == NULL) {
						errorf(ERRMSG_MEM);
						return(false);
					}
					break;

				case PKGCFG_KW_DESCR :
					ppc->descr = strdup(value);
					if (ppc->descr == NULL) {
						errorf(ERRMSG_MEM);
						return(false);
					}
					break;

				case PKGCFG_KW_VERS :
					ppc->version = strdup(value);
					if (ppc->version == NULL) {
						errorf(ERRMSG_MEM);
						return(false);
					}
					break;

				case PKGCFG_KW_REQS :
					if (*value != CHAR_EOS) {
						ppc->requires = strdup(value);
						if (ppc->requires == NULL) {
							errorf(ERRMSG_MEM);
							return(false);
						}
					}
					break;

				case PKGCFG_KW_CFLGS :
					ppc->cflags = str_to_dynary(value, ' ');
					break;

				case PKGCFG_KW_LIBS :
					ppc->libs = str_to_dynary(value, ' ');
					break;

				case PKGCFG_KW_CFLCT :
					/* unused */
					break;

				default :
					break;
			}

			return(true);
		}
	}

	return(true);
}


/********************
 process_variables()

 DESCR
	process string to substitute variables with their values

 IN
	pstr : string to process
	pht : hash table where variables are stored

 OUT
	new string or NULL
************************************************************************/

char *process_variables(char *pstr, htable_t *pht) {
	bool	 bs = false;
	char	 buf[OPT_VALUE_LEN],
			 var[OPT_NAME_LEN],
			*pvar,
			*pbuf;
	size_t	 size;

	size = sizeof(buf);
	pbuf = buf;

	while ((*pstr != CHAR_EOS) && (size > 0)) {
		switch(*pstr) {
			case '\\' :
				bs = true;
				pstr++;
				break;

			case '$' :
				if (bs == false) {
					/* found variable */
					pstr++;
					/* skip '{' */
					pstr++;
					pstr = parse_idtf(pstr, var, sizeof(var));
					if (pstr == NULL) {
						/* debugf("parse_idtf returned null."); */
						return(NULL);
					} else {
						/* check if identifier exists */
						pvar = hash_get(pht, var);
						if (pvar != NULL) {
							/* identifier found, append value */
							while ((*pvar != CHAR_EOS) && (size > 0)) {
								*pbuf = *pvar;
								pbuf++;
								pvar++;
								size--;
							}

							/* skip '}' */
							pstr++;
						}
					}
				} else {
					/* copy character */
					*pbuf = *pstr;
					pbuf++;
					pstr++;
					size--;
					bs = false;
				}
				break;

			default :
				if (bs == true) {
					*pbuf = '\\';
					pbuf++;
					pstr++;
					size--;
					if (size == 0) {
						/* debugf("overflow."); */
						return(NULL);
					}
					bs = false;
				}
				/* copy character */
				*pbuf = *pstr;
				pbuf++;
				pstr++;
				size--;
				break;
		}
	}

	if (size == 0) {
		/* debugf("overflow."); */
		return(NULL);
	}

	*pbuf = CHAR_EOS;
	return(strdup(buf));
}


/****************
 parse_pc_file()

 DESCR
	parse pc file

 IN
	pcfile : file to parse

 OUT
	pkgcell structure or NULL
************************************************************************/

pkgcell *parse_pc_file(char *pcfile) {
	FILE	*fp;
	char	*pstr,
			*pps,
			 buf[TMP_BUF_LEN],
			 line[TMP_BUF_LEN];
	pkgcell	*ppc;

	fp = fopen(pcfile, "r");
	if (fp == NULL) {
		errorf("cannot open .pc file '%s' : %s.", pcfile, strerror(errno));
		return(NULL);
	}

	ppc = pkgcell_init();
	if (ppc == NULL) {
		errorf("cannot initialize pkgcell.");
		return(NULL);
	}

	/* main parsing */
	while (get_line(fp, line, sizeof(line)) == true) {
		/* collect identifier */
		pstr = parse_idtf(line, buf, sizeof(buf));

		switch (*pstr) {
			case ':' :
				/* keyword */
				pstr++;
				pstr = skip_blank(pstr);

				/* process variables */
				pps = process_variables(pstr, ppc->variables);
#ifdef PKGCFG_DEBUG
				debugf("keyword = '%s', value = '%s', string = '%s'", buf, pps, pstr);
#endif
				/* set variable with parsed data */
				if (parse_keyword(ppc, buf, pps) == false) {
					errorf("cannot fill pkgcell structure (keyword).");
					pkgcell_destroy(ppc);
					return(NULL);
				}
				break;

			case '=' :
				/* variable */
				pstr++;
				pstr = skip_blank(pstr);

				/* process variables */
				pps = process_variables(pstr, ppc->variables);
#ifdef PKGCFG_DEBUG
				debugf("variable = '%s', value = '%s', string = '%s'", buf, pps, pstr);
#endif
				/* store variable in hash */
				if (hash_update_dup(ppc->variables, buf, pps) == false) {
					errorf("cannot fill pkgcell structure (variables).");
					pkgcell_destroy(ppc);
					return(NULL);
				}

				break;

			default :
				/* unknown or empty line */
#ifdef PKGCFG_DEBUG
				debugf("unknown = '%s'", line);
#endif
				break;
		}
	}

	fclose(fp);

	return(ppc);
}


/***************
 pkg_cell_add()

 DESCR
	insert a package cell and return a pointer on it

 IN
	mod : module name to process
	ppd : packages data structure

 OUT
	pkgcell structure or NULL
************************************************************************/

pkgcell *pkg_cell_add(pkgdata *ppd, char *mod) {
	char		*pcf;
	pkgcell		*ppc;

	/* get pc file name */
	pcf = hash_get(ppd->files, mod);
	if (pcf == NULL) {
		/* config file not found */
		return NULL;
	}

	/* parse pc file */
	ppc = parse_pc_file(pcf);
	if (ppc == NULL) {
		return(NULL);
	}

	/* store pkgcell in hash */
	if (hash_update(ppd->cells, mod, ppc) == false) {
		return(NULL);
#ifdef PKGCFG_DEBUG
	} else {
		debugf("adding pkgcell for '%s'", mod);
#endif
	}

	/* add module in list */
	da_push(ppd->mods, strdup(mod));

	if (ppc->requires != NULL) {
#ifdef PKGCFG_DEBUG
		debugf("pkgcell requires = '%s'", ppc->requires);
#endif
		if (pkg_recurse(ppd, ppc->requires) == false) {
			pkgcell_destroy(ppc);
			return(NULL);
		}
	}

	return(ppc);
}


/**************
 pkg_recurse()

 DESCR
	recurse packages

 IN
	ppd : packages data structure
	reqs : requires string

 OUT
	boolean
************************************************************************/

bool pkg_recurse(pkgdata *ppd, char *reqs) {
	char			*mod;
	dynary			*pda;
	unsigned int	 i;

#ifdef PKGCFG_DEBUG
debugf("recursing '%s'", reqs);
#endif

	pda = str_to_dynary_adv(reqs, " ,");
	if (pda == NULL) {
		errorf("failed to process requirements in pkg_recurse().");
		return(false);
	}

	for (i = 0 ; i < da_usize(pda) ; i++) {
		/* get module name */
		mod = da_idx(pda, i);

		/*  check if module has been already processed */
		if (hash_get(ppd->cells, mod) == NULL) {
			/*
			 * here we don't check the result as dependencies 
			 * can be missing in some cases.
			 */
			pkg_cell_add(ppd, mod); /* no check */
		}
	}

	da_destroy(pda);

	return(true);
}


/********************
 pkg_single_append()

 DESCR
	append module name if it does not already exists

 IN
	ostr : modules list string
	astr : module to append

 OUT
	return : new list of modules or NULL
************************************************************************/

char *pkg_single_append(char *ostr, char *astr) {
	char	*pstr,
			*buf;
	size_t	 s;

#ifdef PKGCFG_DEBUG
	debugf("single_append '%s', '%s'", ostr, astr);
#endif

	if (*astr == CHAR_EOS)
		return(ostr);

	if ((*ostr != CHAR_EOS) && (ostr != NULL)) {
		pstr = strstr(ostr, astr);
		while (pstr != NULL) {
			pstr = pstr + strlen (astr);
			if ((*pstr == ' ') || (*pstr == CHAR_EOS)) {
				/* found existing value */
				return(ostr);
			}
			pstr = strstr(pstr, astr);
		}

		/* 2 is for the separator and the end of file char */
		s = strlen(ostr) + strlen(astr) + 2;
		buf = (char *) malloc(s);
		if (buf == NULL)
			return(NULL);

		if (snprintf_b(buf, s, "%s %s", ostr, astr) == false) {
			return(NULL);
		}

		free(ostr);

	} else {
		buf = strdup(astr);
	}

	return(buf);
}


/*****************
 pkg_get_cflags()

 DESCR
	get recursed list of cflags

 IN
	ppd : packages data structure

 OUT
	cflags string
************************************************************************/

char *pkg_get_cflags(pkgdata *ppd) {
	return(pkg_get_cflags_adv(ppd, PKGCFG_CFLAGS_ALL));
}


/*********************
 pkg_get_cflags_adv()

 DESCR
	get recursed list of cflags with output filtering

 IN
	ppd : packages data structure
	opts : output filtering options

 OUT
	cflags string
************************************************************************/

char *pkg_get_cflags_adv(pkgdata *ppd, unsigned int opts) {
	char			*cflags = "",
					*pstr;
	dynary			*pda;
	pkgcell			*ppc;
	unsigned int	 i,
					 j,
					 o;

	pda = ppd->mods;

	for (i = 0 ; i < da_usize(pda) ; i++) {
		/* get module name */
		ppc = hash_get(ppd->cells, da_idx(pda, i));

		/* append each element but avoid duplicate*/
		for (j = 0 ; j < da_usize(ppc->cflags) ; j++) {
			/* get element */
			pstr = da_idx(ppc->cflags, j);

			/* check type of element */
			if (pstr[0] == '-' && pstr[1] == 'I') {
				/* include path */
				o = PKGCFG_CFLAGS_I;
			} else {
				/* other */
				o = PKGCFG_CFLAGS_o;
			}

			/* append if not filtered */
			if ((opts & o) != 0)
				cflags = pkg_single_append(cflags, pstr);
		}
	}

	return(cflags);
}


/***************
 pkg_get_libs()

 DESCR
	get recursed list of libs

 IN
	ppd : packages data structure

 OUT
	libs string
************************************************************************/

char *pkg_get_libs(pkgdata *ppd) {
	return(pkg_get_libs_adv(ppd, PKGCFG_LIBS_ALL));
}


/*******************
 pkg_get_libs_adv()

 DESCR
	get recursed list of libs with output filtering

 IN
	ppd : packages data structure
	opts : output filtering options

 OUT
	libs string
************************************************************************/

char *pkg_get_libs_adv(pkgdata *ppd, unsigned int opts) {
	char			*libs = "",
					*pstr;
	dynary			*pda;
	pkgcell			*ppc;
	unsigned int	 i,
					 j,
					 o;

	pda = ppd->mods;

	for (i = 0 ; i < da_usize(pda) ; i++) {
		/* get module name */
		ppc = hash_get(ppd->cells, da_idx(pda, i));

		/* append each element but avoid duplicate*/
		for (j = 0 ; j < da_usize(ppc->libs) ; j++) {
			/* get element */
			pstr = da_idx(ppc->libs, j);

			/* check type of element */
			if (pstr[0] == '-') {
				switch (pstr[1]) {
					case 'L':
						/* lib path */
						o = PKGCFG_LIBS_L;
						break;

					case 'l':
						/* lib name */
						o = PKGCFG_LIBS_l;
						break;

					default:
						/* other */
						o = PKGCFG_LIBS_o;
						break;
				}
			} else {
				/* other */
				o = PKGCFG_LIBS_o;
			}

			/* append if not filtered */
			if ((opts & o) != 0)
				libs = pkg_single_append(libs, pstr);
		}
	}

	return(libs);
}


/*****************
 pkg_mod_exists()

 DESCR
	check if the given module exists

 IN
	mod : module to check

 OUT
	boolean
************************************************************************/

bool pkg_mod_exists(pkgdata *ppd, char *mod) {
	if (hash_get(ppd->files, mod) == NULL) {
		return(false);
	} else {
		return(true);
	}
}


/******************
 compare_version()

 DESCR
	compare two version strings

 IN
	vref : reference version
	vcomp : version to check

 OUT
	<0 if vref is greater than vcomp
	=0 if vref is equal to vcomp
	>0 if vref is smaller than vcomp
************************************************************************/

int compare_version(char *vref, char *vcomp) {
	bool		 bexit = false;
	char		*sr,
			*sc;
	dynary		*vr,
			*vc;
	int		 delta,
			 ref,
			 cmp;
	unsigned int	 i = 0;
	unsigned long	 tl;

	/* need to check da_* returns */
	vr = str_to_dynary(vref, VERSION_CHAR_SEP);
	if (vr == NULL) {
		errorf("cannot parse reference version '%s'.", vref);
		return(false);
	}
	vc = str_to_dynary(vcomp, VERSION_CHAR_SEP);
	if (vc == NULL) {
		errorf("cannot parse comparison version '%s'.", vcomp);
		return(false);
	}

	while (bexit == false) {
		/* process reference version */
		sr = da_idx(vr, i);
		if (sr != NULL) {
			/* convert string */
			if (str_to_ulong(sr, 10, &tl) == false) {
				errorf("cannot get numerical value of '%s'.", sr);
				return(false);
			}
			ref = (int) tl;
		} else {
			/* end of version string */
			ref = 0;
			bexit = true;
		}

		/* process compared version */
		sc = da_idx(vc, i);
		if (sc != NULL) {
			if (str_to_ulong(sc, 10, &tl) == false) {
				errorf("cannot get numerical value of '%s'.", sc);
				return(false);
			}
			cmp = (int) tl;
		} else {
			/* end of version string */
			cmp = 0;
			bexit = true;
		}

		/* compare versions */
		delta = cmp - ref;

		if (delta != 0) {
			/* not equal end of comparison */
			bexit = true;
		} else {
			i++;
		}
	}

	/* destroy dynaries */
	da_destroy(vr);
	da_destroy(vc);

	return(delta);
}
