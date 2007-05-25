/* $Id$ */

/*
 * Copyright (c) 2004 Xavier Santolaria <xavier@santolaria.net>
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


#ifndef _ERRMSG_H_
#define _ERRMSG_H_

#define ERRMSG_FILE_ERRNO	"'%s' : %s."

#define ERRMSG_APPEND		"failed to append '%s' in '%s'."
#define ERRMSG_CLOSE		"failed to close %s '%s' : %s."
#define ERRMSG_GET			"failed to get"
#define ERRMSG_INIT			"failed to initialize"
#define ERRMSG_MEM			"out of memory."
#define ERRMSG_OPEN			"failed to open " _ERRMSG_FILE_ERRNO
#define ERRMSG_OPEN_TMP		"failed to open temporary file" ERRMSG_FILE_ERRNO
#define ERRMSG_PARSE		"failed to parse '%s'."
#define ERRMSG_PROCESS		"failed to process"
#define ERRMSG_REMOVE		"failed to remove " ERRMSG_FILE_ERRNO
#define ERRMSG_REMOVE_TMP	"failed to remove temporary file" ERRMSG_FILE_ERRNO

#endif /* _ERRMSG_H_ */
