/* $Id$ */

/*
 * Copyright (c) 2005 Damien Couderc
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


#include <stdio.h>

#include "compat/pmk_string.h"
#include "parse_lang.h"


/*#define DEBUG_PRSC	1*/


/**********
 variables
***********************************************************************/

/* preprocessor keywords */
char	*pp_keywords[] = {
	RKW_PP_DEF,
	RKW_PP_ELIF,
	RKW_PP_ELSE,
	RKW_PP_ENDF,
	RKW_PP_ERR,
	RKW_PP_IF,
	RKW_PP_IFDF,
	RKW_PP_IFND,
	RKW_PP_INCL,
	RKW_PP_LINE,
	RKW_PP_PRGM,
	RKW_PP_UDEF
};

/* C language reserved keywords (without types) */
char	*c_keywords[] = {
	RKW_C_BOOL, RKW_C_CMPLX, RKW_C_IMGNR,
	RKW_C_AUTO,
	RKW_C_BREAK,
	RKW_C_CASE, RKW_C_CHAR, RKW_C_CONST, RKW_C_CONTN,
	RKW_C_DFLT, RKW_C_DO, RKW_C_DBL,
	RKW_C_ELSE, RKW_C_ENUM, RKW_C_EXTRN,
	RKW_C_FLOAT, RKW_C_FOR,
	RKW_C_GOTO,
	RKW_C_IF, RKW_C_INLN, RKW_C_INT,
	RKW_C_LONG,
	RKW_C_RGSTR, RKW_C_RSTCT, RKW_C_RTRN,
	RKW_C_SHORT, RKW_C_SGND, RKW_C_SIZOF, RKW_C_STTC, RKW_C_STRCT, RKW_C_SWTCH,
	RKW_C_TPDEF,
	RKW_C_UNION, RKW_C_USGND,
	RKW_C_VOID, RKW_C_VLTL,
	RKW_C_WHILE
};
size_t nb_c_keywords = sizeof(c_keywords) / sizeof(char *);

/* C language reserved keywords */
char	*c_type_keywords[] = {
	RKW_C_BOOL, RKW_C_CMPLX, RKW_C_IMGNR,
	RKW_C_CHAR,
	RKW_C_DBL,
	RKW_C_FLOAT,
	RKW_C_INT,
	RKW_C_LONG,
	RKW_C_SHORT,
	RKW_C_VOID
};
size_t nb_c_type_keywords = sizeof(c_type_keywords) / sizeof(char *);


/**********
 functions
***********************************************************************/

/*********************
 prs_c_skip_to_char()

 DESCR
	skip until a given char

 IN
	OUT
************************************************************************/

bool prs_c_skip_to_char(prseng_t *ppe, char c) {
	bool	flag = false;

	/* check for end of line or end of file */
	while ((prseng_eof(ppe) == false) &&
			((prseng_test_char(ppe, c) == false) || (flag == false))) {
		if (prseng_test_char(ppe, '\\') == true) {
			/* unset the flag to avoid stopping if newline is
				preceded by the '\' character */
			flag = false;
		} else {
			/* else unset flag */
			flag = true;
		}

		/* skip character */
		if (prseng_next_char(ppe) == false) {
			return(false);
		}
	}

	if (prseng_test_char(ppe, c) == true) {
		/* skip newline */
		if (prseng_next_char(ppe) == false) {
			return(false);
		}
	}

	return(true);
}


/******************
 prs_c_line_skip()

 DESCR
	skip until end of file

 IN
	OUT
************************************************************************/

bool prs_c_line_skip(prseng_t *ppe) {
	bool	flag = false;

	/* check for end of line or end of file */
	while ((prseng_eof(ppe) == false) &&
			((prseng_test_char(ppe, '\n') == false) || (flag == false))) {
		if (prseng_test_char(ppe, '\\') == true) {
			/* unset the flag to avoid stopping if newline is
				preceded by the '\' character */
			flag = false;
		} else {
			/* else unset flag */
			flag = true;
		}

		/* skip character */
		if (prseng_next_char(ppe) == false) {
			return(false);
		}
	}

	if (prseng_test_char(ppe, '\n') == true) {
		/* skip newline */
		if (prseng_next_char(ppe) == false) {
			return(false);
		}
	}

	return(true);
}


/*********************
 prs_c_comment_skip()

 DESCR
	skip C style comments

 IN
	OUT
************************************************************************/

