/* $Id$ */

/* Public Domain */

/* pkg-config tools test */

#define PKGCFG_DEBUG	1
/*#define HASH_DEBUG	1*/

#include <stdio.h>

#include "../compat/pmk_stdbool.h"
#include "../common.c"
#include "../dynarray.c"
#include "../hash.c"
#include "../parse.c"
#include "../pkgconfig.c"
#include "../pmk_obj.c"
#include "../premake.h"

int main(int argc, char *argv[]) {
	char	*mod,
		*pstr;
	pkgcell	*ppc;
	pkgdata	*ppd;

	ppd = pkgdata_init();
	if (ppd == NULL) {
		errorf("cannot init pkgdata.");
		exit(EXIT_FAILURE);
	}

	scan_dir(PKGCONFIG_DIR, ppd);

	if (argc == 2) {
		mod = argv[1];
		pstr = hash_get(ppd->files, mod);
		if (pstr != NULL) {
			printf("module '%s' found (in file '%s')\n", mod, pstr);
			ppc = parse_pc_file(pstr);

			printf("module name = %s\n", ppc->name);
			printf("module description = %s\n", ppc->descr);
			printf("module version = %s\n", ppc->version);
			printf("module requires: %s\n", ppc->requires);
			/*printf("module  = %s\n", ppc->);*/
			/* XXX clean ppc */

			/* XXX */
			if (pkg_recurse(ppd, mod) == false) {
				errorf("failed on recurse !");
			}
		} else {
			printf("module not found\n");
		}
	}

debugf("destroy pkgdata");
	pkgdata_destroy(ppd);

	return(0);
}
