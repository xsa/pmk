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

#include "common.h"
#include "detect.h"


/*
	detect compiler
*/

bool detect_compiler(char *cpath, pmkdata *pgd) {
	FILE	*tfp,
		*rpipe;
	char	 cfgcmd[MAXPATHLEN],
		 ftmp[MAXPATHLEN],
		 pipebuf[TMP_BUF_LEN];
	int	 r;

	tfp = tmps_open(CC_TEST_FILE, "w", ftmp, sizeof(ftmp), sizeof(CC_TFILE_EXT));
	if (tfp != NULL) {
		/* fill test file */
		fprintf(tfp, COMPILER_TEST);
		fclose(tfp);
	} else {
		errorf("cannot open test file ('%s').", ftmp);
		return(false);
	}

	/* build compiler command */
	snprintf(cfgcmd, sizeof(cfgcmd), CC_TEST_FORMAT,
		cpath, CC_TEST_BIN, ftmp, pgd->buildlog);

	/* get result */
	r = system(cfgcmd);
	if (r == 0) {
		rpipe = popen(CC_TEST_BIN, "r");
		if (rpipe != NULL) {
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get CFLAGS.");
				pclose(rpipe);
				return(false);
			}
			pclose(rpipe);

			pmk_log("%s", pipebuf);
		} else {
		}
	} else {
	}

	return(true);
}