bool prs_c_comment_skip(prseng_t *ppe) {
	bool	flag = false,
			loop = true;

	/* skip '/' character */
	prseng_next_char(ppe);

	if (prseng_eof(ppe) == true) {
		/* XXX err msg unexpected eof */
		return(false);
	}

	switch(prseng_get_char(ppe)) {
		case '*' :
			/* skip '*' character */
			prseng_next_char(ppe);

			if (prseng_eof(ppe) == true) {
				/* XXX err msg unexpected eof */
				return(false);
			}

			/* loop until end of comment is found */
			while (loop == true) {
				if (flag == true) {
					if (prseng_test_char(ppe, '/') == true) {
						/* can exit from the loop after skipping this char */
						loop = false;
					} else {
						/* else unset flag */
						flag = false;
					}
				}

				if (prseng_test_char(ppe, '*') == true) {
					/* set flag to stop if the next char is '/' */
					flag = true;
				}

				/* going to next char */
				prseng_next_char(ppe);

				if (prseng_eof(ppe) == true) {
					/* XXX err msg unexpected eof */
					return(false);
				}
			}
			break;

		case '/' :
			/* skip second '/' character */
			prseng_next_char(ppe);

			if (prs_c_line_skip(ppe) == false) {
				return(false);
			}
			break;

	}

	return(true);
}


/********************
 prs_c_squote_skip()

 DESCR
	process simple quotes

 IN
	OUT
************************************************************************/

bool prs_c_squote_skip(prseng_t *ppe) {
	/* skip starting quote */
	prseng_next_char(ppe);

	if (prseng_eof(ppe) == true) {
		/* XXX err msg unexpected eof */
		return(false);
	}

	/* if it's an escape character ... */
	if (prseng_test_char(ppe, '\\') == true) {
		/* ... then skip it */
		prseng_next_char(ppe);
	}

	if (prseng_eof(ppe) == true) {
		/* XXX err msg unexpected eof */
		return(false);
	}

	/* skip quoted character */
	prseng_next_char(ppe);

	if (prseng_eof(ppe) == true) {
		/* XXX err msg unexpected eof */
		return(false);
	}

	if (prseng_test_char(ppe, '\'') == false) {
		/* XXX msg cannot found ending quote ! */
		return(false);
	}

	/* skip ending quote */
	prseng_next_char(ppe);

	return(true);
}


/********************
 prs_c_dquote_skip()

 DESCR
	process double quotes
 IN
	OUT
************************************************************************/

bool prs_c_dquote_skip(prseng_t *ppe) {
	bool	escape = false;

	/* skip starting double quote */
	prseng_next_char(ppe);

	while ((prseng_test_char(ppe, '"') == false) || (escape == true)) {
		escape = false;

#ifdef DEBUG_PRSC
	debugf("current char = %c", prseng_get_char(ppe));
#endif

		if (prseng_eof(ppe) == true) {
			/* XXX msg cannot find ending double quote ! */
			return(false);
		}

		if (prseng_test_char(ppe, '\\') == true) {
			escape = true;
		}

		prseng_next_char(ppe);
	}

	/* skip ending double quote */
	prseng_next_char(ppe);

	return(true);
}


/*************
 prs_c_skip()

 DESCR
	skip useless stuff like spaces, tabs, newlines and comments

 IN
	OUT
************************************************************************/

void prs_c_skip(prseng_t *ppe) { /* XXX bool */
	bool	do_exit = false;

#ifdef DEBUG_PRSC
	/*debugf("prs_c_skip() : start");*/
#endif
	while (do_exit == false) {
#ifdef DEBUG_PRSC
	/*debugf("prs_c_skip() : char number is %d", (int) prseng_get_char(ppe));*/
#endif
		switch(prseng_get_char(ppe)) {
			case ' ' :
			case '\t' :
			case '\n' :
			case '\f' :
			case 13 :
				/* skip character */
				prseng_next_char(ppe);
				break;

			case '/' :
				/* comment ? */
				prs_c_comment_skip(ppe);
				break;

			default:
				do_exit = true;
		}
	}
#ifdef DEBUG_PRSC
	/*debugf("prs_c_skip() : stop");*/
#endif
}


/***************
 prs_c_prepro()

 DESCR
	handle preprocesor directives

 IN
	OUT
************************************************************************/

bool prs_c_prepro(prs_cmn_t *pcmn, prseng_t *ppe) {
	char	 pp_idtf[MAX_IDTF_LEN];

	/* skip leading '#' character */
	prseng_next_char(ppe);

	if (prseng_eof(ppe) == true) {
		/* XXX err msg unexpected eof */
		return(false);
	}

	if (pcmn->func_ppro != NULL) {
		/* process directive with a call to pcmn->func_ppro() */
		prseng_get_idtf(ppe, pp_idtf, sizeof(pp_idtf), PRS_C_IDTF_STR);

		pcmn->func_ppro(pcmn->data, pp_idtf, ppe);
	} else {
		/* skip directive */
		if (prs_c_line_skip(ppe) == false) {
			return(false);
		}
	}

	return(true);
}


/**************
 prs_c_is_kw()

 DESCR
	check if identifier is a keyword

 IN
 OUT
************************************************************************/

bool prs_c_is_kw(char *idtf, char **kw, size_t nbkw) {
	size_t	i,
			s;

	/* get the size of the identifier */
	s = strlen(idtf) + 1;

	/* loop into the list of keywords */
	for (i = 0 ; i < nbkw ; i++) {
		if (strncmp(idtf, kw[i], s) == 0) {
			/* and return true if one matches */
			return(true);
		}
	}

	/* not a known C keyword */
	return(false);
}


