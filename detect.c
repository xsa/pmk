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


#include <errno.h>
#include <stdlib.h>

#include "compat/pmk_stdio.h"
#include "compat/pmk_unistd.h"
#include "common.h"
#include "detect.h"
#include "lang.h"


/***************
 * keyword data *
 ***********************************************************************/

/*
 * ADD_COMPILER options
 */
kw_t	req_addcomp[] = {
	{CC_KW_ID,	PO_STRING},
	{CC_KW_DESCR,	PO_STRING},
	{CC_KW_MACRO,	PO_STRING}
};

kw_t	opt_addcomp[] = {
	{CC_KW_VERSION,		PO_STRING},
	{CC_KW_SLCFLAGS,	PO_STRING},
	{CC_KW_SLLDFLAGS,	PO_STRING}
};

kwopt_t	kw_addcomp = {
	req_addcomp,
	sizeof(req_addcomp) / sizeof(kw_t),
	opt_addcomp,
	sizeof(opt_addcomp) / sizeof(kw_t)
};

/*
 * ADD_SYSTEM options
 */
kw_t	req_addsys[] = {
	{SYS_KW_NAME,		PO_STRING},
	{SYS_KW_SH_EXT,		PO_STRING},
	{SYS_KW_SH_NONE,	PO_STRING},
	{SYS_KW_SH_VERS,	PO_STRING},
	{SYS_KW_ST_EXT,		PO_STRING},
	{SYS_KW_ST_NONE,	PO_STRING},
	{SYS_KW_ST_VERS,	PO_STRING},
	{SYS_KW_VERSION,	PO_STRING}
};

kwopt_t	kw_addsys = {
	req_addsys,
	sizeof(req_addsys) / sizeof(kw_t),
	NULL,	/* allow 'custom' options */
	0
};

prskw	kw_pmkcomp[] = {
	{"ADD_COMPILER",	PCC_TOK_ADDC,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_addcomp},
	{"ADD_SYSTEM",		PCC_TOK_ADDS,	PRS_KW_CELL,	PRS_TOK_NULL,	&kw_addsys}
};
int	nbkwpc = sizeof(kw_pmkcomp) / sizeof(prskw);


/*************
 * procedures *
 ***********************************************************************/

/*****************************
 * %PROC init_compiler_data() *
 ***********************************************************************
 * %DESCR initialize compiler data structure
 *
 * %PARAM pcd:	compiler data structure
 * %PARAM sz:	number of languages
 *
 * %RETURN boolean status
 ***********************************************************************/

bool init_compiler_data(comp_data_t *pcd, size_t lnb) {
	int	i;

	/* record size of compiler profile array */
	pcd->sz = lnb;

	/* allocate array of compiler profiles */
	pcd->data = (compiler_t *) malloc(sizeof(compiler_t) * lnb);

	if (pcd->data == NULL) {
		return false;
	}

	/* initialize every cell of the array */
	for (i = 0 ; i < (int) lnb ; i++) {
		pcd->data[i].c_id = NULL;
		pcd->data[i].descr = NULL;
		pcd->data[i].c_macro = NULL;
		pcd->data[i].v_macro = NULL;
		pcd->data[i].slcflags = NULL;
		pcd->data[i].slldflags = NULL;
		pcd->data[i].version = NULL;
		pcd->data[i].lang = LANG_UNKNOWN;
	}

	return true;
}


/******************************
 * %PROC clean_compiler_cell() *
 ***********************************************************************
 * %DESCR initialize compiler data structure
 *
 * %PARAM pc:	compiler cell structure
 *
 * %RETURN boolean status
 ***********************************************************************/

void clean_compiler_cell(compiler_t *pc) {
	if (pc->c_id != NULL) {
		free(pc->c_id);
	}

	if (pc->descr != NULL) {
		free(pc->descr);
	}

	if (pc->c_macro != NULL) {
		free(pc->c_macro);
	}

	if (pc->v_macro != NULL) {
		free(pc->v_macro);
	}

	if (pc->slcflags != NULL) {
		free(pc->slcflags);
	}

	if (pc->slldflags != NULL) {
		free(pc->slldflags);
	}

	if (pc->version != NULL) {
		free(pc->version);
	}
}


/******************************
 * %PROC clean_compiler_data() *
 ***********************************************************************
 * %DESCR initialize compiler data structure
 *
 * %PARAM pcd:	compiler data structure
 *
 * %RETURN boolean status
 ***********************************************************************/

