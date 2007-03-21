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

#ifndef _PMKSCAN_H_
#define _PMKSCAN_H_

#include "dynarray.h"
#include "hash.h"
#include "lang.h"
#include "parse_lang.h"

/*************
 * constants *
 ***********************************************************************/

/* pmkscan specific version */
#define PREMAKE_SUBVER_PMKSCAN	"6"

#ifndef DATADIR
/* for lint */
#define DATADIR	"/DATADIR_not_defined"
#endif

#define PMKSCAN_DATA	DATADIR "/pmkscan.dat"
#define PMKSCAN_CFGFILE	"config.h.pmk"
#define PMKSCAN_PMKFILE	"pmkfile"
#define PMKSCAN_MKFILE	"Makefile.pmk"
#define PMKSCAN_CONFIG	"scanfile"

/* languages, should use lgdata later */
#define PMKSCAN_LANG_C		LANG_LABEL_C
#define PMKSCAN_LANG_CXX	LANG_LABEL_CXX

/* label language strings */
#define PMKSCAN_LABEL_C		"c_"
#define PMKSCAN_LABEL_CXX	"cxx_"

#define PSC_MAIN_C		"main"

/* parser tokens *******************************************************/

/* pmkscan data */
enum {
	PSC_TOK_FUNC = 1,
	PSC_TOK_INCL,
	PSC_TOK_TYPE,
	PSC_TOK_ADDHDR,
	PSC_TOK_ADDLIB,
	PSC_TOK_ADDTYP
};

/* pmkscan script */
enum {
	PSC_TOK_PMKF = 1,
	PSC_TOK_MAKF,
	PSC_TOK_ZONE,
	PSC_TOK_DEFLIB,
	PSC_TOK_DEFLIBS,
	PSC_TOK_SETLIB
};


/* command keywords */
#define KW_CMD_ADDHDR	"ADD_HEADER"
#define KW_CMD_ADDLIB	"ADD_LIBRARY"
#define KW_CMD_ADDTYP	"ADD_TYPE"

/* command keyword options */
#define KW_OPT_HDR		"HEADER"
#define KW_OPT_MBR		"MEMBER"
#define KW_OPT_PRC		"PROCEDURE"
#define KW_OPT_SUB		"SUBHDR"

/* script keywords */
#define KW_CMD_GENPF	"GEN_PMKFILE"
#define KW_CMD_GENMF	"GEN_MAKEFILE"
#define KW_CMD_GENZN	"GEN_ZONE"
#define KW_CMD_DEFLIB	"DEFINE_LIB"

/* script keyword options */
#define KW_OPT_ADVTAG	"ADVTAG"
#define KW_OPT_CFGALT	"CFGNAME"
#define KW_OPT_DIR		"DIRECTORY"
#define KW_OPT_DSC		"DISCARD"
#define KW_OPT_EXTMKF	"EXTRAMKF"
#define KW_OPT_EXTTAG	"EXTRATAG"
#define KW_OPT_LIBOBJ	"LIBOBJ"
#define KW_OPT_LINKER	"LINKER"
#define KW_OPT_MKF		"MAKEFILE"
#define KW_OPT_MKFALT	"MKFNAME"
#define KW_OPT_PMK		"PMKFILE"
#define KW_OPT_PMKALT	"PMKNAME"
#define KW_OPT_REC		"RECURSE"
#define KW_OPT_SRCS		"SOURCES"
#define KW_OPT_HDRS		"HEADERS"
#define KW_OPT_UNI		"UNIQUE"
#define KW_OPT_VMAJ		"VMAJ"
#define KW_OPT_VMIN		"VMIN"


/* common options */
#define KW_OPT_LIB		"LIBRARY"
#define KW_OPT_NAM		"NAME"

/* file types **********************************************************/
enum {
	FILE_TYPE_UNKNOWN = 0,
	FILE_TYPE_ASM,
	FILE_TYPE_C,
	FILE_TYPE_CXX,
	FILE_TYPE_DATA,
	FILE_TYPE_IMG,
	FILE_TYPE_HTML,
	FILE_TYPE_LEX,
	/*
		WARNING : respect the following order as it is used in pmkscan
		procedures. Common type MAN comes first followed by category
		types from MAN1 to MAN9.
	*/
	FILE_TYPE_MAN,
	FILE_TYPE_MAN1,
	FILE_TYPE_MAN2,
	FILE_TYPE_MAN3,
	FILE_TYPE_MAN4,
	FILE_TYPE_MAN5,
	FILE_TYPE_MAN6,
	FILE_TYPE_MAN7,
	FILE_TYPE_MAN8,
	FILE_TYPE_MAN9,
	/* end of man categories */
	FILE_TYPE_TEMPL,
	FILE_TYPE_TEXT,
	FILE_TYPE_YACC,
	NB_FILE_TYPE		/* number of file types */
};

