/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
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


#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_string.h"
#include "common.h"
#include "pkgconfig.h"
#include "pmkpc.h"
#include "premake.h"


/*#define DEBUG_PMKPC	1*/

extern char	*optarg;
extern int	 optind;

pcopt	pcoptions[] = {
		{"version",	false,	PMKPC_OPT_VERSION},
		{"modversion",	false,	PMKPC_OPT_MODVERS},
		{"cflags",	false,	PMKPC_OPT_CFLAGS},
		{"libs",	false,	PMKPC_OPT_LIBS},
		{"exists",	false,	PMKPC_OPT_EXISTS}
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
	
	if (opt[0]!= '-' && opt[1]!= '-') {
#ifdef DEBUG_PMKPC
debugf("{pcgetopt} full opt = '%s'", opt);
#endif
		/* not an option */
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
	poc->arg = opt;

	return(true);
}

/*
	XXX
*/

int main(int argc, char *argv[]) {
	bool	 opt_version = false,
		 opt_modvers = false,
		 opt_exists = false,
		 opt_cflags = false,
		 opt_libs = false;
	char	*mod;
	optcell	*poc;
	pkgcell	*ppc;
	pkgdata	*ppd;

	poc = optcell_init();
	if (poc == NULL) {
		errorf("cannot init optcell.");
		exit(EXIT_FAILURE);
	}

	ppd = pkgdata_init();
	if (ppd == NULL) {
		optcell_destroy(poc);
		errorf("cannot init pkgdata.");
		exit(EXIT_FAILURE);
	}

	while (pcgetopt(argc, argv, poc) == true) {
#ifdef DEBUG_PC
debugf("{main} id = %d", poc->id);
#endif
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

			case PMKPC_OPT_UNKNOWN :
			default :
				optcell_destroy(poc);
				errorf("unknown option '%s'.", poc->arg);
				exit(EXIT_FAILURE);
				break;
		}
	}

	argc = argc - poc->idx;
	argv = argv + poc->idx;

	optcell_destroy(poc);

	if (opt_version == true) {
		printf("%s\n", PMKPC_COMPAT_VERSION);
		exit(EXIT_SUCCESS);
	}

	/*scan_dir(PKGCONFIG_DIR, ppd);*/
	pkg_collect("/usr/local/lib/pkgconfig", ppd); /* nice hardcode isn't it ? :) */

	if (argc == 1) {
		mod = argv[0];
#ifdef DEBUG_PMKPC
debugf("{main} mod = '%s'", mod);
#endif
		if (pkg_mod_exists(ppd, mod) == true) {
			/*printf("module '%s' found\n", mod);*/
			ppc = pkg_cell_add(ppd, mod);

			if (opt_modvers == true) {
				/* print module version */
				printf("%s\n", ppc->version);
			}

			if ((opt_cflags == true) || (opt_libs == true)) {
				/* get cflags and/or libs */
				if (pkg_recurse(ppd, mod) == false) {
					errorf("failed on recurse !");
				} else {
					if (opt_cflags == true) {
						printf("%s", pkg_get_cflags(ppd));
					}
					if (opt_libs == true) {
						printf("%s", pkg_get_libs(ppd));
					}
					printf("\n");
				}
			}
		} else {
			printf("module not found\n");
		}
	}

#ifdef DEBUG_PMKPC
debugf("{main} destroy pkgdata");
#endif
	pkgdata_destroy(ppd);

#ifdef DEBUG_PMKPC
debugf("End.");
#endif

	return(0);
}
