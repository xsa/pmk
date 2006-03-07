/* $Id$ */

/*
 * Copyright (c) 2003-2006 Damien Couderc
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


#include <stdlib.h>

#include "hash.h"
#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"

/*#define PMK_HASH_DEBUG 1*/

#ifdef PMK_HASH_DEBUG
#include "common.h"
#endif


/**************
 * basic tools *
 ***********************************************************************/

/******************
 * hash_compute() *
 ***********************************************************************
 DESCR
	compute hash value of the key

 IN
	key :	key string
	tsize :	table size

 OUT
	resulting hash
 ***********************************************************************/

size_t hash_compute(char *key, size_t tsize) {
	size_t			 len = 0,
					 hash = 0;
	unsigned char	*puc;

	/* force unsigned values */
	puc = (unsigned char *) key;

	while (*puc != '\0') {
		/* sum all characters with modulo */
		hash = (hash + (size_t) *puc) % tsize;
		puc++;
		len++;
	}

	/* add the len always with modulo */
	hash = (hash + len) % tsize;

#ifdef PMK_HASH_DEBUG
	debugf("hash_compute() : hash = %zd", hash);
#endif

	return(hash);
}


/*********************
 * hash_size_check() *
 ***********************************************************************
 DESCR
	X

 IN
	X

 OUT
	X
 ***********************************************************************/

herr hash_size_check(htable *pht) {
	/* test remaing free cells */
	if (pht->count >= pht->size) {
		/* number of cell is higher than the supposed table size */
		if (pht->autogrow == false) {
			/* fixed size */

#ifdef PMK_HASH_DEBUG
			debugf("hash_size_check() : hash is full");
#endif
			return(HASH_ADD_FAIL);
		} else {
			/* resize the hash table (long) */
			if (hash_resize(pht, pht->size * 2) == false) {
				/* cannot resize the hash */

#ifdef PMK_HASH_DEBUG
				debugf("hash_size_check() : hash resize failed");
#endif

				return(HASH_ADD_FAIL);
			}
		}
	}

	return(HASH_ADD_OKAY);
}


/**************************
 * hash_node_array_init() *
 ***********************************************************************
 DESCR
	X

 IN
	X

 OUT
	X
 ***********************************************************************/

hnode *hash_node_array_init(size_t table_size) {
	hnode	*phn;
	size_t	 i;

	/* allocate node table */
	phn = (hnode *) malloc(sizeof(hnode) * table_size);
	if (phn != NULL) {
		/* initialise array */
		for(i = 0 ; i < table_size ; i++) {
			phn[i].first = NULL;
			phn[i].last = NULL;
		}
	}

	return(phn);
}


/********************
 * hash_node_seek() *
 ***********************************************************************
 DESCR
	get the node corresponding to the given key

 IN
	pht :	hash table pointer
	key :	key name

 OUT
	hash node pointer or NULL
 ***********************************************************************/

hnode *hash_node_seek(htable *pht, char *key) {
	size_t	hash = 0;

	/* compute hash code */
	hash = hash_compute(key, pht->size);

	return(&(pht->nodetab[hash]));
}


/********************
 * hash_cell_init() *
 ***********************************************************************
 DESCR
	allocate and initialise hash cell

 IN
	key :	key name
	value :	key's associated data

 OUT
	pointer to the generated cell or NULL
 ***********************************************************************/

hcell *hash_cell_init(char *key, void *data) {
	hcell	*phc;

	phc = (hcell *) malloc(sizeof(hcell));
	if (phc != NULL) {
		/* set the key name of the cell */
		if (strlcpy_b(phc->key, key, sizeof(phc->key)) == false) {
			free(phc);
			return(NULL);
		}

		/* set the pointer to the data */
		phc->value = data;
	}

	return(phc);
}


/********************
 * hash_cell_seek() *
 ***********************************************************************
 DESCR
	seek for an existing cell for the given key in the given node

 IN
	phn :	node where to add the cell
	key :	key name

 OUT
	pointer to the cell or NULL
 ***********************************************************************/

