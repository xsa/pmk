/* $Id$ */

/* This file is in the public domain. */

#include <stdio.h>
#include <string.h>
#include "config.h"

#define FMT_STRLCPY_MSG	"Your system %s have strlcpy() in 'string.h'.\n" 
#define FMT_STRLCAT_MSG	"Your system %s have strlcat() in 'string.h'.\n" 
#define MSG_HAS		"does"
#define MSG_HAS_NOT	"does not"

int main(void) {
	char	buf[1024];

#ifdef HAVE_STRLCPY
	printf(FMT_STRLCPY_MSG, MSG_HAS);
#else
	printf(FMT_STRLCPY_MSG, MSG_HAS_NOT);
#endif

#ifdef HAVE_STRLCAT
	printf(FMT_STRLCAT_MSG, MSG_HAS);
#else
	printf(FMT_STRLCAT_MSG, MSG_HAS_NOT);
#endif

	return(0);
}
