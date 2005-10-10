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
#define PMKSCAN_PMKFILE	"pmkfile.scan"
#define PMKSCAN_MKFILE	"Makefile.scan"

/* languages, should use lgdata later */
#define PMKSCAN_LANG_C		"C"
#define PMKSCAN_LANG_CXX	"C++"

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
	PSC_TOK_ZONE
};


/* command keywords */
#define KW_CMD_ADDHDR	"ADD_HEADER"
#define KW_CMD_ADDLIB	"ADD_LIBRARY"
#define KW_CMD_ADDTYP	"ADD_TYPE"

/* command keyword options */
#define KW_OPT_PRC		"PROCEDURE"
#define KW_OPT_LIB		"LIBRARY"
#define KW_OPT_HDR		"HEADER"
#define KW_OPT_MBR		"MEMBER"

/* script keywords */
#define KW_CMD_GENPF	"GEN_PMKFILE"
#define KW_CMD_GENMF	"GEN_MAKEFILE"
#define KW_CMD_GENZN	"GEN_ZONE"

/* script keyword options */
#define KW_OPT_DIR		"DIRECTORY"
#define KW_OPT_DSC		"DISCARD"
#define KW_OPT_MKF		"MAKEFILE"
#define KW_OPT_PMK		"PMKFILE"
#define KW_OPT_REC		"RECURSE"
#define KW_OPT_UNI		"UNIQUE"

/* common options */
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
	FILE_TYPE_MAN9,		/* end of man categories */
	FILE_TYPE_TEMPL,
	FILE_TYPE_TEXT,
	FILE_TYPE_YACC,
	NB_FILE_TYPE		/* number of file type */
};

/* object type *********************************************************/
enum {
	OBJ_TYPE_UNKNOWN = 0,
	OBJ_TYPE_ASM,
	OBJ_TYPE_C,
	OBJ_TYPE_CXX
};

/* misc ****************************************************************/
#define OBJ_SUFFIX		".o"

#define STR_TIME_GEN	"%Y-%m-%d %H:%M"

#define PMKF_HDR_GEN	"pmkfile generated by pmkscan (%s).\n"

#define PMKF_TRGT_CMT	"list of template files"

#define PMKF_DEF_CMT	"main defines (NEED MANUAL EDITION)"
#define PMKF_DEF_STD	"\tPACKAGE = \"mypackage\"\n" \
						"\tVERSION = \"0.0\"\n" \
						"\tBINDIR = \"\\$(PREFIX)/bin\"\n" \
						"\tSBINDIR = \"\\$(PREFIX)/sbin\"\n" \
						"\tMANDIR = \"\\$(PREFIX)/man\"\n" \
						"\tDATADIR = \"\\$(PREFIX)/share/\\$(PACKAGE)\"\n"
#define PMKF_DEF_MAN	"\tMAN%dDIR = \"\\$(MANDIR)/man%d\"\n"

#define PMKF_CMD_NOLABEL	"%s {\n"
#define PMKF_CMD_LABEL		"%s(%s) {\n"
#define PMKF_CMD_END		"}\n"

#define PMKF_COMMENT		"# %s\n"

#define PMKF_VAR_BOOL		"\t%s = %s\n"

#define PMKF_VAR_QUOTED		"\t%s = \"%s\"\n"

#define PMKF_VAR_LIST_BEG	"\t%s = ("
#define PMKF_VAR_LIST_ITEM	"\"%s\", "
#define PMKF_VAR_LIST_END	"\"%s\")\n"

#define MKF_OUTPUT_WIDTH	72
#define MKF_TAB_WIDTH		8

#define MKF_HEADER_GEN	"# Makefile template generated by pmkscan (%s).\n\n"

#define MKF_HEADER_ASM	"AS=\t\t@AS@\n" \
						"ASFLAGS=\n"

#define MKF_HEADER_C	"CC=\t\t@CC@\n" \
						"CFLAGS=\t\t@CFLAGS@\n"

#define MKF_HEADER_CXX	"CXX=\t@CXX@\n" \
						"CXXFLAGS=\t@CXXFLAGS@\n"

#define MKF_HEADER_YACC	"YACC=\t@YACC@\n" \
						"YFLAGS=\t@YFLAGS@\n"

#define MKF_HEADER_LEX	"LEX=\t@LEX@\n" \
						"LFLAGS=\t@LFLAGS@\n"

