/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
 * Copyright (c) 2004 Xavier Santolaria
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


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "cfgtool.h"
#include "common.h"
#include "dynarray.h"
#include "parse.h"
#include "pkgconfig.h"
#include "pmkpc.h"
#include "premake.h"


/*#define DEBUG_PMKPC	1*/

extern char	*optarg;
extern int	 optind;

pcopt	pcoptions[] = {
		{"version",			false,	PMKPC_OPT_VERSION,		NULL},
		{"atleast-pkgconfig-version",	false,	PMKPC_OPT_ATLPKGVERS,		PC_USAGE_VERSION},
		{"exists",			false,	PMKPC_OPT_EXISTS,		NULL},
		{"list-all",			false,	PMKPC_OPT_LISTALL,		NULL},
		{"uninstalled",			false,	PMKPC_OPT_UNINST,		NULL},
		{"debug",			false,	PMKPC_OPT_DEBUG,		NULL},
		{"help",			false,	PMKPC_OPT_HELP,			NULL},
		{"usage",			false,	PMKPC_OPT_USAGE,		NULL},
		{"modversion",			false,	PMKPC_OPT_MODVERS,		NULL},
		{"atleast-version",		false,	PMKPC_OPT_ATLVERS,		PC_USAGE_VERSION},
		{"exact-version",		false,	PMKPC_OPT_EXTVERS,		PC_USAGE_VERSION},
		{"max-version",			false,	PMKPC_OPT_MAXVERS,		PC_USAGE_VERSION},
		{"cflags",			false,	PMKPC_OPT_CFLAGS,		NULL},
		{"cflags-only-I",		false,	PMKPC_OPT_CFLAGS_ONLY_PATH,	NULL},
		{"cflags-only-other",		false,	PMKPC_OPT_CFLAGS_ONLY_OTHER,	NULL},
		{"libs",			false,	PMKPC_OPT_LIBS, 		NULL},
		{"libs_only_l",			false,	PMKPC_OPT_LIBS_ONLY_LIB,	NULL},
		{"libs_only_L",			false,	PMKPC_OPT_LIBS_ONLY_PATH,	NULL},
		{"libs_only_other",		false,	PMKPC_OPT_LIBS_ONLY_OTHER,	NULL},
		{"variable",			false,	PMKPC_OPT_VAR,			PC_USAGE_VARNAME},
		{"define-variable",		false,	PMKPC_OPT_VAR_DEF,		PC_USAGE_VARVAL},
		{"print-errors",		false,	PMKPC_OPT_VAR_PRNT,		NULL},
		{"silence-errors",		false,	PMKPC_OPT_VAR_SILC,		NULL},
		{"errors-to-stdout",		false,	PMKPC_OPT_VAR_STDO,		NULL},
};
size_t	nbpcopt = sizeof(pcoptions) / sizeof(pcopt);

/*
	XXX
*/

optcell *optcell_init(void) {
	optcell	*poc;

	poc = (optcell *) malloc(sizeof(optcell));
	if (poc == NULL) {
		return(NULL);
	}

	poc->idx = 1;
	poc->id = PMKPC_OPT_UNKNOWN;
	poc->arg = NULL;

	return(poc);
}

/*
	XXX
*/

void optcell_destroy(optcell *poc) {
	free(poc); /* no more to free */
}

/*
	XXX
*/

bool pcgetopt(int ac, char *av[], optcell *poc) {
	char		*opt;
	size_t		 l;
	unsigned int	 i,
			 idx;

	idx = poc->idx;

	if (idx >= ac) {
#ifdef DEBUG_PMKPC
debugf("{pcgetopt} ac = %d", ac);
#endif
		/* no more args */
		return(false);
	}

	opt = av[idx];
	
	if ((opt[0] != '-') || (opt[1] != '-')) {
#ifdef DEBUG_PMKPC
debugf("{pcgetopt} full opt = '%s'", opt);
#endif
		/* not an option */
		poc->arg = opt;
		return(false);
	} else {
		/* skip leading "--" */
		opt = opt + 2;
#ifdef DEBUG_PMKPC
debugf("{pcgetopt} opt = '%s'", opt);
#endif
		l = strlen(opt);
		idx++;
#ifdef DEBUG_PMKPC
debugf("{pcgetopt} idx = %d", idx);
#endif
	}

	for (i = 0 ; i < nbpcopt ; i++) {
		if (strncmp(pcoptions[i].name, opt, l) == 0) {
			/* known option */
			poc->id = pcoptions[i].id;
			poc->err = pcoptions[i].name;

			if (pcoptions[i].arg == true) {
				/* got argument */

				/* XXX check idx >= ac */
				poc->arg = av[idx];
				idx++;
			}

			poc->idx = idx;
			return(true);
		}
	}

	poc->id = PMKPC_OPT_UNKNOWN;
	poc->err = opt;

	return(true);
}

