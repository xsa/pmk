/* $Id$ */

/* Public Domain license */

/* hash table test */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../hash.c"

#define TAB_SIZE 256
#define NBKEYS 256


hpair	testab[] = {
			{ "test1", "test value 1"},
			{ "test2", "test value 2"},
			{ "test3", "test value 3"}
		};

int main() {
	int	i,
		n;
	char	tstr[256],
		ttstr[256],
		*val,
		**keys = NULL;
	htable	*hp;

	printf("Testing init\n");
	hp = hash_init(TAB_SIZE / 2);
	hash_set_grow(hp);

	printf("Adding test key\n");
	hash_add(hp, "prefix", "/usr/local");

	printf("Testing key : ");
	val = hash_get(hp, "prefix");
	if (val == NULL) {
		printf("not found\n");
	} else {
		printf("found '%s'\n", val);
	}

	printf("Appending to the test key value\n");
	hash_append(hp, "prefix", "lll", NULL);

	printf("Testing key : ");
	val = hash_get(hp, "prefix");
	if (val == NULL) {
		printf("not found\n");
	} else {
		printf("found '%s'\n", val);
	}

	printf("Appending to the test key value with a separator\n");
	hash_append(hp, "prefix", "/opt", ",");

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

	keys = hash_keys(hp);
	n = hash_nbkey(hp);
	printf("Displaying %d keys :\n", n);
	for(i = 0 ; i < n ; i++) {
		printf("\t%s => %s\n", keys[i], hash_get(hp, keys[i]));
	}

	printf("Removing 3 test keys\n");
	hash_delete(hp, "test1");
	hash_delete(hp, "test2");
	hash_delete(hp, "test3");

	printf("Adding %d random keys\n", NBKEYS);

	for(i = 0 ; i < NBKEYS ; i++) {
		snprintf(tstr, sizeof(tstr), "XXXXXXXXXX");
		if (mktemp(tstr) != NULL) {
			snprintf(ttstr, sizeof(ttstr), "value.%s", tstr);

			n = hash_add(hp, tstr, ttstr);
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

	printf("Testing destroy\n");
	n = hash_destroy(hp);	
	printf("Removed %d key(s)\n", n);

	return(0);
}
