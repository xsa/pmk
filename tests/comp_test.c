/* $Id$ */

/* Public Domain */

/* compiler detection test */

/*#define DEBUG_PRS	1*/

#include <stdio.h>
#include <stdlib.h>

#include "../compat.c"
#include "../common.c"
#include "../detect.c"
#include "../dynarray.c"
#include "../hash.c"
#include "../parse.c"
#include "../pmk_obj.c"

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
		printf("Failed");
		exit(EXIT_FAILURE);
	}

	pcell = comp_get(cdata, cinfo.c_id);
	if (pcell == NULL) {
		printf("Failed.\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Detected compiler : '%s'\n", comp_get_descr(cdata, cinfo.c_id));
		printf("Compiler version : '%s'\n", cinfo.version);
	}
	
	return(0);
}

