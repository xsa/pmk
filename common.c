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

#define EMPTY_OPT_VALUE ""

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "premake.h"
#include "pmk.h"

/*
	put env variable in an option
*/

bool env_to_opt(char *env_name, pmkcmdopt *opt) {
	char	*env_val;
	bool	rval;

	env_val = getenv(env_name);
	if (env_val == NULL) {
		/* env variable name not found */
		strncpy(opt->value, EMPTY_OPT_VALUE, sizeof(EMPTY_OPT_VALUE));
		rval = FALSE;
	} else {
		/* okay get it */
		strncpy(opt->value, env_val, sizeof(env_val));
		rval = TRUE;
		
	}
	return(rval);
}

/*
	get_make_var
*/

void get_make_var(char *varname) {
	FILE	*tfd;
	char	tf[256] = "/tmp/pmk_tst.XXXXXXXX",
		varstr[256],
		result[256];
	int	fd = -1;

	/* XXX mktemp stuff */

	snprintf(varstr, 256, "/usr/bin/make -V %s > %s", varname, tf);
	system(varstr);

	tfd = fopen(tf, "r");
	fgets(result, 256, tfd);
	fclose(tfd);

//	unlink(tf);

	printf("%s => %s\n", varname, result);
}
