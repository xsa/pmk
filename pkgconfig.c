/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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


#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "common.h"
#include "hash.h"
#include "parse.h"
#include "pkgconfig.h"
#include "pmk.h"
#include "premake.h"


/*#define PKGCFG_DEBUG	1*/

/*
 * env PKG_CONFIG_PATH => list of colon separated path
 * env PKG_CONFIG_LIBDIR => replace default location (aka prefix/lib/pkgconfig)
 */

#define PKGCFG_HTABLE_SIZE	32
#define PKGCFG_TOK_ADDCT	1

#define PKGCFG_KW_NAME	0
#define PKGCFG_KW_DESCR	1
#define PKGCFG_KW_VERS	2
#define PKGCFG_KW_REQS	3
#define PKGCFG_KW_LIBS	4
#define PKGCFG_KW_CFLGS	5
#define PKGCFG_KW_CFLCT	6

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

/* config tools data file keyword */
prskw	kw_pmkcfgtool[] = {
	{"ADD_CFGTOOL", PKGCFG_TOK_ADDCT, PRS_KW_CELL}
};
int	nbkwct = sizeof(kw_pmkcfgtool) / sizeof(prskw);


/*
	initialize pkgcell structure
*/

pkgcell *pkgcell_init(void) {
	pkgcell *ppc;

	ppc = (pkgcell *) malloc(sizeof(pkgcell));
	if (ppc == NULL) {
		errorf("cannot initialize pkgcell.");
		return(NULL);
	}

	ppc->variables = hash_init(PKGCFG_HTABLE_SIZE);
	if (ppc->variables == NULL) {
		/*da_destroy(ppc->cflags);*/
		/*da_destroy(ppc->libs);*/
		free(ppc);
		errorf("cannot initialize pkgcell hash table.");
		return(NULL);
	}

	return(ppc);
}

/*
	free pkgcell structure
*/

void pkgcell_destroy(pkgcell *ppc) {
	free(ppc->name);
	free(ppc->descr);
	free(ppc->version);
	free(ppc->requires);
	da_destroy(ppc->cflags);
	da_destroy(ppc->libs);
	hash_destroy(ppc->variables);
}

/*
	initialize pkgdata structure
*/

pkgdata *pkgdata_init(void) {
	pkgdata	*ppd;

	ppd = (pkgdata *) malloc(sizeof(pkgdata));
	if (ppd == NULL)
		return(NULL);

	/* init pc files hash table */
	ppd->files = hash_init(PKGCFG_HT_SIZE);
	if (ppd->files == NULL) {
		free(ppd);
		return(NULL);
	}

	/* init package cells hash table */
	ppd->cells = hash_init_adv(PKGCFG_HT_SIZE, NULL,
		(void (*)(void *))pkgcell_destroy, NULL);
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

/*
	free pkgcell structure
*/

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

/*
	free cfgtcell structure
*/

void cfgtcell_destroy(cfgtcell *pcc) {
#ifdef PKGCFG_DEBUG
debugf("free cfgtcell '%s'", pcc->name);
#endif
	free(pcc->name);
	free(pcc->binary);
	if (pcc->version != NULL)
		free(pcc->version);
	if (pcc->module != NULL)
		free(pcc->module);
	if (pcc->cflags != NULL)
		free(pcc->cflags);
	if (pcc->libs != NULL)
		free(pcc->libs);
	free(pcc);
}

/*
	initialize cfgtdata structure
*/

cfgtdata *cfgtdata_init(void) {
	cfgtdata	*pcd;

	/* initialise configt tool data structure */
	pcd = (cfgtdata *) malloc(sizeof(cfgtdata));
	if (pcd == NULL) {
#ifdef PKGCFG_DEBUG
		debugf("cannot initialize config tool data");
#endif
		return(NULL);
	}

	/* initialize config tool hash table */
	pcd->by_mod = hash_init_adv(CFGTOOL_HT_SIZE, NULL,
			(void (*)(void *)) cfgtcell_destroy, NULL);
	if (pcd->by_mod == NULL) {
		free(pcd);
#ifdef PKGCFG_DEBUG
		debugf("cannot initialize config tool hash table");
#endif
		return(NULL);
	}

	/* initialize config tool hash table */
	pcd->by_bin = hash_init(CFGTOOL_HT_SIZE);
	if (pcd->by_bin == NULL) {
		hash_destroy(pcd->by_mod);
		free(pcd);
#ifdef PKGCFG_DEBUG
		debugf("cannot initialize config tool hash table");
#endif
		return(NULL);
	}

	return(pcd);
}

/*
	free cfgtdata structure
*/

void cfgtdata_destroy(cfgtdata *pcd) {
	hash_destroy(pcd->by_mod);
	hash_destroy(pcd->by_bin);
}

/*
	scan given directory for pc files

	dir : directory to scan
	ppd : pkgdata structure

	return : boolean
*/

bool scan_dir(char *dir, pkgdata *ppd) {
	struct dirent	*pde;
	DIR		*pdir;
	char		*pstr,
			 buf[MAXPATHLEN],
			 fpath[MAXPATHLEN];
	size_t		 l;

	/* open directory */
	pdir = opendir(dir);
	if (pdir == NULL) {
		errorf("cannot open '%s' directory : %s", dir, strerror(errno));
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
				strlcpy(buf, pstr, sizeof(buf));
				buf[l-3] = CHAR_EOS;

				/* build full path of pc file */
				strlcpy(fpath, dir, sizeof(fpath));
				strlcat(fpath, "/", sizeof(fpath));
				strlcat(fpath, pstr, sizeof(fpath));

				hash_update_dup(ppd->files, buf, fpath);
				/* XXX should use had_add_dup */
				/* and detect if the package has already been detected */

#ifdef PKGCFG_DEBUG
debugf("add module '%s' with file '%s'", buf, pstr);
#endif
			}
		}
	} while (pde != NULL);

	return(true);
}

