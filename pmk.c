/* $Id$ */

/*
 * Credits for patches :
 *	- Pierre-Yves Ritschard
 */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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


#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_libgen.h" /* basename, dirname */
#include "compat/pmk_string.h" /* strlcpy */
#include "compat/pmk_unistd.h"
#include "autoconf.h"
#include "common.h"
#include "func.h"
#include "pathtools.h"
#include "pkgconfig.h"
#include "pmk.h"
#include "parse.h"

/*#define PMK_DEBUG	1*/

extern char	*optarg;
extern int	 optind;
extern prskw	 kw_pmkfile[];
extern size_t	 nbkwpf;

int		 cur_line = 0;


/*
	init variables

	pht : global data structure
	template : file to process

	return : -
*/

void process_dyn_var(pmkdata *pgd, char *template) {
	char	*srcdir,
		*basedir,
		*pstr,
		 buf[MAXPATHLEN],
		 rpath[MAXPATHLEN],
		 stpath[MAXPATHLEN],
		 btpath[MAXPATHLEN];
	htable	*pht;

	pht = pgd->htab;

	srcdir = pgd->srcdir;
	basedir = pgd->basedir;

	/* get template path */
	/* NOTE : we use strdup to avoid problem with linux's dirname */
	pstr = strdup(template);
	strlcpy(stpath, dirname(pstr), sizeof(stpath));
	free(pstr);

	/* compute relative path */
	relpath(srcdir, stpath, rpath);

	/* compute builddir_abs with relative path */
	abspath(basedir, rpath, btpath); 
	hash_update_dup(pht, PMK_DIR_BLD_ABS, btpath);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_ABS, buf);
#endif

	/* compute relative path to builddir root */
	relpath(btpath, basedir, buf);
	hash_update_dup(pht, PMK_DIR_BLD_ROOT_REL, buf);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_ROOT_REL, buf);
#endif

	/* set buildir_rel to '.', useful ? */
	hash_update_dup(pht, PMK_DIR_BLD_REL, ".");
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_REL, buf);
#endif

	/* compute and set relative path from basedir to srcdir */
	relpath(btpath, srcdir, buf);
	hash_update_dup(pht, PMK_DIR_SRC_ROOT_REL, buf);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_ROOT_REL, buf);
#endif

	/* set absolute path of template */
	hash_update_dup(pht, PMK_DIR_SRC_ABS, stpath);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_ABS, buf);
#endif

	/* compute and set relative path from template to builddir */
	relpath(btpath, stpath, buf);
	hash_update_dup(pht, PMK_DIR_SRC_REL, buf);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_REL, buf);
#endif
}

/*
	init variables

	pgd : global data structure

        return : -

        NOTE : XXX check hash_add ?
*/

void init_var(pmkdata *pgd) {
	char	 buf[TMP_BUF_LEN],
                *pstr;
	htable	*pht;

	pht = pgd->htab;

	pstr = MK_VAR_CFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS;
		pmk_log("unable to get %s value, set as empty\n");
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	hash_update_dup(pht, pstr, buf); /* XXX check ? */

	pstr = MK_VAR_CXXFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS;
		pmk_log("unable to get %s value, set as empty\n");
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	hash_update_dup(pht, pstr, buf); /* XXX check ? */

	pstr = MK_VAR_CPPFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS;
		pmk_log("unable to get %s value, set as empty\n");
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	hash_update_dup(pht, pstr, buf); /* XXX check ? */

	pstr = MK_VAR_LDFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS;
		pmk_log("unable to get %s value, set as empty\n");
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	hash_update_dup(pht, pstr, buf); /* XXX check ? */

	pstr = MK_VAR_LIBS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS;
		pmk_log("unable to get %s value, set as empty\n");
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	hash_update_dup(pht, pstr, buf); /* XXX check ? */

	pstr = MK_VAR_DEBUG;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS;
		pmk_log("unable to get %s value, set as empty\n");
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	hash_update_dup(pht, pstr, buf); /* XXX check ? */

	/* autoconf shit ? */
	hash_update_dup(pht, "OBJEXT", "o"); /* XXX check ? */


	pstr = hash_get(pht, PMKCONF_BIN_CC);
	if (pstr != NULL)
		hash_update_dup(pht, "CC", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_CXX);
	if (pstr != NULL)
		hash_update_dup(pht, "CXX", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_CPP);
	if (pstr != NULL)
		hash_update_dup(pht, "CPP", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_INSTALL);
        /* append -c for compat with autoconf*/
        snprintf(buf, sizeof(buf), "%s -c", pstr);
	if (pstr != NULL)
		hash_update_dup(pht, "INSTALL", buf);

	pstr = hash_get(pht, PMKCONF_BIN_RANLIB);
	if (pstr != NULL)
		hash_update_dup(pht, "RANLIB", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_SH);
	if (pstr != NULL)
		hash_update_dup(pht, "SHELL", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_STRIP);
	if (pstr != NULL)
		hash_update_dup(pht, "STRIP", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_AWK);
	if (pstr != NULL)
		hash_update_dup(pht, "AWK", pstr);

	pstr = hash_get(pht, PMKCONF_BIN_EGREP);
	if (pstr != NULL)
		hash_update_dup(pht, "EGREP", pstr);

	/* set absolute paths */
	hash_update_dup(pht, PMK_DIR_SRC_ROOT_ABS, pgd->srcdir);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_ROOT_ABS, pgd->srcdir);
