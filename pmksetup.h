/* $Id$ */

/*
 * Copyright (c) 2003-2004 Xavier Santolaria <xavier@santolaria.net>
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

#ifndef _PMKSETUP_H_
#define _PMKSETUP_H_

#include "sys/stat.h"
#include "hash.h"
#include "premake.h"


/* pmksetup specific version */
#define PREMAKE_SUBVER_PMKSETUP	"7"

#define PREMAKE_CONFIG_TMP	PREMAKE_TMP_DIR "/pmk.conf_XXXXXXXX"
#define PREMAKE_CONFIG_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH


#ifndef DATADIR
/* for lint */
#define DATADIR	"/DATADIR_not_defined"
#endif

/* set default priviledged user */
#ifndef PRIVSEP_USER
#define PRIVSEP_USER	"nobody"
#endif


#define PMKCPU_DATA		DATADIR "/pmkcpu.dat"

/* define printf format in pmk.conf */
#define PMKSTP_WRITE_FORMAT	"%s %c \"%s\"\n"

#define PMKSTP_REC_REMV		'r'
#define PMKSTP_REC_UPDT		'u'

/* buffer size in the copy_config() function */
#define MAX_LINE_BUF		MAX_LINE_LEN

#define MAX_CONF_OPT		256	/* XXX temp maximum hash entries */

#define ECHO_CMD	"echo \"one\\c\"; echo -n two; echo three"
#define	ECHO_EMPTY	""
#define	ECHO_N		"-n"
#define	ECHO_C		"\\c"
#define	ECHO_NL		"\\n"
#define	ECHO_HT		"\\t"


#define EMSG_PRIV_FMT	"Failed to change privilege (%s)"

/*
 * Look for location of some predefined binaries.
 * Be sure to update this list in premake.h as well.
 */

static  char *binaries[][2] = {
	{"ar",		PMKCONF_BIN_AR},
	{"as",		PMKCONF_BIN_AS},
	{"awk",		PMKCONF_BIN_AWK},
	{"c++",		PMKCONF_BIN_CXX},
	{"cat",		PMKCONF_BIN_CAT},
	{"cpp",		PMKCONF_BIN_CPP},
	{"echo",	PMKCONF_BIN_ECHO},
	{"egrep",	PMKCONF_BIN_EGREP},
	{"grep",	PMKCONF_BIN_GREP},
	{"id",		PMKCONF_BIN_ID},
	{"install",	PMKCONF_BIN_INSTALL},
	{"lex",		PMKCONF_BIN_LEX},
	{"ln",		PMKCONF_BIN_LN},
	{"pkg-config",  PMKCONF_BIN_PKGCONFIG},
	{"ranlib",	PMKCONF_BIN_RANLIB},
	{"sh",		PMKCONF_BIN_SH},
	{"strip",	PMKCONF_BIN_STRIP},
	{"sudo",	PMKCONF_BIN_SUDO},
	{"tar",		PMKCONF_BIN_TAR},
	{"yacc",	PMKCONF_BIN_YACC}
};

#define MAXBINS	sizeof(binaries) / sizeof(char *) / 2


bool	record_data(htable *, char *, char, char *);
bool	gather_data(htable *);
bool	check_opt(htable *, prsopt *);
bool	open_tmp_config(void);
bool	close_tmp_config(void);
bool	get_env_vars(htable *);
bool	get_binaries(htable *);
bool	predef_vars(htable *);
bool	check_echo(htable *);
bool	check_libpath(htable *);
bool	get_cpu_data(htable *);
bool	dir_exists(const char *);
bool	byte_order_check(htable *);
int	keycomp(const void *, const void *);
bool	write_new_data(htable *);
void	verbosef(const char *, ...);
void	usage(void);
bool	detection_loop(int, char *[]);
void	child_loop(uid_t, gid_t, int, char *[]);
void	parent_loop(pid_t);

#endif	/* _PMKSETUP_H_ */