/* library types *******************************************************/
enum {
	LIB_TYPE_UNKNOWN = 0,
	LIB_TYPE_ASM,
	LIB_TYPE_C,
	LIB_TYPE_CXX,
	NB_LIB_TYPE			/* number of library types */
};

/* object type *********************************************************/
enum {
	OBJ_TYPE_UNKNOWN = 0,
	OBJ_TYPE_ASM,
	OBJ_TYPE_C,
	OBJ_TYPE_CXX
};


/****************
 * misc defines *
 ***********************************************************************/

#define OBJ_SUFFIX		".o"

#define STR_TIME_GEN	"%Y-%m-%d %H:%M"

#define CFGF_HDR_GEN	"/* config file template built by pmkscan (%s) */\n" \
						"/* @configure_input@ */\n\n"


/*******************
 * pmkfile defines *
 ***********************************************************************/

#define PMKF_HDR_GEN	"pmkfile generated by pmkscan (%s).\n"

#define PMKF_TRGT_CMT	"list of template files"

#define PMKF_DEF_CMT	"main defines (NEED MANUAL EDITION)"
#define PMKF_DEF_PKG	"\tPACKAGE = \"mypackage\"\n"
#define PMKF_DEF_DIR	"\tBINDIR = \"\\\\$(PREFIX)/bin\"\n" \
						"\tSBINDIR = \"\\\\$(PREFIX)/sbin\"\n" \
						"\tMANDIR = \"\\\\$(PREFIX)/man\"\n" \
						"\tDATADIR = \"\\\\$(PREFIX)/share/\\\\$(PACKAGE)\"\n"
#define PMKF_DEF_LIB	"\tLIBDIR =  \"\\\\$(PREFIX)/lib\"\n"
#define PMKF_DEF_INC	"\tINCDIR =  \"\\\\$(PREFIX)/include\"\n"
#define PMKF_DEF_MAN	"\tMAN%dDIR = \"\\\\$(MANDIR)/man%d\"\n"
#define PMKF_DEF_TAG	"\t%s = \"extra tag to edit\"\n"

#define PMKF_CMD_NOLABEL	"%s {\n"
#define PMKF_CMD_LABEL		"%s(%s) {\n"
#define PMKF_CMD_END		"}\n"

#define PMKF_COMMENT		"# "

#define PMKF_VAR_BOOL		"\t%s = %s\n"

#define PMKF_VAR_QUOTED		"\t%s = \"%s\"\n"

#define PMKF_VAR_LIST_BEG	"\t%s = ("
#define PMKF_VAR_LIST_ITEM	"\"%s\", "
#define PMKF_VAR_LIST_END	"\"%s\")\n"


/********************
 * makefile defines *
 ***********************************************************************/

/* misc defines ***********************/

#define MKF_OUTPUT_WIDTH	72
#define MKF_TAB_WIDTH		8

#define MKF_LINE_JUMP		"\n"
#define MKF_TWICE_JUMP		"\n\n"

#define MKF_HEADER_GEN		"# Makefile template built by pmkscan (%s)\n" \
							"# @configure_input@\n\n"

#define MKF_OBJ_SRCS_VAR	"%s_SRCS"
#define MKF_OBJECT_SRCS		MKF_OBJ_SRCS_VAR "=\t"
#define MKF_TRGT_OBJS_VAR	"%s_OBJS"
#define MKF_TARGET_OBJS		MKF_TRGT_OBJS_VAR "=\t"

#define MKF_SUBSTVAR		"%s=\t@%s@\n"
#define MKF_VARHDR			"%s=\t"
#define MKF_VAR				"$(%s)"
#define MKF_STCLIB_VAR		"%s=\t$(%s).a\n"
#define MKF_LIB_HEADERS		"%s_HEADERS=\t"
#define MKF_TRGT			"%s: "
#define MKF_VARTRGT			"$(%s): "

#define MKF_HEADER_DATA		"PACKAGE=\t@PACKAGE@\n\n"

/* labels *****************************/

#define MKF_LABEL_AR		"AR"
#define MKF_LABEL_AS		"AS"
#define MKF_LABEL_CPP		"CPP"
#define MKF_LABEL_INSTALL	"INSTALL"
#define MKF_LABEL_LEX		"LEX"
#define MKF_LABEL_LN		"LN"
#define MKF_LABEL_RANLIB	"RANLIB"
#define MKF_LABEL_RM		"RM"
#define MKF_LABEL_YACC		"YACC"

