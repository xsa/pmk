/* $Id$ */

/* Public Domain */

/* pkg-config tools test */

/*#define PKGCFG_DEBUG	1*/
/*#define HASH_DEBUG	1*/

#include <stdlib.h>

#include "../compat/pmk_stdbool.h"
#include "../compat/pmk_stdio.h"
#include "../common.h"
#include "../pkgconfig.h"

int main(int argc, char *argv[]) {
	char	*mod;
	pkgcell	*ppc;
	pkgdata	*ppd;

	ppd = pkgdata_init();
	if (ppd == NULL) {
		errorf("cannot init pkgdata.");
		exit(EXIT_FAILURE);
	}

	/*scan_dir(PKGCONFIG_DIR, ppd);*/
	pkg_collect("/usr/local/lib/pkgconfig", ppd); /* nice hardcode isn't it ? :) */

	if (argc == 2) {
		mod = argv[1];
		if (pkg_mod_exists(ppd, mod) == true) {
			printf("module '%s' found\n", mod);
			ppc = pkg_cell_add(ppd, mod);

			printf("module name = %s\n", ppc->name);
			printf("module description = %s\n", ppc->descr);
			printf("module version = %s\n", ppc->version);
			printf("module requires: %s\n", ppc->requires);
			/*printf("module  = %s\n", ppc->);*/

			/* get cflags and libs */
			if (pkg_recurse(ppd, mod) == false) {
				errorf("failed on recurse !");
			} else {
				printf("\ncflags = '%s'\n", pkg_get_cflags(ppd));
				printf("\nlibs = '%s'\n", pkg_get_libs(ppd));
			}
		} else {
			printf("module not found\n");
		}
	}

#ifdef PKGCFG_DEBUG
debugf("destroy pkgdata");
#endif
	pkgdata_destroy(ppd);

	return(0);
}
