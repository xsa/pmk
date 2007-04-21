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


/*
 *
 * hash table implementation using two-way linked lists for collisions
 *
 */

#include <stdlib.h>
#include <string.h>

#include "hash.h"
/*#include "debug.h"*/


/*************
 * constants *
 *************************************************************************************************************/

char *hmsg[] = {
	"hash table pointer is NULL",			/*  0 */
	"unknown error code",					/*  1 */
	"hash table created",					/*  2 */
	"key added",							/*  3 */
	"key updated",							/*  4 */
	"data appended",						/*  5 */
	"memory allocation failed",				/*  6 */
	"appendition function failed",			/*  7 */
	"duplication function failed",			/*  8 */
	"deallocation function failed",			/*  9 */
	"appendition function not defined",		/* 10 */
	"duplication function not defined",		/* 11 */
	"deallocation function not defined",	/* 12 */
	"cannot add already existing cell",		/* 13 */
	"data pointer is NULL",					/* 14 */
	"hash table is full",					/* 15 */
};


/*********************
 * private functions *
 *************************************************************************************************************/

/* private functions prototypes */
static hash_t	 hash_compute(char *, size_t);
static size_t	 hash_correct_size(size_t);
static hnode_t	*hash_search_node(htable_t *, char *);
static hcell_t	*hash_create_cell(char *, void *);
static void		 hash_add_cell(hnode_t *, hcell_t *);
static void		 hash_update_cell(htable_t *, hcell_t *, void *);
static hcell_t	*hash_search_cell(hnode_t *, char *);
static hcell_t	*hash_extract_cell(hnode_t *, char *);
static void		 hash_free_cell(htable_t *, hcell_t *);
static void		*hash_extract_data(hcell_t *);
static int		 hash_strcmp(const void *, const void *);

/******************
 * hash_compute() *
 *************************************************************************************************************
 %DESCR generate the hashing value corresponding to the provided key

 %PARAM key :	key string to hash
 %PARAM size :	size of hash table

 %RETURN hash value
 *************************************************************************************************************/

static hash_t hash_compute(char *key, size_t size) {
	hash_t	hash = 0,
			l = 0;

	/* for each characters of the key string */
	while (*key != '\0') {
		/* hashing process */
		hash = hash + (hash_t) *key + (l >> 8);
		hash = hash + (hash << 3);
		hash = hash ^ (hash >> 9);

		key++;
		l++;
	}

	/*
	 * use a mask which is faster than a modulo
	 */
	hash = hash & ((hash_t) size - 1);

	return hash;
}


/***********************
 * hash_correct_size() *
 *************************************************************************************************************
 %DESCR provide the first power of two that is equal or greater then the provided number

 %PARAM size : minimal size of the table

 %RETURN the power of two that is next to the provided size
 *************************************************************************************************************/

static size_t hash_correct_size(size_t size) {
	size_t	power_size;
	uint8_t	shift = 0;

	/* minimum size is 2 */
	if (size < 2) {
		return 2;
	}

	/* initialize with the given size */
	power_size = size;

	/* shift to the right until we get the highest bit */
	while (power_size > 1) {
		power_size = power_size >> 1;
		shift++;
	}

	/* shift back to the left to get the original place */
	power_size = power_size << shift;

	/*
	 * if the original value was greater than the actual power of two
	 * then we shift again to the left to get a superior value
	 */
	if (power_size < size) {
		power_size = power_size << 1;
	}

	return power_size;
}


/**********************
 * hash_search_node() *
 *************************************************************************************************************
 %DESCR search the node corresponding to the given key string

 %PARAM pht : hash table pointer
 %PARAM key : key string to look for

 %RETURN node pointer
 *************************************************************************************************************/

static hnode_t *hash_search_node(htable_t *pht, char *key) {
	hash_t	 h;
	hnode_t *phn;

	/* compute hash */
	h = hash_compute(key, pht->size);

	/* get node pointer */
	phn = &(pht->harray[h]);

	return phn;
}


/**********************
 * hash_create_cell() *
 *************************************************************************************************************
 %DESCR allocate and initialize an hash table cell structure

 %PARAM key : key string
 %PARAM data : key data

 %RETURN pointer of the new cell or NULL on failure
 *************************************************************************************************************/

static hcell_t *hash_create_cell(char *key, void *data) {
	hcell_t	*phc;

	/* allocate cell structure */
	phc = (hcell_t *) malloc(sizeof(hcell_t));
	if (phc == NULL) {
		/* allocation failed */
		return NULL;
	}

	/* set key */
	phc->key = strdup(key);

	/* set data */
	phc->data = data;

	/* initialize cell pointers */
	phc->prev = NULL;
	phc->next = NULL;

	return phc;
}


