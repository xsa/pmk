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
#define PREMAKE_SUBVER_PMKSCAN	"5"

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
#define KW_CMD_DEFLIBS	"DEFINE_LIBS"

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
#define PMKF_DEF_STD	"\tPACKAGE = \"mypackage\"\n" \
						"\tVERS_MAJ = \"0\"\n" \
						"\tVERS_MIN = \"0\"\n" \
						"\tVERSION = \"$VERS_MAJ.$VERS_MIN\"\n"
#define PMKF_DEF_DIR	"\tBINDIR = \"\\\\$(PREFIX)/bin\"\n" \
						"\tSBINDIR = \"\\\\$(PREFIX)/sbin\"\n" \
						"\tMANDIR = \"\\\\$(PREFIX)/man\"\n" \
						"\tDATADIR = \"\\\\$(PREFIX)/share/\\\\$(PACKAGE)\"\n"
#define PMKF_DEF_LIB	"\tLIBDIR =  \"\\\\$(PREFIX)/lib\"\n"
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

/* misc defines ********************/

#define MKF_OUTPUT_WIDTH	72
#define MKF_TAB_WIDTH		8

#define MKF_LINE_JUMP		"\n"
#define MKF_TWICE_JUMP		"\n\n"

#define MKF_HEADER_GEN		"# Makefile template built by pmkscan (%s)\n" \
							"# @configure_input@\n\n"

/* macro defines ********************/

#define MKF_HEADER_ASM		"AS=\t\t@AS@\n" \
							"ASFLAGS=\n"

#define MKF_HEADER_C		COMP_LABEL_C "=\t\t@" COMP_LABEL_C "@\n" \
							CFLAGS_LABEL_C "=\t\t@" CFLAGS_LABEL_C "@\n" \
							LDFLAGS_LABEL_C "=\t@" LDFLAGS_LABEL_C "@\n"

#define MKF_HDR_C_SL		SLCFLAGS_LABEL_C "=\t@" SLCFLAGS_LABEL_C "@\n" \
							SLLDFLAGS_LABEL_C "=\t@" SLLDFLAGS_LABEL_C "@\n"
							
#define MKF_HEADER_CXX		COMP_LABEL_CXX "=\t@" COMP_LABEL_CXX "@\n" \
							CFLAGS_LABEL_CXX "=\t@" CFLAGS_LABEL_CXX "@\n" \
							LDFLAGS_LABEL_CXX "=\t@" LDFLAGS_LABEL_CXX "@\n"

#define MKF_HDR_CXX_SL		SLCFLAGS_LABEL_CXX "=\t@" SLCFLAGS_LABEL_CXX "@\n" \
							SLLDFLAGS_LABEL_CXX "=\t@" SLLDFLAGS_LABEL_CXX "@\n"
							
#define MKF_HEADER_YACC		"YACC=\t@YACC@\n" \
							"YFLAGS=\t@YFLAGS@\n"

#define MKF_HEADER_LEX		"LEX=\t@LEX@\n" \
							"LFLAGS=\t@LFLAGS@\n"

#define MKF_HEADER_LD		"LINKER=\t\t@CC@\n" \
							"# LDFLAGS shall contain -lc if used with ld\n" \
							"LDFLAGS=\t@LDFLAGS@\n\n"

/* XXX better handling of shared libs needed here */
#define MKF_HEADER_SHARED	"# XXX broken, obsolete, to remove and replace\n" \
							"SHLIB_SUPPORT=\t@SHLIB_SUPPORT@\n" \
							"SLCLAGS=\t@SLCFLAGS@\n" \
							"SLCXXLAGS=\t@SLCXXFLAGS@\n" \
							"SLLDFLAGS=\t@SLLDFLAGS@\n"
							
#define MKF_HEADER_AR		"AR=\t\t@AR@\n" \
							"ARFLAGS=\tcru\n"
							/*"ARFLAGS=\t@ARFLAGS@\n"*/

#define MKF_HEADER_RANLIB	"RANLIB=\t\t@RANLIB@\n"

#define MKF_HEADER_CPP		"CPP=\t\t@CPP@\n"

