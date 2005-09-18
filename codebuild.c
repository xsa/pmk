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
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

void code_bld_init(code_bld_t *pcb, char *blog) {
	/* init struct */
	pcb->header = NULL;
	pcb->library = NULL;
	pcb->define = NULL;
	pcb->procedure = NULL;
	pcb->type = NULL;
	pcb->member = NULL;
	pcb->alt_cflags = NULL;
	pcb->alt_libs = NULL;
	pcb->subhdrs = NULL;

	pcb->blog = blog;
}


/*********************
 * set_header_name() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

void set_header_name(code_bld_t *pcb, char *header) {
	pcb->header = header;
}


/**********************
 * set_library_name() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

void set_library_name(code_bld_t *pcb, char *library) {
	pcb->library = library;
}


/******************
 * set_language() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool set_language(code_bld_t *pcb, char *lang) {
	size_t	i,
			len;

	len = strlen(lang);

	/* search for language data */
	for (i = 0 ; i < nb_lang_data ; i++) {
		/* if language is found */
		if (strncmp(lang_data[i].name, lang, len) == 0) {
			/* set data */
			pcb->lang = lang_data[i].lang;
			pcb->pld = &lang_data[i];
			return(true);
		}
	}

	/* unknown language */
	pcb->pld = NULL;
	return(false);
}


/******************
 * set_compiler() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *set_compiler(code_bld_t *pcb, htable *pht) {
	pcb->pathcomp = hash_get(pht, pcb->pld->compiler);

	return(pcb->pathcomp);
}


/**********************
 * alt_cflags_label() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

void alt_cflags_label(code_bld_t *pcb, char *cflags) {
	pcb->alt_cflags = cflags;
}


/********************
 * alt_libs_label() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

void alt_libs_label(code_bld_t *pcb, char *libs) {
	pcb->alt_libs = libs;
}


/********************
 * get_lang_label() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *get_lang_label(code_bld_t *pcb) {
	return(pcb->pld->name);
}


/**********************
 * get_cflags_label() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *get_cflags_label(code_bld_t *pcb) {
	/* if an alternative cflags variable exists */
	if (pcb->alt_cflags != NULL) {
		/* then return the alternative */
		return(pcb->alt_cflags);
	} else {
		/* else return the default of the used language */
		return(pcb->pld->cflags);
	}
}


/********************
 * get_libs_label() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *get_libs_label(code_bld_t *pcb) {
	/* if an alternative libs variable exists */
	if (pcb->alt_libs != NULL) {
		/* then return the alternative */
		return(pcb->alt_libs);
	} else {
		/* else return the default */
		return("LIBS");	/* XXX TODO use premake.h standard value (smth like PMK_STD_LIBS) */
	}
}


/******************
 * code_builder() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
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
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool c_code_builder(code_bld_t *pcb) {
	FILE	*fp;
	size_t	 i;

	/* open temporary file */
	fp = tmps_open(TEST_FILE_NAME, "w", pcb->srcfile, sizeof(pcb->srcfile), strlen(C_FILE_EXT));
	if (fp == NULL) {
		errorf("c_code_builder: tmps_open() failed");
		return(false); /* failed to open */
	}

	/* main header */
	if (pcb->header != NULL) {
		fprintf(fp, "/* main header to test */\n");
		fprintf(fp, CODE_C_HDR, pcb->header);
	}

	/* sub headers */
	if (pcb->subhdrs != NULL) {
		fprintf(fp, "/* dependency headers */\n");
		for (i = 0 ; i < da_usize(pcb->subhdrs) ; i++) {
			fprintf(fp, da_idx(pcb->subhdrs, i));
		}
	}

	/* main proc */
	fprintf(fp, CODE_C_BEG);

	/* define test */
	if (pcb->define != NULL) {
		fprintf(fp, CODE_C_DEF, pcb->define);
	}

	/* procedure test */
	if (pcb->procedure != NULL) {
		fprintf(fp, CODE_C_PROC, pcb->procedure);
	}

	/* type test */
	if (pcb->type != NULL) {
		if (pcb->member == NULL) {
			/* simple */
			fprintf(fp, CODE_C_VAR, pcb->type);
			fprintf(fp, CODE_C_TYPE);
		} else {
			/* with member */
			fprintf(fp, CODE_C_VAR, pcb->type);
			fprintf(fp, CODE_C_MEMBER, pcb->member);
		}
	}

	fprintf(fp, CODE_C_END);

	fclose(fp);

	return(true);
}


/******************
 * object_build() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool object_builder(char *buf, size_t sz, code_bld_t *pcb, bool dolink) {
	switch (pcb->lang) {
		case LANG_C :
		case LANG_CXX :
			return(c_object_builder(buf, sz, pcb, dolink));
	}

	return(false);
}


/********************
 * c_object_build() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

bool c_object_builder(char *buf, size_t sz, code_bld_t *pcb, bool dolink) {
	/* start with compiler */
	strlcpy(buf, pcb->pathcomp, sz);

	/* if flags are provided */
	if (pcb->cflags != NULL) {
		strlcat(buf, " ", sz);
		strlcat(buf, pcb->cflags, sz);
	}

	/* copy template of object name */
	strlcpy(pcb->binfile, BIN_TEST_NAME, sz);

	/* if we don't link then append object extension */
	if (dolink == false) {
		strlcat(pcb->binfile, ".o", sz);
	}

	/* randomize name */
	mktemp(pcb->binfile);

	/* append the object name */
	strlcat(buf, " -o ", sz);
	strlcat(buf, pcb->binfile, sz);

	/* if an optional library has been provided */
	if (pcb->library != NULL) {
		strlcat(buf, " -l ", sz);
		strlcat(buf, pcb->library, sz);
	}

	/* if we don't link use -c */
	if (dolink == false) {
		strlcat(buf, " -c", sz);
	}

	/* append source filename */
	strlcat(buf, " ", sz);
	strlcat(buf, pcb->srcfile, sz);

	/* append log redirection */
	strlcat(buf, " >>", sz);
	strlcat(buf, pcb->blog, sz);
	if (strlcat_b(buf, " 2>&1", sz) == false) {
		return(false);
	}

	return(true);
}


/****************
 * cb_cleaner() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

void cb_cleaner(code_bld_t *pcb) {
	if (unlink(pcb->srcfile) == -1) {
		/* cannot remove temporary source file */
		errorf("cannot remove source file '%s'", pcb->srcfile);
	}

	/* No need to check return here as binary could not exists */
	unlink(pcb->binfile);
}

