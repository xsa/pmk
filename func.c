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
#include "common.h"
#include "func.h"
#include "functool.h"
#include "hash.h"
#include "pmk.h"
#include "premake.h"


cmdkw	functab[] = {
		{"DEFINE", pmk_define},
		{"TARGET", pmk_target},
		{"AC_COMPAT", pmk_ac_compat},
		{"CHECK_BINARY", pmk_check_binary},
		{"CHECK_INCLUDE", pmk_check_include},
		{"CHECK_LIB", pmk_check_lib},
		{"CHECK_CONFIG", pmk_check_config},
		{"CHECK_PKG_CONFIG", pmk_check_pkg_config},
		{"CHECK_TYPE", pmk_check_type}
};

int	nbfunc = sizeof(functab) / sizeof(cmdkw);


/*
	all the following functions have the same parameters :

	cmd : command structure
	ht : command options
	gdata : global data

	returns bool

	XXX need to check return value of record_def, record_val and label_set ?
*/

/*
	define variables
*/

bool pmk_define(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	int	n;

	pmk_log("* Parsing define\n");

	n = hash_merge(gdata->htab, ht);
	pmk_log("\tAdded %d definitions.\n", n);
	
	return(true);
}

/*
	set target files (templates) to process
*/

bool pmk_target(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*list;
	dynary	*da;
	int	 i = 0;

	pmk_log("* Collecting targets\n");

	list = strdup((char *)po_get_data(hash_get(ht, "LIST")));
	if (list == NULL) {
		errorf("LIST not assigned in TARGET.");
		return(false);
	}

	da = da_init();
	if (da == NULL) {
		/* XXX */
		errorf("cannot initialize dynary.");
		return(false);
	}
	if (str_to_dynary(list, CHAR_LIST_SEPARATOR, da) == false) {
		errorf("cannot set dynary in TARGET");
		return(false);
	}
	
	for (i=0 ; i < da_usize(da) ; i++) {
		/* da_idx should not returns null so no check */
		pmk_log("\tAdded '%s'.\n", da_idx(da, i));
	}

	gdata->tlist = da;

	return(true);
}

/*
	gnu autoconf compatibility
*/

bool pmk_ac_compat(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*acfile,
		*pstr;

/* XXX must check if valid
	pstr = (char *) hash_get(gdata->htab, "SYSCONFDIR");
	hash_add(gdata->htab, "sysconfdir", strdup(pstr));
*/

	/* if a file is given then it will be parsed later */
	acfile= (char *)po_get_data(hash_get(ht, "FILENAME"));
	if (acfile != NULL) {
		gdata->ac_file = strdup(acfile);
	}

	/* compatibility tags */
	pstr = (char *) hash_get(gdata->htab, "PREFIX");
	hash_add(gdata->htab, "prefix", strdup(pstr));
	
	hash_add(gdata->htab, "exec_prefix", strdup("${prefix}"));
	hash_add(gdata->htab, "bindir", strdup("${exec_prefix}/bin"));
	hash_add(gdata->htab, "sbindir", strdup("${exec_prefix}/sbin"));
	hash_add(gdata->htab, "libexecdir", strdup("${exec_prefix}/libexec"));
	hash_add(gdata->htab, "libdir", strdup("${exec_prefix}/lib"));
	hash_add(gdata->htab, "datadir", strdup("${prefix}/share"));
	hash_add(gdata->htab, "includedir", strdup("${prefix}/include"));
	hash_add(gdata->htab, "mandir", strdup("${prefix}/man"));
	hash_add(gdata->htab, "infodir", strdup("${prefix}/info"));

	pstr = (char *) hash_get(gdata->htab, "BIN_INSTALL");
	hash_add(gdata->htab, "INSTALL", strdup(pstr));

	return(true);
}

/*
	check binary
*/