#define MKF_HEADER_MISC		"INSTALL=\t@INSTALL@\n" \
							"INSTALL_BIN=\t$(INSTALL) -m 755\n" \
							"INSTALL_DATA=\t$(INSTALL) -m 644\n" \
							"INSTALL_DIR=\t$(INSTALL) -d -m 755\n" \
							"INSTALL_MAN=\t$(INSTALL) -m 644\n" \
							"INSTALL_SBIN=\t$(INSTALL) -m 755\n\n" \
							"RM=\t\trm\n" \
							"RMFLAGS=\t-rf\n\n"

#define MKF_HEADER_DATA		"PACKAGE=\t@PACKAGE@\n" \
							"VERSION=\t@VERSION@\n\n"

#define MKF_HEADER_DIR		"PREFIX=\t\t@PREFIX@\n" \
							"BINDIR=\t\t@BINDIR@\n" \
							"SBINDIR=\t@SBINDIR@\n" \
							"DATADIR=\t@DATADIR@\n"
#define MKF_LIB_DIR			"LIBDIR=\t\t@LIBDIR@\n"
#define MKF_MAN_DIR			"MANDIR=\t\t@MANDIR@\n"
#define MKF_MANX_DIR		"MAN%iDIR=\t@MAN%iDIR@\n"
#define MKF_SYSCONF_DIR		"SYSCONFDIR=\t@SYSCONFDIR@\n"

#define MKF_SDIR_LIST		"SUBDIRS=\t"
#define MKF_GEN_FILES		"GEN_FILES=\t"
#define MKF_TEMPLATES		"TEMPLATES=\t"

#define MKF_OBJECT_SRCS		"%s_SRCS=\t"
#define MKF_TARGET_OBJS		"%s_OBJS=\t"

#define MKF_SUBSTVAR		"%s=\t@%s@\n"
#define MKF_VARHDR			"%s=\t"
#define MKF_STCLIB_VAR		"%s=\t%s.a\n"

/* suffix defines ******************/

#define MKF_SUFFIXES		".SUFFIXES: .o .s .c .C .cc .cxx .cpp\n\n"

#define MKF_BLD_ASM_OBJ		"# assembly suffix\n" \
							"# we use CPP to be more portable\n" \
							".s.o:\n" \
							"\t$(CPP) $< | sed '/^#/d' > tmp_asm.s\n" \
							"\t$(AS) $(ASFLAGS) -o $@ tmp_asm.s\n" \
							"\t$(RM) $(RMFLAGS) tmp_asm.s\n\n"

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
							"\t.cpp.o:\n" \
							"\t$(CXX) $(CXXFLAGS) $(SLCXXFLAGS) -o $@ -c $<\n\n"

#define MKF_BLD_YACC_SRC	"# yacc suffix\n" \
							".y.c:\n" \
							"\t$(YACC) $(YFLAGS) $<\n" \
							"\tmv y.tab.c $@\n\n"

#define MKF_BLD_LEX_SRC		"# lex suffix\n" \
							".l.c:\n" \
							"\t$(LEX) $(LFLAGS) $<\n" \
							"\tmv lex.yy.c $@\n\n"

/* target defines ******************/

#define MKF_OBJECT_LABL		"%s: $(%s_SRCS)\n"
#define MKF_TARGET_LABL		"%s: $(%s_OBJS)\n"
#define MKF_TARGET_SIMPLE	"$(%s): $(%s)\n"
#define MKF_TARGET_DEF		"\t$(LD) $(LDFLAGS) -o $@ $(%s_OBJS)\n\n"
#define MKF_TARGET_C		"\t$(CC) $(CLDFLAGS) -o $@ $(%s_OBJS)\n\n"
#define MKF_TARGET_CXX		"\t$(CXX) $(CXXLDFLAGS) -o $@ $(%s_OBJS)\n\n"

#define MKF_TARGET_LIB_STC	"\t$(AR) $(ARFLAGS) $@ $(%s)\n" \
							"\t$(RANLIB) $@\n\n"

#define MKF_TARGET_LIB_SHD	"\t@if $(SHLIB_SUPPORT); then \\\n" \
							"\t\tprintf \"$(CC) $(LDFLAGS) $(SLLDFLAGS) -o $@ $(%s)\\n\"; \\\n" \
							"\t\t$(LD) $(LDFLAGS) $(SLLDFLAGS) -o $@ $(%s); \\\n" \
							"\tfi\n"