hcell *hash_cell_seek(hnode *phn, char *key) {
	hcell	*phc;

	phc = phn->first;

	/* while not at the end of the cell list */
	while (phc != NULL) {
#ifdef PMK_HASH_DEBUG
		debugf("hash_cell_seek() : comparing '%s' with '%s'", key, phc->key);
#endif

		if (strncmp(key, phc->key, MAX_HASH_KEY_LEN) == 0) {
			/* key is matching, exit loop */

#ifdef PMK_HASH_DEBUG
		debugf("hash_cell_seek() : found key '%s'", key);
#endif

			break;
		} else {
			/* no matching, go to next cell */
			phc = phc->next;
		}
	}

	/* return cell or NULL */
	return(phc);
}


/*******************
 * hash_cell_add() *
 ***********************************************************************
 DESCR
	adds a cell to a node using the given method

 IN
	phn :	node where to add the cell
	phc :	pointer to the cell to add
	hmtd :	method to use
				HASH_METHOD_ADD => add only, return error if existing
				HASH_METHOD_UPD => add or update if already exists

 OUT
	return an error code
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.

 NOTE
	this function does not increment the cell number of the hash table.
 ***********************************************************************/

herr hash_cell_add(htable *pht, hcell *phc, hmtd method) {
	hcell	*pseek,
			*ptmp;
	herr	 err = HASH_ADD_FAIL;
	hnode	*phn;

	/* get node */
	phn = hash_node_seek(pht, phc->key);

	/* node is not empty => collision, now looking for a matching cell */
	pseek = hash_cell_seek(phn, phc->key);
	if (pseek != NULL) {
#ifdef PMK_HASH_DEBUG
		debugf("hash_cell_add() : found existing key '%s' in the node", phc->key);
#endif

		/* handle add method */
		switch (method) {
			case HASH_METHOD_ADD :
				/* add only, do not update */

#ifdef PMK_HASH_DEBUG
				debugf("hash_cell_add() : HASH_METHOD_ADD");
#endif

				err = HASH_ADD_FAIL;
				break;

			case HASH_METHOD_UPD :
				/* found matching cell, replace it by new cell */

#ifdef PMK_HASH_DEBUG
				debugf("hash_cell_add() : HASH_METHOD_UPD");
#endif

				ptmp = pseek->prev;
				if (ptmp == NULL) {
					/* first cell of the node */
					phn->first = phc;
#ifdef PMK_HASH_DEBUG
					debugf("hash_cell_add() : updated key '%s' as first of the node", phc->key);
#endif
				} else {
					/* else link to previous */
					ptmp->next = phc;
					phc->prev = ptmp;
				}

				ptmp = pseek->next;
				if (ptmp == NULL) {
					/* last cell of the node */
					phn->last = phc;
#ifdef PMK_HASH_DEBUG
					debugf("hash_cell_add() : updated key '%s' as last of the node", phc->key);
#endif
				} else {
					/* else link to next */
					ptmp->prev = phc;
					phc->next = ptmp;
				}

				/* now we can remove old cell */
				hash_cell_destroy(pht, pseek);

				err = HASH_ADD_UPDT;
				break;

			default :
				err = HASH_ADD_FAIL;
		}
	} else {
		/*
			no matching, linking to last cell
		*/

		/* get last cell */
		ptmp = phn->last;

		if (ptmp == NULL) {
			/* empty node, add as first cell */
			phn->first = phc;

			err = HASH_ADD_OKAY;

#ifdef PMK_HASH_DEBUG
			debugf("hash_cell_add() : added key '%s' as first of the node", phc->key);
#endif
		} else {
			/* link new cell as next */
			ptmp->next = phc;
	
			/* set link to previous */
			phc->prev = ptmp;

			err = HASH_ADD_COLL;
		}

		/* update node's last cell */
		phn->last = phc;

		/* update cell counter */
		pht->count++;

#ifdef PMK_HASH_DEBUG
		debugf("hash_cell_add() : added key '%s' in the node", phc->key);
#endif
	}

	return(err);
}


