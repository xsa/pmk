/* $Id$ */

/*
 * Copyright (c) 2003 Damien Couderc
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


#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

#define MKSTEMPS_REPLACE_CHAR	'X'

int mkstemps(char *template, int suffixlen) {
	struct timeval	 tv;
	char		 subst[] = "aZbY0cXdWe1VfUgT2hSiRj3QkPlO4mNnMo5LpKqJ6rIsHt7GuFvE8wDxCy9BzA",
			*start,
			*end,
			*p;
	int		 fd,
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
	srandom((unsigned int) template * tv.tv_sec + tv.tv_usec); /* XXX make better */

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

