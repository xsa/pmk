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


#ifndef _PMK_PKGCONFIG_H_
#define _PMK_PKGCONFIG_H_

#define PKGCONFIG_DIR		"/usr/local/lib/pkgconfig" /* XXX XXX XXX hardcode !!!!! should use prefix */

#define PKGCFG_HT_SIZE	512

typedef struct s_pkgcell {
	char	*name,
		*descr,
		*version,
		*requires;
	dynary	*cflags,
		*libs;
	htable	*variables;
} pkgcell;

typedef struct s_pkgdata {
	htable	*files,
		*cells;
	dynary	*mods;
} pkgdata;

typedef struct s_pkgkw {
	char		*kw_name;
	unsigned int	 kw_id;
} pkgkw;


/* functions protos */
pkgcell	*pkgcell_init();
void	 pkgcell_destroy(pkgcell *);
pkgdata	*pkgdata_init();
void	 pkgdata_destroy(pkgdata *);
bool	 scan_dir(char *, pkgdata *);
bool	 parse_keyword(pkgcell *, char *, char *);
char	*process_variables(char *, htable *);
pkgcell	*parse_pc_file(char *);
pkgcell	*pkg_cell_add(pkgdata *ppd, char *mod);
bool	 pkg_recurse(pkgdata *, char *);
char	*pkg_get_cflags(pkgdata *);
char	*pkg_get_libs(pkgdata *);
bool	 pkg_mod_exists(pkgdata *ppd, char *mod);

#endif /* _PMK_PKGCONFIG_H_ */

