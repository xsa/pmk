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

#include "compat/pmk_ctype.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "functool.h"
#include "premake.h"
#include "hash_tools.h"
#include "dynarray.h"
#include "common.h"


#define NBLANG	2
lgdata	ldata[NBLANG] = {
	{"C",	"CC",	"CPPFLAGS",	"CFLAGS",	"SLCFLAGS"},
	{"C++",	"CXX",	"CPPFLAGS",	"CXXFLAGS",	"SLCXXFLAGS"}
};


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
	if (strncmp(str, BOOL_STRING_TRUE, 6) == 0) {
		return(true);
	} else {
		return(false);
	}
}

/*
	invert bool value

	value : input boolean value

	return : negation of value
*/

bool invert_bool(bool value) {
		if (value == true) {
			return(false);
		} else {
			return(true);
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
		snprintf(bstr, sizeof(bstr), BOOL_STRING_TRUE);
	} else {
		snprintf(bstr, sizeof(bstr), BOOL_STRING_FALSE);
	}
	return(bstr);
}

/*
	get directory of given filename in given path list

	filename : name of the file to look for
	path : string of the list of path
	storage : storage for resulting directory
	size : size of storage

	returns true if filename is found in one of the list's path
*/

bool get_file_dir_path(char *filename, char *path, char *storage, int size) {
	bool	 rval = false;
	dynary	*bplst;

	/* fill dynary with path */
	bplst= str_to_dynary(path, CHAR_LIST_SEPARATOR);
	if (bplst == NULL) {
		errorf("failed to put a path into a dynamic array.");
		return(false);
	}

	/* try to locate cfgtool */
	if (find_file_dir(bplst, filename, storage, size) == true) {
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
			case '*' :
				*p = 'P';
				break;
			default :
				if (isalnum(*p) == 0) {
					*p = '_';
				} else {
					*p = (char) toupper((int) *p);
				}
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
	int	 s;

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
	
	if (hash_update_dup(ht, def_str, def_val) == HASH_ADD_FAIL)
		return(false);

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
	char	*semidef,
		 have_str[MAX_HASH_VALUE_LEN];
	int	 s;

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(false);

	s = sizeof(have_str);
	if (snprintf(have_str, s, "HAVE_%s", semidef) >= s)
		return(false);

	if (hash_update_dup(ht, have_str, value) == HASH_ADD_FAIL)
		return(false);

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

bool label_set(htable *lht, char *name, bool status) {
	if (hash_update_dup(lht, name, bool_to_str(status)) == HASH_ADD_FAIL)
		return(false);

	return(true);
}

/*
	check label

	ht : label hash table
	name : label name
*/

bool label_check(htable *lht, char *name) {
	bool	 neg = false,
		 rval;
	char	*p;

	if (*name == '!') {
		name++;
		neg = true;
	}

	p = (char *)hash_get(lht, name);
	if (p == NULL)
		return(false);

	rval = check_bool_str(p);

	if (neg == true) {
		rval = invert_bool(rval);
	}

	return(rval);
}

/*
	check depends

	ht : label hash table
	deplst : string that contain the list of label dependencies

	returns true if all dependencies are true
*/

bool depend_check(htable *lht, pmkdata *gd) {
	bool	 rval = true;
	char	*fdep;
	dynary	*da;
	int	 i;
	pmkobj	*po;

	po = hash_get(lht, "DEPEND");
	if (po == NULL) {
		/* no dependencies, check is true */
		return(true);
	}

	da = po_get_list(po);
	if (da == NULL) {
		/* DEPEND is not a list */
		snprintf(gd->errmsg, sizeof(gd->errmsg), "Syntax error in DEPEND !");
		return(false);
	}

	/* check labels one by one */
	for (i = 0 ; (i < da_usize(da)) && (rval == true) ; i++) {
		fdep = da_idx(da, i);
		if (label_check(gd->labl, fdep) == false) {
			rval = false;
			snprintf(gd->errmsg, sizeof(gd->errmsg), "Required '%s' dependency failed.", fdep);
		}
	}

	hash_delete(lht, "DEPEND"); /* not really useful but save memory */

	return(rval);
}

/*
	check the required flag

	ht : hash table of the command options

	returns a boolean
*/

bool require_check(htable *pht) {
	bool	 rval;
	pmkobj	*req;

	req = hash_get(pht, "REQUIRED");
	if (req == NULL) {
		/* by default REQUIRED is true if not specified */
		return(true);
	}

	switch(po_get_type(req)) {
		case PO_BOOL :
			rval = po_get_bool(req);
			break;
		case PO_STRING :
			rval = check_bool_str(po_get_str(req));
			break;
		default :
			rval = false;
			break;
	}

	return(rval);
}

/*
	check language

	lang : language to check

	return : language structure or NULL
*/

lgdata *check_lang(char *lang) {
	int	 i;

	for (i = 0 ; i < NBLANG ; i++) {
		if (strncmp(ldata[i].name, lang, LANG_NAME_LEN) == 0) {
			return(&ldata[i]);
		}
	}

	return(NULL);
}

/*
	check language with it's compiler (symbolic) name

	comp : compiler (symbolic) name

	return : language structure or NULL
*/

lgdata *check_lang_comp(char *comp) {
	int	 i;

	for (i = 0 ; i < NBLANG ; i++) {
		if (strncmp(ldata[i].comp, comp, LANG_NAME_LEN) == 0) {
			return(&ldata[i]);
		}
	}

	return(NULL);
}

/*
	provide data on language used

	pht : hash table that should contain LANG 
	pgd : global data structure 

	return : lgdata structure or NULL for unknown language
*/

lgdata *get_lang(htable *pht, pmkdata *pgd) {
	char	*lang;

	/* check first if language has been provided locally */
	lang = (char *)po_get_data(hash_get(pht, "LANG"));
	if (lang == NULL) {
		/* check global lang if available */
		if (pgd->lang != NULL) {
			return(check_lang(pgd->lang));
		} else {
			/* else return C by default */
			return(&ldata[0]);
		}
	}

	return(check_lang(lang));
}

/*
	provide compiler path

	pht : main hash table
	compname : compiler name from lgdata structure

	return : compiler's path
*/

char *get_comp_path(htable *pht, char *compname) {
	char	key[OPT_NAME_LEN];

	snprintf(key, sizeof(key), "BIN_%s", compname);
	return((char *) hash_get(pht, key));
}

/*
	blah
*/

char *parse_idtf(char *pstr, char *pbuf, size_t size) {
	while (((isalnum(*pstr) != 0) || (*pstr == '_')) && (size > 0)) {
		*pbuf = *pstr;
		pbuf++;
		pstr++;
		size--;
	}

	if (size == 0)
		return(NULL);

	*pbuf = CHAR_EOS;
	
	return(pstr);
}

/*
	check if config tool data is loaded and try to load it if not done

	pgd : global data structure

	return : boolean
*/

bool check_cfgt_data(pmkdata *pgd) {
	if (pgd->cfgt == NULL) {
		pgd->cfgt = parse_cfgt_file();
		if (pgd->cfgt == NULL) {
			return(false);
		}
	}

	return(true);
}

