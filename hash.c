/* $Id$ */

/*
 * Credits for patches :
 *	- Marek Habersack
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


/*******************************************************************
 *                                                                 *
 * Hash-coding functions                                           *
 *                                                                 *
 *******************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "compat/pmk_string.h"

/*#define HASH_DEBUG 1*/

#ifdef HASH_DEBUG
#include "common.h"
#endif

/*
	compute hash (perfect hashing)

	key : key string

	return : resulting hash
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
	init hash structure of character strings

	table_size : number of hash elements

	return : a pointer to the hash structure
*/

htable *hash_init(int table_size) {
	return(hash_init_adv(table_size, free, hash_str_append));
}

/*
	init hash structure of objects

	table_size : number of hash elements
	freefunc : function to free an object
	appdfunc : function to create appended object

	return : a pointer to the hash structure

	NOTE : append function will have three arguments (original object,
	value to append, misc data) and will return the resulting object
	(see hash_append_str for an example).
*/

htable *hash_init_adv(int table_size, void (*freefunc)(void *),
		void *(*appdfunc)(void *, void *, void *)) {
	int	 i;
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
	pht->freeobj = freefunc;
	pht->appdobj = appdfunc;
	pht->nodetab = phn;

	return(pht);
}

/*
	resizing hash table

	ht : hash table to resize
	newsize : new size

	return : boolean 
*/

