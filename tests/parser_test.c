/* $Id$ */

/* Public Domain */

/* parser engine test */

#include <stdio.h>
#include "../autoconf.c"
#include "../common.c"
#include "../detect.c"
#include "../dynarray.c"
#include "../func.c"
#include "../functool.c"
#include "../hash.c"
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
		exit(1);
	}

	pdata = prsdata_init();

	fd = fopen(argv[1], "r");
	if (fd == NULL) {
		errorf("cannot open '%s'.", argv[1]);
		return(false);
	}

	rval = parse_pmkfile(fd, pdata, kw_pmkfile, nbkwpf);
	fclose(fd);

	printf("cleaning ...\n");
	prsdata_destroy(pdata);
	printf("ok\n");

	return(0);
}
