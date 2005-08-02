/* $Id$ */


/*
 * Copyright (c) 2003-2004 Damien Couderc
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
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "pathtools.h"


/*************
 * chkpath() *
 ***********************************************************************
 DESCR
	check a path and resolve "./", "../" and extra '/' characters

 IN
	path : path to check
	buffer : MAXPATHLEN sized buffer that will contain resolved path

 OUT
	returns bool if check is well done
 ***********************************************************************/

bool chkpath(char *path, char *buffer) {
	bool	 loop = true,
			 dot = false,
			 dotdot = false,
			 sep = false;
	char	*pbuf;
	int		 s = MAXPATHLEN;

	if (s < 2) {
		/* not enough space for at least "." */
		/* XXX printf("not enough space\n"); */
		return(false);
	}

	pbuf = buffer;

	while ((loop == true) && (s > 1)) {
		switch (*path) {
			case CHAR_DOT :
				/* unset separator flag */
				sep = false;

				/* found a dot */
				if (dot == false) {
					dot = true;
				} else {
					if (dotdot == false) {
						dotdot = true;
					} else {
						/* third dot found !!! */
						/* printf("triple dot found\n"); */
						return(false);
					}
				}
				break;

			case CHAR_EOS :
				/* exit from loop */
				loop = false;

				/* but consider end of string as a separator before */
				/* no break */

			case CHAR_SEP :
				/* found separator */
				if (dot == true) {
					/*
						if previous directory character sequence is not
						found then current directory character sequence
						is ignored
					*/
					if (dotdot == true) {
						/* if not at the begining of the buffer */
						if (pbuf != buffer) {
							/* if not root look for previous directory */
							if ((pbuf - buffer) > 1) {
								/* skip separator */
								pbuf = pbuf - 2;
								s = s + 2;

								/* look for separator */
								while ((*pbuf != CHAR_SEP) && (pbuf > buffer)) {
									pbuf--;
									s++;
								}

								/* if separator found go next char */
								if (*pbuf == CHAR_SEP) {
									pbuf++;
									s--;
								}
							}
						} else {
							/* try to put dotdotsep */
							if (s > 3) {

								*pbuf = CHAR_DOT;
								pbuf++;
								*pbuf = CHAR_DOT;
								pbuf++;
								*pbuf = CHAR_SEP;
								pbuf++;
								s = s - 3;
							} else {
								/* not enough space in buffer */
								/* XXX printf("not enough space for dotdotsep\n"); */
								return(false);
							}
						}

						/* unset dotdot flag */
						dotdot = false;
					}

					/* unset dot flag */
					dot = false;
				} else {
					if (sep == false) {
						/* previous was not a separator so copy it */
						*pbuf = *path;
						pbuf++;
						s--;

						/* set flag */
						sep = true;
					}
				}
				break;

			default :
				if (dot == true) {
					/* copy char in buffer */
					*pbuf = CHAR_DOT;
					pbuf++;
					s--; /* XXX check */

					if (dotdot == true) {
						/* copy char in buffer */
						*pbuf = CHAR_DOT;
						pbuf++;
						s--; /* XXX check */
					}
				}

				/* copy char in buffer */
				*pbuf = *path;
				pbuf++;
				s--;

				/* unset flags */
				sep = false;
				dot = false;
				dotdot = false; /* XXX let this happen ? ("..string" for example) */
		}

		/* next character */
		path++;
	}

	/* if end of string has not been reached */
	if (loop == true) {
		/* XXX printf("s = %d, loop == true\n", s);*/
		return(false);
	}

	switch(pbuf - buffer) {
		case 0 :
			*pbuf = CHAR_DOT;
			pbuf++;
			break;

		case 1 :
			/* don't try to skip trailing separator */
			break;

		default :
			if (*(pbuf - 1) == CHAR_SEP) {
				/* skip trailing character */
				pbuf--;
			}
	}

	/* NULL terminate string */
	*pbuf = CHAR_EOS;

	return(true);
}


