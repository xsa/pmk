/* $Id$ */

/*
 * Credits for patches :
 *	- Ted Unangst
 */

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

#include "compat/pmk_string.h"
#include "dynarray.h"


/*
	Initialise dynamic array

	returns pointer of dynamic array
*/

dynary *da_init(void) {
	dynary	*ptr = NULL;

	ptr = (dynary *)malloc(sizeof(dynary));
	if (ptr != NULL) {
		ptr->nbcell = 0;
		ptr->nextidx = 0;
		ptr->pary = malloc(DYNARY_AUTO_GROW * sizeof(char *));
	}

	return(ptr);
}

/*
	Resize the array to the given size

	da : array to resize
	nsize : new size

	return : boolean
*/

bool da_resize(dynary *da, size_t nsize) {
	char	**tary;

	tary = realloc(da->pary, nsize * sizeof(char *));
	if (tary != NULL) {
		da->pary = tary;
		da->nbcell = nsize;
		return(true);
	} else {
		return(false);
	}
}

/*
	Get the size of the array

	returns number of cell of the array
*/

size_t	da_size(dynary *da) {
	return((size_t) da->nbcell);
}

/*
	Get the 'used' size of the array

	returns the number of 'used' cells
*/

size_t	da_usize(dynary *da) {
	return((size_t) da->nextidx);
}

/*
	Add a new value at the end of the array

	da : dynamic array
	str : string to append

	return : boolean
*/

bool da_push(dynary *da, char *str) {
	char	 *dup;
	size_t	  gsize;

	dup = strdup(str);
	if (dup == NULL) {
		/* strdup failed */
		return(false);
	}

	if (da->nbcell == da->nextidx) {
		/* compute growing size */
		gsize = (da->nbcell + DYNARY_AUTO_GROW);

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return(false);
		}
	}

	/* insert in last place */
	da->pary[da->nextidx] = dup;
	da->nextidx++;
	
	return(true);
}

/*
	Pop the last value of the array
	
	da : dynamic array

	return : the last element or NULL
*/

char *da_pop(dynary *da) {
	char	 *p;
	size_t	  gsize;

	if (da->nextidx == 0) {
		/* empty */
		return(NULL);
	}

	da->nextidx--;
	p = da->pary[da->nextidx];
	da->pary[da->nextidx] = NULL;

	/* resize if allocated space is too big */
	if ((da->nbcell - da->nextidx) > DYNARY_AUTO_GROW) {
		gsize = da->nbcell - DYNARY_AUTO_GROW;

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return (NULL);
		}
	}

	return(p);
}

/*
	Pop the first value and shift the array

	da : dynamic array

	return : the first cell
*/

char *da_shift(dynary *da) {
	char	*r;
	int	 i;
	size_t	 gsize;

	if (da->nextidx == 0) {
		/* empty */
		return(NULL);
	}

	r = da->pary[0];
	da->nextidx--;

	/* shift remaining values */
	for (i = 0 ; i < da->nextidx ; i++) {
		da->pary[i] = da->pary[i+1];
	}

	/* resize if allocated space is too big */
	if ((da->nbcell - da->nextidx) > DYNARY_AUTO_GROW) {
		gsize = da->nbcell - DYNARY_AUTO_GROW;

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return (NULL);
		}
	}

	return(r);
}

/*
	Get a value

	da : dynamic array
	idx : index of the wanted value

	returns value or NULL
*/

char *da_idx(dynary *da, int idx) {
	if (idx >= da->nextidx) {
		return(NULL);
	}

	return(da->pary[idx]);
}

/*
	destroy dynamic array

	da : dynamic array to destroy
*/

void da_destroy(dynary *da) {
	int	i;

	for (i = 0 ; i < da->nextidx ; i++) {
		free(da->pary[i]);
	}
	free(da->pary);
	free(da);
}
