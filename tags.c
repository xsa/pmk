/* $Id$ */

/*
 * Copyright (c) 2006 Damien Couderc
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


#include <limits.h>
#include <stddef.h>

#include "compat/pmk_ctype.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "tags.h"


/*****************
 * conv_to_tag() *
 ***********************************************************************
 DESCR
 	convert a string to a tag

 IN
	str :		string to convert

 OUT
	pointer to the tag buffer

 TODO
	check if size of buffer has been reached
 ***********************************************************************/

char *conv_to_tag(char *str) {
	static char	 buffer[MAX_TAG_LEN];
	char		*pbuf;
	size_t		 s = MAX_TAG_LEN;

	pbuf = buffer;

	/* process the given string */
	while ((*str != '\0') && (s > 1)) {
		/* if we got an asterisk (pointer character) */
		if (*str == '*') {
			/* then replace by P */
			*pbuf = 'P';
		} else {
			/* check if we have an alphanumeric character */
			if (isalnum((int) *str) == 0) {
				/* no, replace by an underscore */
				*pbuf = '_';
			} else {
				/* yes, convert to uppercase if needed */
				*pbuf = (char) toupper((int) *str);
			}
		}

		/* next character */
		s--;
		pbuf++;
		str++;
	}

	*pbuf = '\0';

	return(buffer);
}


/*************
 * gen_tag() *
 ***********************************************************************
 DESCR
 	generate a tag suffix

 IN
	type :		tag type
	container :	container name
	content :	content name
	misc :		misc content

 OUT
	pointer to the tag suffix buffer
 ***********************************************************************/

char *gen_tag(int type, char *container, char *content, char *misc) {
	static char	 buffer[MAX_TAG_LEN];
	char		cnt[MAX_TAG_LEN],
				ctt[MAX_TAG_LEN],
				msc[MAX_TAG_LEN];

	/* set data pointers */
	switch (type) {
		case TAG_TYPE_HDR_TYP_MBR :
			strlcpy(msc, conv_to_tag(misc), sizeof(msc));
			/* no break */

		case TAG_TYPE_HDR_PRC :
		case TAG_TYPE_HDR_MCR :
		case TAG_TYPE_LIB_PRC :
		case TAG_TYPE_TYP_MBR :
		case TAG_TYPE_HDR_TYPE :
			strlcpy(ctt, conv_to_tag(content), sizeof(ctt));
			/* no break */

		case TAG_TYPE_BIN :
		case TAG_TYPE_CFGTOOL :
		case TAG_TYPE_HDR :
		case TAG_TYPE_LIB :
		case TAG_TYPE_PKGCFG :
		case TAG_TYPE_TYPE :
			strlcpy(cnt, conv_to_tag(container), sizeof(cnt));
			break;
	}

	/* generate tag from format string */
	switch (type) {
		case TAG_TYPE_BIN :
			snprintf(buffer, sizeof(buffer), FMT_TAG_BIN, cnt);
			break;

		case TAG_TYPE_CFGTOOL :
			snprintf(buffer, sizeof(buffer), FMT_TAG_CFGTL, cnt);
			break;

		case TAG_TYPE_HDR :
			snprintf(buffer, sizeof(buffer), FMT_TAG_HDR, cnt);
			break;

		case TAG_TYPE_HDR_PRC :
			snprintf(buffer, sizeof(buffer), FMT_TAG_HPRC, cnt, ctt);
			break;

		case TAG_TYPE_HDR_MCR :
			snprintf(buffer, sizeof(buffer), FMT_TAG_HMCR, cnt, ctt);
			break;

		case TAG_TYPE_LIB :
			snprintf(buffer, sizeof(buffer), FMT_TAG_LIB, cnt);
			break;

		case TAG_TYPE_LIB_PRC :
			snprintf(buffer, sizeof(buffer), FMT_TAG_LPROC, cnt, ctt);
			break;

		case TAG_TYPE_PKGCFG :
			snprintf(buffer, sizeof(buffer), FMT_TAG_PCFG, cnt);
			break;

		case TAG_TYPE_TYPE :
			snprintf(buffer, sizeof(buffer), FMT_TAG_TYPE, cnt);
			break;

		case TAG_TYPE_TYP_MBR :
			snprintf(buffer, sizeof(buffer), FMT_TAG_TMBR, cnt, ctt);
			break;

		case TAG_TYPE_HDR_TYPE :
			snprintf(buffer, sizeof(buffer), FMT_TAG_HTYPE, cnt, ctt);
			break;

		case TAG_TYPE_HDR_TYP_MBR :
			snprintf(buffer, sizeof(buffer), FMT_TAG_HTMBR, cnt, ctt, msc);
			break;

		default :
			return(NULL);
	}

	return(buffer);
}


