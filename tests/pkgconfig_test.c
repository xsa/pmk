/* $Id$ */

/* Public Domain */

/* pkg-config tools test */

#define PKGCFG_DEBUG	1

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
	char	*pstr;
	htable	*pht;
	pkgcell	*ppc;

	pht = hash_init(512);
	if (pht == NULL) {
		errorf("cannot init hash table.");
		exit(EXIT_FAILURE);
	}

	scan_dir(PKGCONFIG_DIR, pht);

	if (argc == 2) {
		pstr = hash_get(pht, argv[1]);
		if (pstr != NULL) {
			printf("module '%s' found (in file '%s')\n", argv[1], pstr);
			ppc = parse_pc_file(pstr);

			printf("module name = %s\n", ppc->name);
			printf("module description = %s\n", ppc->descr);
			printf("module version = %s\n", ppc->version);
			printf("module requires: %s\n", ppc->requires);
			/*printf("module  = %s\n", ppc->);*/
		} else {
			printf("module not found\n");
		}
	}

	hash_destroy(pht);

	return(0);
}
