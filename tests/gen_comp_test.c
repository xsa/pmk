/* $Id$ */

/* Public Domain */

/* generate compiler detection test file */

/*#define PRS_DEBUG	1*/

#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "../compat/compat.h"
#include "../detect.h"
#include "../dynarray.h"
#include "../hash.h"
#include "../parse.h"
#include "../pmk_obj.h"

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

