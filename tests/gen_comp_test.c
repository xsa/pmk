/* $Id$ */

/* Public Domain */

/* generate compiler detection test file */

/*#define PRS_DEBUG	1*/

#include <stdio.h>
#include <stdlib.h>

#include "../compat.c"
#include "../common.c"
#include "../detect.c"
#include "../dynarray.c"
#include "../hash.c"
#include "../parse.c"
#include "../pmk_obj.c"

int main() {
	FILE		*fp;
	comp_data	*cdata;

	printf("Parsing compiler data.\n");
	cdata = parse_comp_file("../data/pmkcomp.dat");

	fp = fopen("cc_test.c", "w");
	if (fp == NULL) {
		printf("Failed to open 'cc_test.c'.\n");
		exit(EXIT_FAILURE);
	}

	gen_test_file(fp, cdata);

	fclose(fp);
	
	compdata_destroy(cdata);

	return(0);
}

