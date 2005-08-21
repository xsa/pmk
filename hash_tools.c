/* $Id$ */

/*
 * Copyright (c) 2004-2005 Damien Couderc
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
#include <ctype.h>

#include "compat/pmk_stdio.h"
#include "compat/pmk_string.h"
#include "hash_tools.h"
#include "premake.h"


/*
	parse string for identifiers

	pstr : string to parse
	pbuf : resulting string buffer
	size : size of buffer

	returns : buffer address or NULL
*/

char *parse_idtf(char *pstr, char *pbuf, size_t size) {
	while (((isalnum(*pstr) != 0) || (*pstr == '_')) && (size > 0)) {
		*pbuf = *pstr;
		pbuf++;
		pstr++;
		size--;
	}

	if (size == 0)
		return(NULL);

	*pbuf = CHAR_EOS;
	
	/* return new cursor */
	return(pstr);
}



/*
	process string to substitute variables with their values

	pstr : string to process
	pht : hash table where variables are stored

	return : new string or NULL
*/

char *process_string(char *pstr, htable *pht) {
	bool	 bs = false;
	char	 buf[OPT_VALUE_LEN],
		 var[OPT_NAME_LEN],
		*pvar,
		*pval,
		*pbuf;
	size_t	 size;

	size = sizeof(buf);
	pbuf = buf;

	while ((*pstr != CHAR_EOS) && (size > 0)) {
		switch(*pstr) {
			case '\\' :
				bs = true;
				pstr++;
				break;

			case '$' :
				if (bs == false) {
					/* found variable */
					pstr++;
					pstr = parse_idtf(pstr, var, size);
					if (pstr == NULL) {
						/* debugf("parse_idtf returned null."); */
						return(NULL);
					} else {
						/* check if identifier exists */
						pvar = hash_get(pht, var);
						if (pvar != NULL) {
							/* process identifer value */
							pval = process_string(pvar, pht);
							pvar = pval;

							/* append value */
							while ((*pvar != CHAR_EOS) && (size > 0)) {
								*pbuf = *pvar;
								pbuf++;
								pvar++;
								size--;
							}

							/* clean processed value */
							free(pval);
						}
					}
				} else {
					/* copy character */
					*pbuf = *pstr;
					pbuf++;
					pstr++;
					size--;
					bs = false;
				}
				break;

			default :
				if (bs == true) {
					*pbuf = '\\';
					pbuf++;
					pstr++;
					size--;
					if (size == 0) {
					/* debugf("overflow."); */
						return(NULL);
					}
					bs = false;
				}
				/* copy character */
				*pbuf = *pstr;
				pbuf++;
				pstr++;
				size--;
				break;
		}
	}

	if (size == 0) {
		/* debugf("overflow.");*/
		return(NULL);
	}

	*pbuf = CHAR_EOS;
	return(strdup(buf));
}

/*
	append only if not already in the string

	pht : hash table
	key : key where to append
	value : value to append

	return : boolean
*/

bool single_append(htable *pht, char *key, char *value) {
	bool	 found = false;
	char	*cval,
		*pstr;
	size_t	 s;

	if (value == NULL)
		return(false);

	if (*value == CHAR_EOS)
		return(true);

	cval = hash_get(pht, key);

	pstr = strstr(cval, value);
	s = strlen (value);
	while ((pstr != NULL) && (found == false)) {
		pstr = pstr + s;
		if ((*pstr == ' ') || (*pstr == CHAR_EOS)) {
			/* found existing value */
			found = true;
		}
		pstr = strstr(pstr, value);
	}

	if (found == false) {
		if (hash_append(pht, key, value, " ") == HASH_ADD_FAIL) {
			/* add failed */
			return(false);
		}
	}

	return(true);
}


