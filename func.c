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


#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "autoconf.h"
#include "cfgtool.h"
#include "common.h"
#include "detect.h"
#include "func.h"
#include "functool.h"
#include "hash.h"
#include "pkgconfig.h"
#include "premake.h"


/*#define SHLIB_DEBUG 1*/

prskw	kw_pmkfile[] = {
	{"DEFINE",		PMK_TOK_DEFINE,	PRS_KW_NODE, PMK_TOK_SETVAR},
	{"SETTINGS",		PMK_TOK_SETNGS,	PRS_KW_NODE, PMK_TOK_SETPRM},
	{"IF",			PMK_TOK_IFCOND,	PRS_KW_NODE, PRS_TOK_NULL},
	{"ELSE",		PMK_TOK_ELCOND,	PRS_KW_NODE, PRS_TOK_NULL},
	{"SWITCHES",		PMK_TOK_SWITCH,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_BINARY",	PMK_TOK_CHKBIN,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_HEADER",	PMK_TOK_CHKINC,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_LIB",		PMK_TOK_CHKLIB,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_CONFIG",	PMK_TOK_CHKCFG,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_PKG_CONFIG",	PMK_TOK_CHKPKG,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_TYPE",		PMK_TOK_CHKTYP,	PRS_KW_CELL, PRS_TOK_NULL},
	{"CHECK_VARIABLE",	PMK_TOK_CHKVAR,	PRS_KW_CELL, PRS_TOK_NULL},
	{"BUILD_SHLIB_NAME",	PMK_TOK_BLDSLN,	PRS_KW_CELL, PRS_TOK_NULL}
};

size_t	nbkwpf = sizeof(kw_pmkfile) / sizeof(prskw);


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
		case PMK_TOK_BLDSLN :
			rval = pmk_build_shlib_name(&cmd, pcell->data, pgd);
			break;
		default :
			errorf("unknown token %d", cmd.token);
			rval = false;
	}

	return(rval);
}

/*
	process node
*/

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

/*
	node functions
	
	these functions have the following parameters:

	cmd : command structure
	pnode : node structure
	pgd : global data

	returns bool

*/


/*
	define variables

*/

bool pmk_define(pmkcmd *cmd, prsnode *pnode, pmkdata *pgd) {
	pmk_log("\n* Parsing define\n");

	return(process_node(pnode, pgd));
}

/*
	settings
*/

bool pmk_settings(pmkcmd *cmd, prsnode *pnode, pmkdata *pgd) {
	pmk_log("\n* Parsing settings\n");

	return(process_node(pnode, pgd));
}

/*
	if condition
*/

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


/*
	else condition
*/

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


/*
	cell functions
	
	these functions have the following parameters:

	cmd : command structure
	ht : command options
	pgd : global data

	returns bool

*/


/*
	switches
*/

bool pmk_switches(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	char	*value;
	hkeys	*phk;
	int	 i,
		 n = 0;
	pmkobj	*po;

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

			if (hash_update_dup(pgd->labl, phk->keys[i], value) != HASH_ADD_FAIL) {
				pmk_log("\tAdded '%s' switch.\n", phk->keys[i]);
				n++;
			} else {
				errorf(HASH_ERR_UPDT_ARG, phk->keys[i]);
				return(false);
			}
		} else {
			pmk_log("\tSkipped '%s' switch (overriden).\n", phk->keys[i]);
		}
	}
	pmk_log("\tTotal %d switch(es) added.\n", n);

	hash_free_hkeys(phk);

	return(true);
}

/*
	check binary
*/

bool pmk_check_binary(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
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

	/* check if a variable name is given */
	varname = po_get_str(hash_get(ht, KW_OPT_VARIABLE));
	if (varname == NULL) {
		vtmp = str_to_def(filename);
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
			if (hash_update_dup(pgd->htab, varname, "") == HASH_ADD_FAIL) {
				errorf(HASH_ERR_UPDT_ARG, varname);
				return(false);
			}
		}
		return(rslt);
	} else {
		pmk_log("yes.\n");
		/* define for template */
		record_def_data(pgd->htab, filename, DEFINE_DEFAULT);

		/* recording path of config tool under the key given by varname */
		if (hash_update_dup(pgd->htab, varname, binpath) == HASH_ADD_FAIL) {
			errorf(HASH_ERR_UPDT_ARG, varname);
			return(false);
		}
		label_set(pgd->labl, cmd->label, true);
		return(true);
	}
}

