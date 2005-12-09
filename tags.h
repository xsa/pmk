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


#ifndef _PMK_TAGS_H_
#define _PMK_TAGS_H_

/*************
 * constants *
 **********************************************************************************************/

/* pmk tags related constants *****************************************************************/

/* tag types */
enum {
	TAG_TYPE_UNKNOWN = 0,
	TAG_TYPE_BIN,			/* binary */
	TAG_TYPE_CFGTOOL,		/* config tool binary (*-config) */
	TAG_TYPE_HDR,			/* header */
	TAG_TYPE_HDR_PRC,		/* header procedure */
	TAG_TYPE_HDR_MCR,		/* header macro */
	TAG_TYPE_LIB,			/* library */
	TAG_TYPE_LIB_PRC,		/* library procedure */
	TAG_TYPE_PKGCFG,		/* pkg-config module */
	TAG_TYPE_TYPE,			/* type */
	TAG_TYPE_TYP_MBR,		/* type member */
	TAG_TYPE_HDR_TYPE,		/* header type */
	TAG_TYPE_HDR_TYP_MBR,	/* header type member */
	/* XXX TODO config and pkgconfir stuff */
};

/* tag format strings */
#define FMT_TAG_BIN		"BIN_%s"			/* binary tag */
#define FMT_TAG_CFGTL	"CFGTOOL_%s"		/* config tool binary */
#define FMT_TAG_HDR		"HDR_%s"			/* header tag */
#define FMT_TAG_HPRC	"HPROC_%s_%s"		/* header procedure tag */
#define FMT_TAG_HMCR	"HMACRO_%s_%s"		/* header macro tag */
#define FMT_TAG_LIB		"LIB_%s"			/* library tag */
#define FMT_TAG_LPROC	"LPROC_%s_%s"		/* library procedure tag */
#define FMT_TAG_PCFG	"PKGCFG_%s"			/* pkg-config module tag */
#define FMT_TAG_TYPE	"TYPE_%s"			/* type tag */
#define FMT_TAG_TMBR	"TMEMB_%s_%s"		/* type member tag */
#define FMT_TAG_HTYPE	"HTYPE_%s_%s"		/* header type tag */
#define FMT_TAG_HTMBR	"HTMEMB_%s_%s_%s"	/* header type member tag */

/* tag prefixes */
#define TAG_PREFIX			"TAG_"		/* advanced tag prefix */
#define TAG_DEF_PREFIX		"TAGDEF_"	/* advanced tag definition prefix */
#define AC_TAG_PREFIX		"HAVE_"		/* autoconf-like tag prefix */
#define BASIC_DEF_PREFIX	"DEF__"		/* basic tag definition format */

/* maximum length of a tag name */
#define MAX_TAG_LEN		128

/* allowed characters in a tag name */
#define TAG_IDTF_CHAR	"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"


/**************
 * prototypes *
 ***********************************************************************/

char	*conv_to_tag(char *);
char	*gen_tag(int, char *, char *, char *);
char	*gen_tag_name(int, char *, char *, char *);
char	*gen_tag_def(int, char *, char *, char *);
char	*gen_ac_tag_name(char *);
char	*gen_basic_tag_def(char *);
char	*gen_from_tmpl(char *);

#endif /* _PMK_TAGS_H_ */

