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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

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
	if (strncmp(str, "TRUE", 6) == 0) {
		return(true);
	} else {
		return(false);
	}
}

/*
	convert boolean into string

	value : boolean value to convert to string

	returns the string
*/

char *bool_to_str(bool value) {
	static char	bstr[6];

	if (value == true) {
		snprintf(bstr, sizeof(bstr), "TRUE");
	} else {
		snprintf(bstr, sizeof(bstr), "FALSE");
	}
	return(bstr);
}

/*
	check version

	vref : reference version
	vers : version to check

	returns true is vers >= vref
*/

bool check_version(char *vref, char *vers) {
	int	i = 0,
		ref,
		cmp;
	char	*sr,
		*sc;
	dynary	*vr,
		*vc;
	bool	exit = false,
		rval = true;

	/* need to check da_* returns */
	vr = da_init();
	if (vr == NULL) {
		errorf("cannot initialize dynary.");
		return(false);
	}
	if (str_to_dynary(vref, '.', vr) == false) {
		errorf("cannot parse reference version '%s'.", vref);
		da_destroy(vr);
		return(false);
	}
	vc = da_init();
	if (vc == NULL) {
		errorf("cannot initialize dynary.");
		return(false);
	}
	if (str_to_dynary(vers, '.', vc) == false) {
		errorf("cannot parse comparison version '%s'.", vers);
		da_destroy(vr);
		da_destroy(vc);
		return(false);
	}

	while (exit == false) {
		sr = da_idx(vr, i);
		sc = da_idx(vc, i);

		if (sr != NULL && sc != NULL) {
			/* check both version */
			ref = atoi(sr); /* XXX should consider using strtol */
			cmp = atoi(sc);

			if (ref > cmp) {
				/* version is lower than required */
				rval = false;
				exit = true;
			} else {
				if (ref < cmp) {
					/* version is greater than required */
					rval = true;
					exit = true;
				}
			}
		} else {
			/* reached end of (at least) one version */
			if (sr != NULL) {
				/* reference has remaining number */
				ref = atoi(sr);
				if (ref > 0) {
					/* reference is greater */
					rval = false;
				}
			}
			exit = true;
		}
		i++;
	}

	da_destroy(vr);
	da_destroy(vc);
	return(rval);
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
	generate semi-definition from a string

	str : string to use

	returns semidef string
*/

char *str_to_def(char *str) {
	char	*newstr,
		*p;

	newstr = strdup(str);
	if (newstr == NULL)
		return(NULL);

	p = newstr;

	while (*p != CHAR_EOS) {
		switch (*p) {
			case '-' :
			case '.' :
			case '/' :
				*p = '_';
				break;
			default :
				*p = (char) toupper((int) *p);
				break;
		}
		p++;
	}

	return(newstr);
}

/*
	record data definition tag (DEF__*)

	ht : hast table to store the definition
	name : data name
	status : status of data

	returns true on success
*/

bool record_def(htable *ht, char *name, bool status) {
	char	*semidef,
		def_str[MAX_HASH_KEY_LEN],
		have_str[MAX_HASH_VALUE_LEN],
		def_val[MAX_HASH_VALUE_LEN];
	int	s;

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(false);

	s = sizeof(def_str);
	if (snprintf(def_str, s, "DEF__%s", semidef) >= s)
		return(false);

	s = sizeof(have_str);
	if (snprintf(have_str, s, "HAVE_%s", semidef) >= s)
		return(false);

	s = sizeof(def_val);
	if (status == true) {
		if (snprintf(def_val, s, "#define %s 1", have_str) >= s) 
			return(false);
	} else {
		if (snprintf(def_val, s, "#undef %s", have_str) >= s)
			return(false);
	}
	
	if (hash_add(ht, def_str, def_val) == HASH_ADD_FAIL)
		return(false);
	/*debugf("%s => %s", def_str, def_val);*/

	free(semidef);
	return(true);
}

/*
	record data tag

	ht : hast table to store the definition
	name : data name
	value : value to store

	returns true on success
*/

bool record_val(htable *ht, char *name, char*value) {
	char	*semidef;

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(false);

	if (hash_add(ht, semidef, value) == HASH_ADD_FAIL)
		return(false);
	/*debugf("%s => %s", semidef, value);*/

	free(semidef);
	return(true);
}

/*
	set label

	ht : label has htable
	name : label name
	status : label status

	returns true on success
*/

bool label_set(htable *ht, char *name, bool status) {
	if (hash_add(ht, name, bool_to_str(status)) == HASH_ADD_FAIL)
		return(false);
	/*debugf("%s => %s", name, hash_get(ht, name));*/

	return(true);
}

/*
	check label

	ht : label hash table
	name : label name
*/

bool label_check(htable *lht, char *name) {
	char	*p;

	p = hash_get(lht, name);
	if (p == NULL)
		return(false);

	return(check_bool_str(p));
}

/*
	check depends

	ht : label hash table
	deplst : string that contain the list of label dependencies

	returns true if all dependencies are true
*/

bool depend_check(htable *lht, pmkdata *gd) {
	bool	rval = true;
	char	*deplst,
		*fdep;
	dynary	*da;
	int	i;

	deplst = hash_get(lht, "DEPEND");
	if (deplst == NULL) {
		/* no dependencies, check is true */
		return(true);
	}

	da = da_init();
	if (da == NULL)
		return(false);

	/* get the label dependencies in a dynary */
	str_to_dynary(deplst, CHAR_LIST_SEPARATOR, da);

	/* check labels one by one */
	for (i = 0 ; (i < da_usize(da)) && (rval == true) ; i++) {
		fdep = da_idx(da, i);
		if (label_check(gd->labl, fdep) == false) {
			rval = false;
			snprintf(gd->errmsg, sizeof(gd->errmsg), "Dependency '%s' failed.", fdep);
		}
	}

	da_destroy(da);

	return(rval);
}

/*
	check the required flag

	ht : hash table of the command options

	returns a boolean
*/

bool require_check(htable *pht) {
	char	*req;

	req = hash_get(pht, "REQUIRED");
	if (req == NULL) {
		/* by default REQUIRED is true if not specified */
		return(true);
	}

	return(check_bool_str(req));
}
