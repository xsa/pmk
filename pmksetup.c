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
/*#define WITHOUT_FORK 1*/

/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"
#include <sys/utsname.h>
#include <sys/wait.h>

#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "detect_cpu.h"
#include "dynarray.h"
#include "parse.h"
#include "pmksetup.h"


FILE	*sfp;			/* scratch file pointer */
char	sfn[MAXPATHLEN];	/* scratch file name */		

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
 * Record data
 *
 *	pht: hash table
 *	key: record name
 *	opchar: operation char
 *		PMKSTP_REC_REMV => remove
 *		PMKSTP_REC_UPDT => add/update
 *	value: record data
 *
 *	returns: true on success
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
			debugf("unknown '%c' operator in record_data.",
								opchar);
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
 * Gathering of system data, predefinied data, ...
 *
 *	pht: storage hash table
 *
 *	returns: true on success else false
 */

bool gather_data(htable *pht) {
	printf("==> Looking for default parameters...\n");

	/* gather env variables */
	if (get_env_vars(pht) == false) {
		errorf("failed to gather env variables.");
		return(false);
	}

 	/* gather env variables and look for specific binaries */
	if (get_binaries(pht) == false) {
		errorf("failed to locate binaries.");
		return(false);
	}
 
	/* set predifined variables */
	if (predef_vars(pht) == false) {
		errorf("predefined variables.");
		return(false);
	}

	check_libpath(pht);

	/* check byte order */
	if (byte_order_check(pht) == false) {
		errorf("failure in byte order check.");
		return(false);
	}

	/* try to detect cpu family and specific data */
	if (get_cpu_data(pht) == false) {
		errorf("failure in cpu detection.");
		return(false);
	}

	if (check_echo(pht) == false) {
		errorf("failure in echo check.");
		return(false);
	}

	return(true);
}

/*
 * Write remaining data
 *
 * 	pht: hash table
 *
 * 	returns: -
 */

bool write_new_data(htable *pht) {
	char		*val;
	hkeys		*phk;
	prsopt		*ppo;
	unsigned int	 i;

	phk = hash_keys(pht);
	if (phk == NULL) {
		verbosef("Nothing to merge.");
		return(true);
	}

	/* sort hash table */
	qsort(phk->keys, phk->nkey, sizeof(char *), keycomp);

	/* processing remaining keys */
	for(i = 0 ; i < phk->nkey ; i++) {
#ifdef PMKSETUP_DEBUG
		debugf("write_new_data: processing '%s'", phk->keys[i]);
#endif
		ppo = hash_get(pht, phk->keys[i]);
		if (ppo == NULL) {
			errorf("unable to get '%s' record.", phk->keys[i]);
			return(false);
		}

		/* check if this key is marked for being removed */
		if (ppo->opchar == PMKSTP_REC_REMV) {
#ifdef PMKSETUP_DEBUG
			debugf("write_new_data: skipping '%s', remove record.",
								phk->keys[i]);
#endif
			continue;
		}

		/* add value */
		val = po_get_str(ppo->value);
		if (val == NULL) {
			errorf("'%s' key has an empty value.", phk->keys[i]);
			return(false);
		}

		fprintf(sfp, PMKSTP_WRITE_FORMAT, phk->keys[i],
						CHAR_ASSIGN_UPDATE, val);
	}

	hash_free_hkeys(phk);

	return(true);
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
 *	returns: an integer greater than, equal to, or
 *		less than 0 according as the character a is
 *		greather, equal to, or less than the character b.
 */

int keycomp(const void *a, const void *b) {
	return(strcmp(*(const char **) a, *(const char **) b));
}

/*
 * Check and process option
 *
 *	pht: htable for comparison
 *	popt: option to process
 *
 *	returns: boolean
 */

bool check_opt(htable *cht, prsopt *popt) {
	char	*recval,
		*optval;
	prsopt	*ppo;

	optval = po_get_str(popt->value);

	if ((popt->opchar == CHAR_COMMENT) || (popt->opchar == CHAR_EOS)) {
		/* found comment or empty line */
		fprintf(sfp, "%s\n", optval);
#ifdef PMKSETUP_DEBUG
		debugf("check_opt() appended '%s\n'", optval);
#endif
		return(true);
	}

#ifdef PMKSETUP_DEBUG
	debugf("check_opt: checking key '%s'", popt->key);
#endif

	ppo = hash_get(ht, popt->key);
	if (ppo == NULL) {
#ifdef PMKSETUP_DEBUG
		debugf("check_opt: key '%s' does not exist.", popt->key);
#endif
		fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, popt->opchar, optval);
		return(true);
	}

	/* XXX check value ? */
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
#ifdef PMKSETUP_DEBUG
					debugf("check_opt() appended '" PMKSTP_WRITE_FORMAT "'", popt->key, CHAR_ASSIGN_UPDATE, recval);
#endif
					break;

			}

			hash_delete(cht, popt->key);
			break;

		case CHAR_ASSIGN_STATIC :
			/* static definition, stay unchanged */
			fprintf(sfp, PMKSTP_WRITE_FORMAT, popt->key, CHAR_ASSIGN_STATIC, optval);
