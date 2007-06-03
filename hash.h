/* $Id$ */

/*
 * Copyright (c) 2006 Damien Couderc
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


#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef _HASH_H_
#define _HASH_H_

/**********
 * macros *
 *************************************************************************************************************/

/* for compatibility with previous code, to remove later */
//#define hash_init(s)			hash_create_simple(s)
//#define hash_init_adv(s,d,f,a)	hash_create(s,false,a,d,f)
//#define htable					htable_t

/*************
 * constants *
 *************************************************************************************************************/

enum {
	HERR_NONE = 0,
	HERR_CREATED,
	HERR_ADDED,
	HERR_UPDATED,
	HERR_APPENDED,
	HERR_MALLOC_FAILED,
	HERR_FAPND_FAILED,
	HERR_FDUP_FAILED,
	HERR_FFREE_FAILED,
	HERR_FAPND_NOT_DEF,
	HERR_FDUP_NOT_DEF,
	HERR_FFREE_NOT_DEF,
	HERR_CELL_EXISTS,
	HERR_NULL_DATA,
	HERR_RESIZE_FAILED,
	HERR_TABLE_FULL
};


/********************
 * type definitions *
 *************************************************************************************************************/

/* hash type */
typedef uint32_t	hash_t;


/* node cell type */
typedef struct hcell_s {
	char			*key;	/* cle */
	void			*data;	/* donnees */
	struct hcell_s	*prev,	/* cellule precedente */
					*next;	/* cellule suivante */
} hcell_t;

/* node type */
typedef struct {
	hcell_t	*first,
			*last;
} hnode_t;

/* hash table type */
typedef struct {
	bool	 autogrow;
	size_t	 count,
			 size;
	hnode_t	*harray;
	int		 herr;
	void	*(*apnd_data)(void *, void *, void *),
			*(*dup_data)(void *),
			 (*free_data)(void *);
} htable_t;


/* hash keys type */
typedef struct {
	size_t	  nkey;
	char	**keys;
} hkeys_t;


/**************
 * prototypes *
 *************************************************************************************************************/

/* public functions prototypes */
htable_t	*hash_create(size_t, bool, void *(*)(void *, void *, void *), void *(*)(void *), void (*)(void *));
bool		 hash_add(htable_t *, char *, void *);
bool		 hash_add_dup(htable_t *, char *, void *);
bool		 hash_update(htable_t *, char *, void *);
bool		 hash_update_dup(htable_t *, char *, void *);
bool		 hash_append(htable_t *, char *, void *, void *);
void		*hash_get(htable_t *, char *);
void		*hash_extract(htable_t *, char *);
void		 hash_delete(htable_t *, char *);
bool		 hash_merge(htable_t *, htable_t *, bool);
bool		 hash_resize(htable_t *, size_t);
bool		 hash_check_grow(htable_t *);
void		 hash_destroy(htable_t *);
size_t		 hash_nbkey(htable_t *);
hkeys_t		*hash_keys(htable_t *);
hkeys_t		*hash_keys_sorted(htable_t *);
void		 hash_free_hkeys(hkeys_t *);
char		*hash_error(htable_t *);

#endif /* _HASH_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