/***********************
 * hash_cell_extract() *
 ***********************************************************************
 DESCR
	extract a cell from the given node if it matches the given key

 IN
	phn :	node where to add the cell
	key :	key name

 OUT
	pointer to the cell or NULL
 ***********************************************************************/

hcell *hash_cell_extract(hnode *phn, char *key) {
	hcell	*phc,
			*prev,
			*next;

	/* looking for a matching cell */
	phc = hash_cell_seek(phn, key);

	/* if a matching cell has been found */
	if (phc != NULL) {
		prev = phc->prev;
		next = phc->next;

		/* if first cell of the list */
		if (prev == NULL) {
			/* update first cell of node */
			phn->first = next;

			/* link next cell as first cell if needed */
			if (next != NULL) {
				next->prev = NULL;
			}
		}

		/* if last cell of the list */
		if (next == NULL) {
			/* update last cell of node */
			phn->last = prev;

			/* link previous cell as first cell if needed */
			if (prev != NULL) {
				prev->next = NULL;
			}
		}
	}

	/* return cell pointer or NULL */
	return(phc);
}


/***********************
 * hash_cell_destroy() *
 ***********************************************************************
 DESCR
	destroy the given cell using hash table recorded method

 IN
	pht :	hash table pointer
	phc :	pointer to the cell to destroy

 OUT
	NONE
 ***********************************************************************/

void *hash_cell_destroy(htable *pht, hcell *phc) {
	if (phc != NULL) {
#ifdef PMK_HASH_DEBUG
		debugf("hash_cell_destroy() : destroying hash cell of key '%s'", phc->key);
#endif
/*debugf("value = '%s'", phc->value);*/

		/* if value is set and freeing function has been provided */
		if ((phc->value != NULL) && (pht->freeobj != NULL)) {
			/* free key data */
			pht->freeobj(phc->value);
		}

		/* free the cell structure */
		free(phc);

#ifdef PMK_HASH_DEBUG
	} else {
		debugf("tried to destroy cell pointed by NULL !");
#endif
	}
}


/*************
 * misc tools *
 ***********************************************************************/


/* XXX */


/******************
 * main procedures *
 ***********************************************************************/


/*******************
 * hash_init_adv() *
 ***********************************************************************
 DESCR
	init hash structure of objects

 IN
	table_size : number of hash elements
	dupfunc : function to duplicate an object
	freefunc : function to free an object
	appdfunc : function to create appended object

 OUT
	a pointer to the hash structure

 NOTE
	append function will have three arguments (original object,
	value to append, misc data) and will return the resulting object
	(see hash_append_str for an example).
 ***********************************************************************/

htable *hash_init_adv(size_t table_size, void *(*dupfunc)(void *),
							void (*freefunc)(void *),
							void *(*appdfunc)(void *, void *, void *)) {

	htable	*pht;
	hnode	*phn;
	size_t	 i;

	/* allocate hash table */
	pht = (htable *) malloc(sizeof(htable));
	if (pht == NULL) {
		/* allocation failed, return NULL */
		return(NULL);
	}

	/* allocate node table */
	phn = hash_node_array_init(table_size);
	if (phn == NULL) {
		/* allocation failed, deallocate hash table and return */
		free(pht);
		return(NULL);
	}

	/* finish init */
	pht->size = table_size;
	pht->count = 0;
	pht->autogrow = false; /* disabled by default */
	pht->dupobj = dupfunc;
	pht->freeobj = freefunc;
	pht->appdobj = appdfunc;
	pht->nodetab = phn;

	return(pht);
}


/***************
 * hash_init() *
 ***********************************************************************
 DESCR
	init hash structure of character strings

 IN
	table_size : number of hash elements

 OUT
	 a pointer to the hash structure
 ***********************************************************************/

htable *hash_init(size_t table_size) {
	return(hash_init_adv(table_size, (void *(*)(void *))strdup,	free, hash_str_append));
}


/*****************
 * hash_resize() *
 ***********************************************************************
 DESCR
	resizing hash table

 IN
	ht : hash table to resize
	newsize : new size

 OUT
	boolean
 ***********************************************************************/