bool hash_resize(htable *ht, int newsize) {
	hcell	*phc,
		*next;
	hnode	*newhn;
	int	 c = 0,
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

	return : number of keys deleted
*/

int hash_destroy(htable *pht) {
	int	 i,
		 s,
		 c = 0;
	hcell	*p,
		*t;

	if (pht == NULL)
		return(0);

	s = pht->size;

	for(i = 0 ; i < s ; i++) {
		p = pht->nodetab[i].first;
		t = NULL;
		while (p != NULL) {
			t = p->next;
			hash_free_hcell(pht, p);
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
	value : value object

	return : an error code
		HASH_ADD_FAIL : addition failed.
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.
*/

int hash_add(htable *pht, char *key, void *value) {
	int	 rval,
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
		hash_free_hcell(pht, phc);
		return(HASH_ADD_FAIL);
	}
	phc->value = value;

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

	return : an error code
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
				np->value = phc->value;
				phc->value = NULL; /* XXX useless ??? */

				/* new cell no longer needed, free struct only */
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

	return : boolean 
*/

bool hash_add_array(htable *pht, hpair *php, int size) {
	return(hash_add_array_adv(pht, php, size, (void *(*)(void *)) strdup));
}

/*
	add an array into the hash table

	ht : storage hash table
	ary : array to add
	size : size of the array
	dup_func : object duplication function

	return : boolean
*/

bool hash_add_array_adv(htable *pht, hpair *php, int size, void *(dupfunc)(void *)) {
	htable	*pmht;
	int	 i;
	bool	 error = false,
		 rval = false;

	pmht = hash_init(size);
	if (pmht == NULL)
		return(false);

	for (i = 0 ; (i < size) && (error == false) ; i ++) {
		if (hash_add(pmht, php[i].key, dupfunc(php[i].value)) == HASH_ADD_FAIL)
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
	append a value to the existing value

	pht : hash structure
	key : key to be appended
	value : value to append
	misc : extra data to transmit to append function 

	return : an error code.
*/

int hash_append(htable *pht, char *key, void *value, void *misc) {
	void	*pobj,
		*robj;
	int	 rval;

	if (value == NULL) {
#ifdef HASH_DEBUG
		debugf("hash_append : value is null");
#endif
		return(HASH_ADD_FAIL);
	}

	if (pht->appdobj == NULL) {
#ifdef HASH_DEBUG
		debugf("hash_append : appobj is null");
#endif
		return(HASH_ADD_FAIL);
	}

	pobj = hash_get(pht, key);
	if (pobj == NULL) {
		/* no previous value, adding given data */
		rval = hash_add(pht, key, value);
	} else {
		robj = pht->appdobj(pobj, value, misc);
		if (robj != NULL) {
			rval = hash_add(pht, key, robj);
			if (rval == HASH_ADD_UPDT)
				rval = HASH_ADD_APPD; /* not an update as we append */
		} else {
#ifdef HASH_DEBUG
			debugf("hash_append : robj is null");
#endif
			rval = HASH_ADD_FAIL;
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
	int	 hash;
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
			hash_free_hcell(pht, phc);
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

	return :  value or NULL
*/

void *hash_get(htable *pht, char *key) {
	int	 hash;
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

	return : number of merged key
*/

int hash_merge(htable *dst_ht, htable *src_ht) {
	int	 i,
		 s,
		 c = 0;
	hcell	*p;

	/* get table size */
	s = src_ht->size;

	for(i = 0 ; i < s ; i++) {
		p = src_ht->nodetab[i].first;
		while (p != NULL) {
			/* add the key in dst_ht */
			if (hash_add(dst_ht, p->key, p->value) != HASH_ADD_FAIL) {
				/* merged one more key */
				c++;

				/* unset p->value to prevent deletion */
				p->value = NULL;
			}
			p = p->next;
		}
	}

	/* return count of merged keys */
	return(c);
}

/*
	get number of keys

	pht : hash table

	return : number of keys
*/

int hash_nbkey(htable *pht) {
	return(pht->count);
}

/*
	get the keys of the hash table

	pht : hash table

	return : an hkeys structure

	NOTE: don't forget to free the array after use.
*/

hkeys *hash_keys(htable *pht) {
	int	  i,
		  j = 0;
	hcell	 *p;
	hkeys	 *phk;

	/* init hkeys struct to be returned */
	phk = (hkeys *)malloc(sizeof(hkeys));
	if (phk != NULL) {

		if (pht->count == 0)
			return(NULL);

		phk->nkey = pht->count;

		/* create an array with a size of the number of keys */
		phk->keys = (char **)malloc(sizeof(char *) * phk->nkey);
		if (phk->keys != NULL) {
			for(i = 0 ; i < pht->size ; i++) {
				p = pht->nodetab[i].first;
				while (p != NULL) {
					/* add the key in key_ary */
					phk->keys[j] = p->key;
					j++;
					p = p->next;
				}
			}
	
			return(phk);
		} else {
#ifdef HASH_DEBUG
                        debugf("keys alloc failed");
#endif
			/* free allocated space only */
			free(phk);
		}
#ifdef HASH_DEBUG
        } else {
                debugf("hkeys struct alloc failed");
#endif
	}

	return(NULL);
}

/*
	free memory allocated to the hcell structure

	phc : structure to free
*/

void hash_free_hcell(htable *pht, hcell *phc) {
	if (phc != NULL) {
#ifdef HASH_DEBUG
		debugf("free phcell '%s'", phc->key);
#endif
		if (phc->value != NULL) {
			if (pht->freeobj != NULL)
				pht->freeobj(phc->value);
		}
		free(phc);
	}
}

/*
	free memory allocated to the hkeys structure

	phk : structure to free
*/

void hash_free_hkeys(hkeys *phk) {
	/* XXX TODO  also free pointed values */
	free(phk->keys);
	free(phk);
}

/*
	append function for string hash
*/

void *hash_str_append(void *orig, void *value, void *sep) {
	char	*pbuf;
	size_t	 s;

	/* compute needed space */
	if (sep != NULL) {
		s = strlen((char *) sep);
	} else {
		s = 0;
	}
	s = s + strlen((char *) orig) + strlen((char *) value) + 1;

	/* allocate space */
	pbuf = (char *) malloc(s);

	if (strlcpy(pbuf, orig, s) >= s) {
		free(value);
		free(pbuf);
#ifdef HASH_DEBUG
		debugf("hash_str_append : strlcpy1 failed");
#endif
		return(NULL);
	}

	if ((sep != NULL) && (pbuf[0] != '\0')) {
		/* adding separator if provided and if
			string is not empty */
		if (strlcat(pbuf, (char *) sep, s) >= s) {
			free(value);
			free(pbuf);
#ifdef HASH_DEBUG
		debugf("hash_str_append : strlcat1 failed");
#endif
			return(NULL);
		}
	}
	if (strlcat(pbuf, value, s) >= s) {
		free(value);
		free(pbuf);
#ifdef HASH_DEBUG
		debugf("hash_str_append : strlcat2 failed");
#endif
		return(NULL);
	}

	free(value);
	return((void *) pbuf);
}