/*******************
 * hash_add_cell() *
 *************************************************************************************************************
 %DESCR add a cell into a given node

 %PARAM phn : node pointer in which the cell must be added
 %PARAM phc : cell to add

 %RETURN NONE
 *************************************************************************************************************/

static void hash_add_cell(hnode_t *phn, hcell_t *phc) {
	/* if the node is empty */
	if (phn->last == NULL) {
		/* set the cell as first link */
		phn->first = phc;
	} else {
		/* link the new cell next to the last cell of the node */
		phc->prev = phn->last;
		phn->last->next = phc;
	}

	/* update the node link to the last cell */
	phn->last = phc;
}


/**********************
 * hash_update_cell() *
 *************************************************************************************************************
 %DESCR update cell data

 %PARAM pht : hash table pointer
 %PARAM phc : cell pointer
 %PARAM data : pointer to new data

 %RETURN NONE
 *************************************************************************************************************/

static void hash_update_cell(htable_t *pht, hcell_t *phc, void *data) {
	if (phc->data != NULL) {
		/* deallocate data if it exists */
		pht->free_data(phc->data);
	}

	/* put new data */
	phc->data = data;
}


/**********************
 * hash_search_cell() *
 *************************************************************************************************************
 %DESCR key based search of a cell in a node

 %PARAM phn : pointer to the node
 %PARAM key : key string

 %RETURN cell pointer or NULL if not found
 *************************************************************************************************************/

static hcell_t *hash_search_cell(hnode_t *phn, char *key) {
	hcell_t	*phc;
	size_t	 ksize;

	/* compute size of string (null terminator included) */
	ksize = strlen(key) + 1;

	/* get first cell of the node */
	phc = phn->first;

	/* search until the last cell */
	while (phc != NULL) {
		/* if the cell matches with the key */
		if (strncmp(phc->key, key, ksize) == 0) {
			/* return cell pointer */
			return phc;
		}

		/* set pointer to next cell */
		phc = phc->next;
	}

	/* value not found, return NULL */
	return NULL;
}


/***********************
 * hash_extract_cell() *
 *************************************************************************************************************
 %DESCR extract a cell from a node

 %PARAM phn : pointer to the node
 %PARAM phc : key string of the cell to extract

 %RETURN pointer to the extracted cell or NULL if not found

 %NOTE the extracted cell will no longer be deallocated with the hash table
 *************************************************************************************************************/

static hcell_t *hash_extract_cell(hnode_t *phn, char *key) {
	hcell_t	*next,
			*phc,
			*prev;

	/* get the cell */
	phc = hash_search_cell(phn, key);

	/* if a matching cell has been found */
	if (phc != NULL) {
		/* get pointers to previous and next cells */
		prev = phc->prev;
		next = phc->next;

		/* update link with previous cell */
		if (prev == NULL) {
			phn->first = next;
		} else {
			prev->next = next;
		}

		/* update link with next cell */
		if (next == NULL) {
			phn->last = prev;
		} else {
			next->prev = prev;
		}
	}

	return phc;
}


/********************
 * hash_free_cell() *
 *************************************************************************************************************
 %DESCR deallocate cell structure 

 %PARAM pht : hash table pointer
 %PARAM phc : cell pointer

 %RETURN NONE
 *************************************************************************************************************/

static void hash_free_cell(htable_t *pht, hcell_t *phc) {
	/* deallocate key */
	free(phc->key);
	
	if (phc->data != NULL) {
		/* deallocate data if it exists */
		pht->free_data(phc->data);
	}
}


/***********************
 * hash_extract_data() *
 *************************************************************************************************************
 %DESCR extract data of a cell

 %PARAM phc : cell pointer

 %RETURN data pointer
 *************************************************************************************************************/

static void *hash_extract_data(hcell_t *phc) {
	void	*pdata;

	/* get the pointer to data */
	pdata = phc->data;

	/* initialize cell's data pointer */
	phc->data = NULL;

	return pdata;
}


/*****************
 * hash_strcmp() *
 *************************************************************************************************************
 %DESCR string compare function wrapper for quick sort

 %PARAM a : string a
 %PARAM b : string b

 %RETURN comparison status
 *************************************************************************************************************/

static int hash_strcmp(const void *a, const void *b) {
	return(strcmp(*(char * const *) a, *(char * const *) b));
}


