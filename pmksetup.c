/* $Id$ */

/*
 * Copyright (c) 2003 Damien Couderc
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "pmksetup.h"
#include "premake.h"

/* XXX override PREMAKE_CONFIG_PATH for test purpose */
#ifdef PMKSETUP_DEBUG
	#undef PREMAKE_CONFIG_PATH
	#define PREMAKE_CONFIG_PATH	"samples/pmk.conf.sample"
	#define PREMAKE_CONFIG_TMP	"/tmp/pmk.notrandom"
#else
	#define PREMAKE_CONFIG_TMP	"/tmp/pmk.XXXXXXXX"
#endif

char	sfn[MAXPATHLEN];	/* scratch file name */		
FILE	*sfp;			/* scratch file pointer */


int main(void) {
	FILE		*config;
	int		len,
			linenum = 0,
			error = 0;
	char		*v;	
	char		line[MAX_LINE_LEN],
			filename[MAXPATHLEN];
 	conf_opt	options;	
	htable		*ht;

	
	if (snprintf(filename, sizeof(filename), "%s", PREMAKE_CONFIG_PATH) != -1) { 
		
		if ((config = fopen(filename, "r")) != NULL) {
			
			if ((ht = hash_init(MAX_CONF_OPT)) == NULL) {
				errorf("cannot create hash table");
				exit(1);
			} 
			if ((open_tmp_config() == -1) ||
					(get_env_vars(ht) == -1) ||
					(get_binaries(ht) == -1))
				exit(1);

			while (fgets(line, sizeof(line), config) != NULL) {
				linenum++;
				len = strlen(line);

				/* replace the trailing '\n' by a NULL char */
				line[len -1] = '\0';

				switch (line[0]) {
					case CHAR_COMMENT :
						fprintf(sfp, "%s\n", line);
						break;
					case '\0' :
						fprintf(sfp, "%s\n", line);
						break;
					case '\t' :
						errorf_line(PREMAKE_CONFIG_PATH, linenum, "syntax error"); 
						return(-1);
						break;
					default :
						if (parse_line(line, linenum, &options) == 0) {
							if ((v = hash_get(ht, options.key)) != NULL) {
								switch (options.opchar) {
									case PMKSETUP_ASSIGN_CHAR :
										if (strncmp(v, options.val, MAX_OPT_VALUE_LEN) == 0)
											fprintf(sfp, "%s%c%s\n", options.key,
												PMKSETUP_ASSIGN_CHAR, options.val);
										break;
									case PMKSETUP_STATIC_CHAR :
										fprintf(sfp, "%s%c%s\n", options.key,
											PMKSETUP_STATIC_CHAR, options.val);
										break;
									default :
										fprintf(sfp, "%s=%s\n", options.key, v);
										break;
								}
							}
						} else
							error = 1;
						break;
				}
			}
			fclose(config);
			hash_destroy(ht);
		}
		if (close_tmp_config() < 0)
			exit(1);

		if (error == 1)
			exit(1);
	}
	return(0);
}



/* open temporary configuration file */
int open_tmp_config(void) {
	int	fd = -1;

	/* creating temporary file to build new configuration file */
	snprintf(sfn, sizeof(sfn), PREMAKE_CONFIG_TMP);

	if ((fd = mkstemp(sfn)) == -1) {
		errorf("could not create temporary file: %s", sfn);
		return(-1);
	}
	if ((sfp = fdopen(fd, "w")) == NULL) {
		if (fd != -1) {
			unlink(sfn);
			close(fd);
		}	
		errorf("cannot open temporary file: %s", sfn);
		return(-1);
	}
	return(0);
}


/* close temporary configuration file */
int close_tmp_config(void) {
	if (sfp) {
		if (fclose(sfp) < 0) {
			errorf("cannot close temporary file: %s", sfp);
			return(-1);
		}
		sfp = NULL;
        
#ifdef PMKSETUP_DEBUG
		debugf("%s has not been deleted!", sfn);
#else
		if (unlink(sfn) == -1) {
			errorf("cannot remove temporary file: %s", sfn);
			return(-1);
		}
#endif  /* PMKSETUP_DEBUG */
	}
	return(0);
}



