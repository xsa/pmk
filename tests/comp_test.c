/* $Id$ */

/* Public Domain */

/* compiler detection test */


#include <stdio.h>

#include "../compat.c"
#include "../common.c"
#include "../detect.c"
#include "../dynarray.c"

int main(int argc, char *argv[]) {
	char		*cc;
	comp_data	 cdata;

	if (argc != 2) {
		printf("you must provide a compiler name\n");
		exit(EXIT_FAILURE);
	}

	cc = argv[1];
	printf("Using compiler : '%s'\n", cc);

	detect_compiler(cc, "/dev/null", &cdata);

	printf("Detected compiler : '%s'\n", cdata.descr);
	printf("Compiler version : '%s'\n", cdata.version);

	return(EXIT_SUCCESS);
}