/* build tools ************************/

#define MKF_HEADER_ASM		MKF_LABEL_AS "=\t\t@" MKF_LABEL_AS "@\n" \
							MKF_LABEL_AS "FLAGS=\n"

#define MKF_HEADER_CPP		MKF_LABEL_CPP "=\t\t@" MKF_LABEL_CPP "@\n"

#define MKF_HEADER_C		COMP_LABEL_C "=\t\t@" COMP_LABEL_C "@\n" \
							CFLAGS_LABEL_C "=\t\t@" CFLAGS_LABEL_C "@\n" \
							LDFLAGS_LABEL_C "=\t@" LDFLAGS_LABEL_C "@\n"

#define MKF_HDR_C_SL		SLCFLAGS_LABEL_C "=\t@" SLCFLAGS_LABEL_C "@\n" \
							SLLDFLAGS_LABEL_C "=\t@" SLLDFLAGS_LABEL_C "@\n"
							
#define MKF_HEADER_CXX		COMP_LABEL_CXX "=\t\t@" COMP_LABEL_CXX "@\n" \
							CFLAGS_LABEL_CXX "=\t@" CFLAGS_LABEL_CXX "@\n" \
							LDFLAGS_LABEL_CXX "=\t@" LDFLAGS_LABEL_CXX "@\n"

#define MKF_HDR_CXX_SL		SLCFLAGS_LABEL_CXX "=\t@" SLCFLAGS_LABEL_CXX "@\n" \
							SLLDFLAGS_LABEL_CXX "=\t@" SLLDFLAGS_LABEL_CXX "@\n"
							
#define MKF_HEADER_YACC		MKF_LABEL_YACC "=\t@" MKF_LABEL_YACC "@\n" \
							MKF_LABEL_YACC "FLAGS=\t@" MKF_LABEL_YACC "FLAGS@\n"

#define MKF_HEADER_LEX		MKF_LABEL_LEX "=\t@" MKF_LABEL_LEX "@\n" \
							MKF_LABEL_LEX "FLAGS=\t@" MKF_LABEL_LEX "FLAGS@\n"

#define MKF_HEADER_LD		"LD=\t\t@LD@\n" \
							"# LDFLAGS shall contain -lc if used with ld\n" \
							"LDFLAGS=\t-lc @LDFLAGS@\n\n"

#define MKF_HEADER_AR		MKF_LABEL_AR "=\t\t@" MKF_LABEL_AR "@\n" \
							MKF_LABEL_AR "FLAGS=\tcru\n"
							/*MKF_LABEL_AR "FLAGS=\t@" MKF_LABEL_AR "FLAGS@\n"*/

#define MKF_HEADER_RANLIB	MKF_LABEL_RANLIB "=\t\t@" MKF_LABEL_RANLIB "@\n"

#define MKF_HEADER_MISC		MKF_LABEL_INSTALL "=\t@" MKF_LABEL_INSTALL"@\n" \
							MKF_LABEL_RM "=\t\trm\n" \
							MKF_LABEL_RM "FLAGS=\t-rf\n"

/* tool aliases ***********************/

#define MKF_HEADER_ALIAS	"INSTALL_BIN=\t$(INSTALL) -m 755\n" \
							"INSTALL_SBIN=\t$(INSTALL) -m 755\n" \
							"INSTALL_STLIB=\t$(INSTALL) -m 644\n" \
							"INSTALL_SHLIB=\t$(INSTALL) -m 755\n" \
							"INSTALL_DATA=\t$(INSTALL) -m 644\n" \
							"INSTALL_DIR=\t$(INSTALL) -d -m 755\n" \
							"INSTALL_MAN=\t$(INSTALL) -m 644\n\n"

/* directories ************************/

#define MKF_HEADER_DIR		"PREFIX=\t\t@PREFIX@\n" \
							"BINDIR=\t\t@BINDIR@\n" \
							"SBINDIR=\t@SBINDIR@\n" \
							"DATADIR=\t@DATADIR@\n"
#define MKF_LIB_DIR			"LIBDIR=\t\t@LIBDIR@\n"
#define MKF_INC_DIR			"INCDIR=\t\t@INCDIR@\n"
#define MKF_MAN_DIR			"MANDIR=\t\t@MANDIR@\n"
#define MKF_MANX_DIR		"MAN%iDIR=\t@MAN%iDIR@\n"
#define MKF_SYSCONF_DIR		"SYSCONFDIR=\t@SYSCONFDIR@\n"

/* simple macros */


/* lists */