/*
	check header file
*/

bool pmk_check_header(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	FILE	*tfp;
	bool	 required,
		 rslt = true,
		 loop = true,
		 clean_funcs = false,
		 check_funcs = false,
		 rval;
	char	 inc_path[MAXPATHLEN],
		 cfgcmd[MAXPATHLEN],
		 ftmp[MAXPATHLEN],
		 btmp[MAXPATHLEN],
		*incfile,
		*incfunc = NULL,
		*target = NULL,
		*ccpath,
		*cflags,
		*pstr;
	dynary	*funcs = NULL,
		*defs;
	int	 i,
		 n,
		 r;
	lgdata	*pld;
	pmkobj	*po;

	pmk_log("\n* Checking header [%s]\n", cmd->label);

	required = require_check(ht);

	/* get include filename */
	incfile = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (incfile == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* check if a function or more must be searched */
	po = hash_get(ht, KW_OPT_FUNCTION);
	if (po == NULL) {
		target = incfile;
	} else {
		/* KW_OPT_FUNCTION provided */
		switch (po_get_type(po)) {
			case PO_STRING:
				pstr = po_get_str(po);
				/* pstr should not be NULL */

				funcs = da_init();
				if (funcs == NULL) {
					errorf("unable to initialise dynary.");
					return(false);
				}

				if (da_push(funcs, pstr) == false) {
					errorf("unable to add '%s' in function list.", pstr);
					return(false);
				}

				clean_funcs = true;
				break;

			case PO_LIST:
				funcs = po_get_list(po);
				/* funcs should not be NULL */
				break;

			default:
				errorf("syntax error for '%s': bad type.", KW_OPT_FUNCTION);
				return(false);
		}
		check_funcs = true;
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		n = da_usize(funcs);
		for (i=0 ; i < n ; i++) {
			pstr = da_idx(funcs, i);
			record_def_data(pgd->htab, pstr, NULL);
		}
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknown language.\n");
		return(invert_bool(required));
	}

	/* get the appropriate compiler */
	ccpath = get_comp_path(pgd->htab, pld->comp);
	if (ccpath == NULL) {
		pmk_log("\tSKIPPED, cannot get compiler path ('%s').\n", pld->comp);
		return(invert_bool(required));
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, PMKVAL_ENV_CFLAGS));
	if (cflags != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, cflags) == NULL) {
                        if (hash_update_dup(pgd->htab, cflags, "") == HASH_ADD_FAIL) {
        			errorf(HASH_ERR_UPDT_ARG, cflags);
        			return(false);
                        }
		}
	} else {
		/* use default variable of the used language */
		cflags = pld->cflg;
	}
	pmk_log("\tStore compiler flags in '%s'.\n", cflags);

	/* use each element of INC_PATH with -I */
	pstr = (char *) hash_get(pgd->htab, PMKCONF_PATH_INC);
	if (pstr == NULL) {
		errorf("%s not available.", PMKCONF_PATH_INC);
		return(false);
	}

	if (get_file_dir_path(incfile, pstr, inc_path, sizeof(inc_path)) == true) {
		pstr = strdup(inc_path);
		if (pstr == NULL) {
			errorf(ERRMSG_MEM);
			return(false);
		}
		strlcpy(inc_path, "-I", sizeof(inc_path));
		strlcat(inc_path, pstr, sizeof(inc_path));
	} else {
		/* include not found, init inc_path with "" */
		strlcpy(inc_path, "", sizeof(inc_path));
	}

	/* check loop */
	while (loop == true) {

		/* fill test file */
		tfp = tmps_open(TEST_FILE_NAME, "w", ftmp, sizeof(ftmp), strlen(C_FILE_EXT));
		if (tfp != NULL) {
			if (check_funcs == false) {
				/* header test */
				pmk_log("\tFound header '%s' : ", incfile);
				fprintf(tfp, INC_TEST_CODE, incfile);
			} else {
				/* header function test */
				incfunc = da_shift(funcs);
				target = incfunc;
				pmk_log("\tFound function '%s' in '%s' : ", incfunc, incfile);
				fprintf(tfp, INC_FUNC_TEST_CODE, incfile, incfunc);
			}

			fclose(tfp);
		} else {
			errorf("cannot open test file.");
			return(false);
		}

		/* build compiler command */
		if (incfunc == NULL) {
			snprintf(cfgcmd, sizeof(cfgcmd), HEADER_CC_FORMAT,
				ccpath, inc_path, BIN_TEST_NAME, ftmp, pgd->buildlog);
		} else {
			/* compute object file name */
			strlcpy(btmp, ftmp, sizeof(btmp));
			btmp[strlen(btmp) - 1] = 'o';

			snprintf(cfgcmd, sizeof(cfgcmd), HEADER_FUNC_CC_FORMAT,
				ccpath, inc_path, btmp, ftmp, pgd->buildlog);
		}

		/* get result */
		r = system(cfgcmd);
		if (r == 0) {
			pmk_log("yes.\n");
			/* define for template */
			record_def_data(pgd->htab, target, DEFINE_DEFAULT);
		} else {
			pmk_log("no.\n");
			record_def_data(pgd->htab, target, NULL);
			rslt = false;
		}

		if (unlink(ftmp) == -1) {
			/* cannot remove temporary file */
			errorf("cannot remove %s", ftmp);
		}

		if (incfunc == NULL) {
			/* No need to check return here as binary could not exists */
			unlink(BIN_TEST_NAME);
			/* exit from loop */
			loop = false;
		} else {
			/* No need to check return here as object file could not exists */
			unlink(btmp);
			/* check if list is empty */
			if (da_usize(funcs) == 0)
				loop = false;
		}
	}

	if (rslt == true) {
			label_set(pgd->labl, cmd->label, true);
			/* process additional defines */
			process_def_list(pgd->htab, defs);

			/* put result in CFLAGS, CXXFLAGS or alternative variable */
			if (single_append(pgd->htab, cflags, strdup(inc_path)) == false) {
				errorf("failed to append '%s' in '%s'.", inc_path, cflags);
				return(false);
			}

			rval = true;
	} else {
			rval = process_required(pgd, cmd, required, NULL, NULL);
			if (rval == false) {
				if (incfunc == NULL) {
					errorf("failed to find header '%s'.", incfile);
				} else {
					errorf("failed to find function '%s'.", incfunc);
				}
			}
	}

	if (clean_funcs == true) {
		da_destroy(funcs);
	}

	return(rval);
}

