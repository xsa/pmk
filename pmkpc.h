/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
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


#ifndef _PMKPC_H_
#define _PMKPC_H_


/**********
 constants
************************************************************************/

/* options id */
#define PMKPC_OPT_UNKNOWN		0
#define PMKPC_OPT_VERSION		1
#define PMKPC_OPT_ATLPKGVERS		2
#define PMKPC_OPT_EXISTS		3
#define PMKPC_OPT_LISTALL		4
#define PMKPC_OPT_UNINST		5
#define PMKPC_OPT_DEBUG			6
#define PMKPC_OPT_HELP			7
#define PMKPC_OPT_USAGE			8

#define PMKPC_OPT_MODVERS		10
#define PMKPC_OPT_ATLVERS		11
#define PMKPC_OPT_EXTVERS		12
#define PMKPC_OPT_MAXVERS		13

#define PMKPC_OPT_CFLAGS		20
#define PMKPC_OPT_CFLAGS_ONLY_PATH	21
#define PMKPC_OPT_CFLAGS_ONLY_OTHER	22

#define PMKPC_OPT_LIBS			30
#define PMKPC_OPT_LIBS_ONLY_LIB		31
#define PMKPC_OPT_LIBS_ONLY_PATH	32
#define PMKPC_OPT_LIBS_ONLY_OTHER	33

#define PMKPC_OPT_VAR			40
#define PMKPC_OPT_VAR_DEF		41
#define PMKPC_OPT_VAR_PRNT		42
#define PMKPC_OPT_VAR_SILC		43
#define PMKPC_OPT_VAR_STDO		44


/* compatibility version with pkg-config */
#define PMKPC_COMPAT_VERSION	"0.15.0"

/* separator for option's argument */
#define PMKPC_ARG_SEP		'='

/* maximum option size */
#define PMKPC_MAX_OPT_SIZE	64

/* usage related defines */
#define PMKPC_USAGE_STR		"Usage: pmkpc"
#define PMKPC_USAGE_OPEN_OPT	" [--"
#define PMKPC_USAGE_CLOSE_OPT	"]"
#define PMKPC_USAGE_ALIGN	"        "

/* string to append to options in the usage() that need it */
#define PC_USAGE_VERSION	"VERSION"
#define PC_USAGE_VARNAME	"VARIABLENAME"
#define PC_USAGE_VARVAL		"VARIABLENAME=VARIABLEVALUE"


/*********************************
 types and structures definitions
************************************************************************/

typedef struct {
	char		*name;
	bool		 arg;
	unsigned int	 id;
	char		*usagearg;
} pcopt;

typedef struct {
	unsigned int	 idx,
			 id;
	char		*arg,
			*err;
} optcell;

typedef struct {
	dynary	*pda;
	htable	*pht;
	pkgdata	*ppd;
} pcdata;


/********************
 function prototypes
************************************************************************/

bool	 list_all(pkgdata *);
bool	 pcgetopt(unsigned int, char **, optcell *);
optcell	*optcell_init(void);
void	 clean(pcdata *);
void	 optcell_destroy(optcell *);
void	 usage(void);

#endif /* _PMKPKGCFG_H_ */
