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


#ifndef _PMK_COMMON_H_
#define _PMK_COMMON_H_

#include <sys/stat.h>
#include <fcntl.h>

#include "pmk.h"


#ifndef S_BLKSIZE
#define S_BLKSIZE	512	/* standard block size */
#endif

#define MAXTOKENS	128	/* max slots in the paths array */

#define TMP_MK_FILE	TMPDIR "/pmk_XXXXXXXX.mk"


/* structure to store multiple path */
typedef struct {
	int	 pathnum;
	char	*pathlst[MAXTOKENS];
} mpath;

/* struct to store pmk.conf defines */
/* WARN opchar has been put first else linux binaries gives segfault !!! */
typedef struct {
        char    opchar,
		key[OPT_NAME_LEN],
		val[OPT_VALUE_LEN];
} cfg_opt;


FILE	*pmk_log_fp;


bool	 get_line(FILE *, char *, int);

bool	 env_to_opt(char *, pmkcmdopt *);
bool	 get_make_var(char *, char *, int);

dynary	*str_to_dynary(char *, char);
dynary	*str_to_dynary_adv(char *, char *);

bool	 find_file(dynary *, char *, char *, int);
bool	 find_file_dir(dynary *, char *, char *, int);

void	 errorf(const char *, ...);
void	 errorf_line(char *, int, const char *, ...);

void	 debugf(const char *, ...);

bool	 pmk_log_open(char *);
void	 pmk_log_close(void);
bool	 pmk_log(const char *, ...);

bool	 copy_text_file(char *, char *);
bool	 fcopy(char *, char *, mode_t);

FILE	*tmp_open(char *, char *, char *, size_t);
FILE	*tmps_open(char *, char *, char *, size_t, int);

#endif /* _PMK_COMMON_H_ */
