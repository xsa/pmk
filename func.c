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
#include <unistd.h>

#include "compat/pmk_string.h"
#include "autoconf.h"
#include "common.h"
#include "func.h"
#include "functool.h"
#include "hash.h"
#include "premake.h"


prskw	kw_pmkfile[] = {
/*		XXX for compatibility */
		{".DEFINE",		PMK_TOK_DEFINE, PRS_KW_NODE, PMK_TOK_SETVAR},
		{".AC_COMPAT",		PMK_TOK_ACCOMP, PRS_KW_CELL, PRS_TOK_NULL},
		{".SWITCHES",		PMK_TOK_SWITCH, PRS_KW_CELL, PRS_TOK_NULL}, /* XXX will be node */
		{".TARGET",		PMK_TOK_TARGET, PRS_KW_CELL, PRS_TOK_NULL},
		{".CHECK_BINARY",	PMK_TOK_CHKBIN, PRS_KW_CELL, PRS_TOK_NULL},
		{".CHECK_INCLUDE",	PMK_TOK_CHKINC, PRS_KW_CELL, PRS_TOK_NULL},
		{".CHECK_LIB",		PMK_TOK_CHKLIB, PRS_KW_CELL, PRS_TOK_NULL},
		{".CHECK_CONFIG",	PMK_TOK_CHKCFG, PRS_KW_CELL, PRS_TOK_NULL},
		{".CHECK_PKG_CONFIG",	PMK_TOK_CHKPKG, PRS_KW_CELL, PRS_TOK_NULL},
		{".CHECK_TYPE",		PMK_TOK_CHKTYP, PRS_KW_CELL, PRS_TOK_NULL},
/*		XXX new format */
		{"DEFINE",		PMK_TOK_DEFINE, PRS_KW_NODE, PMK_TOK_SETVAR},
		{"SETTINGS",		PMK_TOK_SETNGS, PRS_KW_NODE, PMK_TOK_SETPRM},
		{"IF",			PMK_TOK_IFCOND,	PRS_KW_NODE, PRS_TOK_NULL},
		{"SWITCHES",		PMK_TOK_SWITCH, PRS_KW_CELL, PRS_TOK_NULL}, /* XXX will be node */
		{"CHECK_BINARY",	PMK_TOK_CHKBIN, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_HEADER",	PMK_TOK_CHKINC, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_LIB",		PMK_TOK_CHKLIB, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_CONFIG",	PMK_TOK_CHKCFG, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_PKG_CONFIG",	PMK_TOK_CHKPKG, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_TYPE",		PMK_TOK_CHKTYP, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_VARIABLE",	PMK_TOK_CHKVAR, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_INCLUDE",	PMK_TOK_CHKINC, PRS_KW_CELL, PRS_TOK_NULL}
};

int	nbkwpf = sizeof(kw_pmkfile) / sizeof(prskw);


bool func_wrapper(prscell *pcell, pmkdata *pgd) {
	bool	rval;
	pmkcmd	cmd;

	cmd.token = pcell->token;
	cmd.label = pcell->label;

	switch (cmd.token) {
		case PMK_TOK_DEFINE :
			rval = pmk_define(&cmd, pcell->data, pgd);
			break;
		case PMK_TOK_TARGET :
			rval = pmk_target(&cmd, pcell->data, pgd); /* OBSOLETE */
			break;
		case PMK_TOK_ACCOMP :
			rval = pmk_ac_compat(&cmd, pcell->data, pgd); /* OBSOLETE */
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
			rval = pmk_check_include(&cmd, pcell->data, pgd);
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
		default :
			errorf("Unknow token %d", cmd.token);
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
	all the following functions have the same parameters : <== XXXX eeeek not true

	cmd : command structure
	ht : command options
	gdata : global data

	returns bool

*/


/*
	define variables

	XXX TODO process variables in order (and also search in global htable).
*/

bool pmk_define(pmkcmd *cmd, prsnode *pnode, pmkdata *gdata) {
	pmk_log("\n* Parsing define\n");

	return(process_node(pnode, gdata));
}

/*
	set target files (templates) to process
	NOTE : now OBSOLETE with new format, kept for compatibility
*/

bool pmk_target(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	pmkobj	*po;
	prsopt	 opt;

	pmk_log("\n* Parsing target\n");
	pmk_log("\tWARNING : TARGET command is obsolete, consider using SETTINGS.\n");

	strlcpy(opt.key, "TARGET", sizeof(opt.key));

	/* check if TARGET has been provided */
	po = hash_get(ht, "LIST");
	if (po != NULL) {
		opt.value = po;
	} else {
		errorf("syntax error in LIST.");
		return(false);
	}

	return(pmk_set_parameter(cmd, &opt, gdata));
}

/*
	gnu autoconf compatibility
	NOTE : now OBSOLETE with new format, kept for compatibility
*/

bool pmk_ac_compat(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*acfile;
	prsopt	 opt;

	pmk_log("\n* Parsing ac_compat\n");
	pmk_log("\tWARNING : AC_COMPAT command is obsolete, consider using SETTINGS.\n");

	strlcpy(opt.key, "AC_COMPAT", sizeof(opt.key));

	/* check if FILENAME has been provided */
	acfile= po_get_str(hash_get(ht, "FILENAME"));
	if (acfile != NULL) {
		opt.value = po_mk_str(acfile);
	} else {
		opt.value = po_mk_str("");
	}

	return(pmk_set_parameter(cmd, &opt, gdata));
}

/*
	settings
*/

bool pmk_settings(pmkcmd *cmd, prsnode *pnode, pmkdata *gdata) {
	pmk_log("\n* Parsing settings\n");

	return(process_node(pnode, gdata));
}

/*
	if condition
*/

bool pmk_ifcond(pmkcmd *cmd, prsnode *pnode, pmkdata *gdata) {
	if (label_check(gdata->labl, cmd->label) == true) {
		pmk_log("\n< Begin of condition [%s]\n", cmd->label);
		process_node(pnode, gdata);
		pmk_log("\n> End of condition [%s]\n", cmd->label);
	} else {
		pmk_log("\n- Skipping condition [%s]\n", cmd->label);
	}

	return(true);
}

/*
	switches
*/

bool pmk_switches(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*value;
	hkeys	*phk;
	int	 i,
		 n = 0;
	pmkobj	*po;

	pmk_log("\n* Parsing switches\n");

	phk = hash_keys(ht);

	for(i = 0 ; i < phk->nkey ; i++) {
		if (hash_get(gdata->labl, phk->keys[i]) == NULL) {
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

			if (hash_add(gdata->labl, phk->keys[i], strdup(value)) != HASH_ADD_FAIL) {
				pmk_log("\tAdded '%s' switch.\n", phk->keys[i]);
				n++;
			} else {
				errorf("hash add failed.");
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

bool pmk_check_binary(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	bool	 required;
	char	*filename,
		*varname,
		*bpath,
		 binpath[MAXPATHLEN];

	pmk_log("\n* Checking binary [%s]\n", cmd->label);
	required = require_check(ht);

	filename = po_get_str(hash_get(ht, "NAME"));
	if (filename == NULL) {
		filename = po_get_str(hash_get(ht, "FILENAME")); /* OBSOLETE */
		if (filename == NULL) {
			errorf("NAME not assigned in label '%s'", cmd->label);
			return(false);
		}
	}

	varname = po_get_str(hash_get(ht, "VARIABLE"));
	if (varname == NULL) {
		varname = po_get_str(hash_get(ht, "VARNAME")); /* OBSOLETE */
		if (varname == NULL) {
			varname = str_to_def(filename);
		}
	}

	bpath = hash_get(gdata->htab, PMKCONF_PATH_BIN);
	if (bpath == NULL) {
		/* OBSOLETE, check for BIN_PATH for compatibility with 6.0 */
		bpath = hash_get(gdata->htab, PREMAKE_KEY_BINPATH);
		if (bpath == NULL) {
			errorf("BIN_PATH not available.");
			return(false);
		}
		pmk_log("\tWARNING : BIN_PATH is obsolete, update pmk.conf with pmksetup.\n");
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, filename, false);
			if (hash_add(gdata->htab, varname, strdup("")) == HASH_ADD_FAIL) {
				errorf("hash error.");
				return(false);
			}
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	/* try to locate binary */
	pmk_log("\tFound binary '%s' : ", filename);
	if (get_file_path(filename, bpath, binpath, sizeof(binpath)) == false) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			/* define for template */
			record_def(gdata->htab, filename, false);
			if (hash_add(gdata->htab, varname, strdup("")) == HASH_ADD_FAIL) {
				errorf("hash error.");
				return(false);
			}
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, filename, true);
		record_val(gdata->htab, filename, "");
		if (hash_add(gdata->htab, varname, strdup(binpath)) == HASH_ADD_FAIL) {
			errorf("hash error.");
			return(false);
		}
		label_set(gdata->labl, cmd->label, true);
		return(true);
	}
}

/*
	check include file
*/

bool pmk_check_include(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*tfp;
	bool	 required,
		 rval;
	char	 inc_path[TMP_BUF_LEN] = "",
		 cfgcmd[MAXPATHLEN],
		*incfile,
		*incfunc,
		*target,
		*ccpath,
		*pstr;
	dynary	*da;
	int	 r,
		 i;
	lgdata	*pld;

	pmk_log("\n* Checking header [%s]\n", cmd->label);

	required = require_check(ht);

	/* get include filename */
	incfile = po_get_str(hash_get(ht, "NAME"));
	if (incfile == NULL) {
		incfile = po_get_str(hash_get(ht, "INCLUDE")); /* OBSOLETE */
		if (incfile == NULL) {
			errorf("NAME not assigned in label '%s'", cmd->label);
			return(false);
		}
	}

	/* check if a function must be searched */
	incfunc = po_get_str(hash_get(ht, "FUNCTION"));
	if (incfunc == NULL) {
		target = incfile;
	} else {
		target = incfunc;
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, target, false);
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	/* get the language used */
	pld = get_lang(ht, gdata);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknow language.\n");
		return(invert_bool(required));
	}

	/* get the appropriate compiler */
	ccpath = get_comp_path(gdata->htab, pld->comp);
	if (ccpath == NULL) {
		pmk_log("\tSKIPPED, cannot get compiler path ('%s').\n", pld->comp);
		return(invert_bool(required));
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
	}

	/* use each element of INC_PATH with -I */
	pstr = (char *) hash_get(gdata->htab, PMKCONF_PATH_INC);
	if (pstr == NULL) {
		/* OBSOLETE, check for BIN_PATH for compatibility with 6.0 */
		pstr = (char *) hash_get(gdata->htab, PREMAKE_KEY_INCPATH);
		if (pstr == NULL) {
			strlcpy(inc_path, "", sizeof(inc_path));
		} else {
			pmk_log("\tWARNING : INC_PATH is obsolete, update pmk.conf with pmksetup.\n");
		}
	} else {
		r = sizeof(inc_path);
		da = str_to_dynary(pstr, CHAR_LIST_SEPARATOR);
		if (da == NULL) {
			errorf("ARG XXX TODO");  /* XXX TODO PATH CHECK HERE */
			return(false);
		}

		for (i=0 ; i < da_usize(da) ; i++) {
			strlcat(inc_path, " -I", r); /* XXX check */
			strlcat(inc_path, da_idx(da, i), r);
		}
		da_destroy(da);
	}

	/* fill test file */
	tfp = fopen(TEST_FILE_NAME, "w");
	if (tfp != NULL) {
		if (incfunc == NULL) {
			/* header test */
			pmk_log("\tFound header '%s' : ", incfile);
			fprintf(tfp, INC_TEST_CODE, incfile);
		} else {
			/* header function test */
			pmk_log("\tFound function '%s' in '%s' : ", incfunc, incfile);
			fprintf(tfp, INC_FUNC_TEST_CODE, incfile, incfunc);
		}

		fclose(tfp);
	} else {
		errorf("cannot open test file.");
		return(false);
	}

	snprintf(cfgcmd, sizeof(cfgcmd), "%s %s -o %s %s > /dev/null 2>&1",
						ccpath, inc_path,
						BIN_TEST_NAME, TEST_FILE_NAME);
	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, target, true);
		record_val(gdata->htab, target, "");
		label_set(gdata->labl, cmd->label, true);
		rval = true;
	} else {
		pmk_log("no.\n");
		if (required == true) {
			rval = false;
			if (incfunc == NULL) {
				errorf("failed to find header '%s'.", incfile);
			} else {
				errorf("failed to find function '%s'.", incfunc);
			}
		} else {
			/* define for template */
			record_def(gdata->htab, target, false);
			label_set(gdata->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(TEST_FILE_NAME) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", TEST_FILE_NAME);
	}

	/* No need to check return here as binary could not exists */
	unlink(BIN_TEST_NAME);

	return(rval);
}

/*
	check library
*/

bool pmk_check_lib(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*tfp;
	bool	 required,
		 rval;
	char	 cfgcmd[MAXPATHLEN],
		 lib_buf[TMP_BUF_LEN] = "",
		*ccpath,
		*libname,
		*libfunc,
		*target,
		*pstr;
	dynary	*da;
	int	 r,
		 i;
	lgdata	*pld;

	pmk_log("\n* Checking library [%s]\n", cmd->label);

	required = require_check(ht);

	libname = po_get_str(hash_get(ht, "NAME"));
	if (libname == NULL) {
		libname = po_get_str(hash_get(ht, "LIBNAME")); /* OBSOLETE */
		if (libname == NULL) {
			errorf("NAME not assigned in label '%s'.", cmd->label);
			return(false);
		}
	}

	libfunc = po_get_str(hash_get(ht, "FUNCTION"));
	if (libfunc == NULL) {
		target = libname;
	} else {
		target = libfunc;
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, target, false);
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	/* get the language used */
	pld = get_lang(ht, gdata);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknow language.\n");
		return(invert_bool(required));
	}

	/* get the appropriate compiler */
	ccpath = get_comp_path(gdata->htab, pld->comp);
	if (ccpath == NULL) {
		pmk_log("\tSKIPPED, cannot get compiler path ('%s').\n", pld->comp);
		return(invert_bool(required));
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
	}

	/* use each element of LIB_PATH with -L */
	pstr = (char *) hash_get(gdata->htab, PMKCONF_PATH_LIB);
	if (pstr == NULL) {
		/* OBSOLETE, check for BIN_PATH for compatibility with 6.0 */
		pstr = (char *) hash_get(gdata->htab, PREMAKE_KEY_LIBPATH);
		if (pstr == NULL) {
			strlcpy(lib_buf, "", sizeof(lib_buf));
		} else {
			pmk_log("\tWARNING : LIB_PATH is obsolete, update pmk.conf with pmksetup.\n");
		}
	} else {
		r = sizeof(lib_buf);
		da = str_to_dynary(pstr, CHAR_LIST_SEPARATOR); /* XXX TODO PATH */
		if (da == NULL) {
			errorf("ARG XXX TODO");  /* XXX TODO PATH CHECK HERE */
			return(false);
		}

		for (i=0 ; i < da_usize(da) ; i++) {
			strlcat(lib_buf, " -L", r);
			strlcat(lib_buf, da_idx(da, i), r);
		}
		da_destroy(da);
	}

	tfp = fopen(TEST_FILE_NAME, "w");
	if (tfp != NULL) {
		if (libfunc == NULL) {
			pmk_log("\tFound library '%s' : ", libname);
			fprintf(tfp, LIB_TEST_CODE);
		} else {
			pmk_log("\tFound function '%s' in '%s' : ", libfunc, libname);
			fprintf(tfp, LIB_FUNC_TEST_CODE, libfunc, libfunc);
		}

		/* fill test file */
		fclose(tfp);
	} else {
		errorf("cannot open test file.");
		return(false);
	}

	snprintf(cfgcmd, sizeof(cfgcmd), "%s %s -o %s -l%s %s >/dev/null 2>&1",
						ccpath, lib_buf, BIN_TEST_NAME,
						libname, TEST_FILE_NAME);
	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, target, true);
		record_val(gdata->htab, target, "");
		label_set(gdata->labl, cmd->label, true);

		snprintf(lib_buf, sizeof(lib_buf), "-l%s", libname);
		hash_append(gdata->htab, "LIBS", strdup(lib_buf), " "); /* XXX check */

		rval = true;
	} else {
		pmk_log("no.\n");
		if (required == true) {
			rval = false;
			if (libfunc == NULL) {
				errorf("failed to find library '%s'.", libname);
			} else {
				errorf("failed to find function '%s'.", libfunc);
			}
		} else {
			/* define for template */
			record_def(gdata->htab, target, false);
			label_set(gdata->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(TEST_FILE_NAME) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", TEST_FILE_NAME);
	}

	/* No need to check return here as binary could not exists */
	unlink(BIN_TEST_NAME);

	return(rval);
}

/*
	check with *-config utility
*/

bool pmk_check_config(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*rpipe;
	bool	 required = true,
		 rval = false;
	char	 pipebuf[TMP_BUF_LEN],
		 cfgpath[MAXPATHLEN],
		 cfgcmd[MAXPATHLEN],
		*cfgtool,
		*libvers,
		*bpath,
		*cflags,
		*libs;

	pmk_log("\n* Checking with config tool [%s]\n", cmd->label);

	required = require_check(ht);

	cfgtool = po_get_str(hash_get(ht, "NAME"));
	if (cfgtool == NULL) {
		cfgtool = po_get_str(hash_get(ht, "CFGTOOL")); /* OBSOLETE */
		if (cfgtool == NULL) {
			errorf("NAME not assigned in label '%s'.", cmd->label);
			return(false);
		}
	}

	bpath = hash_get(gdata->htab, PMKCONF_PATH_BIN);
	if (bpath == NULL) {
		/* OBSOLETE, check for BIN_PATH for compatibility with 6.0 */
		bpath = hash_get(gdata->htab, PREMAKE_KEY_BINPATH);
		if (bpath == NULL) {
			errorf("BIN_PATH not available.");
			return(false);
		}
		pmk_log("\tWARNING : BIN_PATH is obsolete, update pmk.conf with pmksetup.\n");
	}

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
		if (hash_append(gdata->htab, cflags, strdup(""), " ") == HASH_ADD_FAIL) {
			errorf("hash error.");
			return(false);
		}
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, "LIBS"));
	if (libs != NULL) {
		/* init alternative variable */
		hash_append(gdata->htab, libs, strdup(""), " "); /* XXX check ? */
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, cfgtool, false);
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	/* try to locate cfgtool */
	pmk_log("\tFound config tool '%s' : ", cfgtool);
	if (get_file_path(cfgtool, bpath, cfgpath, sizeof(cfgpath)) == false) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, cfgtool, false);
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
	}
	
	libvers = po_get_str(hash_get(ht, "VERSION"));
	if (libvers != NULL) {
		/* if VERSION is provided then check it */
		snprintf(cfgcmd, sizeof(cfgcmd), "%s --version 2>/dev/null", cfgpath);

		rpipe = popen(cfgcmd, "r");
		if (rpipe == NULL) {
			errorf("cannot get version from '%s'.", cfgtool);
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get version from '%s'.", cfgcmd);
				return(false);
			}
			pclose(rpipe);
			if (check_version(libvers, pipebuf) != true) {
				/* version does not match */
				pmk_log("no (%s).\n", pipebuf);
				if (required == true) {
					rval = false;
				} else {
					record_def(gdata->htab, cfgtool, false);
					label_set(gdata->labl, cmd->label, false);
					rval = true;
				}
				return(rval);
			} else {
				pmk_log("yes (%s).\n", pipebuf);
			}
		}
	}

	/* gather data */
	/* XXX do we keep cfgtool for the tag ? */
	record_def(gdata->htab, cfgtool, true);
	record_val(gdata->htab, cfgtool, "");
	label_set(gdata->labl, cmd->label, true);

	snprintf(cfgcmd, sizeof(cfgcmd), "%s --cflags", cfgpath);
	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* put result in CFLAGS */
		if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get CFLAGS.");
			pclose(rpipe);
			return(false);
		}
		pclose(rpipe);

		/* check for alternative variable for CFLAGS */
		if (cflags != NULL) {
			/* put result in special CFLAGS variable */
			hash_append(gdata->htab, cflags, strdup(pipebuf), " "); /* XXX check ? */
		} else {
			/* put result in CFLAGS */
			hash_append(gdata->htab, "CFLAGS", strdup(pipebuf), " "); /* XXX check ? */
		}
	}

	snprintf(cfgcmd, sizeof(cfgcmd), "%s --libs", cfgpath);
	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* put result in LIBS */
		if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get LIBS");
			pclose(rpipe);
			return(false);
		}
		pclose(rpipe);

		/* check for alternative variable for LIBS */
		if (libs != NULL) {
			/* put result in special LIBS variable */
			hash_append(gdata->htab, libs, strdup(pipebuf), " "); /* XXX check ? */
		} else {
			/* put result in LIBS */
			hash_append(gdata->htab, "LIBS", strdup(pipebuf), " "); /* XXX check ? */
		}
	}

	/* XXX LDFLAGS, CPPFLAGS ? */

	return(true);
}