/*
	clean
*/

void clean(pcdata *p_cd) {
	/* clean data */
	if (p_cd->pda != NULL) {
		da_destroy(p_cd->pda);
	}
	if (p_cd->pht != NULL) {
		hash_destroy(p_cd->pht);
	}
	if (p_cd->ppd != NULL) {
		pkgdata_destroy(p_cd->ppd);
	}
}

/*
 	pmkpc(1) usage
*/

void usage(void) {
	int	i;
	char	option[MAXPATHLEN];
	size_t	alen,	/* alignement length */
		olen,	/* option length */
		cursor;

	alen = strlen(PMKPC_USAGE_ALIGN);

	fprintf(stderr, "%s", PMKPC_USAGE_STR);
	cursor = strlen(PMKPC_USAGE_STR);

	for (i = 0; i < nbpcopt; i++) {
		strlcpy(option, PMKPC_USAGE_OPEN_OPT, sizeof(option));
		strlcat(option, pcoptions[i].name, sizeof(option));

		if (pcoptions[i].usagearg != NULL) {
			strlcat(option, "=", sizeof(option));
			strlcat(option, pcoptions[i].usagearg, sizeof(option)); 
		}

		strlcat(option, PMKPC_USAGE_CLOSE_OPT, sizeof(option));
		olen = strlen(option);
		cursor += olen;

		if (cursor > 79) {
			fprintf(stderr, "\n%s", PMKPC_USAGE_ALIGN);
			cursor = olen + alen;
		}

		fprintf(stderr, "%s", option);
	}
	fprintf(stderr, "\n");
}

/*
	XXX
*/

