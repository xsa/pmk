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

#define PRS_DEBUG	1
#include "../parse.c"


/*
	process option line of configuration file

	pht : storage hash table
	popt : option structure to record

	return : boolean
*/

bool process_fake(htable *pht, prsopt *popt) {
	/*if ((popt->opchar != CHAR_COMMENT) && (popt->opchar != CHAR_EOS)) {                     */
	/*        |+ add options that are not comment neither blank lines +|                      */
	/*        if (hash_update_dup(pht, popt->key, po_get_str(popt->value)) == HASH_ADD_FAIL) {*/
	/*                errorf("hash failure.");                                                */
	/*                return(false);                                                          */
	/*        }                                                                               */
	/*}                                                                                       */
	return(true);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: parser_test [-ch] [file path]\n");
}

/*
	main loop
*/

int main(int argc, char *argv[]) {
	FILE	*fd;
	bool	 loop = true,
		 parse_cfg = false,
		 rval;
	char	 chr;
	prsdata	*pdata;
	htable	*pht;

	while (loop == true) {
		chr = getopt(argc, argv, "ch");
		if (chr == -1) {
			loop = false;
		} else {
			switch (chr) {
				case 'c' :
					/* parse a config file */
					parse_cfg = true;
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					exit(EXIT_FAILURE);
					/* NOTREACHED */
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

	if (argc != 1) {
		printf("filename not provided.\n");
		exit(EXIT_FAILURE);
	}

	pdata = prsdata_init();

	fd = fopen(argv[0], "r");
	if (fd == NULL) {
		errorf("cannot open '%s'.", argv[0]);
		return(false);
	}

	if (parse_cfg == false) {
		rval = parse_pmkfile(fd, pdata, kw_pmkfile, nbkwpf);

		printf("cleaning parsing tree ... ");
		prsdata_destroy(pdata);

		printf("ok\n");
	} else {
		pht = hash_init_adv(1024, NULL, (void (*)(void *)) prsopt_destroy, NULL);
		if (pht == NULL) {
			printf("cannot create hash table.\n");
			exit(EXIT_FAILURE);
		}

		rval = parse_pmkconf(fd, pht, PRS_PMKCONF_SEP, process_fake);

		printf("cleaning hash table ... ");
		hash_destroy(pht);
		printf("ok\n");
	}
	fclose(fd);

	if (rval == true) {
		printf("Parsing succeeded.\n");
	} else {
		printf("Parsing failed.\n");
	}

	return(0);
}
