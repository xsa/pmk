/* $Id$ */

/* Public Domain */

/* dynamic array tests */


#include <stdio.h>

#include "../dynarray.c"


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

int main() {
	int	i;
	dynary	*da;

	printf("Init dynary.\n");
	da = da_init();

	for (i = 0 ; i < 10 ; i++) {
		printf("Add value '%s' ", tval[i]);
		da_push(da, tval[i]);
		printf("(%d)\n", da_size(da));
	}

	for (i = 9 ; i >= 0 ; i--) {
		printf("da[%d] = %s\n", i, da_idx(da, i));
	}

	da_destroy(da);

	return(0);
}
