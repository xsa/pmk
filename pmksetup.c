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

#include <sys/param.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "pmk.h"

/*
	usage
*/

void usage(void) {
	/* nothing yet */
}

/*
	main
*/

int main(int argc, char *argv[]) {
	FILE	*cfd,
		*tfd;
	char	cf[MAXPATHLEN],
		tf[MAXPATHLEN];
	bool	exists=FALSE;

	/* try to open configuration file iif it exists */
	snprintf(cf, sizeof(cf), "%s/%s", SYSCONFDIR, PREMAKE_CONFIG);
	cfd = fopen(cf, "r");
	if (cfd != NULL) {
		exists=TRUE;
	}

	/* bad bad bad, yes it will be changed to something appropriate */
	/* mkstemp powah */
	snprintf(tf, sizeof(tf), "/tmp/pmktmp");
	
	if ((tfd = fopen(tf, "w")) == NULL) {
		/* Hey man ! What's going on ?? */
		err(1, "%s", tf);
	}

	printf("Hey you know what ? I'm doing nothing :)\n");


	fclose(tfd);
	if (exists == TRUE) {
		fclose(cfd);
	}

	/* finished playing with temporary file */
	/* code to overwrite old configuration file */

	return(0);
}
