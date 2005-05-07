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


#ifndef _PMK_STRING_H_
#define _PMK_STRING_H_

#include <string.h>

#include "config.h"
#include "pmk_stdbool.h"


#ifndef HAVE_STRLCPY

/*
	strlcpy() function

	Systems known to have compatible strlcpy in string.h :
		OpenBSD		since 2.4
		FreeBSD		since 3.3
		NetBSD		since 1.5
		SCO OpenServer	since 5.0.6
		SunOs		since 5.8
		MacOSX

	Systems without strlcpy in string.h :
		AIX		(4.3)
		glibc based	(at least before 4 Mar 2002)

	Note: Feel free to send us a notice if your system is not
		listed here but should be.
*/

size_t strlcpy(char *, const char *, size_t);

#endif /* HAVE_STRLCPY */


#ifndef HAVE_STRLCAT

/*
	strlcat() function
*/

size_t strlcat(char *, const char *, size_t);

#endif /* HAVE_STRLCAT */


bool	snprintf_b(char *, size_t, const char *, ...);
bool	strlcat_b(char *, const char *, size_t);
bool	strlcpy_b(char *, const char *, size_t);

#endif /* _PMK_STRING_H_ */