/*
	special check with pkg-config utility

	XXX could store pkg-config path to improve speed
		(avoiding multiple checks for pkg-config path)

*/

bool pmk_check_pkg_config(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*rpipe;
	bool	 required = true,
		 rval;
	char	 pipebuf[TMP_BUF_LEN],
		 pc_cmd[MAXPATHLEN],
		 pc_buf[MAXPATHLEN],
		*target,
		*bpath,
		*pc_path = NULL,
		*libvers,
		*cflags,
		*libs;

	pmk_log("\n* Checking with pkg-config [%s]\n", cmd->label);

	required = require_check(ht);

	target = po_get_str(hash_get(ht, "NAME"));
	if (target == NULL) {
		target = po_get_str(hash_get(ht, "PACKAGE")); /* OBSOLETE */
		if (target == NULL) {
			errorf("NAME not assigned in label '%s'.", cmd->label);
			return(false);
		}
	}

	/* try to get pkg-config path from pmk.conf setting */
	pc_path = (char *) hash_get(gdata->htab, PMKCONF_BIN_PKGCONFIG);
	if (pc_path == NULL) {
		/* nothing in pmksetup hope pmk.conf is obsolete
			and check in the path */
		bpath = hash_get(gdata->htab, PMKCONF_PATH_BIN);
		if (bpath == NULL) {
			/* OBSOLETE, check for BIN_PATH for compatibility with 6.0 */
			bpath = hash_get(gdata->htab, PREMAKE_KEY_BINPATH);
			if (bpath == NULL) {
				errorf("BIN_PATH not available.");
				return(false);
			}
			pmk_log("\tWARNING : BIN_PATH is obsolete, update pmk.conf with pmksetup.\n");
		}
		/* XXX ugly here, should clean ... */
		if (get_file_path("pkg-config", bpath, pc_buf, sizeof(pc_buf)) == true) {
			pc_path = pc_buf;
		}
	}

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
		hash_append(gdata->htab, cflags, strdup(""), " "); /* XXX check ? */
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, "LIBS"));
	if (libs != NULL) {
		/* init alternative variable */
		hash_append(gdata->htab, libs, strdup(""), " "); /* XXX check ? */
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			/* record_def(gdata->htab, target, false); XXX how to manage ? */
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	/* check availability of pkg-config */
	pmk_log("\tFound pkg-config : ");
	if (pc_path == NULL) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			/* record_def(gdata->htab, target, false); XXX how to manage ? */
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
	}

	/* check if package exists */
	pmk_log("\tFound package '%s' : ", target);
	snprintf(pc_cmd, sizeof(pc_cmd), "%s --exists %s 2>/dev/null", pc_path, target);
	if (system(pc_cmd) != 0) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			/* record_def(gdata->htab, target, false); XXX how to manage ? */
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
	}

	libvers = po_get_str(hash_get(ht, "VERSION"));
	if (libvers != NULL) {
		/* if VERSION is provided then check it */
		snprintf(pc_cmd, sizeof(pc_cmd), "%s --modversion %s 2>/dev/null", pc_path, target);

		rpipe = popen(pc_cmd, "r");
		if (rpipe == NULL) {
			errorf("cannot get version from '%s'.", pc_cmd);
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get version from '%s'.", pc_cmd);
				pclose(rpipe);
				return(false);
			}
			pclose(rpipe);
			if (check_version(libvers, pipebuf) != true) {
				/* version does not match */
				pmk_log("no (%s).\n", pipebuf);
				if (required == true) {
					rval = false;
				} else {
					/* record_def(gdata->htab, target, false); XXX how to manage ? */
					label_set(gdata->labl, cmd->label, false);
					rval = true;
				}
				return(rval);
			} else {
				pmk_log("yes (%s).\n", pipebuf);
			}
		}
	}

	/* gather data */
	snprintf(pc_cmd, sizeof(pc_cmd), "%s --cflags %s", pc_path, target);
	rpipe = popen(pc_cmd, "r");
	if (rpipe != NULL) {
		if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get CFLAGS.");
			pclose(rpipe);
			return(false);
		}
		pclose(rpipe);

		/* check for alternative variable for CFLAGS */
		if (cflags != NULL) {
			/* put result in special CFLAGS variable */
			hash_append(gdata->htab, cflags, strdup(pipebuf), " "); /* XXX check ? */
		} else {
			/* put result in CFLAGS */
			hash_append(gdata->htab, "CFLAGS", strdup(pipebuf), " "); /* XXX check ? */
		}
	}

	snprintf(pc_cmd, sizeof(pc_cmd), "%s --libs %s", pc_path, target);
	rpipe = popen(pc_cmd, "r");
	if (rpipe != NULL) {
		if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get LIBS.");
			pclose(rpipe);
			return(false);
		}
		pclose(rpipe);

		/* check for alternative variable for LIBS */
		if (libs != NULL) {
			/* put result in special LIBS variable */
			hash_append(gdata->htab, libs, strdup(pipebuf), " "); /* XXX check ? */
		} else {
			/* put result in LIBS */
			hash_append(gdata->htab, "LIBS", strdup(pipebuf), " "); /* XXX check ? */
		}
	}

	/* XXX LDFLAGS, CPPFLAGS ? */

	return(true);
}

