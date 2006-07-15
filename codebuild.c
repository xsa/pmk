/* $Id$ */

/*
 * Copyright (c) 2005 Damien Couderc
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
#include <unistd.h>

#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"

#include "codebuild.h"
#include "common.h"


/*****************
 * language data *
 ***********************************************************************/

lgdata_t	lang_data[] = { /* XXX TODO use defines */
	{"C",	PMKCONF_BIN_CC,		"CFLAGS",	"SLCFLAGS",		LANG_C},
	{"C++",	PMKCONF_BIN_CXX,	"CXXFLAGS",	"SLCXXFLAGS",	LANG_CXX}
};
size_t		nb_lang_data = sizeof(lang_data) / sizeof(lgdata_t);


/*******************
 * code_bld_init() *
 ***********************************************************************
 DESCR
	initialize code build structure

 IN
	pcb :	code build data structure
	blog :	build log file name

 OUT
	NONE
 ***********************************************************************/

void code_bld_init(code_bld_t *pcb, char *blog) {
	/* init struct */
	pcb->header = NULL;
	pcb->library = NULL;
	pcb->define = NULL;
	pcb->procedure = NULL;
	pcb->type = NULL;
	pcb->member = NULL;
	pcb->pathcomp = NULL;
	pcb->flags = NULL;
	pcb->alt_cflags = NULL;
	pcb->alt_libs = NULL;
	pcb->subhdrs = NULL;

	pcb->blog = blog;
}


/*********************
 * verify_language() *
 ***********************************************************************
 DESCR
	verify support for the given language

 IN
	lang :	language identifier string

 OUT
	boolean
 ***********************************************************************/

int verify_language(char *lang) {
	size_t	i,
			len;

	len = strlen(lang);

	/* search for language data */
	for (i = 0 ; i < nb_lang_data ; i++) {
		/* if language is found */
		if (strncmp(lang_data[i].name, lang, len) == 0) {
			/* return index of the language cell */
			return(i);
		}
	}

	/* unknown language */
	return(UNKNOWN_LANG);
}


/******************
 * set_language() *
 ***********************************************************************
 DESCR
	set the language used for the code building

 IN
	pcb :	code build data structure
	lang :	language identifier string

 OUT
	boolean
 ***********************************************************************/

bool set_language(code_bld_t *pcb, char *lang) {
	int	 i;

	/* verify if the language is supported */
	i = verify_language(lang);
	if (i == UNKNOWN_LANG) {
		/* unknown language */
		pcb->pld = NULL;
		return(false);
	}

	/* language found */
	pcb->lang = lang_data[i].lang;
	pcb->pld = &lang_data[i];
	return(true);
}


/******************
 * set_compiler() *
 ***********************************************************************
 DESCR
	set the path to the compiler to be used for building

 IN
	pcb :	code build data structure
	pht :	XXX

 OUT
	returns the compiler path
 ***********************************************************************/

char *set_compiler(code_bld_t *pcb, htable *pht) {
	pcb->pathcomp = hash_get(pht, pcb->pld->compiler);

	return(pcb->pathcomp);
}


/********************
 * get_lang_label() *
 ***********************************************************************
 DESCR
	get the label of the language used for building

 IN
	pcb :	code build data structure

 OUT
	returns the language label string
 ***********************************************************************/

char *get_lang_label(code_bld_t *pcb) {
	return(pcb->pld->name);
}


/************************
 * get_compiler_label() *
 ***********************************************************************
 DESCR
	get the label of the language used for building

 IN
	pcb :	code build data structure

 OUT
	returns the language label string
 ***********************************************************************/

char *get_compiler_label(code_bld_t *pcb) {
	return(pcb->pld->compiler);
}


/**********************
 * get_cflags_label() *
 ***********************************************************************
 DESCR
	get the compiler flags variable name (can be default or alternative)

 IN
	pcb :	code build data structure

 OUT
	returns the variable name
 ***********************************************************************/

char *get_cflags_label(code_bld_t *pcb) {
	/* if an alternative cflags variable name exists */
	if (pcb->alt_cflags != NULL) {
		/* then return the alternative name */
		return(pcb->alt_cflags);
	} else {
		/* else return the default name of the used language */
		return(pcb->pld->cflags);
	}
}


