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
#include <string.h>
#include <unistd.h>

#include "func.h"


cmdkw	functab[] = {
		{"DEFINE", pmk_define},
		{"TARGET", pmk_target},
		{"AC_COMPAT", pmk_ac_compat},
		{"CHECK_BINARY", pmk_check_binary},
		{"CHECK_INCLUDE", pmk_check_include},
		{"CHECK_LIB", pmk_check_lib},
		{"CHECK_CONFIG", pmk_check_config}
};

int	nbfunc = sizeof(functab) / sizeof(cmdkw);


/*
	all the following functions have the same parameters :

	cmd : command structure
	ht : command options
	gdata : global data

	returns bool
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
	int	i = 0;
	dynary	*da;

	pmk_log("* Collecting targets\n");

	list = strdup(hash_get(ht, "LIST"));
	if (list == NULL) {
		errorf("LIST not assigned in TARGET");
		return(false);
	}
	da = da_init();
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
	pstr = hash_get(gdata->htab, "SYSCONFDIR");
	hash_add(gdata->htab, "sysconfdir", pstr);
*/

	/* if a file is given then it will be parsed later */
	acfile= hash_get(ht, "FILENAME");
	if (acfile != NULL) {
		gdata->ac_file = strdup(acfile);
	}

	/* compatibility tags */
	pstr = hash_get(gdata->htab, "PREFIX");
	hash_add(gdata->htab, "prefix", pstr);
	
	hash_add(gdata->htab, "exec_prefix", "${prefix}");
	hash_add(gdata->htab, "bindir", "${exec_prefix}/bin");
	hash_add(gdata->htab, "sbindir", "${exec_prefix}/sbin");
	hash_add(gdata->htab, "libexecdir", "${exec_prefix}/libexec");
	hash_add(gdata->htab, "libdir", "${exec_prefix}/lib");
	hash_add(gdata->htab, "datadir", "${prefix}/share");
	hash_add(gdata->htab, "includedir", "${prefix}/include");
	hash_add(gdata->htab, "mandir", "${prefix}/man");
	hash_add(gdata->htab, "infodir", "${prefix}/info");

	pstr = hash_get(gdata->htab, "BIN_INSTALL");
	hash_add(gdata->htab, "INSTALL", pstr);

	return(true);
}

/*
	check binary
*/

bool pmk_check_binary(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*filename,
		*bpath,
		binpath[MAXPATHLEN];
	bool	required;

	pmk_log("* Checking binary [%s]\n", cmd->label);

	required = require_check(ht);

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			return(true);
		}
	}

	filename = hash_get(ht, "FILENAME");
	if (filename == NULL) {
		errorf("FILENAME not assigned in label '%s'", cmd->label);
		return(false);
	}

	bpath = hash_get(gdata->htab, "BIN_PATH");
	if (bpath == NULL) {
		errorf("BIN_PATH not available.");
		return(false);
	}

	/* try to locate binary */
	pmk_log("\tFound binary '%s' : ", filename);
	if (get_file_path(filename, bpath, binpath, sizeof(binpath)) == false) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			/* define for template */
			label_set(gdata->labl, cmd->label, false);
			return(true);
		}
	} else {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, filename, true); /* XXX check ?*/
		record_val(gdata->htab, filename, binpath); /* XXX check ? */
		label_set(gdata->labl, cmd->label, true);
		return(true);
	}
}

/*
	check include file
*/

