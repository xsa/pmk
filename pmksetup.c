/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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


/*#define PMKSETUP_DEBUG 1*/

#include <sys/stat.h>
/* include it first as if it was <sys/types.h> - this will avoid errors */ 
#include "compat/pmk_sys_types.h"
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


FILE	*sfp;			/* scratch file pointer */
char	*__progname,		/* program name from argv[0] */
	 sfn[MAXPATHLEN];	/* scratch file name */		

htable	*ht;
int	 verbose_flag = 0;	/* -V option at the cmd-line */ 

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
	bool		 process_clopts = false;
	char		*pstr;
	int		 ch,
			 error = 0;
	prsopt		 *ppo;
	
	extern int	 optind,
			 errno;


#ifndef PMK_USERMODE
	/* pmksetup(8) must be run as root */
	if (getuid() != 0) {
		errorf("you must be root.");
		exit(EXIT_FAILURE);
	}
#endif

	__progname = argv[0];

	ht = hash_init_adv(MAX_CONF_OPT, NULL, (void (*)(void *)) prsopt_destroy, NULL);
	if (ht == NULL) {
		errorf("cannot create hash table.");
		exit(EXIT_FAILURE);
	}

	/*while ((ch = getopt(argc, argv, "a:hr:u:vV")) != -1)*/
	while ((ch = getopt(argc, argv, "hr:u:vV")) != -1)
		switch(ch) {
			case 'r' :
				/* mark to be deleted in hash */
				if (record_data(ht, optarg, PMKSTP_REC_REMV, NULL) == false) {
					errorf("failed to record '%s'.", optarg);
					exit(EXIT_FAILURE);
				}

				process_clopts = true;
				break;

			case 'u' :
				/* add or update value */
				ppo = prsopt_init();
				if (ppo == NULL) {
					errorf("prsopt init failed.");
					exit(EXIT_FAILURE);
				}

				if (parse_clopt(optarg, ppo, PRS_PMKCONF_SEP) == false) {
					errorf("bad argument for -a option: %s.", optarg);
					exit(EXIT_FAILURE);
				}

				/* add in hash */
				pstr = po_get_str(ppo->value);
				if (pstr == NULL) {
					errorf("failed to get argument for -a option");
					exit(EXIT_FAILURE);
				}

				/* record prsopt structure directly, fast and simple */
				ppo->opchar = PMKSTP_REC_UPDT;
				if (hash_update(ht, ppo->key, ppo) == HASH_ADD_FAIL) {
					prsopt_destroy(ppo);	
					errorf("hash update failed.");	
					exit(EXIT_FAILURE);		
				}

				/* XXX use record_data => cost much memory and cpu XXX            */
				/*if (record_data(ht, ppo->key, PMKSTP_REC_UPDT, pstr) == false) {*/
				/*        errorf("failed to record '%s'.", ppo->key);             */
				/*        prsopt_destroy(ppo);                                    */
				/*        exit(EXIT_FAILURE);                                     */
				/*}                                                               */
				/*prsopt_destroy(ppo);						  */

				process_clopts = true;
				break;

			case 'v' :
				fprintf(stderr, "%s\n", PREMAKE_VERSION);
				exit(EXIT_SUCCESS);
				break;

			case 'V' :
				if (verbose_flag == 0)
					verbose_flag = 1;
				break;

			case '?' :
			default :
				usage();
				/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;


	printf("PMKSETUP version %s", PREMAKE_VERSION);
#ifdef DEBUG
	printf(" [SUB #%s] [SNAP #%s]", PREMAKE_SUBVER_PMKSETUP, PREMAKE_SNAP);
#endif
	printf("\n\n");

	/* check if syconfdir exists */
	if (dir_exists(CONFDIR) == false) {
		verbosef("==> Creating '%s' directory.", CONFDIR);
		if (mkdir(CONFDIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
			errorf("cannot create '%s' directory : %s.", 
				CONFDIR, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (process_clopts == false) {
		/* standard behavior, gathering data */
		if (gather_data(ht) == false)
			exit(EXIT_FAILURE);
	}

	if ((open_tmp_config() == false))
		exit(EXIT_FAILURE);

	/* parse configuration file */
	config = fopen(PREMAKE_CONFIG_PATH, "r");
	if (config != NULL) {
		printf("==> Configuration file found: %s\n", PREMAKE_CONFIG_PATH);
		printf("==> Backing up configuration file: %s\n", PREMAKE_CONFIG_PATH_BAK);
		if (rename(PREMAKE_CONFIG_PATH,PREMAKE_CONFIG_PATH_BAK) != 0) {
			fclose(config);
			errorf("configuration file backup failed: %s.", strerror(errno));
			exit(EXIT_FAILURE);
		}
				
		if (parse_pmkconf(config, ht, PRS_PMKCONF_SEP, check_opt) == false) {
			fclose(config);
			errorf("parsing failed.");
			exit(EXIT_FAILURE);
		} else {
			fclose(config);
		}
	} else {
		printf("==> Configuration file not found, generating one...\n");
	}

	printf("==> Merging remaining data...\n");	
	/* writing the remaining data stored in the hash */
	write_new_data(ht);
	/* destroying the hash once we'r done with it */	
	hash_destroy(ht);

	if (close_tmp_config() == false)
		exit(EXIT_FAILURE);

	if (error == 0)
		/* copying the temporary config to the system one */
		if (copy_config(sfn, PREMAKE_CONFIG_PATH) == false)
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
 *	record data
 *
 *	pht : hash table
 *	key : record name
 *	opchar : operation char
 *		PMKSTP_REC_REMV => remove
 *		PMKSTP_REC_UPDT => add/update
 *	value : record data
 *
 *	return : true on success
 */

bool record_data(htable *pht, char *key, char opchar, char *value) {
	prsopt	*ppo;

	switch (opchar) {
		case PMKSTP_REC_REMV :
			/* delete record */
			ppo = prsopt_init_adv(key, opchar, NULL);
			if (ppo == NULL) {
				return(false);
			}
			break;

		case PMKSTP_REC_UPDT :
			/* update record */
			ppo = prsopt_init_adv(key, opchar, value);
			if (ppo == NULL) {
				return(false);
			}
			break;

		default :
			/* unknown operation */
#ifdef PMKSETUP_DEBUG
	debugf("unknown '%c' operator in record_data.", opchar);
#endif
			return(false);
	}

	if (hash_update(pht, ppo->key, ppo) == HASH_ADD_FAIL) {
		prsopt_destroy(ppo);
		return(false);
	}

	return(true);
}


/*
 * gathering of system data, predefinied data, ...
 */

bool gather_data(htable *pht) {
	printf("==> Looking for default parameters...\n");

	/* gather env variables and look for specific binaries */
	if ((get_env_vars(pht) == false) || (get_binaries(pht) == false))
		return(false);

	/* set predifined variables */
	if (predef_vars(pht) == false) {
		errorf("predefined variables."); 
		return(false);
	}

	check_libpath(pht);

	if (byte_order_check(pht) == false) {
		errorf("failure in byte order check.");
		return(false);
	}

	if (check_echo(pht) == false) {
		errorf("failure in echo check.");
		return(false);
	}

	return(true);
}

/*
 * write remaining data
 *
 * 	pht: hash table
 */
void write_new_data(htable *pht) {
	char	*val;
	int	 i;
	hkeys	*phk;
	prsopt	*ppo;

	phk = hash_keys(pht);
	if (phk != NULL) {
		/* sort hash table */
		qsort(phk->keys, phk->nkey, sizeof(char *), keycomp);

		/* processing remaining keys */
		for(i = 0 ; i < phk->nkey ; i++) {
#ifdef PMKSETUP_DEBUG
	debugf("write_new_data: processing '%s'", phk->keys[i]);
#endif
			ppo = hash_get(pht, phk->keys[i]);
			if (ppo == NULL) {
#ifdef PMKSETUP_DEBUG
				debugf("write_new_data: unable to get '%s' record.", phk->keys[i]);
#endif
				/* XXX  exit ? */
			} else {
				/* check if this key is marked for being removed */
				if (ppo->opchar != PMKSTP_REC_REMV) {
					/* add value */
					val = po_get_str(ppo->value);
					if (val != NULL) {
						fprintf(sfp, PMKSTP_WRITE_FORMAT, phk->keys[i], CHAR_ASSIGN_UPDATE, val);
#ifdef PMKSETUP_DEBUG
					} else {
						debugf("write_new_data: skipping '%s', empty value.", phk->keys[i]);
#endif
						/* XXX  exit ? */
					}
#ifdef PMKSETUP_DEBUG
				} else {
					debugf("write_new_data: skipping '%s', remove record.", phk->keys[i]);
#endif
				}
			}
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

bool check_opt(htable *cht, prsopt *popt) {
	char	*recval,
		*optval;
	prsopt	*ppo;

	optval = po_get_str(popt->value);

	if ((popt->opchar == CHAR_COMMENT) || (popt->opchar == CHAR_EOS)) {
		/* found comment or empty line */
		fprintf(sfp, "%s\n", optval);
		return(true);
	}

#ifdef PMKSETUP_DEBUG
	debugf("check_opt: checking key '%s'", popt->key);
#endif

	ppo = hash_get(ht, popt->key);
	/*if ((ppo != NULL) && (ppo->value != NULL)) {*//* on remove value is NULL !!! */
	if (ppo != NULL) {
		/* XXX check value */
		recval = po_get_str(ppo->value);

#ifdef PMKSETUP_DEBUG
		debugf("check_opt: original opchar '%c'", popt->opchar);
#endif

		/* checking the VAR<->VALUE separator */
		switch (popt->opchar) {
			case CHAR_ASSIGN_UPDATE :
#ifdef PMKSETUP_DEBUG
				debugf("check_opt: update opchar '%c'", ppo->opchar);
#endif
				switch (ppo->opchar) {
					case PMKSTP_REC_REMV :
						/* do nothing, value will not be refreshed */
						verbosef("Removing '%s'", popt->key);
						break;
					case PMKSTP_REC_UPDT :
						/* update value */
						fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, CHAR_ASSIGN_UPDATE, recval);
						break;

				}
				hash_delete(cht, popt->key);
				break;

			case CHAR_ASSIGN_STATIC :
				/* static definition, stay unchanged */ 
				fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, CHAR_ASSIGN_STATIC, optval);
				hash_delete(cht, popt->key);
				break;

			default :
				/* should not happen now */
				errorf("unknown operator '%c'.", popt->opchar);
				return(false);
				break;
		}
	} else {
#ifdef PMKSETUP_DEBUG
	debugf("check_opt: key '%s' does not exist.", popt->key);
#endif
		fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, popt->opchar, optval);
	}

	return(true);
}

/*
 * Open temporary configuration file
 *
 *	return:  boolean
 */
bool open_tmp_config(void) {
	int	fd;

	/* creating temporary file to build new configuration file */
	snprintf(sfn, sizeof(sfn), PREMAKE_CONFIG_TMP);

	fd = mkstemp(sfn);
	if (fd == -1) {
		errorf("could not create temporary file '%s' : %s.", 
			sfn, strerror(errno));
		return(false);
	}

	sfp = fdopen(fd, "w");
	if (sfp == NULL) {
		unlink(sfn);
		close(fd);
		errorf("cannot open temporary file '%s' : %s.", 
			sfn, strerror(errno));
		return(false);
	}
	return(true);
}


/*
 * Close temporary configuration file
 *
 *	it deletes the temporary file on exit except if the
 *	PMKSETUP_DEBUG mode is on.
 *
 *	return:  boolean
 */
bool close_tmp_config(void) {
	if (sfp == NULL)
		return(true);

	if (fclose(sfp) != 0) {
		errorf("cannot close temporary file '%s' : %s.", 
			sfp, strerror(errno));
		return(false);
	}

	sfp = NULL; /* XXX useful ? */
	return(true);
}


/*
 * Get the environment variables needed for the configuration file
 *
 *	ht: hash table where we have to store the values
 *
 *	return:  boolean
 */
bool get_env_vars(htable *pht) {
	struct utsname	 utsname;
	char		*bin_path;

	if (uname(&utsname) == -1) {
		errorf("uname : %s.", strerror(errno));
		return(false);
	}

	if (record_data(pht, PMKCONF_OS_NAME, 'u', utsname.sysname) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_OS_NAME, utsname.sysname);

	if (record_data(pht, PMKCONF_OS_VERS, 'u', utsname.release) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_OS_VERS, utsname.release);	
	
	if (record_data(pht, PMKCONF_OS_ARCH, 'u', utsname.machine) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_OS_ARCH, utsname.machine);


	/* getting the environment variable PATH */
	if ((bin_path = getenv("PATH")) == NULL) {
		errorf("could not get the PATH environment variable.");
		return(false);
	}

	/* 
	 * replace the string separator in the PATH 
	 * to fit to our standards
	 */
	char_replace(bin_path, PATH_STR_DELIMITER, CHAR_LIST_SEPARATOR);

	if (record_data(pht, PMKCONF_PATH_BIN, 'u', bin_path) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_PATH_BIN, bin_path);

	return(true);
}


/* 
 * Look for location of some predefined binaries
 *
 *	ht: hash table where we have to store the values
 *
 *	return: boolean 
 */ 
bool get_binaries(htable *pht) {
	char	 fbin[MAXPATHLEN],	/* full binary path */
		*pstr;
	dynary	*stpath;
	int	 i;
	prsopt	*ppo;

        /*
         * splitting the PATH variable and storing in a 
         * dynamic array for later use by find_file
         */

	ppo = hash_get(pht, PMKCONF_PATH_BIN);
	if (ppo == NULL) {
		errorf("unable to get binary path record.");
		return(false);
	}

	pstr = po_get_str(ppo->value);
	if (pstr == NULL) {
		/* XXX error */
		return(false);
	}

	stpath = str_to_dynary(pstr , CHAR_LIST_SEPARATOR);
	if (stpath == NULL) {
		errorf("could not split the PATH environment variable correctly.");
		return(false);	
	}

	if (find_file(stpath, "cc", fbin, sizeof(fbin)) == true) {
		if (record_data(pht, PMKCONF_BIN_CC, 'u', fbin) == false) {
			da_destroy(stpath);
			return(false);
		}
		verbosef("Setting '%s' => '%s'", PMKCONF_BIN_CC, fbin);
	} else {
		if (find_file(stpath, "gcc", fbin, sizeof(fbin)) == true) {
			if (record_data(pht, PMKCONF_BIN_CC, 'u', fbin) == false) {
				da_destroy(stpath);
				return(false);
			}
			verbosef("Setting '%s' => '%s'", PMKCONF_BIN_CC, fbin);
		} else {
			errorf("cannot find a C compiler.");
			da_destroy(stpath);
			return(false);
		}
	}

	for (i = 0; i < MAXBINS; i++) {
		if (find_file(stpath, binaries[i][0], fbin, sizeof(fbin)) == true) {
			if (record_data(pht, binaries[i][1], 'u', fbin) == false) {
				da_destroy(stpath);
				return(false);
			}
			verbosef("Setting '%s' => '%s'", binaries[i][1], fbin);
		} else {
			verbosef("**warning: '%s' Not Found", binaries[i][0]);
		}
	}
	da_destroy(stpath);
	return(true);
}


/*
 * Add to the hash table the predefined variables we
 * cannot get automagically
 *
 *	pht: hash table where we have to store the values
 *
 *	returns:  boolean
 */ 
bool predef_vars(htable *pht) {
	int	i;

	for (i = 0 ; i < nbpredef ; i++) {
		if (record_data(pht, predef[i].key, 'u', predef[i].value) == false) {
			errorf("failed to add '%s'.", predef[i].key);
			return(false);
		}
		verbosef("Setting '%s' => '%s'", predef[i].key, predef[i].value);
	}

	return(true);
}

/*
 *	pht: hash table where we have to store the values
 *
 *	returns:  boolean
 */
bool check_echo(htable *pht) {
	FILE	*echo_pipe = NULL;
	char	buf[TMP_BUF_LEN],
		echocmd[MAXPATHLEN];
	char	*echo_n, *echo_c, *echo_t;
	size_t	s;

	snprintf(echocmd, sizeof(echocmd), ECHO_CMD);

	if ((echo_pipe = popen(echocmd, "r")) == NULL) {
		errorf("unable to execute '%s'.", echocmd);
		return(false);
	}

	s = fread(buf, sizeof(char), sizeof(buf), echo_pipe);
	buf[s] = CHAR_EOS;

	if (feof(echo_pipe) == 0) {
		errorf("pipe not empty.");
		pclose(echo_pipe);
		return(false);
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
					return(false);
				}
			}
		}
	}

	if (record_data(pht, PMKCONF_AC_ECHO_N, 'u', echo_n) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_AC_ECHO_N, echo_n);
	
	if (record_data(pht, PMKCONF_AC_ECHO_C, 'u', echo_c) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_AC_ECHO_C, echo_c);

	if (record_data(pht, PMKCONF_AC_ECHO_T, 'u', echo_t) == false)
		return(false);
	verbosef("Setting '%s' => '%s'", PMKCONF_AC_ECHO_T, echo_t);

	return(true);
}


/*
 *	check pkgconfig libpath
 *
 *	pht: hash table where we have to store the values
 *
 *	returns:  boolean
 *
 *	NOTE: this path must be relative to the real prefix. This means
 *		that we can't rely on predefined prefix.
 */

bool check_libpath(htable *pht) {
	char	 libpath[MAXPATHLEN];

	/* build the path dynamically */
	strlcpy(libpath, "$", sizeof(libpath)); /* variable prefix */
	strlcat(libpath, PMKCONF_MISC_PREFIX, sizeof(libpath)); /* prefix variable name */
	strlcat(libpath, PMKVAL_LIB_PKGCONFIG, sizeof(libpath)); /* pkgconfig path suffix */

	if (hash_get(pht, PMKCONF_BIN_PKGCONFIG) != NULL) {
		if (dir_exists(libpath) == 0) {
			if (record_data(pht, PMKCONF_PC_PATH_LIB, 'u', libpath) == false)
				return(false);
			verbosef("Setting '%s' => '%s'", PMKCONF_PC_PATH_LIB, libpath);
		} else {
			verbosef("**warning: %s does not exist.", libpath);
		}
	}
	return(true);
}

/*
 *	check if a directory does exist 
 *
 *	fdir : directory to search
 *
 *	returns:  boolean
 */

bool dir_exists(const char *fdir) {
        DIR     *dirp;
        size_t  len;
        
	len = strlen(fdir);

	if (len < MAXPATHLEN) {
		dirp = opendir(fdir);
		if (dirp != NULL) {
			closedir(dirp);
			return(true);
		}
	}
	return(false);
}

/*
 *	byte order check
 *
 *	pht : hash table to store value
 *
 *	return : boolean value
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

	if (record_data(pht, PMKCONF_HW_BYTEORDER, 'u', bo_type) == false)
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
 *	returns:  boolean
 */
bool copy_config(const char *tmp_config, const char *config) {
	FILE	*fp_t,
		*fp_c;
	bool	 rval;
	char	 buf[MAX_LINE_BUF];


	if ((fp_t = fopen(tmp_config, "r")) == NULL) {
		errorf("cannot open temporary configuration "
			"file for reading '%s' : %s.", tmp_config, 
				strerror(errno));
		return(false);	
	}
	if ((fp_c = fopen(config, "w")) == NULL) {
		errorf("cannot open '%s' for writing : %s.", 
			config, strerror(errno));

		fclose(fp_t);	
		return(false);
	}

	while(get_line(fp_t, buf, MAX_LINE_BUF) == true) {
		fprintf(fp_c, "%s\n", buf);
	}

	if (feof(fp_t) == 0) {
		errorf("read failure, cannot copy "
			"'%s' to '%s'.", tmp_config, config);
		rval = false;	
	} else
		rval = true;

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
	/*fprintf(stderr, "Usage: %s [-hvV] [-a var=val] "*/
	fprintf(stderr, "Usage: %s [-hvV] "
		"[-r var] [-u var=val]\n", __progname);
	/*fprintf(stderr, "\t-a var=val   Append to a variable in pmk.conf\n");*/
	fprintf(stderr, "\t-r var       Remove a variable in pmk.conf\n");
	fprintf(stderr, "\t-u var=val   Update a variable in pmk.conf\n");
	fprintf(stderr, "\t-h           Display this help menu\n"); 
	fprintf(stderr, "\t-v           Display version number\n");
	fprintf(stderr, "\t-V           Verbose, display debugging messages\n");
	exit(EXIT_FAILURE);
}

