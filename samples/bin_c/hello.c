/* $Id */

/* This file is in the public domain. */

#include <stdio.h>
#include <string.h>

int main(void) {
	char	buf[1024];

	strlcpy(buf, "Hello ", sizeof(buf));
	strlcat(buf, "world", sizeof(buf));

	printf("%s\n", buf);

	return(0);
}
