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


#ifndef _PMK_HASH_H_
#define _PMK_HASH_H_

#include "compat/pmk_stdbool.h"


#ifndef MAX_HASH_KEY_LEN
#	define MAX_HASH_KEY_LEN 256
#endif

#ifndef MAX_HASH_VALUE_LEN
#	define MAX_HASH_VALUE_LEN 512
#endif

#define HASH_ADD_FAIL	0 /* addition or appending failed */
#define HASH_ADD_OKAY	1 /* first add */
#define HASH_ADD_COLL	2 /* collision, key chained */
#define HASH_ADD_UPDT	3 /* key already exists, change value */
#define HASH_ADD_APPD	4 /* value appended */


typedef struct shcell {
	char		 key[MAX_HASH_KEY_LEN];
	void		*value;
	struct shcell	*next;
} hcell;

typedef struct {
	hcell	*first,
		*last;
} hnode;

typedef struct {
	int	 size,
		 count,
		 autogrow;
	void	 (*freeobj)(void *),
		*(*appdobj)(void *, void *, void *);
	hnode	*nodetab; /* array of hnode */
} htable;

typedef struct {
	char	 key[MAX_HASH_KEY_LEN];
	void	*value;
} hpair;

typedef struct {
	int	  nkey;
	char	**keys;
} hkeys;


int	 hash_compute(char *, int);
htable	*hash_init(int);
htable	*hash_init_adv(int, void (*)(void *), void *(*)(void *, void *, void *));
bool	 hash_resize(htable *, int);
void	 hash_set_grow(htable *);
int	 hash_destroy(htable *);
int	 hash_add(htable *, char *, void *);
int	 hash_add_cell(hnode *, hcell *);
bool	 hash_add_array(htable *, hpair *, int);
int	 hash_append(htable *, char *, void *, void *);
void	 hash_delete(htable *, char *);
void	*hash_get(htable *, char *);
int	 hash_merge(htable *, htable *);
int	 hash_nbkey(htable *);
hkeys	*hash_keys(htable *);
void	 hash_free_hcell(htable *, hcell *);
void	 hash_free_hkeys(hkeys *);
void	*hash_str_append(void *, void *, void *);

#endif /* _PMK_HASH_H_ */
