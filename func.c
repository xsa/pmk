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
#include "common.h"
#include "detect.h"
#include "func.h"
#include "functool.h"
#include "hash.h"
#include "pkgconfig.h"
#include "premake.h"


prskw	kw_pmkfile[] = {
		{"DEFINE",		PMK_TOK_DEFINE, PRS_KW_NODE, PMK_TOK_SETVAR},
		{"SETTINGS",		PMK_TOK_SETNGS, PRS_KW_NODE, PMK_TOK_SETPRM},
		{"IF",			PMK_TOK_IFCOND,	PRS_KW_NODE, PRS_TOK_NULL},
		{"SWITCHES",		PMK_TOK_SWITCH, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_BINARY",	PMK_TOK_CHKBIN, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_HEADER",	PMK_TOK_CHKINC, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_LIB",		PMK_TOK_CHKLIB, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_CONFIG",	PMK_TOK_CHKCFG, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_PKG_CONFIG",	PMK_TOK_CHKPKG, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_TYPE",		PMK_TOK_CHKTYP, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_VARIABLE",	PMK_TOK_CHKVAR, PRS_KW_CELL, PRS_TOK_NULL},
		{"CHECK_INCLUDE",	PMK_TOK_CHKINC, PRS_KW_CELL, PRS_TOK_NULL}
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
		pmk_log("\n< Begin of condition [%s]\n", cmd->label);
		process_node(pnode, pgd);
		pmk_log("\n> End of condition [%s]\n", cmd->label);
	} else {
		pmk_log("\n- Skipping condition [%s]\n", cmd->label);
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

bool pmk_check_binary(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	bool	 required;
	char	*filename,
		*varname,
		*bpath,
		 binpath[MAXPATHLEN];

	pmk_log("\n* Checking binary [%s]\n", cmd->label);
	required = require_check(ht);

	filename = po_get_str(hash_get(ht, "NAME"));
	if (filename == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	varname = po_get_str(hash_get(ht, "VARIABLE"));
	if (varname == NULL) {
		varname = str_to_def(filename);
	}

	bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
	if (bpath == NULL) {
		errorf("%s not available.", PMKCONF_PATH_BIN);
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(pgd->htab, filename, false);
			/* not found => not added in htab */ 
			label_set(pgd->labl, cmd->label, false);
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
			record_def(pgd->htab, filename, false);
			if (hash_update_dup(pgd->htab, varname, "") == HASH_ADD_FAIL) {
				errorf("hash error.");
				return(false);
			}
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
		/* define for template */
		record_def(pgd->htab, filename, true);
		record_val(pgd->htab, filename, "");
		if (hash_update_dup(pgd->htab, varname, binpath) == HASH_ADD_FAIL) {
			errorf("hash error.");
			return(false);
		}
		label_set(pgd->labl, cmd->label, true);
		return(true);
	}
}

/*
	check include file
*/

bool pmk_check_header(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	FILE	*tfp;
	bool	 required,
		 rval;
	char	 inc_path[MAXPATHLEN],
		 cfgcmd[MAXPATHLEN],
		 ftmp[MAXPATHLEN],
		 btmp[MAXPATHLEN],
		*incfile,
		*incfunc,
		*target,
		*ccpath,
		*cflags,
		*pstr;
	int	 r;
	lgdata	*pld;

	pmk_log("\n* Checking header [%s]\n", cmd->label);

	required = require_check(ht);

	/* get include filename */
	incfile = po_get_str(hash_get(ht, "NAME"));
	if (incfile == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* check if a function must be searched */
	incfunc = po_get_str(hash_get(ht, "FUNCTION"));
	if (incfunc == NULL) {
		target = incfile;
	} else {
		target = incfunc;
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(pgd->htab, target, false);
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	}

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknow language.\n");
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

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, cflags) == NULL) {
                        if (hash_update_dup(pgd->htab, cflags, "") == HASH_ADD_FAIL) {
        			errorf("hash error.");
        			return(false);
                        }
		}
	}

	/* use each element of INC_PATH with -I */
	pstr = (char *) hash_get(pgd->htab, PMKCONF_PATH_INC);
	if (pstr == NULL) {
		errorf("%s not available.", PMKCONF_PATH_INC);
		return(false);
	}

	if (get_file_dir_path(incfile, pstr, inc_path, sizeof(inc_path)) == true) {
		pstr = strdup(inc_path);
		strlcpy(inc_path, "-I", sizeof(inc_path));
		strlcat(inc_path, pstr, sizeof(inc_path));
	} else {
		/* include not found, init inc_path with "" */
		strlcpy(inc_path, "", sizeof(inc_path));
	}

	/* fill test file */
	tfp = tmps_open(TEST_FILE_NAME, "w", ftmp, sizeof(ftmp), strlen(C_FILE_EXT));
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

	/* build compiler command */
	if (incfunc == NULL) {
		snprintf(cfgcmd, sizeof(cfgcmd), HEADER_CC_FORMAT,
			ccpath, inc_path, BIN_TEST_NAME, ftmp, pgd->buildlog);
	} else {
		/* compute objet file name */
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
		record_def(pgd->htab, target, true);
		record_val(pgd->htab, target, "");
		label_set(pgd->labl, cmd->label, true);

		/* check for alternative variable for CFLAGS */
		if (cflags != NULL) {
			/* put result in special CFLAGS variable */
			if (single_append(pgd->htab, cflags, strdup(inc_path)) == false) {
				errorf("failed to append '%s' in '%s'.", inc_path, cflags);
				return(false);
			}
		} else {
			/* put result in CFLAGS */
			if (single_append(pgd->htab, "CFLAGS", strdup(inc_path)) == false) {
				errorf("failed to append '%s' in CFLAGS.", inc_path);
				return(false);
			}
		}

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
			record_def(pgd->htab, target, false);
			label_set(pgd->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("Can not remove %s", ftmp);
	}

	if (incfunc == NULL) {
		/* No need to check return here as binary could not exists */
		unlink(BIN_TEST_NAME);
	} else {
		/* No need to check return here as objet file could not exists */
		unlink(btmp);
	}

	return(rval);
}

/*
	check library
*/

bool pmk_check_lib(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	FILE	*tfp;
	bool	 required,
		 rval;
	char	 cfgcmd[MAXPATHLEN],
		 lib_buf[TMP_BUF_LEN] = "",
		 ftmp[MAXPATHLEN],
		*main_libs,
		*ccpath,
		*libname,
		*libfunc,
		*target,
		*libs;
	int	 r;
	lgdata	*pld;

	pmk_log("\n* Checking library [%s]\n", cmd->label);

	required = require_check(ht);

	libname = po_get_str(hash_get(ht, "NAME"));
	if (libname == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

	libfunc = po_get_str(hash_get(ht, "FUNCTION"));
	if (libfunc == NULL) {
		target = libname;
	} else {
		target = libfunc;
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(pgd->htab, target, false);
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, "LIBS"));
	if (libs != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, libs) == NULL) {
                        if (hash_update_dup(pgd->htab, libs, "") == HASH_ADD_FAIL) {
        			errorf("hash error.");
        			return(false);
                        }
		}
	}

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknow language.\n");
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

	/* get actual content of LIBS, no need to check as it is initialised */
	main_libs = hash_get(pgd->htab, "LIBS");

	tfp = tmps_open(TEST_FILE_NAME, "w", ftmp, sizeof(ftmp), strlen(C_FILE_EXT));
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

	/* build compiler command */
	snprintf(cfgcmd, sizeof(cfgcmd), LIB_CC_FORMAT,
		ccpath, main_libs, BIN_TEST_NAME, libname, ftmp, pgd->buildlog);

	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def(pgd->htab, target, true);
		record_val(pgd->htab, target, "");
		label_set(pgd->labl, cmd->label, true);

		snprintf(lib_buf, sizeof(lib_buf), "-l%s", libname);

		/* check for alternative variable for LIBS */
		if (libs != NULL) {
			/* put result in special LIBS variable */
			if (single_append(pgd->htab, libs, strdup(lib_buf)) == false) {
				errorf("failed to append '%s' in '%s'.", lib_buf, libs);
				return(false);
			}
		} else {
			/* put result in LIBS */
			if (single_append(pgd->htab, "LIBS", strdup(lib_buf)) == false) {
				errorf("failed to append '%s' in LIBS.", lib_buf);
				return(false);
			}
		}

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
			record_def(pgd->htab, target, false);
			label_set(pgd->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("Can not remove %s", ftmp);
	}

	/* No need to check return here as binary could not exists */
	unlink(BIN_TEST_NAME);

	return(rval);
}

