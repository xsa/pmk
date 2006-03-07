/* $Id$ */

/* Public Domain license */

/* hash table test */

#include <stdlib.h>
#include <unistd.h>

#include "../compat/pmk_stdio.h"
#include "../compat/pmk_string.h"
#include "../hash.h"

#define TAB_SIZE 256
#define NBKEYS 256


hpair	testab[] = {
			{ "test1", "test value 1"},
			{ "test2", "test value 2"},
			{ "test3", "test value 3"}
		};

int main(void) {
	int		n;
	char		tstr[256],
			ttstr[256],
			*val;
	herr		*err;
	hkeys		*phk = NULL;
	htable		*hp;
	unsigned int	i;
	size_t		nbcol;

	printf("Testing init\n");
	hp = hash_init(TAB_SIZE);
	/*hash_set_grow(hp);*/

	printf("Adding test key\n");
	err = hash_add(hp, "prefix", strdup("/usr/local"));
	if (err == HASH_ADD_FAIL) {
		printf("hash error.\n"); 
		exit(EXIT_FAILURE);
	}

	printf("Testing key : ");
	val = hash_get(hp, "prefix");
	if (val == NULL) {
		printf("not found\n");
	} else {
		printf("found '%s'\n", val);
	}

	printf("Appending to the test key value\n");
	err = hash_append(hp, "prefix", strdup("lll"), NULL);
	if (err == HASH_ADD_FAIL) {
		printf("hash error.\n"); 
		exit(EXIT_FAILURE);
	}

	printf("Testing key : ");
	val = hash_get(hp, "prefix");
	if (val == NULL) {
		printf("not found\n");
	} else {
		printf("found '%s'\n", val);
	}

	printf("Appending to the test key value with a separator\n");
	hash_append(hp, "prefix", strdup("/opt"), ",");

	printf("Testing key : ");
	val = hash_get(hp, "prefix");
	if (val == NULL) {
		printf("not found\n");
	} else {
		printf("found '%s'\n", val);
	}

	printf("Removing test key\n");
	hash_delete(hp, "prefix");

	printf("Testing key : ");
	if (hash_get(hp, "prefix") == NULL) {
		printf("not found\n");
	} else {
		printf("found\n");
	}

	n = sizeof(testab) / sizeof(hpair);
	printf("Adding %d test keys\n", n);
	hash_add_array(hp, testab, n);

	phk = hash_keys(hp);
	if (phk == NULL) {
		printf("hash_keys error.\n"); 
		exit(EXIT_FAILURE);
	}

	printf("Displaying %zd keys :\n", n);
	for(i = 0 ; i < phk->nkey ; i++) {
		printf("\t%s => %s\n", (char *) phk->keys[i], (char *) hash_get(hp, phk->keys[i]));
	}

	printf("Removing 3 test keys\n");
	hash_delete(hp, "test1");
	hash_delete(hp, "test2");
	hash_delete(hp, "test3");

	printf("Adding %d random keys\n", NBKEYS);

	nbcol = 0;
	for(i = 0 ; i < NBKEYS ; i++) {
		snprintf(tstr, sizeof(tstr), "XXXXXXXXXXXX");
		/* mktemp() is only used to generate random values */
		if (mktemp(tstr) != NULL) {
			snprintf(ttstr, sizeof(ttstr), "value.%s", tstr);

			n = hash_add(hp, tstr, strdup(ttstr));
			printf("(%3d) ", i);
			switch (n) {
				case HASH_ADD_FAIL:
					printf("Failed add for key %s\n", tstr);
					break;
				case HASH_ADD_OKAY:
					printf("Added for key %s\n", tstr);
					break;
				case HASH_ADD_COLL:
					printf("Collision for key %s\n", tstr);
					nbcol++;
					break;
				case HASH_ADD_UPDT:
					printf("Updated key %s\n", tstr);
					break;
				default:
					printf("Unknown return value %s\n", tstr);
					break;
			}
		} else {
			printf("Random value failed\n");
		}
	}

	printf("Result : %zd collisions for %d keys\n", nbcol, NBKEYS);

	printf("Testing destroy\n");
	n = hash_destroy(hp);
	printf("Removed %d key(s)\n", n);

	return(0);
}
