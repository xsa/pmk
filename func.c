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


#include <stdlib.h>

#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "autoconf.h"
#include "cfgtool.h"
#include "codebuild.h"
#include "common.h"
#include "detect.h"
#include "func.h"
#include "functool.h"
#include "hash.h"
#include "pkgconfig.h"
#include "premake.h"
#include "tags.h"


/*#define SHLIB_DEBUG 1*/


/***************
 * parser data *
 ***********************************************************************/

/* common required options */
kw_t	req_name[] = {
	{KW_OPT_NAME,	PO_STRING}
};

/* CHECK_BIN options */
kw_t	opt_chkbin[] = {
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_REQUIRED,	PO_BOOL},
	{KW_OPT_VARIABLE,	PO_STRING}
};

kwopt_t	kw_chkbin = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chkbin,
	sizeof(opt_chkbin) / sizeof(kw_t)
};

/* CHECK_HEADER options */
kw_t	opt_chkhdr[] = {
	{KW_OPT_CFLAGS,		PO_STRING},
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_SUBHDR,		PO_LIST},
	{KW_OPT_FUNCTION,	PO_STRING | PO_LIST},
	{KW_OPT_LANG,		PO_STRING},
	{KW_OPT_MACRO,		PO_LIST},
	{KW_OPT_REQUIRED,	PO_BOOL}
};

kwopt_t	kw_chkhdr = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chkhdr,
	sizeof(opt_chkhdr) / sizeof(kw_t)
};

/* CHECK_LIB options */
kw_t	opt_chklib[] = {
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_FUNCTION,	PO_STRING | PO_LIST},
	{KW_OPT_LANG,		PO_STRING},
	{KW_OPT_LIBS,		PO_STRING},
	{KW_OPT_MACRO,		PO_LIST},
	{KW_OPT_REQUIRED,	PO_BOOL}
};

kwopt_t	kw_chklib = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chklib,
	sizeof(opt_chklib) / sizeof(kw_t)
};

/* CHECK_CONFIG options */
kw_t	opt_chkcfg[] = {
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_CFLAGS,		PO_STRING},
	{KW_OPT_LIBS,		PO_STRING},
	{KW_OPT_REQUIRED,	PO_BOOL},
	{KW_OPT_VARIABLE,	PO_STRING},
	{KW_OPT_VERSION,	PO_STRING}
};

kwopt_t	kw_chkcfg = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chkcfg,
	sizeof(opt_chkcfg) / sizeof(kw_t)
};

/* CHECK_PKG_CONFIG options */
kw_t	opt_chkpc[] = {
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_CFLAGS,		PO_STRING},
	{KW_OPT_LIBS,		PO_STRING},
	{KW_OPT_REQUIRED,	PO_BOOL},
	{KW_OPT_VERSION,	PO_STRING}
};

kwopt_t	kw_chkpc = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chkpc,
	sizeof(opt_chkpc) / sizeof(kw_t)
};

/* CHECK_TYPE options */
kw_t	opt_chktyp[] = {
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_HEADER,		PO_STRING},
	{KW_OPT_LANG,		PO_STRING},
	{KW_OPT_MEMBER,		PO_STRING},
	{KW_OPT_REQUIRED,	PO_BOOL}
};

kwopt_t	kw_chktyp = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chktyp,
	sizeof(opt_chktyp) / sizeof(kw_t)
};

/* CHECK_VARIABLE options */
kw_t	opt_chkvar[] = {
	{KW_OPT_DEFS,		PO_LIST},
	{KW_OPT_DEPEND,		PO_LIST},
	{KW_OPT_REQUIRED,	PO_BOOL},
	{KW_OPT_VALUE,		PO_STRING}
};

kwopt_t	kw_chkvar = {
	req_name,
	sizeof(req_name) / sizeof(kw_t),
	opt_chkvar,
	sizeof(opt_chkvar) / sizeof(kw_t)
};

/* BUILD_LIB_NAME options */
kw_t	req_bldlib[] = {
	{KW_OPT_NAME,		PO_STRING}
};
kw_t	opt_bldlib[] = {
	{KW_OPT_REQUIRED,	PO_BOOL},
	{KW_OPT_MAJOR,		PO_STRING},
	{KW_OPT_MINOR,		PO_STRING},
	{KW_SL_STATIC,	    PO_STRING},
	{KW_SL_SHARED,	    PO_STRING},
	{KW_SL_VERSION,	    PO_BOOL}
};

kwopt_t	kw_bldlib = {
	req_bldlib,
	sizeof(req_bldlib) / sizeof(kw_t),
	opt_bldlib,
	sizeof(opt_bldlib) / sizeof(kw_t)
};

/* BUILD_SHLIB_NAME options */
kw_t	req_bldshl[] = {
	{KW_OPT_NAME,		PO_STRING}
};
kw_t	opt_bldshl[] = {
	{KW_OPT_REQUIRED,	PO_BOOL},
	{KW_OPT_MAJOR,		PO_STRING},
	{KW_OPT_MINOR,		PO_STRING},
	{KW_SL_VERS_FULL,	PO_STRING},
	{KW_SL_VERS_NONE,	PO_STRING}
};

kwopt_t	kw_bldshl = {
	req_bldshl,
	sizeof(req_bldshl) / sizeof(kw_t),
	opt_bldshl,
	sizeof(opt_bldshl) / sizeof(kw_t)
};

/* keyword list */
prskw	kw_pmkfile[] = {
	{"DEFINE",				PMK_TOK_DEFINE,	PRS_KW_NODE, PMK_TOK_SETVAR,	NULL},
	{"SETTINGS",			PMK_TOK_SETNGS,	PRS_KW_NODE, PMK_TOK_SETPRM,	NULL},
	{"IF",					PMK_TOK_IFCOND,	PRS_KW_NODE, PRS_TOK_NULL,		NULL},
	{"ELSE",				PMK_TOK_ELCOND,	PRS_KW_NODE, PRS_TOK_NULL,		NULL},
	{"SWITCHES",			PMK_TOK_SWITCH,	PRS_KW_CELL, PRS_TOK_NULL,		NULL},
	{"CHECK_BINARY",		PMK_TOK_CHKBIN,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chkbin},
	{"CHECK_HEADER",		PMK_TOK_CHKINC,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chkhdr},
	{"CHECK_LIB",			PMK_TOK_CHKLIB,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chklib},
	{"CHECK_CONFIG",		PMK_TOK_CHKCFG,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chkcfg},
	{"CHECK_PKG_CONFIG",	PMK_TOK_CHKPKG,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chkpc},
	{"CHECK_TYPE",			PMK_TOK_CHKTYP,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chktyp},
	{"CHECK_VARIABLE",		PMK_TOK_CHKVAR,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_chkvar},
	{"BUILD_LIB_NAME",	    PMK_TOK_BLDLIB,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_bldlib},
	{"BUILD_SHLIB_NAME",	PMK_TOK_BLDSLN,	PRS_KW_CELL, PRS_TOK_NULL,		&kw_bldshl}
};

size_t	nbkwpf = sizeof(kw_pmkfile) / sizeof(prskw);


/******************
 * misc functions *
 ***********************************************************************/

/*****************
 * func_wrapper()*
 ***********************************************************************
 DESCR
	wrapper from token to functions

 IN
	pcell : cell structure
	pgd : global data

 OUT
	boolean
 ***********************************************************************/