bool hash_resize(htable *pht, size_t newsize) {
	/*hcell			*phc,                                                  */
	/*                *next;                                               */
	/*hnode			*newhn;                                                */
	/*size_t			 c = 0,                                            */
	/*                 h,                                                  */
	/*                 i;                                                  */
	/*                                                                     */
	/*|+ allocate new node table +|                                        */
	/*newhn = hash_node_array_init(newsize);                               */
	/*if (newhn == NULL)                                                   */
	/*    return(false);                                                   */
	/*                                                                     */
	/*for (i = 0 ; i < pht->size ; i++) {                                  */
	/*    phc = pht->nodetab[i].first;                                     */
	/*    while (phc != NULL) {                                            */
	/*        h = hash_compute(phc->key, newsize);                         */
	/*        next = phc->next;                                            */
	/*        phc->prev = NULL;                                            */
	/*        phc->next = NULL;                                            */
	/*        hash_cell_add(&newhn[h], phc, HASH_METHOD_ADD); |+ XXX ARG +|*/
	/*        phc = next;                                                  */
	/*        c++;                                                         */
	/*    }                                                                */
	/*                                                                     */
	/*}                                                                    */
	/*                                                                     */
	/*free(pht->nodetab);                                                  */
	/*pht->nodetab = newhn;                                                */
	/*pht->size = newsize;                                                 */
	/*                                                                     */
	/*return(true);                                                        */

	return(false); /* XXX TO REWRITE !!! */
}


/*******************
 * hash_set_grow() *
 ***********************************************************************
 DESCR
	set hash table as automatically resizable

 IN
	ht : hash table to set

 OUT
	-
 ***********************************************************************/

void hash_set_grow(htable *ht) {
	ht->autogrow = true;
}


/******************
 * hash_destroy() *
 ***********************************************************************
 DESCR
	destroy hash table

 IN
	pht : hash table to remove

 OUT
	number of keys deleted
 ***********************************************************************/

size_t hash_destroy(htable *pht) {
	hcell			*p,
					*t;
	size_t			 s,
					 c = 0;
	unsigned int	 i;

	if (pht == NULL)
		return(0);

	s = pht->size;

	for(i = 0 ; i < s ; i++) {
		p = pht->nodetab[i].first;
		t = NULL;
		while (p != NULL) {
			t = p->next;
			hash_cell_destroy(pht, p);
			c++; /* removed one more key */
			p = t;
		}
	}

	free(pht->nodetab);
	free(pht);
	pht = NULL;

	return(c);
}


/**************
 * hash_add() *
 ***********************************************************************
 DESCR
	add a key in the hash table

 IN
	pht : hash structure
	key : key string
	value : value object

 OUT
	return an error code
		HASH_ADD_FAIL : addition failed.
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.
 ***********************************************************************/

/* XXX transform to bool with global variable hash_error ? */
herr hash_add(htable *pht, char *key, void *value) {
	hcell	*phc;
	herr	 err;

	/* check size */
	err = hash_size_check(pht);
	if (err != HASH_ADD_OKAY) {
		return(err);
	}

	phc = hash_cell_init(key, value);
	if (phc == NULL) {
		return(HASH_ADD_FAIL);
	}

	err = hash_cell_add(pht, phc, HASH_METHOD_ADD);

	return(err);
}


/*****************
 * hash_update() *
 ***********************************************************************
 DESCR
	update or add a key in the hash table

 IN
	pht : hash structure
	key : key string
	value : value object

 OUT
	return an error code
		HASH_ADD_FAIL : addition failed.
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.
 ***********************************************************************/

/* XXX transform to bool with global variable hash_error ? */
herr hash_update(htable *pht, char *key, void *value) {
	hcell	*phc;
	herr	 err;

	/* check size */
	err = hash_size_check(pht);
	if (err != HASH_ADD_OKAY) {
		return(err);
	}

	phc = hash_cell_init(key, value);
	if (phc == NULL) {
		return(HASH_ADD_FAIL);
	}

	err = hash_cell_add(pht, phc, HASH_METHOD_UPD);

	return(err);
}


