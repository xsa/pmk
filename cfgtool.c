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

/*#define CFGT_DEBUG	1*/

/* config tools data file keyword */
prskw	kw_pmkcfgtool[] = {
	{"ADD_CFGTOOL", CFGT_TOK_ADDCT, PRS_KW_CELL}
};
int	nbkwct = sizeof(kw_pmkcfgtool) / sizeof(prskw);

/*
	free cfgtcell structure
*/

void cfgtcell_destroy(cfgtcell *pcc) {
#ifdef CFGT_DEBUG
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
#ifdef CFGT_DEBUG
		debugf("cannot initialize config tool data");
#endif
		return(NULL);
	}

	/* initialize config tool hash table */
	pcd->by_mod = hash_init_adv(CFGTOOL_HT_SIZE, NULL,
			(void (*)(void *)) cfgtcell_destroy, NULL);
	if (pcd->by_mod == NULL) {
		free(pcd);
#ifdef CFGT_DEBUG
		debugf("cannot initialize config tool hash table");
#endif
		return(NULL);
	}

	/* initialize config tool hash table */
	pcd->by_bin = hash_init(CFGTOOL_HT_SIZE);
	if (pcd->by_bin == NULL) {
		hash_destroy(pcd->by_mod);
		free(pcd);
#ifdef CFGT_DEBUG
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
#ifdef CFGT_DEBUG
debugf("destroying module hash table.");
#endif
	if (pcd->by_mod != NULL) {
		hash_destroy(pcd->by_mod);
#ifdef CFGT_DEBUG
debugf("destroyed module hash table.");
	} else {
debugf("WARNING : by_mod doesn't exists !!!");
#endif
	}

#ifdef CFGT_DEBUG
debugf("destroying binary links hash table.");
#endif
	if (pcd->by_mod != NULL) {
		hash_destroy(pcd->by_bin);
#ifdef CFGT_DEBUG
debugf("destroyed binary links hash table.");
	} else {
debugf("WARNING : by_bin doesn't exists !!!");
#endif
	}
}

/*
	add a new config tool cell

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
		if (pcell->name == NULL) {
			free(pcell);
			errorf(ERRMSG_MEM);
			return(false);
		}
	}

	pstr = po_get_str(hash_get(pht, "BINARY"));
	if (pstr == NULL) {
		free(pcell->name);
		free(pcell);
		return(false);
	} else {
		pcell->binary = strdup(pstr);
		if (pcell->binary == NULL) {
			free(pcell);
			errorf(ERRMSG_MEM);
			return(false);
		}
	}

	pstr = po_get_str(hash_get(pht, "VERSION"));
	if (pstr != NULL) {
		pcell->version = strdup(pstr);
		if (pcell->version == NULL) {
			free(pcell);
			errorf(ERRMSG_MEM);
			return(false);
		}
	} else {
		pcell->version = NULL;
	}

	pstr = po_get_str(hash_get(pht, "MODULE"));
	if (pstr != NULL) {
		pcell->module = strdup(pstr);
		if (pcell->module  == NULL) {
			free(pcell);
			errorf(ERRMSG_MEM);
			return(false);
		}
	} else {
		pcell->module = NULL;
	}

	pstr = po_get_str(hash_get(pht, "CFLAGS"));
	if (pstr != NULL) {
		pcell->cflags = strdup(pstr);
		if (pcell->cflags == NULL) {
			free(pcell);
			errorf(ERRMSG_MEM);
			return(false);
		}
	} else {
		pcell->cflags = NULL;
	}

	pstr = po_get_str(hash_get(pht, "LIBS"));
	if (pstr != NULL) {
		pcell->libs = strdup(pstr);
		if (pcell->libs == NULL) {
			free(pcell);
			errorf(ERRMSG_MEM);
			return(false);
		}
	} else {
		pcell->libs = NULL;
	}

	/* no need to strdup */
	if (hash_update(pcd->by_mod, pcell->name, pcell) == HASH_ADD_FAIL)
		return(false);
#ifdef CFGT_DEBUG
debugf("added cfgtcell '%s'", pcell->name);
#endif
	if (hash_update_dup(pcd->by_bin, pcell->binary, pcell->name) == HASH_ADD_FAIL)
		return(false);
#ifdef CFGT_DEBUG
debugf("added cfgtcell '%s'", pcell->binary);
#endif

	return(true);
}

/*
	parse data from PMKCFG_DATA file

	return : compiler data structure or NULL
*/

cfgtdata *parse_cfgt_file(void) {
	FILE		*fp;
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

	fp = fopen(PMKCFG_DATA, "r");
	if (fp == NULL) {
		hash_destroy(pcd->by_mod);
		hash_destroy(pcd->by_bin);
		free(pcd);
		prsdata_destroy(pdata);
		errorf("cannot open '%s' : %s.", PMKCFG_DATA, strerror(errno));
		return(NULL);
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fp, pdata, kw_pmkcfgtool, nbkwct);
	fclose(fp);

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
		if (strlcpy_b(buf, pcc->binary, sb) == false)
			return(false);

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
	use config tool to get the version

	ctpath : config tool path
	vstr : option string to get version
	buffer : storage buffer for the result
	sbuf : size of buffer

	returns : true on success else false
*/

bool ct_get_version(char *ctpath, char *vstr, char *buffer, size_t sbuf) {
	FILE	*rpipe;
	bool	 rval = false;
	char	 cfgcmd[MAXPATHLEN];


	if (snprintf_b(cfgcmd, sizeof(cfgcmd),
			CT_FORMAT_VERSION, ctpath, vstr) == false)
		return(false);

	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* get version line */
		rval = get_line(rpipe, buffer, sbuf);

		pclose(rpipe);
	}

	return(rval);
}

/*
	use config tool to get data

	ctpath : config tool path
	vstr : option string to get version
	buffer : storage buffer for the result
	sbuf : size of buffer

	returns : true on success else false
*/

bool ct_get_data(char *ctpath, char *ostr, char *mod, char *buffer, size_t sbuf) {
	FILE	*rpipe;
	bool	 rval = false;
	char	 cfgcmd[MAXPATHLEN];

	if (mod == NULL) {
		/* no module */
		if (snprintf_b(cfgcmd, sizeof(cfgcmd), CT_FORMAT_DATA,
						ctpath, ostr) == false)
			return(false);
	} else {
		/* use module */
		if (snprintf_b(cfgcmd, sizeof(cfgcmd), CT_FORMAT_DATA_MOD,
						ctpath, ostr, mod) == false)
			return(false);
	}

	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* get version line */
		rval = get_line(rpipe, buffer, sbuf);

		pclose(rpipe);
	}

	return(rval);
}
