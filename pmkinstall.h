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

#ifndef _PMKINSTALL_H_
#define _PMKINSTALL_H_

#define STRIP_ENV_NAME          "STRIP"

#define DEFAULT_BACKUP_SFX      ".old"

/* default mode */
#define DEFAULT_MODE	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

/* mode masks */
#define USR_MASK	S_IRWXU | S_ISUID		/* user */
#define GRP_MASK	S_IRWXG	| S_ISGID		/* group */
#define OTH_MASK	S_IRWXO				/* other */
#define FULL_MASK	USR_MASK | GRP_MASK | OTH_MASK	/* all */

/* perm masks */
#define R_PERM		S_IRUSR | S_IRGRP | S_IROTH		/* read */
#define W_PERM		S_IWUSR | S_IWGRP | S_IWOTH		/* write */
#define X_PERM		S_IXUSR | S_IXGRP | S_IXOTH		/* execute */
#define S_PERM		S_ISUID | S_ISGID			/* user/group ids */
#define FULL_PERM	R_PERM | W_PERM | X_PERM | S_PERM	/* all */



/* Local functions declaration */
void	strip(char *);
bool	symbolic_to_octal_mode(char *, mode_t *);
bool	check_mode(char *, mode_t *);
bool	process_owner(char *, uid_t *);
bool	process_group(char *, gid_t *);
bool	create_directory(char *, char *, size_t);
bool	build_destination(char *, char *, char *, size_t);
void	usage(void);

#endif /* _PMKINSTALL_H_ */
