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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "detect.h"


comp_cell	compilers[] = {
	{CI_TENDRA,		CD_TENDRA,	"__TenDRA__",		"0"},
	{CI_GNUC,		CD_GNUC,	"__GNUC__",		"__GNUC__"},
	{CI_SUNPRO_C,		CD_SUNPRO_C,	"__SUNPRO_C",		"__SUNPRO_C"},
	{CI_SUNPRO_CXX,		CD_SUNPRO_CXX,	"__SUNPRO_CC",		"__SUNPRO_CC"},
	{CI_COMPAQ_C,		CD_COMPAQ_C,	"__DECC",		"__DECC_VER"},
	{CI_COMPAQ_CXX,		CD_COMPAQ_CXX,	"__DECCXX",		"__DECCXX_VER"},
	{CI_HP_ANSI_C,		CD_HP_ANSI_C,	"__HP_cc",		"0"},
	{CI_HP_ANSI_CXX,	CD_HP_ANSI_CXX,	"__HP_aCC",		"__HP_aCC"},
	{CI_IBM_XLC,		CD_IBM_XLC,	"__xlC__",		"__xlC"},
	{CI_INTEL,		CD_INTEL,	"__INTEL_COMPILER",	"__INTEL_COMPILER"},
	{CI_SGI_MPRO,		CD_SGI_MPRO,	"__sgi",		"__COMPILER_VERSION"}
};

int nbcomp = sizeof(compilers) / sizeof(comp_cell);

/*
	detect compiler
*/

void gen_test_file(FILE *fp) {
	int	i;

	fprintf(fp, COMP_TEST_HEADER);

	for (i = 0 ; i < nbcomp ; i++) {
		fprintf(fp, COMP_TEST_FORMAT,
			compilers[i].descr,
			compilers[i].c_macro,
			compilers[i].c_id,
			compilers[i].v_macro
			);
	}


	fprintf(fp, COMP_TEST_FOOTER);
}

/*
	return index in compiler list
*/

int cid_to_idx(unsigned int id) {
	int	i;

	for (i = 0 ; i < nbcomp ; i++) {
		if (compilers[i].c_id == id)
			return(i);
	}

	return(-1);
}

/*
	detect compiler
*/

bool detect_compiler(char *cpath, char *blog, comp_data *cdata) {
	FILE		*tfp,
			*rpipe;
	char		 cfgcmd[MAXPATHLEN],
			 ftmp[MAXPATHLEN],
			 pipebuf[TMP_BUF_LEN];
	int		 idx,
			 r;
	unsigned int	 cid;

	tfp = tmps_open(CC_TEST_FILE, "w", ftmp, sizeof(ftmp), sizeof(CC_TFILE_EXT));
	if (tfp != NULL) {
		/* fill test file */
		gen_test_file(tfp);
		fclose(tfp);
	} else {
		errorf("cannot open test file ('%s').", ftmp);
		return(false);
	}
	/* build compiler command */
	snprintf(cfgcmd, sizeof(cfgcmd), CC_TEST_FORMAT,
		cpath, CC_TEST_BIN, ftmp, blog);

	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		rpipe = popen(CC_TEST_BIN, "r");
		if (rpipe != NULL) {
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get compiler id.");
				pclose(rpipe);
				return(EXIT_FAILURE);
			} else {
				/* get compiler id */
				cid = (unsigned int) strtoul(pipebuf, NULL, 8);

				idx = cid_to_idx(cid);
				if (idx != -1) {
					cdata->index = idx;
					strlcpy(cdata->descr, compilers[idx].descr,
						sizeof(cdata->descr)); /* XXX check */
				}
			}

			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get compiler version.");
				pclose(rpipe);
				return(EXIT_FAILURE);
			} else {
					strlcpy(cdata->version, pipebuf,
						sizeof(cdata->descr)); /* XXX check */
			}

			/* delete binary */
			unlink(CC_TEST_BIN);
		} else {
		}
	} else {
	}

	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("Can not remove %s\n", ftmp);
	}

	return(true);
}