/*
	collect packages data

	pkglibdir : default packages data directory
	ppd : pkgdata structure

	return : boolean
*/

bool pkg_collect(char *pkglibdir, pkgdata *ppd) {
	char		*pstr;
	dynary		*pda;
	unsigned int	 i;

	pstr = getenv(PMKCFG_ENV_PATH);
	if (pstr != NULL) {
		/* also scan path provided in env variable */
		pda = str_to_dynary(pstr, PKGCFG_CHAR_PATH_SEP);
		
		for (i = 0 ; i < da_usize(pda) ; i++) {
			pstr = da_idx(pda, i);
			if (scan_dir(pstr, ppd) == false) {
				da_destroy(pda);
				return(false);
			}
		}

		da_destroy(pda);
	}

	pstr = getenv(PMKCFG_ENV_LIBDIR);
	if (pstr == NULL) {
		/* scan default pkgconfig lib directory */
		if (scan_dir(pkglibdir, ppd) == false)
			return(false);
	} else {
		/* scan overrided pkgconfig libdir */
		if (scan_dir(pstr, ppd) == false)
			return(false);
	}
	
	return(true);
}


/*
	parse keyword and set data

	ppc : pkgcell structure
	kword : keyword
	value : value to assign

	return : boolean
*/

bool parse_keyword(pkgcell *ppc, char *kword, char *value) {
	int	i;

	for (i = 0 ; i < nb_pkgkw ; i ++) {
		if (strncmp(kword, kw_pkg[i].kw_name, sizeof(kword)) == 0) {
			/* assgin keyword */
			switch (kw_pkg[i].kw_id) {
				case PKGCFG_KW_NAME :
					ppc->name = strdup(value);
					break;

				case PKGCFG_KW_DESCR :
					ppc->descr = strdup(value);
					break;

				case PKGCFG_KW_VERS :
					ppc->version = strdup(value);
					break;

				case PKGCFG_KW_REQS :
					ppc->requires = strdup(value);
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

/*
	process string to substitute variables with their values

	pstr : string to process
	pht : hash table where variables are stored

	return : new string or NULL
*/

char *process_variables(char *pstr, htable *pht) {
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
					pstr = parse_identifier(pstr, var, size);
					if (pstr == NULL) {
/*						debugf("parse_idtf returned null."); XXX */
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
/*						debugf("overflow."); XXX */
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
/*		debugf("overflow."); XXX */
		return(NULL);
	}

	*pbuf = CHAR_EOS;
	return(strdup(buf));
}

/*
	parse pc file

	pcfile : file to parse

	return : pkgcell structure or NULL
*/

pkgcell *parse_pc_file(char *pcfile) {
	FILE	*fp;
	char	*pstr,
		*pps,
		 buf[TMP_BUF_LEN],
		 line[TMP_BUF_LEN];
	pkgcell	*ppc;

	fp = fopen(pcfile, "r");
	if (fp == NULL) {
		errorf("cannot open '%s' : %s.", pcfile, strerror(errno));
		return(NULL);
	}

	ppc = (pkgcell *) malloc(sizeof(pkgcell));
	if (ppc == NULL) {
		errorf("cannot initialize pkgcell.");
		return(NULL);
	}

	ppc->variables = hash_init(PKGCFG_HTABLE_SIZE);
	if (ppc->variables == NULL) {
		errorf("cannot initialize pkgcell hash table.");
		return(NULL);
	}

	/* main parsing */
	while (get_line(fp, line, sizeof(line)) == true) {
		/* collect identifier */
		pstr = parse_identifier(line, buf, sizeof(buf));

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
				parse_keyword(ppc, buf, pps); /* XXX check ? */
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
				hash_update_dup(ppc->variables, buf, pps); /* XXX check, add only ? */

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

/*
	insert a package cell and return a pointer on it

	mod : module name to process
	ppd : packages data structure

	return : pkgcell structure or NULL
*/

pkgcell *pkg_cell_add(pkgdata *ppd, char *mod) {
	char		*pcf;
	pkgcell		*ppc;

	/* get pc file name */
	pcf = hash_get(ppd->files, mod);

	/* parse pc file */
	ppc = parse_pc_file(pcf);

	/* store pkgcell in hash */
	if (hash_update(ppd->cells, mod, ppc) == HASH_ADD_FAIL) {
		return(NULL);
#ifdef PKGCFG_DEBUG
	} else {
debugf("adding pkgcell for '%s'", mod);
#endif
	}

	/* add module in list */
	da_push(ppd->mods, strdup(mod));

#ifdef PKGCFG_DEBUG
debugf("pkgcell requires = '%s'", ppc->requires);
#endif
	if (ppc->requires != NULL) {
		pkg_recurse(ppd, ppc->requires); /* XXX check */
	}

	return(ppc);
}

/*
	recurse packages

	ppd : packages data structure
	reqs : requires string

	return : boolean
*/

bool pkg_recurse(pkgdata *ppd, char *reqs) {
	char		*mod;
	dynary		*pda;
	unsigned int	 i;

#ifdef PKGCFG_DEBUG
debugf("recursing '%s'", reqs);
#endif

	pda = str_to_dynary_adv(reqs, " ,");
	if (pda == NULL) {
		/* XXX errof */
		return(false);
	}

	for (i = 0 ; i < da_usize(pda) ; i++) {
		/* get module name */
		mod = da_idx(pda, i);

		/*  check if module has been already processed */
		if (hash_get(ppd->cells, mod) == NULL) {
			pkg_cell_add(ppd, mod); /* XXX check */
		}
	}

	da_destroy(pda);

	return(true);
}

/*
	append module name if it does not already exists

	ostr : modules list string
	astr : module to append

	return : new list of modules or NULL
*/

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

		snprintf(buf, s, "%s %s", ostr, astr);

		free(ostr);

	} else {
		buf = strdup(astr);
	}

	return(buf);
}

/*
	get recursed list of cflags

	ppd : packages data structure

	return : cflags string
*/

char *pkg_get_cflags(pkgdata *ppd) {
	char		*cflags = "";
	dynary		*pda;
	pkgcell		*ppc;
	unsigned int	 i,
			 j;

	pda = ppd->mods;

	for (i = 0 ; i < da_usize(pda) ; i++) {
		/* get module name */
		ppc = hash_get(ppd->cells, da_idx(pda, i));

		/* append each element but avoid duplicate*/
		for (j = 0 ; j < da_usize(ppc->cflags) ; j++) {
			cflags = pkg_single_append(cflags, da_idx(ppc->cflags, j));
		}
	}

	return(cflags);
}

/*
	get recursed list of libs

	ppd : packages data structure

	return : libs string
*/

char *pkg_get_libs(pkgdata *ppd) {
	char		*libs = "";
	dynary		*pda;
	pkgcell		*ppc;
	unsigned int	 i,
			 j;

	pda = ppd->mods;

	for (i = 0 ; i < da_usize(pda) ; i++) {
		/* get module name */
		ppc = hash_get(ppd->cells, da_idx(pda, i));

		/* append each element but avoid duplicate*/
		for (j = 0 ; j < da_usize(ppc->libs) ; j++) {
			libs = pkg_single_append(libs, da_idx(ppc->libs, j));
		}
	}

	return(libs);
}

/*
	check if the given module exists

	mod : module to check

	return : boolean
*/

bool pkg_mod_exists(pkgdata *ppd, char *mod) {
	if (hash_get(ppd->files, mod) == NULL) {
		return(false);
	} else {
		return(true);
	}
}

/*
	add a new config toom cell

	pcd : config tool data structure
	pht : parsed data

	return : boolean
*/

bool add_cfgtool(cfgtdata *pcd, htable *pht) {
	cfgtcell	*pcell;
	char		*pstr;

	pcell = (cfgtcell *) malloc(sizeof(cfgtcell));
	if (pcell == NULL)
		return(false);

	pstr = po_get_str(hash_get(pht, "NAME"));
	if (pstr == NULL) {
		free(pcell);
		return(false);
	} else {
		pcell->name = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, "BINARY"));
	if (pstr == NULL) {
		free(pcell);
		return(false);
	} else {
		pcell->binary = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, "VERSION"));
	if (pstr != NULL) {
		pcell->version = strdup(pstr);
	} else {
		pcell->version = NULL;
	}

	pstr = po_get_str(hash_get(pht, "MODULE"));
	if (pstr != NULL) {
		pcell->module = strdup(pstr);
	} else {
		pcell->module = NULL;
	}

	pstr = po_get_str(hash_get(pht, "CFLAGS"));
	if (pstr != NULL) {
		pcell->cflags = strdup(pstr);
	} else {
		pcell->cflags = NULL;
	}

	pstr = po_get_str(hash_get(pht, "LIBS"));
	if (pstr != NULL) {
		pcell->libs = strdup(pstr);
	} else {
		pcell->libs = NULL;
	}

	hash_update(pcd->by_mod, pcell->name, pcell); /* no need to strdup */ /* XXX check */
#ifdef PKGCFG_DEBUG
debugf("added cfgtcell '%s'", pcell->name);
#endif
	hash_update_dup(pcd->by_bin, pcell->binary, pcell->name); /* no need to strdup */ /* XXX check */
#ifdef PKGCFG_DEBUG
debugf("added cfgtcell '%s'", pcell->binary);
#endif

	return(true);
}

/*
	parse data from PMKCFG_DATA file

	return : compiler data structure or NULL
*/

cfgtdata *parse_cfgt_file(void) {
	FILE		*fd;
	bool		 rval;
	cfgtdata	*pcd;
	prscell		*pcell;
	prsdata		*pdata;

	/* initialize parsing structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot intialize prsdata.");
		return(NULL);
	}

	/* initialise configt tool data structure */
	pcd = cfgtdata_init();
	if (pcd == NULL) {
		prsdata_destroy(pdata);
		debugf("cannot initialize config tool data");
		return(NULL);
	}

	fd = fopen(PMKCFG_DATA, "r");
	if (fd == NULL) {
		hash_destroy(pcd->by_mod);
		hash_destroy(pcd->by_bin);
		free(pcd);
		prsdata_destroy(pdata);
		errorf("cannot open '%s'", PMKCFG_DATA);
		return(NULL);
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fd, pdata, kw_pmkcfgtool, nbkwct);
	fclose(fd);

	if (rval == true) {
		pcell = pdata->tree->first;
		while (pcell != NULL) {
			switch(pcell->token) {
				case PKGCFG_TOK_ADDCT :
					add_cfgtool(pcd, pcell->data);
					break;
			
				default :
					errorf("parsing of data file failed.");
					hash_destroy(pcd->by_mod);
					hash_destroy(pcd->by_bin);
					free(pcd);
					prsdata_destroy(pdata);
					return(NULL);
					break;
			}

			pcell = pcell->next;
		}
	} else {
		errorf("parsing of data file failed.");
		hash_destroy(pcd->by_mod);
		hash_destroy(pcd->by_bin);
		free(pcd);
		prsdata_destroy(pdata);
		return(NULL);
	}

	/* parsing data no longer needed */
	prsdata_destroy(pdata);

	return(pcd);
}

/*
	get binary name from a given module name

	pgd : global data structure
	mod : module name
	buf : buffer to store binary name
	sb : buffer size

	return : boolean
*/

bool cfgtcell_get_binary(pmkdata *pgd, char *mod, char *buf, size_t sb) {
	cfgtcell	*pcc;
	cfgtdata	*pcd;

	/* init config tool data if it does not exist */
	if (pgd->cfgt == NULL) {
		pgd->cfgt = parse_cfgt_file();
	}

	pcd = pgd->cfgt;

	pcc = hash_get(pcd->by_mod, mod);
	if (pcc == NULL) {
		/* not found */
		return(false);
	} else {
		/* found, copy */
		strlcpy(buf, pcc->binary, sb);
		return(true);
	}
}

/*
	get cell relative to a given config tool filename

	pgd : global data structure
	binary : binary name

	return : cfgtcell structure or NULL
*/

cfgtcell *cfgtcell_get_cell(pmkdata *pgd, char *binary) {
	cfgtdata	*pcd;
	char		*mod;

	/* init config tool data if it does not exist */
	if (pgd->cfgt == NULL) {
		pgd->cfgt = parse_cfgt_file();
	}

	pcd = pgd->cfgt;

	mod = hash_get(pcd->by_bin, binary);
	if (mod == NULL) {
		/* not found */
		return(NULL);
	} else {
		/* found, copy */
		return(hash_get(pcd->by_mod, mod));
	}
}

