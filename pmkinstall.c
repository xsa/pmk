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


#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "compat/pmk_stdbool.h"
#include "common.h"
#include "pmkinstall.h"
#include "premake.h"


#define DEFAULT_MODE	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

extern char	*optarg;
extern int	 optind;


/*
	check_mode
*/

mode_t check_mode(char *mstr) {
	int	nb = 0;
	mode_t	mode = 0;

	if (mstr == NULL)
		return(-1);

	if (isdigit(*mstr) != 0) {
		/* octal value */
		mode = (mode_t) strtol(mstr, NULL, 8); /* XXX check !! */		
	} else {
		/* symbolic value */
		errorf("symbolic value not yet supported");
		exit(1);
	}

	return(mode);
}

/*
	copy file
*/

bool fcopy(char *src, char *dst, int mode) {
	static char	cbuf[S_BLKSIZE];
	bool		exit = false,
			rval = true;
	int		src_fd,
			dst_fd;
	ssize_t		rsz;

	/* try to open both source and destination files */
	src_fd = open(src, O_RDONLY, 0);
	if (src_fd == -1) {
		errorf("Cannot open %s.", src);
		return(false);
	}
	dst_fd = open(dst, O_WRONLY | O_CREAT, mode);
	if (dst_fd == -1) {
		errorf("Cannot open %s.", dst);
		return(false);
	}

	while (exit == false) {
		/* reading data */
		rsz = read(src_fd, cbuf, sizeof(cbuf));
		switch(rsz) {
			case -1:
				/* read error */
				errorf("Failed to read %s", src);
				exit = true;
				rval = false;
				break;

			case 0:
				/* no more data to copy */
				exit = true;
				break;

			default:
				/* data read, trying to write */
				if (write(dst_fd, cbuf, rsz) != rsz) {
					/* write failed */
					errorf("Failed to write %s", dst);
					exit = true;
					rval = false;
				}
				break;
		}
	}

	close(src_fd);
	close(dst_fd);

	/* XXX TODO remove on failure ? */

	return(true);
}

/*
	usage
*/

void usage(void) {
	fprintf(stderr, "usage: pmkinstall [-bcdghmostv] [path]\n");
	/* XXX to finish */
	exit(1);
}


/*
	main
*/

int main(int argc, char *argv[]) {
	bool	 go_exit = false;
	char	 chr;
	int	 mode = DEFAULT_MODE;
	
	while (go_exit == false) {
		chr = getopt(argc, argv, "bcdgh:m:o:stv");
		if (chr == -1) {
			go_exit = true;
		} else {
			switch (chr) {
				case 'b' :
					/* backup */
					/* XXX TODO */
					break;

				case 'c' :
					/* default behavior, do nothing (backwards compat.) */
					break;

				case 'd' :
					/* create directories */
					/* XXX TODO */
					break;

				case 'g' :
					/* specify group */
					/* XXX TODO */
					break;

				case 'm' :
					/* specify mode */
					mode = (int) check_mode(optarg);
/*debugf("mode = %o", mode);*/
					break;

				case 'o' :
					/* specifiy owner */
					/* XXX TODO */
					break;

				case 's' :
					/* strip */
					/* XXX TODO */
					break;

				case 't' :
					/* autconf shit, only for compat */
					break;

				case 'v' :
					/* display version */
					fprintf(stdout, "%s\n", PREMAKE_VERSION);
					exit(0);
					break;

				case 'h' :
				case '?' :
				default :
					usage();
					break;
			}
		}
	}

	argc = argc - optind;
	argv = argv + optind;

	if (argc != 2) {	/* XXX TODO  allow use of more than one source (<2)*/
		usage();
	}

	if (fcopy(argv[0], argv[1], mode) == false) {
		/* copy failed */
		exit(1);
	}

	return(0);
}
