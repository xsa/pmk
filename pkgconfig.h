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

#include "pmk.h"

#ifndef DATADIR
/* for lint */
#define DATADIR	"./data"
#endif

#define PMKCFG_DATA		DATADIR "/pmkcfgtool.dat"
#define PMKCFG_ENV_PATH		"PKG_CONFIG_PATH"
#define PMKCFG_ENV_LIBDIR	"PKG_CONFIG_LIBDIR"

#define PKGCFG_CHAR_PATH_SEP	':'

#define PKGCFG_HT_SIZE	512
#define CFGTOOL_HT_SIZE	32

#define CFGTOOL_OPT_VERSION	"--version"
#define CFGTOOL_OPT_CFLAGS	"--cflags"
#define CFGTOOL_OPT_LIBS	"--libs"

/* packages strucutres */

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


/* config tool structures */

typedef struct {
	char	*name,
		*binary,
		*version,
		*module,
		*cflags,
		*libs;
} cfgtcell;


typedef struct {
	htable	*by_mod,
		*by_bin;
} cfgtdata;

/* functions protos */
pkgcell		*pkgcell_init();
void		 pkgcell_destroy(pkgcell *);
pkgdata		*pkgdata_init();
void		 pkgdata_destroy(pkgdata *);

void		 cfgtcell_destroy(cfgtcell *);
cfgtdata	*cfgtdata_init();
void		 cfgtdata_destroy(cfgtdata *);

bool		 scan_dir(char *, pkgdata *);
bool		 pkg_collect(char *, pkgdata *);
bool		 parse_keyword(pkgcell *, char *, char *);
char		*process_variables(char *, htable *);
pkgcell		*parse_pc_file(char *);
pkgcell		*pkg_cell_add(pkgdata *ppd, char *mod);
bool		 pkg_recurse(pkgdata *, char *);
char		*pkg_get_cflags(pkgdata *);
char		*pkg_get_libs(pkgdata *);
bool		 pkg_mod_exists(pkgdata *ppd, char *mod);

bool		 add_cfgtool(cfgtdata *, htable *);
cfgtdata	*parse_cfgt_file();
bool		 cfgtcell_get_binary(pmkdata *, char *, char *, size_t);
cfgtcell	*cfgtcell_get_cell(pmkdata *, char *);

#endif /* _PMK_PKGCONFIG_H_ */

