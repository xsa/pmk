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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compat/pmk_string.h"
#include "common.h"
#include "dynarray.h"
#include "pmksetup.h"


char	*__progname;		/* program name from argv[0] */

int	 verbose_flag = 0;	/* -V option at the cmd-line */ 
char	 sfn[MAXPATHLEN];	/* scratch file name */		
FILE	*sfp;			/* scratch file pointer */


/*
 * Main program for pmksetup(8)
 */
int main(int argc, char *argv[]) {
	FILE		*config;
	int		 ch,
			 error = 0;
	htable		*ht;
	
	extern int	 optind;


#ifndef USER_TEST
	uid_t		 uid;

	/* pmksetup(8) must be run as root */
	if ((uid = getuid()) != 0) {
		errorf("you must be root.");
		exit(1);
	} 
#endif

	__progname = argv[0];
	
	while ((ch = getopt(argc, argv, "hvV")) != -1)
		switch(ch) {
			case 'v' :
				fprintf(stderr, "%s\n", PREMAKE_VERSION);
				exit(0);
				break;
			case 'V' :
				if (0 == verbose_flag)
					verbose_flag = 1;
				break;
			case '?' :
			default :
				usage();
		}
	argc -= optind;
	argv += optind;


	if ((ht = hash_init(MAX_CONF_OPT)) == NULL) {
		errorf("cannot create hash table.");
		exit(1);
	}

	printf("PMKSETUP version %s", PREMAKE_VERSION);
#ifdef DEBUG
	printf(" [SUB #%s] [SNAP #%s]", PREMAKE_SUBVER_PMKSETUP, PREMAKE_SNAP);
#endif
	printf("\n\n");

	printf("==> Looking for default parameters...\n");
	if ((get_env_vars(ht) == -1) || (get_binaries(ht) == -1))
		exit(1);

	if (predef_vars(ht) == -1) {
		errorf("predefined variables."); 
		exit(1);
	}

	if ((open_tmp_config() == -1))
		exit(1);

	if ((config = fopen(PREMAKE_CONFIG_PATH, "r")) != NULL) {
		printf("==> Configuration file found: %s\n", PREMAKE_CONFIG_PATH);

		/* parse configuration file */
		error = parse_conf(config, ht);

		fclose(config);
	} else {
		printf("==> Configuration file not found, generating one...\n");
		config = NULL;
	}

	printf("==> Merging remaining data...\n");	
	/* writing the remaining data stored in the hash */
	write_new_data(ht);
	/* destroying the hash once we'r done with it */	
	hash_destroy(ht);

	if (close_tmp_config() < 0)
		exit(1);

	if (error == 0)
		/* copying the temporary config to the system one */
		if (copy_config(sfn, PREMAKE_CONFIG_PATH) == -1)
			exit(1);

#ifdef PMKSETUP_DEBUG
	debugf("%s has not been deleted!", sfn);
#else
	if (unlink(sfn) == -1) {
		errorf("cannot remove temporary file: %s : %s.",
			sfn, strerror(errno));
		exit(1);	
	}
#endif	/* PMKSETUP_DEBUG */

	if (error != 0) {
		errorf("returned code '%d' during parsing", error);
		exit(error);
	}
	return(0);
}


/*
 * write remaining data
 *
 * 	ht: hash table
 */
void write_new_data(htable *ht) {
	char	*val;
	int	 i;
	hkeys	*phk;

	phk = hash_keys(ht);

	/* sort hash table */
	qsort(phk->keys, phk->nkey, sizeof(char *), keycomp);

	/* processing remaining keys */
	for(i = 0 ; i < phk->nkey ; i++) {
		val = (char *)get_obj_data(hash_get(ht, phk->keys[i]));
		fprintf(sfp, "%s%c%s\n", phk->keys[i], CHAR_ASSIGN_UPDATE, val);
	}

	hash_free_hkeys(phk);
}


/*
 * Compare the keys in the hash to sort them:
 *
 * 	here sorting the keys alphabetically before
 *	writing the new configuration file with write_new_data()
 *
 *	a: first element to compare
 *	b: second element to compare
 *
 *	returns an integer greater than, equal to, or
 *		less than 0 according as the character a is
 *		greather, equal to, or less than the character b. 
 */
int keycomp(const void *a, const void *b) {
	return(strcmp(*(const char **)a, *(const char **)b));
}


/*
 * Parse configuration
 *
 *	config: old configuration file
 *	ht: hash table
 *
 *	returns error code
 */
