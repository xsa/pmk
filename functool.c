/* $Id$ */

/*
 * Copyright (c) 2003-2006 Damien Couderc
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


#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "dynarray.h"
#include "functool.h"
#include "premake.h"
#include "tags.h"

/*#define FC_DEBUG	1*/


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


/******************
 * find_pattern() *
 ***********************************************************************
 DESCR
	find a file pattern in the given list of path

 IN
	da :		dynary that contains list of path
	pattern :	pattern to find
	pbuf :		buffer to store directory if matching
	bsize :		buffer size

 OUT
	returns true if pattern is found in one of the list of path
 ***********************************************************************/

bool find_pattern(dynary *da, char *pattern, char *pbuf, int bsize) {
	struct dirent	*de;
	DIR				*dp;
	bool			 found;
	char			*path;
	size_t			 i,
					 rsize;

	found = false;
	rsize = sizeof(de->d_name);

	for (i = 0 ; (i < da_usize(da)) && (found == false) ; i++) {
		path = da_idx(da, i);

		dp = opendir(path);
		if (dp == NULL) {
			/* can't open directory */
			continue;
		}

		while ((de = readdir(dp)) != NULL) {
			/* check with pattern */
			if (fnmatch(pattern, de->d_name, 0) == 0) {
				/* file match with pattern */
				if (strlcpy_b(pbuf, path, bsize) == true) {
					/* pbuf set */
					found = true;
				}
				break;
			}
		}
		closedir(dp);
	}
	return(found);
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

bool record_def(htable_t *ht, char *name, bool status) {
	char	*def_str,
			*have_str,
			 def_val[TMP_BUF_LEN];

	def_str = gen_basic_tag_def(name);

	have_str = gen_ac_tag_name(name);

	if (status == true) {
		if (snprintf_b(def_val, sizeof(def_val),
					"#define %s 1", have_str) == false)
			return(false);
	} else {
		if (snprintf_b(def_val, sizeof(def_val),
					"#undef %s", have_str) == false)
			return(false);
	}

#ifdef FC_DEBUG
	debugf("record_def() : def_val = '%s'", def_val);
#endif

	/* pmk style define ID (DEF__*) */
	if (hash_update_dup(ht, def_str, def_val) == false)
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

bool record_def_data(htable_t *ht, char *name, char *value) {
	char	*def_str,
			*have_str,
			 def_val[TMP_BUF_LEN];

	def_str = gen_basic_tag_def(name);

	have_str = gen_ac_tag_name(name);

	if (value != NULL) {
		/* common variable tag HAVE_* */
		if (hash_update_dup(ht, have_str, value) == false)
			return(false);

#ifdef FC_DEBUG
		debugf("record_def_data() : recorded '%s' with '%s'", have_str, value);
#endif

		if (snprintf_b(def_val, sizeof(def_val),
					"#define %s %s",
					have_str, value) == false)
			return(false);
	} else {
		if (snprintf_b(def_val, sizeof(def_val),
					"#undef %s", have_str) == false)
			return(false);
	}

#ifdef FC_DEBUG
	debugf("record_def_data() : def_val = '%s'", def_val);
#endif

	/* pmk style define ID (DEF__*) */
	if (hash_update_dup(ht, def_str, def_val) == false)
		return(false);
#ifdef FC_DEBUG
	debugf("record_def_data() : recorded '%s' with '%s'", def_str, def_val);
#endif

	return(true);
}


/********************
 * record_def_adv() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	boolean
 ***********************************************************************/

bool record_def_adv(htable_t *ht, int type, char *ctn, char *ctt, char *msc, char *val) {
	char	*tag_def,
			*tag_name,
			 buf[TMP_BUF_LEN];

	tag_def = gen_tag_def(type, ctn, ctt, msc);
	tag_name = gen_tag_name(type, ctn, ctt, msc);

	if (val == NULL) {
		/* if value is NULL then undefine tag */
		if (snprintf_b(buf, sizeof(buf), "#undef %s", tag_name) == false) {
			return(false);
		}
	} else {
		/* else value is not NULL so define tag with value */
		if (snprintf_b(buf, sizeof(buf), "#define %s %s", tag_name, val) == false) {
			return(false);
		}

		/* record tag */
		if (hash_update_dup(ht, tag_name, val) == false) {
			return(false);
		}
	}

	/* record tag define */
	if (hash_update_dup(ht, tag_def, buf) == false) {
		return(false);
	}

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

bool process_def_list(htable_t *ht, dynary *da, bool define) {
	char	*name,
			*value;
	int		 i,
			 n;

	if (define == true) {
		value = "1";
	} else {
		value = NULL;
	}

	/* process additional defines */
	if (da != NULL) {
		n = da_usize(da);
		for (i = 0 ; i < n ; i++) {
			name = da_idx(da, i);
			pmk_log("\tProcessing additional define '%s': ", name);
			if (record_def_data(ht, name, value) == false) {
				return(false);
			}
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

bool label_set(htable_t *lht, char *name, bool status) {
	if (hash_update_dup(lht, name, bool_to_str(status)) == false)
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

bool label_check(htable_t *lht, char *name) {
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

bool depend_check(htable_t *lht, pmkdata *gd) {
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

bool require_check(htable_t *pht) {
	pmkobj	*req;

	req = hash_get(pht, KW_OPT_REQUIRED);
	if (req == NULL) {
		/* by default REQUIRED is true if not specified */
		return(true);
	}

	if (po_get_type(req) != PO_BOOL) {
		return(false);
	}

	return(po_get_bool(req));
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

char *get_lang_str(htable_t *pht, pmkdata *pgd) {
	char	*lang;

	/* check first if language has been provided locally */
	lang = (char *)po_get_data(hash_get(pht, KW_OPT_LANG));
	if (lang == NULL) {
		/* get global lang (could be NULL */
		lang = pgd->lang;
	}

	return(lang);
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


/*****************************
 * obsolete_string_to_list() *
 ***********************************************************************
 DESCR
	check if an option has an obsolete type of PO_STRING and replace it
	by PO_LIST type data. Display a warning if data has obosolete type.

 IN
	ht :	options hash table
	opt :	option name

 OUT
	NONE

 NOTE :
	OBSOLETE, to remove later when useless
 ***********************************************************************/

bool obsolete_string_to_list(htable_t *ht, char *opt) {
	char	*pstr;
	dynary	*da;
	pmkobj	*po;

/*debugf("opt = '%s'", opt);*/
	po = hash_get(ht, opt);
	if (po == NULL) {
		return(false);
	}

	if (po_get_type(po) == PO_STRING) {
		/* pstr should not be NULL */
		pstr = po_get_str(po);

		pmk_log("\tWARNING: providing a quoted string to the option '%s' is obsolete.\n", opt);
		pmk_log("\t\tCompatibility will be removed in future releases.\n");

		da = da_init();
		if (da == NULL) {
			errorf("unable to initialise dynary.");
			return(false);
		}

		if (da_push(da, strdup(pstr)) == false) {
			errorf("unable to add '%s' in function list.", pstr);
			return(false);
		}

		po = po_mk_list(da);

		hash_update(ht, opt, po);
	}

	return(true);
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