#endif
	hash_update_dup(pht, PMK_DIR_BLD_ROOT_ABS, pgd->basedir);
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_ROOT_ABS, pgd->basedir);
#endif
}

/*
	process option line of configuration file

	pht : storage hash table
	popt : option structure to record

	return : boolean
*/

bool process_opt(htable *pht, prsopt *popt) {
	if ((popt->opchar != CHAR_COMMENT) && (popt->opchar != CHAR_EOS)) {
		/* add options that are not comment neither blank lines */
		if (hash_update_dup(pht, popt->key, po_get_str(popt->value)) == HASH_ADD_FAIL) {
			errorf("hash failure.");
			return(false);
		}
	}
	return(true);
}

/*
	process the target file to replace tags

	target : path of the target file
	pgd : global data structure

	return : boolean
*/

bool process_template(char *template, pmkdata *pgd) {
	FILE	*tfd,
		*dfd;
	bool	 ac_flag;
	char	*plb,
		*pbf,
		*ptn,
		*pfn,
		*ptmp,
		 lbuf[MAXPATHLEN],
		 buf[MAXPATHLEN],
		 tbuf[MAXPATHLEN],
		 tpath[MAXPATHLEN],
		 fpath[MAXPATHLEN],
		 cibuf[TMP_BUF_LEN];
	htable	*pht;

	pht = pgd->htab;

	/* save template name */
	ptn = strdup(basename(template));

	/* get the file name */
	if (strlcpy(lbuf, ptn, sizeof(lbuf)) >= sizeof(lbuf)) {
		errorf("buffer overflow");
		return(false);
	}

	/* remove the last suffix */
	ptmp = strrchr(lbuf, '.');
	if (ptmp != NULL) {
		*ptmp = CHAR_EOS;
	} else {
		errorf("error while creating name of template");
		return(false);
	}

	/* save generated file name */
	pfn = strdup(basename(lbuf));

	/* get relative path from srcdir */
	/* NOTE : we use strdup to avoid problem with linux's dirname */
	ptmp = strdup(template);
	relpath(pgd->srcdir, dirname(ptmp), buf); /* XXX check ? */
	free(ptmp);
	/* apply to basedir */
	abspath(pgd->basedir, buf, tpath); /* XXX check ? */

	/* create path if it does not exists */
	if (makepath(tpath, S_IRWXU | S_IRWXG | S_IRWXO) == false) {
		errorf("cannot build template path '%s'.", tpath);
		return(false);
	}
	
	/* append filename */
	abspath(tpath, lbuf, fpath);

	tfd = fopen(template, "r");
	if (tfd == NULL) {
		errorf("cannot open %s.", template);
		return(false);
	}

	dfd = fopen(fpath, "w");
	if (dfd == NULL) {
		fclose(tfd); /* close already opened tfd before leaving */
		errorf("cannot open %s.", fpath);
		return(false);
	}

	process_dyn_var(pgd, template); /* XXX should use directly path ? */

	if (pgd->ac_file == NULL) {
		ac_flag = false;
	} else {
		ac_flag = true;
		ac_process_dyn_var(pgd, template); /* XXX should use directly path ? */
	}

	snprintf(cibuf, sizeof(cibuf), "%s, generated from %s by PMK.", pfn, ptn);
	hash_update_dup(pht, "configure_input", cibuf); /* XXX check ? */
	free(ptn);
	free(pfn);

	while (fgets(lbuf, sizeof(lbuf), tfd) != NULL) {
		plb = lbuf;
		pbf = buf;

		while (*plb != CHAR_EOS) {
			if (*plb == PMK_TAG_CHAR) {
				plb++;
				/* get tag identifier */
				ptmp = parse_identifier(plb, tbuf, sizeof(tbuf));
				if (ptmp == NULL) {
					errorf("process_template(), buffer too small", fpath);
					return(false);
				}

				plb = ptmp;

				if (*ptmp == PMK_TAG_CHAR) {
					ptmp = (char *) hash_get(pht, tbuf);
					if (ptmp == NULL) {
						/* not a valid tag, put it back */
						*pbf = PMK_TAG_CHAR; /* first tag character */
						pbf++;
						ptmp = tbuf;
					} else {
						/* tag ok, skip ending tag char */
						plb++;
					}

				} else {
					/* not an identifier */
					ptmp = tbuf;
					*pbf = PMK_TAG_CHAR; /* first tag character */
					pbf++;
				}

				/* copy */
				while (*ptmp != CHAR_EOS) {
					*pbf = *ptmp;
					pbf++;
					ptmp++;
				}
			} else {
				/* copy normal char */
				*pbf = *plb;
				pbf++;
				plb++;
                        }
		}

                *pbf = CHAR_EOS;
		/* saving parsed line */
		fprintf(dfd, "%s", buf);
	}

	fclose(dfd);
	fclose(tfd);

	pmk_log("Created '%s'.\n", fpath);

	if (ac_flag == true) {
		/* clean dyn_var */
		ac_clean_dyn_var(pht);
	}

	hash_delete(pht, "configure_input");
		
	return(true);
}