/********************
 * get_libs_label() *
 ***********************************************************************
 DESCR
	get the linker flags variable name (can be default or alternative)

 IN
	pcb :	code build data structure

 OUT
	returns the variable name
 ***********************************************************************/

char *get_libs_label(code_bld_t *pcb) {
	/* if an alternative libs variable name exists */
	if (pcb->alt_libs != NULL) {
		/* then return the alternative name */
		return(pcb->alt_libs);
	} else {
		/* else return the default name */
		return("LIBS");	/* XXX TODO use premake.h standard value (smth like PMK_STD_LIBS) */
	}
}


/*****************
 * code_logger() *
 ***********************************************************************
 DESCR
	write code in the temporary source file and log it

 IN
	tfp :	temporary source file descriptor
	lfp :	log file descriptor
	fmt :	message format string

 OUT
	NONE

 NOTE
	TODO : check returned values XXX
 ***********************************************************************/


void code_logger(FILE *tfp, FILE *lfp, const char *fmt, ...) {
	va_list	plst;

	va_start(plst, fmt);
	vfprintf(tfp, fmt, plst);
	vfprintf(lfp, fmt, plst);
	va_end(plst);
}


/******************
 * code_builder() *
 ***********************************************************************
 DESCR
	code building wrapper

 IN
	pcb :	code build data structure

 OUT
	boolean
 ***********************************************************************/

bool code_builder(code_bld_t *pcb) {
	switch (pcb->lang) {
		case LANG_C :
		case LANG_CXX :
			return(c_code_builder(pcb));
	}

	return(false);
}


/********************
 * c_code_builder() *
 ***********************************************************************
 DESCR
	C language code building

 IN
	pcb :	code build data structure

 OUT
	boolean
 ***********************************************************************/

bool c_code_builder(code_bld_t *pcb) {
	FILE	*tfp,
			*lfp;
	size_t	 i;

	/* open temporary file */
	tfp = tmps_open(TEST_FILE_NAME, "w", pcb->srcfile, sizeof(pcb->srcfile), strlen(C_FILE_EXT));
	if (tfp == NULL) {
		errorf("c_code_builder: tmps_open() failed");
		return(false); /* failed to open */
	}

	lfp = fopen(pcb->blog, "a");
	if (lfp == NULL) {
		errorf("c_code_builder: build log fopen() failed");
		return(false); /* failed to open */
	}

	fprintf(lfp, "Generated source file:\n");

	/* sub headers */
	if (pcb->subhdrs != NULL) {
		code_logger(tfp, lfp, "/* dependency headers */\n");
		for (i = 0 ; i < da_usize(pcb->subhdrs) ; i++) {
			code_logger(tfp, lfp, CODE_C_HDR, (char *) da_idx(pcb->subhdrs, i));
		}
	}

	/* main header */
	if (pcb->header != NULL) {
		code_logger(tfp, lfp, "/* main header to test */\n");
		code_logger(tfp, lfp, CODE_C_HDR, pcb->header);
	}

	/* main proc */
	code_logger(tfp, lfp, CODE_C_BEG);

	/* define test */
	if (pcb->define != NULL) {
		code_logger(tfp, lfp, CODE_C_DEF, pcb->define);
	}

	/* procedure test */
	if (pcb->procedure != NULL) {
		code_logger(tfp, lfp, CODE_C_PROC, pcb->procedure);
	}

	/* type test */
	if (pcb->type != NULL) {
		if (pcb->member == NULL) {
			/* simple */
			code_logger(tfp, lfp, CODE_C_VAR, pcb->type);
			code_logger(tfp, lfp, CODE_C_TYPE);
		} else {
			/* with member */
			code_logger(tfp, lfp, CODE_C_VAR, pcb->type);
			code_logger(tfp, lfp, CODE_C_MEMBER, pcb->member);
		}
	}

	code_logger(tfp, lfp, CODE_C_END);

	fclose(tfp);
	fclose(lfp);

	if (ferror(tfp) != 0) {
		errorf("c_code_builder: generated source I/O failure");
		return(false);
	}

	if (ferror(lfp) != 0) {
		errorf("c_code_builder: build log I/O failure");
		return(false);
	}

	return(true);
}