/*
	check library
*/

bool pmk_check_lib(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	FILE	*tfp;
	bool	 required,
		 rslt = true,
		 loop = true,
		 clean_funcs = false,
		 check_funcs = false,
		 rval;
	char	 cfgcmd[MAXPATHLEN],
		 lib_buf[TMP_BUF_LEN] = "",
		 ftmp[MAXPATHLEN],
		*main_libs,
		*ccpath,
		*libname,
		*libfunc = NULL,
		*target = NULL,
		*libs,
		*pstr;
	dynary	*funcs = NULL,
		*defs;
	int	 i,
		 n,
		 r;
	lgdata	*pld;
	pmkobj	*po;

	pmk_log("\n* Checking library [%s]\n", cmd->label);

	required = require_check(ht);

	libname = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (libname == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

	po = hash_get(ht, KW_OPT_FUNCTION);
	if (po == NULL) {
		target = libname;
	} else {
		/* KW_OPT_FUNCTION provided */
		switch (po_get_type(po)) {
			case PO_STRING:
				pstr = po_get_str(po);
				/* pstr should not be NULL */

				funcs = da_init();
				if (funcs == NULL) {
					errorf("unable to initialise dynary.");
					return(false);
				}

				if (da_push(funcs, pstr) == false) {
					errorf("unable to add '%s' in function list.", pstr);
					return(false);
				}

				clean_funcs = true;
				break;

			case PO_LIST:
				funcs = po_get_list(po);
				/* funcs should not be NULL */
				break;

			default:
				errorf("syntax error for '%s': bad type.", KW_OPT_FUNCTION);
				return(false);
		}
		check_funcs = true;
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		n = da_usize(funcs);
		for (i=0 ; i < n ; i++) {
			pstr = da_idx(funcs, i);
			record_def_data(pgd->htab, pstr, NULL);
		}
		return(process_required(pgd, cmd, required, NULL, NULL));
	}

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknown language.\n");
		return(invert_bool(required));
	}

	/* get the appropriate compiler */
	ccpath = get_comp_path(pgd->htab, pld->comp);
	if (ccpath == NULL) {
		pmk_log("\tSKIPPED, cannot get compiler path ('%s').\n", pld->comp);
		return(invert_bool(required));
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, KW_OPT_LIBS));
	if (libs != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, libs) == NULL) {
                        if (hash_update_dup(pgd->htab, libs, "") == HASH_ADD_FAIL) {
        			errorf(HASH_ERR_UPDT_ARG, libs);
        			return(false);
                        }
		}
	} else {
		/* use default library variable */
		libs = PMKVAL_ENV_LIBS;
	}
	pmk_log("\tStore library flags in '%s'.\n", libs);

	/* get actual content of LIBS, no need to check as it is initialised */
	main_libs = hash_get(pgd->htab, PMKVAL_ENV_LIBS);

	/* check loop */
	while (loop == true) {
		tfp = tmps_open(TEST_FILE_NAME, "w", ftmp, sizeof(ftmp), strlen(C_FILE_EXT));
		if (tfp != NULL) {
			if (check_funcs == false) {
				pmk_log("\tFound library '%s' : ", libname);
				fprintf(tfp, LIB_TEST_CODE);
			} else {
				libfunc = da_shift(funcs);
				target = libfunc;
				pmk_log("\tFound function '%s' in '%s' : ", libfunc, libname);
				fprintf(tfp, LIB_FUNC_TEST_CODE, libfunc, libfunc);
			}

			/* fill test file */
			fclose(tfp);
		} else {
			errorf("cannot open test file.");
			return(false);
		}

		/* build compiler command */
		snprintf(cfgcmd, sizeof(cfgcmd), LIB_CC_FORMAT,
			ccpath, main_libs, BIN_TEST_NAME, libname, ftmp, pgd->buildlog);
	
		/* get result */
		r = system(cfgcmd);
		if (r == 0) {
			pmk_log("yes.\n");
			/* define for template */
			record_def_data(pgd->htab, target, DEFINE_DEFAULT);
		} else {
			pmk_log("no.\n");
			record_def_data(pgd->htab, target, NULL);
			rslt = false;
		}

		if (unlink(ftmp) == -1) {
			/* cannot remove temporary file */
			errorf("cannot remove %s", ftmp);
		}

		if (libfunc == NULL) {
			/* No need to check return here as binary could not exists */
			unlink(BIN_TEST_NAME);
			/* exit from loop */
			loop = false;
		} else {
			/* check if list is empty */
			if (da_usize(funcs) == 0)
				loop = false;
		}
	}

	if (rslt == true) {
			label_set(pgd->labl, cmd->label, true);
			/* process additional defines */
			process_def_list(pgd->htab, defs);

			snprintf(lib_buf, sizeof(lib_buf), "-l%s", libname);

			/* put result in LIBS or alternative variable */
			if (single_append(pgd->htab, libs, strdup(lib_buf)) == false) {
				errorf("failed to append '%s' in '%s'.", lib_buf, libs);
				return(false);
			}

			rval = true;
	} else {
			rval = process_required(pgd, cmd, required, NULL, NULL);
			if (rval == false) {
				if (libfunc == NULL) {
					errorf("failed to find library '%s'.", libname);
				} else {
					errorf("failed to find function '%s'.", libfunc);
				}
			}
	}

	if (clean_funcs == true) {
		da_destroy(funcs);
	}

	return(rval);
}