/*
	process the parsed command

	pdata : parsed data
	pgd : global data structure

	return : boolean
*/

bool process_cmd(prsdata *pdata, pmkdata *pgd) {
	return(process_node(pdata->tree, pgd));
}

/*
	process command line values

	val : array of defines
	nbval : size of the array
	pgd : global data structure

	return : boolean (true on success)
*/

bool parse_cmdline(char **val, int nbval, pmkdata *pgd) {
	bool	 rval = true;
	htable	*ht;
	int	 i;
	prsopt	 opt;

	/* don't init pscell */
	ht = pgd->htab;

	for (i = 0 ; (i < nbval) && (rval == true) ; i++) {
		/* parse option */
		rval = parse_opt(val[i], &opt, PRS_PMKCONF_SEP);
		if (rval == true) {
			if (hash_update(ht, opt.key, opt.value) == HASH_ADD_FAIL) { /* no need to strdup */
				errorf("%s", PRS_ERR_HASH);
				rval = false;
			}
		}
	}

	return(rval);
}

/*
	clean global data

	pgd : global data structure

	return : -
*/

void clean(pmkdata *pgd) {
	if (pgd->htab != NULL) {
		hash_destroy(pgd->htab);
	}

	if (pgd->labl != NULL) {
		hash_destroy(pgd->labl);
	}
	if (pgd->cfgt != NULL) {
		cfgtdata_destroy(pgd->cfgt);
	}
	if (pgd->tlist != NULL) {
		da_destroy(pgd->tlist);
	}

	if (pgd->ac_file != NULL) {
		free(pgd->ac_file);
	}

	if (pgd->lang != NULL) {
		free(pgd->lang);
	}
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmk [-vh] [-e list] [-d list] [-b path]\n");
	fprintf(stderr, "\t[-f pmkfile] [-o ovrfile] [options]\n");
}

/*
	main
*/