#define MKF_HEADER_LD	"LINKER=\t\t@CC@\n" \
						"# LDFLAGS shall contain -lc if used with ld\n" \
						"LDFLAGS=\t@LDFLAGS@\n\n"

#define MKF_HEADER_CPP	"CPP=\t\t@CPP@\n"

#define MKF_HEADER_MISC	"INSTALL=\t@INSTALL@\n" \
						"INSTALL_BIN=\t$(INSTALL) -m 755\n" \
						"INSTALL_DATA=\t$(INSTALL) -m 644\n" \
						"INSTALL_DIR=\t$(INSTALL) -d -m 755\n" \
						"INSTALL_MAN=\t$(INSTALL) -m 644\n" \
						"INSTALL_SBIN=\t$(INSTALL) -m 755\n\n" \
						"RM=\t\trm\n" \
						"RMFLAGS=\t-rf\n\n"

#define MKF_HEADER_DATA	"PACKAGE=\t@PACKAGE@\n" \
						"VERSION=\t@VERSION@\n\n"

#define MKF_HEADER_DIR	"PREFIX=\t\t@PREFIX@\n" \
						"BINDIR=\t\t@BINDIR@\n" \
						"SBINDIR=\t@SBINDIR@\n" \
						"DATADIR=\t@DATADIR@\n"
#define MKF_MAN_DIR		"MANDIR=\t\t@MANDIR@\n"
#define MKF_MANX_DIR	"MAN%dDIR=\t@MAN%dDIR@\n"
#define MKF_SYSCONF_DIR	"SYSCONFDIR=\t@SYSCONFDIR@\n"

#define MKF_LINE_JUMP	"\n"
#define MKF_TWICE_JUMP	"\n\n"

#define MKF_SUFFIXES	".SUFFIXES: .o .s .c .C .cc .cxx .cpp\n\n"

#define MKF_BLD_ASM_OBJ		"# assembly suffix\n" \
							"# we use CPP to be more portable\n" \
							".s.o:\n" \
							"\t$(CPP) $< | sed '/^#/d' > tmp_asm.s\n" \
							"\t$(AS) $(ASFLAGS) -o $@ tmp_asm.s\n" \
							"\t$(RM) $(RMFLAGS) tmp_asm.s\n\n"

#define MKF_BLD_C_OBJ		"# C suffixes\n" \
							".c.o:\n" \
							"\t$(CC) $(CFLAGS) -o $@ -c $<\n" \
							"\n.C.o:\n" \
							"\t$(CC) $(CFLAGS) -o $@ -c $<\n" \
							"\n.cc.o:\n" \
							"\t$(CC) $(CFLAGS) -o $@ -c $<\n\n"

#define MKF_BLD_CXX_OBJ		"# C++ suffixes\n" \
							".cxx.o:\n" \
							"\t$(CXX) $(CXXFLAGS) -o $@ -c $<\n" \
							"\n.cpp.o:\n" \
							"\t$(CXX) $(CXXFLAGS) -o $@ -c $<\n\n" \

#define MKF_BLD_YACC_SRC	"# yacc suffixes\n" \
							".y.c:\n" \
							"\t$(YACC) $(YFLAGS) $<\n" \
							"\tmv y.tab.c $@\n\n"

#define MKF_BLD_LEX_SRC		"# lex suffixes\n" \
							".l.c:\n" \
							"\t$(LEX) $(LFLAGS) $<\n" \
							"\tmv lex.yy.c $@\n\n"

#define MKF_OBJECT_SRCS		"%s_SRCS=\t"
#define MKF_OBJECT_LABL		"%s: $(%s_SRCS)\n"

#define MKF_TARGET_OBJS		"%s_OBJS=\t"
#define MKF_TARGET_LABL		"%s: $(%s_OBJS)\n"
#define MKF_TARGET_DEF		"\t$(LD) $(LDFLAGS) -o $@ $(%s_OBJS)\n\n"
#define MKF_TARGET_C		"\t$(CC) $(LDFLAGS) -o $@ $(%s_OBJS)\n\n"
#define MKF_TARGET_CXX		"\t$(CXX) $(LDFLAGS) -o $@ $(%s_OBJS)\n\n"

#define MKF_TARGET_CLN		"%s_clean:\n" \
							"\t$(RM) $(RMFLAGS) $(%s_OBJS)\n" \
							"\t$(RM) $(RMFLAGS) %s\n\n"

