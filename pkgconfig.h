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


#ifndef _PMK_PKGCONFIG_H_
#define _PMK_PKGCONFIG_H_

#include "dynarray.h"
#include "hash.h"


/**********
 constants
************************************************************************/

#define PMKCFG_ENV_PATH		"PKG_CONFIG_PATH"
#define PMKCFG_ENV_LIBDIR	"PKG_CONFIG_LIBDIR"

#define PKGCFG_CHAR_PATH_SEP	':'

#define PKGCFG_HT_SIZE	512

#define VERSION_CHAR_SEP	'.'


/* specific flags */
#define PKGCFG_CFLAGS_I		1
#define PKGCFG_CFLAGS_o		2
#define PKGCFG_CFLAGS_ALL	PKGCFG_CFLAGS_I | PKGCFG_CFLAGS_o

#define PKGCFG_LIBS_L		1
#define PKGCFG_LIBS_l		2
#define PKGCFG_LIBS_o		4
#define PKGCFG_LIBS_ALL		PKGCFG_LIBS_L | PKGCFG_LIBS_l | PKGCFG_LIBS_o


/**********
 constants
************************************************************************/

/* package structures */

typedef struct {
	char	*name,
		*descr,
		*version,
		*requires;
	dynary	*cflags,
		*libs;
	htable	*variables;
} pkgcell;

typedef struct {
	htable	*files,
		*cells;
	dynary	*mods;
} pkgdata;

typedef struct {
	char		*kw_name;
	unsigned int	 kw_id;
} pkgkw;


/********************
 function prototypes
************************************************************************/

pkgcell		*pkgcell_init(void);
void		 pkgcell_destroy(pkgcell *);
pkgdata		*pkgdata_init(void);
void		 pkgdata_destroy(pkgdata *);

char		*skip_blank(char *);
bool		 scan_dir(char *, pkgdata *);
bool		 pkg_collect(char *, pkgdata *);
bool		 parse_keyword(pkgcell *, char *, char *);
char		*process_variables(char *, htable *);
pkgcell		*parse_pc_file(char *);
pkgcell		*pkg_cell_add(pkgdata *, char *);
bool		 pkg_recurse(pkgdata *, char *);
char		*pkg_single_append(char *, char *);
char		*pkg_get_cflags(pkgdata *);
char		*pkg_get_cflags_adv(pkgdata *, unsigned int);
char		*pkg_get_libs(pkgdata *);
char		*pkg_get_libs_adv(pkgdata *, unsigned int);
bool		 pkg_mod_exists(pkgdata *, char *);

int			 compare_version(char *, char *);


#endif /* _PMK_PKGCONFIG_H_ */