int main(int argc, char *argv[]) {
	FILE		*fp;
	bool		 go_exit = false,
			 pmkfile_set = false,
			 ovrfile_set = false,
			 basedir_set = false,
			 buildlog = false;
	char		*pstr,
			*enable_sw = NULL,
			*disable_sw = NULL,
			 buf[MAXPATHLEN];
	dynary		*da;
	int		 rval = 0,
			 nbpd,
			 nbcd,
			 ovrsw = 0,
			 i,
			 chr;
	pmkdata		 gdata;
	prsdata		*pdata;

	/* get current path */
	if (getcwd(buf, sizeof(buf)) == NULL) {
		errorf("Unable to get current directory");
		exit(EXIT_FAILURE);
	}

	while (go_exit == false) {
		chr = getopt(argc, argv, "b:d:e:f:hlo:v");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'b' :
					/* XXX check if path is valid */

					if (uabspath(buf, optarg, gdata.basedir) == false) {
						errorf("Cannot use basedir argument");
						exit(EXIT_FAILURE);
					}
					basedir_set = true;
					break;

				case 'e' :
					/* enable switch(es) */
					enable_sw = strdup(optarg);
					break;

				case 'd' :
					/* disable switch(es) */
					disable_sw = strdup(optarg);
					break;

				case 'f' :
					/* pmk file path */
					/* XXX check if path is valid */
					if (uabspath(buf, optarg, gdata.pmkfile) == false) {
						errorf("Cannot use file argument");
						exit(EXIT_FAILURE);
					}

					/* path of pmkfile is also the srcdir base */
					/* NOTE : we use strdup to avoid problem with linux's dirname */
					pstr = strdup(gdata.pmkfile); 
					strlcpy(gdata.srcdir, dirname(pstr), sizeof(gdata.srcdir)); /* XXX check ??? */
					free(pstr);

					pmkfile_set = true;
					break;

				case 'l' :
					/* enable log of test build */
					buildlog = true;
					break;

				case 'o' :
					/* file path to override pmk.conf */
					/* XXX check if path is valid */
					if (uabspath(buf, optarg, gdata.ovrfile) == false) {
						errorf("Cannot use file argument");
						exit(EXIT_FAILURE);
					}

					ovrfile_set = true;
					break;

				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(EXIT_SUCCESS);
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					exit(EXIT_FAILURE);
					break;
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

	/* set basedir if needed */
	if (basedir_set == false) {
		strlcpy(gdata.basedir, buf, sizeof(gdata.basedir));
	}

	/* set pmkfile and srcdir if needed */
	if (pmkfile_set == false) {
		strlcpy(gdata.srcdir, buf, sizeof(gdata.srcdir));
		abspath(gdata.srcdir, PREMAKE_FILENAME, gdata.pmkfile); /* should not fail */
	}

	if (buildlog == true) {
		strlcpy(gdata.buildlog, PMK_BUILD_LOG, sizeof(gdata.buildlog));
	} else {
		strlcpy(gdata.buildlog, "/dev/null", sizeof(gdata.buildlog));
	}

	/* initialise global data hash table */
	gdata.htab = hash_init(MAX_DATA_KEY);
	if (gdata.htab == NULL) {
		clean(&gdata);
		errorf("cannot initialize hash table for data.");
		exit(EXIT_FAILURE);
	}

	gdata.labl = hash_init(MAX_LABEL_KEY);
	if (gdata.labl == NULL) {
		clean(&gdata);
		errorf("cannot initialize hash table for labels.");
		exit(EXIT_FAILURE);
	}

	/* init prsdata structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot intialize prsdata.");
		exit(EXIT_FAILURE);
	}

	fp = fopen(PREMAKE_CONFIG_PATH, "r");
	if (fp != NULL) {
		if (parse_pmkconf(fp, gdata.htab, PRS_PMKCONF_SEP, process_opt) == false) {
			/* parsing failed */
			clean(&gdata);
			errorf("failed to parse '%s'", PREMAKE_CONFIG_PATH);
			exit(EXIT_FAILURE);
		}
		fclose(fp);
		nbpd = hash_nbkey(gdata.htab);
	} else {
		/* configuration file not found */
		clean(&gdata);
		errorf("cannot parse '%s', run pmksetup", PREMAKE_CONFIG_PATH);
		exit(EXIT_FAILURE);
	}

	/* initialize some variables */
	init_var(&gdata);

	if (enable_sw != NULL) {
		/* command line switches to enable */
		da = str_to_dynary(enable_sw, CHAR_LIST_SEPARATOR);
		if (da != NULL) {
			pstr = da_pop(da);	
			while (pstr != NULL) {
				hash_update_dup(gdata.labl, pstr, BOOL_STRING_TRUE); /* XXX BOOL */
				ovrsw++; /* increment number of overriden switches */
				free(pstr);
				pstr = da_pop(da);	
			}
			da_destroy(da);
		}
	}

	if (disable_sw != NULL) {
		/* command line switches to disable */
		da = str_to_dynary(disable_sw, CHAR_LIST_SEPARATOR);
		if (da != NULL) {
			do {
				pstr = da_pop(da);
				if (pstr != NULL) {
					hash_update_dup(gdata.labl, pstr, BOOL_STRING_FALSE); /* XXX BOOL check */
					ovrsw++; /* increment number of overriden switches */
					free(pstr);
				}
			} while (pstr != NULL);
			da_destroy(da);
		}
	}

	if (ovrfile_set == true) {
		/* read override file */
		fp = fopen(gdata.ovrfile, "r");
		if (fp != NULL) {
			if (parse_pmkconf(fp, gdata.htab, PRS_PMKCONF_SEP, process_opt) == false) {
				/* parsing failed */
				clean(&gdata);
				errorf("failed to parse '%s'", gdata.ovrfile);
				exit(EXIT_FAILURE);
			}
			fclose(fp);
			nbpd = hash_nbkey(gdata.htab);
		} else {
			/* configuration file not found */
			clean(&gdata);
			errorf("cannot parse '%s'");
			exit(EXIT_FAILURE);
		}
	}

	if (argc != 0) {
		/* parse optional arguments that override pmk.conf and override file */
		if (parse_cmdline(argv, argc, &gdata) == true) {
			nbcd = argc;
		} else {
			clean(&gdata);
			errorf("incorrect optional arguments");
			exit(EXIT_FAILURE);
		}
	} else {
		nbcd = 0;
	}

	/* open log file */
	if (pmk_log_open(PREMAKE_LOG) == false) {
		clean(&gdata);
		errorf("while opening %s.", PREMAKE_LOG);
		exit(EXIT_FAILURE);
	}

	pmk_log("PMK version %s", PREMAKE_VERSION);
