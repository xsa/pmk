/* $Id$ */

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

/*
 * Credits for patches :
 *	- Ted Unangst
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


/*****************
 * da_init_adv() *
 ***********************************************************************
 DESCR
	Initialise dynamic array and provide specific free function

 IN
	freefunc : free function

 OUT
	returns pointer of dynamic array
 ***********************************************************************/

dynary *da_init_adv(void (*freefunc)(void *)) {
	dynary	*da = NULL;

	da = (dynary *)malloc(sizeof(dynary));
	if (da != NULL) {
		da->nbcell = 0;
		da->nextidx = 0;
		da->pary = malloc(DYNARY_AUTO_GROW * sizeof(void *));
		if (freefunc != NULL) {
			da->freeobj = freefunc;
		} else {
			da->freeobj = free;
		}
	}

	return(da);
}


/*************
 * da_init() *
 ***********************************************************************
 DESCR
	Initialise dynamic array

 IN
	NONE

 OUT
	returns pointer of dynamic array
 ***********************************************************************/

dynary *da_init(void) {
	return(da_init_adv(NULL));
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
		for (i = 0 ; i < da->nextidx ; i++) {
			da->freeobj(da->pary[i]);
		}
		free(da->pary);
		free(da);
	}
}


/***************
 * da_resize() *
 ***********************************************************************
 DESCR
	Resize the array to the given size

 IN
	da : array to resize
	nsize : new size

 OUT
	return : boolean
 ***********************************************************************/

bool da_resize(dynary *da, size_t nsize) {
	void	**tary;

	if (da == NULL)
		return(false);

	tary = realloc(da->pary, nsize * sizeof(void *));
	if (tary != NULL) {
		da->pary = tary;
		da->nbcell = nsize;
		return(true);
	} else {
		return(false);
	}
}


/*************
 * da_size() *
 ***********************************************************************
 DESCR
	Get the size of the array

 IN
	NONE

 OUT
	returns number of cell of the array
 ***********************************************************************/

size_t da_size(dynary *da) {
	if (da == NULL)
		abort(); /* shouldn't happen */

	return((size_t) da->nbcell);
}


/**************
 * da_usize() *
 ***********************************************************************
 DESCR
	Get the 'used' size of the array

 IN
	NONE

 OUT
	returns the number of 'used' cells
 ***********************************************************************/

size_t da_usize(dynary *da) {
	if (da == NULL)
		abort(); /* shouldn't happen */

	return((size_t) da->nextidx);
}


/*************
 * da_push() *
 ***********************************************************************
 DESCR
	Add a new value at the end of the array

 IN
	da : dynamic array
	pval : string to append

 OUT
	return : boolean
 ***********************************************************************/

bool da_push(dynary *da, void *pval) {
	size_t	gsize;

	if (da == NULL)
		return(false);

	if (da->nbcell == da->nextidx) {
		/* compute growing size */
		gsize = (da->nbcell + DYNARY_AUTO_GROW);

		if (da_resize(da, gsize) == false) {
			/* cannot resize dynary */
			return(false);
		}
	}

	/* insert in last place */
	da->pary[da->nextidx] = pval;
	da->nextidx++;

	return(true);
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

	if (da == NULL)
		return(NULL);

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

	if (da == NULL)
		return(NULL);

	if (da->nextidx == 0) {
		/* empty */
		return(NULL);
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
			return (NULL);
		}
	}

	return(p);
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

void *da_idx(dynary *da, int idx) {
	if (da == NULL)
		return(NULL);

	if (idx >= da->nextidx) {
		return(NULL);
	}

	return(da->pary[idx]);
}


/*************
 * da_find() *
 ***********************************************************************
 DESCR
	try to find an element in the given dynary

 IN
 	da :	dynary structure
 	str :	string to find

 OUT
 	boolean relative to the result of the search

 NOTE : this only work with dynaries intialised with da_init()
 ***********************************************************************/

bool da_find(dynary *da, char *str) {
	bool		 rslt = false;
	size_t		 i,
				 s;

	/* compute size of string including the delimiter */
	s = strlen(str) + 1;

	/* for each item of the dynary */
	for (i = 0 ; i < da_usize(da) ; i++) {
		/* check if equal to the string */
		if (strncmp(str, da_idx(da, i), s) == 0) {
			/* and set the flag if true */
			rslt = true;
			break;
		}
	}

	return(rslt);
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

static int da_strcmp(const void *a, const void *b) {
	return(strcmp(*(char * const *)a, *(char * const *) b));
}

void da_sort(dynary *da) {
	qsort((void *) da->pary, da->nextidx, sizeof(char *), da_strcmp);
}