#define MKF_TRGT_ALL_VAR	"ALL_TARGETS=\t"
#define MKF_TRGT_CLEAN_VAR	"ALL_CLEAN_TARGETS=\t"
#define MKF_TRGT_INST_VAR	"INSTALL_TARGETS=\tinstall_bin " \
							"install_sbin install_man install_data\n\n"
#define MKF_TRGT_DEINST_VAR	"DEINSTALL_TARGETS=\tdeinstall_bin " \
							"deinstall_sbin deinstall_man deinstall_data\n\n"
#define MKF_FILE_BIN_VAR	"# by default we consider all binaries as non privileged\n" \
							"BIN_FILES=\t$(ALL_TARGETS)\n\n"
#define MKF_FILE_SBIN_VAR	"# move privileged binaries here if needed\n" \
							"SBIN_FILES=\n\n"
#define MKF_FILE_MAN_VAR	"MAN%d_FILES=\t"
#define MKF_FILE_DATA_VAR	"DATA_FILES=\t"

#define MKF_TARGET_ALL		"all: $(ALL_TARGETS)\n\n" \
							"clean: $(ALL_CLEAN_TARGETS)\n\n"

#define MKF_TARGET_INST		"install: all $(INSTALL_TARGETS)\n\n" \
							"deinstall: $(DEINSTALL_TARGETS)\n\n"

#define MKF_INST_BIN		"install_bin:\n" \
							"\t# install binaries\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(BINDIR)\n" \
							"\t@for f in $(BIN_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(INSTALL_BIN) $$f $(DESTDIR)$(BINDIR)/$$d\\n\"; \\\n" \
							"\t\t$(INSTALL_BIN) $$f $(DESTDIR)$(BINDIR)/$$d; \\\n" \
							"\tdone\n\n" \
							"install_sbin:\n" \
							"\t# install privileged binaries\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(SBINDIR)\n" \
							"\t@for f in $(SBIN_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(INSTALL_SBIN) $$f $(DESTDIR)$(SBINDIR)/$$d\\n\"; \\\n" \
							"\t\t$(INSTALL_SBIN) $$f $(DESTDIR)$(SBINDIR)/$$d; \\\n" \
							"\tdone\n\n"

#define MKF_DEINST_BIN		"deinstall_bin:\n" \
							"\t# deinstall binaries\n" \
							"\t@for f in $(BIN_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(RM) $(RMFLAGS) $(DESTDIR)$(BINDIR)/$$d\\n\"; \\\n" \
							"\t\t$(RM) $(RMFLAGS) $(DESTDIR)$(BINDIR)/$$d; \\\n" \
							"\tdone\n\n" \
							"deinstall_sbin:\n" \
							"\t# deinstall privileged binaries\n" \
							"\t@for f in $(SBIN_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(RM) $(RMFLAGS) $(DESTDIR)$(SBINDIR)/$$d\\n\"; \\\n" \
							"\t\t$(RM) $(RMFLAGS) $(DESTDIR)$(SBINDIR)/$$d; \\\n" \
							"\tdone\n\n"

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

#define MKF_INST_DATA		"install_data:\n" \
							"\t# install data files\n" \
							"\t$(INSTALL_DIR) $(DESTDIR)$(DATADIR)\n" \
							"\t@for f in $(DATA_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(INSTALL_DATA) $$f $(DESTDIR)$(DATADIR)/$$d\\n\"; \\\n" \
							"\t\t$(INSTALL_DATA) $$f $(DESTDIR)$(DATADIR)/$$d; \\\n" \
							"\tdone\n\n"

#define MKF_DEINST_DATA		"deinstall_data:\n" \
							"\t# deinstall data files\n" \
							"\t@for f in $(DATA_FILES); do \\\n" \
							"\t\td=`basename $$f`; \\\n" \
							"\t\tprintf \"$(RM) $(RMFLAGS) $(DESTDIR)$(DATADIR)/$$d\\n\"; \\\n" \
							"\t\t$(RM) $(RMFLAGS) $(DESTDIR)$(DATADIR)/$$d; \\\n" \
							"\tdone\n\n"


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

/* node structure */
typedef struct {
	char		*fname,			/* filename */
				*obj_name,		/* object name */
				*prefix,		/* prefix name */
				*dname;			/* directory name */
	bool		 isdep,			/* is a dependency flag ? */
				 mainproc;		/* has main() proc flag ? */
	dynary		*system_inc,	/* system include list */
				*local_inc,		/* local include list */
				*func_calls,	/* function call list */
				*func_decls,	/* function declaration list */
				*type_idtfs,	/* type identifier list */
				*src_deps,		/* source dependency list */
				*obj_links,		/* object link dependencies */
				*obj_deps;		/* type dependency list */
	ftype_t		 type;			/* file type */
	int			 score;			/* hit score */
} scn_node_t;