/*
	check type
*/

bool pmk_check_type(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*tfp;
	bool	 required,
		 rval;
	char	 cfgcmd[MAXPATHLEN],
		*type,
		*ccpath;
	int	 r;
	lgdata	*pld;

	pmk_log("\n* Checking type [%s]\n", cmd->label);

	required = require_check(ht);

	type = po_get_str(hash_get(ht, "NAME"));
	if (type == NULL) {
		type = po_get_str(hash_get(ht, "TYPE")); /* OBSOLETE */
		if (type == NULL) {
			errorf("NAME not assigned in label '%s'", cmd->label);
			return(false);
		}
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, type, false);
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	/* get the language used */
	pld = get_lang(ht, gdata);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknow language.\n");
		return(invert_bool(required));
	}

	/* get the appropriate compiler */
	ccpath = get_comp_path(gdata->htab, pld->comp);
	if (ccpath == NULL) {
		pmk_log("\tSKIPPED, cannot get compiler path ('%s').\n", pld->comp);
		return(invert_bool(required));
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
	}

	tfp = fopen(TEST_FILE_NAME, "w");
	if (tfp != NULL) {
		pmk_log("\tFound type '%s' : ", type);
		fprintf(tfp, TYPE_TEST_CODE, type, type);
		fclose(tfp);
	}

	snprintf(cfgcmd, sizeof(cfgcmd), "%s -o %s %s > /dev/null 2>&1",
						ccpath, BIN_TEST_NAME,
						TEST_FILE_NAME);

	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, type, true);
		record_val(gdata->htab, type, "");
		label_set(gdata->labl, cmd->label, true);
		rval = true;
	} else {
		pmk_log("no.\n");
		if (required == true) {
			rval = false;
			errorf("failed to find type '%s'.", type);
		} else {
			/* define for template */
			record_def(gdata->htab, type, false);
			label_set(gdata->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(TEST_FILE_NAME) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", TEST_FILE_NAME);
	}

	/* No need to check return here as binary could not exists */
	unlink(BIN_TEST_NAME);

	return(rval);
}

/*
	check if variable exists and optionally it's value
*/

bool pmk_check_variable(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	bool	 required,
		 rval = false;
	char	*var,
		*value,
		*varval;

	pmk_log("\n* Checking variable [%s]\n", cmd->label);

	required = require_check(ht);

	var = po_get_str(hash_get(ht, "NAME"));
	if (var == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, var, false);
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	}

	pmk_log("\tFound variable '%s' : ", var);

	/* trying to get variable */
	varval = hash_get(gdata->htab, var);
	if (varval != NULL) {
		pmk_log("yes.\n");

		value = po_get_str(hash_get(ht, "VALUE"));
		if (value == NULL) {
			record_val(gdata->htab, var, "");
			rval = true;
		} else {
			pmk_log("\tVariable match value '%s' : ", value);

			if (strncmp(value, varval, sizeof(varval)) == 0) {
				pmk_log("yes.\n");
				record_val(gdata->htab, var, "");
				rval = true;
			} else {
				pmk_log("no.\n");
				if (required == true) {
					rval = false;
					errorf("variable value does not match ('%s' ! '%s').", value, varval);
				} else {
					/* define for template */
					label_set(gdata->labl, cmd->label, false);
					rval = true;
				}
			}
		}
	} else {
		pmk_log("no.\n");
		if (required == true) {
			errorf("failed to find variable '%s'.", var);
		} else {
			/* define for template */
			label_set(gdata->labl, cmd->label, false);
			rval = true;
		}
	}

	record_def(gdata->htab, var, rval);
	label_set(gdata->labl, cmd->label, rval);

	return(rval);
}

