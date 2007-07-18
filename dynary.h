/* $Id: dynarray.h 1434 2005-08-02 20:18:31Z mipsator $ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
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


#ifndef _PMK_DYNARY_H_
#define _PMK_DYNARY_H_

#include "compat/pmk_stdbool.h"


/*************
 * constants *
 ************************************************************************/

#ifndef DYNARY_AUTO_GROW
#define	DYNARY_AUTO_GROW	4
#endif

#define DYNARY_NOT_FOUND	-1


/**********************************
 * type and structure definitions *
 ***********************************************************************/

typedef struct {
	int		  nbcell,
			  nextidx,
			  (*cmpobj)(const void *, const void *);
	void	  (*freeobj)(void *),
			**pary;
} dynary;


/************************
 * functions prototypes *
 ***********************************************************************/

dynary	*da_create(void (*)(void *), int (*)(const void *, const void *));
void	 da_destroy(dynary *);
bool	 da_resize(dynary *, size_t);
size_t	 da_size(const dynary *);
size_t	 da_used(const dynary *);
bool	 da_push(dynary *, void *);
void	*da_pop(dynary *);
void	*da_shift(dynary *);
void	*da_idx(const dynary *, int);
int		 da_find_index(const dynary *, const void *);
bool	 da_find(const dynary *, const void *);
void	 da_sort(const dynary *);

#endif /* _PMK_DYNARY_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
