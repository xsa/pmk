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


#ifndef _PMK_H_
#define _PMK_H_

#include "premake.h"
#include "hash.h"
#include "dynarray.h"


/* prefix character used for commands */
#define PMK_CHAR_COMMAND	'.'

/* keyword to mark end of command */
#define PMK_END_COMMAND		".END"

/* character to assign a value to a key */
#define PMK_KEY_CHAR		'='

/* character used as delimiter for template tags */
#define PMK_TAG_CHAR		'@'

/* maximal size of a command */
#define MAX_CMD_NAME_LEN	64

/* maximal size of a command label */
#define MAX_LABEL_NAME_LEN	64

/* maximal size of a command string : <prefix><command>(<label>) */
#define MAX_CMD_LEN		MAX_CMD_NAME_LEN + MAX_LABEL_NAME_LEN + 2

/* maximal number of options per command */
#define MAX_CMD_OPT		32

/* maximal number of templates */
#define MAX_TEMPLATES		32

/* maximal number of key in datahash */
#define MAX_DATA_KEY		1024

/* command option type */
typedef struct {
	char	name[MAX_OPT_NAME_LEN],
		value[MAX_OPT_VALUE_LEN];
} pmkcmdopt;

/* command type */
typedef struct {
	char		name[MAX_CMD_NAME_LEN],
			label[MAX_LABEL_NAME_LEN];
} pmkcmd;

/* pmk data */
typedef struct {
	htable	*htab;
	dynary	*tlist;
} pmkdata;


#endif /* _PMK_H_ */
