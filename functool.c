/* $Id$ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
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


#include <stdlib.h>
#include <stdarg.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "functool.h"
#include "premake.h"
#include "dynarray.h"
#include "common.h"

/*#define FC_DEBUG	1*/

/*****************
 * language data *
 ***********************************************************************/

#define NBLANG	2
lgdata	ldata[NBLANG] = {
	{"C",	"CC",	"CPPFLAGS",	"CFLAGS",	"SLCFLAGS"},
	{"C++",	"CXX",	"CPPFLAGS",	"CXXFLAGS",	"SLCXXFLAGS"}
};


/********************
 * check_bool_str() *
 ***********************************************************************
 DESCR
	check boolean string

 IN
	str :	string to check

 OUT
	boolean

 NOTE
	strncmp checks only on a length of 6 because lenght of "true" is 5
	and "false" is 6 chr. If str is longer we don't need to check as
	we are sure that the result is false.
 ***********************************************************************/

bool check_bool_str(char *str) {
	if (strncmp(str, BOOL_STRING_TRUE, 6) == 0) {
		return(true);
	} else {
		return(false);
	}
}


/*****************
 * invert_bool() *
 ***********************************************************************
 DESCR
	invert bool value

 IN
	value :	input boolean value

 OUT
	negation of value
 ***********************************************************************/

bool invert_bool(bool value) {
		if (value == true) {
			return(false);
		} else {
			return(true);
		}
}


/*****************
 * bool_to_str() *
 ***********************************************************************
 DESCR
	convert boolean into string

 IN
	value :	boolean value to convert to string

 OUT
	returns the string
 ***********************************************************************/

char *bool_to_str(bool value) {
	static char	bstr[6];

	if (value == true) {
		snprintf(bstr, sizeof(bstr), BOOL_STRING_TRUE); /* no check */
	} else {
		snprintf(bstr, sizeof(bstr), BOOL_STRING_FALSE); /* no check */
	}
	return(bstr);
}


/***********************
 * get_file_dir_path() *
 ***********************************************************************
 DESCR
	get directory of given filename in given path list

 IN
	filename :	name of the file to look for
	path :		string of the list of path
	storage :	storage for resulting directory
	size :		size of storage

 OUT
	returns true if filename is found in one of the list's path
 ***********************************************************************/

