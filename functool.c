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

#include "functool.h"


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
	/* XXX use strncmp to check both version but not sure it is 100% right */
	if (strncmp(vref, vers, MAX_VERS_LEN) > 0) {
		return(false);
	} else {
		return(true);
	}
}

/*
	get path of given filename in given path list

	filename : name of the file to look for
	path : string of the list of path
	storage : storage for resulting path
	size : size of storage

	returns true if filename is found in one of the list's path
*/

bool get_file_path(char *filename, char *path, char *storage, int size) {
	bool	rval = false;
	dynary	*bplst;

	/* init dynary */
	bplst = da_init();
	if (bplst == NULL) {
		errorf("Cannot init dynamic array.");
		return(false);
	}

	/* fill dynary with path */
	if (str_to_dynary(path, CHAR_LIST_SEPARATOR, bplst) == false) {
		errorf("Failed to put a path into a dynamic array.");
		return(false);
	}

	/* try to locate cfgtool */
	if (find_file(bplst, filename, storage, size) == true) {
		rval = true;
	} else {
		rval = false;
	}

	da_destroy(bplst);
	return(rval);
}

/*
*/

char *str_to_def(char *str) {
	char	*newstr;

	newstr = strdup(str);

	/* XXX parse line */

	return(newstr);
}
