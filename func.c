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
#include <string.h>

#include "func.h"


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
	str_to_dynary(list, ',', da);
	
	for (i=0 ; i < da_size(da) ; i++) {
		pmk_log("\tAdded '%s'.\n", da_idx(da, i));
	}

	gdata->tlist = da;

	return(true);
}

/*
	check binary
*/

bool pmk_check_binary(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*filename;
	bool	required = true;

	pmk_log("* Checking binary [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	filename = hash_get(ht, "FILENAME");

	pmk_log("\tFound binary '%s' : ", filename);
	pmk_log("XXX.\n");
	return(true);
}

/*
	check include file
*/

bool pmk_check_include(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*incfile,
		*incfunc,
		incpath[MAXPATHLEN],
		strbuf[512]; /* storage buffer */
	FILE	*ifp;
	bool	required = true;

	pmk_log("* Checking include [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	/* get include filename */
	incfile = hash_get(ht, "INCLUDE");
	if (incfile == NULL) {
		errorf("INCLUDE not assigned in label %s", cmd->label);
		return(false);
	}

	/* check if a function must be searched */
	incfunc = hash_get(ht, "FUNCTION");

	/* XXX should look in INC_PATH */
	snprintf(incpath, MAXPATHLEN, "/usr/include/%s", incfile);

	ifp = fopen(incpath, "r");

	pmk_log("\tFound header '%s' : ", incfile);
	if (ifp != NULL) {
		pmk_log("yes.\n");
		if (incfunc != NULL) {
			pmk_log("\tFound function '%s' : ", incfunc);
			while (get_line(ifp, strbuf, sizeof(strbuf)) == true) {
				/* XXX should check in a better way ? */
				if (strstr(strbuf, incfunc) != NULL) {
					pmk_log("yes.\n");
					return(true);
				}
			}
			pmk_log("no.\n");
			errorf("Failed to find '%s' in '%s'", incfunc, incfile);
			return(false);
		} else {
			/* no function to test */
			return(true);
		}
	} else {
		errorf("Failed to find %s", incfunc, incfile);
		pmk_log("no.\n");
		return(false);
	}
}

/*
	check library
*/

bool pmk_check_lib(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	char	*libname,
		*libvers;
	bool	required = true;

	pmk_log("* Checking library [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	libname = hash_get(ht, "LIBNAME");
	libvers = hash_get(ht, "VERSION");

	pmk_log("\tFound library '%s %s' : ", libname, libvers);
	pmk_log("XXX.\n");
	return(true);
}

/*
	check with *-config utility
*/

bool pmk_check_config(pmkcmd *cmd, htable *ht, pmkdata *gdata) {
	FILE	*rpipe;
	char	version[MAX_VERS_LEN],
		cfgpath[MAXPATHLEN],
		cfgcmd[MAXPATHLEN],
		*cfgtool,
		*libvers,
		*bpath;
	bool	required = true,
		rval = false;

	pmk_log("* Checking with config tool [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	cfgtool = hash_get(ht, "CFGTOOL");
	if (cfgtool == NULL) {
		errorf("CFGTOOL not provided.");
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
			errorf("Cannot get version from '%s'.", cfgcmd);
			return(false);
		} else {
			pmk_log("\tFound version >= %s : ", libvers);
	
			get_line(rpipe, version, sizeof(version)); /* XXX check returned value ? */
			if (check_version(libvers, version) == true) {
				pmk_log("Yes (%s).\n", version);
				rval = true;
			} else {
				pmk_log("No (%s).\n", version);
				if (required == true) {
					rval = false;
				} else {
					/* XXX DEV_ */
					rval = true;
				}
			}
	
			pclose(rpipe);
			return(true);
		}
	}

	/* XXX HAVE_... */
}
