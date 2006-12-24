/* $Id$ */

/*
* Copyright (c) 2003-2006 Damien Couderc
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


#ifndef _PREMAKE_H_
#define _PREMAKE_H_

#include "compat/pmk_stdbool.h"
#include "errmsg.h"
#include "pmk_obj.h"

#include <sys/param.h>
#ifndef MAXPATHLEN
#	define MAXPATHLEN 512
#endif

#ifndef TRUE
#	define TRUE	1
#	define FALSE	0
#endif

#ifndef SYSCONFDIR
#	define SYSCONFDIR	"/etc"
#endif

#ifndef CONFDIR
#	define CONFDIR	SYSCONFDIR "/pmk"
#endif

#ifndef TMPDIR
#	define TMPDIR	"."
#endif

#ifndef TMPBINDIR
#	define TMPBINDIR	"."
#endif

/* major version */
#define PREMAKE_VMAJOR	"0"
/* minor version */
#define PREMAKE_VMINOR	"10"
/* minor subversion, only used for bugfixes  */
#define PREMAKE_VSUB	"1"

/* full version */
#ifndef PREMAKE_VSUB
#define PREMAKE_VFULL	PREMAKE_VMAJOR "." PREMAKE_VMINOR
#else
#define PREMAKE_VFULL	PREMAKE_VMAJOR "." PREMAKE_VMINOR "." PREMAKE_VSUB
#endif

/* only used for snapshots, comment for release */
/*#define PREMAKE_SNAP		"20061028"*/

/* build version string */
#ifndef PREMAKE_SNAP
#define PREMAKE_VERSION		PREMAKE_VFULL
#else
#define PREMAKE_VERSION		"SNAP" PREMAKE_SNAP
#endif

#define PREMAKE_CONFIG		"pmk.conf"
#define PREMAKE_FILENAME	"pmkfile"
#define PREMAKE_PKGFILE		"pkgfile"

#define PREMAKE_CONFIG_PATH	CONFDIR "/" PREMAKE_CONFIG
#define PREMAKE_CONFIG_PATH_BAK	PREMAKE_CONFIG_PATH ".bak"

#define PREMAKE_TMP_DIR		TMPDIR

/*
	pmk.conf keys
*/

/* key names (see also pmksetup.h) */
#define PMKCONF_BIN_AR			"BIN_AR"
#define PMKCONF_BIN_AS			"BIN_AS"
#define PMKCONF_BIN_AWK			"BIN_AWK"
#define PMKCONF_BIN_CAT			"BIN_CAT"
#define PMKCONF_BIN_C89			"BIN_C89"
#define PMKCONF_BIN_C99			"BIN_C99"
#define PMKCONF_BIN_CC			"BIN_CC"
#define PMKCONF_BIN_CPP			"BIN_CPP"
#define PMKCONF_BIN_CXX			"BIN_CXX"
#define PMKCONF_BIN_ECHO		"BIN_ECHO"
#define PMKCONF_BIN_EGREP		"BIN_EGREP"
#define PMKCONF_BIN_GREP		"BIN_GREP"
#define PMKCONF_BIN_ID			"BIN_ID"
#define PMKCONF_BIN_INSTALL		"BIN_INSTALL"
#define PMKCONF_BIN_LEX			"BIN_LEX"
#define PMKCONF_BIN_LN			"BIN_LN"
#define PMKCONF_BIN_PKGCONFIG	"BIN_PKGCONFIG"
#define PMKCONF_BIN_RANLIB		"BIN_RANLIB"
#define PMKCONF_BIN_SH			"BIN_SH"
#define PMKCONF_BIN_STRIP		"BIN_STRIP"
#define PMKCONF_BIN_SUDO		"BIN_SUDO"
#define PMKCONF_BIN_TAR			"BIN_TAR"
#define PMKCONF_BIN_YACC		"BIN_YACC"

#define PMKCONF_HW_CPU_ARCH		"HW_CPU_ARCH"
#define PMKCONF_HW_BYTEORDER	"HW_BYTEORDER"

#define PMKCONF_PATH_BIN	"PATH_BIN"
#define PMKCONF_PATH_INC	"PATH_INC"
#define PMKCONF_PATH_LIB	"PATH_LIB"

#define PMKCONF_PC_PATH_LIB	"PC_PATH_LIB"

#define PMKCONF_OS_NAME		"OS_NAME"
#define PMKCONF_OS_VERS		"OS_VERSION"
#define PMKCONF_OS_ARCH		"OS_ARCH"

#define PMKCONF_MISC_SYSCONFDIR	"SYSCONFDIR"
#define PMKCONF_MISC_PREFIX		"PREFIX"

#define PMKCONF_AC_ECHO_N	"AC_ECHO_N"
#define PMKCONF_AC_ECHO_C	"AC_ECHO_C"
#define PMKCONF_AC_ECHO_T	"AC_ECHO_T"


/* specific values */
#define PMKVAL_BIN_PKGCONFIG	"pkg-config"
#define PMKVAL_LIB_PKGCONFIG	"/lib/pkgconfig" /* preceeded by PREFIX */

#define PMKVAL_ENV_LIBS		"LIBS"
#define PMKVAL_ENV_CFLAGS	"CFLAGS"

/* prefix character used for comments */
#define CHAR_COMMENT		'#'

/* end of string */
#define CHAR_EOS		'\0'

/* carriage return */
#define CHAR_CR                 '\n'

/* pmk.conf separators */
#define CHAR_ASSIGN_UPDATE	'='
#define CHAR_ASSIGN_STATIC	':'
#define CHAR_LIST_SEPARATOR	','

/* misc boolean stuff */
#define BOOL_STRING_TRUE	"TRUE"
#define BOOL_STRING_FALSE	"FALSE"

/* hw values */
#define HW_ENDIAN_BIG		"BIG_ENDIAN"
#define HW_ENDIAN_LITTLE	"LITTLE_ENDIAN"
#define HW_ENDIAN_UNKNOWN	"UNKNOWN_ENDIAN"

/* make specific variable names */
#define MK_VAR_CFLAGS		"CFLAGS"
#define MK_VAR_CXXFLAGS		"CXXFLAGS"
#define MK_VAR_CPPFLAGS		"CPPFLAGS"
#define MK_VAR_LDFLAGS		"LDFLAGS"
#define MK_VAR_CLDFLAGS		"CLDFLAGS"
#define MK_VAR_CXXLDFLAGS	"CXXLDFLAGS"
#define MK_VAR_LIBS			"LIBS"
#define MK_VAR_DEBUG		"DEBUG"
#define MK_VAR_SL_BUILD		"SHARED_LIB_TARGETS"
#define MK_VAR_SL_CLEAN		"SHLIB_CLEAN_TARGETS"
#define MK_VAR_SL_INST		"SHLIB_INST_TARGETS"
#define MK_VAR_SL_DEINST	"SHLIB_DEINST_TARGETS"

/* maximal size of an error message */
#define MAX_ERR_MSG_LEN		256

/* maximal size of a command name */
#define CMD_LEN			32

/* maximal size of a command label */
#define LABEL_LEN		64

/* maximal sizes for command pair of option (name and value) */
#define OPT_NAME_LEN		64
#define OPT_VALUE_LEN		MAXPATHLEN /* can contain a path */

/* maximal size of a line */
#define MAX_LINE_LEN		MAXPATHLEN + OPT_NAME_LEN

/* maximal number of key in data hash */
#define MAX_DATA_KEY		1024

/* size for temporary buffers */
#define TMP_BUF_LEN		512

/* string delimiter PATH variables */
#define PATH_STR_DELIMITER	':'

#endif /* _PREMAKE_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
