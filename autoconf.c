/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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


#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>

#include "compat/pmk_sys_types.h"
#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_string.h"
#include "compat/pmk_unistd.h"
#include "autoconf.h"
#include "common.h"
#include "functool.h"
#include "pathtools.h"
#include "premake.h"


#define AC_DEBUG	1


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
		*pstr,
		*defname;
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
#ifdef AC_DEBUG
	debugf("computed fsrc = '%s'", fsrc);
#endif

	if (abspath(pgd->basedir, fpath, fname) == false) {
		errorf("failed to compute absolute path for '%s' (basedir).", fpath);
		return(false);
	}
#ifdef AC_DEBUG
	debugf("computed fname = '%s'", fname);
#endif

	fp_in = fopen(fsrc, "r");
	if (fp_in == NULL) {
		errorf("failed to open '%s'.", fsrc);
		return(false);
	}

	/* open temporary file ? */
	fp_out = tmp_open(PMK_TMP_AC_CONF, "w", ftmp, sizeof(ftmp));
	if (fp_out == NULL) {
		errorf("failed to open '%s'.", ftmp);
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

			/* generate define name (DEF__*) */
			defname = build_def_name(buf);
			if (defname == NULL) {
				errorf("unable to build define name for '%s'", buf);
				return(false);
			}
#ifdef AC_DEBUG
			debugf("built define name = '%s'", defname);
#endif

			/* get the defined value */
			pstr = (char *) hash_get(ht, defname);
			if (pstr != NULL) {
				fprintf(fp_out, "%s /* pmk parsed */\n", pstr);
#ifdef AC_DEBUG
				debugf("define value for %s = '%s'", buf, pstr);
#endif
			} else {
				pstr = (char *) hash_get(ht, buf);
				if (pstr != NULL) {
					fprintf(fp_out, "#define %s %s /* pmk parsed */\n", buf, pstr);
#ifdef AC_DEBUG
					debugf("define value for %s = '%s'", buf, pstr);
#endif
				} else {
					fprintf(fp_out, "#undef %s\t/* pmk parsed */\n", buf);
#ifdef AC_DEBUG
					debugf("unset define for '%s'", buf);
#endif
				}
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
		if (unlink(ftmp) == -1)
			errorf(ERRMSG_REMOVE_TMP, ftmp, strerror(errno));
		return(false);
	}

	/*
		Try to erase original file if it exists.
		If build directory is different than source directory
		then there is nothing to delete (so we don't check the result).
	*/
	unlink(fname);
	/* copy new one */
	rval = copy_text_file(ftmp, fname);

	if (unlink(ftmp) == -1) {
		errorf(ERRMSG_REMOVE_TMP, ftmp, strerror(errno));
		return(false);
	}

	pmk_log("Saved '%s'.\n", fname);

	return(rval);
}

/*
	process special variables

	pht : storage table
	pgd : global data
	template : target file
*/

