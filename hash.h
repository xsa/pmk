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


/*************
 * constants *
 ***********************************************************************/

#ifndef _PMK_HASH_H_
#define _PMK_HASH_H_

#include "compat/pmk_stdbool.h"


#ifndef MAX_HASH_KEY_LEN
#	define MAX_HASH_KEY_LEN 256
#endif

#ifndef MAX_HASH_VALUE_LEN
#	define MAX_HASH_VALUE_LEN 512
#endif

enum {
	HASH_ADD_FAIL = 0,	/* addition or appending failed */
	HASH_ADD_OKAY,		/* first add */
	HASH_ADD_COLL,		/* collision, key chained */
	HASH_ADD_UPDT,		/* key already exists, change value */
	HASH_ADD_APPD		/* value appended */
};

enum {
	HASH_METHOD_ADD = 1,
	HASH_METHOD_UPD = 2
};

/* error messages */
#define HASH_ERR_UPDT		"Hash table update failed."
#define HASH_ERR_UPDT_ARG	"Failed to update hash table key '%s'."


/**********************************
 * type and structure definitions *
 ***********************************************************************/

typedef struct s_hcell {
	char			 key[MAX_HASH_KEY_LEN];
	void			*value;
	struct s_hcell	*prev,
					*next;
} hcell;

typedef struct {
	hcell	*first,
			*last;
} hnode;

typedef struct {
	bool			 autogrow;
	size_t			 count,
					 size;
	void			*(*dupobj)(void *),
					 (*freeobj)(void *),
					*(*appdobj)(void *, void *, void *);
	hnode			*nodetab; /* array of hnode */
} htable;

typedef struct {
	char	 key[MAX_HASH_KEY_LEN];
	void	*value;
} hpair;

typedef struct {
	size_t	  nkey;
	char	**keys;
} hkeys;

typedef unsigned char	hmtd;

typedef unsigned char	herr;


/***********************
 * function prototypes *
 ***********************************************************************/

size_t			 hash_compute(char *, size_t);
herr			 hash_size_check(htable *);
hnode			*hash_node_array_init(size_t);
hnode			*hash_node_seek(htable *, char *);
hcell			*hash_cell_init(char *, void *);
hcell			*hash_cell_seek(hnode *, char *);
herr			 hash_cell_add(htable *, hcell *, hmtd);
hcell			*hash_cell_extract(hnode *, char *);
void			*hash_cell_destroy(htable *, hcell *);

htable			*hash_init(size_t);
htable			*hash_init_adv(size_t, void *(*)(void *), void (*)(void *), void *(*)(void *, void *, void *));
bool			 hash_resize(htable *, size_t);
void			 hash_set_grow(htable *);
size_t			 hash_destroy(htable *);
herr			 hash_add(htable *, char *, void *);
herr			 hash_update(htable *, char *, void *);
herr			 hash_update_dup(htable *, char *, void *);

bool			 hash_add_array(htable *, hpair *, size_t);
bool			 hash_add_array_adv(htable *, hpair *, size_t, void *(*)(void *));
herr			 hash_append(htable *, char *, void *, void *);
void			 hash_delete(htable *, char *);
void			*hash_extract(htable *, char *);
void			*hash_get(htable *, char *);
size_t			 hash_merge(htable *, htable *);
size_t			 hash_nbkey(htable *);
hkeys			*hash_keys(htable *);
hkeys			*hash_keys_sorted(htable *);
void			 hash_free_hcell(htable *, hcell *);
void			 hash_free_hkeys(hkeys *);
void			*hash_str_append(void *, void *, void *);

#endif /* _PMK_HASH_H_ */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
