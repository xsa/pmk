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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compat/pmk_string.h"
#include "compat/pmk_libgen.h"
#include "autoconf.h"
#include "common.h"
#include "pathtools.h"
#include "premake.h"


/*
	parse a config file assuming compatibility with autoconf

	ht : def table
	fpath : file path

	return : boolean
*/

bool ac_parse_config(htable *ht, char *fpath) {
	FILE	*fp_in,
		*fp_out;
	bool	 rval = true;
	char	 line[TMP_BUF_LEN],
		 buf[TMP_BUF_LEN],
		 ftmp[MAXPATHLEN],
		*pstr;
	int	 i,
		 s,
		 fe;

	if (strlcpy(ftmp, "config_tmp", sizeof(ftmp)) >= sizeof(ftmp)) {
		return(false);
	}
	fp_out = fopen(ftmp, "w");
	if (fp_out == NULL)
		return(false);

	fp_in = fopen(fpath, "r");
	if (fp_in == NULL) {
		fclose(fp_out);
		return(false);
	}

	while (get_line(fp_in, line, sizeof(line)) == true) {
		/* look for '#define' */
		pstr = strstr(line, "#define");
		if (pstr == NULL) {
			/* else try to find '#undef' */
			pstr = strstr(line, "#undef");
			if (pstr != NULL) {
				/* jump after "#undef" */
				pstr = pstr + sizeof("#undef");
			}
		} else {
			/* jump after "#define" */
			pstr = pstr + sizeof("#define");
		}

		if (pstr != NULL) {
			/* look for the definition name */
			while (isspace(*pstr) != 0) {
				/* ignore spaces */
				pstr++;
			}
			s = sizeof(line);
			i = 0;
			while (((*pstr == '_') || (isalpha(*pstr) != 0)) && (i < s)) {
				buf[i] = *pstr;
				pstr++;
				i++;
			}
			buf[i] = CHAR_EOS;

			/* check the defined value */
			pstr = (char *) hash_get(ht, buf);
			/* XXX could use value of DEF__ */
			if (pstr != NULL) {
				fprintf(fp_out, "#define %s 1\t/* pmk parsed */\n", buf);
			} else {
				fprintf(fp_out, "#undef %s\t/* pmk parsed */\n", buf);
			}
		} else {
			/* write line as is */
			fprintf(fp_out, "%s\n", line);
		}
	}

	fe = feof(fp_in);
	fclose(fp_in);
	fclose(fp_out);

	if (fe == 0) {
		unlink(ftmp);
		return(false);
	}

	/* erase orig and copy new one */
	unlink(fpath);
	rval = copy_text_file(ftmp, fpath);

	unlink(ftmp);

	return(rval);
}

/*
	process special variables

	pht : storage table
	pgd : global data
	template : target file
*/

void ac_process_dyn_var(htable *pht, pmkdata *pgd, char *template) {
	char	*srcdir,
		*basedir,
		*pstr,
		 buf[MAXPATHLEN],
		 ac_dir[MAXPATHLEN],
		 abs_ad[MAXPATHLEN],
		 abs_bd[MAXPATHLEN],
		 abs_sd[MAXPATHLEN];

	/* should process variables like following (if i believe autoconf manual) :
		- srcdir : the relative path to the directory that contains the source code for that `Makefile'.
		- abs_srcdir : absolute path of srcdir.
		- top_srcdir : the relative path to the top-level source code directory for the package.
			In the top-level directory, this is the same as srcdir.
		- abs_top_srcdir : absolute path of top_srcdir.
		- builddir : rigorously equal to ".", added for symmetry only.
		- abs_builddir : absolute path of builddir.
		- top_builddir : the relative path to the top-level of the current build tree.
			In the top-level directory, this is the same as builddir.
		- abs_top_builddir : absolute path of top_builddir.
	*/

	srcdir = pgd->srcdir;
	basedir = pgd->basedir;

	/* extract ac_dir from template
		NOTE : ac_dir is relative */
	/* NOTE : we use strdup to avoid problem with linux's dirname */
	pstr = strdup(template);
	strlcpy(abs_ad, dirname(pstr), sizeof(abs_ad));
	free(pstr);
	relpath(srcdir, abs_ad, ac_dir);

	/* init builddir */
	hash_add(pht, "abs_top_builddir", strdup(basedir));

	/* set abs_builddir */
	abspath(basedir, ac_dir, abs_bd);
	hash_add(pht, "abs_builddir", strdup(abs_bd));

	/* compute top_builddir */
	relpath(abs_bd, basedir, buf);
	hash_add(pht, "top_builddir", strdup(buf));

	/* Mr GNU said : rigorously equal to ".". So i did :) */
	hash_add(pht, "builddir", strdup("."));

	/* set absolute srcdir */
	hash_add(pht, "abs_top_srcdir", strdup(srcdir));

	/* compute top_srcdir */
	relpath(abs_bd, srcdir, buf);
	hash_add(pht, "top_srcdir", strdup(buf));

	/* absolute path of template */
	abspath(srcdir, ac_dir, abs_sd);
	hash_add(pht, "abs_srcdir", strdup(abs_sd));

	/* relative path to template */
	relpath(abs_bd, abs_sd, buf);
	hash_add(pht, "srcdir", strdup(buf));
}