#define MKF_TARGET_SL_C		"\t$(CC) $(CLDFLAGS) $(SLCLDFLAGS) -o $@ $(%s)\n\n" /* XXX make better */
#define MKF_TARGET_SL_CXX	"\t$(CXX) $(CXXLDFLAGS) $(SLCXXLDFLAGS) -o $@ $(%s)\n\n" /* XXX make better */

#define MKF_TARGET_CLN		"%s_clean:\n" \
							"\t$(RM) $(RMFLAGS) $(%s_OBJS)\n" \
							"\t$(RM) $(RMFLAGS) %s\n\n"

#define MKF_TARGET_LIB_CLN	"\t$(RM) $(RMFLAGS) $(%s)\n" /* XXX add lib objects ? */

#define MKF_TRGT_ALL_VAR	"ALL_TARGETS=\t$(ALL_BIN_TARGETS) $(ALL_LIB_TARGETS)" /* XXX make it dynamic */
#define MKF_TRGT_ALL_BIN	"ALL_BIN_TARGETS=\t"
#define MKF_TRGT_ALL_LIB	"ALL_LIB_TARGETS=\t"
/*#define MKF_TRGT_ALL_LIB	"ALL_LIB_TARGETS=\t$(ST_LIB_TARGETS) $(SH_LIB_TARGETS)"*/
#define MKF_TRGT_ALL_ST_LIB	"ST_LIB_TARGETS=\t"
#define MKF_TRGT_ALL_SH_LIB	"SH_LIB_TARGETS=\t"

#define MKF_TRGT_CLEAN_VAR	"ALL_CLEAN_TARGETS=\t$(ALL_BIN_CLEAN_TARGETS) $(ALL_LIB_CLEAN_TARGETS)" /* XXX make it dynamic */
#define MKF_TRGT_CLEAN_BIN	"ALL_BIN_CLEAN_TARGETS=\t"
#define MKF_TRGT_CLEAN_LIB	"ALL_LIB_CLEAN_TARGETS=\t"
#define MKF_TRGT_CLEAN_STL	"ST_LIB_CLEAN_TARGETS=\t"
#define MKF_TRGT_CLEAN_SHL	"SH_LIB_CLEAN_TARGETS=\t"

#define MKF_TRGT_INST_VAR	"INSTALL_TARGETS=\tinstall_bin " \
							"install_sbin install_man install_data\n\n"
#define MKF_TRGT_DEINST_VAR	"DEINSTALL_TARGETS=\tdeinstall_bin " \
							"deinstall_sbin deinstall_man deinstall_data\n\n"
#define MKF_FILE_BIN_VAR	"# by default we consider all binaries as non privileged\n" \
							"BIN_FILES=\t$(ALL_BIN_TARGETS)\n\n"
#define MKF_FILE_SBIN_VAR	"# move privileged binaries here if needed\n" \
							"SBIN_FILES=\n\n"
#define MKF_FILE_BIN_VAR	"# by default we consider all binaries as non privileged\n" \
							"BIN_FILES=\t$(ALL_BIN_TARGETS)\n\n"
#define MKF_FILE_MAN_VAR	"MAN%d_FILES=\t"
#define MKF_FILE_DATA_VAR	"DATA_FILES=\t"

#define MKF_TARGET_ALL		"all: $(ALL_TARGETS)\n\n" \
							"clean: $(ALL_CLEAN_TARGETS)\n\n"

#define MKF_TARGET_CFG		"config: $(GEN_FILES)\n\n" \
							"$(GEN_FILES): $(TEMPLATES)\n" \
							"\t@pmk\n\n"

#define MKF_TARGET_INST		"install: all $(INSTALL_TARGETS)\n\n" \
							"deinstall: $(DEINSTALL_TARGETS)\n\n"