/********************
 * shared_builder() *
 ***********************************************************************
 DESCR
	shared code building wrapper

 IN
	pcb :	code build data structure

 OUT
	boolean
 ***********************************************************************/

bool shared_builder(code_bld_t *pcb) {
	switch (pcb->lang) {
		case LANG_C :
		case LANG_CXX :
			return(c_shared_builder(pcb));
	}

	return(false);
}


/**********************
 * c_shared_builder() *
 ***********************************************************************
 DESCR
	C shared code builder

 IN
	pcb :	code build data structure

 OUT
	boolean
 ***********************************************************************/

bool c_shared_builder(code_bld_t *pcb) {
	FILE	*tfp,
			*lfp;

	/* open temporary file */
	tfp = tmps_open(TEST_FILE_NAME, "w", pcb->srcfile, sizeof(pcb->srcfile), strlen(C_FILE_EXT));
	if (tfp == NULL) {
		errorf("c_shared_builder: tmps_open() failed");
		return(false); /* failed to open */
	}

	lfp = fopen(pcb->blog, "a");
	if (lfp == NULL) {
		errorf("c_shared_builder: build log fopen() failed");
		return(false); /* failed to open */
	}

	fprintf(lfp, "Generated source file:\n");

	/* shared function */
	code_logger(tfp, lfp, CODE_C_SHARED);

	fclose(tfp);
	fclose(lfp);

	if (ferror(tfp) != 0) {
		errorf("c_shared_builder: generated source I/O failure");
		return(false);
	}

	if (ferror(lfp) != 0) {
		errorf("c_shared_builder: build log I/O failure");
		return(false);
	}

	return(true);
}


/*********************
 * cmdline_builder() *
 ***********************************************************************
 DESCR
	wrapper for compiler command line building

 IN
	pcb :		code build data structure
	dolink :	linking indicator

 OUT
	boolean
 ***********************************************************************/

bool cmdline_builder(code_bld_t *pcb, bool dolink) {
	FILE	*lfp;
	bool	 r = false;
	
	switch (pcb->lang) {
		case LANG_C :
		case LANG_CXX :
			r = c_cmdline_builder(pcb, dolink);
	}

	if (r == true) {
		lfp = fopen(pcb->blog, "a");
		if (lfp == NULL) {
			errorf("c_code_builder: build log fopen() failed");
			return(false); /* failed to open */
		}
	
		fprintf(lfp, "Generated command line:\n");
		fprintf(lfp, "%s\n", pcb->bldcmd);

		fclose(lfp);

		if (ferror(lfp) != 0) {
			errorf("c_code_builder: build log I/O failure");
			return(false);
		}
	}

	return(r);
}


/***********************
 * c_cmdline_builder() *
 ***********************************************************************
 DESCR
	build C compiler command line

 IN
	pcb :		code build data structure
	dolink :	linking indicator

 OUT
	boolean
 ***********************************************************************/

bool c_cmdline_builder(code_bld_t *pcb, bool dolink) {
	/* start with compiler */
	if (pcb->pathcomp == NULL) {
		return(false);
	}
	strlcpy(pcb->bldcmd, pcb->pathcomp, sizeof(pcb->bldcmd));

	/* if flags are provided */
	if (pcb->flags != NULL) {
		strlcat(pcb->bldcmd, " ", sizeof(pcb->bldcmd));
		strlcat(pcb->bldcmd, pcb->flags, sizeof(pcb->bldcmd));
	}

	/* copy template of object name */
	strlcpy(pcb->binfile, BIN_TEST_NAME, sizeof(pcb->binfile));

	/* if we don't link then append object extension */
	if (dolink == false) {
		strlcat(pcb->binfile, ".o", sizeof(pcb->binfile));
	}

	/* randomize name */
	mktemp(pcb->binfile);

	/* append the object name */
	strlcat(pcb->bldcmd, " -o ", sizeof(pcb->bldcmd));
	strlcat(pcb->bldcmd, pcb->binfile, sizeof(pcb->bldcmd));

	/* if an optional library has been provided */
	if (pcb->library != NULL) {
		strlcat(pcb->bldcmd, " -l ", sizeof(pcb->bldcmd));
		strlcat(pcb->bldcmd, pcb->library, sizeof(pcb->bldcmd));
	}

	/* if we don't link use -c */
	if (dolink == false) {
		strlcat(pcb->bldcmd, " -c", sizeof(pcb->bldcmd));
	}

	/* append source filename */
	strlcat(pcb->bldcmd, " ", sizeof(pcb->bldcmd));
	strlcat(pcb->bldcmd, pcb->srcfile, sizeof(pcb->bldcmd));

	/* append log redirection */
	strlcat(pcb->bldcmd, " >>", sizeof(pcb->bldcmd));
	strlcat(pcb->bldcmd, pcb->blog, sizeof(pcb->bldcmd));
	if (strlcat_b(pcb->bldcmd, " 2>&1", sizeof(pcb->bldcmd)) == false) {
		return(false);
	}

	return(true);
}