#ifdef DEBUG
	pmk_log(" [SUB #%s] [SNAP #%s]", PREMAKE_SUBVER_PMK, PREMAKE_SNAP);
#endif
	pmk_log("\n\n");

	pmk_log("Loaded %d predefinined variables.\n", nbpd);
	pmk_log("Loaded %d overridden switches.\n", ovrsw);
	pmk_log("Loaded %d overridden variables.\n", nbcd);
	pmk_log("Total : %d variables.\n\n", hash_nbkey(gdata.htab));

	pmk_log("Parsing '%s'\n", gdata.pmkfile);

	/* open pmk file */
	fp = fopen(gdata.pmkfile, "r");
	if (fp == NULL) {
		clean(&gdata);
		errorf("while opening %s.", gdata.pmkfile);
		exit(EXIT_FAILURE);
	}

	if (parse_pmkfile(fp, pdata, kw_pmkfile, nbkwpf) == false) {
		/* XXX too much things here */
		clean(&gdata);
		fflush(fp);
		fclose(fp);
		pmk_log_close();
		errorf("while opening %s.", gdata.pmkfile);
		exit(EXIT_FAILURE);
	}

	fflush(fp);
	fclose(fp);

	pmk_log("\nProcessing commands :\n");
	if (process_cmd(pdata, &gdata) == false) {
		/* an error occured while parsing */
		rval = 1;
	} else {
		pmk_log("\nProcess templates :\n");

		da = gdata.tlist;

		if (da == NULL) {
			errorf("no target given.");
			exit(EXIT_FAILURE);
		}
		for (i = 0 ; (i < da_usize(da)) && (rval == 0) ; i++) {
			pstr = da_idx(da, i);
			if (pstr != NULL) {
				abspath(gdata.srcdir, pstr, buf); /* XXX check ??? */
				if (process_template(buf, &gdata) == false) {
					/* failure while processing template */
					errorf("Failed to process '%s'.", buf);
					rval = 1;
				}
			}
		}

		if (gdata.ac_file != NULL) {
			pmk_log("\nProcess '%s' for autoconf compatibility.\n", gdata.ac_file);
			if (ac_parse_config(&gdata) == false) {
				/* failure while processing autoconf file */
				errorf("Failed to process autoconf config file.");
				rval = 1;
			}
		}

		pmk_log("\nEnd of log\n");
	}

	/* flush and close files */
	pmk_log_close();

	/* clean global data */
	clean(&gdata);

	return(rval);
}
