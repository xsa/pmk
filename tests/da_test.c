/* $Id$ */

/* Public Domain */

/* dynamic array tests */


#include <stdio.h>
#include <stdlib.h>

#include "../compat/pmk_string.h"
#include "../dynarray.h"


char	*tval[10] = {
		"zero",
		"one",
		"two",
		"three",
		"four",
		"five",
		"six",
		"seven",
		"eight",
		"nine"
};

int main(void) {
	char	*p;
	dynary	*da;
	int	i;

	printf("Init dynary.\n");
	da = da_init();

	printf("\nAdding 9 values :\n");
	for (i = 0 ; i < 10 ; i++) {
		printf("\tAdd value '%s' ", tval[i]);
		da_push(da, strdup(tval[i]));
		printf("(%d)\n", da_size(da));
	}

	printf("\nTesting values :\n");
	for (i = 9 ; i >= 0 ; i--) {
		printf("\tda[%d] = %s\n", i, (char *)da_idx(da, i));
	}

	printf("\nRemoving values using da_pop :\n");
	do {
		p = da_pop(da);
		if (p != NULL) {
			printf("\tPoped '%s' (%d)\n", p, da_size(da));
			free(p);
		}
	} while (p != NULL);

	printf("\nAdding 9 values :\n");
	for (i = 0 ; i < 10 ; i++) {
		printf("\tAdd value '%s' ", tval[i]);
		da_push(da, strdup(tval[i]));
		printf("(%d)\n", da_size(da));
	}

	printf("\nTesting values :\n");
	for (i = 9 ; i >= 0 ; i--) {
		printf("\tda[%d] = %s\n", i, (char *)da_idx(da, i));
	}

	printf("\nRemoving values using da_shift :\n");
	do {
		p = da_shift(da);
		if (p != NULL) {
			printf("\tShifted '%s' (%d)\n", p, da_size(da));
			free(p);
		}
	} while (p!= NULL);

	printf("\nCleaning.\n");
	da_destroy(da);

	return(0);
}
