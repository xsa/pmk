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


#include <sys/types.h>
#include <stdlib.h>

#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "prseng.h"


/*#define DEBUG_PRSENG	1*/


/*****************
 * prseng_init() *
 ***********************************************************************
 DESCR
	initialize parsing engine from a file

 IN
	fp :	file descriptor
	data :	extra data to transmit or NULL

 OUT
	parsing engine structure or NULL
 ***********************************************************************/

prseng_t *prseng_init(FILE *fp, void *data) {
	prseng_t	*ppe;
	size_t		 s;

	ppe = (prseng_t *) malloc(sizeof(prseng_t));
	if (ppe != NULL) {
		/* init the buffer */
		fseek(fp, 0, SEEK_SET);
		s = fread((void *) ppe->buf, sizeof(char),
				(sizeof(ppe->buf ) - 1), fp);

		/* on error return NULL */
		if (ferror(fp) != 0) {
			free(ppe);
			return(NULL);
		}

		/* end the string */
		ppe->buf[s] = CHAR_EOS;

		/* extra data that could be needed */
		ppe->data = data;

		/* misc init */
		ppe->fp = fp;
		ppe->eof = false;
		ppe->err = false;
		ppe->str = NULL;
		ppe->cur = ppe->buf;
		ppe->linenum = 1;
		ppe->offset = 0;
	}

	return(ppe);
}


/*********************
 * prseng_init_str() *
 ***********************************************************************
 DESCR
	initialize parsing engine from a string

 IN
	str :	string to parse
	data :	extra data to transmit or NULL

 OUT
	parsing engine structure or NULL
 ***********************************************************************/

prseng_t *prseng_init_str(char *str, void *data) {
	prseng_t	*ppe;

	ppe = (prseng_t *) malloc(sizeof(prseng_t));
	if (ppe != NULL) {
		/* init the buffer */
		strlcpy(ppe->buf, str, sizeof(ppe->buf)); /* no check */

		/* keep track of start of line */
		ppe->str = str;

		/* extra data that could be needed */
		ppe->data = data;

		/* misc init */
		ppe->fp = NULL;
		ppe->eof = true;
		ppe->err = false;
		ppe->cur = ppe->buf;
		ppe->linenum = 1;
		ppe->offset = 0;
	}

	return(ppe);
}


/********************
 * prseng_destroy() *
 ***********************************************************************
 DESCR
	destroy parsing engine structure

 IN
	ppe :	structure to destroy

 OUT
	NONE
 ***********************************************************************/

void prseng_destroy(prseng_t *ppe) {
	free(ppe);
}


/*******************
 * prseng_update() *
 ***********************************************************************
 DESCR
	update parsing engine buffer

 IN
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool prseng_update(prseng_t *ppe) {
	size_t	 s;

	/* if the cursor didn't move ... */
	if (ppe->cur == ppe->buf) {
		/* then skip update */
		return(true);
	}

	/* compute offset */
	ppe->offset = (ppe->cur - ppe->buf) + ppe->offset;

	if (ppe->fp != NULL) {
		/* set the reading pointer at desired offset */
		fseek(ppe->fp, ppe->offset, SEEK_SET);

		if (feof(ppe->fp) != 0) {
			ppe->eof = true;
		}

		/* fill buffer with size - 1 characters */
		s = fread((void *) ppe->buf, sizeof(char),
				(sizeof(ppe->buf ) - 1), ppe->fp);

		/* ferror ? */
		if (ferror(ppe->fp) != 0) {
			ppe->err = true;
			return(false);
		}

		/* put EOS right after the last read char */
		ppe->buf[s] = CHAR_EOS;

		/* check if end of file has been reached */
		if (feof(ppe->fp) != 0) {
			ppe->eof = true;
		}
	} else {
		strlcpy(ppe->buf, (ppe->str + ppe->offset),
				sizeof(ppe->buf)); /* no check ? */
	}

	/* rewind cursor to the beginning of the buffer */
	ppe->cur = ppe->buf;

	/* buffer filled, return pointer to the first character */
	return(true);
}