bool func_wrapper(prscell *pcell, pmkdata *pgd) {
	bool	rval;
	pmkcmd	cmd;

	cmd.token = pcell->token;
	cmd.label = pcell->label;

	switch (cmd.token) {
		case PMK_TOK_DEFINE :
			rval = pmk_define(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_SWITCH :
			rval = pmk_switches(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_SETNGS :
			rval = pmk_settings(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_IFCOND :
			rval = pmk_ifcond(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_ELCOND :
			rval = pmk_elcond(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_SETVAR :
			rval = pmk_set_variable(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_SETPRM :
			rval = pmk_set_parameter(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKBIN :
			rval = pmk_check_binary(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKINC :
			rval = pmk_check_header(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKLIB :
			rval = pmk_check_lib(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKCFG :
			rval = pmk_check_config(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKPKG :
			rval = pmk_check_pkg_config(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKTYP :
			rval = pmk_check_type(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_CHKVAR :
			rval = pmk_check_variable(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_BLDLIB :
			rval = pmk_build_lib_name(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_BLDSLN :
			rval = pmk_build_shlib_name(&cmd, pcell->data, pgd);
			break;
		default :
			errorf("unknown token %d", cmd.token);
			rval = false;
	}

	return(rval);
}


/******************
 * process_node() *
 ***********************************************************************
 DESCR
 	processing of given node

 IN
	pnode : node structure
	pgd : global data

 OUT
	boolean
 ***********************************************************************/

bool process_node(prsnode *pnode, pmkdata *pgd) {
	prscell	*pcell;

	/* init pcell with the first cell of pdata */
	pcell = pnode->first;

	while (pcell != NULL) {
		if (func_wrapper(pcell, pgd) == false)
			return(false);

		pcell = pcell->next;
	}

	return(true);
}


/*****************
 * node functions
 *
 * IN
 *	cmd : command structure
 *	pnode : node structure
 *	pgd : global data
 *
 * OUT
 *	boolean
 ***********************************************************************/

/****************
 * pmk_define() *
 ***********************************************************************
 DESCR
	define variables
 ***********************************************************************/

bool pmk_define(pmkcmd *cmd, prsnode *pnode, pmkdata *pgd) {
	pmk_log("\n* Parsing define\n");

	return(process_node(pnode, pgd));
}


/******************
 * pmk_settings() *
 ***********************************************************************
 DESCR
	pmk settings
 ***********************************************************************/

bool pmk_settings(pmkcmd *cmd, prsnode *pnode, pmkdata *pgd) {
	pmk_log("\n* Parsing settings\n");

	return(process_node(pnode, pgd));
}


/****************
 * pmk_ifcond() *
 ***********************************************************************
 DESCR
	if condition
 ***********************************************************************/

bool pmk_ifcond(pmkcmd *cmd, prsnode *pnode, pmkdata *pgd) {
	if (label_check(pgd->labl, cmd->label) == true) {
		pmk_log("\n< Begin of 'IF' condition [%s]\n", cmd->label);
		process_node(pnode, pgd);
		pmk_log("\n> End of 'IF' condition [%s]\n", cmd->label);
	} else {
		pmk_log("\n- Skipping 'IF' condition [%s]\n", cmd->label);
	}

	return(true);
}


/****************
 * pmk_elcond() *
 ***********************************************************************
 DESCR
	else condition
 ***********************************************************************/

bool pmk_elcond(pmkcmd *cmd, prsnode *pnode, pmkdata *pgd) {
	if (label_check(pgd->labl, cmd->label) == false) {
		pmk_log("\n< Begin of 'ELSE' condition [%s]\n", cmd->label);
		process_node(pnode, pgd);
		pmk_log("\n> End of 'ELSE' condition [%s]\n", cmd->label);
	} else {
		pmk_log("\n- Skipping 'ELSE' condition [%s]\n", cmd->label);
	}

	return(true);
}


/*****************
 * cell functions
 *
 * IN
 *	cmd : command structure
 *	ht : command options
 *	pgd : global data
 *
 * OUT
 *	boolean
 ***********************************************************************/

/******************
 * pmk_switches() *
 ***********************************************************************
 DESCR
	switches
 ***********************************************************************/

bool pmk_switches(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	char			*value;
	hkeys_t			*phk;
	pmkobj			*po;
	unsigned int	 i,
					 n = 0;

	pmk_log("\n* Parsing switches\n");

	phk = hash_keys(ht);

	for(i = 0 ; i < phk->nkey ; i++) {
		if (hash_get(pgd->labl, phk->keys[i]) == NULL) {
			po = hash_get(ht, phk->keys[i]);
			switch(po_get_type(po)) {
				case PO_BOOL :
					value = bool_to_str(po_get_bool(po));
					break;
				case PO_STRING :
					value = po_get_str(po);
					break;
				default :
					errorf("bad type for switch '%s'.", phk->keys[i]);
					return(false);
					break;
			}

			if (hash_update_dup(pgd->labl, phk->keys[i], value) == false) {
				errorf("updating '%s' : %s", phk->keys[i], hash_error(pgd->labl));
				return(false);
			}

			pmk_log("\tAdded '%s' switch.\n", phk->keys[i]);
			n++;
		} else {
			pmk_log("\tSkipped '%s' switch (overriden).\n", phk->keys[i]);
		}
	}
	pmk_log("\tTotal %d switch(es) added.\n", n);

	hash_free_hkeys(phk);

	return(true);
}


/**********************
 * pmk_check_binary() *
 ***********************************************************************
 DESCR
	check binary
 ***********************************************************************/

bool pmk_check_binary(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool	 required,
			 rslt;
	char	*filename,
			*varname,
			*vtmp,
			*bpath,
			 binpath[MAXPATHLEN];

	pmk_log("\n* Checking binary [%s]\n", cmd->label);
	required = require_check(ht);

	filename = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (filename == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}
	record_def_adv(pgd->htab, TAG_TYPE_BIN, filename, NULL, NULL, NULL);

	/* check if a variable name is given */
	varname = po_get_str(hash_get(ht, KW_OPT_VARIABLE));
	if (varname == NULL) {
		vtmp = conv_to_tag(filename);
		if (vtmp == NULL) {
			errorf("failed to generate definition name for "
				"'%s' in label '%s'", filename, cmd->label);
			return(false);
		}
		/* if not then use default naming scheme */
		varname = strdup(vtmp);
		if (varname == NULL) {
			errorf(ERRMSG_MEM);
			return(false);
		}
	}

	bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
	if (bpath == NULL) {
		errorf("%s not available.", PMKCONF_PATH_BIN);
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, filename, NULL));
	}

	/* try to locate binary */
	pmk_log("\tFound binary '%s' : ", filename);
	if (get_file_path(filename, bpath, binpath, sizeof(binpath)) == false) {
		pmk_log("no.\n");
		rslt = process_required(pgd, cmd, required, filename, NULL);
		if (rslt == true) {
			/* set path as empty for the key given by varname */
			if (hash_update_dup(pgd->htab, varname, "") == false) {
				errorf("updating '%s' : %s", varname, hash_error(pgd->htab));
				return(false);
			}
		}
		return(rslt);
	} else {
		pmk_log("yes.\n");
		/* define for template */
		record_def_data(pgd->htab, filename, DEFINE_DEFAULT);
		record_def_adv(pgd->htab, TAG_TYPE_BIN, filename, NULL, NULL, binpath);

		/* recording path of config tool under the key given by varname */
		if (hash_update_dup(pgd->htab, varname, binpath) == false) {
			errorf("updating '%s' : %s", varname, hash_error(pgd->htab));
			return(false);
		}
		label_set(pgd->labl, cmd->label, true);
		return(true);
	}
}


/**********************
 * pmk_check_header() *
 ***********************************************************************
 DESCR
	check header file
 ***********************************************************************/

bool pmk_check_header(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool		 required,
				 rslt = true,
				 rbis,
				 rval;
	char		 inc_path[MAXPATHLEN] = "",
				*lang,
				*pstr,
				*target = NULL;
	code_bld_t	 scb;
	dynary		*funcs,
				*macros,
				*shdrs,
				*defs;
	int			 i,
				 n;

	pmk_log("\n* Checking header [%s]\n", cmd->label);

	code_bld_init(&scb, pgd->buildlog);

	required = require_check(ht);

	/* get include filename */
	pstr = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (pstr == NULL) {
		errorf("%s not assigned in label '%s'", KW_OPT_NAME, cmd->label);
		return(false);
	}
	scb.header = pstr;
	record_def_data(pgd->htab, scb.header, NULL);
	record_def_adv(pgd->htab, TAG_TYPE_HDR, scb.header, NULL, NULL, NULL);

	/* any macro(s) to check ? */
	macros = po_get_list(hash_get(ht, KW_OPT_MACRO));
	if (macros != NULL) {
		for (i = 0 ; i < (int) da_usize(macros) ; i++) {
			pstr = da_idx(macros, i);
			record_def_data(pgd->htab, pstr, NULL);
			record_def_adv(pgd->htab, TAG_TYPE_HDR_MCR, scb.header, pstr, NULL, NULL);
		}
	}

	/* optional header sub dependencies */
	shdrs = po_get_list(hash_get(ht, KW_OPT_SUBHDR));

	/* check if a function or more must be searched */
	obsolete_string_to_list(ht, KW_OPT_FUNCTION);
	funcs = po_get_list(hash_get(ht, KW_OPT_FUNCTION));
	if (funcs != NULL) {
		n = da_usize(funcs);
		for (i = 0 ; i < n ; i++) {
			pstr = da_idx(funcs, i);
			record_def_data(pgd->htab, pstr, NULL);
			record_def_adv(pgd->htab, TAG_TYPE_HDR_PRC, scb.header, pstr, NULL, NULL);
		}
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));
	if (process_def_list(pgd->htab, defs, false) == false) {
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get language */
	lang = get_lang_str(ht, pgd);
	if (lang == NULL) {
		errorf("Failed, language has not been set.");
		return(false);
	}

	/* init build structure */
	if (set_language(&scb, lang) == false) {
		pmk_log("\tSkipped, unknown language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get the appropriate compiler */
	pstr = set_compiler(&scb, pgd->htab);
	if (pstr == NULL) {
		pmk_log("\tSkipped, cannot get compiler path for language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	pmk_log("\tUse %s language with %s compiler.\n", lang, pstr);

	/* check for alternative variable for CFLAGS */
	pstr = po_get_str(hash_get(ht, PMKVAL_ENV_CFLAGS));
	if (pstr != NULL) {
		/* init alternative variable */
		if (hash_get(pgd->htab, pstr) == NULL) {
			if (hash_update_dup(pgd->htab, pstr, "") == false) {
				errorf("updating '%s' : %s", pstr, hash_error(pgd->htab));
				return(false);
			}
		}

		scb.alt_cflags = pstr;
	}
	pmk_log("\tStore compiler flags in '%s'.\n", get_cflags_label(&scb));

	/* use each element of INC_PATH with -I */
	pstr = (char *) hash_get(pgd->htab, PMKCONF_PATH_INC);
	if (pstr == NULL) {
		errorf("%s not available.", PMKCONF_PATH_INC);
		return(false);
	}

	if (get_file_dir_path(scb.header, pstr, inc_path, sizeof(inc_path)) == true) {
		pstr = strdup(inc_path);
		if (pstr == NULL) {
			errorf(ERRMSG_MEM);
			return(false);
		}
		strlcpy(inc_path, "-I", sizeof(inc_path)); /* no check */
		if (strlcat_b(inc_path, pstr, sizeof(inc_path)) == false) {
			errorf("failed to add '%s' in include path.", inc_path);
			return(false);
		}
	} else {
		/* include not found, init inc_path with "" */
		if (strlcpy_b(inc_path, "", sizeof(inc_path)) == false) {
			errorf("failed to initialise include path.");
			return(false);
		}
	}
	set_cflags(&scb, inc_path);

	/*
		check header file
	*/
	pmk_log("\tFound header '%s' : ", scb.header);

	/* build test source file */
	if (code_builder(&scb) == false) {
		errorf("cannot build test file.");
		return(false);
	}

	/* build compiler command */
	 if (cmdline_builder(&scb, LINK_SRC) == false) {
		errorf(ERR_MSG_CC_CMD);
		return(false);
	}

	/* launch build and get result */
	rslt = object_builder(&scb);

	/* clean files */
	cb_cleaner(&scb);

	/* if result is false and dependency headers are provided */
	if ((rslt == false) && (shdrs != NULL)) {
		/* output result of previous test */
		pmk_log("no.\n");

		/*
			check header file with sub headers
		*/
		pmk_log("\tFound header '%s' using dependency headers : ", scb.header);

		scb.subhdrs = shdrs;

		/* build test source file */
		if (code_builder(&scb) == false) {
			errorf("cannot build test file.");
			return(false);
		}

		/* build compiler command */
		 if (cmdline_builder(&scb, LINK_SRC) == false) {
			errorf(ERR_MSG_CC_CMD);
			return(false);
		}

		/* launch build and get result */
		rslt = object_builder(&scb);

		/* clean files */
		cb_cleaner(&scb);

		/* XXX TODO set sub header flag ? */
	}

	/* process result */
	if (rslt == false) {
		pmk_log("no.\n");
		return(process_required(pgd, cmd, required, NULL, NULL));
	}
	pmk_log("yes.\n");
	/* define for template */
	record_def_data(pgd->htab, scb.header, DEFINE_DEFAULT);
	record_def_adv(pgd->htab, TAG_TYPE_HDR, scb.header, NULL, NULL, DEFINE_DEFAULT);

	/*
		check macros in header file
	*/
	if (macros != NULL) {
		while ((target = da_shift(macros)) && (target != NULL)) {
			/* init rbis */
			rbis = false;

			/* if header is okay */
			pmk_log("\tFound macro '%s' : ", target);

			scb.define = target;

			/* fill test file */
			if (code_builder(&scb) == false) {
				errorf("cannot build test file.");
				return(false);
			}

			/* build compiler command */
			 if (cmdline_builder(&scb, LINK_SRC) == false) {
				errorf(ERR_MSG_CC_CMD);
				return(false);
			}

			/* launch build and get result */
			rbis = object_builder(&scb);

			/* clean files */
			cb_cleaner(&scb);

			/* process result */
			if (rbis == true) {
				pmk_log("yes.\n");
				/* define for template */
				record_def_data(pgd->htab, target, DEFINE_DEFAULT);
				record_def_adv(pgd->htab, TAG_TYPE_HDR_MCR, scb.header, target, NULL, DEFINE_DEFAULT);
			} else {
				pmk_log("no.\n");
				rslt = false;
			}
		}

		/* reset define */
		scb.define = NULL;
	}

	/*
		check functions in header file
	*/
	if (funcs != NULL) {
		while ((target = da_shift(funcs)) && (target != NULL)) {
			/* test each function */
			pmk_log("\tFound function '%s' : ", target);

			scb.procedure = target;

			/* fill test file */
			if (code_builder(&scb) == false) {
				errorf("cannot build test file.");
				return(false);
			}

			 if (cmdline_builder(&scb, LINK_SRC) == false) {
				errorf(ERR_MSG_CC_CMD);
				return(false);
			}

			/* launch build and get result */
			rbis = object_builder(&scb);

			/* clean files */
			cb_cleaner(&scb);

			/* process result */
			if (rbis == true) {
				pmk_log("yes.\n");

				/* define for template */
				record_def_data(pgd->htab, target, DEFINE_DEFAULT);
				record_def_adv(pgd->htab, TAG_TYPE_HDR_PRC, scb.header, target, NULL, DEFINE_DEFAULT);
			} else {
				pmk_log("no.\n");
				rslt = false;
			}
		}
	}

	if (rslt == true) {
		label_set(pgd->labl, cmd->label, true);
		/* process additional defines */
		if (process_def_list(pgd->htab, defs, true) == false) {
			return(false);
		}

		/* put result in CFLAGS, CXXFLAGS or alternative variable */
		if (single_append(pgd->htab, get_cflags_label(&scb), strdup(inc_path)) == false) {
			errorf("failed to append '%s' in '%s'.", inc_path, get_cflags_label(&scb));
			return(false);
		}

		rval = true;
	} else {
		rval = process_required(pgd, cmd, required, NULL, NULL);
	}

	return(rval);
}


/*******************
 * pmk_check_lib() *
 ***********************************************************************
 DESCR
	check library
 ***********************************************************************/

bool pmk_check_lib(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool		 required,
				 rbis,
				 rslt = true,
				 rval;
	char		 lib_buf[TMP_BUF_LEN] = "",
				 lib_path[TMP_BUF_LEN] = "",
				*lang,
				 pattern[TMP_BUF_LEN] = "",
				*pstr,
				*target = NULL;
	code_bld_t	 scb;
	dynary		*da,
				*funcs,
				*defs;
	int			 i,
				 n;

	pmk_log("\n* Checking library [%s]\n", cmd->label);

	code_bld_init(&scb, pgd->buildlog);

	required = require_check(ht);

	pstr = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (pstr == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}
	scb.library = pstr;
	snprintf(lib_buf, sizeof(lib_buf), "lib%s", scb.library);
	record_def_data(pgd->htab, lib_buf, NULL);
	record_def_adv(pgd->htab, TAG_TYPE_LIB, scb.library, NULL, NULL, NULL);

	/* check if a function or more must be searched */
	obsolete_string_to_list(ht, KW_OPT_FUNCTION);
	funcs = po_get_list(hash_get(ht, KW_OPT_FUNCTION));
	if (funcs != NULL) {
		n = da_usize(funcs);
		for (i = 0 ; i < n ; i++) {
			pstr = da_idx(funcs, i);
			record_def_data(pgd->htab, pstr, NULL);
			record_def_adv(pgd->htab, TAG_TYPE_LIB_PRC, scb.library, pstr, NULL, NULL);
		}
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));
	if (process_def_list(pgd->htab, defs, false) == false) {
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get language */
	lang = get_lang_str(ht, pgd);
	if (lang == NULL) {
		errorf("Failed, language has not been set.");
		return(false);
	}

	/* init build structure */
	if (set_language(&scb, lang) == false) {
		pmk_log("\tSkipped, unknown language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get the appropriate compiler */
	pstr = set_compiler(&scb, pgd->htab);
	if (pstr == NULL) {
		pmk_log("\tSkipped, cannot get compiler path for language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	pmk_log("\tUse %s language with %s compiler.\n", lang, pstr);

	if (rslt == true) {
		/* check for alternative variable for LIBS */
		pstr = po_get_str(hash_get(ht, KW_OPT_LIBS));
		if (pstr != NULL) {
			/* init alternative variable */
			if (hash_get(pgd->htab, pstr) == NULL) {
				if (hash_update_dup(pgd->htab, pstr, "") == false) {
					errorf("updating '%s' : %s", pstr, hash_error(pgd->htab));
					return(false);
				}
			}

			scb.alt_libs = pstr;
		}
		pmk_log("\tStore library flags in '%s'.\n", get_libs_label(&scb));

		/* get actual content of LIBS, no need to check as it is initialised */
		set_cflags(&scb, hash_get(pgd->htab, PMKVAL_ENV_LIBS));
	}

	/* get the list of library path */
	pstr = (char *) hash_get(pgd->htab, PMKCONF_PATH_LIB);
	if (pstr == NULL) {
		errorf("%s not available.", PMKCONF_PATH_LIB);
		return(false);
	}

	/* convert to dynary */
	da = str_to_dynary(pstr, PATH_STR_DELIMITER);
	if (da == NULL) {
		errorf("unable to convert '%s'",PMKCONF_PATH_LIB);
		return(false);
	}

	/* generate library pattern */
	snprintf(pattern, sizeof(pattern), "lib%s*.*", scb.library);

	/* parse list of path to find the pattern */
	if (find_pattern(da, pattern, lib_buf, sizeof(lib_buf)) == true) {
		snprintf(lib_path, sizeof(lib_path), "-L%s", lib_buf);
		set_ldflags(&scb, lib_path);
	}

	/* clean dynary now useless */
	da_destroy(da);
	
	/*
		check library
	*/
	pmk_log("\tFound library '%s' : ", scb.library);

	/* build test source file */
	if (code_builder(&scb) == false) {
		errorf("cannot build test file.");
		return(false);
	}

	/* build compiler command */
	 if (cmdline_builder(&scb, LINK_SRC) == false) {
		errorf(ERR_MSG_CC_CMD);
		return(false);
	}

	/* launch build and get result */
	rslt = object_builder(&scb);

	/* clean files */
	cb_cleaner(&scb);

	/* process result */
	if (rslt == false) {
		pmk_log("no.\n");
		return(process_required(pgd, cmd, required, NULL, NULL));
	}
	pmk_log("yes.\n");
	/* define for template */
	record_def_data(pgd->htab, scb.library, DEFINE_DEFAULT);
	record_def_adv(pgd->htab, TAG_TYPE_LIB, scb.library, pstr, NULL, DEFINE_DEFAULT);

	/*
		check functions
	*/

	if (funcs != NULL) {
		while ((target = da_shift(funcs)) && (target != NULL)) {
			/* test each function */
			pmk_log("\tFound function '%s' : ", target);

			scb.procedure = target;

			/* build test source file */
			if (code_builder(&scb) == false) {
				errorf("cannot build test file.");
				return(false);
			}

			/* build compiler command */
			 if (cmdline_builder(&scb, LINK_SRC) == false) {
				errorf(ERR_MSG_CC_CMD);
				return(false);
			}

			/* launch build and get result */
			rbis = object_builder(&scb);

			/* clean files */
			cb_cleaner(&scb);

			/* process result */
			if (rbis == true) {
				pmk_log("yes.\n");

				/* define for template */
				record_def_data(pgd->htab, target, DEFINE_DEFAULT);
				record_def_adv(pgd->htab, TAG_TYPE_LIB_PRC, scb.library, target, NULL, DEFINE_DEFAULT);
			} else {
				pmk_log("no.\n");
				rslt = false;
			}
		}
	}

	if (rslt == true) {
		label_set(pgd->labl, cmd->label, true);
		/* process additional defines */
		if (process_def_list(pgd->htab, defs, true) == false) {
			return(false);
		}

		if (scb.ldflags != NULL) {
			if (snprintf_b(lib_buf, sizeof(lib_buf), "%s -l%s", scb.ldflags, scb.library) == false) {
				errorf("failed to build library path or name.");
				return(false);
			}
		} else {
			if (snprintf_b(lib_buf, sizeof(lib_buf), "-l%s", scb.library) == false) {
				errorf("failed to build library name.");
				return(false);
			}
		}

		/* put result in LIBS or alternative variable */
		if (single_append(pgd->htab, get_libs_label(&scb), strdup(lib_buf)) == false) {
			errorf("failed to append '%s' in '%s'.", lib_buf, get_libs_label(&scb));
			return(false);
		}

		rval = true;
	} else {
		rval = process_required(pgd, cmd, required, NULL, NULL);
	}

	return(rval);
}


/**********************
 * pmk_check_config() *
 ***********************************************************************
 DESCR
	check with *-config utility
 ***********************************************************************/

bool pmk_check_config(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool		 required = true,
				 rval = false;
	char		 pipebuf[TMP_BUF_LEN],
				 cfgpath[MAXPATHLEN],
				*bpath,
				*cfgtool,
				*cflags,
				*lang,
				*libs,
				*libvers,
				*modname,
				*opt,
				*varname,
				*vtmp;
	code_bld_t	 scb;
	dynary		*defs;
	cfgtcell	*pcc = NULL;

	pmk_log("\n* Checking with config tool [%s]\n", cmd->label);

	code_bld_init(&scb, pgd->buildlog);
	
	required = require_check(ht);

	cfgtool = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (cfgtool == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}
	record_def_adv(pgd->htab, TAG_TYPE_CFGTOOL, cfgtool, NULL, NULL, NULL);

	/* check dependencies */
	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, cfgtool, NULL));
	}

	/* check if a module name is given */
	modname = po_get_str(hash_get(ht, KW_OPT_MODULE));

	/* check if a variable name is given */
	varname = po_get_str(hash_get(ht, KW_OPT_VARIABLE));
	if (varname == NULL) {
		vtmp = conv_to_tag(cfgtool);
		if (vtmp == NULL) {
			errorf("VARIABLE not assigned in label '%s'.", cmd->label);
			return(false);
		}
		/* if not then use default naming scheme */
		varname = strdup(vtmp);
		if (varname == NULL) {
			errorf(ERRMSG_MEM);
			return(false);
		}
	}

	bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
	if (bpath == NULL) {
		errorf("%s not available.", PMKCONF_PATH_BIN);
		return(false);
	}

	/* get language */
	lang = get_lang_str(ht, pgd);
	if (lang == NULL) {
		errorf("Failed, language has not been set.");
		return(false);
	}

	/* init build structure */
	if (set_language(&scb, lang) == false) {
		pmk_log("\tSkipped, unknown language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));
	if (process_def_list(pgd->htab, defs, false) == false) {
		return(false);
	}

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, PMKVAL_ENV_CFLAGS));
	if (cflags != NULL) {
		/* init alternative variable */
		if (hash_get(pgd->htab, cflags) == NULL) {
			if (hash_update_dup(pgd->htab, cflags, "") == false) {
				errorf("updating '%s' : %s", cflags, hash_error(pgd->htab));
				return(false);
			}
		}
	} else {
		/* use default variable of the used language */
		cflags = get_cflags_label(&scb);
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, KW_OPT_LIBS));
	if (libs != NULL) {
		/* init alternative variable */
		if (hash_get(pgd->htab, libs) == NULL) {
			if (hash_update_dup(pgd->htab, libs, "") == false) {
				errorf("updating '%s' : %s", libs, hash_error(pgd->htab));
				return(false);
			}
		}
	} else {
		/* use default library variable */
		libs = PMKVAL_ENV_LIBS;
	}

	/* try to locate cfgtool */
	pmk_log("\tFound config tool '%s' : ", cfgtool);
	if (get_file_path(cfgtool, bpath, cfgpath, sizeof(cfgpath)) == false) {
		pmk_log("no.\n");
		rval = process_required(pgd, cmd, required, cfgtool, NULL);
		if (rval == true) {
			/* set path as empty for the key given by varname */
			if (hash_update_dup(pgd->htab, varname, "") == false) {
				errorf("updating '%s' : %s", varname, hash_error(pgd->htab));
				return(false);
			}
		}
		return(rval);
	} else {
		pmk_log("yes.\n");
		/* recording path of config tool under the key given by varname */
		if (hash_update_dup(pgd->htab, varname, cfgpath) == false) {
			errorf("updating '%s' : %s", varname, hash_error(pgd->htab));
			return(false);
		}
	}

	/* check if config tool data is loaded */
	if (check_cfgt_data(pgd) == false) {
		errorf("unable to load config tool data.");
		return(false);
	}

	/* check if specific tool option exists */
	pmk_log("\tUse specific options : ");
	pcc = cfgtcell_get_cell(pgd->cfgt, cfgtool);
	if (pcc != NULL) {
		pmk_log("yes\n");
	} else {
		pmk_log("no\n");
	}

	libvers = po_get_str(hash_get(ht, KW_OPT_VERSION));

	/* if VERSION is provided then check it */
	if (libvers != NULL) {
		/* check if specific option exists */
		if ((pcc != NULL) && (pcc->version != NULL)) {
			opt = pcc->version;
		} else {
			opt = CFGTOOL_OPT_VERSION;
		}

		if (ct_get_version(cfgpath, opt, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get version from '%s'.", cfgpath);
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);

			if (compare_version(libvers, pipebuf) < 0) {
				/* version does not match */
				pmk_log("no (%s).\n", pipebuf);
				return(process_required(pgd, cmd, required, cfgtool, NULL));
			} else {
				pmk_log("yes (%s).\n", pipebuf);
			}
		}
	}

	/* record gathered data */
	record_def_data(pgd->htab, cfgtool, DEFINE_DEFAULT);
	record_def_adv(pgd->htab, TAG_TYPE_CFGTOOL, cfgtool, NULL, NULL, DEFINE_DEFAULT);
	label_set(pgd->labl, cmd->label, true);
	/* process additional defines */
	if (process_def_list(pgd->htab, defs, true) == false) {
		return(false);
	}

	/* check if specific option exists */
	if ((pcc != NULL) && (pcc->cflags != NULL)) {
		opt = pcc->cflags;
	} else {
		opt = CFGTOOL_OPT_CFLAGS;
	}

	if (ct_get_data(cfgpath, opt, modname, pipebuf, sizeof(pipebuf)) == false) {
		errorf("cannot get CFLAGS.");
		return(false);
	} else {
		/* put result in CFLAGS, CXXFLAGS or alternative variable */
		if (single_append(pgd->htab, cflags, strdup(pipebuf)) == false) {
			errorf("failed to append '%s' in '%s'.", pipebuf, cflags);
			return(false);
		} else {
			pmk_log("\tStored compiler flags in '%s'.\n", cflags);
		}
	}

	/* check if specific option exists */
	if ((pcc != NULL) && (pcc->libs != NULL)) {
		opt = pcc->libs;
	} else {
		opt = CFGTOOL_OPT_LIBS;
	}

	if (ct_get_data(cfgpath, opt, "", pipebuf, sizeof(pipebuf)) == false) {
		errorf("cannot get LIBS.");
		return(false);
	} else {
		/* put result in LIBS or alternative variable */
		if (single_append(pgd->htab, libs, strdup(pipebuf)) == false) {
			errorf("failed to append '%s' in '%s'.", pipebuf, libs);
			return(false);
		} else {
			pmk_log("\tStored library flags in '%s'.\n", libs);
		}
	}

	return(true);
}


/**************************
 * pmk_check_pkg_config() *
 ***********************************************************************
 DESCR
	check pkg-config module using internal support
 ***********************************************************************/

bool pmk_check_pkg_config(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool		 required = true;
	char		 pc_cmd[MAXPATHLEN],
				 pc_buf[MAXPATHLEN],
				*bpath,
				*cflags,
				*lang,
				*libs,
				*libvers,
				*pc_path,
				*pipebuf,
				*pstr,
				*target;
	code_bld_t	 scb;
	dynary		*defs;
	pkgcell		*ppc;
	pkgdata		*ppd;

	pmk_log("\n* Checking pkg-config module [%s]\n", cmd->label);

	code_bld_init(&scb, pgd->buildlog);

	required = require_check(ht);

	target = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (target == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}
	record_def_adv(pgd->htab, TAG_TYPE_PKGCFG, target, NULL, NULL, NULL);

	/* check dependencies */
	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, target, NULL));
	}

	/* try to get pkg-config lib path from pmk.conf */
	pstr = hash_get(pgd->htab, PMKCONF_PC_PATH_LIB);
	if (pstr == NULL) {
		pmk_log("\tUnable to get %s from %s.\n",
			PMKCONF_PC_PATH_LIB, PREMAKE_CONFIG_PATH);
		return(false);
	}

	pc_path = process_string(pstr, pgd->htab);
	if (pc_path == NULL) {
		pmk_log("\tUnable to find pkg-config libdir.\n");
		pmk_log("\tWARNING : pkg-config may not be installed\n");
		pmk_log("\t or pmk.conf need to be updated.\n");
		return(false);
	}

	/* get language */
	lang = get_lang_str(ht, pgd);
	if (lang == NULL) {
		errorf("Failed, language has not been set.");
		return(false);
	}

	/* init build structure */
	if (set_language(&scb, lang) == false) {
		pmk_log("\tSkipped, unknown language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));
	if (process_def_list(pgd->htab, defs, false) == false) {
		return(false);
	}

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, PMKVAL_ENV_CFLAGS));
	if (cflags != NULL) {
		/* init alternative variable */
		if (hash_get(pgd->htab, cflags) == NULL) {
			if (hash_update_dup(pgd->htab, cflags, "") == false) {
				errorf("updating '%s' : %s", cflags, hash_error(pgd->htab));
				return(false);
			}
		}
	} else {
		/* use default variable of the used language */
		cflags = get_cflags_label(&scb);
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, KW_OPT_LIBS));
	if (libs != NULL) {
		/* init alternative variable */
		if (hash_get(pgd->htab, libs) == NULL) {
			if (hash_update_dup(pgd->htab, libs, "") == false) {
				errorf("updating '%s' : %s", libs, hash_error(pgd->htab));
				return(false);
			}
		}
	} else {
		/* use default library variable */
		libs = PMKVAL_ENV_LIBS;
	}

	ppd = pkgdata_init();
	if (ppd == NULL) {
		errorf("cannot init pkgdata.");
		return(false);
	}

	/* collect packages data */
	if (pkg_collect(pc_path, ppd) == false) {
		pkgdata_destroy(ppd);
		errorf("cannot collect packages data");
		return(false);
	}

	/* check if package exists */
	pmk_log("\tFound package '%s' : ", target);
	if (pkg_mod_exists(ppd, target) == false) {
		pkgdata_destroy(ppd);
		pmk_log("no.\n");

		/* get binary path */
		bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
		if (bpath == NULL) {
		        errorf("%s not available.", PMKCONF_PATH_BIN);
			return(false);
		}

		/* check if config tool data is loaded */
		if (check_cfgt_data(pgd) == false) {
			errorf("unable to load config tool data.");
			return(false);
		}

		/* set config tool filename */
		if (cfgtcell_get_binary(pgd->cfgt, target, pc_cmd, sizeof(pc_cmd)) == false) {
			if (snprintf_b(pc_cmd, sizeof(pc_cmd), "%s-config", target) == false) {
				errorf("overflow in snprintf().");
				return(false);
			}
		}

		/* looking for it in the path */
		if (get_file_path(pc_cmd, bpath, pc_buf, sizeof(pc_buf)) == true) {
			/* use CHECK_CONFIG */
			pmk_log("\tFound alternative '%s' tool.\n", pc_cmd);

			/* override NAME to use divert on CHECK_CONFIG */
			hash_update(ht, KW_OPT_NAME, po_mk_str(pc_cmd));
			pmk_log("\tWARNING: rerouting to CHECK_CONFIG\n");
			pmk_log("\tPlease consider using directly CHECK_CONFIG with '%s'\n", pc_cmd);

			/* call pmk_check_config with the config tool */
			return(pmk_check_config(cmd, ht, pgd));
		}

		return(process_required(pgd, cmd, required, target, NULL));
	} else {
		pmk_log("yes.\n");
	}

	libvers = po_get_str(hash_get(ht, KW_OPT_VERSION));
	if (libvers != NULL) {
		/* if VERSION is provided then check it */
		ppc = pkg_cell_add(ppd, target);
		if (ppc == NULL) {
			pkgdata_destroy(ppd);
			errorf("cannot get version.");
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);

			pipebuf = ppc->version;
			if (compare_version(libvers, pipebuf) < 0) {
				/* version does not match */
				pmk_log("no (%s).\n", pipebuf);

				pkgdata_destroy(ppd);

				return(process_required(pgd, cmd, required, target, NULL));
			} else {
				pmk_log("yes (%s).\n", pipebuf);
			}
		}
	}

	/* gather data */
	if (pkg_recurse(ppd, target) == false) {
		pkgdata_destroy(ppd);
		errorf("failed to gather packages data (pkg_recurse() function call)");
		return(false);
	}

	/* record gathered data */
	record_def_data(pgd->htab, target, DEFINE_DEFAULT);
	record_def_adv(pgd->htab, TAG_TYPE_PKGCFG, target, NULL, NULL, DEFINE_DEFAULT);
	label_set(pgd->labl, cmd->label, true);
	/* process additional defines */
	if (process_def_list(pgd->htab, defs, true) == false) {
		return(false);
	}

	/* get cflags recursively */
	pipebuf = pkg_get_cflags(ppd);
	if (pipebuf == NULL) {
		pkgdata_destroy(ppd);
		errorf("cannot get CFLAGS.");
		return(false);
	} else {
		/* put result in CFLAGS, CXXFLAGS or alternative variable */
		if (single_append(pgd->htab, cflags, pipebuf) == false) {
			pkgdata_destroy(ppd);
			errorf("failed to append '%s' in '%s'.", pipebuf, cflags);
			return(false);
		} else {
			pmk_log("\tStored compiler flags in '%s'.\n", cflags);
		}
	}

	/* get libs recursively */
	pipebuf = pkg_get_libs(ppd);
	if (pipebuf == NULL) {
		pkgdata_destroy(ppd);
		errorf("cannot get LIBS.");
		return(false);
	} else {
		/* put result in LIBS or alternative variable */
		if (single_append(pgd->htab, libs, pipebuf) == false) {
			pkgdata_destroy(ppd);
			errorf("failed to append '%s' in '%s'.", pipebuf, libs);
			return(false);
		} else {
			pmk_log("\tStored library flags in '%s'.\n", libs);
		}
	}

	return(true);
}


/********************
 * pmk_check_type() *
 ***********************************************************************
 DESCR
	check type
 ***********************************************************************/

bool pmk_check_type(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool		 required,
				 rslt = true,
				 rval;
	char		 inc_path[MAXPATHLEN] = "",
				*type,
				*member,
				*lang,
				*pstr;
	code_bld_t	 scb;
	dynary		*defs;

	pmk_log("\n* Checking type [%s]\n", cmd->label);

	code_bld_init(&scb, pgd->buildlog);

	required = require_check(ht);

	/* get type name */
	type = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (type == NULL) {
		errorf("%s not assigned in label '%s'", KW_OPT_NAME, cmd->label);
		return(false);
	}
	record_def_data(pgd->htab, type, NULL);
	record_def_adv(pgd->htab, TAG_TYPE_TYPE, type, NULL, NULL, NULL);

	/* check if an header must be used */
	scb.header = po_get_str(hash_get(ht, KW_OPT_HEADER));
	if (scb.header != NULL) {
		record_def_data(pgd->htab, scb.header, NULL);
	}

	/* check if a structure member is given */
	member = po_get_str(hash_get(ht, KW_OPT_MEMBER));
	if (member != NULL) {
		record_def_data(pgd->htab, member, NULL);
	}

	if (scb.header != NULL) {
		record_def_adv(pgd->htab, TAG_TYPE_HDR_TYPE, scb.header, type, NULL, NULL);
		if (member != NULL) {
			record_def_adv(pgd->htab, TAG_TYPE_HDR_TYP_MBR, scb.header, type, member, NULL);
		}
	} else {
		if (member != NULL) {
			record_def_adv(pgd->htab, TAG_TYPE_TYP_MBR, type, member, NULL, NULL);
		}
	}


	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));
	if (process_def_list(pgd->htab, defs, false) == false) {
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get language */
	lang = get_lang_str(ht, pgd);
	if (lang == NULL) {
		errorf("Failed, language has not been set.");
		return(false);
	}

	/* init build structure */
	if (set_language(&scb, lang) == false) {
		pmk_log("\tSkipped, unknown language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get the appropriate compiler */
	pstr = set_compiler(&scb, pgd->htab);
	if (pstr == NULL) {
		pmk_log("\tSkipped, cannot get compiler path for language '%s'.\n", lang);
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	pmk_log("\tUse %s language with %s compiler.\n", lang, pstr);

	/* check optional header */
	if (scb.header != NULL) {
		/* use each element of INC_PATH with -I */
		pstr = (char *) hash_get(pgd->htab, PMKCONF_PATH_INC);
		if (pstr == NULL) {
			errorf("%s not available.", PMKCONF_PATH_INC);
			return(false);
		}

		if (get_file_dir_path(scb.header, pstr, inc_path, sizeof(inc_path)) == true) {
			pstr = strdup(inc_path);
			if (pstr == NULL) {
				errorf(ERRMSG_MEM);
				return(false);
			}
			strlcpy(inc_path, "-I", sizeof(inc_path)); /* no check */
			if (strlcat_b(inc_path, pstr, sizeof(inc_path)) == false) {
				errorf("failed to add '%s' in include path.", inc_path);
				return(false);
			}
		} else {
			/* include not found, init inc_path with "" */
			if (strlcpy_b(inc_path, "", sizeof(inc_path)) == false) {
				errorf("failed to initialise include path.");
				return(false);
			}
		}
		set_cflags(&scb, inc_path);

		/*
			check header file
		*/
		pmk_log("\tFound header '%s' : ", scb.header);

		/* build test source file */
		if (code_builder(&scb) == false) {
			errorf("cannot build test file.");
			return(false);
		}

		/* build compiler command */
		 if (cmdline_builder(&scb, LINK_SRC) == false) {
			errorf(ERR_MSG_CC_CMD);
			return(false);
		}

		/* launch build and get result */
		rslt = object_builder(&scb);

		/* clean files */
		cb_cleaner(&scb);

		/* process result */
		if (rslt == false) {
			pmk_log("no.\n");
			return(process_required(pgd, cmd, required, NULL, NULL));
		}

		pmk_log("yes.\n");
		/* define for template */
		record_def_data(pgd->htab, scb.header, DEFINE_DEFAULT);
		record_def_adv(pgd->htab, TAG_TYPE_HDR_TYPE, scb.header, type, NULL, DEFINE_DEFAULT);
	}

	scb.type = type;

	pmk_log("\tFound type '%s' : ", scb.type);

	/* build test source file */
	if (code_builder(&scb) == false) {
		errorf("cannot build test file.");
		return(false);
	}

	/* build compiler command */
	 if (cmdline_builder(&scb, LINK_SRC) == false) {
		errorf(ERR_MSG_CC_CMD);
		return(false);
	}

	/* launch build and get result */
	rslt = object_builder(&scb);

	/* clean files */
	cb_cleaner(&scb);

	/* process result */
	if (rslt == false) {
		pmk_log("no.\n");
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	pmk_log("yes.\n");

	/* define for template */
	record_def_data(pgd->htab, type, DEFINE_DEFAULT);
	record_def_adv(pgd->htab, TAG_TYPE_TYPE, type, NULL, NULL, DEFINE_DEFAULT);

	/* check optional member */
	if (member != NULL) {
		scb.member = member;

		pmk_log("\tFound member '%s' : ", scb.member);

		/* build test source file */
		if (code_builder(&scb) == false) {
			errorf("cannot build test file.");
			return(false);
		}

		/* build compiler command */
		 if (cmdline_builder(&scb, LINK_SRC) == false) {
			errorf(ERR_MSG_CC_CMD);
			return(false);
		}

		/* clean files */
		cb_cleaner(&scb);

		/* process result */
		if (rslt == false) {
			pmk_log("no.\n");
			return(process_required(pgd, cmd, required, NULL, NULL));
		}

		pmk_log("yes.\n");

		/* define for template */
		record_def_data(pgd->htab, member, DEFINE_DEFAULT);
		record_def_adv(pgd->htab, TAG_TYPE_HDR_TYPE, type, member, NULL, DEFINE_DEFAULT);
		if (scb.header != NULL) {
			record_def_adv(pgd->htab, TAG_TYPE_HDR_TYP_MBR, scb.header, type, member, DEFINE_DEFAULT);
		}
	}

	if (rslt == true) {
		label_set(pgd->labl, cmd->label, true);
		/* process additional defines */
		if (process_def_list(pgd->htab, defs, true) == false) {
			return(false);
		}

		rval = true;
	} else {
		rval = process_required(pgd, cmd, required, scb.type, NULL);
	}

	return(rval);
}


/************************
 * pmk_check_variable() *
 ***********************************************************************
 DESCR
	check if variable exists and optionally it's value
 ***********************************************************************/

bool pmk_check_variable(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool	 required,
			 rval = false;
	char	*var,
			*value,
			*varval;
	dynary	*defs;

	pmk_log("\n* Checking variable [%s]\n", cmd->label);

	required = require_check(ht);

	var = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (var == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));
	if (process_def_list(pgd->htab, defs, false) == false) {
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, var, NULL));
	}

	pmk_log("\tFound variable '%s' : ", var);

	/* trying to get variable */
	varval = hash_get(pgd->htab, var);
	if (varval != NULL) {
		pmk_log("yes.\n");

		value = po_get_str(hash_get(ht, KW_OPT_VALUE));
		if (value == NULL) {
			label_set(pgd->labl, cmd->label, true);
			/* process additional defines */
			if (process_def_list(pgd->htab, defs, true) == false) {
				return(false);
			}

			rval = true;
		} else {
			pmk_log("\tVariable match value '%s' : ", value);

			if (strncmp(value, varval, sizeof(varval)) == 0) {
				pmk_log("yes.\n");
				label_set(pgd->labl, cmd->label, true);
				if (process_def_list(pgd->htab, defs, true) == false) {
					return(false);
				}

				rval = true;
			} else {
				pmk_log("no.\n");
				rval = process_required(pgd, cmd, required, NULL, NULL);
				if (rval == false) {
					errorf("variable value does not match ('%s' != '%s').", value, varval);
				}
			}
		}
	} else {
		pmk_log("no.\n");
		rval = process_required(pgd, cmd, required, NULL, NULL);
		if (rval == false) {
			errorf("failed to find variable '%s'.", var);
		}
	}

	return(rval);
}


/************************
 * pmk_build_lib_name() *
 ***********************************************************************
 DESCR
	build names of a library
 ***********************************************************************/

bool pmk_build_lib_name(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	bool	 required,
			 versbool;
	char	*shvar = NULL,
			*stvar = NULL,
			*pstr,
			*value;
	pmkobj	*po;

	pmk_log("\n* Building library name\n");

	required = require_check(ht);
	
	pmk_log("\tShared library support : ");
	if (pgd->sys_detect == false) {
		pmk_log("no.\n");
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	pmk_log("yes.\n");

	pstr = hash_get(pgd->htab, SL_SYS_LABEL);
	pmk_log("\tUsing support for '%s'.\n", pstr);

	/* get name (REQUIRED) */
	pstr = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (pstr == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

#ifdef SHLIB_DEBUG
	debugf("pstr(name) = '%s'", pstr);
#endif
	value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
	debugf("value = '%s'", value);
#endif
	hash_update(pgd->htab, LIB_KW_NAME, value);

	/* get major version number */
	pstr = po_get_str(hash_get(ht, KW_OPT_MAJOR));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
		debugf("pstr(major) = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
		debugf("value = '%s'", value);
#endif
		hash_update(pgd->htab, LIB_KW_MAJ, value); /* no dup */
	}

	/* get minor version number */
	pstr = po_get_str(hash_get(ht, KW_OPT_MINOR));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
		debugf("pstr(minor) = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
		debugf("value = '%s'", value);
#endif
		hash_update(pgd->htab, LIB_KW_MIN, value); /* no dup */
	}

	/* get shared lib variable */
	pstr = po_get_str(hash_get(ht, KW_SL_SHARED));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
		debugf("pstr(SHARED) = '%s'", pstr);
#endif
		shvar = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
		debugf("shvar = '%s'", shvar);
#endif
	}

	/* get static lib variable */
	pstr = po_get_str(hash_get(ht, KW_SL_STATIC));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
		debugf("pstr(STATIC) = '%s'", pstr);
#endif
		stvar = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
		debugf("stvar = '%s'", stvar);
#endif
	}

	/* get versioning boolean */
	po = hash_get(ht, KW_SL_VERSION);
	if (po != NULL) {
		versbool = po_get_bool(po);
	} else {
		/* no version by default */
		versbool = false;
	}

	/* process shared lib name */
	if (shvar != NULL) {
		if (versbool == false) {
			/* get libname without version */
			pstr = hash_get(pgd->htab, LIB_KW_SH_NONE);
#ifdef SHLIB_DEBUG
			debugf("pstr(LIB_KW_SH_NONE) = '%s'", pstr);
#endif
			value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
			debugf("value = '%s'", value);
#endif
		} else {
			pstr = hash_get(pgd->htab, LIB_KW_SH_VERS);
#ifdef SHLIB_DEBUG
			debugf("pstr(LIB_KW_SH_VERS) = '%s'", pstr);
#endif
			value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
			debugf("value = '%s'", value);
#endif
		}

		/* no dup, no free */
		if (hash_update(pgd->htab, shvar, value) == false) {
			errorf("updating '%s' : %s", shvar, hash_error(pgd->htab));
			return(false);
		}
		pmk_log("\tSetting %s to '%s'\n", shvar, value);
#ifdef SHLIB_DEBUG
		debugf("save in '%s'", shvar);
#endif
		free(shvar);
	}

	/* process static lib name */
	if (stvar != NULL) {
		if (versbool == false) {
			/* get libname without version */
			pstr = hash_get(pgd->htab, LIB_KW_ST_NONE);
#ifdef SHLIB_DEBUG
			debugf("pstr(LIB_KW_ST_NONE) = '%s'", pstr);
#endif
			value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
			debugf("value = '%s'", value);
#endif
		} else {
			pstr = hash_get(pgd->htab, LIB_KW_ST_VERS);
#ifdef SHLIB_DEBUG
			debugf("pstr(LIB_KW_ST_VERS) = '%s'", pstr);
#endif
			value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
			debugf("value = '%s'", value);
#endif
		}

		/* no dup, no free */
		if (hash_update(pgd->htab, stvar, value) == false) {
			errorf("updating '%s' : %s", stvar, hash_error(pgd->htab));
			return(false);
		}
		pmk_log("\tSetting %s to '%s'\n", stvar, value);
#ifdef SHLIB_DEBUG
		debugf("save in '%s'", stvar);
#endif
		free(stvar);
	}

	return(true);
}


/**************************
 * pmk_build_shlib_name() *
 ***********************************************************************
 DESCR
	build name of a shared library
 ***********************************************************************/

bool pmk_build_shlib_name(pmkcmd *cmd, htable_t *ht, pmkdata *pgd) {
	pmk_log("\n* Building shared library name\n");

	pmk_log("\tBUILD_SHLIB_NAME() has been obsoleted by BUILD_LIB_NAME().\n");
	pmk_log("\tRead the man page for further information.\n");

	return(false);
}


/********************
 * option functions *
 ***********************************************************************
 * IN
 *	cmd : command structure
 *	popt : option structure
 *	pgd : global data
 *
 * OUT
 *	boolean
 ***********************************************************************/

/***********************
 * pmk_set_parameter() *
 ***********************************************************************
 DESCR
	set parameter
 ***********************************************************************/

bool pmk_set_parameter(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	/* gnu autoconf compatibility */
	if (strncmp(popt->key, KW_SETNGS_ACCOMP, sizeof(popt->key)) == 0) {
		return(pmk_setparam_accompat(cmd, popt, pgd));
	}

	/* global language */
	if (strncmp(popt->key, KW_SETNGS_GLANG, sizeof(popt->key)) == 0) {
		return(pmk_setparam_glang(cmd, popt, pgd));
	}

	/* set target files */
	if (strncmp(popt->key, KW_SETNGS_TARGET, sizeof(popt->key)) == 0) {
		return(pmk_setparam_target(cmd, popt, pgd));
	}

	/* compiler detection */
	if (strncmp(popt->key, KW_SETNGS_CCDTCT, sizeof(popt->key)) == 0) {
		return(pmk_setparam_detect(cmd, popt, pgd));
	}

	/* found unknown setting */
	pmk_log("\tunknown '%s' setting.\n", popt->key);
	return(false);
}


/***************************
 * pmk_setparam_accompat() *
 ***********************************************************************
 DESCR
	set autoconf compatability parameter
 ***********************************************************************/

bool pmk_setparam_accompat(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	char	*pstr;

	pmk_log("\tSetting autoconf compatibility :\n");

	/* XXX must check if valid
		pstr = (char *) hash_get(pgd->htab, "SYSCONFDIR");
		hash_update_dup(pgd->htab, "sysconfdir", pstr);
	*/

	/* if a file is given then it will be parsed later */
	pstr = po_get_str(popt->value);
	if (*pstr != CHAR_EOS) {
		pgd->ac_file = strdup(pstr);
		if (pgd->ac_file == NULL) {
			errorf(ERRMSG_MEM);
			return(false);
		}
		pmk_log("\t\tSet file to '%s'.\n", pstr);
		if (hash_update_dup(pgd->htab, AC_VAR_DEF, AC_VALUE_DEF) == false) {
			errorf("updating '%s' : %s", AC_VAR_DEF, hash_error(pgd->htab));
			return(false);
		}
		pmk_log("\t\tSet '%s' value to '%s'.\n", AC_VAR_DEF, AC_VALUE_DEF);
	}

	/* compatibility tags */
	if (ac_set_variables(pgd->htab) == false)
		return(false); /* XXX error message ? */
	pmk_log("\t\tSet specific variables.\n");

	return(true);
}


/************************
 * pmk_setparam_glang() *
 ***********************************************************************
 DESCR
	set global lang parameter
 ***********************************************************************/

bool pmk_setparam_glang(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	bool	 rval = false;
	char	*pstr;

	pmk_log("\tSetting global language :\n");

	/* set global language */
	pstr = po_get_str(popt->value);

	if (pstr != NULL) {
		/* check if provided lang is supported */
		if (verify_language(pstr) != LANG_UNKNOWN) {
			pgd->lang = strdup(pstr);
			if (pgd->lang == NULL) {
				errorf(ERRMSG_MEM);
				return(false);
			}
			pmk_log("\t\tSet to '%s'.\n", pstr);
			rval = true;
		} else {
			errorf("unknown language.");
		}
	} else {
		errorf("syntax error in LANG.");
	}

	return(rval);
}


/*************************
 * pmk_setparam_target() *
 ***********************************************************************
 DESCR
	set target list parameter
 ***********************************************************************/

bool pmk_setparam_target(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	dynary	*da;
	int		 i = 0,
			 n;

	pmk_log("\tCollecting targets :\n");

	da = po_get_list(popt->value);
	if (da == NULL) {
		errorf("syntax error in TARGET.");
		return(false);
	}

	n = da_usize(da);
	for (i=0 ; i < n ; i++) {
		pmk_log("\t\tAdded '%s'.\n", da_idx(da, i));
	}

	pgd->tlist = da;

	pmk_log("\t\tTotal %d target(s) added.\n", n);

	return(true);
}


/*************************
 * pmk_setparam_detect() *
 ***********************************************************************
 DESCR
	detect a list of compilers
 ***********************************************************************/

bool pmk_setparam_detect(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	char			*ccpath,
					*lang,
					*osname,
					*pstr,
					 temp[64]; /* XXX size */
	code_bld_t		 scb;
	comp_parse_t	*pcp;
	compiler_t		*pc;
	dynary			*da;
	hkeys_t			*phk;
	int				 i = 0,
					 n;

	pmk_log("\tDetecting compiler shared library support :\n");

	da = po_get_list(popt->value);
	if (da == NULL) {
		errorf("syntax error in DETECT.");
		return(false);
	}

	/* get os name */
	osname = hash_get(pgd->htab, PMKCONF_OS_NAME);

	/*
	 * parsing compiler data to build a database that will be used further
	 */
	pmk_log("\t\tGathering data for compiler detection.\n");
	pcp = parse_comp_file(PMKCOMP_DATA, osname);
	
	/* process each language provided in detection list */
	n = da_usize(da);
	for (i=0 ; i < n ; i++) {
		lang = da_idx(da, i);

		/* init code building structure */
		code_bld_init(&scb, pgd->buildlog);

		/* init build structure */
		if (set_language(&scb, lang) == false) {
			/*
			 * if language name is invalid then check
			 * for obsolete compiler name
			 */
			pstr = obsolete_get_lang_from_comp(lang); /* OBSOLETE */
			if (pstr == NULL) {
				errorf("Unknown language'%s'.\n", lang);
				return(false);
			}

			pmk_log("\t\tWARNING: obsolete value for compiler detection,\n");
			pmk_log("\t\t\tuse language label instead.\n");

			lang = pstr;
			if (set_language(&scb, lang) == false) {
				errorf("Unknown language '%s'.\n", lang);
				return(false);
			}
		}

		/* get the appropriate compiler */
		set_compiler(&scb, pgd->htab);
		pstr = get_compiler_label(&scb);
		ccpath = hash_get(pgd->htab, pstr);
		if (ccpath == NULL) {
			errorf("Cannot get compiler path ('%s').\n", pstr);
			return(false);
		}

		pmk_log("\t\tDetecting %s compiler : ", lang);

		/* try to identify the compiler */
		pc = &(pgd->comp_data.data[scb.lang]);
		if (comp_detect(ccpath, pgd->buildlog, pc, pcp, scb.pld->slcflags) == false) {
			pmk_log("failed.\n");
			return(false);
		}
		/* set the language type, so the compiler data are valid */
		pc->lang = scb.lang; /* XXX set in detect proc ? */

		pmk_log("%s (version %s).\n", pc->descr, pc->version);

		/* set shared lib compiler flags */
		pmk_log("\t\tSetting %s to '%s'\n", scb.pld->slcflags, pc->slcflags);
		if (hash_update_dup(pgd->htab, scb.pld->slcflags, pc->slcflags) == false) {
			return(false);
		}

		/* set shared lib linker flags */
		pmk_log("\t\tSetting %s to '%s'\n", scb.pld->slldflags, pc->slldflags);
		if (hash_update_dup(pgd->htab, scb.pld->slldflags, pc->slldflags) == false) {
			return(false);
		}

		/* try to compile shared object */
		pmk_log("\t\tChecking shared library support : ");
		if (check_so_support(&scb, pc->slcflags, pc->slldflags) == true) {
			pmk_log("ok.\n");

			/* shared lib support */
			snprintf(temp, sizeof(temp), "%s ", scb.pld->mk_bld_rule);
			if (hash_append(pgd->htab, MK_VAR_SL_BUILD, strdup(temp), NULL) == false) {
				return(false);
			}
			snprintf(temp, sizeof(temp), "%s ", scb.pld->mk_cln_rule);
			if (hash_append(pgd->htab, MK_VAR_SL_CLEAN, strdup(temp), NULL) == false) {
				return(false);
			}
			snprintf(temp, sizeof(temp), "%s ", scb.pld->mk_inst_rule);
			if (hash_append(pgd->htab, MK_VAR_SL_INST, strdup(temp), NULL) == false) {
				return(false);
			}
			snprintf(temp, sizeof(temp), "%s ", scb.pld->mk_deinst_rule);
			if (hash_append(pgd->htab, MK_VAR_SL_DEINST, strdup(temp), NULL) == false) {
				return(false);
			}
		} else {
			pmk_log("failed.\n");
		}

		/* clean cdata */
		cb_cleaner(&scb);
	}

	/* get system data */
	pmk_log("\t\tGetting system data for %s : ", osname);
	phk = hash_keys_sorted(pcp->sht);
	if (phk == NULL) {
		pmk_log("failed.\n");
	} else {
		for (i = 0 ; i < (int) phk->nkey ; i++) {
			pstr = hash_get(pcp->sht, phk->keys[i])	;
			hash_update_dup(pgd->htab, phk->keys[i], pstr); /* XXX check */
		}
		pmk_log("ok.\n");

		/* mark system as detected */
		pgd->sys_detect = true;
	}

	/* clean now useless parsed data */
	destroy_comp_parse(pcp);

	return(true);
}


/**********************
 * pmk_set_variable() *
 ***********************************************************************
 DESCR
	set variable
 ***********************************************************************/

bool pmk_set_variable(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	char	 buffer[TMP_BUF_LEN],
			*pstr,
			*value,
			*defname;

	/* XXX better way to do (specific hash for override)
	if (hash_get(pgd->htab, popt->key) == NULL) {
	*/
		/* process value string */
		pstr = po_get_str(popt->value);
		if (pstr == NULL) {
			errorf("bad value for '%s'.", popt->key);
			return(false);
		}
		value = process_string(pstr, pgd->htab);
		if (value != NULL) {
			if (hash_update(pgd->htab, popt->key, value) == false) { /* no need to strdup */
				errorf("updating '%s' : %s", popt->key, hash_error(pgd->htab));
				return(false);
			}

			/* check return for message : defined or redefined */
			switch(pgd->htab->herr) {
				case HERR_ADDED :
					pmk_log("\tdefined");
					break;

				case HERR_UPDATED :
					pmk_log("\tredefined");
					break;
			}

			/* remaining part of the message */
			pmk_log(" '%s' variable.\n", popt->key);

			/* store definition for autoconf compatibility */
			defname = gen_basic_tag_def(popt->key);
			if (defname == NULL) {
				errorf("unable to build define name for '%s'.", popt->key);
				return(false);
			}
			if (snprintf_b(buffer, sizeof(buffer),
					"#define %s \"%s\"", popt->key,
					value) == false) {
				errorf("buffer overflow for define value of '%s'.", popt->key);
				return(false);
			}
			if (hash_update_dup(pgd->htab, defname, buffer) == false) {
				errorf("updating '%s' : %s", defname, hash_error(pgd->htab));
				return(false);
			}

		} else {
			pmk_log("\tFailed processing of '%s'.\n", popt->key);
			return(false);
		}
	/*
	} else {
		pmk_log("\tSkipped '%s' define (overriden).\n", popt->key);
	}
	*/
	return(true);
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
