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
#include <sys/types.h>
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "pmksetup.h"
#include "premake.h"
#include "readconf.h"



int main(int argc, char *argv[]) {
	FILE	*f;
	char	config[MAXPATHLEN];
	htable	*ht;

	/* try to open configuration file if it exists */
	snprintf(config, sizeof(config), "%s", PREMAKE_CONFIG_PATH);

	if(config != NULL) {
		if ((f = fopen(config, "r")) != NULL) {
			debugf("Reading configuration file: %s", 
						PREMAKE_CONFIG_PATH); 

                	if ((ht = hash_init(MAX_CONF_OPT)) == NULL) {
                        	errorf("cannot create hash table");
                        	exit(1);
			}
			read_config_file(f, ht);
			
			/* XXX will need to compare values
			 * from the configuration file and the ones
			 * found on the system
			 */
			hash_destroy(ht);
		}
	}

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

	if ((mtfd = mkstemp(tf)) != -1)
		tfd = fdopen(mtfd, "w");
		
	/* XXX this way it passes -ansi -pedantic ... should be rewritten */
	if ((mtfd == -1) || (tfd == NULL)) {
		if (mtfd != -1) {
			unlink(tf);
			close(mtfd);
		}	
		errorf("%s", tf);
		exit(1);
	}	

	fprintf(tfd, "# premake configuration file, read by pmk(1)" 
		" and created by pmksetup(8)\n#\n");
	fprintf(tfd, "# See pmk.conf(5) for more details\n\n");

	get_env_vars(tfd);

	fclose(tfd);

#ifdef PMKSETUP_DEBUG
	debugf("%s has not been deleted!", tf);
#else
	if (unlink(tf) == -1) {
	 	errorf("cannot remove temporary file");
		exit(1);	
	}
#endif  /* PMKSETUP_DEBUG */
	return(0);
}

/* Get the environment variables needed for the configuration file */

int get_env_vars(FILE *f) { 
	int	j;
	char	*bin_path, buf[MAXPATHLEN];
	struct	utsname	utsname;
	mpath	stpath;

	if (uname(&utsname) == -1) {
		errorf("uname");
		exit(1);
	}
	if ((bin_path = getenv("PATH")) == NULL) {
		errorf("could not get the PATH environment variable");
		exit(1);
	}

	/* 
	 *splitting the PATH variable and storing in a struct 
	 * for later use by find_file
	 */ 
	if ((strsplit(bin_path, &stpath, STR_DELIMITER)) == -1) {
		errorf("could not split the PATH correctly");
		exit(1);
	}

	/* Temporarily printing the variables */
	fprintf(f, "OS_NAME=%s\n", utsname.sysname);
	fprintf(f, "OS_RELEASE=%s\n", utsname.release);
	fprintf(f, "ARCH=%s\n", utsname.machine);
	fprintf(f, "BIN_PATH=%s\n", bin_path);

	/* 
	 * XXX should store the above variables in a hash instead of
	 * looping here. 
	 */
	debugf("Looking for needed binaries...");
	for (j = 0; j < MAXBINS; j++) {
		if (find_file(&stpath, binaries[j], buf, MAXPATHLEN) == 0) {
			fprintf(stderr, "Looking for %s => %s\n", 
				binaries[j], buf);
		}
		else {
			errorf("%s not found", binaries[j]);
			exit(1);
		}
	}
	return(0);
}