/****************
 * prseng_eof() *
 ***********************************************************************
 DESCR
	check for end of file

 IN
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool prseng_eof(prseng_t *ppe) {
	if (ppe->err == true) {
		/* file error, consider it as an end of file */
		return(true);
	}

	/* not reached end of file neither get end of string */
	if ((ppe->eof == false) || (*ppe->cur != '\0')) {
		return(false);
	}

	/* end of file and buffer */
	return(true);
}


/*********************
 * prseng_get_char() *
 ***********************************************************************
 DESCR
	get character of parsing cursor

 IN
	ppe :	parsing engine structure

 OUT
	character
 ***********************************************************************/

char prseng_get_char(prseng_t *ppe) {
	/* return character at the current cursor position */
	return(*ppe->cur);
}


/**********************
 * prseng_next_char() *
 ***********************************************************************
 DESCR
	set parsing cursor to next character

 IN
	ppe :	parsing engine structure

 OUT
	boolean
 ***********************************************************************/

bool prseng_next_char(prseng_t *ppe) {
	if (*ppe->cur == '\n') {
#ifdef DEBUG_PRSENG
	debugf("prseng_next_char() : new line = %d", ppe->linenum);
#endif
		ppe->linenum++;
	}

	/* set the cursor to the next char */
	ppe->cur++;

#ifdef DEBUG_PRSENG
	debugf("prseng_next_char() : new char = '%c'.", *ppe->cur);
#endif

	/* if end of string try to update buffer */
	if (*ppe->cur == CHAR_EOS) {
		if (prseng_update(ppe) == false) {
			return(false);
		}
	}


	return(true);
}


/**********************
 * prseng_test_char() *
 ***********************************************************************
 DESCR
	test character at the parsing cursor position

 IN
	ppe :	parsing engine structure
	c :		char to compare with

 OUT
	boolean
 ***********************************************************************/

bool prseng_test_char(prseng_t *ppe, char c) {
	if (*ppe->cur == c) {
		return(true);
	}

	return(false);
}


/***************************
 * prseng_test_idtf_char() *
 ***********************************************************************
 DESCR
	test if character at the parsing cursor is an identifier character
	contained in the given string

 IN
	pstr :	string of identifier characters
	c :		character to test

 OUT
	boolean
 TODO
	XXX do it directly with ppe ????
 ***********************************************************************/

bool prseng_test_idtf_char(char *pstr, char c) {
	/* parse the allowed characters string */
	while (*pstr != CHAR_EOS) {
		if (*pstr == c) {
			/* character matches */
			return(true);
		}

		/* next character */
		pstr++;
	}

	/* not found */
	return(false);
}


/*********************
 * prseng_get_idtf() *
 ***********************************************************************
 DESCR
	get an identifier from the actual parsing cursor position

 IN
	ppe :	parsing engine structure
	pbuf :	identifier storage buffer
	s :		size of buffer
	pstr :	identifier allowed characters

 OUT
	boolean
 ***********************************************************************/

bool prseng_get_idtf(prseng_t *ppe, char *pbuf, size_t s, char *pstr) {
	/* leave space for the end of string character */
	s--;

	/* loop while we have allowed char and enough place in the buffer */
	while ((prseng_test_idtf_char(pstr, *ppe->cur) == true) && (s > 0)) {
		/* copy character */
		*pbuf = *ppe->cur;

		/* update buffer cursor */
		pbuf++;
		s--;

		/* next character */
		if (prseng_next_char(ppe) == false) {
			*pbuf = CHAR_EOS;
			return(false);
		}
	}

	*pbuf = CHAR_EOS;
/*
	if (prseng_test_idtf_char(pstr, *ppe->cur) == true) {
		return(false);
	}
*/
	return(true);
}