/*************
 * relpath() *
 ***********************************************************************
 DESCR
	provide a relative path

 IN
	from : source path
	to : destination path
	buffer : MAXPATHLEN sized buffer that will contain relative path

 OUT
	boolean
 ***********************************************************************/

bool relpath(char *from, char *to, char *buffer) {
	bool	bexit = false;
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
	while (bexit == false) {
		if ((*from == *to) && (*from != CHAR_EOS)) {
			from++;
			to++;
		} else {
			bexit = true;
		}
	}

	/* go back on the last '/' if needed */
	if ((*from != CHAR_EOS) && (*to != CHAR_EOS)) {
		while (*from != CHAR_SEP) {
			from--;
			to--;
		}
	}

	/* ignore leading '/' for to */
	if (*to == CHAR_SEP) {
		to++;
	}

	/* count directories */
	while (*from != CHAR_EOS) {
		if (*from == CHAR_SEP) {
			/* one more */
			if (strlcat_b(buffer, "../", MAXPATHLEN) == false)
				return(false);
		}
		from++;
	}
	/* appending remaining value of "to" */
	if (strlcat_b(buffer, to, MAXPATHLEN) == false)
		return(false);

	if (buffer[0] == CHAR_EOS) {
		/* same path, return "." */
		strlcpy(buffer, ".", MAXPATHLEN); /* should not fail */
	}

	return(true);
}


/*************
 * abspath() *
 ***********************************************************************
 DESCR
	create a "valid" absolute path

 IN
	base : absolute base path
	rel : relative path from the base
	buffer : MAXPATHLEN buffer that will contain resulting absolute path

 OUT
	returns true on success
 ***********************************************************************/

bool abspath(char *base, char *rel, char *buffer) {
	char	tmp_buf[MAXPATHLEN]; /* XXX should be greater ? */

	/* check if 'rel' is not relative */
	if (*rel == CHAR_SEP) {
		return(false);
	}

	strlcpy(tmp_buf, base, sizeof(tmp_buf)); /* no check */
	strlcat(tmp_buf, "/", sizeof(tmp_buf)); /* no check */
	if (strlcat_b(tmp_buf, rel, sizeof(tmp_buf)) == false)
		return(false);

	return(chkpath(tmp_buf, buffer));
}


/**************
 * uabspath() *
 ***********************************************************************
 DESCR
	provide an absolute path from a path with unknown type.

 IN
	base : absolute base path
	upath : path with unknown type (relative or absolute)
	buffer : MAXPATHLEN sized buffer

 OUT
	boolean

 NOTE
	if upath is absolute then buffer is filled with
	upath else the buffer will contain the computed path.
 ***********************************************************************/

bool uabspath(char *base, char *upath, char *buffer) {
	if (*upath == CHAR_SEP) {
		/* upath is absolute */
		return(chkpath(upath, buffer));
	} else {
		/* upath is relative, we can call abspath */
		return(abspath(base, upath, buffer));
	}
}


/**************
 * makepath() *
 ***********************************************************************
 DESCR
	build path if it does not exists

 IN
	path : path to build
	mode : permissions used to create new directories

 OUT
	return : boolean
 ***********************************************************************/

bool makepath(char *path, mode_t mode) {
	bool		 bexit = false;
	char		 save,
				*pstr,
				*copy;
	struct stat	 sb;

	if (*path != CHAR_SEP) {
		/* path is not absolute */
		return(false);
	}

	/* work on a copy */
	copy = strdup(path);
	if (copy == NULL)
		return(false);

	pstr = copy;
	pstr++; /* skip leading separator */
	while (bexit != true) {
		if ((*pstr == CHAR_SEP) || (*pstr == CHAR_EOS)) {
			/* separator found, replacing to make  */
			save = *pstr;
			*pstr = CHAR_EOS;

			/* check if the path already exists */
			if (stat(copy, &sb) != 0) {
				if (mkdir(copy, mode) != 0) {
					free(copy);
					return(false);
				}
			}

			/* put separator back */
			if (save == CHAR_EOS) {
				bexit = true;
			} else {
				*pstr = save;
			}
		}
		pstr++;
	}

	free(copy);
	return(true);
}