int parse_conf(FILE *config, htable *ht) {
 	cfg_opt	 options;
	char	 line[MAX_LINE_LEN],
		*v;	
	int	 linenum = 0,
		 error = 0;

	/* parsing the configuration file */
	while (get_line(config, line, sizeof(line)) == true) {
		linenum++;

		/* checking first character of the line */
		switch (line[0]) {
			case CHAR_COMMENT :
				fprintf(sfp, "%s\n", line);
				break;
			case '\0' :	/* empty char */
				fprintf(sfp, "%s\n", line);
				break;
			case '\t' :	/* TAB char */
				errorf_line(PREMAKE_CONFIG_PATH, linenum, "syntax error."); 
				return(-1);
				break;
			default :
				if (parse_conf_line(line, linenum, &options) == 0) {
					if ((v = (char *)get_obj_data(hash_get(ht, options.key))) != NULL)
						/* checking the VAR<->VALUE separator */
						switch (options.opchar) {
							case CHAR_ASSIGN_UPDATE :
								/* get newer value */
								fprintf(sfp, "%s%c%s\n", options.key, CHAR_ASSIGN_UPDATE, v);
								hash_delete(ht, options.key);
								break;
							case CHAR_ASSIGN_STATIC :
								/* static definition, stay unchanged */ 
								fprintf(sfp, "%s%c%s\n", options.key, CHAR_ASSIGN_STATIC, options.val);
								hash_delete(ht, options.key);
								break;
							default :
								error = 1;
								break;
						}
					else
						fprintf(sfp, "%s%c%s\n", options.key, options.opchar, options.val);
				} else
					error = 1;
				break;
		}
	}
	return(error);
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
		errorf("could not create temporary file: %s : %s.", 
			sfn, strerror(errno));
		return(-1);
	}
	if ((sfp = fdopen(fd, "w")) == NULL) {
		if (fd != -1) {
			unlink(sfn);
			close(fd);
		}	
		errorf("cannot open temporary file: %s : %s.", 
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
			errorf("cannot close temporary file: %s : %s.", 
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
	struct utsname	 utsname;
	char		*bin_path;

	if (uname(&utsname) == -1) {
		errorf("uname.");
		return(-1);
	}

	if (hash_add(ht, PREMAKE_KEY_OSNAME, mk_obj_str(utsname.sysname)) == HASH_ADD_FAIL)
		return(-1);
		verbosef("Setting '%s' => '%s'", PREMAKE_KEY_OSNAME, utsname.sysname);

	if (hash_add(ht, PREMAKE_KEY_OSVERS, mk_obj_str(utsname.release)) == HASH_ADD_FAIL)
		return(-1);
		verbosef("Setting '%s' => '%s'", PREMAKE_KEY_OSVERS, utsname.release);	
	
	if (hash_add(ht, PREMAKE_KEY_OSARCH, mk_obj_str(utsname.machine)) == HASH_ADD_FAIL)
		return(-1);
		verbosef("Setting '%s' => '%s'", PREMAKE_KEY_OSARCH, utsname.machine);


	/* getting the environment variable PATH */
	if ((bin_path = getenv("PATH")) == NULL) {
		errorf("could not get the PATH environment variable.");
		return(-1);
	}

	/* 
	 * replace the string separator in the PATH 
	 * to fit to our standards
	 */
	char_replace(bin_path, PATH_STR_DELIMITER, CHAR_LIST_SEPARATOR);

	if (hash_add(ht, PREMAKE_KEY_BINPATH, mk_obj_str(bin_path)) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PREMAKE_KEY_BINPATH, bin_path);

	return(0);
}


/* 
 * Look for location of some predefined binaries
 *
 *	ht: hash table where we have to store the values
 *
 *	returns:  0 on success
 *		 -1 on failure
 */ 
int get_binaries(htable *ht) {
	char	 fbin[MAXPATHLEN];	/* full binary path */
	dynary	*stpath;
	int	 i;

        /*
         * splitting the PATH variable and storing in a 
         * dynamic array for later use by find_file
         */
	if ((stpath = da_init()) == NULL) {
		errorf("cannot initalize the dynamic array."); 
		return(-1);	
	}

	if (str_to_dynary((char *)get_obj_data(hash_get(ht, PREMAKE_KEY_BINPATH)), 
			CHAR_LIST_SEPARATOR, stpath) == 0) {
		errorf("could not split the PATH environment variable correctly.");
		da_destroy(stpath);
		return(-1);	
	}

	if (find_file(stpath, "cc", fbin, sizeof(fbin)) == 1) {
		if (hash_add(ht, "BIN_CC", mk_obj_str(fbin)) == HASH_ADD_FAIL) {
			da_destroy(stpath);
			return(-1);
		}
		verbosef("Setting '%s' => '%s'", "BIN_CC", fbin);
	} else {
		if (find_file(stpath, "gcc", fbin, sizeof(fbin)) == 1) {
			if (hash_add(ht, "BIN_CC", mk_obj_str(fbin)) == HASH_ADD_FAIL) {
				da_destroy(stpath);
				return(-1);
			}
			verbosef("Setting '%s' => '%s'", "BIN_CC", fbin);
		} else {
			errorf("cannot find a C compiler.");
			da_destroy(stpath);
			return(-1);
		}
	}

	for (i = 0; i < MAXBINS; i++) {
		if (find_file(stpath, binaries[i][0], fbin, sizeof(fbin)) == 1) {
			if (hash_add(ht, binaries[i][1], mk_obj_str(fbin)) == HASH_ADD_FAIL) {
				da_destroy(stpath);
				return(-1);
			}
			verbosef("Setting '%s' => '%s'", binaries[i][1], fbin);
		} else
			verbosef("**warning: '%s' Not Found", binaries[i][0]);
	}
	da_destroy(stpath);
	return(0);
}


/*
 * Add to the hash table the predefined variables we
 * cannot get automagically
 *
 *	ht: hash table where we have to store the values
 *
 *	returns:  0 on success
 *		 -1 on failure
 */ 
int predef_vars(htable *ht) {
	hpair	predef[] = {
			{PREMAKE_KEY_SYSCONFDIR,	mk_obj_str(SYSCONFDIR)},
			{PREMAKE_KEY_PREFIX,		mk_obj_str("/usr/local")},
			{PREMAKE_KEY_INCPATH,		mk_obj_str("/usr/include")},
			{PREMAKE_KEY_LIBPATH,		mk_obj_str("/usr/lib")}
		};

	if (hash_add_array(ht, predef, sizeof(predef)/sizeof(hpair)) == 0) 
		return(-1);

	verbosef("Setting '%s' => '%s'", PREMAKE_KEY_SYSCONFDIR, SYSCONFDIR);
	verbosef("Setting '%s' => '%s'", PREMAKE_KEY_PREFIX, "/usr/local");
	verbosef("Setting '%s' => '%s'", PREMAKE_KEY_INCPATH, "/usr/include");
	verbosef("Setting '%s' => '%s'", PREMAKE_KEY_LIBPATH, "/usr/lib");

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
	FILE	*fp_t,
		*fp_c;
	int	 rval;
	char	 buf[MAX_LINE_BUF];


	if ((fp_t = fopen(tmp_config, "r")) == NULL) {
		errorf("cannot open temporary configuration "
			"file for reading: %s : %s.", tmp_config, 
				strerror(errno));
		return(-1);	
	}
	if ((fp_c = fopen(config, "w")) == NULL) {
		errorf("cannot open %s for writing: %s.", 
			config, strerror(errno));

		fclose(fp_t);	
		return(-1);
	}

	while(get_line(fp_t, buf, MAX_LINE_BUF) == 1) {
		fprintf(fp_c, "%s\n", buf);
	}

	if (feof(fp_t) == 0) {
		errorf("read failure, cannot copy "
			"%s to %s.", tmp_config, config);
		rval = -1;	
	} else
		rval = 0;

	fclose(fp_t);
	fclose(fp_c);

	return(rval);
}


/*
 * Replace a character in a string
 *
 *	buf: string we want to modify
 *	search: the char with want to replace
 *	replace: the new char which will replace 'search'
 */ 
void char_replace(char *buf, const char search, const char replace) {
	char	c;
	int	i = 0;

	while ((c = buf[i]) != CHAR_EOS) {
		if (c == search)
			c = replace;
		buf[i] = c;
		i++;
	}
}


/*
 * Simple formated verbose function
 */
void verbosef(const char *fmt, ...) {
	char	buf[256];
	va_list	plst;

	if (verbose_flag == 1) {
		va_start(plst, fmt);
		vsnprintf(buf, sizeof(buf), fmt, plst);
		va_end(plst);

		printf("%s\n", buf);
	}
}


/*
 * pmksetup(8) usage
 */
void usage(void) {
	fprintf(stderr, "Usage: %s [options]\n", __progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h	Display this help menu\n"); 
	fprintf(stderr, "  -v	Display version number\n");
	fprintf(stderr, "  -V	Verbose, display debugging messages\n");
	exit(1);
}
