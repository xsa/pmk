/* $Id: dynarray.c 1462 2005-08-21 08:52:06Z mipsator $ */

/*
 * Copyright (c) 2003-2007 Damien Couderc
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

/*
 * Credits for patches :
 *	- Ted Unangst
 */

/*
 *
 * generic dynamic array implementation
 *
 */

#include <stdlib.h>

#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "dynary.h"

/*
 * TODO
 *
 * - add da_iterator_init() and da_iterate()
 *
 */


/***************
 * da_create() *
 ***********************************************************************
 DESCR
	Initialise dynamic array and related functions

 IN
	freefunc : object free function
	cmpfunc : object comparison function

 OUT
	returns pointer on dynamic array
 ***********************************************************************/

dynary *da_create(void (*freefunc)(void *), int (*cmpfunc)(const void *, const void *)) {
	dynary	*da = NULL;

	/* check for valid free function */
	if (freefunc == NULL) {
		return NULL;
	}

	da = (dynary *)malloc(sizeof(dynary));
	if (da != NULL) {
		/* init number of cells */
		da->nbcell = 0;

		/* init next index */
		da->nextidx = 0;

		/* allocate array */
		da->pary = malloc(DYNARY_AUTO_GROW * sizeof(void *));
		if (da->pary == NULL ) {
			free(da);
			return NULL;
		}

		/* set functions */
		da->freeobj = freefunc;
		da->cmpobj = cmpfunc;
	}

	return da;
}


/****************
 * da_destroy() *
 ***********************************************************************
 DESCR
	destroy dynamic array

 IN
	da : dynamic array to destroy

 OUT
	NONE
 ***********************************************************************/

void da_destroy(dynary *da) {
	int	i;

	if (da != NULL) {
		/* free all cells */
		for (i = 0 ; i < da->nextidx ; i++) {
			/* using freeing function */
			da->freeobj(da->pary[i]);
		}

		/* free array */
		free(da->pary);

		/* free dynary */
		free(da);
	}
}


/***************
 * da_resize() *
 ***********************************************************************
 DESCR
	Resize the array to the given size

 IN
	da :	array to resize
	nsize :	new size

 OUT
	return : boolean

 TODO
	check size overflow
 ***********************************************************************/

bool da_resize(dynary *da, size_t nsize) {
	void	**tary;

	/* check for valid pointer */
	if (da == NULL) {
		return false;
	}

	/* reallocate array memory to new size */
	tary = realloc(da->pary, nsize * sizeof(void *));
	if (tary != NULL) {
		/* reallocation succeeded, update dynary structure */
		da->pary = tary;
		da->nbcell = nsize;
		return true;
	} else {
		return false;
	}
}


/*************
 * da_size() *
 ***********************************************************************
 DESCR
	Get the size of the array

 IN
	da :	dynamic array

 OUT
	returns number of cell of the array
 ***********************************************************************/

size_t da_size(const dynary *da) {
	if (da == NULL) {
		abort(); /* shouldn't happen */
	}

	return ((size_t) da->nbcell);
}


/**************
 * da_used() *
 ***********************************************************************
 DESCR
	Get the 'used' size of the array

 IN
	da :	dynamic array

 OUT
	returns the number of 'used' cells
 ***********************************************************************/

size_t da_used(const dynary *da) {
	if (da == NULL) {
		abort(); /* shouldn't happen */
	}

	return ((size_t) da->nextidx);
}


/*************
 * da_push() *
 ***********************************************************************
 DESCR
	Add a new value at the end of the array

 IN
	da :	dynamic array
	data :	string to append

 OUT
	return : boolean
 ***********************************************************************/

bool da_push(dynary *da, void *data) {
	size_t	gsize;

	if (da == NULL) {
		return false;
	}

	if (da->nbcell == da->nextidx) {
		/* compute growing size */
		gsize = (da->nbcell + DYNARY_AUTO_GROW);

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return false;
		}
	}

	/* insert in last place */
	da->pary[da->nextidx] = data;
	da->nextidx++;

	return true;
}


/************
 * da_pop() *
 ***********************************************************************
 DESCR
	Pop the last value of the array

 IN
	da : dynamic array

 OUT
	return : the last element or NULL
 ***********************************************************************/

void *da_pop(dynary *da) {
	void	 *p;
	size_t	  gsize;

	if (da == NULL) {
		return NULL;
	}

	if (da->nextidx == 0) {
		/* empty */
		return NULL;
	}

	da->nextidx--;
	p = da->pary[da->nextidx];
	da->pary[da->nextidx] = NULL;

	/* resize if allocated space is too big */
	if ((da->nbcell - da->nextidx) > DYNARY_AUTO_GROW) {
		gsize = da->nbcell - DYNARY_AUTO_GROW;

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return NULL;
		}
	}

	return p;
}


/**************
 * da_shift() *
 ***********************************************************************
 DESCR
	Pop the first value and shift the array

 IN
	da : dynamic array

 OUT
	return : the first cell
 ***********************************************************************/

void *da_shift(dynary *da) {
	void	*p;
	int		 i;
	size_t	 gsize;

	if (da == NULL) {
		return NULL;
	}

	if (da->nextidx == 0) {
		/* empty */
		return NULL;
	}

	p = da->pary[0];
	da->nextidx--;

	/* shift remaining values */
	for (i = 0 ; i < da->nextidx ; i++) {
		da->pary[i] = da->pary[i+1];
	}

	/* clear previous latest element */
	da->pary[da->nextidx] = NULL;

	/* resize if allocated space is too big */
	if ((da->nbcell - da->nextidx) > DYNARY_AUTO_GROW) {
		gsize = da->nbcell - DYNARY_AUTO_GROW;

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return NULL;
		}
	}

	return p;
}


/************
 * da_idx() *
 ***********************************************************************
 DESCR
	Get a value

 IN
	da : dynamic array
	idx : index of the wanted value

 OUT
	returns value or NULL
 ***********************************************************************/

void *da_idx(const dynary *da, int idx) {
	if (da == NULL) {
		return NULL;
	}

	if (idx >= da->nextidx) {
		return NULL;
	}

	return da->pary[idx];
}


/*******************
 * da_find_index() *
 ***********************************************************************
 DESCR
	try to find the index of the first matching data in the given dynary

 IN
 	da :	dynary structure
 	data :	data to find

 OUT
 	cell index or DYNARY_NOT_FOUND
 ***********************************************************************/

int da_find_index(const dynary *da, const void *data) {
	size_t		 i;

	if (da->cmpobj == NULL) {
		abort(); /* shouldn't happen */
	}

	/* for each item of the dynary */
	for (i = 0 ; i < da_used(da) ; i++) {
		/* check if equal to the string */
		if (da->cmpobj(data, da_idx(da, i)) == 0) {
			/* return index */
			return (int) i;
		}
	}

	return DYNARY_NOT_FOUND;
}


/*************
 * da_find() *
 ***********************************************************************
 DESCR
	try to find the first matching data in the given dynary

 IN
 	da :	dynary structure
 	data :	data to find

 OUT
 	boolean relative to the result of the search
 ***********************************************************************/

bool da_find(const dynary *da, const void *data) {
	if (da_find_index(da, data) == DYNARY_NOT_FOUND) {
		return false;
	} else {
		return true;
	}
}


/*************
 * da_sort() *
 ***********************************************************************
 DESCR
	sort the dynary

 IN
 	da :	dynary structure

 OUT
	NONE

 NOTE : this only work with dynaries intialised with da_init()
 ***********************************************************************/

void da_sort(const dynary *da) {
	qsort((void *) da->pary, da->nextidx, sizeof(char *), da->cmpobj);
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