/*
	check with *-config utility

	XXX add MODULE option ?
*/

bool pmk_check_config(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
	FILE		*rpipe;
	bool		 required = true,
			 rval = false;
	char		 pipebuf[TMP_BUF_LEN],
			 cfgpath[MAXPATHLEN],
			 cfgcmd[MAXPATHLEN],
			*cfgtool,
			*varname,
			*libvers,
			*bpath,
			*cflags,
			*libs,
			*opt;
	cfgtcell	*pcc = NULL;

	pmk_log("\n* Checking with config tool [%s]\n", cmd->label);
	required = require_check(ht);

	cfgtool = po_get_str(hash_get(ht, "NAME"));
	if (cfgtool == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

	varname = po_get_str(hash_get(ht, "VARIABLE"));
	if (varname == NULL) {
		varname = str_to_def(cfgtool);
	}

	bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
	if (bpath == NULL) {
		errorf("%s not available.", PMKCONF_PATH_BIN);
		return(false);
	}

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, cflags) == NULL) {
                        if (hash_update_dup(pgd->htab, cflags, "") == HASH_ADD_FAIL) {
        			errorf("hash error.");
        			return(false);
                        }
		}
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, "LIBS"));
	if (libs != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, libs) == NULL) {
                        if (hash_update_dup(pgd->htab, libs, "") == HASH_ADD_FAIL) {
        			errorf("hash error.");
        			return(false);
                        }
		}
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(pgd->htab, cfgtool, false);
			label_set(pgd->labl, cmd->label, false);
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
			record_def(pgd->htab, cfgtool, false);
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
		/* recording path of config tool */
		if (hash_update_dup(pgd->htab, varname, cfgpath) == HASH_ADD_FAIL) {
			errorf("hash error.");
			return(false);
		}
	}

	/* check if specific tool option exists */
	pmk_log("\tUsing specific options : ");
	pcc = cfgtcell_get_cell(pgd, cfgtool);
	if (pcc != NULL) {
		pmk_log("yes\n");
	} else {
		pmk_log("no\n");
	}

	libvers = po_get_str(hash_get(ht, "VERSION"));

	/* if VERSION is provided then check it */
	if (libvers != NULL) {
		/* check if specific option exists */
		if ((pcc != NULL) && (pcc->version != NULL)) {
			opt = pcc->version;
		} else {
			opt = CFGTOOL_OPT_VERSION;
		}

		snprintf(cfgcmd, sizeof(cfgcmd), "%s %s 2>/dev/null", cfgpath, opt);

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
					record_def(pgd->htab, cfgtool, false);
					label_set(pgd->labl, cmd->label, false);
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
	record_def(pgd->htab, cfgtool, true);
	record_val(pgd->htab, cfgtool, "");
	label_set(pgd->labl, cmd->label, true);

	/* check if specific option exists */
	if ((pcc != NULL) && (pcc->cflags != NULL)) {
		opt = pcc->cflags;
	} else {
		opt = CFGTOOL_OPT_CFLAGS;
	}

	snprintf(cfgcmd, sizeof(cfgcmd), "%s %s 2>/dev/null", cfgpath, opt);

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
			if (single_append(pgd->htab, cflags, strdup(pipebuf)) == false) {
				errorf("failed to append '%s' in '%s'.", pipebuf, cflags);
				return(false);
			}
		} else {
			/* put result in CFLAGS */
			if (single_append(pgd->htab, "CFLAGS", strdup(pipebuf)) == false) {
				errorf("failed to append '%s' in CFLAGS.", pipebuf);
				return(false);
			}
		}
	}

	/* check if specific option exists */
	if ((pcc != NULL) && (pcc->libs != NULL)) {
		opt = pcc->libs;
	} else {
		opt = CFGTOOL_OPT_LIBS;
	}

	snprintf(cfgcmd, sizeof(cfgcmd), "%s %s 2>/dev/null", cfgpath, opt);

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
			if (single_append(pgd->htab, libs, strdup(pipebuf)) == false) {
				errorf("failed to append '%s' in '%s'.", pipebuf, libs);
				return(false);
			}
		} else {
			/* put result in LIBS */
			if (single_append(pgd->htab, "LIBS", strdup(pipebuf)) == false) {
				errorf("failed to append '%s' in LIBS.", pipebuf);
				return(false);
			}
		}
	}

	/* XXX LDFLAGS, CPPFLAGS ? */

	return(true);
}

