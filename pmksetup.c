/* $Id$ */

/*
 * Copyright (c) 2003 Damien Couderc
 * Copyright (c) 2003-2004 Xavier Santolaria
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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "dynarray.h"
#include "parse.h"
#include "pmksetup.h"


char	*__progname;		/* program name from argv[0] */

int	 verbose_flag = 0;	/* -V option at the cmd-line */ 
char	 sfn[MAXPATHLEN];	/* scratch file name */		
FILE	*sfp;			/* scratch file pointer */

hpair	predef[] = {
		{PMKCONF_MISC_SYSCONFDIR,	SYSCONFDIR},
		{PMKCONF_MISC_PREFIX,		"/usr/local"},
		{PMKCONF_PATH_INC,		"/usr/include"},
		{PMKCONF_PATH_LIB,		"/usr/lib"}
};

int	nbpredef = sizeof(predef) / sizeof(hpair);


/*
 * Main program for pmksetup(8)
 */
int main(int argc, char *argv[]) {
	FILE		*config;
	int		 ch,
			 error = 0;
	htable		*ht;
	
	extern int	 optind,
			 errno;


#ifndef PMK_USERMODE
	uid_t		 uid;

	/* pmksetup(8) must be run as root */
	if ((uid = getuid()) != 0) {
		errorf("you must be root.");
		exit(EXIT_FAILURE);
	}
#endif

	__progname = argv[0];
	
	while ((ch = getopt(argc, argv, "hvV")) != -1)
		switch(ch) {
			case 'v' :
				fprintf(stderr, "%s\n", PREMAKE_VERSION);
				exit(EXIT_SUCCESS);
				break;
			case 'V' :
				if (0 == verbose_flag)
					verbose_flag = 1;
				break;
			case '?' :
			default :
				usage();
				/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;


	if ((ht = hash_init(MAX_CONF_OPT)) == NULL) {
		errorf("cannot create hash table.");
		exit(EXIT_FAILURE);
	}

	printf("PMKSETUP version %s", PREMAKE_VERSION);
#ifdef DEBUG
	printf(" [SUB #%s] [SNAP #%s]", PREMAKE_SUBVER_PMKSETUP, PREMAKE_SNAP);
#endif
	printf("\n\n");

	/* check if syconfdir exists */
	if (dir_exists(CONFDIR) == -1) {
		verbosef("==> Creating '%s' directory.", CONFDIR);
		if (mkdir(CONFDIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
			errorf("cannot create '%s' directory : %s.", 
				CONFDIR, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	printf("==> Looking for default parameters...\n");
	if ((get_env_vars(ht) == -1) || (get_binaries(ht) == -1))
		exit(EXIT_FAILURE);

	if (predef_vars(ht) == -1) {
		errorf("predefined variables."); 
		exit(EXIT_FAILURE);
	}

	check_libpath(ht);

	if (byte_order_check(ht) == false) {
		errorf("failure in byte order check.");
		exit(EXIT_FAILURE);
	}

	if (check_echo(ht) == -1) {
		errorf("failure in echo check.");
		exit(EXIT_FAILURE);
	}

	if ((open_tmp_config() == -1))
		exit(EXIT_FAILURE);

	/* parse configuration file */
	if ((config = fopen(PREMAKE_CONFIG_PATH, "r")) != NULL) {
		printf("==> Configuration file found: %s\n", PREMAKE_CONFIG_PATH);
		/* XXX FIXME TODO check ? */
		parse_pmkconf(config, ht, PRS_PMKCONF_SEP, check_opt);
		fclose(config);
	} else {
		printf("==> Configuration file not found, generating one...\n");
	}

	printf("==> Merging remaining data...\n");	
	/* writing the remaining data stored in the hash */
	write_new_data(ht);
	/* destroying the hash once we'r done with it */	
	hash_destroy(ht);

	if (close_tmp_config() < 0)
		exit(EXIT_FAILURE);

	if (error == 0)
		/* copying the temporary config to the system one */
		if (copy_config(sfn, PREMAKE_CONFIG_PATH) == -1)
			exit(EXIT_FAILURE);

#ifdef PMKSETUP_DEBUG
	debugf("%s has not been deleted!", sfn);
#else
	if (unlink(sfn) == -1) {
		errorf("cannot remove temporary file: '%s' : %s.",
			sfn, strerror(errno));
		exit(EXIT_FAILURE);	
	}
#endif	/* PMKSETUP_DEBUG */

	if (error != 0) {
		errorf("returned code '%d' during parsing.", error);
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
	if (phk != NULL) {
		/* sort hash table */
		qsort(phk->keys, phk->nkey, sizeof(char *), keycomp);

		/* processing remaining keys */
		for(i = 0 ; i < phk->nkey ; i++) {
			val = (char *) hash_get(ht, phk->keys[i]);
			fprintf(sfp, PMKSTP_WRITE_FORMAT, phk->keys[i], CHAR_ASSIGN_UPDATE, val);
		}

		hash_free_hkeys(phk);
	} else {
		verbosef("Nothing to merge.");
	}
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
 *	check and process option
 *
 *	pht : htable for comparison
 *	popt : option to process
 *
 *	return : boolean
 */

bool check_opt(htable *pht, prsopt *popt) {
	char	*recval,
		*optval;

	optval = po_get_str(popt->value);

	if ((popt->opchar == CHAR_COMMENT) || (popt->opchar == CHAR_EOS)) {
		/* found comment or empty line */
		fprintf(sfp, "%s\n", optval);
		return(true);
	}

	if ((recval = (char *) hash_get(pht, popt->key)) != NULL) {
		/* checking the VAR<->VALUE separator */
		switch (popt->opchar) {
			case CHAR_ASSIGN_UPDATE :
				/* get newer value */
				fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, CHAR_ASSIGN_UPDATE, recval);
				hash_delete(pht, popt->key);
				break;

			case CHAR_ASSIGN_STATIC :
				/* static definition, stay unchanged */ 
				fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, CHAR_ASSIGN_STATIC, optval);
				hash_delete(pht, popt->key);
				break;

				default :
					/* should not happen now */
					errorf("unknown operator '%c'.", popt->opchar);
					return(false);
					break;
		}
	} else {
		fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, popt->opchar, optval);
	}

	return(true);
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
		errorf("could not create temporary file '%s' : %s.", 
			sfn, strerror(errno));
		return(-1);
	}
	if ((sfp = fdopen(fd, "w")) == NULL) {
		if (fd != -1) {
			unlink(sfn);
			close(fd);
		}	
		errorf("cannot open temporary file '%s' : %s.", 
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
			errorf("cannot close temporary file '%s' : %s.", 
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
		errorf("uname : %s.", strerror(errno));
		return(-1);
	}

	if (hash_update_dup(ht, PMKCONF_OS_NAME, utsname.sysname) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_OS_NAME, utsname.sysname);

	if (hash_update_dup(ht, PMKCONF_OS_VERS, utsname.release) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_OS_VERS, utsname.release);	
	
	if (hash_update_dup(ht, PMKCONF_OS_ARCH, utsname.machine) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_OS_ARCH, utsname.machine);


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

	if (hash_update_dup(ht, PMKCONF_PATH_BIN, bin_path) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_PATH_BIN, bin_path);

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
	stpath = str_to_dynary((char *) hash_get(ht, PMKCONF_PATH_BIN), CHAR_LIST_SEPARATOR);
	if (stpath == NULL) {
		errorf("could not split the PATH environment variable correctly.");
		return(-1);	
	}

	if (find_file(stpath, "cc", fbin, sizeof(fbin)) == true) {
		if (hash_update_dup(ht, "BIN_CC", fbin) == HASH_ADD_FAIL) {
			da_destroy(stpath);
			return(-1);
		}
		verbosef("Setting '%s' => '%s'", "BIN_CC", fbin);
	} else {
		if (find_file(stpath, "gcc", fbin, sizeof(fbin)) == true) {
			if (hash_update_dup(ht, "BIN_CC", fbin) == HASH_ADD_FAIL) {
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
		if (find_file(stpath, binaries[i][0], fbin, sizeof(fbin)) == true) {
			if (hash_update_dup(ht, binaries[i][1], fbin) == HASH_ADD_FAIL) {
				da_destroy(stpath);
				return(-1);
			}
			verbosef("Setting '%s' => '%s'", binaries[i][1], fbin);
		} else {
			verbosef("**warning: '%s' Not Found", binaries[i][0]);
		}
	}
	da_destroy(stpath);
	return(0);
}


/*
 * Add to the hash table the predefined variables we
 * cannot get automagically
 *
 *	pht: hash table where we have to store the values
 *
 *	returns:  0 on success
 *		 -1 on failure
 */ 
int predef_vars(htable *pht) {
	int	i;

	for (i = 0 ; i < nbpredef ; i++) {
		verbosef("Setting '%s' => '%s'", predef[i].key, predef[i].value);
		if (hash_update_dup(pht, predef[i].key, predef[i].value) == HASH_ADD_FAIL) {
			errorf("failed to add '%s'.", predef[i].key);
			return(-1);
		}
	}

	return(0);
}

/*
 *	pht: hash table where we have to store the values
 *
 *	returns:  0 on success
 *		 -1 on failure
 */
int check_echo(htable *pht) {
	FILE	*echo_pipe = NULL;
	char	buf[TMP_BUF_LEN],
		echocmd[MAXPATHLEN];
	char	*echo_n, *echo_c, *echo_t;
	size_t	s;

	snprintf(echocmd, sizeof(echocmd), ECHO_CMD);

	if ((echo_pipe = popen(echocmd, "r")) == NULL) {
		errorf("unable to execute '%s'.", echocmd);
		return(-1);
	}

	s = fread(buf, sizeof(char), sizeof(buf), echo_pipe);
	buf[s] = CHAR_EOS;

	if (feof(echo_pipe) == 0) {
		errorf("pipe not empty.");
		pclose(echo_pipe);
		return(-1);
	}

	pclose(echo_pipe);

	if (strncmp(buf, "one\\c\n-n two\nthree\n", sizeof(buf)) == 0) {
		/* ECHO_N= ECHO_C='\n' ECHO_T='\t' */
		echo_n = ECHO_EMPTY;
		echo_c = ECHO_NL;
		echo_t = ECHO_HT;
	} else {
		if (strncmp(buf, "one\\c\ntwothree\n", sizeof(buf)) == 0) {
			/* ECHO_N='-n' ECHO_C= ECHO_T= */
			echo_n = ECHO_N;
			echo_c = ECHO_EMPTY;
			echo_t = ECHO_EMPTY;
		} else {
			if (strncmp(buf, "one-n two\nthree\n", sizeof(buf)) == 0) {
				/* ECHO_N= ECHO_C='\c' ECHO_T=  */
				echo_n = ECHO_EMPTY;
				echo_c = ECHO_C;
				echo_t = ECHO_EMPTY;
			} else {
				if (strncmp(buf, "onetwothree\n", sizeof(buf)) == 0) {
					/* ECHO_N= ECHO_C='\\c' ECHO_T= */
					echo_n = ECHO_EMPTY;
					echo_c = ECHO_C;
					echo_t = ECHO_EMPTY;
				} else {
					errorf("unable to set ECHO_* variables.");
					return(-1);
				}
			}
		}
	}

	if (hash_update_dup(pht, PMKCONF_AC_ECHO_N, echo_n) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_AC_ECHO_N, echo_n);
	
	if (hash_update_dup(pht, PMKCONF_AC_ECHO_C, echo_c) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_AC_ECHO_C, echo_c);

	if (hash_update_dup(pht, PMKCONF_AC_ECHO_T, echo_t) == HASH_ADD_FAIL)
		return(-1);
	verbosef("Setting '%s' => '%s'", PMKCONF_AC_ECHO_T, echo_t);

	return(0);
}


/*
 *	pht: hash table where we have to store the values
 *
 *	returns:  0 on success
 *		 -1 on failure
 */

int check_libpath(htable *pht) {
	char	libpath[MAXPATHLEN];

	strlcpy(libpath, hash_get(pht, PMKCONF_MISC_PREFIX), sizeof(libpath));
	strlcat(libpath, PMKVAL_LIB_PKGCONFIG, sizeof(libpath));

	if (hash_get(pht, PMKCONF_BIN_PKGCONFIG) != NULL) {
		if (dir_exists(libpath) == 0) {
			if (hash_update_dup(pht, PMKCONF_PC_PATH_LIB, libpath) == HASH_ADD_FAIL)
				return(-1);
			verbosef("Setting '%s' => '%s'", PMKCONF_PC_PATH_LIB, libpath);
		} else {
			verbosef("**warning: %s does not exist.", libpath);
		}
	}
	return(0);
}

/*
 *	check if a directory does exist 
 *
 *	fdir : directory to search
 *
 *	returns:  0 on success
 *		 -1 on failure 
 */

int dir_exists(const char *fdir) {
        DIR     *dirp;
        size_t  len;
        
	len = strlen(fdir);

	if (len < MAXPATHLEN) {
		dirp = opendir(fdir);
		if (dirp != NULL) {
			closedir(dirp);
			return(0);
		}
	}
	return(-1);
}

/*
	byte order check

	pht : hash table to store value

	return : boolean value
*/

bool byte_order_check(htable *pht) {
	char	bo_type[16];
	int	num = 0x41424344;

	if (((((char *)&num)[0]) == 0x41) && ((((char *)&num)[1]) == 0x42) &&
	    ((((char *)&num)[2]) == 0x43) && ((((char *)&num)[3]) == 0x44)) {
		strlcpy(bo_type, HW_ENDIAN_BIG, sizeof(bo_type));
	} else {
		if ( ((((char *)&num)[3]) == 0x41) && ((((char *)&num)[2]) == 0x42) && 
		    ((((char *)&num)[1]) == 0x43) && ((((char *)&num)[0]) == 0x44) ) {
			strlcpy(bo_type, HW_ENDIAN_LITTLE, sizeof(bo_type));
		} else {
			strlcpy(bo_type, HW_ENDIAN_UNKNOWN, sizeof(bo_type));
		}
	}

	if (hash_update_dup(pht, PMKCONF_HW_BYTEORDER, bo_type) == HASH_ADD_FAIL)
		return(false);

	verbosef("Setting '%s' => '%s'", PMKCONF_HW_BYTEORDER, bo_type);

	return(true);
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
			"file for reading '%s' : %s.", tmp_config, 
				strerror(errno));
		return(-1);	
	}
	if ((fp_c = fopen(config, "w")) == NULL) {
		errorf("cannot open '%s' for writing : %s.", 
			config, strerror(errno));

		fclose(fp_t);	
		return(-1);
	}

	while(get_line(fp_t, buf, MAX_LINE_BUF) == true) {
		fprintf(fp_c, "%s\n", buf);
	}

	if (feof(fp_t) == 0) {
		errorf("read failure, cannot copy "
			"'%s' to '%s'.", tmp_config, config);
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
	char	buf[MAX_ERR_MSG_LEN];
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
	exit(EXIT_FAILURE);
}

