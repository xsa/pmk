/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
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

#include "compat/pmk_stdbool.h"
#include "cfgtool.h"
#include "common.h"
#include "premake.h"

/*
	XXX
*/

bool ct_get_version(char *ctpath, char *vstr, char *buffer, size_t sbuf) {
	FILE	*rpipe;
	bool	 rval = false;
	char	 cfgcmd[MAXPATHLEN];


	snprintf(cfgcmd, sizeof(cfgcmd), CT_FORMAT_VERSION, ctpath, vstr);

	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* get version line */
		if (get_line(rpipe, buffer, sbuf) == true) {
			rval = true;
		}

		pclose(rpipe);
	}

	return(rval);
}

/*
	XXX
*/

bool ct_get_data(char *ctpath, char *ostr, char *mod, char *buffer, size_t sbuf) {
	FILE	*rpipe;
	bool	 rval = false;
	char	 cfgcmd[MAXPATHLEN];


	snprintf(cfgcmd, sizeof(cfgcmd), CT_FORMAT_DATA, ctpath, ostr, mod);

	rpipe = popen(cfgcmd, "r");
	if (rpipe != NULL) {
		/* get version line */
		if (get_line(rpipe, buffer, sbuf) == true) {
			rval = true;
		}

		pclose(rpipe);
	}

	return(rval);
}

