/* $Id$ */

/*
 * Copyright (c) 2006 Damien Couderc
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


#ifndef _PMK_OBJECT_H_
#define _PMK_OBJECT_H_

#include "dynarray.h"
#include "hash.h"

typedef unsigned char	potype;

typedef struct {
	potype	 type;
	void	*data;
} pmkobj;

/* define pmk object types */
#define PO_NULL		0x00
#define PO_BOOL		0x01
#define PO_STRING	0x02
#define PO_LIST		0x04
#define PO_HASH		0x08


pmkobj	*po_mk_bool(bool);
pmkobj	*po_mk_str(char *);
pmkobj	*po_mk_list(dynary *);
pmkobj	*po_mk_hash(htable_t *);
pmkobj	*po_dup(pmkobj *);
potype	 po_get_type(pmkobj *);
void	*po_get_data(pmkobj *);
bool	 po_get_bool(pmkobj *);
char	*po_get_str(pmkobj *);
dynary	*po_get_list(pmkobj *);
void	 po_free(pmkobj *);
pmkobj	*po_append(void *, void *, void *);

#endif /* _PMK_OBJECT_H_ */
