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

#ifndef SYSCONFDIR
#	define SYSCONFDIR	"/etc"
#endif

#ifndef TRUE
#	define TRUE	1
#	define FALSE	0
#endif

#ifndef MAXPATHLEN
#	define MAXPATHLEN 512
#endif


#define PREMAKE_VERSION		"0.1"

#define PREMAKE_FILENAME	"pmkfile"
#define PREMAKE_CONFIG		"pmk.conf"
#define PREMAKE_LOG		"pmk.log"

/* prefix character used for comments */
#define CHAR_COMMENT	'#'

/* prefix character used for commands */
#define CHAR_COMMAND	'.'

/* keyword to mark end of command */
#define END_COMMAND	".END"


/* maximal size of an error message */
#define MAX_ERR_MSG_LEN		256

/* maximal size of a command */
#define MAX_CMD_NAME_LEN	64

/* maximal size of a command label */
#define MAX_LABEL_LEN		64

/* maximal size of a command string : <prefix><command>(<label>) */
#define MAX_CMD_LEN		MAX_CMD_NAME_LEN + MAX_LABEL_LEN + 2

/* maximal sizes for command pair of option (name and value) */
#define MAX_OPT_NAME_LEN	64
#define MAX_OPT_VALUE_LEN	MAXPATHLEN /* can contain a path */

/* maximal size of a line */
#define MAX_LINE_LEN		MAXPATHLEN + MAX_OPT_NAME_LEN

/* maximal number of options per command */
#define MAX_CMD_OPT	32


/* command tokens */
#define	TOK_NULL	 0
#define	TOK_DEFINE	 1
#define TOK_CHK_BIN	 2
#define TOK_CHK_INC	 3
#define TOK_CHK_LIB	 4


/* boolean type */
typedef unsigned char bool;
/* command option type */
typedef struct {
	char	name[MAX_OPT_NAME_LEN],
		value[MAX_OPT_VALUE_LEN];
} pmkcmdopt;
/* command type */
typedef struct {
	int		type; /* command token */
	char		name[MAX_CMD_LEN],
			label[MAX_LABEL_LEN];
	pmkcmdopt	opt[MAX_CMD_OPT];
} pmkcmd;