bool get_file_dir_path(char *filename, char *path, char *storage, int size) {
	bool	 rval = false;
	dynary	*bplst;

	/* fill dynary with path */
	bplst= str_to_dynary(path, PATH_STR_DELIMITER);
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


/****************
 * str_to_def() *
 ***********************************************************************
 DESCR
	generate semi-definition from a string

 IN
	str :	string to use

 OUT
	returns semidef string
 ***********************************************************************/

char *str_to_def(char *str) {
	static char	 buffer[TMP_BUF_LEN];
	char		*p;

	if (strlcpy_b(buffer, str, sizeof(buffer)) == false)
		return(NULL); /* buffer too small */

	p = buffer;

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

	return(buffer);
}


/********************
 * build_def_name() *
 ***********************************************************************
 DESCR
	build_def

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *build_def_name(char *name) {
	static char	 def_str[MAX_HASH_KEY_LEN];
	char		*semidef;

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(NULL);

	if (snprintf_b(def_str, sizeof(def_str), "DEF__%s", semidef) == false)
		return(NULL);

	return(def_str);
}


/****************
 * record_def() *
 ***********************************************************************
 DESCR
	record data definition tag (DEF__*)

 IN
	ht :		hash table to store the definition
	name :		data name
	status :	status of data

 OUT
	boolean
 ***********************************************************************/

bool record_def(htable *ht, char *name, bool status) {
	char	*semidef,
			 def_str[MAX_HASH_KEY_LEN],
			 have_str[MAX_HASH_VALUE_LEN],
			 def_val[MAX_HASH_VALUE_LEN];

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(false);

	if (snprintf_b(def_str, sizeof(def_str), "DEF__%s", semidef) == false)
		return(false);

	if (snprintf_b(have_str, sizeof(def_str), "HAVE_%s", semidef) == false)
		return(false);

	if (status == true) {
		if (snprintf_b(def_val, sizeof(def_str),
					"#define %s 1", have_str) == false)
			return(false);
	} else {
		if (snprintf_b(def_val, sizeof(def_str),
					"#undef %s", have_str) == false)
			return(false);
	}

#ifdef FC_DEBUG
	debugf("record_def() : def_val = '%s'", def_val);
#endif

	/* pmk style define ID (DEF__*) */
	if (hash_update_dup(ht, def_str, def_val) == HASH_ADD_FAIL)
		return(false);
#ifdef FC_DEBUG
	debugf("record_def() : recorded '%s' with '%s'", def_str, def_val);
#endif

	return(true);
}


/*********************
 * record_def_data() *
 ***********************************************************************
 DESCR
	record definition data (DEF__* and HAVE_*)

 IN
	ht :	hash table to store the definition
	name :	tag name
	value :	tag value

 OUT
	boolean
 ***********************************************************************/

bool record_def_data(htable *ht, char *name, char *value) {
	char	*semidef,
			 def_str[MAX_HASH_KEY_LEN],
			 def_val[MAX_HASH_VALUE_LEN],
			 have_str[MAX_HASH_VALUE_LEN];

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(false);

	if (snprintf_b(def_str, sizeof(def_str), "DEF__%s", semidef) == false)
		return(false);

	if (snprintf_b(have_str, sizeof(def_str), "HAVE_%s", semidef) == false)
		return(false);

	if (value != NULL) {
		/* common variable tag HAVE_* */
		if (hash_update_dup(ht, have_str, value) == HASH_ADD_FAIL)
			return(false);

#ifdef FC_DEBUG
		debugf("record_def_data() : recorded '%s' with '%s'", have_str, value);
#endif

		if (snprintf_b(def_val, sizeof(def_str),
					"#define %s %s",
					have_str, value) == false)
			return(false);
	} else {
		if (snprintf_b(def_val, sizeof(def_str),
					"#undef %s", have_str) == false)
			return(false);
	}

#ifdef FC_DEBUG
	debugf("record_def_data() : def_val = '%s'", def_val);
#endif

	/* pmk style define ID (DEF__*) */
	if (hash_update_dup(ht, def_str, def_val) == HASH_ADD_FAIL)
		return(false);
#ifdef FC_DEBUG
	debugf("record_def_data() : recorded '%s' with '%s'", def_str, def_val);
#endif

	return(true);
}


/****************
 * record_val() *
 ***********************************************************************
 DESCR
	record data tag
	XXX TODO to remove

 IN
	ht :	hash table to store the definition
	name :	data name
	value :	value to store

 OUT
	boolean
 ***********************************************************************/

bool record_val(htable *ht, char *name, char *value) {
	char	*semidef,
			 have_str[MAX_HASH_VALUE_LEN];

	semidef = str_to_def(name);
	if (semidef == NULL)
		return(false);

	if (snprintf_b(have_str, sizeof(have_str),
					"HAVE_%s", semidef) == false)
		return(false);

	if (hash_update_dup(ht, have_str, value) == HASH_ADD_FAIL)
		return(false);
#ifdef FC_DEBUG
	debugf("record_val() : recorded '%s' with '%s'", have_str, value);
#endif

	return(true);
}


/**********************
 * process_def_list() *
 ***********************************************************************
 DESCR
	process list of defines

 IN
	ht :	storage hash table
	da :	list of defines

 OUT
	boolean
 ***********************************************************************/

bool process_def_list(htable *ht, dynary *da) {
	char	*name;
	int		 i,
			 n;

	/* process additional defines */
	if (da != NULL) {
		n = da_usize(da);
		for (i = 0 ; i < n ; i++) {
			name = da_idx(da, i);
			pmk_log("\tProcessing additional define '%s': ", name);
			record_def_data(ht, name, "1");
			pmk_log("ok.\n");
		}
	}

	return(true);
}


/***************
 * label_set() *
 ***********************************************************************
 DESCR
	set label

 IN
	ht :		label hash table
	name :		label name
	status :	label status

 OUT
	boolean
 ***********************************************************************/

bool label_set(htable *lht, char *name, bool status) {
	if (hash_update_dup(lht, name, bool_to_str(status)) == HASH_ADD_FAIL)
		return(false);

	return(true);
}


/*****************
 * label_check() *
 ***********************************************************************
 DESCR
	check label

 IN
	ht :	label hash table
	name :	label name

 OUT
	boolean
 ***********************************************************************/

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


/******************
 * depend_check() *
 ***********************************************************************
 DESCR
	check dependencies

 IN
	ht :		label hash table
	deplst :	string that contain the list of label dependencies

 OUT
	boolean
 ***********************************************************************/

bool depend_check(htable *lht, pmkdata *gd) {
	bool		 rval = true;
	char		*fdep;
	dynary		*da;
	pmkobj		*po;
	unsigned int	 i;

	po = hash_get(lht, KW_OPT_DEPEND);
	if (po == NULL) {
		/* no dependencies, check is true */
		return(true);
	}

	da = po_get_list(po);
	if (da == NULL) {
		/* DEPEND is not a list */
		strlcpy(gd->errmsg, "Syntax error in DEPEND !",
					sizeof(gd->errmsg)); /* no check */
		return(false);
	}

	/* check labels one by one */
	for (i = 0 ; (i < da_usize(da)) && (rval == true) ; i++) {
		fdep = da_idx(da, i);
		if (label_check(gd->labl, fdep) == false) {
			rval = false;
			snprintf(gd->errmsg, sizeof(gd->errmsg),
					"Required '%s' dependency failed.",
					fdep); /* no check */
		}
	}

	hash_delete(lht, KW_OPT_DEPEND); /* not really useful but save memory */

	return(rval);
}


/*******************
 * require_check() *
 ***********************************************************************
 DESCR
	check the required flag

 IN
	ht :	hash table of the command options

 OUT
	returns a boolean
 ***********************************************************************/

bool require_check(htable *pht) {
	bool	 rval;
	pmkobj	*req;

	req = hash_get(pht, KW_OPT_REQUIRED);
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


/****************
 * check_lang() *
 ***********************************************************************
 DESCR
	check language

 IN
	lang :	language to check

 OUT
	language structure or NULL
 ***********************************************************************/

lgdata *check_lang(char *lang) {
	int	 i;

	for (i = 0 ; i < NBLANG ; i++) {
		if (strncmp(ldata[i].name, lang, LANG_NAME_LEN) == 0) {
			return(&ldata[i]);
		}
	}

	return(NULL);
}


/*********************
 * check_lang_comp() *
 ***********************************************************************
 DESCR
	check language with its compiler (symbolic) name

 IN
	comp :	compiler (symbolic) name

 OUT
	language structure or NULL
 ***********************************************************************/

lgdata *check_lang_comp(char *comp) {
	int	 i;

	for (i = 0 ; i < NBLANG ; i++) {
		if (strncmp(ldata[i].comp, comp, LANG_NAME_LEN) == 0) {
			return(&ldata[i]);
		}
	}

	return(NULL);
}


/**************
 * get_lang() *
 ***********************************************************************
 DESCR
	provide data on language used

 IN
	pht :	hash table that should contain LANG
	pgd :	global data structure

 OUT
	lgdata structure or NULL for unknown language
 ***********************************************************************/

lgdata *get_lang(htable *pht, pmkdata *pgd) {
	char	*lang;

	/* check first if language has been provided locally */
	lang = (char *)po_get_data(hash_get(pht, KW_OPT_LANG));
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


/******************
 * get_lang_str() *
 ***********************************************************************
 DESCR
	provide data on language used

 IN
	pht :	hash table that should contain LANG
	pgd :	global data structure

 OUT
	language string or NULL if language cannot be determined
 ***********************************************************************/

char *get_lang_str(htable *pht, pmkdata *pgd) {
	char	*lang;

	/* check first if language has been provided locally */
	lang = (char *)po_get_data(hash_get(pht, KW_OPT_LANG));
	if (lang == NULL) {
		/* get global lang (could be NULL */
		lang = pgd->lang;
	}

	return(lang);
}


/*******************
 * get_comp_path() *
 ***********************************************************************
 DESCR
	provide compiler path

 IN
	pht :		main hash table
	compname :	compiler name from lgdata structure

 OUT
	compiler's path
 ***********************************************************************/

char *get_comp_path(htable *pht, char *compname) {
	char	key[OPT_NAME_LEN];

	if (snprintf_b(key, sizeof(key), "BIN_%s", compname) == false)
		return(NULL);

	return((char *) hash_get(pht, key));
}


/*********************
 * check_cfgt_data() *
 ***********************************************************************
 DESCR
	check if config tool data is loaded and try to load it if not done

 IN
	pgd :	global data structure

 OUT
	boolean
 ***********************************************************************/

bool check_cfgt_data(pmkdata *pgd) {
	if (pgd->cfgt == NULL) {
		pgd->cfgt = parse_cfgt_file();
		if (pgd->cfgt == NULL) {
			return(false);
		}
	}

	return(true);
}


/**********************
 * process_required() *
 ***********************************************************************
 DESCR
	process the required flag on failure

 IN
	pgd :		global data structure
	pcmd :		command data structure
	required :	required flag
	key :		optional define to record (not processed if NULL)
	value :		optional value to record (see key)

 OUT
	boolean
 ***********************************************************************/

bool process_required(pmkdata *pgd, pmkcmd *pcmd, bool required,
					char *key, char *value) {
	if (key != NULL) {
		/* record definition */
		record_def_data(pgd->htab, key, value);
	}

	/* set label value */
	label_set(pgd->labl, pcmd->label, false);

	/* if required is false then we can
	   continue (true) else stop (false) */
	return(invert_bool(required));
}


/********************
 * c_file_builder() *
 ***********************************************************************
 DESCR
	build c style test file

 IN
	fnbuf :	file name buffer
	fbsz :	file name buffer size
	tmpl :	content template

 OUT
	boolean
 ***********************************************************************/

bool c_file_builder(char *fnbuf, size_t fbsz, char *tmpl, ...) {
	FILE	*tfp;
	int	 r;
	va_list	 ap;

	/* open temporary file */
	tfp = tmps_open(TEST_FILE_NAME, "w", fnbuf, fbsz, strlen(C_FILE_EXT));
	if (tfp == NULL) {
		/* verbose(VERBOSE_LEVEL_MAX, "c_file_builder: tmps_open() failed"); */
		return(false); /* failed to open */
	}

	va_start(ap, tmpl);
	r = vfprintf(tfp, tmpl, ap);
	va_end(ap);

	fclose(tfp);

	/* vprintf failed */
	if (r == -1)
		return(false);

	/* everything is okay, tchuss */
	return(true);
}

