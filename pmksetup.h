/* $Id$ */

/*
 * Copyright (c) 2003 Xavier Santolaria 
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

#ifndef _PMKSETUP_H_
#define _PMKSETUP_H_

#include "hash.h"	 
#include "premake.h"


#define PREMAKE_CONFIG_TMP	"/tmp/pmk.XXXXXXXX"

/* buffer size in the copy_config() function */
#define MAX_LINE_BUF		MAX_LINE_LEN

#define MAX_CONF_OPT		256	/* XXX temp maximum hash entries */
#define PATH_STR_DELIMITER	':'	/* string delimiter of the $PATH variable */    


/* Look for location of some predefined binaries */
#define MAXBINS			6	/* max slots in the binaries array */
static  char *binaries[MAXBINS][2] = {
	{"ar",		PREMAKE_KEY_BIN_AR},
	{"c++",		PREMAKE_KEY_BIN_CXX},
	{"cat",		PREMAKE_KEY_BIN_CAT},
	{"grep",	PREMAKE_KEY_BIN_GREP},
	{"install",	PREMAKE_KEY_BIN_INSTALL},
	{"ranlib",	PREMAKE_KEY_BIN_RANLIB}
};


/* Local functions declaration */
int	parse_conf(FILE *, htable *);
int	open_tmp_config(void);
int	close_tmp_config(void);
int	get_env_vars(htable *);
int	get_binaries(htable *);
int	predef_vars(htable *);
int	copy_config(const char *, const char *);
int	keycomp(const void *, const void *);
void	char_replace(char *, const char, const char);
void	write_new_data(htable *);
void	verbosef(const char *fmt, ...);
void	usage(void);

#endif	/* _PMKSETUP_H_ */