/* scanning zone data structure */
typedef struct {
	bool		 found[NB_FILE_TYPE],	/* file type flags */
				 found_src,				/* source file flag */
				 recursive,				/* recursive scan flag */
				 unique,				/* unique file flag */
				 gen_pmk,				/* pmkfile generation flag */
				 gen_mkf;				/* makefile generation flag */
	char		*directory,				/* initial directory */
				*mkf_name;				/* optional makefile name */
	dynary		*dirlist,				/* scanned directory list */
				*dirscan,				/* directory list to scan (just a pointer) */
				*manpgs,				/* man pages dynary */
				*datafiles,				/* data files dynary */
				*discard,				/* discard list */
				*templates;				/* template files */
	htable		*nodes,					/* global nodes table */
				*objects,				/* zone objects */
				*targets,				/* zone targets */
				*h_checks,				/* zone header checks */
				*t_checks;				/* zone type checks */
	scn_node_t	*pnode;
} scn_zone_t;

/* check type */
typedef struct {
	char	*name,
			*header,
			*library,
			*member;
	dynary	*procs;
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

/* init functions */
scn_node_t	*scan_node_init(char *);
void		 scan_node_destroy(scn_node_t *);
scn_zone_t	*scan_zone_init(htable *);
void		 scan_zone_destroy(scn_zone_t *);

/* pmkfile specific */
check_t		*mk_chk_cell(htable *, int);
bool		 parse_data_file(prsdata *, scandata *);
char		*conv_to_label(char *, ...);
void		 build_cmd_begin(char *, size_t, char *, char *);
void		 build_cmd_end(char *, size_t);
void		 build_comment(char *, size_t, char *, ...);
void		 build_boolean(char *, size_t, char *, bool);
void		 build_quoted(char *, size_t, char *, char *);
bool		 build_list(char *, size_t, char *, dynary *);
bool		 set_lang(char *, size_t, ftype_t);
bool		 check_header(htable *, char *, scandata *, scn_node_t *);
bool		 check_type(htable *, char *, scandata *, scn_node_t *);
bool		 gen_checks(scn_zone_t *, scandata *);
bool		 scan_build_pmk(char *, scn_zone_t *, scandata *);

/* makefile specific */
bool		 find_deps(dynary *, dynary *);
void		 extract_dir(char *, char *, size_t);
void		 build_path(char *, char *, char *, size_t);
bool		 recurse_obj_deps(htable *, dynary *, char *);
bool		 gen_objects(scn_zone_t *);
bool		 recurse_src_deps(scn_zone_t *, dynary *, char *);
bool		 gen_targets(scn_zone_t *);
size_t		 fprintf_width(size_t, size_t, size_t, FILE *, char *);
void		 mkf_output_header(FILE *, scn_zone_t *);
void		 mkf_output_srcs(FILE *, scn_zone_t *);
void		 mkf_output_objs(FILE *, scn_zone_t *);
void		 mkf_output_bld_trgs(FILE *, scn_zone_t *);
void		 mkf_output_man_trgs(FILE *, scn_zone_t *);
void		 mkf_output_data_trgs(FILE *, scn_zone_t *);
void		 mkf_output_obj_rules(FILE *, scn_zone_t *);
void		 mkf_output_trg_rules(FILE *, scn_zone_t *);
void		 mkf_output_man_inst(FILE *, scn_zone_t *);
bool		 scan_build_mkf(char *, scn_zone_t *);

/* common functions */
void		 psc_log(char *, char *, ...);
void		 str_to_upper(char *, size_t, char *);
ftype_t		 check_file_ext(char *);
bool		 process_ppro(void *, char *, prseng_t *);
bool		 process_proc_call(void *, char *, prseng_t *);
bool		 process_proc_decl(void *, char *, prseng_t *);
bool		 process_type(void *, char *, prseng_t *);
bool		 parse_file(prs_cmn_t *, char *, ftype_t, bool);
bool		 process_zone(prs_cmn_t *, scandata *);
bool		 parse_script(char *, prs_cmn_t *, scandata *);
bool		 scan_node_file(prs_cmn_t *, char *, bool);
bool		 scan_dir(prs_cmn_t *, char *, bool);
void		 usage(void);

#endif /* _PMKSCAN_H_ */

