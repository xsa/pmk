/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
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


#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "cfgtool.h"
#include "common.h"
#include "errno.h"
#include "parse.h"
#include "premake.h"

#define CFGT_TOK_ADDCT	1

/* config tools data file keyword */
prskw	kw_pmkcfgtool[] = {
	{"ADD_CFGTOOL", CFGT_TOK_ADDCT, PRS_KW_CELL}
};
int	nbkwct = sizeof(kw_pmkcfgtool) / sizeof(prskw);

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
#ifdef PKGCFG_DEBUG
debugf("destroying module hash table.");
#endif
	if (pcd->by_mod != NULL) {
		hash_destroy(pcd->by_mod);
#ifdef PKGCFG_DEBUG
debugf("destroyed module hash table.");
	} else {
debugf("WARNING : by_mod doesn't exists !!!");
#endif
	}

#ifdef PKGCFG_DEBUG
debugf("destroying binary links hash table.");
#endif
	if (pcd->by_mod != NULL) {
		hash_destroy(pcd->by_bin);
#ifdef PKGCFG_DEBUG
debugf("destroyed binary links hash table.");
	} else {
debugf("WARNING : by_bin doesn't exists !!!");
#endif
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
		free(pcell->name);
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
		errorf("cannot open '%s' : %s.", PMKCFG_DATA, strerror(errno));
		return(NULL);
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fd, pdata, kw_pmkcfgtool, nbkwct);
	fclose(fd);

	if (rval == true) {
		pcell = pdata->tree->first;
		while (pcell != NULL) {
			switch(pcell->token) {
				case CFGT_TOK_ADDCT :
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

bool cfgtcell_get_binary(cfgtdata *pcd, char *mod, char *buf, size_t sb) {
	cfgtcell	*pcc;

	if (pcd == NULL) {
		/* not found */
		return(false);
	}

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

cfgtcell *cfgtcell_get_cell(cfgtdata *pcd, char *binary) {
	char		*mod;

	if (pcd == NULL) {
		/* not found */
		return(false);
	}

	mod = hash_get(pcd->by_bin, binary);
	if (mod == NULL) {
		/* not found */
		return(NULL);
	} else {
		/* found, copy */
		return(hash_get(pcd->by_mod, mod));
	}
}

/*
	XXX
*/

bool ct_get_version(char *ctpath, char *vstr, char *buffer, size_t sbuf) {
	FILE	*rpipe;
	bool	 rval = false;
	char	 cfgcmd[MAXPATHLEN];


	snprintf(cfgcmd, sizeof(cfgcmd), CT_FORMAT_VERSION, ctpath, vstr);

	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* get version line */
		if (get_line(rpipe, buffer, sbuf) == true) {
			rval = true;
		}

		pclose(rpipe);
	}

	return(rval);
}

/*
	XXX
*/

bool ct_get_data(char *ctpath, char *ostr, char *mod, char *buffer, size_t sbuf) {
	FILE	*rpipe;
	bool	 rval = false;
	char	 cfgcmd[MAXPATHLEN];


	snprintf(cfgcmd, sizeof(cfgcmd), CT_FORMAT_DATA, ctpath, ostr, mod);

	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* get version line */
		if (get_line(rpipe, buffer, sbuf) == true) {
			rval = true;
		}

		pclose(rpipe);
	}

	return(rval);
}