/********************
 * object_builder() *
 ***********************************************************************
 DESCR
	execute object building command line

 IN
	compiler command line builder

 OUT
	boolean
 ***********************************************************************/

bool object_builder(code_bld_t *pcb) {
	if (system(pcb->bldcmd) == 0) {
		return(true);
	} else {
		return(false);
	}
}


/****************
 * cb_cleaner() *
 ***********************************************************************
 DESCR
	clean code builder files (temporary source and eventually generated
	binary)

 IN
	compiler command line builder

 OUT
	NONE
 ***********************************************************************/

void cb_cleaner(code_bld_t *pcb) {
	if (unlink(pcb->srcfile) == -1) {
		/* cannot remove temporary source file */
		errorf("cannot remove source file '%s'", pcb->srcfile);
	}

	/* No need to check return here as binary could not exists */
	unlink(pcb->binfile);
}


/**********************
 * check_so_support() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool check_so_support(char *buildlog, htable *htab) {
	char		*lang;
	code_bld_t	 scb;

	code_bld_init(&scb, buildlog);
	lang = "C"; /* XXX TODO language relative to the compiler */
	set_language(&scb, lang);
	set_compiler(&scb, htab); /* XXX check */

	pmk_log("\t\tCompiling shared objet : ");
	
	if (shared_builder(&scb) == false) {
		/* XXX failed */
		/*debugf("shared_builder = false");*/
		pmk_log("Failed\n");
		return(false);
	} else {
		/*debugf("shared_builder = true");*/
		/* XXX set shared stuff variables */
		if (cmdline_builder(&scb, false) == false) {
			/* XXX failed */
			/*debugf("cmdline_builder = false");*/
			pmk_log("Failed\n");
			return(false);
		} else {
			/*debugf("cmdline_builder = true");*/
			if (object_builder(&scb) == false) {
				/* XXX */
				/*debugf("object_builder = false");*/
				pmk_log("Failed\n");
				return(false);
			} else {
				/* XXX */
				/*debugf("object_builder = true");*/
				pmk_log("Succeeded\n");
			}
		}
	}

	/* try linking shared object */
	pmk_log("\t\tLinking shared objet : X_TODO_X\n");

	cb_cleaner(&scb);

	return(false);
}


/*********************************
 * obsolete_get_lang_from_comp() *
 ***********************************************************************
 DESCR
	provide compiler path

 IN
	compname :	compiler name from lgdata structure

 OUT
	language name

 NOTE :
	OBSOLETE, to remove later when useless
 ***********************************************************************/

char *obsolete_get_lang_from_comp(char *compname) {
	char	key[OPT_NAME_LEN];
	size_t	i,
			len;

	/* failed to build compiler label */
	if (snprintf_b(key, sizeof(key), "BIN_%s", compname) == false) {
		return(NULL);
	}

	/* length of the generated string */
	len = strlen(key);

	/* search for matching compiler label */
	for (i = 0 ; i < nb_lang_data ; i++) {
		/* if compiler is found */
		if (strncmp(lang_data[i].compiler, key, len) == 0) {
			/* return the language name */
			return(lang_data[i].name);
		}
	}

	/* unknown compiler */
	return(NULL);
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
