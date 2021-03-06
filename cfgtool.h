/* $Id$ */

/*
 * Copyright (c) 2006 Damien Couderc
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


#ifndef _PMK_CFGTOOL_H_
#define _PMK_CFGTOOL_H_

#include "hash_tools.h"


/*************
 * constants *
 ***********************************************************************/

#define CFGT_TOK_ADDCT	1

#ifndef DATADIR
/* for lint */
#define DATADIR	"/DATADIR_not_defined"
#endif

#define PMKCFG_DATA		DATADIR "/pmkcfgtool.dat"

/* version format line: tool_path version_option */
#define CT_FORMAT_VERSION	"%s %s 2>/dev/null"
/* data gathering format line: tool_path data_option */
#define CT_FORMAT_DATA		"%s %s 2>/dev/null"
/* module gathering format line: tool_path data_option module */
#define CT_FORMAT_DATA_MOD	"%s %s %s 2>/dev/null"

#define CFGTOOL_HT_SIZE	32

#define CFGTOOL_OPT_VERSION	"--version"
#define CFGTOOL_OPT_CFLAGS	"--cflags"
#define CFGTOOL_OPT_LIBS	"--libs"


/********************
 * type definitions *
 ***********************************************************************/

typedef struct {
	char	*name,
			*binary,
			*version,
			*module,
			*cflags,
			*libs;
} cfgtcell;


typedef struct {
	htable_t	*by_mod,
				*by_bin;
} cfgtdata;


/**************
 * prototypes *
 ***********************************************************************/

void		 cfgtcell_destroy(cfgtcell *);
cfgtdata	*cfgtdata_init(void);
void		 cfgtdata_destroy(cfgtdata *);

bool		 add_cfgtool(cfgtdata *, htable_t *);
cfgtdata	*parse_cfgt_file(void);
bool		 cfgtcell_get_binary(cfgtdata *, char *, char *, size_t);
cfgtcell	*cfgtcell_get_cell(cfgtdata *, char *);

bool		 ct_get_version(char *, char *, char *, size_t);
bool		 ct_get_data(char *, char *, char *, char *, size_t);

#endif /* _PMK_CFGTOOL_H_ */

