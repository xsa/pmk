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


#ifndef _PMK_H_
#define _PMK_H_

#include "dynarray.h"
#include "hash.h"
#include "premake.h"


/* pmk specific version */
#define PREMAKE_SUBVER_PMK	"7"

/* character to assign a value to a key */
#define PMK_KEY_CHAR		'='

/* character used as delimiter for template tags */
#define PMK_TAG_CHAR		'@'

/* maximal number of options per command */
#define MAX_CMD_OPT		32

/* maximal number of templates */
#define MAX_TEMPLATES		32

/* maximal number of key in data hash */
#define MAX_DATA_KEY		1024

/* maximal number of key in label hash */
#define MAX_LABEL_KEY		1024

/* pmk's directory tags */
#define PMK_DIR_BLD_ABS "builddir_abs"
#define PMK_DIR_BLD_REL "builddir_rel"
#define PMK_DIR_BLD_ROOT_ABS "builddir_root_abs"
#define PMK_DIR_BLD_ROOT_REL "builddir_root_rel"
#define PMK_DIR_SRC_ABS "srcdir_abs"
#define PMK_DIR_SRC_REL "srcdir_rel"
#define PMK_DIR_SRC_ROOT_ABS "srcdir_root_abs"
#define PMK_DIR_SRC_ROOT_REL "srcdir_root_rel"

#define PMK_TMP_AC_CONF TMPDIR "/pmk_ac_XXXXXXXX"

/* build logs */
#define PMK_BUILD_LOG	"pmk_build.log"

/* command option type */
typedef struct {
	char	name[OPT_NAME_LEN],
		value[OPT_VALUE_LEN];
} pmkcmdopt;

/* command type */
typedef struct {
	int	 token;
	char	*label;
} pmkcmd;

/* pmk data */
typedef struct {
	htable	*htab,
		*labl;
	dynary	*tlist;
	char	*ac_file,
		*lang,
		 basedir[MAXPATHLEN],
		 srcdir[MAXPATHLEN],
		 pmkfile[MAXPATHLEN],
		 ovrfile[MAXPATHLEN],
		 buildlog[MAXPATHLEN],
		 errmsg[MAX_ERR_MSG_LEN];
} pmkdata;


#endif /* _PMK_H_ */