void clean_compiler_data(comp_data_t *pcd) {
	int	i;

	/* clean every cell of the array */
	for (i = 0 ; i < (int) pcd->sz ; i++) {
		clean_compiler_cell(&(pcd->data[i]));
	}

	/* free allocated memory of the	array */
	free(pcd->data);

	pcd->sz = 0;
}

	
/***************************
 * %PROC compcell_destroy() *
 ***********************************************************************
 * %DESCR clean comp_prscell_t structure
 *
 * %PARAM pcc: structure to clean
 *
 * %RETURN NONE
 ***********************************************************************/

void compcell_destroy(comp_prscell_t *pcc) {
	free(pcc->c_id);
	free(pcc->descr);
	free(pcc->c_macro);
	free(pcc->v_macro);
	free(pcc->slcflags);
	free(pcc->slldflags);
	free(pcc);
}


/**************************
 * %PROC init_comp_parse() *
 ***********************************************************************
 * %DESCR initialize comp_parse_t structure from compiler data file
 *
 * %RETURN new structure or NULL on failure
 ***********************************************************************/

comp_parse_t *init_comp_parse(void) {
	comp_parse_t	*pcp;

	/* allocate compiler data structure */
	pcp = (comp_parse_t *) malloc(sizeof(comp_parse_t));
	if (pcp == NULL) {
		return NULL;
	}

	/* init compilers hash table */
	pcp->cht = hash_init_adv(MAX_COMP, NULL, (void (*)(void *)) compcell_destroy, NULL);
	if (pcp->cht == NULL) {
		free(pcp);
		/*debugf("cannot initialize comp_data");*/
		return NULL;
	}

	/* init systems hash table */
	pcp->sht = hash_init(MAX_OS);
	if (pcp->sht == NULL) {
		free(pcp);
		hash_destroy(pcp->sht);
		/*debugf("cannot initialize comp_data");*/
		return NULL;
	}

	/* init ok */
	return pcp;
}


/*****************************
 * %PROC destroy_comp_parse() *
 ***********************************************************************
 * %DESCR destroy comp_parse_t structure
 *
 * %PARAM pcp:	compiler parsed data structure
 *
 * %RETURN none
 ***********************************************************************/

void destroy_comp_parse(comp_parse_t *pcp) {
	/* destroy compiler cell hash table */
	hash_destroy(pcp->cht);

	/* destroy system cell hash table */
	hash_destroy(pcp->sht);
}


/***********************
 * %PROC add_compiler() *
 ***********************************************************************
 * %DESCR add a new compiler cell
 *
 * %PARAM pcp:	compiler parsed data structure
 * %PARAM pht:	compiler cell hash table
 *
 * %RETURN boolean
 ***********************************************************************/

bool add_compiler(comp_parse_t *pcp, htable *pht) {
	comp_prscell_t	*pcell;
	char			*pstr,
					 tstr[TMP_BUF_LEN];

	pcell = (comp_prscell_t *) malloc(sizeof(comp_prscell_t));
	if (pcell == NULL)
		return false;

	pstr = po_get_str(hash_get(pht, CC_KW_ID));
	if (pstr == NULL) {
		free(pcell);
		return false;
	} else {
		pcell->c_id = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, CC_KW_DESCR));
	if (pstr == NULL) {
		free(pcell);
		return false;
	} else {
		pcell->descr = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, CC_KW_MACRO));
	if (pstr == NULL) {
		free(pcell);
		return false;
	} else {
		pcell->c_macro = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, CC_KW_VERSION));
	if (pstr == NULL) {
		pcell->v_macro = strdup(DEF_NOVERSION);
	} else {
		if (snprintf_b(tstr, sizeof(tstr),
					DEF_VERSION, pstr) == false) {
			free(pcell);
			return false;
		}
		pcell->v_macro = strdup(tstr);
	}

	pstr = po_get_str(hash_get(pht, CC_KW_SLCFLAGS));
	if (pstr == NULL) {
		pcell->slcflags = strdup(""); /* default to empty string */
	} else {
		pcell->slcflags = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, CC_KW_SLLDFLAGS));
	if (pstr == NULL) {
		pcell->slldflags = strdup(""); /* default to empty string */
	} else {
		pcell->slldflags = strdup(pstr);
	}

	/* no need to strdup */
	if (hash_update(pcp->cht, pcell->c_id, pcell) == HASH_ADD_FAIL) {
		return false;
	}

	return true;
}


/*********************
 * %PROC add_system() *
 ***********************************************************************
 * %DESCR add a new system cell
 *
 * %PARAM pcp:	compiler parsed data structure
 * %PARAM pht:	system cell hash table
 *
 * %RETURN boolean
 ***********************************************************************/