#define MKF_SDIR_LIST		"SUBDIRS=\t"
#define MKF_GEN_FILES		"GEN_FILES=\t"
#define MKF_TEMPLATES		"TEMPLATES=\t"

/* suffix defines */

#define MKF_SUFFIXES		".SUFFIXES: .o .s .c .C .cc .cxx .cpp\n\n"

#define MKF_BLD_ASM_OBJ		"# assembly suffix\n" \
							"# we use CPP to be more portable\n" \
							".s.o:\n" \
							"\t$(" MKF_LABEL_CPP ") $< | sed '/^#/d' > tmp_asm.s\n" \
							"\t$(" MKF_LABEL_AS ") $(" MKF_LABEL_AS "FLAGS) -o $@ tmp_asm.s\n" \
							"\t$(" MKF_LABEL_RM ") $(" MKF_LABEL_RM "FLAGS) tmp_asm.s\n\n"

#define MKF_BLD_C_OBJ		"# C suffixes\n" \
							".c.o:\n" \
							"\t$(CC) $(CFLAGS) $(SLCFLAGS) -o $@ -c $<\n" \
							"\n.C.o:\n" \
							"\t$(CC) $(CFLAGS) $(SLCFLAGS) -o $@ -c $<\n" \
							"\n.cc.o:\n" \
							"\t$(CC) $(CFLAGS) $(SLCFLAGS) -o $@ -c $<\n\n"

#define MKF_BLD_CXX_OBJ		"# C++ suffixes\n" \
							".cxx.o:\n" \
							"\t$(CXX) $(CXXFLAGS) $(SLCXXFLAGS) -o $@ -c $<\n" \
							"\n.cpp.o:\n" \
							"\t$(CXX) $(CXXFLAGS) $(SLCXXFLAGS) -o $@ -c $<\n\n"

#define MKF_BLD_YACC_SRC	"# yacc suffix\n" \
							".y.c:\n" \
							"\t$(" MKF_LABEL_YACC ") $(YFLAGS) $<\n" \
							"\tmv y.tab.c $@\n\n"

#define MKF_BLD_LEX_SRC		"# lex suffix\n" \
							".l.c:\n" \
							"\t$(" MKF_LABEL_LEX ") $(LFLAGS) $<\n" \
							"\tmv lex.yy.c $@\n\n"


/* target labels **********************/

#define MKF_OBJECT_LABL		"%s: $(" MKF_OBJ_SRCS_VAR ")\n"
#define MKF_TARGET_LABL		"%s: $(" MKF_TRGT_OBJS_VAR ")\n"

/* main targets ***********************/

#define MKF_TARGET_SIMPLE	"$(%s): $(%s)\n"

/* building ***************************/

#define MKF_TARGET_DEF		"\t$(LD) $(LDFLAGS) -o $@ $(%s_OBJS)\n\n"

#define MKF_TARGET_C		"\t$(CC) $(CLDFLAGS) -o $@ $(%s_OBJS)\n\n"

#define MKF_TARGET_CXX		"\t$(CXX) $(CXXLDFLAGS) -o $@ $(%s_OBJS)\n\n"

#define MKF_TARGET_LIB_STC	"\t$(" MKF_LABEL_AR ") $(" MKF_LABEL_AR "FLAGS) $@ $(%s)\n" \
							"\t$(" MKF_LABEL_RANLIB ") $@\n\n"

#define MKF_TARGET_LIB_SHD	"\t$(LD) $(LDFLAGS) $(SLLDFLAGS) -o $@ $(%s)\n"

#define MKF_TARGET_SL_C		"\t$(CC) $(CLDFLAGS) $(SLCLDFLAGS) -o $@ $(%s)\n\n" /* XXX make better */

#define MKF_TARGET_SL_CXX	"\t$(CXX) $(CXXLDFLAGS) $(SLCXXLDFLAGS) -o $@ $(%s)\n\n" /* XXX make better */

#define MKF_TARGET_CLN		"$(%s)_clean:\n" \
							"\t$(" MKF_LABEL_RM ") $("MKF_LABEL_RM "FLAGS) $(%s_OBJS)\n" \
							"\t$(" MKF_LABEL_RM ") $(" MKF_LABEL_RM "FLAGS) $(%s)\n\n"

#define MKF_TARGET_LIB_CLN	"\t$(" MKF_LABEL_RM ") $(" MKF_LABEL_RM "FLAGS) $(%s)\n"

#define MKF_TRGT_BLD_VAR	"BUILD_TARGETS"
#define MKF_LIB_BLD_VAR		"LIB_BUILD_TARGETS"
#define MKF_STATIC_LIB_VAR	"STATIC_LIB_TARGETS"
#define MKF_SHARED_LIB_VAR	"SHARED_LIB_TARGETS"
#define MKF_C_SHLIB_VAR		"C_SHLIB_TARGETS"
#define MKF_CXX_SHLIB_VAR	"CXX_SHLIB_TARGETS"

