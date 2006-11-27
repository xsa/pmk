/* $Id$ */

/*
 * Copyright (c) 2003-2006 Damien Couderc
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

/*
 * Credits for patches :
 *	- Pierre-Yves Ritschard
 */


#include <sys/stat.h>
/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"

#include <errno.h>
#include <stdlib.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h" /* basename, dirname */
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h" /* strlcpy */
#include "compat/pmk_unistd.h"
#include "autoconf.h"
#include "common.h"
#include "func.h"
#include "hash_tools.h"
#include "lang.h"
#include "pathtools.h"
#include "pmk.h"
#include "tags.h"

/*#define PMK_DEBUG	1*/


/*************
 * variables *
 ***********************************************************************/

extern char		*optarg;
extern int		 optind;
extern prskw	 kw_pmkfile[];
extern size_t	 nbkwpf;

int				 cur_line = 0;


/******************
 * init functions *
 ***********************************************************************/

/******************
 * pmkdata_init() *
 ***********************************************************************
 DESCR
	init pmkdata structure

 IN
	NONE

 OUT
	pmkdata structure
 ***********************************************************************/

pmkdata *pmkdata_init(void) {
	pmkdata	*ppd;

	ppd = (pmkdata *) malloc(sizeof(pmkdata));
	if (ppd == NULL)
		return(NULL);

		/* initialise global data hash table */
	ppd->htab = hash_create_simple(MAX_DATA_KEY);
	if (ppd->htab == NULL) {
		errorf("cannot initialize hash table for data.");
		return(NULL);
	}

	ppd->labl = hash_create_simple(MAX_LABEL_KEY);
	if (ppd->labl == NULL) {
		hash_destroy(ppd->htab);
		errorf("cannot initialize hash table for labels.");
		return(NULL);
	}

	if (init_compiler_data(&(ppd->comp_data), LANG_NUMBER) == false) {
		hash_destroy(ppd->htab);
		hash_destroy(ppd->labl);
		errorf("cannot initialize compiler data structure.");
		return(NULL);
	}

	/* init compiler detection flag */
	ppd->sys_detect = false;

	/* init autoconf file */
	ppd->ac_file = NULL;

	/* init template list */
	ppd->tlist = NULL;

	/* init default language */
	ppd->lang = NULL;

	/* init on demand */
	ppd->cfgt = NULL;

	return(ppd);
}


/**************
 * init_var() *
 ***********************************************************************
 DESCR
	init variables

 IN
	pgd :	global data structure

 OUT
	NONE
 ***********************************************************************/

bool init_var(pmkdata *pgd) {
	char		 buf[TMP_BUF_LEN],
				*pstr;
	htable_t	*pht;

	pht = pgd->htab;

	pstr = MK_VAR_CFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_CXXFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_CPPFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_LDFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_CLDFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_CXXLDFLAGS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_LIBS;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif
	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	pstr = MK_VAR_DEBUG;
	if (get_make_var(pstr, buf, sizeof(buf)) == false) {
		buf[0] = CHAR_EOS; /* empty string */
	}
#ifdef PMK_DEBUG
debugf("%s = '%s'", pstr, buf);
#endif

	if (hash_update_dup(pht, pstr, buf) == false) {
		return(false);
	}

	/* autoconf shit ? *//* XXX YES !!! => to move ! */
	if (hash_update_dup(pht, "OBJEXT", "o") == false) {
		return(false);
	}


	pstr = hash_get(pht, PMKCONF_BIN_CC);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "CC", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_CXX);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "CXX", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_CPP);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "CPP", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_INSTALL);
	if (pstr != NULL) {
		/* append -c for compat with autoconf*/
		if (snprintf_b(buf, sizeof(buf), "%s -c", pstr) == false) {
			errorf(PMK_ERR_OVRFLOW);
			return(false);
		}

		if (hash_update_dup(pht, "INSTALL", buf) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_AR);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "AR", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_RANLIB);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "RANLIB", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_SH);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "SHELL", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_STRIP);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "STRIP", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_AWK);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "AWK", pstr) == false) {
			return(false);
		}
	}

	pstr = hash_get(pht, PMKCONF_BIN_EGREP);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "EGREP", pstr) == false) {
			return(false);
		}
	}

	/* shared lib support */
	if (hash_update_dup(pht, MK_VAR_SL_BUILD, "") == false) {
		return(false);
	}
	if (hash_update_dup(pht, MK_VAR_SL_CLEAN, "") == false) {
		return(false);
	}
	if (hash_update_dup(pht, MK_VAR_SL_INST, "") == false) {
		return(false);
	}
	if (hash_update_dup(pht, MK_VAR_SL_DEINST, "") == false) {
		return(false);
	}

	/* set absolute paths */
	if (hash_update_dup(pht, PMK_DIR_SRC_ROOT_ABS, pgd->srcdir) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_ROOT_ABS, pgd->srcdir);
