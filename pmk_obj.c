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
	XXX useful ?
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
	XXX
*/

potype po_get_type(pmkobj *po) {
	if (po != NULL) {
		return(po->type);
	} else {
		return(PO_NULL);
	}
}

/*
	XXX
*/

void *po_get_data(pmkobj *po) {
	if (po != NULL) {
		return(po->data);
	} else {
		return(NULL);
	}
}

/*
	XXX
*/

void po_free(pmkobj *po) {
	if (po != NULL) {
		switch (po->type) {
			case PO_NULL :
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
	XXX
*/

pmkobj *po_append(void *orig, void *value, void *misc) {
	char	*pbuf;
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