bool add_system(comp_parse_t *pcp, htable *pht, char *osname) {
	char			*name,
					*pstr;
	hkeys			*phk;
	unsigned int	 i;

	name = po_get_str(hash_get(pht, SYS_KW_NAME));
	if (name == NULL) {
		return false;
	}

	if (strncmp(osname, name, sizeof(osname)) != 0) {
		/* not the target system, skipping */
		return true;
	}

	hash_add(pht, SL_SYS_LABEL, po_mk_str(strdup(name))); /* XXX check */
	hash_delete(pht, SYS_KW_NAME); /* XXX check ? */

	/* remaining values are system overrides, save it */
	phk = hash_keys(pht);
	for (i = 0 ; i < phk->nkey ; i++) {
		pstr = po_get_str(hash_get(pht, phk->keys[i]))	;
		hash_update_dup(pcp->sht, phk->keys[i], pstr); /* XXX check */
	}

	return true;
}


/**************************
 * %PROC parse_comp_file() *
 ***********************************************************************
 * %DESCR parse compiler data file
 *
 * %PARAM cdfile:	compiler data file to parse
 * %PARAM osname:	name of the target OS
 *
 * %RETURN pointer to allocated compiler parsing structure
 ***********************************************************************/

comp_parse_t *parse_comp_file(char *cdfile, char *osname) {
	FILE			*fd;
	bool			 rval;
	comp_parse_t	*pcp;
	prscell			*pcell;
	prsdata			*pdata;

	/* try to open compilers data file */
	fd = fopen(cdfile, "r");
	if (fd == NULL) {
		errorf("cannot open '%s' : %s.", cdfile, strerror(errno));
		return NULL;
	}

	/* init compiler data structure */
	pcp = init_comp_parse();
	if (pcp == NULL) {
		fclose(fd);
		return NULL;
	}

	/* initialize parsing structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot initialize prsdata.");
		destroy_comp_parse(pcp);
		fclose(fd);
		return NULL;
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fd, pdata, kw_pmkcomp, nbkwpc);
	fclose(fd);

	if (rval == true) {
		pcell = pdata->tree->first;

		while (pcell != NULL) {
			switch(pcell->token) {
				case PCC_TOK_ADDC :
					if (add_compiler(pcp, pcell->data) == false) {
						errorf("add_compiler() failed in parse_comp_file_adv().");
						destroy_comp_parse(pcp);
						prsdata_destroy(pdata);
						return NULL;
					}
					break;

				case PCC_TOK_ADDS :
					if (osname != NULL) {
						if (add_system(pcp, pcell->data, osname) == false) {
							errorf("add_system() failed in parse_comp_file_adv().");
							destroy_comp_parse(pcp);
							prsdata_destroy(pdata);
							return NULL;
						}
					}
					break;

				default :
					errorf("parsing of data file failed.");
					destroy_comp_parse(pcp);
					prsdata_destroy(pdata);
					return NULL;
					break;
			}

			pcell = pcell->next;
		}
	} else {
		errorf("parsing of data file failed.");
		destroy_comp_parse(pcp);
		prsdata_destroy(pdata);
		return NULL;
	}

	/* parsing data no longer needed */
	prsdata_destroy(pdata);

	return pcp;
}


/************************
 * %PROC gen_test_file() *
 ***********************************************************************
 * %DESCR generate test file
 *
 * %PARAM pcp:		compiler data structure
 * %PARAM fnbuf:	filename buffer
 * %PARAM bsz:		buffer size
 *
 * %RETURN boolean status
 ***********************************************************************/

bool gen_test_file(comp_parse_t *pcp, char *fnbuf, size_t bsz) {
	FILE				*fp;
	comp_prscell_t		*pcell;
	hkeys				*phk;
	unsigned int		 i;

	/* get key list of parsed compiler data */
	phk = hash_keys(pcp->cht);
	if (phk == NULL) {
		errorf("no parsed compiler data found");
		return false;
	}

	/* open source file for writing */
	fp = tmps_open(CC_TEST_FILE, "w", fnbuf, bsz, strlen(CC_TFILE_EXT));
	if (fp == NULL) {
		errorf("cannot open compiler test file ('%s').", fnbuf);
		hash_free_hkeys(phk);
		return false;
	}

	/*
	 * fill the source test file with the code that try to identify the compiler
	 */
	fprintf(fp, COMP_TEST_HEADER);

	for (i = 0 ; i < phk->nkey ; i++) {
		pcell = hash_get(pcp->cht, phk->keys[i]);
		fprintf(fp, COMP_TEST_FORMAT, pcell->descr, pcell->c_macro, pcell->c_id, pcell->v_macro);
	}

	fprintf(fp, COMP_TEST_FOOTER);

	fclose(fp);
	hash_free_hkeys(phk);

	return true;
}


