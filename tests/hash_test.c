#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../hash.c"

#define TAB_SIZE 256


int main() {
	int	i,
		r;
	char	tstr[256],
		ttstr[256];
	htable	*hp;

	printf("Testing init\n");
	hp = hash_init(TAB_SIZE);

	printf("Adding random keys\n");
/*
	hash_add(hp, "prefix", "/usr/locallll");
*/

	for(i = 0 ; i < 255 ; i++) {
		snprintf(tstr, sizeof(tstr), "XXXXXXXXXX");
		if (mktemp(tstr) != NULL) {
			snprintf(ttstr, sizeof(ttstr), "value.%s", tstr);

			r = hash_add(hp, tstr, ttstr);
			switch (r) {
				case HASH_ADD_FAIL:
					printf("Failed add for key %s\n", tstr);
					break;
				case HASH_ADD_OKAY:
					printf("Added for key %s\n", tstr);
					break;
				case HASH_ADD_COLL:
					printf("Collision for key %s\n", tstr);
					break;
			}
		} else {
			printf("probleme\n");
		}
	}

	printf("Testing key\n");
	if (hash_get(hp, "prefix") == NULL) {
		printf("key prefix not found\n");
	}

	printf("Testing destroy\n");
	r = hash_destroy(hp);	
	printf("Removed %d key(s)\n", r);

	return(0);
}
