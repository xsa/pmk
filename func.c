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

	returns TRUE is str is "TRUE" else returns FALSE

	NOTE : strncmp checks only on a length of 6 because
		lenght of "TRUE" is 5 and "FALSE" is 6 chr.
		If str is longer we don't need to check as
		we are sure that the result is FALSE.

*/

bool check_bool_str(char *str) {
	if (strncmp(str, "TRUE", 6) == 0) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/*
	define variables
*/

bool pmk_define(pmkcmd *cmd, htable *ht) {
	pmk_log("* Parsing define\n");
	return(TRUE);
}

/*
	check binary
*/

bool pmk_check_binary(pmkcmd *cmd, htable *ht) {
	char	*filename;
	bool	required = TRUE;

	pmk_log("* Checking binary [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	filename = hash_get(ht, "FILENAME");

	pmk_log("\tFound binary '%s' : ", filename);
	pmk_log("XXX.\n");
	return(TRUE);
}

/*
	check include file
*/

bool pmk_check_include(pmkcmd *cmd, htable *ht) {
	char	*incfile,
		*incfunc,
		incpath[MAXPATHLEN],
		strbuf[512];
	FILE	*ifp;
	bool	required = TRUE;

	pmk_log("* Checking include [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	/* get include filename */
	incfile = hash_get(ht, "INCLUDE");
	if (incfile == NULL) {
		errorf("INCLUDE not assigned in label %s", cmd->label);
		return(FALSE);
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
			while (get_line(ifp, strbuf, sizeof(strbuf))) {
				/* XXX should check in a better way ? */
				if (strstr(strbuf, incfunc) != NULL) {
					pmk_log("yes.\n");
					return(TRUE);
				}
			}
			pmk_log("no.\n");
			errorf("Failed to find '%s' in '%s'", incfunc, incfile);
			return(FALSE);
		}
	} else {
		errorf("Failed to find %s", incfunc, incfile);
		pmk_log("no.\n");
		return(FALSE);
	}
}

/*
	check library
*/

bool pmk_check_lib(pmkcmd *cmd, htable *ht) {
	char	*libname,
		*libvers;
	bool	required = TRUE;

	pmk_log("* Checking library [%s]\n", cmd->label);

	required = check_bool_str(hash_get(ht, "REQUIRED"));

	libname = hash_get(ht, "LIBNAME");
	libvers = hash_get(ht, "VERSION");

	pmk_log("\tFound library '%s %s' : ", libname, libvers);
	pmk_log("XXX.\n");
	return(TRUE);
}

/*
	check with *-config utility
*/

bool pmk_check_config(pmkcmd *cmd, htable *ht) {
	FILE	*rpipe;
	char	version[16],
		cfgcmd[MAXPATHLEN],
		*cfgtool,
		*libvers;
	bool	required = TRUE;

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
		return(FALSE);
	} else {
		pmk_log("yes.\n");
		pmk_log("\tFound version >= %s : ", libvers);

		get_line(rpipe, version, sizeof(version));
		/* XXX should check version */
		pmk_log("yes/no (%s)\n", version);

		pclose(rpipe);
		return(TRUE);
	}
}