/*
	clean dynamic variables

	pht : storage table
*/

void ac_clean_dyn_var(htable *pht) {
	hash_delete(pht, "srcdir");
	hash_delete(pht, "abs_srcdir");
	hash_delete(pht, "top_srcdir");
	hash_delete(pht, "abs_top_srcdir");
	hash_delete(pht, "builddir");
	hash_delete(pht, "abs_builddir");
	hash_delete(pht, "top_builddir");
	hash_delete(pht, "abs_top_builddir");
}

/*
	set autoconf specific variables
*/

void ac_set_variables(htable *pht) {
	char	 buf[TMP_BUF_LEN],
                *pstr;

	/* path variables */
	pstr = (char *) hash_get(pht, "PREFIX");
	hash_add(pht, "prefix", strdup(pstr));
	
	pstr = (char *) hash_get(pht, "SYSCONFDIR");
	hash_add(pht, "sysconfdir", strdup(pstr));

	
	hash_add(pht, "exec_prefix", strdup("${prefix}"));
	hash_add(pht, "bindir", strdup("${exec_prefix}/bin"));
	hash_add(pht, "sbindir", strdup("${exec_prefix}/sbin"));
	hash_add(pht, "libexecdir", strdup("${exec_prefix}/libexec"));
	hash_add(pht, "libdir", strdup("${exec_prefix}/lib"));
	hash_add(pht, "datadir", strdup("${prefix}/share"));
	hash_add(pht, "includedir", strdup("${prefix}/include"));
	hash_add(pht, "mandir", strdup("${prefix}/man"));
	hash_add(pht, "infodir", strdup("${prefix}/info"));
	hash_add(pht, "sharedstatedir", strdup("${prefix}/com"));
	hash_add(pht, "localstatedir", strdup("${prefix}/var"));
	hash_add(pht, "oldincludedir", strdup("/usr/include"));

	hash_add(pht, "INSTALL_DATA", strdup("${INSTALL} -m 644"));
	hash_add(pht, "INSTALL_PROGRAM", strdup("${INSTALL}"));
	hash_add(pht, "INSTALL_SCRIPT", strdup("${INSTALL}"));
	hash_add(pht, "INSTALL_STRIP_PROGRAM", strdup("${SHELL} $(install_sh) -c -s"));

/*
	XXX
	well i dunno if the following really need to be full compatible with autoconf 
*/
	pstr = (char *) hash_get(pht, "OS_ARCH");
	hash_add(pht, "host_cpu", strdup(pstr));
	hash_add(pht, "build_cpu", strdup(pstr)); /* XXX  ouargl cross compiling ... */
	hash_add(pht, "target_cpu", strdup(pstr)); /* XXX  ouargl cross compiling ... */

        pstr = (char *) hash_get(pht, "OS_NAME");
        strlcpy(buf, pstr, sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "OS_VERSION");
        strlcat(buf, pstr, sizeof(buf)); /* XXX check */
	hash_add(pht, "host_os", strdup(buf));
	hash_add(pht, "build_os", strdup(buf)); /* XXX  ouargl cross compiling ... */
	hash_add(pht, "target_os", strdup(buf)); /* XXX  ouargl cross compiling ... */

	hash_add(pht, "host_vendor", strdup("vendorisnotset"));
	hash_add(pht, "build_vendor", strdup("vendorisnotset"));
	hash_add(pht, "target_vendor", strdup("vendorisnotset"));

        pstr = (char *) hash_get(pht, "host_cpu");
        strlcpy(buf, pstr, sizeof(buf)); /* no need to check here */
        strlcat(buf, "-", sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "host_vendor");
        strlcat(buf, pstr, sizeof(buf)); /* no need to check here */
        strlcat(buf, "-", sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "host_os");
        strlcat(buf, pstr, sizeof(buf)); /* XXX check */

	hash_add(pht, "host", strdup(buf));
	hash_add(pht, "build", strdup(buf));
	hash_add(pht, "target", strdup(buf));

	hash_add(pht, "host_alias", strdup(""));
	hash_add(pht, "build_alias", strdup(""));
	hash_add(pht, "target_alias", strdup(""));

	hash_add(pht, "build_triplet", strdup(""));
	hash_add(pht, "host_triplet", strdup(""));
	hash_add(pht, "target_triplet", strdup(""));

/*	XXX TODO verify the following */
	hash_add(pht, "SET_MAKE", strdup(""));
	hash_add(pht, "AMDEP_TRUE", strdup(""));
	hash_add(pht, "AMDEP_FALSE", strdup("#"));
	hash_add(pht, "AMDEPBACKSLASH", strdup("\\"));
	hash_add(pht, "AMTAR", strdup("echo 'PMK: set as useless'"));
	hash_add(pht, "ACLOCAL", strdup("echo 'PMK: set as useless'"));
	hash_add(pht, "AUTOCONF", strdup("echo 'PMK: set as useless'"));
	hash_add(pht, "AUTOHEADER", strdup("echo 'PMK: set as useless'"));
	hash_add(pht, "AUTOMAKE", strdup("echo 'PMK: set as useless'"));
	hash_add(pht, "MAKEINFO", strdup("echo 'PMK: set as useless'"));
	hash_add(pht, "ECHO_C", strdup("\\c"));
	hash_add(pht, "ECHO_N", strdup(""));
	hash_add(pht, "ECHO_T", strdup(""));
	hash_add(pht, "EXEEXT", strdup("")); /* cygwin shit ! */
	hash_add(pht, "PACKAGE_BUGREPORT", strdup(""));
	hash_add(pht, "PACKAGE_NAME", strdup(""));
	hash_add(pht, "PACKAGE_STRING", strdup(""));
	hash_add(pht, "PACKAGE_TARNAME", strdup(""));
	hash_add(pht, "PACKAGE_VERSION", strdup(""));
	hash_add(pht, "PACKAGE_SEPARATOR", strdup(""));
	hash_add(pht, "SET_MAKE", strdup(""));
	hash_add(pht, "CYGPATH_W", strdup(""));
	hash_add(pht, "DEPDIR", strdup(".deps"));
	hash_add(pht, "CCDEPMODE", strdup(""));
	hash_add(pht, "LIBOBJS", strdup(""));
	hash_add(pht, "LTLIBOBJS", strdup(""));
	hash_add(pht, "PATH_SEPARATOR", strdup(":")); /* default shell is sh */

        /* XXX AC_CHECK_PROG stuff, should be moved somewhere */
        pstr = (char *) hash_get(pht, "CC");
	hash_add(pht, "ac_ct_CC", strdup(pstr)); /* XXX shit ? */
        pstr = (char *) hash_get(pht, "RANLIB");
	hash_add(pht, "ac_ct_RANLIB", strdup(pstr)); /* XXX shit ? */
        pstr = (char *) hash_get(pht, "STRIP");
	hash_add(pht, "ac_ct_STRIP", strdup(pstr)); /* XXX shit ? */

	hash_add(pht, "install_sh", strdup("PMK: set as useless")); /* provide our own ? */
	hash_add(pht, "program_transform_name", strdup("s,x,x,"));


	/* byte order */
	pstr = (char *) hash_get(pht, PMKCONF_HW_BYTEORDER);
	if (strncmp(pstr, HW_ENDIAN_BIG, sizeof(pstr)) == 0) {
		hash_add(pht, "WORDS_BIGENDIAN", strdup("1"));
	}
}