#define INST_LIST_RULE(comment, target, list, dir, inst)	\
							"# " comment "\n" \
							target ":\n" \
							"\t@if test -n \"$(" list ")\"; then \\\n" \
							"\t\tprintf \"$(INSTALL_DIR) $(DESTDIR)$(" dir ")\\n\"; \\\n" \
							"\t\t$(INSTALL_DIR) $(DESTDIR)$(" dir "); \\\n" \
							"\t\tlist=\"$(" list ")\"; \\\n" \
							"\t\tfor f in $$list; do \\\n" \
							"\t\t\td=`basename $$f`; \\\n" \
							"\t\t\tprintf \"" inst " $$f $(DESTDIR)$(" dir ")/$$d\\n\"; \\\n" \
							"\t\t\t" inst " $$f $(DESTDIR)$(" dir ")/$$d; \\\n" \
							"\t\tdone \\\n" \
							"\tfi\n\n" \

#define DEINST_LIST_RULE(comment, target, list, dir, deinst)	\
							"# " comment "\n" \
							target ":\n" \
							"\t@if test -n \"$(" list ")\"; then \\\n" \
							"\t\tlist=\"$(" list ")\"; \\\n" \
							"\t\tfor f in $$list; do \\\n" \
							"\t\t\td=`basename $$f`; \\\n" \
							"\t\t\tprintf \"\t" deinst " $(DESTDIR)$(" dir ")/$$d\\n\"; \\\n" \
							"\t\t\t" deinst " $(DESTDIR)$(" dir ")/$$d; \\\n" \
							"\t\tdone \\\n" \
							"\tfi\n\n" \

#define MKF_INST_BIN \
		INST_LIST_RULE(	"install binaries",				\
						"install_bin",					\
						"BIN_FILES",					\
						"BINDIR",						\
						"$(INSTALL_BIN)")				\
		INST_LIST_RULE(	"install privileged binaries",	\
						"install_sbin",					\
						"SBIN_FILES",					\
						"SBINDIR",						\
						"$(INSTALL_BIN)")

#define MKF_DEINST_BIN \
		DEINST_LIST_RULE(	"deinstall binaries",				\
							"deinstall_bin",					\
							"BIN_FILES",						\
							"BINDIR",							\
							"$(RM) $(RMFLAGS)")					\
		DEINST_LIST_RULE(	"deinstall privileged binaries",	\
							"deinstall_sbin",					\
							"SBIN_FILES",						\
							"SBINDIR",							\
							"$(RM) $(RMFLAGS)")


#define MKF_INST_MAN_H		"install_man:\n" \
							"\t# install manual pages\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(MANDIR)\n"
#define MKF_INST_MAN_B		"\t# man%d\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(MAN%dDIR)\n" \
							"\t@for f in $(MAN%d_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(INSTALL_DATA) $$f $(DESTDIR)$(MAN%dDIR)/$$d\\n\"; \\\n" \
							"\t\t$(INSTALL_DATA) $$f $(DESTDIR)/$(MAN%dDIR)/$$d; \\\n" \
							"\tdone\n"

#define MKF_DEINST_MAN_H	"deinstall_man:\n" \
							"\t# deinstall manual pages\n"
#define MKF_DEINST_MAN_B	"\t# man%d\n" \
							"\t@for f in $(MAN%d_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(RM) $(RMFLAGS) $(DESTDIR)/$(MAN%dDIR)/$$d\\n\"; \\\n" \
							"\t\t$(RM) $(RMFLAGS) $(DESTDIR)/$(MAN%dDIR)/$$d; \\\n" \
							"\tdone\n"

#define MKF_INST_DATA \
		INST_LIST_RULE(	"install data files",	\
						"install_data",			\
						"DATA_FILES",			\
						"DATADIR",				\
						"$(INSTALL_DATA)")

#define MKF_DEINST_DATA \
		DEINST_LIST_RULE(	"deinstall data files",	\
							"deinstall_data",		\
							"DATA_FILES",			\
							"DATADIR",				\
							"$(RM) $(RMFLAGS)")



#define MKF_DIST_CLEAN		"distclean: clean\n" \
							"\t$(RM) $(RMFLAGS) $(GEN_FILES)\n" \
							"\t@for d in $(SUBDIRS); do \\\n" \
							"\t\tprintf \"$(RM) $(RMFLAGS) $$d/*.scan $$d/*.log $$d/*.core\\n\"; \\\n" \
							"\t\t$(RM) $(RMFLAGS) $$d/*.scan $$d/*.log $$d/*.core; \\\n" \
							"\tdone\n\n"

