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
	dynary *ptr = NULL;

	ptr = (dynary *)malloc(sizeof(dynary));
	if (ptr != NULL) {
		ptr->nbcell = 0;
		ptr->nextidx = 0;
		ptr->pary = malloc(DYNARY_AUTO_GROW * sizeof(char *));
	}

	return(ptr);
}

/*
	Get the size of the array

	returns number of cell of the array
*/

int	da_size(dynary *da) {
	return(da->nbcell);
}

/*
	Get the 'used' size of the array

	returns the number of 'used' cells
*/

int	da_usize(dynary *da) {
	return(da->nextidx);
}

/*
	Add a new value at the end of the array

	da : dynamic array
	str : string to append

	returns 1 on success else 0 on failure
*/

int da_push(dynary *da, char *str) {
	char	*dup,
		**tary;
	size_t	gsize;

	dup = strdup(str);
	if (dup == NULL) {
		/* strdup failed */
		return(0);
	}

	if (da->nbcell == da->nextidx) {
		/* compute growing size */
		gsize = (da->nbcell + DYNARY_AUTO_GROW);

		tary = realloc(da->pary, gsize * sizeof(char *));
		if (tary == NULL) {
			/* beware, da->pary is still allocated */
			return (0);
		}
		da->pary = tary;
		da->nbcell = gsize;
	}

	/* insert in last place */
	da->pary[da->nextidx] = dup;
	da->nextidx++;
	
	return(1);
}

/*
	Pop the last value of the array
	
	da : dynamic array

	return the last element or NULL
*/

char *da_pop(dynary *da) {
	char	*p = NULL,
		**tary;
	size_t	gsize;

	da->nextidx--;
	p = da->pary[da->nextidx];
	da->pary[da->nextidx] = NULL;

	/* resize if allocated space is too big */
	if ((da->nbcell - da->nextidx) > DYNARY_AUTO_GROW) {
		gsize = da->nbcell - DYNARY_AUTO_GROW;

		tary = realloc(da->pary, gsize * sizeof(char *));
		if (tary == NULL) {
			/* beware, da->pary is still allocated */
			return (NULL);
		}
		da->pary = tary;
		da->nbcell = gsize;
	}

	return(p);
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

	for (i = 0; i < da->nextidx; i++) {
		free(da->pary[i]);
	}
	free(da->pary);
	free(da);
}
