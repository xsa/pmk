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
#include "premake.h"


/*#define PKGCFG_DEBUG	1*/

/*
 * env PKG_CONFIG_PATH => list of colon separated path
 * env PKG_CONFIG_LIBDIR => replace default location (aka prefix/lib/pkgconfig)
 */

#define PKGCFG_HTABLE_SIZE 32

#define PKGCFG_KW_NAME	0
#define PKGCFG_KW_DESCR	1
#define PKGCFG_KW_VERS	2
#define PKGCFG_KW_REQS	3
#define PKGCFG_KW_LIBS	4
#define PKGCFG_KW_CFLGS	5
#define PKGCFG_KW_CFLCT	6

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

/*
	XXX
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
	XXX
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
	XXX
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
	XXX
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
	scan given directory for pc files
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
	parse keyword and set data
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
*/

bool pkg_recurse(pkgdata *ppd, char *reqs) {
	char		*mod,
			*pcf;
	dynary		*pda;
	pkgcell		*ppc;
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
			/* get pc file name */
			pcf = hash_get(ppd->files, mod);

			/* parse pc file */
			ppc = parse_pc_file(pcf);

			/* store pkgcell in hash */
			hash_update(ppd->cells, mod, ppc); /* XXX check */
#ifdef PKGCFG_DEBUG
debugf("adding pkgcell for '%s'", mod);
#endif

			/* add module in list */
			da_push(ppd->mods, strdup(mod));

#ifdef PKGCFG_DEBUG
debugf("pkgcell requires = '%s'", ppc->requires);
#endif
			if (ppc->requires != NULL) {
				pkg_recurse(ppd, ppc->requires); /* XXX check */
			}
		}
	}

	da_destroy(pda);

	return(true);
}

/*
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

	if (*ostr != CHAR_EOS) {
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

		snprintf(buf, s, "%s %s", ostr, astr);

		free(ostr);

	} else {
		buf = strdup(astr);
	}

	return(buf);
}

/*
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
*/



