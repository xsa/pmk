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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"


/*
	compute hash (perfect hashing)

	key : key string

	returns : resulting hash
*/

int hash_compute(char *key, int table_size) {
	int		len = 0,
			hash = 0;
	unsigned char	c;
	
	c = *key;
	while (c != '\0') {
		/* sum all characters with modulo */
		hash = (hash + (c)) % table_size;
		c = *key;
		key++;
		len++;
	}

	/* add the len always with modulo */
	hash = (hash + len) % table_size;

	return(hash);
}

/*
	init hash structure

	table_size : number of hash elements

	returns a pointer to the hash structure
*/

htable *hash_init(int table_size) {
	int	i;
	htable	*pht;
	hnode	*phn;

	/* allocate hash table */
	pht = (htable *) malloc(sizeof(htable));
	if (pht != NULL) {
		/* allocate node table */
		phn = (hnode *) malloc(sizeof(hnode) * table_size);
		if (phn == NULL) {
			/* allocation failed then
				deallocate and return NULL */
			free(pht);
			return(NULL);
		}
	} else {
		/* allocation failed, return NULL */
		return(NULL);
	}

	for(i = 0 ; i < table_size ; i++) {
		phn[i].first = NULL;
		phn[i].last = NULL;
	}

	/* finish init */
	pht->size = table_size;
	pht->count = 0;
	pht->nodetab = phn;

	return(pht);
}

/*
	destroy hash table

	pht : hash table to remove

	returns number of keys deleted
*/

int hash_destroy(htable *pht) {
	int	i,
		s,
		c = 0;
	hcell	*p,
		*t;

	s = pht->size;

	for(i = 0 ; i < s ; i++) {
		p = pht->nodetab[i].first;
		t = NULL;
		while (p != NULL) {
			t = p->next;
			free(p);
			c++; /* removed one more key */
			p = t;
		}
	}

	free(pht->nodetab);
	free(pht);

	return(c);
}

/*
	add a key

	pht : hash structure
	key : key string
	value : value string
*/

int hash_add(htable *pht, char *key, char *value) {
	int	rval,
		hash,
		size,
		exit,
		i;
	hnode	*phn = NULL;
	hcell	*phc = NULL,
		*np = NULL;

	rval = HASH_ADD_FAIL;

	size = pht->size;
	/* compute hash code */
	hash = hash_compute(key, size);

	/* test remaing free cells */
	i = pht->count;
	if (i >= size) {
		/* number of cell is higher than the supposed table size */
		return(0);
	}

	
	/* if okay put key & value in a new cell */
	phc = (hcell *) malloc(sizeof(hcell));
	strncpy(phc->key, key, MAX_HASH_KEY_LEN);
	strncpy(phc->value, value, MAX_HASH_VALUE_LEN);
	phc->next = NULL;

	phn = pht->nodetab;

	if (phn[hash].first == NULL) {
		/* hash code unused */
		phn[hash].first = phc;
		phn[hash].last = phc;

		pht->count++;
		rval = HASH_ADD_OKAY;
	} else {
		/* collision, hash code already used */
		np = phn[hash].first;
		exit = 0;
		/* looking for last element */
		while (exit == 0) {
			if (strncmp(key, np->key, MAX_HASH_KEY_LEN) == 0) {
				/* key already exists */
				strncpy(np->value, value, MAX_HASH_VALUE_LEN);
				rval = HASH_ADD_UPDT;
				exit = 1;
				/* new cell no longer needed */
				free(phc);
			} else {
				if (np->next == NULL) {
					/* last element found */
					phn[hash].last = phc;
					/* chaining new cell */
					np->next = phc;

					pht->count++;
					rval = HASH_ADD_COLL;
					exit = 1;
				} else {
					np = np->next;
				}
			}
		}
	}

	return(rval);
}

/*

	remove a key from the hash table

	pht : hash table
	key : key to search
*/

void hash_delete(htable *pht, char *key) {
	int	hash;
	hcell	*phc,
		*last;

	/* compute hash code */
	hash = hash_compute(key, pht->size);

	phc = pht->nodetab[hash].first;
	last = NULL;
	while (phc != NULL) {
		/* hash not empty */
		if (strncmp(key, phc->key, MAX_HASH_KEY_LEN) == 0) {
			/* found key */
			if (last == NULL) {
				/* first cell */
				pht->nodetab[hash].first = NULL;
				pht->nodetab[hash].last = NULL;
			} else {
				last->next = phc->next;
				if (phc->next == NULL) {
					/* delete last cell, update node */
					pht->nodetab[hash].last = last;
				}
			}
			free(phc);
			phc = NULL;

			pht->count--;
		} else {
			/* go to next cell */
			last = phc;
			phc = phc->next;
		}
	}
	/* key not found */
}

/*
	get a key from the hash table

	pht : hash table
	key : key to search

	returns the value or NULL
*/

char *hash_get(htable *pht, char *key) {
	int	hash;
	hcell	*phc;

	/* compute hash code */
	hash = hash_compute(key, pht->size);

	phc = pht->nodetab[hash].first;
	while (phc != NULL) {
		/* hash not empty */
		if (strncmp(key, phc->key, MAX_HASH_KEY_LEN) == 0) {
			/* found key, return pointer on value */
			return(phc->key);
		} else {
			/* go to next cell */
			phc = phc->next;
		}
	}

	/* key not found */
	return(NULL);
}
