#include <stdio.h>
#include "../common.c"
#include "../dynarray.c"
#include "../hash.c"
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
		errorf("cannot open '%s'", argv[1]);
		return(false);
	}

	rval = parse(fd, pdata);
	fclose(fd);

	prsdata_destroy(pdata);

	return(0);
}
