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

#include <stdio.h>
#include <string.h> 

#include "common.h"
#include "premake.h"
#include "readconf.h"


/* 
 * Reads the configuration file 
 *
 *	f: configuration file
 *	ht: hash table initialized to store the configuration variables 
 */

int read_config_file(FILE *f, htable *ht) {
	int		len, linenum = 0;
	char		line[MAX_LINE_LEN];
	conf_opt	options; 

	while (!feof(f) && !ferror(f)) {
		if (fgets(line, sizeof(line), f) != NULL) {
			linenum++;
			len = strlen(line);

	 		if (line[len - 1] != '\n' && !feof(f)) {
				error_line(PREMAKE_CONFIG_PATH, 
						linenum, "line too long");
				exit(1);
			}

			/* replace the trailing '\n' by a NULL char */
			line[len -1] = '\0';

			/* parse the line */
			parse_line(line, linenum, &options);

			/* 
			 * updating the hash table with the
			 * values returned by parse_line()
			 */
			hash_add(ht, options.key, options.val);
		}
	}
	fclose(f);
	return(0);
}

/* 
 * Parses a given line. 
 * atm, using samples/pmk.conf.sample -> /etc/pmk.conf
 *
 * XXX improve parser for specific errors and distinguish options
 *	using the ':' or '=' delimiter.
 *
 *	line: line to parse
 *	linenum: line number
 *	opts: struct where where we store the key->value data 
 */

int parse_line(char *line, int linenum, conf_opt *opts) {
	char	*p, *last,
		*delimiters = ":=";

	switch (line[0]) {
		case CHAR_COMMENT :
			/* ignore comments */
			break;
		case '\0' :
			/* ignore empty lines */
			break;
		case '\t' :
			/* return a syntax error if line starts with a TAB */ 
			error_line(PREMAKE_CONFIG_PATH, linenum,
				"syntax error");
			exit(1);
			break;
		default :
			/* XXX add errors checks */
			if ((p = strtok_r(line, delimiters, &last)) != NULL)
				strncpy(opts->key, p, MAX_OPT_NAME_LEN);
			if ((p = strtok_r(NULL, delimiters, &last)) != NULL)
				strncpy(opts->val, p, MAX_OPT_NAME_LEN);
			break;
	}
	return(0);
}
