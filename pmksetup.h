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

#ifndef _PMKSETUP_H_
#define _PMKSETUP_H_

#include "hash.h"	 
#include "premake.h"


/* pmksetup specific version */
#define PREMAKE_SUBVER_PMKSETUP	"5"

#define PREMAKE_CONFIG_TMP	"/tmp/pmk.XXXXXXXX"

/* define printf format in pmk.conf */
#define PMKSTP_WRITE_FORMAT	"%s %c \"%s\"\n"

/* buffer size in the copy_config() function */
#define MAX_LINE_BUF		MAX_LINE_LEN

#define MAX_CONF_OPT		256	/* XXX temp maximum hash entries */
#define PATH_STR_DELIMITER	':'	/* string delimiter of the $PATH variable */    


/*
 * Look for location of some predefined binaries.
 * Be sure to update this list in premake.h as well.
 */
#define MAXBINS			13	/* max slots in the binaries array */
static  char *binaries[MAXBINS][2] = {
	{"ar",		PMKCONF_BIN_AR},
	{"awk",		PMKCONF_BIN_AWK},
	{"c++",		PMKCONF_BIN_CXX},
	{"cat",		PMKCONF_BIN_CAT},
	{"cpp",		PMKCONF_BIN_CPP},
	{"egrep",	PMKCONF_BIN_EGREP},
	{"grep",	PMKCONF_BIN_GREP},
	{"install",	PMKCONF_BIN_INSTALL},
	{"pkg-config",  PMKCONF_BIN_PKGCONFIG},
	{"ranlib",	PMKCONF_BIN_RANLIB},
	{"sh",		PMKCONF_BIN_SH},
	{"strip",	PMKCONF_BIN_STRIP},
	{"tar",		PMKCONF_BIN_TAR}
};


/* Local functions declaration */
bool	check_opt(htable *, prsopt *);
int	open_tmp_config(void);
int	close_tmp_config(void);
int	get_env_vars(htable *);
int	get_binaries(htable *);
int	predef_vars(htable *);
bool	byte_order_check(htable *pht);
int	copy_config(const char *, const char *);
int	keycomp(const void *, const void *);
void	char_replace(char *, const char, const char);
void	write_new_data(htable *);
void	verbosef(const char *fmt, ...);
void	usage(void);

#endif	/* _PMKSETUP_H_ */
