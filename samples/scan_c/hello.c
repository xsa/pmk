/* $Id$ */

/* public domain */

#include <stdio.h>
#include <string.h>

#define STR	"hello, i am a character string :)"

int main() {
	char buf[256];

	strlcpy(buf, STR, sizeof(buf));
	printf("%s\n", buf);

	return(0);
}