bool pmk_check_include(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*tfp;
	bool	required,
		rval;
	char	*incfile,
		*incfunc,
		*target,
		*ccpath,
		*pstr,
		inc_path[TMP_BUF_LEN] = "",
		cfgcmd[MAXPATHLEN];
	dynary	*da;
	int	r, i;

	pmk_log("* Checking include [%s]\n", cmd->label);

	required = require_check(ht);

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			return(true);
		}
	}

	/* get include filename */
	incfile = hash_get(ht, "INCLUDE");
	if (incfile == NULL) {
		errorf("INCLUDE not assigned in label '%s'", cmd->label);
		return(false);
	}

	/* check if a function must be searched */
	incfunc = hash_get(ht, "FUNCTION");

	tfp = fopen(INC_TEST_NAME, "w");
	if (tfp != NULL) {
		if (incfunc == NULL) {
			pmk_log("\tFound header '%s' : ", incfile);
			fprintf(tfp, INC_TEST_CODE, incfile);
			target = incfile;
		} else {
			pmk_log("\tFound function '%s' in '%s' : ", incfunc, incfile);
			fprintf(tfp, INC_FUNC_TEST_CODE, incfile, incfunc);
			target = incfunc;
		}

		/* fill test file */
		fclose(tfp);
	} else {
		errorf("cannot open test file.");
		return(false);
	}

	ccpath = hash_get(gdata->htab, "BIN_CC");
	if (ccpath == NULL) {
		errorf("cannot get compiler path.");
		return(false);
	}

	/* use each element of INC_PATH with -I */
	pstr = hash_get(gdata->htab, "INC_PATH");
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
						BIN_TEST_NAME, INC_TEST_NAME);
	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, target, true); /* XXX check ? */
		record_val(gdata->htab, target, ""); /* XXX check ? */
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
			record_def(gdata->htab, target, false); /* XXX check ?*/
			label_set(gdata->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(INC_TEST_NAME) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", INC_TEST_NAME);
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
	bool	required,
		rval;
	char	cfgcmd[MAXPATHLEN],
		*ccpath,
		*libname,
		*libfunc,
		*target,
		*pstr,
		lib_buf[TMP_BUF_LEN] = "";
	dynary	*da;
	int	r, i;

	pmk_log("* Checking library [%s]\n", cmd->label);

	required = require_check(ht);

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			return(true);
		}
	}

	libname = hash_get(ht, "LIBNAME");
	if (libname == NULL) {
		errorf("LIBNAME not assigned in label '%s'.", cmd->label);
		return(false);
	}

	libfunc = hash_get(ht, "FUNCTION");

	tfp = fopen(INC_TEST_NAME, "w");
	if (tfp != NULL) {
		if (libfunc == NULL) {
			pmk_log("\tFound library '%s' : ", libname);
			fprintf(tfp, LIB_TEST_CODE);
			target = libname;
		} else {
			pmk_log("\tFound function '%s' in '%s' : ", libfunc, libname);
			fprintf(tfp, LIB_FUNC_TEST_CODE, libfunc, libfunc);
			target = libfunc;
		}

		/* fill test file */
		fclose(tfp);
	} else {
		errorf("cannot open test file.");
		return(false);
	}

	ccpath = hash_get(gdata->htab, "BIN_CC");
	if (ccpath == NULL) {
		errorf("cannot get compiler path.");
		return(false);
	}

	/* use each element of LIB_PATH with -L */
	pstr = hash_get(gdata->htab, "LIB_PATH");
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
						libname, INC_TEST_NAME);
	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		pmk_log("yes.\n");
		/* define for template */
		record_def(gdata->htab, target, true); /* XXX check ?*/
		record_val(gdata->htab, target, ""); /* XXX check ? */
		snprintf(lib_buf, sizeof(lib_buf), "-l%s", libname);
		hash_append(gdata->htab, "LIBS", lib_buf, " "); /* XXX check ? */
		label_set(gdata->labl, cmd->label, true);
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
			record_def(gdata->htab, target, false); /* XXX check ? */
			label_set(gdata->labl, cmd->label, false);
			rval = true;
		}
	}

	if (unlink(INC_TEST_NAME) == -1) {
		/* cannot remove temporary file */
		fprintf(stderr, "Can not remove %s\n", INC_TEST_NAME);
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
	char	pipebuf[TMP_BUF_LEN],
		cfgpath[MAXPATHLEN],
		cfgcmd[MAXPATHLEN],
		*cfgtool,
		*libvers,
		*bpath;
	bool	required = true,
		rval = false;

	pmk_log("* Checking with config tool [%s]\n", cmd->label);

	required = require_check(ht);

	if (depend_check(ht, gdata) == false) {
		pmk_log("\t%s\n", gdata->errmsg);
		if (required == true) {
			return(false);
		} else {
			return(true);
		}
	}

	cfgtool = hash_get(ht, "CFGTOOL");
	if (cfgtool == NULL) {
		errorf("CFGTOOL not assigned in label '%s'.", cmd->label);
		return(false);
	}

	bpath = hash_get(gdata->htab, "BIN_PATH");
	if (bpath == NULL) {
		errorf("BIN_PATH not available.");
		return(false);
	}

	/* try to locate cfgtool */
	pmk_log("\tFound config tool '%s' : ", cfgtool);
	if (get_file_path(cfgtool, bpath, cfgpath, sizeof(cfgpath)) == false) {
		pmk_log("no.\n");
		if (required == true) {
			return(false);
		} else {
			/* XXX DEF_XXX_CONFIG */
			return(true);
		}
	} else {
		pmk_log("yes.\n");
	}
	
	libvers = hash_get(ht, "VERSION");
	if (libvers != NULL) {
		snprintf(cfgcmd, sizeof(cfgcmd), "%s --version", cfgpath);

		rpipe = popen(cfgcmd, "r");
		if (rpipe == NULL) {
			errorf("cannot get version from '%s'.", cfgcmd); /* XXX should correct this message ? */
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
			get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
			pclose(rpipe);
			if (check_version(libvers, pipebuf) == true) {
				pmk_log("yes (%s).\n", pipebuf);
				rval = true;
			
				/* XXX do we keep cfgtool for the tag ? */
				record_def(gdata->htab, cfgtool, true); /* XXX check ?*/
				record_val(gdata->htab, cfgtool, ""); /* XXX check ? */

				snprintf(cfgcmd, sizeof(cfgcmd), "%s --cflags", cfgpath);
				rpipe = popen(cfgcmd, "r");
				if (rpipe != NULL) {
					/* put result in CFLAGS */
					get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
					pclose(rpipe);
					hash_append(gdata->htab, "CFLAGS", pipebuf, " "); /* XXX check ? */
				}

				snprintf(cfgcmd, sizeof(cfgcmd), "%s --libs", cfgpath);
				rpipe = popen(cfgcmd, "r");
				if (rpipe != NULL) {
					/* put result in LIBS */
					get_line(rpipe, pipebuf, sizeof(pipebuf)); /* XXX check ? */
					pclose(rpipe);
					hash_append(gdata->htab, "LIBS", pipebuf, " "); /* XXX check ? */
				}

				/* XXX LDFLAGS, CPPFLAGS, LDFLAGS ? */
			} else {
				pmk_log("no (%s).\n", pipebuf);
				if (required == true) {
					rval = false;
				} else {
					/* XXX DEV_ */
					rval = true;
				}
			}
		}
	}

	/* XXX HAVE_... */
	return(rval);
}