/* comment the following define to disable experimental pkgconfig support */
#define EXPERIMENTAL_PKGCONFIG	1

#ifndef EXPERIMENTAL_PKGCONFIG
/*
	special check with pkg-config utility

	XXX could store pkg-config path to improve speed
		(avoiding multiple checks for pkg-config path)

*/
#else
/*
	check pkg-config module using internal support
*/
#endif

bool pmk_check_pkg_config(pmkcmd *cmd, htable *ht, pmkdata *pgd) {
#ifndef EXPERIMENTAL_PKGCONFIG
	FILE	*rpipe;
#endif
	bool	 required = true,
		 rval;
	char	*target,
#ifndef EXPERIMENTAL_PKGCONFIG
		 pipebuf[TMP_BUF_LEN],
		*pc_path = NULL,
#else
		*pipebuf,
#endif
		 pc_cmd[MAXPATHLEN],
		 pc_buf[MAXPATHLEN],
		*bpath,
		*libvers,
		*cflags,
		*libs;
#ifdef EXPERIMENTAL_PKGCONFIG
	pkgcell	*ppc;
	pkgdata	*ppd;
#endif

#ifndef EXPERIMENTAL_PKGCONFIG
	pmk_log("\n* Checking with pkg-config [%s]\n", cmd->label);
#else
	pmk_log("\n* Checking pkg-config module [%s]\n", cmd->label);
#endif
	required = require_check(ht);

	target = po_get_str(hash_get(ht, "NAME"));
	if (target == NULL) {
		errorf("NAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

#ifndef EXPERIMENTAL_PKGCONFIG
	/* try to get pkg-config path from pmk.conf setting */
	pc_path = (char *) hash_get(pgd->htab, PMKCONF_BIN_PKGCONFIG);

	/* nothing in pmk.conf, hope it is obsolete
		and look for it in the binary path */
	if (pc_path == NULL) {
		/* get binary path */
		bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
		if (bpath == NULL) {
		        errorf("%s not available.", PMKCONF_PATH_BIN);
			return(false);
		}

		/* looking for it in the path */
		if (get_file_path(PMKVAL_BIN_PKGCONFIG, bpath, pc_buf, sizeof(pc_buf)) == true) {
			pc_path = pc_buf;
		}
	}
#endif

	/* check for alternative variable for CFLAGS */
	cflags = po_get_str(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, cflags) == NULL) {
                        if (hash_update_dup(pgd->htab, cflags, "") == HASH_ADD_FAIL) {
        			errorf("hash error.");
        			return(false);
                        }
		}
	}

	/* check for alternative variable for LIBS */
	libs = po_get_str(hash_get(ht, "LIBS"));
	if (libs != NULL) {
		/* init alternative variable */
                if (hash_get(pgd->htab, libs) == NULL) {
                        if (hash_update_dup(pgd->htab, libs, "") == HASH_ADD_FAIL) {
        			errorf("hash error.");
        			return(false);
                        }
		}
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			/* record_def(pgd->htab, target, false); XXX how to manage ? */
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	}

#ifndef EXPERIMENTAL_PKGCONFIG
	/* check availability of pkg-config */
	pmk_log("\tFound pkg-config : ");
	if (pc_path == NULL) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			/* record_def(pgd->htab, target, false); XXX how to manage ? */
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
	}