/*
	set variable
*/

bool pmk_set_parameter(pmkcmd *cmd, prsopt *popt, pmkdata *gdata) {
	bool	 rval = false;
	char	*pstr;
	dynary	*da;
	int	 i = 0,
		 n;

	/* gnu autoconf compatibility */
	if (strncmp(popt->key, "AC_COMPAT", sizeof(popt->key)) == 0) {
		pmk_log("\tSetting autoconf compatibility :\n");

		/* XXX must check if valid
			pstr = (char *) hash_get(gdata->htab, "SYSCONFDIR");
			hash_add(gdata->htab, "sysconfdir", strdup(pstr));
		*/

		/* if a file is given then it will be parsed later */
		pstr = po_get_str(popt->value);
		if (*pstr != CHAR_EOS) {
			gdata->ac_file = strdup(pstr);
			pmk_log("\t\tSet file to '%s'.\n", pstr);
			hash_add(gdata->htab, AC_VAR_DEF, strdup(AC_VALUE_DEF)); /* XXX TODO check */
			pmk_log("\t\tSet '%s' value to '%s'.\n", AC_VAR_DEF, AC_VALUE_DEF);
		}

		/* compatibility tags */
		ac_set_variables(gdata->htab);
		pmk_log("\t\tSet specific variables.\n");

		return(true);
	}

	/* global language */
	if (strncmp(popt->key, "LANG", sizeof(popt->key)) == 0) {
		pmk_log("\tSetting global language :\n");

		/* set global language */
		pstr = po_get_str(popt->value);

		if (pstr != NULL) {
			/* check if provided lang is supported */
			if (check_lang(pstr) != NULL) {
				gdata->lang = strdup(pstr);
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

	/* set target files */
	if (strncmp(popt->key, "TARGET", sizeof(popt->key)) == 0) {
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

		gdata->tlist = da;

		pmk_log("\t\tTotal %d target(s) added.\n", n);

		return(true);
	}

	/* found unknown setting */
	pmk_log("\tunknown '%s' setting.\n", popt->key);
	return(false);
}

/*
	set parameter
*/

bool pmk_set_variable(pmkcmd *cmd, prsopt *popt, pmkdata *gdata) {
	char	*value;

	if (hash_get(gdata->htab, popt->key) == NULL) {
		/* process value string */
		value = process_string(po_get_str(popt->value), gdata->htab);
		if (value != NULL) {
			hash_add(gdata->htab, popt->key, value); /* no need to strdup */
			pmk_log("\tAdded '%s' variable.\n", popt->key);
		} else {
			pmk_log("\tFailed processing of '%s'.\n", popt->key);
			return(false);
		}
	} else {
		pmk_log("\tSkipped '%s' define (overriden).\n", popt->key);
	}
	return(true);
}

