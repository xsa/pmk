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

#include "hash.h"
#include "compat/pmk_string.h"


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
	pht->autogrow = 0; /* disabled by default */
	pht->nodetab = phn;

	return(pht);
}

/*
	resizing hash table

	ht : hash table to resize
	newsize : new size

	returns 1 on success else 0
*/

bool hash_resize(htable *ht, int newsize) {
	hcell	*phc,
		*next;
	hnode	*newhn;
	int	c = 0,
		h,
		i;

	/* allocate new node table */
	newhn = (hnode *) malloc(sizeof(hnode) * newsize);
	if (newhn == NULL)
		return(false);

	/* init */
	for (i = 0 ; i < newsize ; i++) {
		newhn[i].first = NULL;
		newhn[i].last = NULL;
	}

	for (i = 0 ; i < ht->size ; i++) {
		phc = ht->nodetab[i].first;
		while (phc != NULL) {
			h = hash_compute(phc->key, newsize);
			next = phc->next;
			hash_add_cell(&newhn[h], phc);
			phc = next;
			c++;
		}

	}

	free(ht->nodetab);
	ht->nodetab = newhn;
	ht->size = newsize;

	return(true);
}

/*
	set hash table as automatically resizable

	ht : hash table to set
*/

void hash_set_grow(htable *ht) {
	ht->autogrow = 1;
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
	pht = NULL;

	return(c);
}

/*
	add a key in the hash table

	pht : hash structure
	key : key string
	value : value string

	returns an error code :
		HASH_ADD_FAIL : addition failed.
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.
*/

int hash_add(htable *pht, char *key, char *value) {
	int	rval,
		hash,
		size;
	hnode	*phn;
	hcell	*phc = NULL;

	rval = HASH_ADD_FAIL;

	size = pht->size;

	/* test remaing free cells */
	if (pht->count >= size) {
		/* number of cell is higher than the supposed table size */
		if (pht->autogrow == 0) {
			/* fixed size */
			return(HASH_ADD_FAIL);
		} else {
			/* resize the hash table (long) */
			if (hash_resize(pht, size * 2) == false) {
				/* cannot resize the hash */
				return(HASH_ADD_FAIL);
			}
		}
	}

	
	phc = (hcell *) malloc(sizeof(hcell));
	if (phc == NULL)
		return(rval);

	/* if okay put key & value in a new cell */
	if (strlcpy(phc->key, key, sizeof(phc->key)) >= sizeof(phc->key)) {
		free(phc);
		return(HASH_ADD_FAIL);
	}
	if (strlcpy(phc->value, value, sizeof(phc->value)) >= sizeof(phc->value)) {
		free(phc);
		return(HASH_ADD_FAIL);
	}

	/* compute hash code */
	hash = hash_compute(key, size);

	phn = &pht->nodetab[hash];
	rval = hash_add_cell(phn, phc);

	if (rval == HASH_ADD_OKAY || rval == HASH_ADD_COLL)
		pht->count++;

	return(rval);
}

/*
	add a new cell in a node

	phn : storage node
	phc : cell to add

	returns an error code :
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.

	NOTE : this function does not increment the cell number of the hash table.
*/

int hash_add_cell(hnode *phn, hcell *phc) {
	hcell	*np;

	phc->next = NULL;
	if (phn->first == NULL) {
		/* hash code unused */
		phn->first = phc;
		phn->last = phc;

		return(HASH_ADD_OKAY);
	} else {
		/* collision, hash code already used */
		np = phn->first;

		/* looking for last element */
		while (1) {
			if (strncmp(phc->key, np->key, sizeof(phc->key)) == 0) {
				/* key already exists */
				strlcpy(np->value, phc->value, sizeof(np->value));
				/* new cell no longer needed */
				free(phc);
				return(HASH_ADD_UPDT);
			} else {
				if (np->next == NULL) {
					/* last element found */
					phn->last = phc;
					/* chaining new cell */
					np->next = phc;

					return(HASH_ADD_COLL);
				} else {
					np = np->next;
				}
			}
		}
	}
}