bool pmk_check_binary(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*filename,
		*bpath,
		 binpath[MAXPATHLEN];
	bool	 required;

	pmk_log("* Checking binary [%s]\n", cmd->label);
	required = require_check(ht);

	filename = (char *)po_get_data(hash_get(ht, "FILENAME"));
	if (filename == NULL) {
		errorf("FILENAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	bpath = (char *) hash_get(gdata->htab, "BIN_PATH");
	if (bpath == NULL) {
		errorf("BIN_PATH not available.");
		return(false);
	}

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			record_def(gdata->htab, filename, false);
			hash_add(gdata->htab, str_to_def(filename), strdup("")); /* XXX check ? */
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
			hash_add(gdata->htab, str_to_def(filename), strdup("")); /* XXX check ? */
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, filename, true);
		record_val(gdata->htab, filename, "");
		hash_add(gdata->htab, str_to_def(filename), strdup(binpath)); /* XXX check ? */
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

	pmk_log("* Checking include [%s]\n", cmd->label);

	required = require_check(ht);

	/* get include filename */
	incfile = (char *)po_get_data(hash_get(ht, "INCLUDE"));
	if (incfile == NULL) {
		errorf("INCLUDE not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* check if a function must be searched */
	incfunc = (char *)po_get_data(hash_get(ht, "FUNCTION"));
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

	/* get the appropriate compiler */
	snprintf(cfgcmd, sizeof(cfgcmd), "BIN_%s", pld->comp);
	ccpath = (char *) hash_get(gdata->htab, cfgcmd);
	if (ccpath == NULL) {
		errorf("cannot get compiler path.");
		return(false);
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
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

	/* use each element of INC_PATH with -I */
	pstr = (char *) hash_get(gdata->htab, "INC_PATH");
	if (pstr == NULL) {
		strlcpy(inc_path, "", sizeof(inc_path));
	} else {
		da = da_init();
		r = sizeof(inc_path);
		str_to_dynary(pstr, CHAR_LIST_SEPARATOR, da);
		for (i=0 ; i < da_usize(da) ; i++) {
			strlcat(inc_path, " -I", r);
			strlcat(inc_path, da_idx(da, i), r);
		}
		da_destroy(da);
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

	pmk_log("* Checking library [%s]\n", cmd->label);

	required = require_check(ht);

	libname = (char *)po_get_data(hash_get(ht, "LIBNAME"));
	if (libname == NULL) {
		errorf("LIBNAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

	libfunc = (char *)po_get_data(hash_get(ht, "FUNCTION"));
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

	/* get the appropriate compiler */
	snprintf(cfgcmd, sizeof(cfgcmd), "BIN_%s", pld->comp);
	ccpath = (char *) hash_get(gdata->htab, cfgcmd);
	if (ccpath == NULL) {
		errorf("cannot get compiler path.");
		return(false);
	} else {
		pmk_log("\tUse %s language with %s compiler.\n", pld->name, pld->comp);
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

	/* use each element of LIB_PATH with -L */
	pstr = (char *) hash_get(gdata->htab, "LIB_PATH");
	if (pstr == NULL) {
		strlcpy(lib_buf, "", sizeof(lib_buf));
	} else {
		da = da_init();
		r = sizeof(lib_buf);
		str_to_dynary(pstr, CHAR_LIST_SEPARATOR, da);
		for (i=0 ; i < da_usize(da) ; i++) {
			strlcat(lib_buf, " -L", r);
			strlcat(lib_buf, da_idx(da, i), r);
		}
		da_destroy(da);
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
		hash_append(gdata->htab, "LIBS", strdup(lib_buf), " ");

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

	pmk_log("* Checking with config tool [%s]\n", cmd->label);

	required = require_check(ht);

	cfgtool = (char *)po_get_data(hash_get(ht, "CFGTOOL"));
	if (cfgtool == NULL) {
		errorf("CFGTOOL not assigned in label '%s'.", cmd->label);
		return(false);
	}

	bpath = (char *) hash_get(gdata->htab, "BIN_PATH");
	if (bpath == NULL) {
		errorf("BIN_PATH not available.");
		return(false);
	}

	/* check for alternative variable for CFLAGS */
	cflags = (char *)po_get_data(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
		hash_append(gdata->htab, cflags, strdup(""), " "); /* XXX check ? */
	}

	/* check for alternative variable for LIBS */
	libs = (char *)po_get_data(hash_get(ht, "LIBS"));
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
	
	libvers = (char *)po_get_data(hash_get(ht, "VERSION"));
	if (libvers != NULL) {
		/* if VERSION is provided then check it */
		snprintf(cfgcmd, sizeof(cfgcmd), "%s --version 2>/dev/null", cfgpath);

		rpipe = popen(cfgcmd, "r");
		if (rpipe == NULL) {
			errorf("cannot get version from '%s'.", cfgcmd); /* XXX should correct this message ? */
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
			get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
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
		get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
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
		get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
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

	XXX should add a switch to use global LIBS & CFLAGS or local
	definitions, example :
		gtk => GTK_LIBS & GTK_CFLAGS
*/

bool pmk_check_pkg_config(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*rpipe;
	bool	 required = true,
		 rval;
	char	 pipebuf[TMP_BUF_LEN],
		 pc_cmd[MAXPATHLEN],
		 pc_path[MAXPATHLEN],
		*target,
		*bpath,
		*libvers,
		*cflags,
		*libs;

	pmk_log("* Checking with pkg-config [%s]\n", cmd->label);

	required = require_check(ht);

	target = (char *)po_get_data(hash_get(ht, "PACKAGE"));
	if (target == NULL) {
		errorf("no TARGET set");
		return(false);
	}

	bpath = (char *) hash_get(gdata->htab, "BIN_PATH");
	if (bpath == NULL) {
		errorf("BIN_PATH not available.");
		return(false);
	}

	/* check for alternative variable for CFLAGS */
	cflags = (char *)po_get_data(hash_get(ht, "CFLAGS"));
	if (cflags != NULL) {
		/* init alternative variable */
		hash_append(gdata->htab, cflags, strdup(""), " "); /* XXX check ? */
	}

	/* check for alternative variable for LIBS */
	libs = (char *)po_get_data(hash_get(ht, "LIBS"));
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
	if (get_file_path("pkg-config", bpath, pc_path, sizeof(pc_path)) == false) {
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

	libvers = (char *)po_get_data(hash_get(ht, "VERSION"));
	if (libvers != NULL) {
		/* if VERSION is provided then check it */
		snprintf(pc_cmd, sizeof(pc_cmd), "%s --modversion %s 2>/dev/null", pc_path, target);

		rpipe = popen(pc_cmd, "r");
		if (rpipe == NULL) {
			errorf("cannot get version from '%s'.", pc_cmd); /* XXX should correct this message ? */
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
			get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
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
		get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
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
		get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
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

	pmk_log("* Checking type [%s]\n", cmd->label);

	required = require_check(ht);

	type = (char *)po_get_data(hash_get(ht, "TYPE"));
	if (type == NULL) {
		errorf("TYPE not assigned in label '%s'", cmd->label);
		return(false);
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

	/* get the appropriate compiler */
	snprintf(cfgcmd, sizeof(cfgcmd), "BIN_%s", pld->comp);
	ccpath = (char *) hash_get(gdata->htab, cfgcmd);
	if (ccpath == NULL) {
		errorf("cannot get compiler path.");
		return(false);
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