/*************
 prs_c_file()

 DESCR
	C file parsing main function

 IN
	OUT
************************************************************************/

bool prs_c_file(prs_cmn_t *pcmn, FILE *fp) {
	bool			 idtf_flag = false,
					 type_flag = false;
	char			 idtf[MAX_IDTF_LEN],
					 type[MAX_IDTF_LEN];
	prseng_t		*ppe;

	/* init prseng	 */
	ppe = prseng_init(fp, NULL);
	if (ppe == NULL) {
		return(false);
	}

	/* while end of file is not reached */
	while (prseng_eof(ppe) == false) {
#ifdef DEBUG_PRSC
		/*debugf("cursor: '%.16s'", ppe->cur);*/
#endif
		prs_c_skip(ppe);
#ifdef DEBUG_PRSC
		debugf("cursor after skipping useless : '%.16s'", ppe->cur);
#endif

		if (prseng_test_char(ppe, '#') == true) {
			/* parse preprocessing directive */
			prs_c_prepro(pcmn, ppe);
			continue;
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not '#'");
#endif
		}

		if (prseng_test_char(ppe, '(') == true) {
			if (idtf_flag == true) {
				if (type_flag == true) {
#ifdef DEBUG_PRSC
					debugf("possible function declaration of '%s'", idtf);
#endif
					/* if an identifier flag is on => function call */
					if (pcmn->func_decl != NULL) {
						if (pcmn->func_decl(pcmn->data, idtf, ppe) == false) {
							return(false);
						}
					}

				} else {
#ifdef DEBUG_PRSC
					debugf("possible function call of '%s'", idtf);
#endif
					/* if an identifier flag is on => function call */
					if (pcmn->func_proc != NULL) {
						if (pcmn->func_proc(pcmn->data, idtf, ppe) == false) {
							return(false);
						}
					}

				}
				idtf_flag = false;
			}

			/* skip character */
			prseng_next_char(ppe);

			continue;
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not '('");
#endif
		}

		if (prseng_test_char(ppe, '*') == true) {
			if (idtf_flag == true) {
				/* if the idtf flag is on => type identifier */
#ifdef DEBUG_PRSC
				debugf("possible type identifier '%s'", idtf);
#endif
				/* processing, call to pcmn->func_type */
				pcmn->func_type(pcmn->data, idtf, ppe);

				idtf_flag = false;
			}
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not '*'");
#endif
		}

		if (prseng_test_char(ppe, '[') == true) {
			if (type_flag == true) {
				/* if a type flag is on => type identifier */
#ifdef DEBUG_PRSC
				debugf("possible type identifier '%s'", type);
#endif
				/* processing, call to pcmn->func_type */
				pcmn->func_type(pcmn->data, idtf, ppe);

				type_flag = false;
			}

			/* skip character */
			prseng_next_char(ppe);

			continue;
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not '['");
#endif
		}

		if (prseng_test_char(ppe, '"') == true) {
			if (prs_c_dquote_skip(ppe) == false) {
				return(false);
			}

			continue;
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not '\"'");
#endif
		}

		if (prseng_test_char(ppe, '\'') == true) {
			if (prs_c_squote_skip(ppe) == false) {
				return(false);
			}

			continue;
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not '\''");
#endif
		}

		/* if it's a misc char ... */
		if (prseng_test_idtf_char(PRS_C_MISC_STR,
					prseng_get_char(ppe)) == true) {
#ifdef DEBUG_PRSC
			debugf("cursor '%.16s is a misc char", ppe->cur);
#endif
			/* clear flags */
			idtf_flag = false; /* XXX here idtf could contain a constant */
			type_flag = false;

			/* skip character */
			prseng_next_char(ppe);

			continue;
#ifdef DEBUG_PRSC
		} else {
				debugf("cursor is not a misc char");
#endif
		}

		if (idtf_flag == true) {
			/* save previous idtf as it could be a type */
			strlcpy(type, idtf, sizeof(type)); /* no check needed */
#ifdef DEBUG_PRSC
			debugf("save type identifier '%s'", type);
#endif
			idtf_flag = false;
			type_flag = true;
		}

		prseng_get_idtf(ppe, idtf, sizeof(idtf), PRS_C_IDTF_STR);
#ifdef DEBUG_PRSC
			debugf("found identifier '%s'", idtf);
#endif

		/* check if the identifier is a type keyword */
		if (prs_c_is_kw(idtf, c_type_keywords, nb_c_type_keywords) == true) {
			/* if yes then we have to mark this identifier */
			type_flag = true;
#ifdef DEBUG_PRSC
			debugf("skipped type keyword '%s'", idtf);
#endif
			continue;
		}

		/* check if the identifier is a keyword */
		if (prs_c_is_kw(idtf, c_keywords, nb_c_keywords) == false) {
			/* if not then we have to mark this identifier */
			idtf_flag = true;
#ifdef DEBUG_PRSC
		} else {
			debugf("skipped keyword '%s'", idtf);
#endif
		}
	}

	prseng_destroy(ppe);

	return(true);
}