/*
	add an array into the hash table

	ht : storage hash table
	ary : array to add
	size : size of the array

	returns 1 on succes else 0
*/

bool hash_add_array(htable *pht, hpair *php, int size) {
	htable	*pmht;
	int	i;
	bool	error = false,
		rval = false;

	pmht = hash_init(size);
	if (pmht == NULL)
		return(false);

	for (i = 0 ; (i < size) && (error == false) ; i ++) {
		if (hash_add(pmht, php[i].key, php[i].value) == HASH_ADD_FAIL)
			error = true;
	}

	if (error == false) {
		hash_merge(pht, pmht);
		rval = true;
	}

	hash_destroy(pmht);

	return(rval);
}

/*
	append a value to the existing

	pht : hash structure
	key : key string
	value : value string
	sep : separator (can be NULL)

	returns an error code.

	NOTE : the separator is only used while appending, if the key doesn't exists
		then only the value is added.
*/

int hash_append(htable *pht, char *key, char *value, char *sep) {
	char	*pstr,
		buf[MAX_HASH_VALUE_LEN] = "";
	int	rval,
		s;

	pstr = hash_get(pht, key);
	if (pstr == NULL) {
		rval = hash_add(pht, key, value);
	} else {
		s = sizeof(buf);
		if (strlcat(buf, pstr, s) >= s)
			return(HASH_ADD_FAIL);
		if ((sep != NULL) && (buf[0] != '\0')) {
			/* adding separator if provided and if
				string is not empty */
			if (strlcat(buf, sep, s) >= s)
				return(HASH_ADD_FAIL);
		}
		if (strlcat(buf, value, s) >= s)
			return(HASH_ADD_FAIL);

		rval = hash_add(pht, key, buf);
		if (rval == HASH_ADD_UPDT)
			rval = HASH_ADD_APPD; /* not an update as we append */
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
				if (phc->next == NULL) {
					/* only one cell */
					pht->nodetab[hash].first = NULL;
					pht->nodetab[hash].last = NULL;
				} else {
					/* re-link with next cell */
					pht->nodetab[hash].first = phc->next;
				}
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
			return(phc->value);
		} else {
			/* go to next cell */
			phc = phc->next;
		}
	}

	/* key not found */
	return(NULL);
}

/*
	merge one hash table into another

	dst_ht : destination table
	src_ht : table to merge

	returns number of merged key
*/

int hash_merge(htable *dst_ht, htable *src_ht) {
	int	i,
		s,
		c = 0;
	hcell	*p;

	/* get table size */
	s = src_ht->size;

	for(i = 0 ; i < s ; i++) {
		p = src_ht->nodetab[i].first;
		while (p != NULL) {
			/* add the key in dst_ht */
			if (hash_add(dst_ht, p->key, p->value) != HASH_ADD_FAIL)
				c++; /* merged one more key */
			p = p->next;
		}
	}

	/* return count of merged keys */
	return(c);
}

/*
	get number of keys

	pht : hash table

	returns number of keys
*/

int hash_nbkey(htable *pht) {
	return(pht->count);
}

/*
	get the keys of the hash table

	pht : hash table

	returns an array of keys
*/

char **hash_keys(htable *pht) {
	char	**key_ary;
	int	nbk,
		i,
		j = 0;
	hcell	*p;

	nbk = pht->count;

	/* create an array with a size of the number of keys */
	key_ary = (char **)malloc(sizeof(char *) * nbk);
	if (key_ary != NULL) {
		for(i = 0 ; i < pht->size ; i++) {
			p = pht->nodetab[i].first;
			while (p != NULL) {
				/* add the key in key_ary */
				key_ary[j] = p->key;
				j++;
				p = p->next;
			}
		}
	}

	return(key_ary);
}