/******************
 * gen_tag_name() *
 ***********************************************************************
 DESCR
	generate a tag name

 IN
	type :		tag type
	container :	container name
	content :	content name
	misc :		misc content

 OUT
	pointer to the tag name buffer
 ***********************************************************************/

char *gen_tag_name(int type, char *container, char *content, char *misc) {
	static char	 buffer[MAX_TAG_LEN];
	char		*tag;

	/* generate tag suffix */
	tag = gen_tag(type, container, content, misc);

	/* concatenate prefix and converted name */
	strlcpy(buffer, TAG_PREFIX, sizeof(buffer));
	strlcat(buffer, tag, sizeof(buffer));

	return(buffer);
}


/*****************
 * gen_tag_def() *
 ***********************************************************************
 DESCR
	generate a tag definition name

 IN
	type :		tag type
	container :	container name
	content :	content name
	misc :		misc content

 OUT
	pointer to the tag definition name buffer
 ***********************************************************************/

char *gen_tag_def(int type, char *container, char *content, char *misc) {
	static char	 buffer[MAX_TAG_LEN];
	char		*tag;

	/* generate tag suffix */
	tag = gen_tag(type, container, content, misc);

	/* concatenate prefix and converted name */
	strlcpy(buffer, TAG_DEF_PREFIX, sizeof(buffer));
	strlcat(buffer, tag, sizeof(buffer));

	return(buffer);
}


/*********************
 * gen_ac_tag_name() *
 ***********************************************************************
 DESCR
	generate an autoconf-like tag name

 IN
	content :	content name

 OUT
	pointer to the tag name buffer
 ***********************************************************************/

char *gen_ac_tag_name(char *content) {
	static char	 buffer[MAX_TAG_LEN];
	char		*tag;

	/* convert name to tag */
	tag = conv_to_tag(content);

	/* concatenate prefix and converted name */
	strlcpy(buffer, AC_TAG_PREFIX, sizeof(buffer));
	strlcat(buffer, tag, sizeof(buffer));

	return(buffer);
}


/***********************
 * gen_basic_tag_def() *
 ***********************************************************************
 DESCR
	generate an autoconf-like tag name

 IN
	content :	content name

 OUT
	pointer to the tag name buffer
 ***********************************************************************/

char *gen_basic_tag_def(char *content) {
	static char	 buffer[MAX_TAG_LEN];
	char		*tag;

	/* convert name to tag */
	tag = conv_to_tag(content);

	/* concatenate prefix and converted name */
	strlcpy(buffer, BASIC_DEF_PREFIX, sizeof(buffer));
	strlcat(buffer, tag, sizeof(buffer));

	return(buffer);
}


/*******************
 * gen_from_tmpl() *
 ***********************************************************************
 DESCR
	generate a file name from a template name

 IN
	template :	template file name

 OUT
	pointer to the file name buffer
 ***********************************************************************/

char *gen_from_tmpl(char *template) {
	static char	 buffer[PATH_MAX];
	char		*pstr;

	/* copy template into buffer */
	strlcpy(buffer, template, sizeof(buffer));

	/* find and remove the last suffix */
	pstr = strrchr(buffer, '.');
	if (pstr != NULL) {
		*pstr = '\0';
	}

	return(buffer);
}

