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


/*******************************************************************
 *                                                                 *
 * Path manipulation tools                                         *
 *                                                                 *
 *******************************************************************/


#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "pathtools.h"

#define CHAR_DOT '.'
#define CHAR_EOS '\0'
#define CHAR_SEP '/'

/*
	check a path and resolve "./", "../" and extra '/' characters

	path : path to check
	buffer : MAXPATHLEN sized buffer that will contain resolved path

	returns bool is check is well done
*/

bool chkpath(char *path, char *buffer) {
	bool	 exit = false,
		 dot = false,
		 dotdot = false,
		 sep = false;
	char	*pstr,
		*pbuf;
	int	 s = MAXPATHLEN;

	pstr = path;
	pbuf = buffer;

	while ((exit == false) && (s > 0)) {
		switch (*pstr) {
			case CHAR_DOT :
				/* found a dot */
				if (dot == false) {
					dot = true;
				} else {
					if (dotdot == false) {
						dotdot = true;
					} else {
						/* third dot found ??? */
						return(false);
					}
				}
				break;

			case CHAR_EOS :
				exit = true;
				/* process end of line as a separator too */

			case CHAR_SEP :
				/* found separator */
				if (dot == true) {
					if (dotdot == true) {
						/* found "../", going one dir back if possible */
							
						pbuf--; /* go back on last char */
						s++;

						if (pbuf > buffer) {
							/* skip trailing '/' if not root */
							pbuf--;
							s++;
						}

						/* now searching previous dir */
						while ((*pbuf != '/') && (pbuf > buffer)) {
							pbuf--;
							s++;
						}

						/* okay found, now set pointer to next char */
						pbuf++;
						s--;

						dotdot = false;
					}
					/* if found only "./" then just ignore */
					dot = false;
				} else {
					if (sep == false) {
						/* previous was not a separator so copy it */
						sep = true;
						*pbuf = *pstr;
						pbuf++;
						s--;
					}
					/* if previous har was a separator the drop it */
				}
				break;
			default :
				/* dot already found ? */
				if (dot == true) {
					if (dotdot == true) {
						/* should have a separator after two dot ??? */
						return(false);
					}

					/* hidden dir */
					if (s > 1) {
						/* got enough space for dot and current char */
						*pbuf = CHAR_DOT;
						pbuf++;
						s--;
					} else {
						/* not enough space */
						return(false);
					}
					dot = false;
				}

				/* copy char */
				*pbuf = *pstr;
				pbuf++;
				s--;

				if (sep == true) {
					sep = false;
				}
				break;
		}
		pstr++;
	}

	if (sep == true) {
		/*  skip trailing separator */
		pbuf--;
		s++;
	}

	if (s > 0) {
		/* NULL terminate the string */
		*pbuf = CHAR_EOS;
		return(true);
	} else {
		return(false);
	}
}

/*
	provide a relative path

	from : source path
	to : destination path
	buffer : MAXPATHLEN sized buffer that will contain relative path
*/

bool relpath(char *from, char *to, char *buffer) {
	bool	exit = false;
	char	from_buf[MAXPATHLEN],
		to_buf[MAXPATHLEN];

	/* check 'from' path */
	if (chkpath(from, from_buf) != true) {
		return(false);
	}
	from = from_buf;

	/* check 'to' path */
	if (chkpath(to, to_buf) != true) {
		return(false);
	}
	to = to_buf;

	buffer[0] = CHAR_EOS;

	/* loop until common base is dropped */
	while (exit == false) {
		if ((*from == *to) && (*from != CHAR_EOS)) {
			from++;
			to++;
		} else {
			exit = true;
		}
	}

	/* go back on the last '/' if needed */
	if ((*from != CHAR_EOS) && (*to != CHAR_EOS)) {
		while (*from != '/') {
			from--;
			to--;
		}
	}

	/* ignore leading '/' for to */
	if (*to == '/') {
		to++;
	}

	/* count directories */
	while (*from != CHAR_EOS) {
		if (*from == '/') {
			/* one more */
			strlcat(buffer, "../", MAXPATHLEN);
		}
		from++;
	}
	/* appending remaining value of "to" */
	strlcat(buffer, to, MAXPATHLEN);

	if (buffer[0] == CHAR_EOS) {
		/* same path, return "." */
		strlcpy(buffer, ".", MAXPATHLEN);
	}

	return(true);
}

/*
	create a "valid" absolute path

	base : absolute base path
	rel : relative path from the base
	buffer : MAXPATHLEN buffer that will contain resulting absolute path

	returns true on success
*/

bool abspath(char *base, char *rel, char *buffer) {
	char	tmp_buf[MAXPATHLEN]; /* XXX should be greater ? */

	/* check if 'rel' is not relative */
	if (*rel == CHAR_SEP) {
		return(false);
	}

	strlcpy(tmp_buf, base, MAXPATHLEN);
	strlcat(tmp_buf, "/", MAXPATHLEN);
	strlcat(tmp_buf, rel, MAXPATHLEN);

	return(chkpath(tmp_buf, buffer));
}

/*
	provide an absolute path from a path with unknow type.

	base : absolute base path
	upath : path with unknow type (relative or absolute)
	buffer : MAXPATHLEN sized buffer

	NOTE : if upath is absolute then buffer is filled with
	upath else the buffer will contain the computed path.
*/

bool uabspath(char *base, char *upath, char *buffer) {
	if (*upath == CHAR_SEP) {
		/* upath is absolute */
		return(chkpath(upath, buffer));
	} else {
		/* upath is relative, we can call abspath */
		return(abspath(base, upath, buffer));
	}
}