/*********************
 * hash_update_dup() *
 ***********************************************************************
 DESCR
	update or add a key in the hash table but duplicate the value

 IN
	pht : hash structure
	key : key string
	value : value object

 OUT
	return an error code
		HASH_ADD_FAIL : addition failed.
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.
 ***********************************************************************/

herr hash_update_dup(htable *pht, char *key, void *value) {
	return(hash_update(pht, key, pht->dupobj(value)));
}


/********************
 * hash_add_array() *
 ***********************************************************************
 DESCR
	add an array into the hash table

 IN
	ht : storage hash table
	ary : array to add
	size : size of the array

 OUT
	boolean
 ***********************************************************************/

bool hash_add_array(htable *pht, hpair *php, size_t size) {
	return(hash_add_array_adv(pht, php, size, (void *(*)(void *)) strdup));
}

/************************
 * hash_add_array_adv() *
 ***********************************************************************
 DESCR
	add an array into the hash table

 IN
	ht : storage hash table
	ary : array to add
	size : size of the array
	dup_func : object duplication function

 OUT
	boolean
 ***********************************************************************/

bool hash_add_array_adv(htable *pht, hpair *php, size_t size, void *(dupfunc)(void *)) {
	bool	 error = false,
			 rval = false;
	htable	*pmht;
	size_t	 i;

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


/*****************
 * hash_append() *
 ***********************************************************************
 DESCR
	append a value to the existing value

 IN
	pht : hash structure
	key : key to be appended
	value : value to append
	misc : extra data to transmit to append function

 OUT
	return an error code.
		HASH_ADD_OKAY : added (no collision).
		HASH_ADD_COLL : added (collision, key chained).
		HASH_ADD_UPDT : key already exists, change value.
 ***********************************************************************/

herr hash_append(htable *pht, char *key, void *value, void *misc) {
	herr	 rval;
	void	*pobj,
			*robj;

	if (value == NULL) {
#ifdef PMK_HASH_DEBUG
		debugf("hash_append : value is null");
#endif
		return(HASH_ADD_FAIL);
	}

	if (pht->appdobj == NULL) {
#ifdef PMK_HASH_DEBUG
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
#ifdef PMK_HASH_DEBUG
			debugf("hash_append : robj is not null");
#endif
			rval = hash_update(pht, key, robj);

			if (rval == HASH_ADD_UPDT) {
				rval = HASH_ADD_APPD; /* not an update as we append */
			}
		} else {
#ifdef PMK_HASH_DEBUG
			debugf("hash_append : robj is null");
#endif
			rval = HASH_ADD_FAIL;
		}
	}

	return(rval);
}


/*****************
 * hash_delete() *
 ***********************************************************************
 DESCR
	remove a key from the hash table

 IN
	pht : hash table
	key : key to search

 OUT
	-
 ***********************************************************************/

void hash_delete(htable *pht, char *key) {
	hcell	*phc;
	hnode	*phn;

	/* get the key node */
	phn = hash_node_seek(pht, key);

	/* looking for a matching cell */
	phc = hash_cell_extract(phn, key);
	if (phc != NULL) {
		/* if found, delete cell */
		hash_cell_destroy(pht, phc);
		
		/* update cell counter */
		pht->count--;
	}
}


/******************
 * hash_extract() *
 ***********************************************************************
 DESCR
	remove a key from the hash table

 IN
	pht : hash table
	key : key to search

 OUT
	-
 ***********************************************************************/

void *hash_extract(htable *pht, char *key) {
	hcell	*phc;
	hnode	*phn;
	void	*data = NULL;

	/* get the key node */
	phn = hash_node_seek(pht, key);

	/* looking for a matching cell */
	phc = hash_cell_extract(phn, key);

	/* if a key has been found */
	if (phc != NULL) {
		/* get data */
		data = phc->value;

		/* unlink data from cell */
		phc->value = NULL;

		/* delete useless cell */
		hash_cell_destroy(pht, phc);
		
		/* update cell counter */
		pht->count--;
	}

	/* return data or NULL */
	return(data);
}