#ifdef PMKSETUP_DEBUG
			debugf("check_opt() appended '" PMKSTP_WRITE_FORMAT "'", popt->key, CHAR_ASSIGN_STATIC, optval);
#endif
			hash_delete(cht, popt->key);
			break;

		default :
			/* should not happen now */
			errorf("unknown operator '%c'.", popt->opchar);
			return(false);
			break;
	}

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
 *	returns: boolean
 */

bool get_binaries(htable *pht) {
	char		 fbin[MAXPATHLEN],	/* full binary path */
			*pstr;
	dynary		*stpath;
	prsopt		*ppo;
	unsigned int	 i;

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
		errorf("unable to get binary path value.");
		return(false);
	}

	stpath = str_to_dynary(pstr , PATH_STR_DELIMITER);
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
 * Check echo output
 *
 *	pht: hash table where we have to store the values
 *
 *	returns:  boolean
 */

bool check_echo(htable *pht) {
	FILE	*echo_pipe = NULL;
	char	buf[TMP_BUF_LEN],
		echocmd[TMP_BUF_LEN];
	char	*echo_n, *echo_c, *echo_t;
	size_t	s;

	snprintf(echocmd, sizeof(echocmd), ECHO_CMD); /* should not fail */

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
 * Check pkgconfig libpath
 *
 *	pht: hash table where we have to store the values
 *
 *	returns:  boolean
 *
 * NOTE: this path must be relative to the real prefix. This means
 * that we can't rely on predefined prefix.
 */

bool check_libpath(htable *pht) {
	char	 libpath[MAXPATHLEN];

	/* build the path dynamically */
	/* variable prefix */
	strlcpy(libpath, "$", sizeof(libpath)); /* should not fail */

	/* prefix variable name */
	strlcat(libpath, PMKCONF_MISC_PREFIX,
				sizeof(libpath)); /* no check yet */

	/* pkgconfig path suffix */
	if (strlcat_b(libpath, PMKVAL_LIB_PKGCONFIG,
				sizeof(libpath)) == false)
		return(false);

	if (hash_get(pht, PMKCONF_BIN_PKGCONFIG) != NULL) {
		if (dir_exists(libpath) == 0) {
			if (record_data(pht, PMKCONF_PC_PATH_LIB,
						'u', libpath) == false)
				return(false);

			verbosef("Setting '%s' => '%s'",
					PMKCONF_PC_PATH_LIB, libpath);
		} else {
			verbosef("**warning: %s does not exist.", libpath);
		}
	}
	return(true);
}

/*
 * Check the cpu family 
 *
 *	uname_m: uname machine string 
 *
 *	returns:  cpu family string or NULL
 */

bool get_cpu_data(htable *pht) {
	char		*uname_m;
	char		*pstr;
	hkeys		*phk;
	htable		*spht;
	pmkobj		*po;
	prsdata		*pdata;
	prsopt		*ppo;
	unsigned int	 i;

	ppo = hash_get(pht, PMKCONF_OS_ARCH);
	if (ppo == NULL) {
		errorf("failed to get value for %s", PMKCONF_OS_ARCH);
		return(false);
	}

	po = ppo->value;
	if (po == NULL) {
		errorf("unexpected data for %s", PMKCONF_OS_ARCH);
		return(false);
	}

	uname_m = po_get_str(po);
	if (uname_m == NULL) {
		errorf("unexpected value for %s", PMKCONF_OS_ARCH);
		return(false);
	}

	pdata = parse_cpu_data(PMKCPU_DATA);
	if (pdata == NULL) {
		/* error message already done */
		return(false);
	}

	pstr = check_cpu_arch(uname_m, pdata); /* no check, never NULL */
	if (record_data(pht, PMKCONF_HW_CPU_ARCH, 'u', pstr) == false) {
		errorf("failed to record value for %s", PMKCONF_HW_CPU_ARCH);
		return(false);
	}
	verbosef("Setting '%s' => '%s'", PMKCONF_HW_CPU_ARCH, pstr);

	spht = arch_wrapper(pdata, pstr);
	if (spht != NULL) {
		phk = hash_keys(spht);
		if (phk != NULL) {
			for(i = 0 ; i < phk->nkey ; i++) {
				pstr = hash_get(spht, phk->keys[i]); /* should not be NULL */

				if (record_data(pht, phk->keys[i], 'u', pstr) == false)
					return(false);
				verbosef("Setting '%s' => '%s'", phk->keys[i], pstr);
			}

			hash_free_hkeys(phk);
		}
		hash_destroy(spht);
	}

	prsdata_destroy(pdata);

	return(true);
}

/*
 * Check if a directory does exist
 *
 *	fdir: directory to search
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
 * Byte order check
 *
 *	pht: hash table to store value
 *
 *	returns: boolean value
 */

bool byte_order_check(htable *pht) {
	char	bo_type[16];
	int	num = 0x41424344;

	if (((((char *)&num)[0]) == 0x41) && ((((char *)&num)[1]) == 0x42) &&
	    ((((char *)&num)[2]) == 0x43) && ((((char *)&num)[3]) == 0x44)) {
		strlcpy(bo_type, HW_ENDIAN_BIG,
				sizeof(bo_type)); /* should not fail */
	} else {
		if (((((char *)&num)[3]) == 0x41) &&
				((((char *)&num)[2]) == 0x42) &&
				((((char *)&num)[1]) == 0x43) &&
				((((char *)&num)[0]) == 0x44)) {
			strlcpy(bo_type, HW_ENDIAN_LITTLE,
					sizeof(bo_type)); /* should not fail */
		} else {
			strlcpy(bo_type, HW_ENDIAN_UNKNOWN,
					sizeof(bo_type)); /* should not fail */
		}
	}

	if (record_data(pht, PMKCONF_HW_BYTEORDER, 'u', bo_type) == false)
		return(false);

	verbosef("Setting '%s' => '%s'", PMKCONF_HW_BYTEORDER, bo_type);

	return(true);
}

/*
 * Simple formated verbose function
 *
 *	fmt: format string
 *
 *	returns: -
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
	fprintf(stderr, "usage: pmksetup [-hVv] "
		"[-r variable] [-u variable=value]\n");
	exit(EXIT_FAILURE);
}

/*
 * Detection loop
 *
 *	argc:	argument number
 *	argv:	argument array
 *
 *	returns: true on success else false
 */

bool detection_loop(int argc, char *argv[]) {
	FILE		*config;
	bool		 process_clopts = false,
			 cfg_backup = false;
	char		*pstr;
	int		 ch;
	prsopt		 *ppo;
	
	extern int	 optind;
#ifndef errno
	extern int	 errno;
#endif /* errno */

	/*while ((ch = getopt(argc, argv, "a:hr:u:vV")) != -1)*/
	while ((ch = getopt(argc, argv, "hr:u:vV")) != -1)
		switch(ch) {
			case 'r' :
				/* mark to be deleted in hash */
				if (record_data(ht, optarg, PMKSTP_REC_REMV, NULL) == false) {
					errorf("failed to record '%s'.", optarg);
					return(false);
				}

				process_clopts = true;
				break;

			case 'u' :
				/* add or update value */
				ppo = prsopt_init();
				if (ppo == NULL) {
					errorf("prsopt init failed.");
					return(false);
				}

				if (parse_clopt(optarg, ppo, PRS_PMKCONF_SEP) == false) {
					errorf("bad argument for -a option: %s.", optarg);
					return(false);
				}

				/* add in hash */
				pstr = po_get_str(ppo->value);
				if (pstr == NULL) {
					errorf("failed to get argument for -a option");
					return(false);
				}

				/* record prsopt structure directly, fast and simple */
				ppo->opchar = PMKSTP_REC_UPDT;
				if (hash_update(ht, ppo->key, ppo) == HASH_ADD_FAIL) {
					prsopt_destroy(ppo);	
					errorf("hash update failed.");	
					return(false);
				}

				/* XXX use record_data => cost much memory and cpu XXX            */
				/*if (record_data(ht, ppo->key, PMKSTP_REC_UPDT, pstr) == false) {*/
				/*        errorf("failed to record '%s'.", ppo->key);             */
				/*        prsopt_destroy(ppo);                                    */
				/*        return(false);                                          */
				/*}                                                               */
				/*prsopt_destroy(ppo);						  */

				process_clopts = true;
				break;

			case 'v' :
				fprintf(stderr, "%s\n", PREMAKE_VERSION);
				return(true);
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
#endif /* DEBUG */
	printf("\n\n");

	if (process_clopts == false) {
		/* standard behavior, gathering data */
		if (gather_data(ht) == false)
			return(false);
	}

	/* parse configuration file */
	config = fopen(PREMAKE_CONFIG_PATH, "r");
	if (config != NULL) {
		/* switch backup flag */
		cfg_backup = true;

		printf("==> Configuration file found: %s\n",
						PREMAKE_CONFIG_PATH);
		if (parse_pmkconf(config, ht, PRS_PMKCONF_SEP,
						check_opt) == false) {
			fclose(config);
			errorf("parsing failed.");
			return(false);
		} else {
			fclose(config);
		}
	} else {
		printf("==> Configuration file not found.\n");
	}

	printf("==> Merging remaining data...\n");
	/* writing the remaining data stored in the hash */
	if (write_new_data(ht) == false)
		return(false);

	return(true);
}

/*
 * Child main loop
 *
 *	uid:	privsep user id
 *	gid:	privsep user group
 *	argc:	argument number
 *	argv:	argument array
 *
 *	returns: -
 */

void child_loop(uid_t uid, gid_t gid, int argc, char *argv[]) {
#ifndef WITHOUT_FORK
	if (getuid() == 0) {
		/* user has root privs, DROP THEM ! */
		if (setgid(gid) != 0) {
			/* failed to change gid */
			errorf(EMSG_PRIV_FMT, "gid");
			_exit(EXIT_FAILURE);
		}

		if (setegid(gid) != 0) {
			/* failed to change egid */
			errorf(EMSG_PRIV_FMT, "egid");
			_exit(EXIT_FAILURE);
		}

		if (setuid(uid) != 0) {
			/* failed to change uid */
			errorf(EMSG_PRIV_FMT, "uid");
			_exit(EXIT_FAILURE);
		}

		if (seteuid(uid) != 0) {
			/* failed to change euid */
			errorf(EMSG_PRIV_FMT, "euid");
			_exit(EXIT_FAILURE);
		}
	}
#endif /* WITHOUT_FORK */

#ifdef PMKSETUP_DEBUG
	debugf("gid = %d", getgid());
	debugf("uid = %d", getuid());
#endif /* PMKSETUP_DEBUG */

	ht = hash_init_adv(MAX_CONF_OPT, NULL, (void (*)(void *)) prsopt_destroy, NULL);
	if (ht == NULL) {
		errorf("cannot create hash table.");
		exit(EXIT_FAILURE);
	}

	if (detection_loop(argc, argv) == false) {
		hash_destroy(ht);
		exit(EXIT_FAILURE);
	}

	/* destroying the hash once we're done with it */
	hash_destroy(ht);
		
}

/*
 * Parent main loop
 *
 *	pid: pid of the child
 *
 *	returns: -
 */

void parent_loop(pid_t pid) {
	bool	error = false;
#ifndef WITHOUT_FORK
	int	status;

	waitpid(pid, &status, WUNTRACED);

#ifdef PMKSETUP_DEBUG
	debugf("waitpid() status = %d", WEXITSTATUS(status));
#endif /* PMKSETUP_DEBUG */

	if (status != 0) {
		errorf("child failed (status %d)", status);
		exit(EXIT_FAILURE);
	}
#endif /* WITHOUT_FORK */

	if (sfp != NULL) {
		if (fclose(sfp) != 0) {
			/* hazardous result => exit */
			errorf("cannot close temporary file '%s' : %s.",
						sfp, strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* check if syconfdir exists */
		if (access(CONFDIR, F_OK) != 0) { /* no race condition, just mkdir() */
			verbosef("==> Creating '%s' directory.", CONFDIR);
			if (mkdir(CONFDIR, S_IRWXU | S_IRGRP | S_IXGRP |
						S_IROTH | S_IXOTH) != 0) {
				errorf("cannot create '%s' directory : %s.",
						CONFDIR, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		/*
			check if pmk.conf already exists

			NOTE: no race condition here for access(), BUT
			BE CAREFUL if changes are to be made.
		*/ 
		if (access(PREMAKE_CONFIG_PATH, F_OK) == 0) { /* see above */
			/* backup configuration file */
			printf("==> Backing up configuration file: %s\n",
						PREMAKE_CONFIG_PATH_BAK);

			if (rename(PREMAKE_CONFIG_PATH,
					PREMAKE_CONFIG_PATH_BAK) != 0) {
				errorf("configuration file backup failed: %s.",
							strerror(errno));
				error = true;
			}
		}

		/* copying the temporary config to the system one */
		printf("==> Saving configuration file: %s\n", PREMAKE_CONFIG_PATH);
		if (fcopy(sfn, PREMAKE_CONFIG_PATH, PREMAKE_CONFIG_MODE) == false)
			exit(EXIT_FAILURE);

#ifdef PMKSETUP_DEBUG
		debugf("%s has not been deleted!", sfn);
#else
		if (unlink(sfn) == -1) {
			errorf("cannot remove temporary file: '%s' : %s.",
						sfn, strerror(errno));
			error = true;
		}
#endif	/* PMKSETUP_DEBUG */
	}

	if (error == true)
		exit(EXIT_FAILURE);
}

/*
 * Main loop
 */

int main(int argc, char *argv[]) {
	struct passwd	*pw;
	gid_t		 gid = 0;
	pid_t		 pid;
	uid_t		 uid = 0;

	if (getuid() == 0) {
#ifdef PMKSETUP_DEBUG
		debugf("PRIVSEP_USER = '%s'", PRIVSEP_USER);
#endif /* PMKSETUP_DEBUG */
		pw = getpwnam(PRIVSEP_USER);
		if (pw == NULL) {
			errorf("cannot get user data for '%s'.", PRIVSEP_USER);
			exit(EXIT_FAILURE);
		}
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}

	sfp = tmp_open(PREMAKE_CONFIG_TMP, "w", sfn, sizeof(sfn));
	if (sfp == NULL) {
		errorf("cannot open temporary file '%s' : %s.",
				sfn, strerror(errno));
		exit(EXIT_FAILURE);
	}

#ifndef WITHOUT_FORK
	/* forking detection code */
	pid = fork();
	switch (pid) {
		case -1:
			/* fork failed */
			errorf("fork failed.");
			break;

		case 0:
			/* child start here after forking */
#endif /* WITHOUT_FORK */
			child_loop(uid, gid, argc, argv);
#ifndef WITHOUT_FORK
			break;

		default:
			/* parent continue here after forking */
#else
			pid = getpid();
#endif /* WITHOUT_FORK */
			parent_loop(pid);
#ifndef WITHOUT_FORK
			break;
	}
#endif /* WITHOUT_FORK */

	return(EXIT_SUCCESS);

}