/* XXX recursive makefiles, not enabled yet
#define MKF_RECURS_TRGT		"# recursive targets wrapper\n" \
							"all_recursive \\\n" \
							"clean_recursive \\\n" \
							"install_recursive \\\n" \
							"deinstall_recursive \\\n" \
							"install_bin_recursive \\\n" \
							"deinstall_bin_recursive \\\n" \
							"install_data_recursive \\\n" \
							"deinstall_data_recursive \\\n" \
							"install_man_recursive \\\n" \
							"deinstall_man_recursive \\\n" \
							"distclean_recursive:\n"

#define MKF_RECURS_PROC		"\t@target=`echo $@ | sed s/_recursive//`; \\\n" \
							"\tfor d in $(SUBDIRS); do \\\n" \
							"\t\tprintf \"Recursive make $target in $$d\\n\"; \\\n" \
							"\t\tcd $$d; \\\n" \
							"\t\t$(MAKE) $target; \\\n" \
							"\tdone\\\n"
*/

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
				*templates;				/* template files */
	htable		*nodes,					/* global nodes table */
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
	htable	*headers,
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
scn_zone_t	*scan_zone_init(htable *);
void		 scan_zone_destroy(scn_zone_t *);

/* pmkfile specific ****************************************************/
check_t		*init_chk_cell(char *);
void		 destroy_chk_cell(check_t *);
check_t		*mk_chk_cell(htable *, int);
bool		 parse_data_file(prsdata *, scandata *);
char		*conv_to_label(ftype_t, char *, ...);
bool		 recurse_sys_deps(htable *, dynary *, char *);
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
bool		 output_header(htable *, char *, FILE *);
bool		 output_library(htable *, char *, FILE *);
bool		 output_type(htable *, char *, FILE *);
bool		 scan_build_pmk(scn_zone_t *);
bool		 scan_build_cfg(scn_zone_t *);

/* makefile specific ***************************************************/
bool		 find_deps(dynary *, dynary *);
void		 extract_dir(char *, char *, size_t);
void		 build_path(char *, char *, char *, size_t);
bool		 recurse_obj_deps(htable *, dynary *, char *);
bool		 gen_objects(scn_zone_t *);
bool		 recurse_src_deps(scn_zone_t *, dynary *, char *);
bool		 gen_targets(scn_zone_t *);
bool		 gen_lib_targets(scn_zone_t *);
size_t		 fprintf_width(size_t, size_t, size_t, FILE *, char *);
void		 mkf_output_header(FILE *, scn_zone_t *);
void		 mkf_output_recurs(FILE *, scn_zone_t *);
void		 mkf_output_srcs(FILE *, scn_zone_t *);
void		 mkf_output_objs(FILE *, scn_zone_t *);
void		 mkf_output_bld_trgs(FILE *, scn_zone_t *);
void		 mkf_output_man_trgs(FILE *, scn_zone_t *);
void		 mkf_output_data_trgs(FILE *, scn_zone_t *);
void		 mkf_output_obj_rules(FILE *, scn_zone_t *);
void		 mkf_output_trg_rules(FILE *, scn_zone_t *);
void		 mkf_output_lib_trg_rules(FILE *, scn_zone_t *);
void		 mkf_output_man_inst(FILE *, scn_zone_t *);
bool		 scan_build_mkf(scn_zone_t *);

/* common functions */
void		 psc_log(char *, char *, ...);
void		 str_to_upper(char *, size_t, char *);
ftype_t		 check_file_ext(char *);
bool		 process_ppro(void *, char *, prseng_t *);
bool		 process_proc_call(void *, char *, prseng_t *);
bool		 process_proc_decl(void *, char *, prseng_t *);
bool		 process_type(void *, char *, prseng_t *);
bool		 parse_deflib(htable *, htable *);
bool		 parse_zone_opts(prs_cmn_t *, htable *, htable *);
bool		 parse_file(prs_cmn_t *, char *, ftype_t, bool);
bool		 process_zone(prs_cmn_t *, scandata *);
bool		 parse_script(char *, prs_cmn_t *, scandata *);
bool		 scan_node_file(prs_cmn_t *, char *, bool);
bool		 scan_dir(prs_cmn_t *, char *, bool);
void		 usage(void);

#endif /* _PMKSCAN_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