/********************
 * public functions *
 *************************************************************************************************************/

/*****************
 * hash_create() *
 *************************************************************************************************************
 %DESCR allocation and initialization of hash table structure

 %PARAM size :		hash table size
 %PARAM autogrow :	enable or disable autogrow
 %PARAM fa :		data appending function
 %PARAM fd :		data duplication function
 %PARAM ff :		data deallocation function

 %RETURN hash table pointer or NULL on failure
 *************************************************************************************************************/

htable_t *hash_create(	size_t size,
						bool autogrow,
						void *(*fa)(void *, void *, void *),
						void *(*fd)(void *),
						void (*ff)(void *)) {
	htable_t	*pht;
	size_t		 csize,
				 i;

	csize = hash_correct_size(size);

	/* allocate main structure */
	pht = (htable_t *) malloc(sizeof (htable_t));
	if (pht == NULL) {
		/* allocation failed */
		return NULL;
	}

	/* allocate node array */
	pht->harray = (hnode_t *) malloc(sizeof (hnode_t) * csize);
	if (pht->harray == NULL) {
		/* allocation failed */
		free(pht);
		return NULL;
	}

	/* set value of automatic resizing indicator */
	pht->autogrow = autogrow;

	/* set size */
	pht->size = csize;

	/* initialize usage counter */
	pht->count = 0;

	/* initialise hash error indicator */
	pht->herr = HERR_CREATED;

	/* initialize node structures */
	for (i = 0 ; i < csize ; i++) {
		pht->harray[i].first = NULL;
		pht->harray[i].last = NULL;
	}

	/* set data append function */
	pht->apnd_data = fa;

	/* set data duplicate function */
	pht->dup_data = fd;

	/* set deallocation function pointer */
	pht->free_data = ff;

	return pht;
}


/**************
 * hash_add() *
 *************************************************************************************************************
 %DESCR addition (without update) of a key

 %PARAM pht : hash table pointer
 %PARAM key : key string
 %PARAM data : key data

 %RETURN true if correctly added or false if the key already exists or has failed to be added

 %NOTE the error detail is set into hash table structure
 *************************************************************************************************************/

bool hash_add(htable_t *pht, char *key, void *data) {
	hcell_t	*phc;
	hnode_t	*phn;

	/* check size */
	if (hash_check_grow(pht) == false) {
		return false;
	}

	/* get node */
	phn = hash_search_node(pht, key);

	/* look for existing key */
	if (hash_search_cell(phn, key) != NULL) {
		/* the key is already there */
		pht->herr = HERR_CELL_EXISTS;
		return false;
	}

	/* create cell */
	phc = hash_create_cell(key, data);
	if (phc == NULL) {
		/* creation failed */
		pht->herr = HERR_MALLOC_FAILED;
		return false;
	}

	/* add cell */
	hash_add_cell(phn, phc);

	/* update counter */
	pht->count++;

	pht->herr = HERR_ADDED;
	return true;
}


/******************
 * hash_add_dup() *
 *************************************************************************************************************
 %DESCR addition (without update) of a key using data duplication function

 %PARAM pht : hash table pointer
 %PARAM key : key string
 %PARAM data : key data

 %RETURN true if correctly added or false if the key already exists or has failed to be added

 %NOTE the error detail is set into hash table structure
 *************************************************************************************************************/

bool hash_add_dup(htable_t *pht, char *key, void *data) {
	bool	 rval;
	void	*dupdata;

	/* check if duplication function is available */
	if (pht->dup_data == NULL) {
		pht->herr = HERR_FDUP_NOT_DEF;
		return(false);
	}

	/* duplicate data */
	dupdata = pht->dup_data(data);

	/* update key data */
	rval = hash_add(pht, key, dupdata);

	if (rval == false) {
		/* deallocate duplicated data */
		pht->free_data(data);
	}

	return rval;
}


/*****************
 * hash_update() *
 *************************************************************************************************************
 %DESCR addition or update of a key

 %PARAM pht : hash table pointer
 %PARAM key : key string
 %PARAM data : key data

 %RETURN true on success else false
 *************************************************************************************************************/