bool ac_process_dyn_var(pmkdata *pgd, char *template) {
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
	if (hash_update_dup(pht, "abs_top_builddir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* set abs_builddir */
	pstr = hash_get(pht, PMK_DIR_BLD_ABS);
	if (hash_update_dup(pht, "abs_builddir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* compute top_builddir */
	pstr = hash_get(pht, PMK_DIR_BLD_ROOT_REL);
	if (hash_update_dup(pht, "top_builddir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* Mr GNU said : rigorously equal to ".". So i did :) */
	pstr = hash_get(pht, PMK_DIR_BLD_REL);
	if (hash_update_dup(pht, "builddir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* set absolute srcdir */
	pstr = hash_get(pht, PMK_DIR_SRC_ROOT_ABS);
	if (hash_update_dup(pht, "abs_top_srcdir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* compute top_srcdir */
	pstr = hash_get(pht, PMK_DIR_SRC_ROOT_REL);
	if (hash_update_dup(pht, "top_srcdir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* absolute path of template */
	pstr = hash_get(pht, PMK_DIR_SRC_ABS);
	if (hash_update_dup(pht, "abs_srcdir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* relative path to template */
	pstr = hash_get(pht, PMK_DIR_SRC_REL);
	if (hash_update_dup(pht, "srcdir", pstr) == HASH_ADD_FAIL)
		return(false);

	/* all operations succeeded */
	return(true);
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

bool ac_set_variables(htable *pht) {
	char	 buf[TMP_BUF_LEN],
                *pstr;

	/* path variables */
	pstr = (char *) hash_get(pht, "PREFIX");
	if (hash_update_dup(pht, "prefix", pstr) == HASH_ADD_FAIL)
		return(false);
	
	pstr = (char *) hash_get(pht, "SYSCONFDIR");
	if (hash_update_dup(pht, "sysconfdir", pstr) == HASH_ADD_FAIL)
		return(false);

	
	if (hash_update_dup(pht, "exec_prefix", "${prefix}") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "bindir", "${exec_prefix}/bin") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "sbindir", "${exec_prefix}/sbin") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "libexecdir", "${exec_prefix}/libexec") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "libdir", "${exec_prefix}/lib") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "datadir", "${prefix}/share") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "includedir", "${prefix}/include") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "mandir", "${prefix}/man") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "infodir", "${prefix}/info") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "sharedstatedir", "${prefix}/com") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "localstatedir", "${prefix}/var") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "oldincludedir", "/usr/include") == HASH_ADD_FAIL)
		return(false);

	if (hash_update_dup(pht, "INSTALL_DATA", "${INSTALL} -m 644") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "INSTALL_PROGRAM", "${INSTALL}") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "INSTALL_SCRIPT", "${INSTALL}") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "INSTALL_STRIP_PROGRAM",
			"${SHELL} $(install_sh) -c -s") == HASH_ADD_FAIL)
		return(false);

/*
	well i dunno if the following really needs to be full compatible with autoconf
*/
	pstr = (char *) hash_get(pht, "OS_ARCH");
	if (hash_update_dup(pht, "host_cpu", pstr) == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "build_cpu", pstr) == HASH_ADD_FAIL)
		return(false); /* XXX  ouargl cross compiling ... */
	if (hash_update_dup(pht, "target_cpu", pstr) == HASH_ADD_FAIL)
		return(false); /* XXX  ouargl cross compiling ... */

        pstr = (char *) hash_get(pht, "OS_NAME");
        strlcpy(buf, pstr, sizeof(buf)); /* no need to check here */
        pstr = (char *) hash_get(pht, "OS_VERSION");
        if (strlcat(buf, pstr, sizeof(buf)) >= sizeof(buf))
		return(false);

	if (hash_update_dup(pht, "host_os", buf) == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "build_os", buf) == HASH_ADD_FAIL)
		return(false); /* XXX  ouargl cross compiling ... */
	if (hash_update_dup(pht, "target_os", buf) == HASH_ADD_FAIL)
		return(false); /* XXX  ouargl cross compiling ... */

	if (hash_update_dup(pht, "host_vendor", "vendorisnotset") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "build_vendor", "vendorisnotset") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "target_vendor", "vendorisnotset") == HASH_ADD_FAIL)
		return(false);

	pstr = (char *) hash_get(pht, "host_cpu");
	strlcpy(buf, pstr, sizeof(buf)); /* no need to check here */
	strlcat(buf, "-", sizeof(buf)); /* no need to check here */
	pstr = (char *) hash_get(pht, "host_vendor");
	strlcat(buf, pstr, sizeof(buf)); /* no need to check here */
	strlcat(buf, "-", sizeof(buf)); /* no need to check here */
	pstr = (char *) hash_get(pht, "host_os");
	if (strlcat(buf, pstr, sizeof(buf)) >= sizeof(buf))
		return(false); /* overflow */

	if (hash_update_dup(pht, "host", buf) == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "build", buf) == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "target", buf) == HASH_ADD_FAIL)
		return(false);

	if (hash_update_dup(pht, "host_alias", "") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "build_alias", "") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "target_alias", "") == HASH_ADD_FAIL)
		return(false);

	if (hash_update_dup(pht, "build_triplet", "") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "host_triplet", "") == HASH_ADD_FAIL)
		return(false);
	if (hash_update_dup(pht, "target_triplet", "") == HASH_ADD_FAIL)
		return(false);

/* use values from pmk.conf */
	pstr = (char *) hash_get(pht, PMKCONF_AC_ECHO_C);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "ECHO_C", pstr) == HASH_ADD_FAIL)
			return(false);
	} /* XXX need else ? */

	pstr = (char *) hash_get(pht, PMKCONF_AC_ECHO_N);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "ECHO_N", pstr) == HASH_ADD_FAIL)
			return(false);
	} /* XXX need else ? */

	pstr = (char *) hash_get(pht, PMKCONF_AC_ECHO_T);
	if (pstr != NULL) {
		if (hash_update_dup(pht, "ECHO_T", pstr) == HASH_ADD_FAIL)
			return(false);
	} /* XXX need else ? */

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

	if (hash_update_dup(pht, "install_sh", "pmkinstall") == HASH_ADD_FAIL)
		return(false); /* provide our own */
	if (hash_update_dup(pht, "program_transform_name", "s,x,x,") == HASH_ADD_FAIL)
		return(false);

	/* byte order */
	pstr = (char *) hash_get(pht, PMKCONF_HW_BYTEORDER);
	if (strncmp(pstr, HW_ENDIAN_BIG, sizeof(pstr)) == 0) {
		if (hash_update_dup(pht, "WORDS_BIGENDIAN", "1") == HASH_ADD_FAIL)
			return(false);
	}

	return(true);
}
