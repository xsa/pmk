/* $Id$ */

/* Public Domain */

/* compiler detection test */

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

int main(int argc, char *argv[]) {
	char		*cc;
	comp_cell	*pcell;
	comp_data	*cdata;
	comp_info	 cinfo;

	if (argc != 2) {
		printf("you must provide a compiler name\n");
		exit(EXIT_FAILURE);
	}

	cc = argv[1];
	printf("Using compiler : '%s'\n", cc);

	printf("Parsing compiler data.\n");
	cdata = parse_comp_file("../data/pmkcomp.dat");

	printf("Detecting compiler ...\n");
	if (detect_compiler(cc, "/dev/null", cdata, &cinfo) == false) {
		compdata_destroy(cdata);
		printf("Failed");
		exit(EXIT_FAILURE);
	}

	pcell = comp_get(cdata, cinfo.c_id);
	if (pcell == NULL) {
		compdata_destroy(cdata);
		printf("Failed.\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Detected compiler : '%s'\n", comp_get_descr(cdata, cinfo.c_id));
		printf("Compiler version : '%s'\n", cinfo.version);
	}

	compdata_destroy(cdata);

	return(0);
}

