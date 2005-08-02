/* $Id$ */

/* Public Domain */

/* make variables test */

#include <stdio.h>

#include "../common.h"

int main(void) {
	char	tmp[256];

	get_make_var("CC", tmp, 256);
	printf("CC => %s\n", tmp);

	get_make_var("CFLAGS", tmp, 256);
	printf("CFLAGS => %s\n", tmp);

	get_make_var("LDFLAGS", tmp, 256);
	printf("LDFLAGS => %s\n", tmp);

	return(0);
}