/**********************
 * %PROC comp_identify *
 ***********************************************************************
 * %DESCR identify given compiler
 *
 * %PARAM cpath:	path of the compiler to identify
 * %PARAM blog:		build log file name
 * %PARAM pc:		compiler cell to populate
 * %PARAM pcp:		compiler parsing structure
 *
 * %RETURN boolean
 ***********************************************************************/

bool comp_identify(char *cpath, char *blog, compiler_t *pc, comp_parse_t *pcp) {
	FILE		*rpipe;
	bool		 failed = false;
	char		 cfgcmd[MAXPATHLEN],
				 ftmp[MAXPATHLEN],
				 pipebuf[TMP_BUF_LEN];
	int			 r;

	if (gen_test_file(pcp, ftmp, sizeof(ftmp)) == false) {
		return false;
	}

	/* build compiler command */
	if (snprintf_b(cfgcmd, sizeof(cfgcmd), CC_TEST_FORMAT,
				cpath, CC_TEST_BIN, ftmp, blog) == false) {
		errorf("failed to build compiler command line.");
		return false;
	}

	/* get result */
	r = system(cfgcmd);

	/* test file no longer needed */
	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("cannot remove %s : %s.", ftmp, strerror(errno));
	}

	/* if compiled without error */
	if (r == 0) {
		rpipe = popen(CC_TEST_BIN, "r");
		if (rpipe != NULL) {
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get compiler id.");
				failed = true;
			} else {
				/* get compiler id */
				pc->c_id = strdup(pipebuf);

				if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
					errorf("cannot get compiler version.");
					failed = true;
				} else {
					pc->version = strdup(pipebuf);
				}
			}

			pclose(rpipe);

		} else {
			errorf("failed to get output from test binary.");
		}

		/* delete binary */
		if (unlink(CC_TEST_BIN) == -1) {
			errorf("cannot remove %s : %s.", CC_TEST_BIN, strerror(errno));
		}

		if (failed == true) {
			return false;
		}
	} else {
		errorf("failed to build test binary : %s.", strerror(errno));
		return false;
	}

	return true;
}


/********************
 * %PROC comp_detect *
 ***********************************************************************
 * %DESCR identify given compiler
 *
 * %PARAM cpath:	path of the compiler to identify
 * %PARAM blog:		build log file name
 * %PARAM pc:		compiler cell to populate
 * %PARAM pcp:		compiler parsing structure
 * %PARAM label:		XXX
 *
 * %RETURN boolean
 ***********************************************************************/

bool comp_detect(char *cpath, char *blog, compiler_t *pc, comp_parse_t *pcp, char *label) {
	char			 buf[TMP_BUF_LEN],
					*pstr;
	comp_prscell_t	*pcell;

	if (comp_identify(cpath, blog, pc, pcp) == false) {
		return false;
	}

	/* look for the detected compiler data */
	pcell = hash_get(pcp->cht, pc->c_id);
	if (pcell == NULL) {
		errorf("unknown compiler identifier : %s\n", pc->c_id);
		return false;
	}

	/*
	 * populate compiler cell with identified compiler data
	 *
	 * c_id and version have already been set by comp_identify
	 * lang XXX ?
	 */
	pc->descr = strdup(pcell->descr);
	pc->c_macro = strdup(pcell->c_macro); /* XXX useless ? */
	pc->v_macro = strdup(pcell->v_macro); /* XXX useless ? */

	/*
	 * check system specific flags for shared library support
	 */

	/* generate tag for system specific compiler flags */
	if (snprintf_b(buf, sizeof(buf), "%s_%s", label, pc->c_id) == false) {
		errorf("overflow.\n");
		return(false);
	}

	/* check if an override exists for compiler flags */
	pstr = hash_get(pcp->sht, buf);
	if (pstr == NULL) {
		/* take compiler's generic flags */
		pc->slcflags = strdup(pcell->slcflags);
	} else {
		/* take system specific flags */
		pc->slcflags = strdup(pstr);
	}

	/* generate tag for system specific linker flags */
	if (snprintf_b(buf, sizeof(buf), "%s_%s", SL_LDFLAG_VARNAME, pc->c_id) == false) {
		errorf("overflow.\n");
		return(false);
	}

	/* check if an override exists for linker flags */
	pstr = hash_get(pcp->sht, buf);
	if (pstr == NULL) {
		/* take compiler's generic flags */
		pc->slldflags = strdup(pcell->slldflags);
	} else {
		/* take system specific flags */
		pc->slldflags = strdup(pstr);
	}
	
	/*pcd->data[i].lang = LANG_UNKNOWN;*/

	return true;
}

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
