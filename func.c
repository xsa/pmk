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
	check boolean string

	str : string to check

	returns true is str is "true" else returns false

	NOTE : strncmp checks only on a length of 6 because
		lenght of "true" is 5 and "false" is 6 chr.
		If str is longer we don't need to check as
		we are sure that the result is false.

*/

bool check_bool_str(char *str) {
	if (strncmp(str, "true", 6) == 0) {
		return(true);
	} else {
		return(false);
	}
}

/*
	check version

	vref : reference version
	vers : version to check

	returns true is vers >= vref
*/

bool check_version(char *vref, char *vers) {
	/* XXX to do */
	return(true);
}

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
		strbuf[512];
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
	char	version[16],
		cfgcmd[MAXPATHLEN],
		*cfgtool,
		*libvers;
	bool	required = true;

	pmk_log("* Checking with config tool [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	cfgtool = hash_get(ht, "CFGTOOL");
	libvers = hash_get(ht, "VERSION");

	pmk_log("\tFound config tool '%s' : ", cfgtool);

	/* XXX should try to locate cfgtool */
	snprintf(cfgcmd, sizeof(cfgcmd), "%s --version", cfgtool);

	rpipe = popen(cfgcmd, "r");
	if (rpipe == NULL) {
		pmk_log("no.\n");
		return(false);
	} else {
		pmk_log("yes.\n");
		pmk_log("\tFound version >= %s : ", libvers);

		get_line(rpipe, version, sizeof(version)); /* XXX check returned value ? */
		/* XXX should check version */
		pmk_log("yes/no (%s)\n", version);

		pclose(rpipe);
		return(true);
	}
}