bool hash_update(htable_t *pht, char * key, void *data) {
	hcell_t	*phc;
	hnode_t	*phn;

	/* check size */
	if (hash_check_grow(pht) == false) {
		return false;
	}

	/* get node */
	phn = hash_search_node(pht, key);

	/* try to get an existent cell */
	phc = hash_search_cell(phn, key);

	/* if the cell has not been found */
	if (phc == NULL) {
		/* create a new cell */
		phc = hash_create_cell(key, data);
		if (phc == NULL) {
			/* creation failed */
			return false;
		}

		/* add cell */
		hash_add_cell(phn, phc);

		/* update counter */
		pht->count++;

		pht->herr = HERR_ADDED;
	} else {
		/* update cell */
		hash_update_cell(pht, phc, data);

		pht->herr = HERR_UPDATED;
	}

	return true;
}


/*********************
 * hash_update_dup() *
 *************************************************************************************************************
 %DESCR addition or update of a key using data duplication function

 %PARAM pht : hash table pointer
 %PARAM key : key string
 %PARAM data : key data

 %RETURN true on success else false
 *************************************************************************************************************/

bool hash_update_dup(htable_t *pht, char *key, void *data) {
	bool	 rval;
	void	*dupdata;

	/* check if duplication function is available */
	if (pht->dup_data == NULL) {
		pht->herr = HERR_FDUP_NOT_DEF;
		return(false);
	}

	/* duplicate data */
	dupdata = pht->dup_data(data);

	/* update key data */
	rval = hash_update(pht, key, dupdata);

	if (rval == false) {
		/* deallocate duplicated data */
		pht->free_data(data);
	}

	return rval;
}


/*****************
 * hash_append() *
 ***********************************************************************
 %DESCR append a value to the existing key data

 %PARAM pht :	hash structure
 %PARAM key :	key to be modified
 %PARAM value :	value to append into key data
 %PARAM misc :	extra data to transmit to append function

 %RETURN true on success else false
 ***********************************************************************/

bool hash_append(htable_t *pht, char *key, void *data, void *misc) {
	hcell_t	*phc;
	hnode_t	*phn;
	void	*newdata;

	if (data == NULL) {
		pht->herr = HERR_NULL_DATA;
		return(false);
	}

	if (pht->apnd_data == NULL) {
		pht->herr = HERR_FAPND_NOT_DEF;
		return(false);
	}

	/* get node */
	phn = hash_search_node(pht, key);

	/* try to get an existent cell */
	phc = hash_search_cell(phn, key);

	/* if the cell has not been found */
	if (phc == NULL) {
		/* create a new cell */
		phc = hash_create_cell(key, data);
		if (phc == NULL) {
			/* creation failed */
			return false;
		}

		/* add cell */
		hash_add_cell(phn, phc);

		/* update counter */
		pht->count++;
	} else {
		/* append data to existing cell data */
		newdata = pht->apnd_data(phc->data, data, misc);

		if (newdata == NULL) {
			/* appending failed */
			pht->herr = HERR_FAPND_FAILED;
			return false;
		}

		/* update cell with new data */
		hash_update_cell(pht, phc, newdata);
	}

	/* override error status */
	pht->herr = HERR_APPENDED;

	return true;
}


/**************
 * hash_get() *
 *************************************************************************************************************
 %DESCR get a pointer to the key data

 %PARAM pht : hash table pointer
 %PARAM key : string of the key to look for

 %RETURN key data pointer or NULL if not found
 *************************************************************************************************************/

void *hash_get(htable_t *pht, char *key) {
	hcell_t	*phc;
	hnode_t	*phn;

	/* get node */
	phn = hash_search_node(pht, key);

	/* try to get an existent cell */
	phc = hash_search_cell(phn, key);

	if (phc == NULL) {
		/* no cell found */
		return NULL;
	}

	/* return cell data */
	return phc->data;
}


/******************
 * hash_extract() *
 *************************************************************************************************************
 %DESCR extract key from the hash table 

 %PARAM pht : hash table pointer
 %PARAM key : string of the key to extract

 %RETURN key data pointer or NULL if not found

 %NOTE key cell will be removed from the node, data will not be deallocated by hash table deallocation
 *************************************************************************************************************/

void *hash_extract(htable_t *pht, char *key) {
	hcell_t	*phc;
	hnode_t	*phn;
	void	*pdata;

	/* get node */
	phn = hash_search_node(pht, key);

	/* extract cell from the node */
	phc = hash_extract_cell(phn, key);
	if (phc == NULL) {
		/* cell not found, key does not exists */
		return NULL;
	}

	/* extract data from the cell (to preserve it from cell destruction) */
	pdata = hash_extract_data(phc);

	/* desallocate cell structure */
	hash_free_cell(pht, phc);

	/* update counter */
	pht->count--;

	/* return data pointer */
	return pdata;
}


