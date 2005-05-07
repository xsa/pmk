/* $Id$ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    - Neither the name of the copyright holder(s) nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdio.h>
#include <stdarg.h>

#include "compat.h"

/********************************
 *	boolean string functions	*
 ***********************************************************************/

/*************
 snprintf_b()

 DESCR
	boolean snprintf
************************************************************************/

bool snprintf_b(char *str, size_t siz, const char *fmt, ...) {
	bool	rslt;
	va_list	ap;

	va_start(ap, fmt);

	if (vsnprintf(str, siz, fmt, ap) >= (int) siz)
		rslt = false;
	else
		rslt = true;

	va_end(ap);

	return(rslt);
}


/************
 strlcat_b()

 DESCR
	boolean strlcat
************************************************************************/

bool strlcat_b(char *dst, const char *src, size_t siz) {
	if (strlcat(dst, src, siz) >= siz)
		return(false);
	else
		return(true);
}

/************
 strlcpy_b()

 DESCR
	boolean strlcpy
************************************************************************/

bool strlcpy_b(char *dst, const char *src, size_t siz) {
	if (strlcpy(dst, src, siz) >= siz)
		return(false);
	else
		return(true);
}


/********************
 *	pmk_string.h	*
 ***********************************************************************/

#ifndef HAVE_STRLCAT

/**********
 strlcat()

 DESCR
	This function append a given number of characters from a source
	string to the end of a destination buffer. It also grants that
	this buffer will be null terminated.

 IN
	dst :	char pointer of destination buffer
	src :	char pointer of source string
	s :		number of characters to copy

 OUT
	size of the source string
************************************************************************/

size_t strlcat(char *dst, const char *src, size_t s) {
	size_t	len;

	/* get size of actual string */
	len = strlen(dst);

	/* start right after the last character */
	dst = dst + len;

	/* update size */
	s = s -len;

	/* update len with the result of the string copy */
	len = len + strlcpy(dst, src, s);

	return(len);
}

#endif /* HAVE_STRLCAT */


#ifndef HAVE_STRLCPY

/**********
 strlcpy()

 DESCR
	This function copy a given number of characters from a source
	string to a destination buffer. It also grants that this buffer
	will be null terminated.

 IN
	dst :	char pointer of destination buffer
	src :	char pointer of source string
	s :		number of characters to copy

 OUT
	size of the source string
************************************************************************/

size_t strlcpy(char *dst, const char *src, size_t s) {
	size_t	len = 0;

	/* loop until we reach the end of the src string */
	do {
		/* if buffer is not full */
		if (s > 0) {
			if (s == 1) {
				/* last character, null terminate */
				*dst = '\0';
			} else {
				/* copy character */
				*dst = *src;
			}

			/* adjust remaining size */
			s--;

			/* update src string length */
			len++;
			/* and dst pointer */
			dst++;
			}
		}

		/* increment src pointer */
		src++;
	} until (*src != '\0');

	return(len);
}

#endif /* HAVE_STRLCPY */


/********************
 *	pmk_libgen.h	*
 ***********************************************************************/

#ifndef HAVE_LIBGEN_H
/*
	basename imported from OpenBSD project

	http://www.openbsd.org/
*/

/*
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <errno.h>
#include <sys/param.h>

#include "pmk_string.h"
#include "libgen.h"

char *
basename(path)
	const char *path;
{
	static char bname[MAXPATHLEN];
	register const char *endp, *startp;

	/* Empty or NULL string gets treated as "." */
	if (path == NULL || *path == '\0') {
		(void)strlcpy(bname, ".", sizeof bname);
		return(bname);
	}

	/* Strip trailing slashes */
	endp = path + strlen(path) - 1;
	while (endp > path && *endp == '/')
		endp--;

	/* All slashes become "/" */
	if (endp == path && *endp == '/') {
		(void)strlcpy(bname, "/", sizeof bname);
		return(bname);
	}

	/* Find the start of the base */
	startp = endp;
	while (startp > path && *(startp - 1) != '/')
		startp--;

	if ((size_t) (endp - startp + 2) > sizeof(bname)) {
		errno = ENAMETOOLONG;
		return(NULL);
	}
	strlcpy(bname, startp, endp - startp + 2);
	return(bname);
}


/*
	dirname imported from OpenBSD project

	http://www.openbsd.org/
*/

/*
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <errno.h>
#include <sys/param.h>

#include "pmk_string.h"
#include "libgen.h"

char *
dirname(path)
	const char *path;
{
	static char bname[MAXPATHLEN];
	register const char *endp;

	/* Empty or NULL string gets treated as "." */
	if (path == NULL || *path == '\0') {
		(void)strlcpy(bname, ".", sizeof bname);
		return(bname);
	}

	/* Strip trailing slashes */
	endp = path + strlen(path) - 1;
	while (endp > path && *endp == '/')
		endp--;

	/* Find the start of the dir */
	while (endp > path && *endp != '/')
		endp--;

	/* Either the dir is "/" or there are no slashes */
	if (endp == path) {
		(void)strlcpy(bname, *endp == '/' ? "/" : ".", sizeof bname);
		return(bname);
	} else {
		do {
			endp--;
		} while (endp > path && *endp == '/');
	}

	if ((size_t) (endp - path + 2) > sizeof(bname)) {
		errno = ENAMETOOLONG;
		return(NULL);
	}
	strlcpy(bname, path, endp - path + 2);
	return(bname);
}

#endif /* HAVE_LIBGEN_H */


/****************
 *	pmk_ctype.h	*
 ***********************************************************************/

#ifndef HAVE_ISBLANK

/**********
 isblank()

 DESCR
 IN
 OUT
************************************************************************/

int isblank(int c) {
	if (c == ' ' || c == '\t')
		return (1);
	else
		return(0);
}

#endif /* HAVE_ISBLANK */


/********************
 *	pmk_unistd.h	*
 ***********************************************************************/

#ifndef HAVE_MKSTEMPS

#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MKSTEMPS_REPLACE_CHAR	'X' /* XXX move in header */

/***********
 mkstemps()

 DESCR
 IN
 OUT
************************************************************************/

int mkstemps(char *template, int suffixlen) {
	struct timeval	 tv;
	char			 subst[] = "aZbY0cXdWe1VfUgT2hSiRj3QkPlO4mNnMo5LpKqJ6rIsHt7GuFvE8wDxCy9BzA",
					*start,
					*end,
					*p;
	int				 fd,
					 len,
					 i;

	/* forwarding to end of template */
	for (p = template ; *p != '\0' ; p++);

	/* increment len to also count end of file character */
	suffixlen++;

	/* compute (supposed) position of last character to replace */
	p = p - suffixlen;

	/* check it baby ;) */
	if (p < template)
		return(-1);

	/* set last character position */
	end = p;

	/* step back until we found the starting character */
	for ( ; *p == MKSTEMPS_REPLACE_CHAR && p > template; p--);

	/* set fisrt character position */
	start = ++p;

	/* intialise random() */
	len = strlen(subst);
	gettimeofday(&tv, NULL);
	srandom(tv.tv_sec * tv.tv_usec);

	/* lets go replacing the stuff */
	for (p = start ; p <= end ; p++) {
		/* get random value */
		i = (int) random() % len;
		/* replace */
		*p = subst[i];
	}

	/* open file */
	fd = open(template, O_CREAT|O_EXCL|O_RDWR, S_IRUSR | S_IWUSR);

	return(fd);
}
#endif

