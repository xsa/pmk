/* $Id$ */

/*
 * Credits for patches :
 *	- Pierre-Yves Ritschard
 */

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


#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compat/pmk_libgen.h" /* basename, dirname */
#include "compat/pmk_string.h" /* strlcpy */
#include "autoconf.h"
#include "common.h"
#include "func.h"
#include "pathtools.h"
#include "pmk.h"
#include "parse.h"


extern char	*optarg;
extern int	 optind;
extern cmdkw	 functab[];
extern int	 nbfunc;

int		 cur_line = 0;

/* keyword data */
htable		*keyhash;


/*
	read configuration file

	ht : hash table that will contain data
	filename : file to read

	return : boolean
*/

bool read_conf(htable *ht, char *filename) {
	FILE	*fp;
	cfg_opt	 co;
	char	 buf[MAX_LINE_LEN];
	int	 ln = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		errorf("cannot open %s.", filename);
		return(false);
	}

	while (get_line(fp, buf, sizeof(buf)) == true) {
		switch (buf[0]) {
			case CHAR_COMMENT :
				/* ignore comment */
				break;

			case CHAR_EOS :
				/* ignore empty line */
				break;

			default :
				if (parse_conf_line(buf, ln, &co) == 0) {
					/* parse ok */
					if (hash_add(ht, co.key, po_mk_str(co.val)) == HASH_ADD_FAIL) {
						errorf("hash failure.");
						return(false);
					}
				} else {
					/* incorrect line */
					return(false);
				}
				break;
		}
		ln++; /* increment line number */
	}

	if (feof(fp) == 0) {
		/* error occuered before EOF */
		errorf_line(PREMAKE_CONFIG_PATH, cur_line, "end of file not reached.");
		return(false);
	}

	fclose(fp);

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
	bool	 replace,
		 ac_flag;
	char	*plb,
		*pbf,
		*ptbf,
		*ptmp,
		 lbuf[MAXPATHLEN],
		 buf[MAXPATHLEN],
		 tbuf[MAXPATHLEN],
		 fpath[MAXPATHLEN];
	htable	*ht;

	ht = pgd->htab;

	/* get the file name */
	strlcpy(lbuf, basename(template), MAXPATHLEN); /* XXX check ? */

	/* remove the last suffix */
	ptmp = strrchr(lbuf, '.');
	if (ptmp != NULL) {
		*ptmp = CHAR_EOS;
	} else {
		errorf("error while creating name of template");
		return(false);
	}

	/* get relative path from srcdir */
	/* NOTE : we use strdup to avoid problem with linux's dirname */
	ptmp = strdup(template);
	relpath(pgd->srcdir, dirname(ptmp), buf); /* XXX check ? */
	free(ptmp);
	/* apply to basedir */
	abspath(pgd->basedir, buf, tbuf); /* XXX check ? */

	/* XXX TODO should create directories of the path ? */
	
	/* append filename */
	abspath(tbuf, lbuf, fpath);

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

	if (pgd->ac_file == NULL) {
		ac_flag = false;
	} else {
		ac_flag = true;
		ac_process_dyn_var(ht, pgd, template); /* XXX should use directly path */
	}

	while (fgets(lbuf, sizeof(lbuf), tfd) != NULL) {
		plb = lbuf;
		pbf = buf;
		ptbf = tbuf;
		replace = false;
		while (*plb != CHAR_EOS) {
			if (replace == false) {
				if (*plb == PMK_TAG_CHAR) {
					/* found begining of tag */
					replace = true;
				} else {
					/* copy normal text */
					*pbf = *plb;
					pbf++;
				}
			} else {
				if (*plb == PMK_TAG_CHAR) {
					/* tag identified */
					replace = false;
					*ptbf = CHAR_EOS;

					ptmp = (char *)po_get_data(hash_get(ht, tbuf));
					if (ptmp == NULL) {
						/* not a valid tag, put it back */
						*pbf = PMK_TAG_CHAR;
						pbf++;

						ptmp = tbuf;
						while (*ptmp != CHAR_EOS) {
							*pbf = *ptmp;
							pbf++;
							ptmp++;
						}
						*pbf = PMK_TAG_CHAR;
						pbf++;
					} else {
						/* replace with value */
						while (*ptmp != CHAR_EOS) {
							*pbf = *ptmp;
							pbf++;
							ptmp++;
						}
					}
					ptbf = tbuf;
				} else {
					/* continue getting tag name */
					*ptbf = *plb;
					ptbf++;
				}
			}
			plb++;
		}
		if (replace == true) {
			/* not a tag, copy tbuf in buf */
			*pbf = PMK_TAG_CHAR;
			pbf++;
			*ptbf = CHAR_EOS;
			ptmp = tbuf;
			while (*ptmp != CHAR_EOS) {
				*pbf = *ptmp;
				pbf++;
				ptmp++;
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
		ac_clean_dyn_var(ht);
	}
	
	return(true);
}

/*
	check if the given string is a valid command

	cmdname : command to check
	pgd : global data structure (for pmkfile name)

	return : boolean (true is command is valid)
*/

bool check_cmd(char *cmdname, pmkdata *pgd) {
	if ((char *)po_get_data(hash_get(keyhash, cmdname)) == NULL) {
		errorf("unknown command %s.", cmdname);
		return(false);
	} else {
		return(true);
	}
}

/*
	process the parsed command

	pdata : parsed data
	pgd : global data structure

	return : boolean
*/

bool process_cmd(prsdata *pdata, pmkdata *pgd) {
	char	*aidx;
	int	 idx;
	pmkcmd	 cmd;
	prscell	*pcell;

	/* init pcell with the first cell of pdata */
	pcell = pdata->first;

	while (pcell != NULL) {
		/* XXX */
		if (check_cmd(pcell->name, pgd) == false)
			return(false);

		cmd.name = pcell->name;
		cmd.label = pcell->label;

		aidx = (char *)po_get_data(hash_get(keyhash, cmd.name));
		if (aidx == NULL)
			return(false);

		/* getting index of function in functab */
		idx = atoi(aidx);
		/* launching cmd function */
		if ((*functab[idx].fnp)(&cmd, pcell->ht, pgd) == false)
			return(false);

		pcell = pcell->next;
	}

	return(true);
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

	/* don't init pscell */
	ht = pgd->htab;

	for (i = 0 ; (i < nbval) && (rval == true) ; i++) {
		/* parse option */
		rval = parse_opt(val[i], ht);
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

	if (pgd->tlist != NULL) {
		da_destroy(pgd->tlist);
	}
	if (pgd->ac_file != NULL) {
		free(pgd->ac_file);
	}
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmk [-vh] [-b path] [-f pmkfile] [-o overridefile] [options]\n");
}

/*
	main
*/

int main(int argc, char *argv[]) {
	FILE	*fd;
	bool	 go_exit = false,
		 pmkfile_set = false,
		 ovrfile_set = false,
		 basedir_set = false;
	char	*pstr,
		 buf[MAXPATHLEN],
		 idxstr[4]; /* max 999 cmds, should be enough :) */
	dynary	*da;
	int	 rval = 0,
		 nbpd,
		 nbcd,
		 i,
		 chr;
	pmkdata	 gdata;
	prsdata	*pdata;

	/* get current path */
	getcwd(buf, sizeof(buf));

	while (go_exit == false) {
		chr = getopt(argc, argv, "b:f:ho:v");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'b' :
					/* XXX check if path is valid */

					if (uabspath(buf, optarg, gdata.basedir) == false) {
						errorf("Cannot use basedir argument");
						exit(1);
					}
					basedir_set = true;
					break;

				case 'f' :
					/* pmk file path */
					/* XXX check if path is valid */
					if (uabspath(buf, optarg, gdata.pmkfile) == false) {
						errorf("Cannot use file argument");
						exit(1);
					}

					/* path of pmkfile is also the srcdir base */
					/* NOTE : we use strdup to avoid problem with linux's dirname */
					pstr = strdup(gdata.pmkfile); 
					strlcpy(gdata.srcdir, dirname(pstr), sizeof(gdata.srcdir)); /* XXX check ??? */
					free(pstr);

					pmkfile_set = true;
					break;

				case 'o' :
					/* file path to override pmk.conf */
					/* XXX check if path is valid */
					if (uabspath(buf, optarg, gdata.ovrfile) == false) {
						errorf("Cannot use file argument");
						exit(1);
					}

					ovrfile_set = true;
					break;

				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(0);
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					exit(1);
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

	keyhash = hash_init(nbfunc);
	if (keyhash != NULL) {
		/* fill keywords hash */
		for(i = 0 ; i < nbfunc ; i++) {
			snprintf(idxstr, 4, "%d", i);
			if (hash_add(keyhash, functab[i].kw, po_mk_str(idxstr)) == HASH_ADD_FAIL) {
				errorf("hash failure");
				exit(1);
			}
		}
	} else {
		errorf("cannot initialize keyword table");
		exit(1);
	}

	/* initialise global data hash table */
	gdata.htab = hash_init(MAX_DATA_KEY);
	if (gdata.htab == NULL) {
		clean(&gdata);
		errorf("cannot initialize hash table for data.");
		exit(1);
	} else {
		/* initialize some variables */
		hash_add(gdata.htab, "CFLAGS", po_mk_str("")); /* XXX check ? */
		hash_add(gdata.htab, "CPPFLAGS", po_mk_str("")); /* XXX check ? */
		hash_add(gdata.htab, "LIBS", po_mk_str("")); /* XXX check ? */
	}

	gdata.labl = hash_init(MAX_LABEL_KEY);
	if (gdata.labl == NULL) {
		clean(&gdata);
		errorf("cannot initialize hash table for labels.");
		exit(1);
	}

	/* init prsdata structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot intialize prsdata.");
		exit(1);
	}

	if (read_conf(gdata.htab, PREMAKE_CONFIG_PATH) == true) {
		nbpd = hash_nbkey(gdata.htab);
	} else {
		/* configuration file not found */
		clean(&gdata);
		errorf("cannot open '%s', run pmksetup", PREMAKE_CONFIG_PATH);
		exit(1);
	}

	if (ovrfile_set == true) {
		/* read override file */
		if (read_conf(gdata.htab, gdata.ovrfile) == true) {
			nbpd = hash_nbkey(gdata.htab);
		} else {
			/* configuration file not found */
			clean(&gdata);
			errorf("cannot open '%s', run pmksetup");
			exit(1);
		}
	}

	if (argc != 0) {
		/* parse optional arguments that override pmk.conf and override file */
		if (parse_cmdline(argv, argc, &gdata) == true) {
			nbcd = argc;
		} else {
			clean(&gdata);
			errorf("incorrect optional arguments");
			exit(1);
		}
	} else {
		nbcd = 0;
	}

	/* open log file */
	if (pmk_log_open(PREMAKE_LOG) == false) {
		clean(&gdata);
		errorf("while opening %s.", PREMAKE_LOG);
		exit(1);
	}

	pmk_log("PMK version %s", PREMAKE_VERSION);
#ifdef DEBUG
	pmk_log(" [SUB #%s] [SNAP #%s]", PREMAKE_SUBVER_PMK, PREMAKE_SNAP);
#endif
	pmk_log("\n\n");


	/* print number of hashed command */
	pmk_log("Hashed %d pmk keywords.\n", keyhash->count);

	pmk_log("Loaded %d predefinined variables.\n", nbpd);
	pmk_log("Loaded %d overrided variables.\n", nbcd);
	pmk_log("Total : %d variables.\n\n", hash_nbkey(gdata.htab));

	pmk_log("Parsing '%s'\n", gdata.pmkfile);

	/* open pmk file */
	fd = fopen(gdata.pmkfile, "r");
	if (fd == NULL) {
		clean(&gdata);
		errorf("while opening %s.", gdata.pmkfile);
		exit(1);
	}

	if (parse(fd, pdata) == false) {
		/* XXX too much things here */
		clean(&gdata);
		fflush(fd);
		fclose(fd);
		pmk_log_close();
		errorf("while opening %s.", gdata.pmkfile);
		exit(1);
	}

	fflush(fd);
	fclose(fd);

	pmk_log("Processing commands :\n");
	if (process_cmd(pdata, &gdata) == false) {
		/* an error occured while parsing */
		rval = 1;
	} else {
		pmk_log("\nProcess templates :\n");

		da = gdata.tlist;

		if (da == NULL) {
			errorf("no target given.");
			exit(1);
		}
		for (i = 0 ; (i < da_usize(da)) && (rval == 0) ; i++) {
			pstr = da_idx(da, i);
			if (pstr != NULL) {
				abspath(gdata.srcdir, pstr, buf); /* XXX check ??? */
				if (process_template(buf, &gdata) == false) {
					/* failure while processing template */
					rval = 1;
				}
			}
		}

		if (gdata.ac_file != NULL) {
			pmk_log("\nProcess '%s' for autoconf compatibility.\n", gdata.ac_file);
			ac_parse_config(gdata.htab, gdata.ac_file);
		}

		pmk_log("\nEnd of log\n");
	}

	/* flush and close files */
	pmk_log_close();

	/* clear cmd hash */
	if (keyhash != NULL) {
		hash_destroy(keyhash);
	}

	/* clean global data */
	clean(&gdata);

	return(rval);
}