/*****************
 * hash_delete() *
 *************************************************************************************************************
 %DESCR remove a key from the hash table

 %PARAM pht : hash table pointer
 %PARAM key : key to delete

 %RETURN NONE
 *************************************************************************************************************/

void hash_delete(htable_t *pht, char *key) {
	hcell_t	*phc;
	hnode_t	*phn;

	/* get node */
	phn = hash_search_node(pht, key);

	/* extract cell */
	phc = hash_extract_cell(phn, key);
	if (phc != NULL) {
		/* cell found, deallocate cell structure and data within */
		hash_free_cell(pht, phc);

		/* update counter */
		pht->count--;
	}
}


/****************
 * hash_merge() *
 ***********************************************************************
 %DESCR merge a hash table into another one

 %PARAM dst_ht : destination hash table
 %PARAM src_ht : source hash table to be merged
 %PARAM update : boolean to allow update of key during merge

 %RETURN true on success else false
 ***********************************************************************/

bool hash_merge(htable_t *pdht, htable_t *psht, bool update) {
	hcell_t			*phc;
	size_t			 i;

	/* for each node of the source hash table */
	for(i = 0 ; i < psht->size ; i++) {
		/* get the first cell */
		phc = psht->harray[i].first;

		/* process all of the node cells */
		while (phc != NULL) {
			/* add the key in destination hash_table */
			if (update == true) {
				/* update is allowed */
				if (hash_update_dup(pdht, phc->key, phc->data) == false) {
					return false;
				}
			} else {
				/* add only */
				if (hash_add_dup(pdht, phc->key, phc->data) == false) {
					return false;
				}
			}

			/* go to next cell */
			phc = phc->next;
		}
	}

	return true;
}


/*****************
 * hash_resize() *
 ***********************************************************************
 %DESCR resizing hash table

 %PARAM pht :	hash table to resize
 %PARAM size :	new hash table size

 %RETURN true on success else false
 ***********************************************************************/

bool hash_resize(htable_t *pht, size_t size) {
	hcell_t	*phc,
			*next;
	hnode_t	*phn,
			*pold;
	size_t	 i,
			 s;

	/* save actual node array */
	pold = pht->harray;

	/* allocate new node array */
	pht->harray = (hnode_t *) malloc(sizeof(hnode_t) * size);
	if (pht->harray == NULL) {
		/* allocation failed */
		pht->herr = HERR_MALLOC_FAILED;
		/* restore old node array */
		pht->harray = pold;

		return false;
	}

	/* save old size */
	s = pht->size;

	/* set new size */
	pht->size = size;

	/* initialize nodes of the new array */
	for (i = 0 ; i < size ; i++) {
		pht->harray[i].first = NULL;
		pht->harray[i].last = NULL;
	}

	/* process previous node array */
	for (i = 0 ; i < s ; i++) {
		/* get first cell of the node */
		phc = pold[i].first;

		/* process each cell of the current node */
		while (phc != NULL) {
			/* get node */
			phn = hash_search_node(pht, phc->key);

			/* save pointer to next cell */
			next = phc->next;

			/* add cell into the node */
			hash_add_cell(phn, phc);

			/* next cell of the old node array */
			phc = next;
		}
	}

	/* deallocate old array structure */
	free(pold);

	return true;
}


/*********************
 * hash_check_grow() *
 *************************************************************************************************************
 %DESCR check if hash table is full and resize it if allowed

 %PARAM pht : hash table pointer

 %RETURN true on success else false if table is full or if resizing failed
 *************************************************************************************************************/

bool hash_check_grow(htable_t *pht) {
	/* compare number of cells with table size */
	if (pht->count >= pht->size) {
		/* table is full */
		if (pht->autogrow == false) {
			/* autogrow is disabled, check fails */
			pht->herr = HERR_TABLE_FULL;
			return false;
		}
		
		/* autogrow is enabled, resize the table */
		if (hash_resize(pht, pht->size * 2) == false) {
			/* cannot resize the hash */
			return false;
		}
	}

	return true;
}


/******************
 * hash_destroy() *
 *************************************************************************************************************
 %DESCR deallocate hash table

 %PARAM pht : hash table pointer

 %RETURN NONE
 *************************************************************************************************************/

