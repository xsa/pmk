/* $Id$ */

/* Public Domain license */

/* hash table test */

#include <stdlib.h>
#include <unistd.h>

#include "../compat/pmk_stdio.h"
#include "../compat/pmk_string.h"
#include "../hash_tools.h"

#define TAB_SIZE 256
#define NBKEYS 256


/*hpair	testab[] = {                                 */
/*                        { "test1", "test value 1"},*/
/*                        { "test2", "test value 2"},*/
/*                        { "test3", "test value 3"} */
/*                };                                 */

int main(void) {
	bool		r;
	char		tstr[256],
			ttstr[256],
			*val;
	hkeys_t		*phk = NULL;
	htable_t	*hp;
	unsigned int	i;

	printf("Testing init\n");
	hp = hash_create_simple(TAB_SIZE / 2);
	hp->autogrow = true;

	printf("Adding test key\n");
	hash_add(hp, "prefix", strdup("/usr/local"));

	printf("Testing key : ");
	val = hash_get(hp, "prefix");
	if (val == NULL) {
		printf("not found\n");
	} else {
		printf("found '%s'\n", val);
	}

	printf("Appending to the test key value\n");
	hash_append(hp, "prefix", strdup("lll"), NULL);

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

	/*n = sizeof(testab) / sizeof(hpair);*/
	/*printf("Adding %d test keys\n", n);*/
	/*hash_add_array(hp, testab, n);     */
	hash_add_dup(hp, "test1", "value 1");
	hash_add_dup(hp, "test2", "value 2");
	hash_add_dup(hp, "test3", "value 3");

	phk = hash_keys(hp);
	if (phk == NULL) {
		printf("Nothing to display !!!\n");
	} else {
		printf("Displaying keys :\n");
		for(i = 0 ; i < phk->nkey ; i++) {
			printf("\t%s", (char *) phk->keys[i]);
			printf(" => %s\n", (char *) hash_get(hp, phk->keys[i]));
		}
	}

	printf("Removing 3 test keys\n");
	hash_delete(hp, "test1");
	hash_delete(hp, "test2");
	hash_delete(hp, "test 3");

	printf("Adding %d random keys\n", NBKEYS);

	for(i = 0 ; i < NBKEYS ; i++) {
		snprintf(tstr, sizeof(tstr), "XXXXXXXXXX");
		/* mktemp() is only used to generate random values */
		if (mktemp(tstr) != NULL) {
			snprintf(ttstr, sizeof(ttstr), "value.%s", tstr);

			r = hash_add(hp, tstr, strdup(ttstr));
			printf("(%3d) ", i);
			switch (hp->herr) {
				case HERR_ADDED:
					printf("Added for key %s\n", tstr);
					break;

				case HERR_UPDATED:
					printf("Updated key %s\n", tstr);
					break;

				default:
					printf("Failed add for key %s\n", tstr);
					break;
			}
		} else {
			printf("Random value failed\n");
		}
	}

	printf("Testing destroy\n");
	/*printf("Removing %d key(s)\n", n);*/
	hash_destroy(hp);

	return(0);
}
