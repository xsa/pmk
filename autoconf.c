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
#include <unistd.h>

#include "compat/pmk_string.h"
#include "common.h"
#include "premake.h"
#include "autoconf.h"


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

	XXX TODO :)
*/

void ac_process_dyn_var(void) {
	/* should process variables like :
		- srcdir
		- abs_srcdir
		- top_srcdir
		- abs_top_srcdir
		- builddir
		- abs_builddir
		- top_builddir
		- abs_top_builddir
	*/
}