/*
 * Get the environment variables needed for the configuration file
 *
 *	ht: hash table where we have to store the values
 */
int get_env_vars(htable *ht) { 
	struct	utsname	utsname;

	if (uname(&utsname) == -1) {
		errorf("uname");
		return(-1);
	}

	/* XXX should think about a better way to do it though */
	hash_add(ht, PREMAKE_KEY_OSNAME, utsname.sysname);
	hash_add(ht, PREMAKE_KEY_OSVERS, utsname.release);
	hash_add(ht, PREMAKE_KEY_OSARCH, utsname.machine);

	return(0);
}


/* 
 * Get the _must be_ binaries on the system
 *
 *	ht: hash table where we have to store the values
 */ 
int get_binaries(htable *ht) {
	int	i;
	char	*bin_path,
		fbin[MAXPATHLEN];	/* full binary path */
	mpath   stpath;

	if ((bin_path = getenv("PATH")) == NULL) {
		errorf("could not get the PATH environment variable");
		return(-1);
	}

	hash_add(ht, PREMAKE_KEY_BINPATH, bin_path);

        /*
         *splitting the PATH variable and storing in a struct
         * for later use by find_file
         */
	if ((strsplit(bin_path, &stpath, STR_DELIMITER)) == -1) {
		errorf("could not split the PATH environment variable correctly");
		return(-1);
	}

	debugf("Looking for needed binaries...");
	for (i = 0; i < MAXBINS; i++) {
		if (find_file(&stpath, binaries[i][0], fbin, MAXPATHLEN) == 0) {
			fprintf(stderr, "Looking for %s => %s ... found\n", binaries[i][0], fbin);
			hash_add(ht, binaries[i][1], fbin);
		} else {
			errorf("%s not found", binaries[i][0]);
			return(-1);
		}
	}
	return(0);
}

/*
 * Parses a given line.
 * atm, using samples/pmk.conf.sample -> /etc/pmk.conf
 *      
 *	line: line to parse
 *	linenum: line number
 *	opts: struct where where we store the key->value data   
 *
 *	returns 0 on success else 1
 */
int parse_line(char *line, int linenum, conf_opt *opts) {
	char	key[MAX_OPT_NAME_LEN],
		value[MAX_OPT_VALUE_LEN],
		c;
	int	i = 0, j = 0, k = 0,
		found_op = 0;


	while ((c = line[i]) != '\0') {
		if (found_op == 0) {
			if ((c == PMKSETUP_ASSIGN_CHAR) || (c == PMKSETUP_STATIC_CHAR)) {
				/* operator found, terminate key and copy key name in struct */
				found_op = 1;
				key[j] = '\0';
				strncpy(opts->key, key, MAX_OPT_NAME_LEN);

				/* set operator in struct */
				opts->opchar = c;
			} else {
				key[j] = c;
				j++;
				if (j > MAX_OPT_NAME_LEN) {
					/* too small, cannot store key name */
					errorf_line(PREMAKE_CONFIG_PATH, linenum, "Key name too long.");
					return(-1);
				}
			}
		} else {
			value[k] = c;
			k++;
			if (k > MAX_OPT_VALUE_LEN) {
				/* too small, cannot store key value */
				errorf_line(PREMAKE_CONFIG_PATH, linenum, "Key value too long.");
				return(-1);
			}
		}
		i++;
	}
	if (found_op == 1) {
		/* line parsed without any error */
		value[k] = '\0';
		strncpy(opts->val, value, MAX_OPT_VALUE_LEN);
		return(0);			
	} else {
		/* missing operator */
		errorf_line(PREMAKE_CONFIG_PATH, linenum, "Operator not found.");
		return(-1);
	}
}