#define MKF_TRGT_CLEAN_VAR	"CLEAN_TARGETS"
#define MKF_BIN_CLEAN_VAR	"BIN_CLEAN_TARGETS"
#define MKF_LIB_CLEAN_VAR	"LIB_CLEAN_TARGETS"
#define MKF_STLIB_CLN_VAR	"STLIB_CLEAN_TARGETS"
#define MKF_SHLIB_CLN_VAR	"SHLIB_CLEAN_TARGETS"
#define MKF_C_SHL_CLN_VAR	"C_SHLIB_CLEAN_TARGETS"
#define MKF_CXX_SHL_CLN_VAR	"CXX_SHLIB_CLEAN_TARGETS"

#define MKF_TRGT_INST_VAR		"INSTALL_TARGETS"
#define MKF_BIN_INST_VAR		"BIN_INSTALL_TARGETS"
#define MKF_LIB_INST_VAR		"LIB_INSTALL_TARGETS"
#define MKF_STLIB_INST_VAR		"STLIB_INST_TARGETS"
#define MKF_SHLIB_INST_VAR		"SHLIB_INST_TARGETS"
#define MKF_C_SHL_INST_VAR		"C_SHLIB_INST_TARGETS"
#define MKF_CXX_SHL_INST_VAR	"CXX_SHLIB_INST_TARGETS"

#define MKF_TRGT_DEINST_VAR		"DEINSTALL_TARGETS"
#define MKF_BIN_DEINST_VAR		"BIN_DEINSTALL_TARGETS"
#define MKF_LIB_DEINST_VAR		"LIB_DEINSTALL_TARGETS"
#define MKF_STLIB_DEINST_VAR	"STLIB_DEINST_TARGETS"
#define MKF_SHLIB_DEINST_VAR	"SHLIB_DEINST_TARGETS"
#define MKF_C_SHL_DEINST_VAR	"C_SHLIB_DEINST_TARGETS"
#define MKF_CXX_SHL_DEINST_VAR	"CXX_SHLIB_DEINST_TARGETS"

#define MKF_TRGT_ALL_BIN	"ALL_BIN_TARGETS"
#define MKF_TRGT_ALL_LIB	MKF_LIB_BLD_VAR "=\tstatic_libs $(" MKF_SHARED_LIB_VAR ")\n"

#define MKF_LIB_CLEAN_ALL	MKF_LIB_CLEAN_VAR "=\tstatic_libs_clean $(" MKF_SHLIB_CLN_VAR ")\n"
#define MKF_TRGT_CLEAN_STL	MKF_STLIB_CLN_VAR "=\t"
#define MKF_TRGT_CLEAN_SHL	MKF_SHLIB_CLN_VAR "=\t"

#define MKF_TRGT_INST_LIBHDR	"lib_headers_install"
#define MKF_TRGT_INST_STLIB		"static_libs_install"
#define MKF_TRGT_INST_SHLIB		"shared_libs_install"
#define MKF_TRGT_INST_BIN		"install_bin"
#define MKF_TRGT_INST_LIB		"install_lib"
#define MKF_TRGT_INST_MAN		"install_man"
#define MKF_TRGT_INST_DATA		"install_data"
#define MKF_LIB_INSTALL_ALL		MKF_LIB_INST_VAR "=\t" MKF_TRGT_INST_LIBHDR " " MKF_TRGT_INST_STLIB " $(" MKF_SHLIB_INST_VAR ")\n"
#define MKF_GTRGT_INST_BIN		"#\n# binary target rules\n#\n\n" \
								"# main binary install target\n" \
								"install_bin: install_bindir $(" MKF_BIN_INST_VAR ")\n\n" \
								"# install binary directory\n" \
								"install_bindir:\n" \
								"\t$(INSTALL_DIR) $(DESTDIR)$(BINDIR)\n\n"
#define MKF_GTRGT_INST_LIB		"#\n# library target rules\n#\n\n" \
								"# main library install target\n" \
								"install_lib: install_libdir $(" MKF_LIB_INST_VAR ")\n\n" \
								"# install library directory\n" \
								"install_libdir:\n" \
								"\t$(INSTALL_DIR) $(DESTDIR)$(LIBDIR)\n\n"