/*
	check with *-config utility

*/

bool pmk_check_config(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	bool		 required = true,
			 rval = false;
	char		 pipebuf[TMP_BUF_LEN],
			 cfgpath[MAXPATHLEN],
			*cfgtool,
			*varname,
			*vtmp,
			*modname,
			*libvers,
			*bpath,
			*cflags,
			*libs,
			*opt;
	dynary		*defs;
	cfgtcell	*pcc = NULL;
	lgdata		*pld;

	pmk_log("\n* Checking with config tool [%s]\n", cmd->label);
	required = require_check(ht);

	cfgtool = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (cfgtool == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

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
		vtmp = str_to_def(cfgtool);
		if (vtmp == NULL) {
			errorf("VARIABLE not assigned in label '%s'.",
				cmd->label);
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

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknown language.\n");
		return(invert_bool(required));
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, PMKVAL_ENV_CFLAGS));
	if (cflags != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, cflags) == NULL) {
                        if (hash_update_dup(pgd->htab, cflags, "") == HASH_ADD_FAIL) {
        			errorf(HASH_ERR_UPDT_ARG, cflags);
        			return(false);
                        }
		}
	} else {
		/* use default variable of the used language */
		cflags = pld->cflg;
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, KW_OPT_LIBS));
	if (libs != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, libs) == NULL) {
                        if (hash_update_dup(pgd->htab, libs, "") == HASH_ADD_FAIL) {
        			errorf(HASH_ERR_UPDT_ARG, libs);
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
			if (hash_update_dup(pgd->htab, varname, "") == HASH_ADD_FAIL) {
				errorf(HASH_ERR_UPDT_ARG, varname);
				return(false);
			}
		}
		return(rval);
	} else {
		pmk_log("yes.\n");
		/* recording path of config tool under the key given by varname */
		if (hash_update_dup(pgd->htab, varname, cfgpath) == HASH_ADD_FAIL) {
			errorf(HASH_ERR_UPDT_ARG, varname);
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
	label_set(pgd->labl, cmd->label, true);
	/* process additional defines */
	process_def_list(pgd->htab, defs);

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

/*
	check pkg-config module using internal support
*/

bool pmk_check_pkg_config(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	bool	 required = true,
		 rval;
	char	*target,
		*pipebuf,
		*pstr,
		 pc_cmd[MAXPATHLEN],
		 pc_buf[MAXPATHLEN],
		*bpath,
		*pc_path,
		*libvers,
		*cflags,
		*libs;
	dynary	*defs;
	lgdata	*pld;
	pkgcell	*ppc;
	pkgdata	*ppd;

	pmk_log("\n* Checking pkg-config module [%s]\n", cmd->label);

	required = require_check(ht);

	target = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (target == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

	/* check dependencies */
	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, NULL, NULL)); /* XXX need to define something ? */
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

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknown language.\n");
		return(invert_bool(required));
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, PMKVAL_ENV_CFLAGS));
	if (cflags != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, cflags) == NULL) {
                        if (hash_update_dup(pgd->htab, cflags, "") == HASH_ADD_FAIL) {
        			errorf(HASH_ERR_UPDT_ARG, cflags);
        			return(false);
                        }
		}
	} else {
		/* use default variable of the used language */
		cflags = pld->cflg;
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, KW_OPT_LIBS));
	if (libs != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, libs) == NULL) {
                        if (hash_update_dup(pgd->htab, libs, "") == HASH_ADD_FAIL) {
        			errorf(HASH_ERR_UPDT_ARG, libs);
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
			if (snprintf(pc_cmd, sizeof(pc_cmd), "%s-config", target) >= sizeof(pc_cmd)) {
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

		return(process_required(pgd, cmd, required, NULL, NULL));
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
				rval = process_required(pgd, cmd, required, NULL, NULL);

				pkgdata_destroy(ppd);

				return(rval);
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

	/* process additional defines */
	process_def_list(pgd->htab, defs);

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

/*
	check type
*/

bool pmk_check_type(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	FILE	*tfp;
	bool	 required,
		 rval;
	char	 cfgcmd[MAXPATHLEN],
		 ftmp[MAXPATHLEN],
		*type,
		*header,
		*member,
		*ccpath;
	dynary	*defs;
	int	 r;
	lgdata	*pld;

	pmk_log("\n* Checking type [%s]\n", cmd->label);

	required = require_check(ht);

	type = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (type == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* check if an header must be used */
	header = po_get_str(hash_get(ht, KW_OPT_HEADER));

	/* check if a structure member is given */
	member = po_get_str(hash_get(ht, KW_OPT_MEMBER));

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, type, NULL));
	}

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknown language.\n");
		return(invert_bool(required));
	}

	/* get the appropriate compiler */
	ccpath = get_comp_path(pgd->htab, pld->comp);
	if (ccpath == NULL) {
		pmk_log("\tSKIPPED, cannot get compiler path ('%s').\n", pld->comp);
		return(invert_bool(required));
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));

	tfp = tmps_open(TEST_FILE_NAME, "w", ftmp, sizeof(ftmp), strlen(C_FILE_EXT));
	if (tfp != NULL) {
		if (header == NULL) {
			/* header provided */
			if (member == NULL) {
				fprintf(tfp, TYPE_TEST_CODE, type);
			} else {
				fprintf(tfp, TYPE_MEMBER_TEST_CODE, type, member);
			}
		} else {
			pmk_log("\tUse header '%s'.\n", header);
			if (member == NULL) {
				fprintf(tfp, TYPE_INC_TEST_CODE, header, type);
			} else {
				fprintf(tfp, TYPE_INC_MEMBER_TEST_CODE, header, type, member);
			}
		}

		if (member == NULL) {
			pmk_log("\tFound type '%s' : ", type);
		} else {
			pmk_log("\tFound member '%s' in '%s' : ", member, type);
		}
		fclose(tfp);
	}

	/* build compiler command */
	snprintf(cfgcmd, sizeof(cfgcmd), TYPE_CC_FORMAT,
		ccpath, BIN_TEST_NAME, ftmp, pgd->buildlog);

	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def_data(pgd->htab, type, DEFINE_DEFAULT);
		label_set(pgd->labl, cmd->label, true);
		/* process additional defines */
		process_def_list(pgd->htab, defs);

		rval = true;
	} else {
		pmk_log("no.\n");
		rval = process_required(pgd, cmd, required, type, NULL);
		if (rval == false) {
			errorf("failed to find type '%s'.", type);
		}
	}

	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("cannot remove %s", ftmp);
	}

	/* No need to check return here as binary could not exists */
	unlink(BIN_TEST_NAME);

	return(rval);
}