void hash_destroy(htable_t *pht) {
	hcell_t	*phc,
			*next;
	size_t	 i;

	/* process each node of the table */
	for (i = 0 ; i < pht->size ; i++) {
		/* get first cell of the node */
		phc = pht->harray[i].first;

		/* process each cell of the node */
		while (phc != NULL) {
			/* save next cell's pointer */
			next = phc->next;

			/* deallocate cell */
			hash_free_cell(pht, phc);

			/* update with saved pointer to the next cell */
			phc = next;
		}
	}

	/* deallocate node array */
	free(pht->harray);

	/* deallocate hash table structure */
	free(pht);
}


/****************
 * hash_nbkey() *
 ***********************************************************************
 %DESCR get number of keys

 %PARAM pht : hash table

 %RETURN number of keys
 ***********************************************************************/

size_t hash_nbkey(htable_t *pht) {
	return pht->count;
}


/***************
 * hash_keys() *
 ***********************************************************************
 %DESCR get the keys of the hash table

 %PARAM pht : hash table

 %RETURN hkeys structure

 %NOTE don't forget to free the array after use.
 ***********************************************************************/

hkeys_t *hash_keys(htable_t *pht) {
	hcell_t			*phc;
	hkeys_t			*phk;
	unsigned int	 i,
					 j = 0;

	/* if hash table is empty */
	if (pht->count == 0) {
		return NULL;
	}

	/* init hkeys struct to be returned */
	phk = (hkeys_t *) malloc(sizeof(hkeys_t));
	if (phk == NULL) {
		return NULL;
	}

	phk->nkey = pht->count;

	/* create an array with a size of the number of keys */
	phk->keys = (char **) malloc(sizeof(char *) * phk->nkey);
	if (phk->keys == NULL) {
		/* free allocated space only */
		free(phk);
		return NULL;
	}

	for(i = 0 ; i < pht->size ; i++) {
		phc = pht->harray[i].first;
		while (phc != NULL) {
			/* add the key in key_ary */
			phk->keys[j] = phc->key;
			j++;
			phc = phc->next;
		}
	}

	return phk;
}


/**********************
 * hash_keys_sorted() *
 ***********************************************************************
 %DESCR get the sorted list of keys of the hash table

 %PARAM pht : hash table

 %RETURN return an hkeys structure

 %NOTE don't forget to free the array after use
 ***********************************************************************/

hkeys_t *hash_keys_sorted(htable_t *pht) {
	hkeys_t	*phk;

	/* build hkey structure */
	phk = hash_keys(pht);

	if (phk != NULL) {
		/* sort key names */
		qsort((void *) phk->keys, phk->nkey, sizeof(char *), hash_strcmp);
	}
	return(phk);
}


/*********************
 * hash_free_hkeys() *
 ***********************************************************************
 %DESCR free memory allocated to the hkeys structure

 %PARAM phk : structure to free

 %RETURN NONE
 ***********************************************************************/

void hash_free_hkeys(hkeys_t *phk) {
	/* doesn't free pointed values */
	free(phk->keys);
	free(phk);
}


/****************
 * hash_error() *
 ***********************************************************************
 %DESCR error message for latest hash table operation status

 %PARAM herr : error code

 %RETURN NONE
 ***********************************************************************/

char *hash_error(htable_t *pht) {
	char	*pstr;

	if (pht == NULL) {
		return hmsg[0];
	}

	switch (pht->herr) {
		/*case HERR_NONE :  */
		/*    pstr = hmsg[];*/
		/*    break;        */

		case HERR_CREATED :
			pstr = hmsg[2];
			break;

		case HERR_ADDED	:
			pstr = hmsg[3];
			break;

		case HERR_UPDATED :
			pstr = hmsg[4];
			break;

		case HERR_APPENDED :
			pstr = hmsg[5];
			break;

		case HERR_MALLOC_FAILED :
			pstr = hmsg[6];
			break;

		case HERR_FAPND_FAILED :
			pstr = hmsg[7];
			break;

		case HERR_FDUP_FAILED :
			pstr = hmsg[8];
			break;

		case HERR_FFREE_FAILED :
			pstr = hmsg[9];
			break;

		case HERR_FAPND_NOT_DEF :
			pstr = hmsg[10];
			break;

		case HERR_FDUP_NOT_DEF :
			pstr = hmsg[11];
			break;

		case HERR_FFREE_NOT_DEF :
			pstr = hmsg[12];
			break;

		case HERR_CELL_EXISTS :
			pstr = hmsg[13];
			break;

		case HERR_NULL_DATA :
			pstr = hmsg[14];
			break;

		case HERR_TABLE_FULL :
			pstr = hmsg[15];
			break;

		default :
			pstr = hmsg[1];
	}

	return pstr;
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