#define MKF_TRGT_DEINST_LIBHDR	"lib_headers_deinstall"
#define MKF_TRGT_DEINST_STLIB	"static_libs_deinstall"
#define MKF_TRGT_DEINST_SHLIB	"shared_libs_deinstall"
#define MKF_TRGT_DEINST_MAN		"deinstall_man"
#define MKF_TRGT_DEINST_DATA	"deinstall_data"
#define MKF_LIB_DEINSTALL_ALL	MKF_LIB_DEINST_VAR "=\t" MKF_TRGT_DEINST_LIBHDR " " MKF_TRGT_DEINST_STLIB " $(" MKF_SHLIB_DEINST_VAR ")\n"

#define MKF_TRGT_STLIBS		"# static library targets\n" \
							"static_libs: $(" MKF_STATIC_LIB_VAR ")\n\n" \
							"static_libs_clean: $(" MKF_STLIB_CLN_VAR ")\n\n" \
							MKF_TRGT_INST_STLIB ": $(" MKF_STLIB_INST_VAR ")\n\n" \
							MKF_TRGT_DEINST_STLIB ": $(" MKF_STLIB_DEINST_VAR ")\n\n"
#define MKF_TRGT_C_SHLIBS	"# C language shared library targets\n" \
							MK_BLD_TARGET_C ": $(" MKF_C_SHLIB_VAR ")\n\n" \
							MK_CLN_TARGET_C ": $(" MKF_C_SHL_CLN_VAR ")\n\n" \
							MK_INST_TARGET_C ": $(" MKF_C_SHL_INST_VAR ")\n\n" \
							MK_DEINST_TARGET_C ": $(" MKF_C_SHL_DEINST_VAR ")\n\n"
#define MKF_TRGT_CXX_SHLIBS	"# C++ language shared library targets\n" \
							MK_BLD_TARGET_CXX ": $(" MKF_CXX_SHLIB_VAR ")\n\n" \
							MK_CLN_TARGET_CXX ": $(" MKF_CXX_SHL_CLN_VAR ")\n\n" \
							MK_INST_TARGET_CXX ": $(" MKF_CXX_SHL_INST_VAR ")\n\n" \
							MK_DEINST_TARGET_CXX ": $(" MKF_CXX_SHL_DEINST_VAR ")\n\n"

#define MKF_FILE_BIN_VAR	"# by default we consider all binaries as non privileged\n" \
							"BIN_FILES=\t$(ALL_BIN_TARGETS)\n\n"
#define MKF_FILE_SBIN_VAR	"# move privileged binaries here if needed\n" \
							"SBIN_FILES=\n\n"
#define MKF_FILE_MAN_VAR	"MAN%d_FILES=\t"
#define MKF_FILE_DATA_VAR	"DATA_FILES=\t"

#define MKF_TARGET_ALL		"all: $(" MKF_TRGT_BLD_VAR ")\n\n" \
							"clean: $(" MKF_TRGT_CLEAN_VAR ")\n\n"

#define MKF_TARGET_CFG		"config: $(GEN_FILES)\n\n" \
							"$(GEN_FILES): $(TEMPLATES)\n" \
							"\t@pmk\n\n"

#define MKF_TARGET_INST		"install: $(" MKF_TRGT_INST_VAR ")\n\n" \
							"deinstall: $(" MKF_TRGT_DEINST_VAR ")\n\n"

#define MKF_INST_BIN		"$(%s)_install: $(%s)\n" \
							"\t$(INSTALL_BIN) $(%s) $(DESTDIR)$(BINDIR)/$(%s)\n\n"
#define MKF_DEINST_BIN		"$(%s)_deinstall:\n" \
							"\t$(RM) $(RMFLAGS) $(DESTDIR)$(BINDIR)/$(%s)\n\n"
#define MKF_INST_STLIB		"\t$(INSTALL_STLIB) $(%s) $(DESTDIR)$(LIBDIR)/$(%s)\n"
#define MKF_INST_SHLIB		"\t$(INSTALL_SHLIB) $(%s) $(DESTDIR)$(LIBDIR)/$(%s)\n"

/* XXX put dependencies in rule ? */
#define MKF_INST_MAN_H		"# install manual pages\n" \
							"install_man:\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(MANDIR)\n"
#define MKF_INST_MAN_D		"\t# man%d\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(MAN%uDIR)\n"
#define MKF_INST_MAN_P		"\t$(INSTALL_DATA) %s $(DESTDIR)$(MAN%uDIR)/%s\n"

#define MKF_DEINST_MAN_H	"# deinstall manual pages\n" \
							"deinstall_man:\n"	
#define MKF_DEINST_MAN_D	"\t# man%d\n"
#define MKF_DEINST_MAN_P	"\t$(RM) $(RMFLAGS) $(DESTDIR)$(MAN%uDIR)/%s\n"

