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

#include "compat/pmk_stdbool.h"
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
	scan given directory for pc files
*/

bool scan_dir(char *dir, htable *pht) {
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

				hash_update_dup(pht, buf, fpath);
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

				case PKGCFG_KW_LIBS :
					ppc->libs = strdup(value);
					break;

				case PKGCFG_KW_CFLGS :
					ppc->cflags = strdup(value);
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
	parse pc file
*/

pkgcell *parse_pc_file(char *pcfile) {
	FILE	*fp;
	char	*pstr,
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
#ifdef PKGCFG_DEBUG
debugf("keyword = '%s', value = '%s'", buf, pstr);
#endif
				/* set variable with parsed data */
				parse_keyword(ppc, buf, pstr);
				break;

			case '=' :
				/* variable */
				pstr++;
				pstr = skip_blank(pstr);

#ifdef PKGCFG_DEBUG
debugf("variable = '%s', value = '%s'", buf, pstr);
#endif
				/* store variable in hash */
				hash_update_dup(ppc->variables, buf, pstr); /* XXX check, add only ? */
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

void pkg_collect(void) {
	
}

/*
*/

void pkg_get_cflags(void) {
}

/*
*/

void pkg_get_libs(void) {
}

/*
*/



