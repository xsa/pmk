/* $Id$ */

/*
 * Copyright (c) 2003 Damien Couderc
 * Copyright (c) 2003 Xavier Santolaria
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
#include <sys/utsname.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pmk.h"


#define PMKSETUP_DEBUG


/* Local functions declaration */
int	create_tmp_config(void);
int	get_env_vars(FILE *);


int main(int argc, char *argv[]) {
	char	config[MAXPATHLEN];

	/* try to open configuration file if it exists */
	snprintf(config, sizeof(config), "%s", PREMAKE_CONFIG_PATH);

	/* XXX open configuration file if it does exist */

	/* create simple temp config */
	create_tmp_config();

	return(0);
}

/* Create temp configuration file */

int create_tmp_config(void) {
	FILE	*tfd;
	int	mtfd = -1;
	char	tf[MAXPATHLEN];


	/* creating temporary file to build new configuration file */
	snprintf(tf, sizeof(tf), "/tmp/pmk.XXXXXXXX");
	if ((mtfd = mkstemp(tf)) == -1 ||
		(tfd = fdopen(mtfd, "w")) == NULL) {
			if (mtfd != -1) {
				unlink(tf);
				close(mtfd);
			}
			err(1, "%s", tf);
	}	

	fprintf(tfd, "# premake configuration file, read by pmk(1)" 
		" and created by pmksetup(8)\n#\n");
	fprintf(tfd, "# See pmk.conf(5) for more details\n\n");

	/* phase 1: create simple config */
	get_env_vars(tfd);

	fclose(tfd);

#ifdef PMKSETUP_DEBUG
	fprintf(stderr, "[DEBUG] %s has not been deleted !\n", tf);
#else
	if (unlink(tf) == -1)
		/* cannot remove temporary file */
		err(1, "%s", tf);

#endif  /* PMKSETUP_DEBUG */

	return(0);
}

/* Get the environment variables needed for the configuration file */

int get_env_vars(FILE *tfd) { 

	char	*bin_path;
	struct	utsname	utsname;

	if (uname(&utsname) == -1)
		err(1, "uname");	

	if ((bin_path = getenv("PATH")) == NULL) 
		err(1, "getenv");

	/* Temporarily printing the variables */
	fprintf(tfd, "OS_NAME=%s\n", utsname.sysname);
	fprintf(tfd, "OS_RELEASE=%s\n", utsname.release);
	fprintf(tfd, "ARCH=%s\n", utsname.machine);
	fprintf(tfd, "BIN_PATH=%s\n", bin_path);		

        return (0);
}
