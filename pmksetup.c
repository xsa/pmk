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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "pmksetup.h"
#include "premake.h"

#define PREMAKE_CONFIG_TMP	"/tmp/pmk.XXXXXXXX"

/* buffer size in the copy_config() function */
#define	MAX_LINE_BUF		MAX_LINE_LEN


char	sfn[MAXPATHLEN];	/* scratch file name */		
FILE	*sfp;			/* scratch file pointer */


int main(void) {
	FILE		*config;
	int		linenum = 0,
			error = 0;
	size_t		len;	
	uid_t		uid;
	char		*v;	
	char		line[MAX_LINE_LEN],
			filename[MAXPATHLEN];
 	cfg_opt		options;	
	htable		*ht;


	/* pmksetup(8) must be run as root */
	if ((uid = getuid()) != 0) {
		errorf("you must be root.");
		exit(1);
	} 
	
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

			/* parsing the configuration file */
			while (fgets(line, sizeof(line), config) != NULL) {
				linenum++;
				len = strlen(line);

				/* replace the trailing '\n' by a NULL char */
				line[len -1] = '\0';

				/* checking first character of the line */
				switch (line[0]) {
					case CHAR_COMMENT :
						fprintf(sfp, "%s\n", line);
						break;
					case '\0' :	/* empty char */
						fprintf(sfp, "%s\n", line);
						break;
					case '\t' :	/* TAB char */
						errorf_line(PREMAKE_CONFIG_PATH, linenum, "syntax error"); 
						return(-1);
						break;
					default :
						if (parse_conf_line(line, linenum, &options) == 0) {
							if ((v = hash_get(ht, options.key)) != NULL) {
								/* checking the VAR<->VALUE separator */
								switch (options.opchar) {
									case PMKSETUP_ASSIGN_CHAR :
										/* get newer value */
										fprintf(sfp, "%s%c%s\n", options.key, 
												PMKSETUP_ASSIGN_CHAR, v);
										break;
									case PMKSETUP_STATIC_CHAR :
										/* static definition, stay unchanged */ 
										fprintf(sfp, "%s%c%s\n", options.key,
												PMKSETUP_STATIC_CHAR, options.val);
										break;
									default :
										error = 1;
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

		if (error == 0) {
			/* copying the temporary config to the system one */
			if (copy_config(sfn, PREMAKE_CONFIG_PATH) == -1)
				exit(1);
		}

#ifdef PMKSETUP_DEBUG
		debugf("%s has not been deleted!", sfn);
#else
		if (unlink(sfn) == -1) {
			errorf("cannot remove temporary file: %s : %s",
				sfn, strerror(errno));
			exit(1);	
		}
#endif	/* PMKSETUP_DEBUG */

		 if (error == 1)
			exit(1);

	}
	return(0);
}


/*
 * Open temporary configuration file
 *
 *	returns:  0 on success
 *		 -1 on failure
 */
int open_tmp_config(void) {
	int	fd = -1;

	/* creating temporary file to build new configuration file */
	snprintf(sfn, sizeof(sfn), PREMAKE_CONFIG_TMP);

	if ((fd = mkstemp(sfn)) == -1) {
		errorf("could not create temporary file: %s : %s", 
			sfn, strerror(errno));
		return(-1);
	}
	if ((sfp = fdopen(fd, "w")) == NULL) {
		if (fd != -1) {
			unlink(sfn);
			close(fd);
		}	
		errorf("cannot open temporary file: %s : %s", 
			sfn, strerror(errno));
		return(-1);
	}
	return(0);
}


/*
 * Close temporary configuration file
 *
 *	it deletes the temporary file on exit except if the
 *	PMKSETUP_DEBUG mode is on.
 *
 *	returns:  0 on success
 *		 -1 on failure 
 */
int close_tmp_config(void) {
	if (sfp) {
		if (fclose(sfp) < 0) {
			errorf("cannot close temporary file: %s : %s", 
				sfp, strerror(errno));
			return(-1);
		}
		sfp = NULL;
	}        
	return(0);
}



/*
 * Get the environment variables needed for the configuration file
 *
 *	ht: hash table where we have to store the values
 *
 *	returns:  0 on success
 *		 -1 on failure
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
 *
 *	returns:  0 on success
 *		 -1 on failure
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
 * Copy the temporary configuration file to the system one
 *
 *	tmp_config: temporary configuration file name
 *	config: system configuration file name
 *
 *	returns:  0 on success
 *		 -1 on failure  	
 */
int copy_config(const char *tmp_config, const char *config) {
	FILE	*fp_t, *fp_c;
	int	rval;
	char	buf[MAX_LINE_BUF];


	if ((fp_t = fopen(tmp_config, "r")) == NULL) {
		errorf("cannot open temporary configuration "
			"file for reading: %s : %s", tmp_config, 
				strerror(errno));
		return(-1);	
	}
	if ((fp_c = fopen(config, "w")) == NULL) {
		errorf("cannot open %s for writing: %s", 
			config, strerror(errno));

		fclose(fp_t);	
		return(-1);
	}

	while(get_line(fp_t, buf, MAX_LINE_BUF) == 1) {
		fprintf(fp_c, "%s\n", buf);
	}

	if (feof(fp_t) == 0) {
		errorf("read failure, cannot copy "
			"%s to %s", tmp_config, config);
		rval = -1;	
	} else
		rval = 0;

	fclose(fp_t);
	fclose(fp_c);

	return(rval);
}