#endif

	if (hash_update_dup(pht, PMK_DIR_BLD_ROOT_ABS, pgd->basedir) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_ROOT_ABS, pgd->basedir);
#endif

	return(true);
}


/**********************
 * template functions *
 ***********************************************************************/

/***********************
 * process_dyn_paths() *
 ***********************************************************************
 DESCR
	process dynamic paths

 IN
	pgd :		global data structure
	tmplpath :	file to process

 OUT
	NONE
 ***********************************************************************/

bool process_dyn_paths(pmkdata *pgd, char *tmplpath) {
	char		*srcdir,
				*basedir,
				 tpath[MAXPATHLEN];
	pmkdyn_t	*pdd;

	pdd = &(pgd->dyndata);

	srcdir = pgd->srcdir;
	basedir = pgd->basedir;

	/*
		get template path

		NOTE : we use strdup() to avoid problem with linux's dirname
	*/
	if (strlcpy_b(tpath, tmplpath, sizeof(tpath)) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* save template filename */
	if (strlcpy_b(pdd->tmpl_name, basename(tpath), sizeof(pdd->tmpl_name)) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* set absolute path of template */
	if (strlcpy_b(pdd->src_abs, dirname(tpath), sizeof(pdd->src_abs)) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* compute relative path */
	if (relpath(srcdir, pdd->src_abs, tpath) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* compute builddir_abs with relative path */
	if (abspath(basedir, tpath, pdd->bld_abs) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* compute relative path to builddir root */
	if (relpath(pdd->bld_abs, basedir, pdd->bld_root_rel) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* set buildir_rel to '.', useful ? */
	if (strlcpy_b(pdd->bld_rel, ".", sizeof(pdd->bld_rel)) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* compute and set relative path from basedir to srcdir */
	if (relpath(pdd->bld_abs, srcdir, pdd->src_root_rel) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	/* compute and set relative path from template to builddir */
	if (relpath(pdd->bld_abs, pdd->src_abs, pdd->src_rel) == false) {
		errorf(PMK_ERR_OVRFLOW);
		return(false);
	}

	return(true);
}


/*********************
 * process_dyn_var() *
 ***********************************************************************
 DESCR
	set dynamic path variables

 IN
	pgd : 		global data structure
	template :	file to process

 OUT
	NONE
 ***********************************************************************/

bool process_dyn_var(pmkdata *pgd) {
	htable_t	*pht;
	pmkdyn_t	*pdd;

	pht = pgd->htab;
	pdd = &(pgd->dyndata);

	if (hash_update_dup(pht, PMK_DIR_BLD_ROOT_REL, pdd->bld_root_rel) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_ROOT_REL, pdd->bld_root_rel);
#endif

	if (hash_update_dup(pht, PMK_DIR_BLD_ABS, pdd->bld_abs) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_ABS, pdd->bld_abs);
#endif

	if (hash_update_dup(pht, PMK_DIR_BLD_REL, pdd->bld_rel) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_BLD_REL, pdd->bld_rel);
#endif

	if (hash_update_dup(pht, PMK_DIR_SRC_ROOT_REL, pdd->src_root_rel) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_ROOT_REL, pdd->src_root_rel);
#endif

	if (hash_update_dup(pht, PMK_DIR_SRC_ABS, pdd->src_abs) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_ABS, pdd->src_abs);
#endif

	if (hash_update_dup(pht, PMK_DIR_SRC_REL, pdd->src_rel) == false) {
		return(false);
	}
#ifdef PMK_DEBUG
	debugf("%s = '%s'", PMK_DIR_SRC_REL, pdd->src_rel);
#endif

	return(true);
}


/**********************
 * process_template() *
 ***********************************************************************
 DESCR
	process the target file to replace tags

 IN
	target :	path of the target file
	pgd :		global data structure

 OUT
	boolean
 ***********************************************************************/

bool process_template(pmkdata *pgd, char *template) {
	FILE		*tfp,
				*gfp;
	bool		 ac_flag;
	char		*gfname,
				*pstr,
				 gfpath[MAXPATHLEN],
				 buf[TMP_BUF_LEN];
	htable_t	*pht;
	pmkdyn_t	*pdd;
	prseng_t	*ppe;

	pht = pgd->htab;
	pdd = &(pgd->dyndata);

	/* create path if it does not exists */
	if (makepath(pdd->bld_abs, S_IRWXU | S_IRWXG | S_IRWXO) == false) {
		errorf("cannot build template generated file path '%s'.", pdd->bld_abs);
		return(false);
	}

	/* generate full path for generated file */
	gfname = gen_from_tmpl(pdd->tmpl_name);
	if (abspath(pdd->bld_abs, gfname, gfpath) == false) {
		errorf("failed to build absolute path from '%s' and '%s'", pdd->bld_abs, gfname);
		return(false);
	}

	tfp = fopen(template, "r");
	if (tfp == NULL) {
		errorf("cannot open '%s' : %s.", template, strerror(errno));
		return(false);
	}

	gfp = fopen(gfpath, "w");
	if (gfp == NULL) {
		fclose(tfp); /* close already opened tfp before leaving */
		errorf("cannot open '%s' : %s.", gfpath, strerror(errno));
		return(false);
	}

	if (process_dyn_var(pgd) == false) {
		errorf("failed to process dynamic variables.");
		return(false);
	}

	/* check if autoconf dynamic variables are needed */
	if (pgd->ac_file == NULL) {
		ac_flag = false;
	} else {
		ac_flag = true;
		/* XXX should use directly path ? */
		if (ac_process_dyn_var(pgd, template) == false) {
			errorf("failed to process autoconf dynamic variables.");
			return(false);
		}
	}

	/* generate @configure_input@ tag */
	/* XXX handle in pmkscan templates ? */
	if (snprintf_b(buf, sizeof(buf), PMK_GENMSG, gfname, pdd->tmpl_name) == false) {
		return(false);
	}
	if (hash_update_dup(pht, "configure_input", buf) == false) {
		return(false);
	}

	/* init prseng with template */
	ppe = prseng_init(tfp, NULL);
	if (ppe == NULL) {
		errorf("parse engine init failed.");
		return(false);
	}

	/* while end of template is not reached */
	while (prseng_eof(ppe) == false) {
		if (prseng_test_char(ppe, PMK_TAG_CHAR) == true) {
			/* skip tag character */
			if (prseng_next_char(ppe) == false) {
				return(false);
			}

			/* get tag identifier */
			if (prseng_get_idtf(ppe, buf, sizeof(buf), PMK_TAG_IDTF_STR) == true) {
				/* if valid tag identifier */
				if (prseng_test_char(ppe, PMK_TAG_CHAR) == true) {
					/* skip end of tag character */
					if (prseng_next_char(ppe) == false) {
						return(false);
					}

					/* try to get tag content */
					pstr = (char *) hash_get(pht, buf);
					if (pstr != NULL) {
						/* put data */
						fprintf(gfp, "%s", pstr);
					} else {
						/* else no data, put back tag def */
						fprintf(gfp, "%c%s%c", PMK_TAG_CHAR, buf, PMK_TAG_CHAR);
					}
				} else {
					/* else not a valid tag identifier */
					fprintf(gfp, "%c%s%c", PMK_TAG_CHAR, buf, prseng_get_char(ppe));

					/* skip character */
					if (prseng_next_char(ppe) == false) {
						return(false);
					}
				}
			} else {
				/* else failed to get a tag identifier */
				fprintf(gfp, "%c%s", PMK_TAG_CHAR, buf);
			}
		} else {
			/* put character in generated file */
			fputc(prseng_get_char(ppe), gfp);

			/* next character */
			if (prseng_next_char(ppe) == false) {
				return(false);
			}
		}
	}

	fclose(gfp);
	fclose(tfp);

	pmk_log("Created '%s'.\n", gfpath);

	if (ac_flag == true) {
		/* clean autoconf dynamic variables */
		ac_clean_dyn_var(pht);
	}

	hash_delete(pht, "configure_input");

	return(true);
}


/*******************
 * template_main() *
 ***********************************************************************
 DESCR
	main procedure for template management

 IN
	pgd :		global data structure

 OUT
	boolean
 ***********************************************************************/

bool template_main(pmkdata *pgd) {
	char			*pstr,
					 buf[MAXPATHLEN];
	dynary			*da;
	unsigned int	 i;

	da = pgd->tlist;
	if (da == NULL) {
		errorf("no target given.");
		exit(EXIT_FAILURE);
	}

	/* process each template */
	for (i = 0 ; i < da_usize(da) ; i++) {
		pstr = da_idx(da, i);
		if (pstr != NULL) {
			if (abspath(pgd->srcdir, pstr, buf) == false) {
				errorf("failed to build absolute path of '%s'.", pstr);
				return(false);
			}

			if (process_dyn_paths(pgd, buf) == false) {
				errorf("failed to process dynamic paths for '%s'.", buf);
				return(false);
			}

			if (process_template(pgd, buf) == false) {
				/* failure while processing template */
				errorf("failed to process template '%s'.", buf);
				return(false);
			}
		}
	}

	/* process eventual autoconf style config file */
	if (pgd->ac_file != NULL) {
		pmk_log("\nProcess '%s' for autoconf compatibility.\n", pgd->ac_file);
		if (ac_parse_config(pgd) == false) {
			/* failure while processing autoconf file */
			errorf("failed to process autoconf config file '%s'.", pgd->ac_file);
			return(false);
		}
	}

	return(true);
}


/***************************
 * miscellaneous functions *
 ***********************************************************************/

/*****************
 * process_cmd() *
 ***********************************************************************
 DESCR
	process the parsed command

 IN
	pdata : parsed data
	pgd : global data structure

 OUT
	boolean
 ***********************************************************************/

bool process_cmd(prsdata *pdata, pmkdata *pgd) {
	return(process_node(pdata->tree, pgd));
}


/*******************
 * parse_cmdline() *
 ***********************************************************************
 DESCR
	process command line values

 IN
	val : array of defines
	nbval : size of the array
	pgd : global data structure

 OUT
	boolean (true on success)
 ***********************************************************************/

bool parse_cmdline(char **val, int nbval, pmkdata *pgd) {
	bool		 rval = true;
	char		*pstr;
	htable_t	*ht;
	int			 i;
	prsopt		 opt;

	/* don't init pscell */
	ht = pgd->htab;

	for (i = 0 ; (i < nbval) && (rval == true) ; i++) {
		/* parse option */
		rval = parse_clopt(val[i], &opt, PRS_PMKCONF_SEP);
		if (rval == true) {
			pstr = po_get_str(opt.value);
			if (pstr == NULL) {
				errorf("unable to get value for %s.", opt.key);
				return(false);
			}

			/* no need to strdup */
			if (hash_update(ht, opt.key, pstr) == false) {
				errorf("%s.", PRS_ERR_HASH);
				rval = false;
			}
		}
	}

	return(rval);
}


/***********
 * clean() *
 ***********************************************************************
 DESCR
	clean global data

 IN
	pgd : global data structure

 OUT
	NONE
 ***********************************************************************/

void clean(pmkdata *pgd) {
	if (pgd->htab != NULL) {
		hash_destroy(pgd->htab);
	}

	if (pgd->labl != NULL) {
		hash_destroy(pgd->labl);
	}

	clean_compiler_data(&(pgd->comp_data));

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


/*********************
 * set_switch_list() *
 ***********************************************************************
 DESCR
	set or unset a switch

 IN
	swlst :	list of switches to process
	state :	switch state to set (true or false)

 OUT
	boolean
 ***********************************************************************/

bool set_switch_list(pmkdata *pgd, char *swlst, bool state, int *ovrsw) {
	char	*pstr,
			*ststr;
	dynary	*da;

	if (state == true) {
		ststr = BOOL_STRING_TRUE;
	} else {
		ststr = BOOL_STRING_FALSE;
	}

	da = str_to_dynary(swlst, CHAR_LIST_SEPARATOR);
	if (da != NULL) {
		do {
			pstr = da_pop(da);
			if (pstr != NULL) {
				if (hash_update_dup(pgd->labl, pstr, ststr) == false)
					return(false);
				*ovrsw++; /* increment number of overriden switches */
				free(pstr);
			}
		} while (pstr != NULL);
		da_destroy(da);
	}

	return(true);
}


/***********
 * usage() *
 ***********************************************************************
 DESCR
	usage

 IN
	NONE

 OUT
	NONE
 ***********************************************************************/

void usage(void) {
	fprintf(stderr, "usage: pmk [-vh] [-e list] [-d list] [-b path]\n");
	fprintf(stderr, "\t[-f pmkfile] [-o ovrfile] [options]\n");
}


/**********
 * main() *
 ***********************************************************************
 DESCR
	main loop
 ***********************************************************************/

int main(int argc, char *argv[]) {
	FILE			*fp;
	bool			 go_exit = false,
					 pmkfile_set = false,
					 ovrfile_set = false,
					 basedir_set = false,
					 buildlog = false;
	char			*pstr,
					*enable_sw = NULL,
					*disable_sw = NULL,
					 buf[MAXPATHLEN];
	int				 rval = 0,
					 nbpd,
					 nbcd,
					 ovrsw = 0,
					 chr;
	pmkdata			*pgd;
	prsdata			*pdata;

	/* get current path */
	if (getcwd(buf, sizeof(buf)) == NULL) {
		errorf("unable to get current directory.");
		exit(EXIT_FAILURE);
	}

	/* initialise global data hash table */
	pgd = pmkdata_init();
	if (pgd == NULL) {
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

					if (uabspath(buf, optarg, pgd->basedir) == false) {
						errorf("cannot use basedir argument.");
						exit(EXIT_FAILURE);
					}
					basedir_set = true;
					break;

				case 'e' :
					/* enable switch(es) */
					enable_sw = strdup(optarg);
					if (enable_sw == NULL) {
						errorf(ERRMSG_MEM);
						exit(EXIT_FAILURE);
					}
					break;

				case 'd' :
					/* disable switch(es) */
					disable_sw = strdup(optarg);
					if (disable_sw == NULL) {
						errorf(ERRMSG_MEM);
						exit(EXIT_FAILURE);
					}
					break;

				case 'f' :
					/* pmk file path */
					/* XXX check if path in optarg is valid */
					if (uabspath(buf, optarg, pgd->pmkfile) == false) {
						errorf("cannot use file argument.");
						exit(EXIT_FAILURE);
					}

					/*
						path of pmkfile is also the srcdir base

						NOTE : we use strdup to avoid
						problem with linux's dirname
					*/
					pstr = strdup(pgd->pmkfile);
					if (pstr == NULL) {
						free(pstr);
						errorf(ERRMSG_MEM);
						exit(EXIT_FAILURE);
					}
					if (strlcpy_b(pgd->srcdir, dirname(pstr),
							sizeof(pgd->srcdir)) == false) {
						free(pstr);
						errorf(PMK_ERR_OVRFLOW);
						exit(EXIT_FAILURE);
					}
					free(pstr);

					pmkfile_set = true;
					break;

				case 'l' :
					/* enable log of test build */
					buildlog = true;
					break;

				case 'o' :
					/* file path to override pmk.conf */
					/* XXX check if path in optarg is valid */
					if (uabspath(buf, optarg, pgd->ovrfile) == false) {
						errorf("cannot use file argument.");
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
		if (strlcpy_b(pgd->basedir, buf,
					sizeof(pgd->basedir)) == false) {
			errorf("failed to set 'basedir'.");
			exit(EXIT_FAILURE);
		}
	}

	/* set pmkfile and srcdir if needed */
	if (pmkfile_set == false) {
		if (strlcpy_b(pgd->srcdir, buf,
					sizeof(pgd->srcdir)) == false) {
			errorf("failed to set 'srcdir'.");
			exit(EXIT_FAILURE);
		}

		abspath(pgd->srcdir, PREMAKE_FILENAME, pgd->pmkfile); /* should not fail */
	}

	if (buildlog == true) {
		/* set build log name */
		if (strlcpy_b(pgd->buildlog, PMK_BUILD_LOG,
					sizeof(pgd->buildlog)) == false) {
			errorf(PMK_ERR_BLDLOG);
			exit(EXIT_FAILURE);
		}

		/* remove previous build log */
		unlink(pgd->buildlog);
	} else {
		/* redirect build output to /dev/null */
		if (strlcpy_b(pgd->buildlog, "/dev/null",
					sizeof(pgd->buildlog)) == false) {
			errorf(PMK_ERR_BLDLOG);
			exit(EXIT_FAILURE);
		}
	}

	/* init prsdata structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot intialize prsdata.");
		exit(EXIT_FAILURE);
	}

	fp = fopen(PREMAKE_CONFIG_PATH, "r");
	if (fp != NULL) {
		if (parse_pmkconf(fp, pgd->htab, PRS_PMKCONF_SEP, process_opt) == false) {
			/* parsing failed */
			clean(pgd);
			fclose(fp);
			errorf(ERRMSG_PARSE, PREMAKE_CONFIG_PATH);
			exit(EXIT_FAILURE);
		}
		fclose(fp);
		nbpd = hash_nbkey(pgd->htab);
	} else {
		/* configuration file not found */
		clean(pgd);
		errorf(ERRMSG_PARSE " Run pmksetup(8).", PREMAKE_CONFIG_PATH);
		exit(EXIT_FAILURE);
	}

	/* initialize some variables */
	if (init_var(pgd) == false) {
		/* hash error on variable initialization */
		clean(pgd);
		errorf("failed to do variable init.", PREMAKE_CONFIG_PATH);
		exit(EXIT_FAILURE);
	}

	if (enable_sw != NULL) {
		/* command line switches to enable */
		if (set_switch_list(pgd, enable_sw, true, &ovrsw) == false) {
			errorf("failed to set enabled switches.");
			exit(EXIT_FAILURE);
		}
	}

	if (disable_sw != NULL) {
		/* command line switches to disable */
		if (set_switch_list(pgd, disable_sw, false, &ovrsw) == false) {
			errorf("failed to set disabled switches.");
			exit(EXIT_FAILURE);
		}
	}

	if (ovrfile_set == true) {
		/* read override file */
		fp = fopen(pgd->ovrfile, "r");
		if (fp != NULL) {
			if (parse_pmkconf(fp, pgd->htab, PRS_PMKCONF_SEP, process_opt) == false) {
				/* parsing failed */
				clean(pgd);
				fclose(fp);
				errorf(ERRMSG_PARSE, pgd->ovrfile);
				exit(EXIT_FAILURE);
			}
			fclose(fp);
			nbpd = hash_nbkey(pgd->htab);
		} else {
			/* configuration file not found */
			clean(pgd);
			errorf("cannot parse '%s' : %s.",
				pgd->ovrfile, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (argc != 0) {
		/* parse optional arguments that override pmk.conf and override file */
		if (parse_cmdline(argv, argc, pgd) == true) {
			nbcd = argc;
		} else {
			clean(pgd);
			errorf("incorrect optional arguments.");
			exit(EXIT_FAILURE);
		}
	} else {
		nbcd = 0;
	}

	/* open log file */
	if (pmk_log_open(PMK_LOG) == false) {
		clean(pgd);
		errorf("while opening '%s'.", PMK_LOG);
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
	pmk_log("Total : %d variables.\n\n", hash_nbkey(pgd->htab));

	pmk_log("Parsing '%s'\n", pgd->pmkfile);

	/* open pmk file */
	fp = fopen(pgd->pmkfile, "r");
	if (fp == NULL) {
		clean(pgd);
		errorf("failed to open '%s' : %s.", pgd->pmkfile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (parse_pmkfile(fp, pdata, kw_pmkfile, nbkwpf) == true) {
		pmk_log("\nProcessing commands :\n");
		if (process_cmd(pdata, pgd) == false) {
			/* an error occured while parsing */
			rval = 1;
		} else {
			pmk_log("\nProcess templates :\n");

			if (template_main(pgd) == false) {
				/* an error occured while processing templates */
				rval = 1;
			}

			pmk_log("\nEnd of log\n");
		}
	} else {
		errorf("failed to open '%s'.", pgd->pmkfile);
		rval = 1;
	}

	fclose(fp);

	/* flush and close files */
	pmk_log_close();

	/* clean global data */
	clean(pgd);

	return(rval);
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