#define MKF_INST_DATA_H		"# install data files\n" \
							"install_data: $(DATA_FILES)\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(DATADIR)\n"
#define MKF_INST_DATA_P		"\t$(INSTALL_DATA) %s $(DESTDIR)$(DATADIR)/%s\n"

#define MKF_DEINST_DATA_H	"# deinstall data files\n" \
							"deinstall_data:\n"
#define MKF_DEINST_DATA_P	"\t$(RM) $(RMFLAGS) $(DESTDIR)$(DATADIR)/%s\n"


#define MKF_DIST_CLEAN		"distclean: clean\n" \
							"\t$(RM) $(RMFLAGS) $(GEN_FILES)\n"


/**********************************
 * type and structure definitions *
 ***********************************************************************/

/* pseudo token type */
typedef unsigned char	ttype_t;

/* file type and extension struct */
typedef unsigned char	ftype_t;
typedef struct {
	char	*ext;
	ftype_t	 type;
} scn_ext_t;

/* library type */
typedef unsigned char	ltype_t;
typedef struct {
	char	*lang;
	ltype_t	 type;
} lib_type_t;

/* node structure */
typedef struct {
	char		*fname,			/* filename */
				*obj_name,		/* object name */
				*prefix,		/* prefix name */
				*label,			/* binary label */
				*dname;			/* directory name */
	bool		 isdep,			/* dependency flag */
				 mainproc;		/* has main() proc flag ? */
	dynary		*system_inc,	/* system include list */
				*local_inc,		/* local include list */
				*func_calls,	/* function call list */
				*func_decls,	/* function declaration list */
				*type_idtfs,	/* type identifier list */
				*src_deps,		/* source dependency list */
				*sys_deps,		/* system header dependency list */
				*obj_links,		/* object link dependencies */
				*obj_deps;		/* type dependency list */
	ftype_t		 type;			/* file type */
	int			 score;			/* hit score */
} scn_node_t;

/* library cell */
typedef struct {
	char		*lib_name,		/* library name */
				*lib_label,		/* library name label */
                *lib_vmaj,      /* major version number */
                *lib_vmin,      /* minor version number */
				*lib_srcs,		/* library sources variable */
				*lib_hdrs,		/* library headers variable */
				*lib_objs,		/* library objects variable */
				*lib_static,	/* static library filename */
				*lib_shared;	/* shared library filename */
	dynary		*src_list,		/* object dependency list */
				*hdr_list,		/* object dependency list */
				*obj_deps;		/* object dependency list */
	ltype_t		 type;			/* link type */
} lib_cell_t;
				
/* scanning zone data structure */
typedef struct {
	bool		 found[NB_FILE_TYPE],	/* file type flags */
				 found_src,				/* source file flag */
				 advtag,				/* use advanced tagging */
				 gen_pmk,				/* pmkfile generation flag */
				 gen_mkf,				/* makefile generation flag */
				 gen_lib,				/* library generation flag *//* XXX to remove ? */
				 lib_type[NB_LIB_TYPE],	/* file type flags *//* XXX to init */
				 recursive,				/* recursive scan flag */
				 unique;				/* unique file flag */
	char		*directory,				/* initial directory */
				*cfg_name,				/* alternate config file name */
				*mkf_name,				/* alternate makefile name */
				*pmk_name,				/* alternative pmkfile name */
				*ext_mkf;				/* extra to append to makefile template */
	dynary		*dirlist,				/* scanned directory list */
				*dirscan,				/* directory list to scan (just a pointer) */
				*exttags,				/* extra tags */
				*tags,					/* zone tags */
				*manpgs,				/* man pages dynary */
				*datafiles,				/* data files dynary */
				*discard,				/* discard list */
				*templates,				/* template files */
				*generated;				/* files to be generated from templates */
	htable_t	*nodes,					/* global nodes table */
				*objects,				/* zone objects */
				*targets,				/* zone targets */
				*libraries,				/* zone libraries */
				*h_checks,				/* zone header checks */
				*l_checks,				/* zone header checks */
				*t_checks;				/* zone type checks */
	scn_node_t	*pnode;
} scn_zone_t;

/* check type */
typedef struct {
	char	*name,
			*header,
			*library,
			*member;
	dynary	*procs,
			*subhdrs;
	ftype_t	 ftype;
} check_t;

/* scanning data parsed from dat file */
typedef struct {
	htable_t	*headers,
				*libraries,
				*types;
} scandata;


/************************
 * functions prototypes *
 ***********************************************************************/

