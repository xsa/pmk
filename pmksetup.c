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

/*
#define PMKSETUP_DEBUG 1
*/

#include <sys/param.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	int	mtfd = -1;
	char	cf[MAXPATHLEN],
		tf[MAXPATHLEN];
	bool	exists=FALSE;

	/* try to open configuration file if it exists */
	snprintf(cf, sizeof(cf), "%s/%s", SYSCONFDIR, PREMAKE_CONFIG);
	cfd = fopen(cf, "r");
	if (cfd != NULL) {
		exists=TRUE;
	}

	/* creating temporary file to build new configuration file */
	snprintf(tf, sizeof(tf), "/tmp/pmk.XXXXXXXXXX");
	mtfd = mkstemp(tf);
	if (mtfd == -1) {
		/* name randomize failed */
		err(1, "%s", tf);
	}
	tfd = fdopen(mtfd, "w");
	if (tfd == NULL) {
		/* cannot open temporary file */
		err(1, "%s", tf);
	}

	fprintf(tfd, "# PREMAKE");

	printf("Hey you know what ? I'm doing nothing :)\n");


	fclose(tfd);
	if (exists == TRUE) {
		/* configuration file was opened */
		fclose(cfd);
	}

	/* finished playing with temporary file */
	/* XXX code to overwrite old configuration file */

#ifndef PMKSETUP_DEBUG
	if (unlink(tf) == -1) {
		/* canot remove temporary file */
		err(1, "%s", tf);
	}
#else
	printf("[DEBUG] %s has not been deleted !\n", tf);
#endif

	return(0);
}
