/* $Id$ */

/*
 * Copyright (c) 2003 Damien Couderc
 * Copyright (c) 2004 Martin Reindl
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

/********************************
 *                              *
 *    ISO C99 boolean type      *
 *                              *
 ********************************/


#ifndef _STDBOOL_H_
#define _STDBOOL_H_

/*
	Sun Forte 5.4 C compiler and previous versions give out a
	"warning: _Bool is a keyword in ISO C99"
	message when we use typedef_. In addition, every Sun Pro C compiler
	which conforms to ANSI C (and therefore sets __STDC__ to 1) shows the
	same behaviour. We avoid this by defining _Bool in a macro in these
	cases.
*/

#ifndef HAVE__BOOL
#	if defined __SUNPRO_C && (__SUNPRO_C < 0x550 || __STDC__ == 1)
#		define _Bool unsigned char
#	else
typedef unsigned char _Bool;
#	endif
#endif

#define bool _Bool
#define true    1
#define false   0
#define __bool_true_false_are_defined 1


#endif /* _STDBOOL_H_ */