/* init functions ******************************************************/
scn_node_t	*scan_node_init(char *);
void		 scan_node_destroy(scn_node_t *);
lib_cell_t	*lib_cell_init(char *, dynary *, dynary *, ltype_t);
void		 lib_cell_destroy(lib_cell_t *);
scn_zone_t	*scan_zone_init(htable_t *);
void		 scan_zone_destroy(scn_zone_t *);

/* pmkfile specific ****************************************************/
check_t		*init_chk_cell(char *);
void		 destroy_chk_cell(check_t *);
check_t		*mk_chk_cell(htable_t *, int);
bool		 parse_data_file(prsdata *, scandata *);
char		*conv_to_label(ftype_t, char *, ...);
bool		 recurse_sys_deps(htable_t *, dynary *, char *);
bool		 add_library(scn_zone_t *, char *, scandata *, scn_node_t *);
bool		 check_header(scn_zone_t *, char *, scandata *, scn_node_t *);
bool		 check_type(scn_zone_t *, char *, scandata *, scn_node_t *);
bool		 gen_checks(scn_zone_t *, scandata *);
void		 build_cmd_begin(FILE *, char *, char *);
void		 build_cmd_end(FILE *);
void		 build_comment(FILE *, char *, ...);
void		 build_boolean(FILE *, char *, bool);
void		 build_quoted(FILE *, char *, char *);
bool		 build_list(FILE *, char *, dynary *);
bool		 set_lang(FILE *, ftype_t);
bool		 output_header(htable_t *, char *, FILE *);
bool		 output_library(htable_t *, char *, FILE *);
bool		 output_type(htable_t *, char *, FILE *);
bool		 scan_build_pmk(scn_zone_t *);
bool		 scan_build_cfg(scn_zone_t *);

/* makefile specific ***************************************************/
bool		 find_deps(dynary *, dynary *);
void		 extract_dir(char *, char *, size_t);
void		 build_path(char *, char *, char *, size_t);
bool		 recurse_obj_deps(htable_t *, dynary *, char *);
bool		 gen_objects(scn_zone_t *);
bool		 recurse_src_deps(scn_zone_t *, dynary *, char *);
bool		 gen_targets(scn_zone_t *);
bool		 gen_lib_targets(scn_zone_t *);
size_t		 fprintf_width(size_t, size_t, size_t, FILE *, char *);
void		 mkf_output_header(FILE *, scn_zone_t *);
void		 mkf_output_recurs(FILE *, scn_zone_t *);
void		 mkf_output_srcs(FILE *, scn_zone_t *);
void		 mkf_output_bins(FILE *, scn_zone_t *);
void		 mkf_output_libs(FILE *, scn_zone_t *);
void		 mkf_output_objs(FILE *, scn_zone_t *);
void		 mkf_output_suffixes(FILE *, scn_zone_t *);
void		 mkf_output_build_trgs(FILE *, scn_zone_t *);
void		 mkf_output_clean_trgs(FILE *, scn_zone_t *);
void		 mkf_output_inst_trgs(FILE *, scn_zone_t *);
void		 mkf_output_deinst_trgs(FILE *, scn_zone_t *);
void		 mkf_output_man_trgs(FILE *, scn_zone_t *);
void		 mkf_output_data_trgs(FILE *, scn_zone_t *);
void		 mkf_output_obj_rules(FILE *, scn_zone_t *);
void		 mkf_output_trg_rules(FILE *, scn_zone_t *);
void		 mkf_output_lib_trg_rules(FILE *, scn_zone_t *);
void		 mkf_output_man_inst(FILE *, scn_zone_t *);
void		 mkf_output_data_inst(FILE *, scn_zone_t *);
bool		 scan_build_mkf(scn_zone_t *);

/* common functions */
void		 psc_log(char *, char *, ...);
void		 str_to_upper(char *, size_t, char *);
ftype_t		 check_file_ext(char *);
bool		 process_ppro(void *, char *, prseng_t *);
bool		 process_proc_call(void *, char *, prseng_t *);
bool		 process_proc_decl(void *, char *, prseng_t *);
bool		 process_type(void *, char *, prseng_t *);
bool		 parse_deflib(htable_t *, htable_t *);
bool		 parse_zone_opts(prs_cmn_t *, htable_t *, htable_t *);
bool		 parse_file(prs_cmn_t *, char *, ftype_t, bool);
bool		 process_zone(prs_cmn_t *, scandata *);
bool		 parse_script(char *, prs_cmn_t *, scandata *);
bool		 scan_node_file(prs_cmn_t *, char *, bool);
bool		 scan_dir(prs_cmn_t *, char *, bool);
void		 usage(void);

#endif /* _PMKSCAN_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
