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

#include "compat/pmk_string.h"
#include "pmk_obj.h"


/*
	make bool object

	value : boolean to objectify

	return : objet
*/

pmkobj *po_mk_bool(bool value) {
	bool	*pb;
	pmkobj	*po;

	po = (pmkobj *) malloc(sizeof(pmkobj));
	if (po != NULL) {
		po->type = PO_BOOL;
		pb = (bool *) malloc(sizeof(bool));
		if (pb == NULL) {
			free(po);
			return(NULL);
		}

		*pb = value;
		po->data = pb;
	}

	return(po);
}

/*
	make string object

	str : string to objectify

	return : objet
*/

pmkobj *po_mk_str(char *str) {
	pmkobj	*p;

	p = (pmkobj *) malloc(sizeof(pmkobj));
	if (p != NULL) {
		p->type = PO_STRING;
		p->data = strdup(str);
	}

	return(p);
}

/*
	make list object

	pda : dynary to objectify

	return : object
*/

pmkobj *po_mk_list(dynary *pda) {
	pmkobj	*p;

	p = (pmkobj *) malloc(sizeof(pmkobj));
	if (p != NULL) {
		p->type = PO_LIST;
		p->data = pda; /* XXX copy ? */
	}

	return(p);
}

/*
	make hash object

	pht : hash table to objectify

	return : object
*/

pmkobj *po_mk_hash(htable *pht) {
	pmkobj	*p;

	p = (pmkobj *) malloc(sizeof(pmkobj));
	if (p != NULL) {
		p->type = PO_HASH;
		p->data = pht; /* XXX copy ? */
	}

	return(p);
}

/*
	duplicate object

	po : object to copy

	return : copied object
*/

pmkobj *po_dup(pmkobj *po) {
	pmkobj	*dup;

	dup = (pmkobj *) malloc(sizeof(pmkobj));
	if (dup != NULL) {
		dup->type = po->type;
		switch (dup->type) {
			case PO_STRING :
				dup->data = strdup(po->data);
				break;

			default :
				return(NULL); /* XXX TODO */
				break;
		}
	}

	return(dup);
}

/*
	get object type

	po : objet

	return : type of object
*/

potype po_get_type(pmkobj *po) {
	if (po != NULL) {
		return(po->type);
	} else {
		return(PO_NULL);
	}
}

/*
	get object data

	po : objet

	return : generic data of object or NULL
*/

void *po_get_data(pmkobj *po) {
	if (po != NULL) {
		return(po->data);
	} else {
		return(NULL);
	}
}

/*
	get data from string object

	po : object

	return : string or NULL
*/

bool po_get_bool(pmkobj *po) {
	if (po != NULL) {
		if (po->type == PO_BOOL) {
			return(*(bool *) po->data);
		}
	}
	return(false); /* XXX grr should return a *bool */
}

/*
	get data from string object

	po : object

	return : string or NULL
*/

char *po_get_str(pmkobj *po) {
	if (po != NULL) {
		if (po->type == PO_STRING) {
			return(po->data);
		}
	}
	return(NULL);
}

/*
	get data from list object

	po : object

	return : dynary or NULL
*/

dynary *po_get_list(pmkobj *po) {
	if (po != NULL) {
		if (po->type == PO_LIST) {
			return(po->data);
		}
	}
	return(NULL);
}

/*
	free object

	po : object to deallocate

	return : -
*/

void po_free(pmkobj *po) {
	if (po != NULL) {
		switch (po->type) {
			case PO_NULL :
				break;
			case PO_BOOL :
				free(po->data);
				break;
			case PO_STRING :
				free(po->data);
				break;
			case PO_LIST :
				da_destroy(po->data);
				break;
			case PO_HASH :
				hash_destroy(po->data);
				break;
			default :
				free(po->data);
				break;
		}

		free(po);
	}
}

/*
	append a value in an object

	orig : original object
	value : data to append
	misc : misc specific data :
		- separator for a string object
		- unused for a list object 

	return : updated object or NULL if failed
*/

pmkobj *po_append(void *orig, void *value, void *misc) {
	char	*pbuf;
/*	dynary	*da; XXX TODO */
	pmkobj	*po_orig,
		*po_value,
		*po;
	size_t	 s;

	po_orig = (pmkobj *) orig;
	po_value = (pmkobj *) value;

	if (value == NULL)
		return(NULL);

	if (po_orig->type != po_value->type)
		return(NULL);

	po = (pmkobj *) malloc(sizeof(pmkobj));
	if (po == NULL) {
		return(NULL);
	}

	switch (po_orig->type) {
		case PO_BOOL :
			/* not supported */
			return(NULL);
			break;

		case PO_STRING :
			/* compute needed space */
			if (misc != NULL) {
				s = strlen((char *) misc);
			} else {
				s = 0;
			}
			s = s + strlen(po_orig->data) + strlen(po_value->data) + 1;

			/* allocate space */
			pbuf = (char *) malloc(s);

			if (strlcat(pbuf, po_orig->data, s) >= s) {
				free(value);
				free(pbuf);
				return(NULL);
			}
	
			if ((misc != NULL) && (pbuf[0] != '\0')) {
				/* adding separator if provided and if
					string is not empty */
				if (strlcat(pbuf, (char *) misc, s) >= s) {
					free(value);
					free(pbuf);
					return(NULL);
				}
			}
			if (strlcat(pbuf, po_value->data, s) >= s) {
				free(value);
				free(pbuf);
				return(NULL);
			}

			po = po_mk_str(pbuf);
			free(value);
			free(pbuf);
			break;

		case PO_LIST :
			/* XXX TODO */
			return(NULL);
			break;

		default :
			return(NULL);
			break;
	}

	return(po);
}
