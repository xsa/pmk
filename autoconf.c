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

	return : XXX
*/

bool ac_parse_config(htable *ht, char *fpath) {
	FILE	*fp_in,
		*fp_out;
	bool	rval = true;
	char	line[TMP_BUF_LEN],
		buf[TMP_BUF_LEN],
		ftmp[MAXPATHLEN],
		*pstr;
	int	i,
		s,
		fe;

	strlcpy(ftmp, "config_tmp", sizeof(ftmp)); /* XXX check ? */
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
			pstr = hash_get(ht, buf);
			/* XXX could use value of DEF__ */
			if (pstr != NULL) {
				/* debugf("defining '%s'", buf); XXX */
				fprintf(fp_out, "#define %s 1\t/* pmk parsed */\n", buf);
			} else {
				/* debugf("undefining '%s'", buf); XXX */
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
		return(false); /* XXX correct ? */
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
	char	*ac_dir,
		*srcdir,
		*basedir,
		buf[MAXPATHLEN];

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

	ac_dir = strdup(dirname(template));
	srcdir = pgd->srcdir;
	basedir = pgd->basedir;

	/* init builddir */
	hash_add(pht, "abs_top_builddir", basedir);

	/* compute top_builddir */
	relpath(ac_dir, basedir, buf);
	hash_add(pht, "top_builddir", buf);

	/* set abs_builddir */
	hash_add(pht, "abs_builddir", ac_dir);

	/* Mr GNU said : rigorously equal to ".". So i did :) */
	hash_add(pht, "builddir", ".");

	/* set srcdir */
	hash_add(pht, "abs_top_srcdir", srcdir);

	/* compute top_srcdir */
	relpath(basedir, srcdir, buf);
	hash_add(pht, "top_srcdir", buf);

	/* absolute path of template */
	hash_add(pht, "abs_srcdir", ac_dir);

	/* relative path to template */
	relpath(basedir, ac_dir, buf);
	hash_add(pht, "srcdir", buf);

	free(ac_dir);
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