#else
	ppd = pkgdata_init();
	if (ppd == NULL) {
		errorf("cannot init pkgdata.");
		return(false);
	}

	scan_dir(PKGCONFIG_DIR, ppd); /* XXX check */
#endif

	/* check if package exists */
	pmk_log("\tFound package '%s' : ", target);
#ifndef EXPERIMENTAL_PKGCONFIG
	snprintf(pc_cmd, sizeof(pc_cmd), "%s --exists %s 2>/dev/null", pc_path, target);
	if (system(pc_cmd) != 0) {
#else
	if (pkg_mod_exists(ppd, target) == false) { 
		pkgdata_destroy(ppd);
#endif
		pmk_log("no.\n");

#ifdef EXPERIMENTAL_PKGCONFIG
		/* get binary path */
		bpath = hash_get(pgd->htab, PMKCONF_PATH_BIN);
		if (bpath == NULL) {
		        errorf("%s not available.", PMKCONF_PATH_BIN);
			return(false);
		}

		/* set config tool filename */
		if (cfgtcell_get_binary(pgd, target, pc_cmd, sizeof(pc_cmd)) == false) {
			snprintf(pc_cmd, sizeof(pc_cmd), "%s-config", target); /* XXX check */
		}

		/* looking for it in the path */
		if (get_file_path(pc_cmd, bpath, pc_buf, sizeof(pc_buf)) == true) {
			/* use CHECK_CONFIG */
			pmk_log("\tFound alternative '%s' tool.\n", pc_cmd);

			/* override NAME to use divert on CHECK_CONFIG */
			hash_update(ht, "NAME", po_mk_str(pc_cmd));
			pmk_log("\tWARNING: rerouting to CHECK_CONFIG\n");
			pmk_log("\tPlease consider using directly CHECK_CONFIG with '%s'\n", pc_cmd);

			/* call pmk_check_config with the config tool */
			return(pmk_check_config(cmd, ht, pgd));
		}
#endif
		if (required == true) {
			return(false);
		} else {
			/* record_def(pgd->htab, target, false); XXX how to manage ? */
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
	}

	libvers = po_get_str(hash_get(ht, "VERSION"));
	if (libvers != NULL) {
		/* if VERSION is provided then check it */
#ifndef EXPERIMENTAL_PKGCONFIG
		snprintf(pc_cmd, sizeof(pc_cmd), "%s --modversion %s 2>/dev/null", pc_path, target);

		rpipe = popen(pc_cmd, "r");
		if (rpipe == NULL) {
#else
		ppc = pkg_cell_add(ppd, target);
		if (ppc == NULL) {
			pkgdata_destroy(ppd);
#endif
			errorf("cannot get version.");
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
#ifndef EXPERIMENTAL_PKGCONFIG
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get version from '%s'.", pc_cmd);
				pclose(rpipe);
				return(false);
			}
			pclose(rpipe);
#else
			pipebuf = ppc->version;
#endif
			if (check_version(libvers, pipebuf) != true) {
				/* version does not match */
				pmk_log("no (%s).\n", pipebuf);
				if (required == true) {
					rval = false;
				} else {
					/* record_def(pgd->htab, target, false); XXX how to manage ? */
					label_set(pgd->labl, cmd->label, false);
					rval = true;
				}
#ifdef EXPERIMENTAL_PKGCONFIG
				pkgdata_destroy(ppd);
#endif
				return(rval);
			} else {
				pmk_log("yes (%s).\n", pipebuf);
			}
		}
	}

	/* gather data */
#ifndef EXPERIMENTAL_PKGCONFIG
	snprintf(pc_cmd, sizeof(pc_cmd), "%s --cflags %s", pc_path, target);
	rpipe = popen(pc_cmd, "r");
	if (rpipe != NULL) {
		if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get CFLAGS.");
			pclose(rpipe);
			return(false);
		}
		pclose(rpipe);
#else
	if (pkg_recurse(ppd, target) == false) {
		pkgdata_destroy(ppd);
		errorf("failed on recurse !"); /* XXX TODO better message :) */
		return(false);
	}

	/* get cflags recursively */
	pipebuf = pkg_get_cflags(ppd);
	if (pipebuf == NULL) {
		pkgdata_destroy(ppd);
		errorf("cannot get CFLAGS.");
		return(false);
	} else {
#endif

		/* check for alternative variable for CFLAGS */
		if (cflags != NULL) {
			/* put result in special CFLAGS variable */
#ifndef EXPERIMENTAL_PKGCONFIG
			if (single_append(pgd->htab, cflags, strdup(pipebuf)) == false) {
#else
			if (single_append(pgd->htab, cflags, pipebuf) == false) {
				pkgdata_destroy(ppd);
#endif
				errorf("failed to append '%s' in '%s'.", pipebuf, cflags);
				return(false);
			}
		} else {
			/* put result in CFLAGS */
#ifndef EXPERIMENTAL_PKGCONFIG
			if (single_append(pgd->htab, "CFLAGS", strdup(pipebuf)) == false) {
#else
			if (single_append(pgd->htab, "CFLAGS", pipebuf) == false) {
				pkgdata_destroy(ppd);
#endif
				errorf("failed to append '%s' in CFLAGS.", pipebuf);
				return(false);
			}
		}
	}
#ifndef EXPERIMENTAL_PKGCONFIG
#else
#endif

#ifndef EXPERIMENTAL_PKGCONFIG
	snprintf(pc_cmd, sizeof(pc_cmd), "%s --libs %s", pc_path, target);
	rpipe = popen(pc_cmd, "r");
	if (rpipe != NULL) {
		if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
			errorf("cannot get LIBS.");
			pclose(rpipe);
			return(false);
		}
		pclose(rpipe);
#else
	/* get cflags recursively */
	pipebuf = pkg_get_libs(ppd);
	if (pipebuf == NULL) {
		pkgdata_destroy(ppd);
		errorf("cannot get LIBS.");
		return(false);
	} else {
#endif

		/* check for alternative variable for LIBS */
		if (libs != NULL) {
			/* put result in special LIBS variable */
#ifndef EXPERIMENTAL_PKGCONFIG
			if (single_append(pgd->htab, libs, strdup(pipebuf)) == false) {
#else
			if (single_append(pgd->htab, libs, pipebuf) == false) {
				pkgdata_destroy(ppd);
#endif
				errorf("failed to append '%s' in '%s'.", pipebuf, libs);
				return(false);
			}
		} else {
			/* put result in LIBS */
#ifndef EXPERIMENTAL_PKGCONFIG
			if (single_append(pgd->htab, "LIBS", strdup(pipebuf)) == false) {
#else
			if (single_append(pgd->htab, "LIBS", pipebuf) == false) {
				pkgdata_destroy(ppd);
#endif
				errorf("failed to append '%s' in LIBS.", pipebuf);
				return(false);
			}
		}
	}

	/* XXX LDFLAGS, CPPFLAGS ? */

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
	int	 r;
	lgdata	*pld;

	pmk_log("\n* Checking type [%s]\n", cmd->label);

	required = require_check(ht);

	type = po_get_str(hash_get(ht, "NAME"));
	if (type == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* check if an header must be used */
	header = po_get_str(hash_get(ht, "HEADER"));

	/* check if a structure member is given */
	member = po_get_str(hash_get(ht, "MEMBER"));

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(pgd->htab, type, false);
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	}

	/* get the language used */
	pld = get_lang(ht, pgd);
	if (pld == NULL) {
		pmk_log("\tSKIPPED, unknow language.\n");
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
		record_def(pgd->htab, type, true);
		record_val(pgd->htab, type, "");
		label_set(pgd->labl, cmd->label, true);
		rval = true;
	} else {
		pmk_log("no.\n");
		if (required == true) {
			rval = false;
			errorf("failed to find type '%s'.", type);
		} else {
			/* define for template */
			record_def(pgd->htab, type, false);
			label_set(pgd->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("Can not remove %s", ftmp);
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

	pmk_log("\n* Checking variable [%s]\n", cmd->label);

	required = require_check(ht);

	var = po_get_str(hash_get(ht, "NAME"));
	if (var == NULL) {
		errorf("NAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	if (depend_check(ht, pgd) == false) {
		pmk_log("\t%s\n", pgd->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(pgd->htab, var, false);
			label_set(pgd->labl, cmd->label, false);
			return(true);
		}
	}

	pmk_log("\tFound variable '%s' : ", var);

	/* trying to get variable */
	varval = hash_get(pgd->htab, var);
	if (varval != NULL) {
		pmk_log("yes.\n");

		value = po_get_str(hash_get(ht, "VALUE"));
		if (value == NULL) {
			label_set(pgd->labl, cmd->label, true);

			rval = true;
		} else {
			pmk_log("\tVariable match value '%s' : ", value);

			if (strncmp(value, varval, sizeof(varval)) == 0) {
				pmk_log("yes.\n");
				label_set(pgd->labl, cmd->label, true);

				rval = true;
			} else {
				pmk_log("no.\n");
				if (required == true) {
					rval = false;
					errorf("variable value does not match ('%s' ! '%s').", value, varval);
				} else {
					/* define for template */
					label_set(pgd->labl, cmd->label, false);
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
			label_set(pgd->labl, cmd->label, false);
			rval = true;
		}
	}

	return(rval);
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
	bool		 rval = false;
	char		*pstr,
			*ccpath;
	comp_cell	*pcell;
	comp_data	*cdata;
	comp_info	 cinfo;
	dynary		*da;
	int		 i = 0,
			 n;
	lgdata		*pld;

	/* gnu autoconf compatibility */
	if (strncmp(popt->key, "AC_COMPAT", sizeof(popt->key)) == 0) {
		pmk_log("\tSetting autoconf compatibility :\n");

		/* XXX must check if valid
			pstr = (char *) hash_get(pgd->htab, "SYSCONFDIR");
			hash_update_dup(pgd->htab, "sysconfdir", pstr);
		*/

		/* if a file is given then it will be parsed later */
		pstr = po_get_str(popt->value);
		if (*pstr != CHAR_EOS) {
			pgd->ac_file = strdup(pstr);
			pmk_log("\t\tSet file to '%s'.\n", pstr);
			if (hash_update_dup(pgd->htab, AC_VAR_DEF, AC_VALUE_DEF) == HASH_ADD_FAIL) {
				errorf("failed to add value for '%s' in hash table.", AC_VAR_DEF);
				return(false);
			}
			pmk_log("\t\tSet '%s' value to '%s'.\n", AC_VAR_DEF, AC_VALUE_DEF);
		}

		/* compatibility tags */
		ac_set_variables(pgd->htab);
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
				pgd->lang = strdup(pstr);
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

		pgd->tlist = da;

		pmk_log("\t\tTotal %d target(s) added.\n", n);

		return(true);
	}

	/* set target files */
	if (strncmp(popt->key, "DETECT", sizeof(popt->key)) == 0) {
		pmk_log("\tDetecting compilers :\n");

		da = po_get_list(popt->value);
		if (da == NULL) {
			errorf("syntax error in DETECT.");
			return(false);
		}

		pmk_log("\t\tGathering data for compiler detection.\n");
		cdata = parse_comp_file(PMKCOMP_DATA);
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
						pcell = comp_get(cdata, cinfo.c_id);
						pmk_log("\t\tSetting %s to '%s'\n", pld->slflg, pcell->slflags);
						hash_update_dup(pgd->htab, pld->slflg, pcell->slflags); /* XXX check */
					} else {
						errorf("unable to set shared library compiler flags (%s).\n", pld->slflg);
						return(false);
					}
				}

			}
		}

		/* clean cdata */
		compdata_destroy(cdata);

		return(true);
	}

	/* found unknown setting */
	pmk_log("\tunknown '%s' setting.\n", popt->key);
	return(false);
}

/*
	set parameter
*/

bool pmk_set_variable(pmkcmd *cmd, prsopt *popt, pmkdata *pgd) {
	char	*value;
	int	 hval;

/* XXX better way to do (specific hash for override)
	if (hash_get(pgd->htab, popt->key) == NULL) {
*/
		/* process value string */
		value = process_string(po_get_str(popt->value), pgd->htab);
		if (value != NULL) {
			hval = hash_update(pgd->htab, popt->key, value); /* no need to strdup */
			/* check return for message : defined or redefined */
			switch (hval) {
				case HASH_ADD_FAIL:
					errorf("failed to set '%s' variable.");
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

