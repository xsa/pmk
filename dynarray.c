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


/*******************************************************************
 *                                                                 *
 * Dynamic Array                                                   *
 *                                                                 *
 * Simulate an array of string with a variable size.               *
 *                                                                 *
 *******************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynarray.h"


/*
	Initialise dynamic array

	returns pointer of dynamic array
*/

dynary *da_init(void) {
	dynary *ptr = NULL;

	ptr = (dynary *)malloc(sizeof(dynary));
	if (ptr != NULL) {
		ptr->nbcell = 0;
		ptr->first = NULL;
		ptr->last = NULL;
	}

	return(ptr);
}

/*
	Get number size of the array

	returns number of cell of the array
*/

int	da_size(dynary *da) {
	return(da->nbcell);
}

/*
	Add a new value at the end of the array

	da : dynamic array
	str : string to append

	returns 1 on success else 0 on failure
*/

int da_push(dynary *da, char *str) {
	da_cell	*pdac,
		*last;

	pdac = (da_cell *)malloc(sizeof(da_cell));
	if (pdac == NULL) {
		/* malloc failed */
		return(0);
	}

	pdac->val = strdup(str);
	if (pdac->val == NULL) {
		/* strdup failed */
		free(pdac);
		return(0);
	}

	pdac->next = NULL;

	last = da->last;
	if (last != NULL) {
		/* chain in last place */
		last->next = pdac;
	} else {
		/* first cell */
		da->first = pdac;
		da->last = pdac;
	}
	da->last = pdac;
	da->nbcell++;
	
	/* added */
	return(1);
}

/*
	Get a value

	da : dynamic array
	idx : index of the wanted value

	returns value or NULL
*/

char *da_idx(dynary *da, int idx) {
	int	i = 0;
	da_cell	*ptr;

	if (idx >= da->nbcell) {
		return(NULL);
	}

	ptr = da->first;
	while (i < idx) {
		ptr = ptr->next;
		i++;
	}

	return(ptr->val);
}

/*
	destroy dynamic array

	da : dynamic array to destroy
*/

void da_destroy(dynary *da) {
	da_cell *ptr,
		*del;

	if (da->nbcell != 0) {
		ptr = da->first;
		while (ptr != NULL) {
			del = ptr;
			ptr = ptr->next;
			free(del);
		}
		free(da);
	}
}
