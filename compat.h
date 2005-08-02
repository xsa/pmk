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


#ifndef _PMK_COMPAT_H_
#define _PMK_COMPAT_H_

#include <stdarg.h>
#include "compat/pmk_string.h"

/**********************************
 * constants and type definitions *
 ***********************************************************************/

enum {
	PARSE_CHAR,
	PARSE_FLAGS,		/* +,-, ,#,0 */
	PARSE_FLD_WIDTH,
	PARSE_DOT,
	PARSE_PRECISION,
	PARSE_LEN_MDFR,		/* hh,h,l,ll,j,z,t,L */
	PARSE_CONV_SPEC		/* d,i,o,u,x,X,f,F,e,E,g,G,a,A,c,s,p,n,% */
};

enum {
	MDFR_NORMAL,
	MDFR_CHAR,
	MDFR_SHORT,
	MDFR_LONG,
	MDFR_LONG_LONG,
	MDFR_INTMAX,
	MDFR_SIZET,
	MDFR_PTRDIFF,
	MDFR_LONG_DBL
};

/* conversion flag masks */
#define	FLAG_NONE				0x0000
#define	FLAG_SIGNED				0x0001
#define	FLAG_LEFT_JUSTIFIED		0x0002
#define	FLAG_SPACE_PREFIX		0x0004
#define	FLAG_ALTERNATIVE_FORM	0x0008
#define	FLAG_ZERO_PADDING		0x0010
#define	FLAG_PRECISION			0x0020
#define	FLAG_UPPERCASE			0x0040
#define	FLAG_EXPONENT			0x0080
#define	FLAG_G_CONVERSION		0x0100
#define FLAG_ALL				0xffff

/* base definition */
#define BASE_OCT	8
#define BASE_DEC	10
#define BASE_HEX	16


#ifdef HAVE_LONG_LONG
	typedef long long	signed_t;
#else /* HAVE_LONG_LONG */
	typedef long		signed_t;
#endif /* HAVE_LONG_LONG */

#ifdef HAVE_UNSIGNED_LONG_LONG
	typedef unsigned long long	unsigned_t;
#else /* HAVE_UNSIGNED_LONG_LONG */
	typedef unsigned long		unsigned_t;
#endif /* HAVE_UNSIGNED_LONG_LONG */

#ifdef HAVE_LONG_DOUBLE
	typedef long double	float_t;
#else /* HAVE_LONG_DOUBLE */
	typedef double		float_t;
#endif /* HAVE_LONG_DOUBLE */


typedef unsigned char	base_t;
typedef unsigned int	flag_t;

#ifdef HAVE_UNSIGNED_LONG_LONG
#define BYTESIZE	sizeof(unsigned long long)
#else /* HAVE_UNSIGNED_LONG_LONG */
#define BYTESIZE	sizeof(unsigned long)
#endif /* HAVE_UNSIGNED_LONG_LONG */

#define UPPER_BASE	"0123456789ABCDEF"
#define LOWER_BASE	"0123456789abcdef"


/*
	compute approximative string size needed for maximum int conversion

	verified with values up to 2^128, 16 bytes values.
*/
#define MAXINTLEN	((BYTESIZE * 3) - (BYTESIZE / 2))

#define MKSTEMPS_REPLACE_CHAR	'X'


/**************
 * prototypes *
 ***********************************************************************/

int		 pmk_vsnprintf(char *, size_t, const char *, va_list);
size_t	 pmk_strlcpy(char *, const char *, size_t);
size_t	 pmk_strlcat(char *, const char *, size_t);
char	*pmk_dirname(char *);
char	*pmk_basename(char *);
int		 pmk_isblank(int);
int		 pmk_mkstemps(char *, int);

bool	 snprintf_b(char *, size_t, const char *, ...);
bool	 strlcat_b(char *, const char *, size_t);
bool	 strlcpy_b(char *, const char *, size_t);

#endif /* _PMK_COMPAT_H_ */

