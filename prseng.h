/* $Id$ */

/*
 * Copyright (c) 2005 Damien Couderc
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


#ifndef _PRSENG_H_
#define _PRSENG_H_

#include "compat/pmk_stdbool.h"

/***********
 constants
***********************************************************************/

#define CHAR_EOS		'\0'

/* parsing engine buffer length */
#define PRSENG_BUF_LEN	512


/*********************
 types and structures
***********************************************************************/

typedef struct {
	FILE			*fp;					/* file structure pointer */
	bool			 eof,					/* end of file flag */
					 err;					/* error flag */
	char			*str,					/* string to parse (prseng_init_str) */
					*cur,					/* parsing cursor */
					 buf[PRSENG_BUF_LEN];	/* buffer window */
	unsigned int	 linenum;				/* current line */
	unsigned long	 offset;				/* offset of the buffer window */
	void			*data;
} prseng_t;


/********************
 function prototypes
***********************************************************************/

prseng_t		*prseng_init(FILE *, void *);
prseng_t		*prseng_init_str(char *, void *);
void			 prseng_destroy(prseng_t *);

bool			 prseng_update(prseng_t *);
bool			 prseng_eof(prseng_t *);

char			 prseng_get_char(prseng_t *);
bool			 prseng_next_char(prseng_t *);
bool			 prseng_test_char(prseng_t *, char);

bool			 prseng_test_idtf_char(char *, char);
bool			 prseng_get_idtf(prseng_t *, char *, size_t, char *);


#endif /* _PRSENG_H_ */

