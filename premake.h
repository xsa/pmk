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


#ifndef _PREMAKE_H_
#define _PREMAKE_H_

#include "compat/pmk_stdbool.h"
#include "pmk_obj.h"

/* We include sys/param.h to get MAXPATHLEN.
   This is known to work under following operanding systems :
	OpenBSD
	FreeBSD
	NetBSD
	MACOSX
	Solaris			(not verified)
	SunOS
	HPUX
	AIX			(not verified)
	IRIX			(not verified)
	OSF1
	Ultrix			(not verified)
	Linux based systems	(Debian, Mandrake, RedHat, Slackware, Suse)
	DG-UX			(not verified)
	4.4BSD			(not verified)

   Some systems does not provide the same location :
   	Chorus			arpa/ftp.h


   Comments about this stuff is welcome. If your system is not
   supported then take contact with us to fix it.
*/
#include <sys/param.h>
#ifndef MAXPATHLEN
#	define MAXPATHLEN 512
#endif

#ifndef TRUE
#	define TRUE	1
#	define FALSE	0
#endif


#define PREMAKE_MAJOR		"0"
#define PREMAKE_MINOR		"6"
#define PREMAKE_SNAP		"4" /* only used for snapshots */
#define PREMAKE_VERSION		PREMAKE_MAJOR "." PREMAKE_MINOR

#define PREMAKE_FILENAME	"pmkfile"
#define PREMAKE_CONFIG		"pmk.conf"
#define PREMAKE_LOG		"pmk.log"

#define PREMAKE_CONFIG_PATH	CONFDIR "/" PREMAKE_CONFIG 

/*
	pmk.conf keys
*/

/* OBSOLETE */
#define PREMAKE_KEY_BINPATH	"BIN_PATH"
#define PREMAKE_KEY_INCPATH	"INC_PATH"
#define PREMAKE_KEY_LIBPATH	"LIB_PATH"

/* new key names */
#define PMKCONF_BIN_AR		"BIN_AR"
#define PMKCONF_BIN_CAT		"BIN_CAT"
#define PMKCONF_BIN_CXX		"BIN_CXX"
#define PMKCONF_BIN_GREP	"BIN_GREP"
#define PMKCONF_BIN_INSTALL	"BIN_INSTALL"
#define PMKCONF_BIN_RANLIB	"BIN_RANLIB"
#define PMKCONF_BIN_PKGCONFIG	"BIN_PKGCONFIG"

#define PMKCONF_HW_BYTEORDER	"HW_BYTEORDER"

#define PMKCONF_PATH_BIN	"PATH_BIN"
#define PMKCONF_PATH_INC	"PATH_INC"
#define PMKCONF_PATH_LIB	"PATH_LIB"

#define PMKCONF_OS_NAME		"OS_NAME"
#define PMKCONF_OS_VERS		"OS_VERSION"
#define PMKCONF_OS_ARCH		"OS_ARCH"

#define PMKCONF_MISC_SYSCONFDIR	"SYSCONFDIR"
#define PMKCONF_MISC_PREFIX	"PREFIX"


/* prefix character used for comments */
#define CHAR_COMMENT		'#'

/* end of string */
#define CHAR_EOS		'\0'

/* pmk.conf separators */
#define CHAR_ASSIGN_UPDATE	'='
#define CHAR_ASSIGN_STATIC	':'
#define CHAR_LIST_SEPARATOR	','

/* hw values */
#define HW_ENDIAN_BIG		"BIG_ENDIAN"
#define HW_ENDIAN_LITTLE	"LITTLE_ENDIAN"
#define HW_ENDIAN_UNKNOW	"UNKNOW_ENDIAN"

/* maximal size of an error message */
#define MAX_ERR_MSG_LEN		256

/* maximal size of a command name */
#define CMD_LEN		32

/* maximal size of a command label */
#define LABEL_LEN	64

/* maximal sizes for command pair of option (name and value) */
#define OPT_NAME_LEN	64
#define OPT_VALUE_LEN	MAXPATHLEN /* can contain a path */

/* maximal size of a line */
#define MAX_LINE_LEN		MAXPATHLEN + OPT_NAME_LEN

/* size for temporary buffers */
#define TMP_BUF_LEN		512


#endif /* _PREMAKE_H_ */