/*
	check if variable exists and optionally it's value
*/

bool pmk_check_variable(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
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

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		return(process_required(pgd, cmd, required, var, NULL));
	}

	/* look for additional defines */
	defs = po_get_list(hash_get(ht, KW_OPT_DEFS));

	pmk_log("\tFound variable '%s' : ", var);

	/* trying to get variable */
	varval = hash_get(pgd->htab, var);
	if (varval != NULL) {
		pmk_log("yes.\n");

		value = po_get_str(hash_get(ht, KW_OPT_VALUE));
		if (value == NULL) {
			label_set(pgd->labl, cmd->label, true);
			/* process additional defines */
			process_def_list(pgd->htab, defs);

			rval = true;
		} else {
			pmk_log("\tVariable match value '%s' : ", value);

			if (strncmp(value, varval, sizeof(varval)) == 0) {
				pmk_log("yes.\n");
				label_set(pgd->labl, cmd->label, true);

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


/*
	build name of a shared library
*/

bool pmk_build_shlib_name(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	char	*variable,
		*versvar,
		*versmaj,
		*pstr,
		*value;

	pmk_log("\n* Building shared library name\n");

	pmk_log("\tShared library support : ");

	if (pgd->slht == NULL) {
		pmk_log("no.\n");
		errorf("Your system is not supported yet.");
		return(false); /* XXX REQUIRED option ? */
	} else {
		pmk_log("yes.\n");
	}

	pstr = po_get_str(hash_get(ht, KW_OPT_NAME));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
debugf("pstr(name) = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
debugf("value = '%s'", value);
#endif
		hash_update(pgd->slht, SL_KW_LIB_NAME, value);
	}

	pstr = po_get_str(hash_get(ht, KW_OPT_MAJOR));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
debugf("pstr(major) = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
debugf("value = '%s'", value);
#endif
		hash_update(pgd->slht, SL_KW_LIB_MAJ, value); /* no dup */
	}

	pstr = po_get_str(hash_get(ht, KW_OPT_MINOR));
	if (pstr != NULL) {
#ifdef SHLIB_DEBUG
debugf("pstr(minor) = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->htab);
#ifdef SHLIB_DEBUG
debugf("value = '%s'", value);
#endif
		hash_update(pgd->slht, SL_KW_LIB_MIN, value); /* no dup */
	}

	/* get libname without version */
	variable = po_get_str(hash_get(ht, KW_SL_VERS_NONE));
	if (variable != NULL) {
		pstr = hash_get(pgd->slht, SL_KW_LIB_VNONE);
#ifdef SHLIB_DEBUG
debugf("pstr = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->slht);
#ifdef SHLIB_DEBUG
debugf("value = '%s'", value);
#endif

		/* no dup */
		if (hash_update(pgd->htab, variable, value) == HASH_ADD_FAIL) {
			errorf(HASH_ERR_UPDT_ARG, variable);
			return(false);
		}
		pmk_log("\tSetting %s to '%s'\n", variable, value);
#ifdef SHLIB_DEBUG
debugf("save in '%s'", variable);
#endif
	} else {
#ifdef SHLIB_DEBUG
debugf("variable not set");
#endif
	}

	versvar = po_get_str(hash_get(ht, KW_SL_VERS_FULL));
	if (versvar != NULL) {
		pstr = hash_get(pgd->slht, SL_KW_LIB_VFULL);
#ifdef SHLIB_DEBUG
debugf("pstr = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->slht);
#ifdef SHLIB_DEBUG
debugf("value = '%s'", value);
#endif
		if (hash_update(pgd->htab, versvar, value) == HASH_ADD_FAIL) {
			errorf(HASH_ERR_UPDT_ARG, versvar);
			return(false);
		}
		pmk_log("\tSetting %s to '%s'\n", versvar, value);
#ifdef SHLIB_DEBUG
debugf("save in '%s'", versvar);
#endif
	} else {
#ifdef SHLIB_DEBUG
debugf("versvar not set");
#endif
	}

	versmaj = po_get_str(hash_get(ht, KW_SL_VERS_MAJ));
	if (versmaj != NULL) {
		pstr = hash_get(pgd->slht, SL_KW_LIB_VMAJ);
#ifdef SHLIB_DEBUG
debugf("pstr = '%s'", pstr);
#endif
		value = process_string(pstr, pgd->slht);
#ifdef SHLIB_DEBUG
debugf("value = '%s'", value);
#endif
		if (hash_update(pgd->htab, versmaj, value) == HASH_ADD_FAIL) {
			errorf(HASH_ERR_UPDT_ARG, versmaj);
			return(false);
		}
		pmk_log("\tSetting %s to '%s'\n", versmaj, value);
#ifdef SHLIB_DEBUG
debugf("save in '%s'", versmaj);
#endif
	} else {
#ifdef SHLIB_DEBUG
debugf("versmaj not set");
#endif
	}

	return(true);
}


/*
	option functions
	
	these functions have the following parameters:

	cmd : command structure
	popt : option structure
	pgd : global data

	returns bool

*/


/*
	set parameter
*/

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

	/* set target files */
	if (strncmp(popt->key, KW_SETNGS_CCDTCT, sizeof(popt->key)) == 0) {
		return(pmk_setparam_detect(cmd, popt, pgd));
	}

	/* found unknown setting */
	pmk_log("\tunknown '%s' setting.\n", popt->key);
	return(false);
}

/*
	set accompat parameter
*/

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
		if (hash_update_dup(pgd->htab, AC_VAR_DEF, AC_VALUE_DEF) == HASH_ADD_FAIL) {
			errorf(HASH_ERR_UPDT_ARG, AC_VAR_DEF);
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

/*
	set global lang parameter
*/

bool pmk_setparam_glang(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	bool		 rval = false;
	char		*pstr;

	pmk_log("\tSetting global language :\n");

	/* set global language */
	pstr = po_get_str(popt->value);

	if (pstr != NULL) {
		/* check if provided lang is supported */
		if (check_lang(pstr) != NULL) {
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

/*
	set accompat parameter
*/

bool pmk_setparam_target(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	dynary		*da;
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

/*
	set accompat parameter
*/

bool pmk_setparam_detect(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	char		*pstr,
			*ostr,
			*ccpath,
			 buf[TMP_BUF_LEN];
	comp_cell	*pcell;
	comp_data	*cdata;
	comp_info	 cinfo;
	dynary		*da;
	int		 i = 0,
			 n;
	lgdata		*pld;

	pmk_log("\tDetecting compilers :\n");

	da = po_get_list(popt->value);
	if (da == NULL) {
		errorf("syntax error in DETECT.");
		return(false);
	}

	pmk_log("\t\tGathering data for compiler detection.\n");
	cdata = parse_comp_file_adv(PMKCOMP_DATA, pgd->htab);
	if (cdata == NULL) {
		return(false);
	}

	n = da_usize(da);
	for (i=0 ; i < n ; i++) {
		pstr = da_idx(da, i);
		pmk_log("\t\tDetecting '%s' : ", pstr);
		/* get the appropriate compiler */
		ccpath = get_comp_path(pgd->htab, pstr);
		if (ccpath == NULL) {
			errorf("\ncannot get compiler path ('%s').\n", pstr);
			return(false);
		} else {
			if (detect_compiler(ccpath, pgd->buildlog, cdata, &cinfo) == true) {
				pmk_log("%s (version %s).\n",
					comp_get_descr(cdata, cinfo.c_id), cinfo.version);

				/* set shared lib flags */
				pld = check_lang_comp(pstr);
				if (pld != NULL) {
					/* check if an override exists for compiler flags */
					if (snprintf(buf, sizeof(buf), "%s_%s", pld->slflg, cinfo.c_id) >= sizeof(buf)) {
						errorf("overflow.\n");
						return(false);
					}

					if (cdata->sht != NULL) { /* XXX need to skip if system has no data ? */
						ostr = hash_get(cdata->sht, buf);
					} else {
						ostr = NULL;
					}

					/* set shared lib compiler flags */
					if (ostr != NULL) {
						pmk_log("\tFound system specific %s.\n", pld->slflg);
					} else {
						pcell = comp_get(cdata, cinfo.c_id);
						ostr = pcell->slcflags;
					}

					pmk_log("\t\tSetting %s to '%s'\n", pld->slflg, ostr);
					if (hash_update_dup(pgd->htab, pld->slflg, ostr) == HASH_ADD_FAIL)
						return(false);

					/* check if an override exists for linking flags */
					if (snprintf(buf, sizeof(buf), "%s_%s", SL_LDFLAG_VARNAME, cinfo.c_id) >= sizeof(buf)) {
						errorf("overflow.\n");
						return(false);
					}

					if (cdata->sht != NULL) { /* XXX need to skip if system has no data ? */
						ostr = hash_get(cdata->sht, buf);
					} else {
						ostr = NULL;
					}

					/* set shared lib compiler flags */
					if (ostr != NULL) {
						pmk_log("\tFound system specific %s.\n", SL_LDFLAG_VARNAME);
					} else {
						pcell = comp_get(cdata, cinfo.c_id);
						ostr = pcell->slldflags;
					}

					/* set shared lib linking flags */
					pmk_log("\t\tSetting %s to '%s'\n", SL_LDFLAG_VARNAME, ostr);
					if (hash_update_dup(pgd->htab, SL_LDFLAG_VARNAME, ostr) == HASH_ADD_FAIL)
						return(false);
				} else {
					errorf("unable to set shared library compiler flags (%s).\n", pld->slflg);
					return(false);
				}
			} else {
				pmk_log("failed.\n");
				return(false);
			}
		}
	}

	/* move shared lib hash table into global structure */
	pgd->slht = cdata->sht;
	cdata->sht = NULL; /* avoid data destruction */

	/* clean cdata */
	compdata_destroy(cdata);

	return(true);
}

/*
	set variable
*/

bool pmk_set_variable(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	char	 buffer[TMP_BUF_LEN], /* XXX */
		*pstr,
		*value,
		*defname;
	int	 hval;

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
			hval = hash_update(pgd->htab, popt->key, value); /* no need to strdup */
			/* check return for message : defined or redefined */
			switch (hval) {
				case HASH_ADD_FAIL:
					errorf(HASH_ERR_UPDT_ARG, popt->key);
					return(false);
					break;

				case HASH_ADD_OKAY:
				case HASH_ADD_COLL:
					pmk_log("\tdefined");
					break;

				case HASH_ADD_UPDT:
					pmk_log("\tredefined");
					break;

			}
			/* remaining part of the message */
			pmk_log(" '%s' variable.\n", popt->key);

			/* store definition for autoconf compatibility */
			defname = build_def_name(popt->key);
			if (defname == NULL) {
				errorf("unable to build define name for '%s'.", popt->key);
				return(false);
			}
			if (snprintf(buffer, sizeof(buffer), "#define %s \"%s\"", popt->key, value) >= sizeof(buffer)) {
				errorf("buffer overflow for define value of '%s'.", popt->key);
				return(false);
			}
			if (hash_update_dup(pgd->htab, defname, buffer) == HASH_ADD_FAIL) {
				errorf(HASH_ERR_UPDT_ARG, defname);
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