/**************
 * hash_get() *
 ***********************************************************************
 DESCR
	get a key from the hash table

 IN
	pht : hash table
	key : key to search

 OUT
	return the value or NULL
 ***********************************************************************/

void *hash_get(htable *pht, char *key) {
	hcell	*phc;
	hnode	*phn;
	void	*data = NULL;

	/* get the key node */
	phn = hash_node_seek(pht, key);

	/* looking for a matching cell */
	phc = hash_cell_seek(phn, key);

	/* if the key has been found */
	if (phc != NULL) {
		data = phc->value;
	}

	return(data);
}


/****************
 * hash_merge() *
 ***********************************************************************
 DESCR
	merge one hash table into another

 IN
	dst_ht : destination table
	src_ht : table to merge

 OUT
	number of merged key
 ***********************************************************************/

size_t hash_merge(htable *dst_ht, htable *src_ht) {
	hcell			*p;
	size_t			 s,
					 c = 0;
	unsigned int	 i;

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

/****************
 * hash_nbkey() *
 ***********************************************************************
 DESCR
	get number of keys

 IN
	pht : hash table

 OUT
	number of keys
 ***********************************************************************/

size_t hash_nbkey(htable *pht) {
	return(pht->count);
}

/***************
 * hash_keys() *
 ***********************************************************************
 DESCR
	get the keys of the hash table

 IN
	pht : hash table

 OUT
	hkeys structure

 NOTE
	don't forget to free the array after use.
 ***********************************************************************/

hkeys *hash_keys(htable *pht) {
	hcell			*p;
	hkeys			*phk;
	size_t			 i,
					 j = 0;

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
#ifdef PMK_HASH_DEBUG
			debugf("keys alloc failed");
#endif
			/* free allocated space only */
			free(phk);
		}
#ifdef PMK_HASH_DEBUG
	} else {
		debugf("hkeys struct alloc failed");
#endif
	}

	return(NULL);
}


/**********************
 * hash_keys_sorted() *
 ***********************************************************************
 DESCR
	get the sorted list of keys of the hash table

 IN
	pht : hash table

 OUT
	return an hkeys structure

 NOTE
	don't forget to free the array after use
 ***********************************************************************/

static int hash_strcmp(const void *a, const void *b) {
	return(strcmp(*(char * const *)a, *(char * const *) b));
}

hkeys *hash_keys_sorted(htable *pht) {
	hkeys	*phk;

	/* build hkey structure */
	phk = hash_keys(pht);

	if (phk != NULL) {
		/* sort key names */
		qsort((void *) phk->keys, phk->nkey, sizeof(char *),
							hash_strcmp);
	}
	return(phk);
}


/*********************
 * hash_free_hkeys() *
 ***********************************************************************
 DESCR
	free memory allocated to the hkeys structure

 IN
	phk : structure to free

 OUT
	-
 ***********************************************************************/

void hash_free_hkeys(hkeys *phk) {
	/* doesn't free pointed values */
	free(phk->keys);
	free(phk);
}

/*********************
 * hash_str_append() *
 ***********************************************************************
 DESCR
	append function for string hash

 IN
	orig : XXX
	value : XXX
	sep : XXX

 OUT
	-
 ***********************************************************************/

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

	if (strlcpy_b(pbuf, orig, s) == false) {
		free(value);
		free(pbuf);
#ifdef PMK_HASH_DEBUG
		debugf("hash_str_append : strlcpy1 failed");
#endif
		return(NULL);
	}

	if ((sep != NULL) && (pbuf[0] != '\0')) {
		/* adding separator if provided and if
			string is not empty */
		if (strlcat_b(pbuf, (char *) sep, s) == false) {
			free(value);
			free(pbuf);
#ifdef PMK_HASH_DEBUG
		debugf("hash_str_append : strlcat1 failed");
#endif
			return(NULL);
		}
	}
	if (strlcat_b(pbuf, value, s) == false) {
		free(value);
		free(pbuf);
#ifdef PMK_HASH_DEBUG
		debugf("hash_str_append : strlcat2 failed");
#endif
		return(NULL);
	}

	free(value);
	return((void *) pbuf);
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
