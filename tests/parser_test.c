/* $Id$ */

/* Public Domain */

/* parser engine test */

#include <stdio.h>
#include "../autoconf.c"
#include "../common.c"
#include "../cfgtool.c"
#include "../detect.c"
#include "../dynarray.c"
#include "../func.c"
#include "../functool.c"
#include "../hash.c"
#include "../hash_tools.c"
#include "../pathtools.c"
#include "../pmk_obj.c"
#include "../pkgconfig.c"

#define DEBUG_PRS	1
#include "../parse.c"

int main(int argc, char *argv[]) {
	FILE	*fd;
	bool	 rval;
	prsdata	*pdata;

	if (argc != 2) {
		printf("filename not provided.\n");
		exit(EXIT_FAILURE);
	}

	pdata = prsdata_init();

	fd = fopen(argv[1], "r");
	if (fd == NULL) {
		errorf("cannot open '%s'.", argv[1]);
		return(false);
	}

	rval = parse_pmkfile(fd, pdata, kw_pmkfile, nbkwpf);
	fclose(fd);

	if (rval == true) {
		printf("Parsing succeeded.\n");
	} else {
		printf("Parsing failed.\n");
	}

	printf("cleaning parsing tree ... ");
	prsdata_destroy(pdata);
	printf("ok\n");

	return(0);
}