int main(int argc, char *argv[]) {
	FILE		*fp;
	bool		 opt_version = false,
			 opt_modvers = false,
			 opt_exists = false,
			 opt_cflags = false,
			 opt_libs = false;
	cfgtcell	*pcc = NULL;
	cfgtdata	*pcd = NULL;
	char		*mod,
			*bpath,
			*opt,
			*pc_path,
			 pc_cmd[MAXPATHLEN],
			 pc_buf[MAXPATHLEN],
			 buf[TMP_BUF_LEN];
	optcell		*poc;
	pcdata		 gdata;
	pkgcell		*ppc;
	unsigned int	 i;

	poc = optcell_init();
	if (poc == NULL) {
		errorf("cannot init optcell.");
		exit(EXIT_FAILURE);
	}

	gdata.pda = da_init();
	if (gdata.pda == NULL) {
		optcell_destroy(poc);
		errorf("cannot init dynary.");
		exit(EXIT_FAILURE);
	}

	gdata.ppd = pkgdata_init();
	if (gdata.ppd == NULL) {
		optcell_destroy(poc);
		clean(&gdata);
		errorf("cannot init pkgdata.");
		exit(EXIT_FAILURE);
	}

	/* initialise global data hash table */
	gdata.pht = hash_init(MAX_DATA_KEY);
	if (gdata.pht == NULL) {
		clean(&gdata);
		errorf("cannot initialize hash table for data.");
		exit(EXIT_FAILURE);
	}

	while (poc->idx != argc) {
		if (pcgetopt(argc, argv, poc) == true) {
#ifdef DEBUG_PMKPC
debugf("{main} id = %d", poc->id);
#endif
			/* found an option */
			switch (poc->id) {
				case PMKPC_OPT_VERSION :
					opt_version = true;
					break;

				case PMKPC_OPT_MODVERS :
					opt_modvers = true;
					break;

				case PMKPC_OPT_CFLAGS :
					opt_cflags = true;
					break;

				case PMKPC_OPT_LIBS :
					opt_libs = true;
					break;

				case PMKPC_OPT_EXISTS :
					opt_exists = true;
					break;

				case PMKPC_OPT_HELP :
					fprintf(stderr, "For detailed help "
						"please see the pmkpc(1) "
						"man page.\n");
					exit(EXIT_FAILURE);

				case PMKPC_OPT_USAGE :
					usage();
					exit(EXIT_FAILURE);

				case PMKPC_OPT_ATLPKGVERS :
				case PMKPC_OPT_LISTALL :
				case PMKPC_OPT_UNINST :
				case PMKPC_OPT_DEBUG :
				case PMKPC_OPT_ATLVERS :
				case PMKPC_OPT_EXTVERS :
				case PMKPC_OPT_MAXVERS :
				case PMKPC_OPT_CFLAGS_ONLY_PATH	:
				case PMKPC_OPT_CFLAGS_ONLY_OTHER :
				case PMKPC_OPT_LIBS_ONLY_LIB :
				case PMKPC_OPT_LIBS_ONLY_PATH :
				case PMKPC_OPT_LIBS_ONLY_OTHER :
				case PMKPC_OPT_VAR :
				case PMKPC_OPT_VAR_DEF :
				case PMKPC_OPT_VAR_PRNT :
				case PMKPC_OPT_VAR_SILC :
				case PMKPC_OPT_VAR_STDO :
					optcell_destroy(poc);
					clean(&gdata);
					errorf("option '--%s' is not yet implemented.", poc->err);
					exit(EXIT_FAILURE);
					break;

				case PMKPC_OPT_UNKNOWN :
				default :
					optcell_destroy(poc);
					clean(&gdata);
					errorf("unknown option '--%s'.", poc->err);
					exit(EXIT_FAILURE);
					break;
			}
		} else {
			mod = argv[poc->idx];

			if (mod[0] == '-') {
				switch (mod[1]) {
					case '?' :
						usage();
						exit(EXIT_FAILURE);
						break;
					default :
						errorf("unknow option -%c", mod[1]);
						exit(EXIT_SUCCESS);
						break;
				}
			} else {
				/* add new module */
				da_push(gdata.pda, strdup(mod)); /* XXX check */
#ifdef DEBUG_PMKPC
debugf("{main} new mod = '%s'", mod);
#endif
				poc->idx++;
			}
		}
	}

	optcell_destroy(poc);

	if (opt_version == true) {
		printf("%s\n", PMKPC_COMPAT_VERSION);
		clean(&gdata);
		exit(EXIT_SUCCESS);
	}

	fp = fopen(PREMAKE_CONFIG_PATH, "r");
	if (fp != NULL) {
		if (parse_pmkconf(fp, gdata.pht, PRS_PMKCONF_SEP, process_opt) == false) {
			/* parsing failed */
			clean(&gdata);
			errorf("failed to parse '%s'.", PREMAKE_CONFIG_PATH);
			exit(EXIT_FAILURE);
#ifdef DEBUG_PMKPC
		} else {
debugf("{main} parsed '%s'", PREMAKE_CONFIG_PATH);
#endif
		}
		fclose(fp);
	} else {
		clean(&gdata);
		errorf("cannot open '%s' : %s.", PREMAKE_CONFIG_PATH, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* try to get pkg-config lib path from pmk.conf */
	pc_path = hash_get(gdata.pht, PMKCONF_PC_PATH_LIB);
	if (pc_path == NULL) {
		printf("Unable to find pkg-config libdir.\n");
		clean(&gdata);
		exit(EXIT_FAILURE);
	}

	pkg_collect(pc_path, gdata.ppd); /* nice hardcode isn't it ? :) */

#ifdef DEBUG_PMKPC
debugf("{main} usize = '%d'", da_usize(gdata.pda));
#endif
	for (i = 0 ; i < da_usize(gdata.pda) ; i++) {
		mod = da_idx(gdata.pda, i);
#ifdef DEBUG_PMKPC
debugf("{main} mod = '%s' (i = %d)", mod, i);
#endif
		if (pkg_mod_exists(gdata.ppd, mod) == true) {
#ifdef DEBUG_PMKPC
debugf("module '%s' found", mod);
#endif
			ppc = pkg_cell_add(gdata.ppd, mod); /* ppc is part of ppd, don't destroy */

			if (opt_modvers == true) {
				/* print module version */
				printf("%s\n", ppc->version);
			}

			if ((opt_cflags == true) || (opt_libs == true)) {
				/* get cflags and/or libs */
				if (pkg_recurse(gdata.ppd, mod) == false) {
					clean(&gdata);
					errorf("failed on recurse !");
					exit(EXIT_FAILURE);
				}

				if (opt_cflags == true) {
					printf("%s", pkg_get_cflags(gdata.ppd));
				}
				if (opt_libs == true) {
					printf("%s", pkg_get_libs(gdata.ppd));
				}
				printf("\n");
			}
		} else {
#ifdef DEBUG_PMKPC
debugf("module not found");
#endif

			/* init config tool data if needed */
			if (pcd == NULL) {
				pcd = parse_cfgt_file();
				if (pcd == NULL) {
					clean(&gdata);
					errorf("unable to load config tool data.");
					exit(EXIT_FAILURE);
				}
			}

			/* set config tool filename */
			if (cfgtcell_get_binary(pcd, mod, pc_cmd, sizeof(pc_cmd)) == false) {
				snprintf(pc_cmd, sizeof(pc_cmd), "%s-config", mod); /* XXX check */
			}

			bpath = hash_get(gdata.pht, PMKCONF_PATH_BIN);
			if (bpath == NULL) {
				/* XXX */
				exit(EXIT_FAILURE);
			}

			pcc = cfgtcell_get_cell(pcd, pc_cmd);

			/* looking for it in the path */
			if (get_file_path(pc_cmd, bpath, pc_buf, sizeof(pc_buf)) == true) {
				/* use CHECK_CONFIG */
#ifdef DEBUG_PMKPC
debugf("Found alternative '%s' tool.", pc_cmd);
#endif

				if (opt_modvers == true) {
					/* check if specific option exists */
					if ((pcc != NULL) && (pcc->version != NULL)) {
						opt = pcc->version;
					} else {
						opt = CFGTOOL_OPT_VERSION;
					}

					if (ct_get_version(pc_buf, opt, buf, sizeof(buf)) == false) {
						clean(&gdata);
						errorf("cannot get version from '%s'.", pc_cmd);
						exit(EXIT_FAILURE);
					} else {
						printf("%s\n", buf);
					}
				}

				/* cflags and libs stuff */
				if ((opt_cflags == true) || (opt_libs == true)) {
					if (opt_cflags == true) {
						/* check if there is a special option */
						if ((pcc != NULL) && (pcc->cflags != NULL)) {
							opt = pcc->cflags;
						} else {
							opt = CFGTOOL_OPT_CFLAGS;
						}

						/* get cflags from config tool */
						if (ct_get_data(pc_buf, opt, "", buf, sizeof(buf)) == false) {
							errorf("cannot get CFLAGS.");
							exit(EXIT_FAILURE);
						} else {
							printf("%s ", buf);
						}
					}
					if (opt_libs == true) {
						/* check if there is a special option */
						if ((pcc != NULL) && (pcc->libs != NULL)) {
							opt = pcc->libs;
						} else {
							opt = CFGTOOL_OPT_LIBS;
						}

						/* get libs from config tool */
						if (ct_get_data(pc_buf, opt, "", buf, sizeof(buf)) == false) {
							errorf("cannot get LIBS.");
							exit(EXIT_FAILURE);
						} else {
							printf("%s ", buf);
						}
					}
					printf("\n");
				}
			} else {
				errorf("'%s' not found.", pc_cmd);
				exit(EXIT_FAILURE);
			}
		}
	}
#ifdef DEBUG_PMKPC
debugf("{main} destroy data");
#endif
	clean(&gdata);
	if (pcd != NULL) {
		/* XXX msg */
		cfgtdata_destroy(pcd);
	}

#ifdef DEBUG_PMKPC
debugf("End.");
#endif

	return(0);
}
