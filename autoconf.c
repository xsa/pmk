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


#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>

#include "compat/pmk_string.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_unistd.h"
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

bool ac_parse_config(pmkdata *pgd) {
	FILE	*fp_in,
		*fp_out;
	bool	 rval = true;
	char	 line[TMP_BUF_LEN],
		 buf[TMP_BUF_LEN],
		 fsrc[MAXPATHLEN],
		 fname[MAXPATHLEN],
		 ftmp[MAXPATHLEN],
		*fpath,
		*pstr;
	htable	*ht;
	int	 i,
		 s,
		 fe;

	ht = pgd->htab;
	fpath = pgd->ac_file;

	/* compute full path */
	if (abspath(pgd->srcdir, fpath, fsrc) == false) {
		errorf("failed to compute absolute path for '%s' (srcdir).", fpath);
		return(false);
	}

	if (abspath(pgd->basedir, fpath, fname) == false) {
		errorf("failed to compute absolute path for '%s' (basedir).", fpath);
		return(false);
	}

	fp_in = fopen(fsrc, "r");
	if (fp_in == NULL) {
		return(false);
	}

	/* open temporary file ? */
	fp_out = tmp_open(PMK_TMP_AC_CONF, "w", ftmp, sizeof(ftmp));
	if (fp_out == NULL) {
		fclose(fp_in);
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
	unlink(fname);
	rval = copy_text_file(ftmp, fname);

	unlink(ftmp);

	pmk_log("Saved '%s'.\n", fname);

	return(rval);
}

/*
	process special variables

	pht : storage table
	pgd : global data
	template : target file
*/

void ac_process_dyn_var(pmkdata *pgd, char *template) {
	char	*pstr;
	htable	*pht;

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

	   The pmk directories should be equivalent to that.
	*/

	pht = pgd->htab;

	/* init builddir */
	pstr = hash_get(pht, PMK_DIR_BLD_ROOT_ABS);
	hash_update_dup(pht, "abs_top_builddir", pstr);

	/* set abs_builddir */
	pstr = hash_get(pht, PMK_DIR_BLD_ABS);
	hash_update_dup(pht, "abs_builddir", pstr);

	/* compute top_builddir */
	pstr = hash_get(pht, PMK_DIR_BLD_ROOT_REL);
	hash_update_dup(pht, "top_builddir", pstr);

	/* Mr GNU said : rigorously equal to ".". So i did :) */
	pstr = hash_get(pht, PMK_DIR_BLD_REL);
	hash_update_dup(pht, "builddir", pstr);

	/* set absolute srcdir */
	pstr = hash_get(pht, PMK_DIR_SRC_ROOT_ABS);
	hash_update_dup(pht, "abs_top_srcdir", pstr);

	/* compute top_srcdir */
	pstr = hash_get(pht, PMK_DIR_SRC_ROOT_REL);
	hash_update_dup(pht, "top_srcdir", pstr);

	/* absolute path of template */
	pstr = hash_get(pht, PMK_DIR_SRC_ABS);
	hash_update_dup(pht, "abs_srcdir", pstr);

	/* relative path to template */
	pstr = hash_get(pht, PMK_DIR_SRC_REL);
	hash_update_dup(pht, "srcdir", pstr);
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
	hash_update_dup(pht, "prefix", pstr);
	
	pstr = (char *) hash_get(pht, "SYSCONFDIR");
	hash_update_dup(pht, "sysconfdir", pstr);

	
	hash_update_dup(pht, "exec_prefix", "${prefix}");
	hash_update_dup(pht, "bindir", "${exec_prefix}/bin");
	hash_update_dup(pht, "sbindir", "${exec_prefix}/sbin");
	hash_update_dup(pht, "libexecdir", "${exec_prefix}/libexec");
	hash_update_dup(pht, "libdir", "${exec_prefix}/lib");
	hash_update_dup(pht, "datadir", "${prefix}/share");
	hash_update_dup(pht, "includedir", "${prefix}/include");
	hash_update_dup(pht, "mandir", "${prefix}/man");
	hash_update_dup(pht, "infodir", "${prefix}/info");
	hash_update_dup(pht, "sharedstatedir", "${prefix}/com");
	hash_update_dup(pht, "localstatedir", "${prefix}/var");
	hash_update_dup(pht, "oldincludedir", "/usr/include");

	hash_update_dup(pht, "INSTALL_DATA", "${INSTALL} -m 644");
	hash_update_dup(pht, "INSTALL_PROGRAM", "${INSTALL}");
	hash_update_dup(pht, "INSTALL_SCRIPT", "${INSTALL}");
	hash_update_dup(pht, "INSTALL_STRIP_PROGRAM", "${SHELL} $(install_sh) -c -s");

/*
	well i dunno if the following really needs to be full compatible with autoconf 
*/
	pstr = (char *) hash_get(pht, "OS_ARCH");
	hash_update_dup(pht, "host_cpu", pstr);
	hash_update_dup(pht, "build_cpu", pstr); /* XXX  ouargl cross compiling ... */
	hash_update_dup(pht, "target_cpu", pstr); /* XXX  ouargl cross compiling ... */

        pstr = (char *) hash_get(pht, "OS_NAME");
        strlcpy(buf, pstr, sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "OS_VERSION");
        strlcat(buf, pstr, sizeof(buf)); /* XXX check */
	hash_update_dup(pht, "host_os", buf);
	hash_update_dup(pht, "build_os", buf); /* XXX  ouargl cross compiling ... */
	hash_update_dup(pht, "target_os", buf); /* XXX  ouargl cross compiling ... */

	hash_update_dup(pht, "host_vendor", "vendorisnotset");
	hash_update_dup(pht, "build_vendor", "vendorisnotset");
	hash_update_dup(pht, "target_vendor", "vendorisnotset");

        pstr = (char *) hash_get(pht, "host_cpu");
        strlcpy(buf, pstr, sizeof(buf)); /* no need to check here */
        strlcat(buf, "-", sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "host_vendor");
        strlcat(buf, pstr, sizeof(buf)); /* no need to check here */
        strlcat(buf, "-", sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "host_os");
        strlcat(buf, pstr, sizeof(buf)); /* XXX check */

	hash_update_dup(pht, "host", strdup(buf));
	hash_update_dup(pht, "build", strdup(buf));
	hash_update_dup(pht, "target", strdup(buf));

	hash_update_dup(pht, "host_alias", strdup(""));
	hash_update_dup(pht, "build_alias", strdup(""));
	hash_update_dup(pht, "target_alias", strdup(""));

	hash_update_dup(pht, "build_triplet", strdup(""));
	hash_update_dup(pht, "host_triplet", strdup(""));
	hash_update_dup(pht, "target_triplet", strdup(""));

/* XXX TODO use values from pmksetup */
	hash_update_dup(pht, "ECHO_C", "\\c");
	hash_update_dup(pht, "ECHO_N", "");
	hash_update_dup(pht, "ECHO_T", "");

/* XXX TODO verify the following */
	/*hash_add(pht, "SET_MAKE", strdup(""));                            */
	/*hash_add(pht, "AMDEP_TRUE", strdup(""));                          */
	/*hash_add(pht, "AMDEP_FALSE", strdup("#"));                        */
	/*hash_add(pht, "AMDEPBACKSLASH", strdup("\\"));                    */
	/*hash_add(pht, "AMTAR", strdup("echo 'PMK: set as useless'"));     */
	/*hash_add(pht, "ACLOCAL", strdup("echo 'PMK: set as useless'"));   */
	/*hash_add(pht, "AUTOCONF", strdup("echo 'PMK: set as useless'"));  */
	/*hash_add(pht, "AUTOHEADER", strdup("echo 'PMK: set as useless'"));*/
	/*hash_add(pht, "AUTOMAKE", strdup("echo 'PMK: set as useless'"));  */
	/*hash_add(pht, "MAKEINFO", strdup("echo 'PMK: set as useless'"));  */
	/*hash_add(pht, "EXEEXT", strdup("")); |+ cygwin shit ! +|          */
	/*hash_add(pht, "PACKAGE_BUGREPORT", strdup(""));                   */
	/*hash_add(pht, "PACKAGE_NAME", strdup(""));                        */
	/*hash_add(pht, "PACKAGE_STRING", strdup(""));                      */
	/*hash_add(pht, "PACKAGE_TARNAME", strdup(""));                     */
	/*hash_add(pht, "PACKAGE_VERSION", strdup(""));                     */
	/*hash_add(pht, "PACKAGE_SEPARATOR", strdup(""));                   */
	/*hash_add(pht, "CYGPATH_W", strdup(""));                           */
	/*hash_add(pht, "DEPDIR", strdup(".deps"));                         */
	/*hash_add(pht, "CCDEPMODE", strdup(""));                           */
	/*hash_add(pht, "LIBOBJS", strdup(""));                             */
	/*hash_add(pht, "LTLIBOBJS", strdup(""));                           */
	/*hash_add(pht, "PATH_SEPARATOR", strdup(":")); |+ default shell is sh +|*/

        /* XXX AC_CHECK_PROG stuff, should be moved somewhere */
	/*pstr = (char *) hash_get(pht, "CC");                        */
	/*hash_update_dup(pht, "ac_ct_CC", pstr); |+ XXX shit ? +|    */
	/*pstr = (char *) hash_get(pht, "RANLIB");                    */
	/*hash_update_dup(pht, "ac_ct_RANLIB", pstr); |+ XXX shit ? +|*/
	/*pstr = (char *) hash_get(pht, "STRIP");                     */
	/*hash_update_dup(pht, "ac_ct_STRIP", pstr); |+ XXX shit ? +| */

	hash_update_dup(pht, "install_sh", strdup("pmkinstall")); /* provide our own ? */
	hash_update_dup(pht, "program_transform_name", strdup("s,x,x,"));


	/* byte order */
	pstr = (char *) hash_get(pht, PMKCONF_HW_BYTEORDER);
	if (strncmp(pstr, HW_ENDIAN_BIG, sizeof(pstr)) == 0) {
		hash_update_dup(pht, "WORDS_BIGENDIAN", "1");
	}
}
