/* $Id$ */

/*
 * Copyright (c) 2003-2004 Damien Couderc
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
#include <stdio.h>
#include <stdlib.h>

#include "compat/pmk_unistd.h"
#include "common.h"
#include "detect.h"

prskw	kw_pmkcomp[] = {
	{"ADD_COMPILER",	PCC_TOK_ADDC,	PRS_KW_CELL},
	{"ADD_SYSTEM",		PCC_TOK_ADDS,	PRS_KW_CELL}
};

int	nbkwpc = sizeof(kw_pmkcomp) / sizeof(prskw);


/*
	init comp_data structure

	return : new structure or NULL
*/

comp_data *compdata_init(size_t csize, size_t ssize) {
	comp_data	*cdata;

	/* allocate compiler data structure */
	cdata = (comp_data *) malloc(sizeof(comp_data));
	if (cdata == NULL)
		return(NULL);

	/* init compilers hash table */
	cdata->cht = hash_init_adv(csize, NULL, (void (*)(void *)) compcell_destroy, NULL);
	if (cdata->cht == NULL) {
		free(cdata);
		/*debugf("cannot initialize comp_data");*/
		return(NULL);
	}

	/* init systems hash table */
	cdata->sht = hash_init(ssize);
	if (cdata->sht == NULL) {
		free(cdata);
		hash_destroy(cdata->sht);
		/*debugf("cannot initialize comp_data");*/
		return(NULL);
	}

	/* init ok */
	return(cdata);
}

/*
	clean comp_data structure

	pcd : structure to clean

	return : -
*/

void compdata_destroy(comp_data *pcd) {
	hash_destroy(pcd->cht);
	hash_destroy(pcd->sht);
}

/*
	clean comp_cell structure

	pcc : structure to clean

	return : -
*/

void compcell_destroy(comp_cell *pcc) {
	free(pcc->c_id);
	free(pcc->descr);
	free(pcc->c_macro);
	free(pcc->v_macro);
	free(pcc->slcflags);
	free(pcc->slldflags);
	free(pcc);
}

/*
	add a new compiler cell

	pcd : compiler data structure
	pht : parsed data

	return : boolean
*/

bool add_compiler(comp_data *pcd, htable *pht) {
	comp_cell	*pcell;
	char		*pstr,
			 tstr[TMP_BUF_LEN];

	pcell = (comp_cell *) malloc(sizeof(comp_cell));
	if (pcell == NULL)
		return(false);

	pstr = po_get_str(hash_get(pht, "ID"));
	if (pstr == NULL) {
		free(pcell);
		return(false);
	} else {
		pcell->c_id = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, "DESCR"));
	if (pstr == NULL) {
		free(pcell);
		return(false);
	} else {
		pcell->descr = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, "MACRO"));
	if (pstr == NULL) {
		free(pcell);
		return(false);
	} else {
		pcell->c_macro = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, "VERSION"));
	if (pstr == NULL) {
		pcell->v_macro = strdup(DEF_NOVERSION);
	} else {
		if (snprintf_b(tstr, sizeof(tstr),
					DEF_VERSION, pstr) == false) {
			free(pcell);
			return(false);
		}
		pcell->v_macro = strdup(tstr);
	}

	pstr = po_get_str(hash_get(pht, "SLCFLAGS"));
	if (pstr == NULL) {
		pcell->slcflags = strdup(""); /* default to empty string */
	} else {
		pcell->slcflags = strdup(pstr);
	}

	pstr = po_get_str(hash_get(pht, "SLLDFLAGS"));
	if (pstr == NULL) {
		pcell->slldflags = strdup(""); /* default to empty string */
	} else {
		pcell->slldflags = strdup(pstr);
	}

	/* no need to strdup */
	if (hash_update(pcd->cht, pcell->c_id, pcell) == HASH_ADD_FAIL)
		return(false);

	return(true);
}

/*
	add a new system cell

	pcd : compiler data structure
	pht : parsed data

	return : boolean
*/

bool add_system(comp_data *pcd, htable *pht, char *osname) {
	char		*name,
			*pstr;
	hkeys		*phk;
	unsigned int	 i;

	name = po_get_str(hash_get(pht, "NAME"));
	if (name == NULL) {
		return(false);
	} else {
		if (strncmp(osname, name, sizeof(osname)) != 0) {
			/* not the target system, skipping */
			return(true);
		} else {
			hash_delete(pht, "NAME");
		}
	}

	/* remaining values are system overrides, save it */
	phk = hash_keys(pht);
	for(i = 0 ; i < phk->nkey ; i++) {
		pstr = po_get_str(hash_get(pht, phk->keys[i]))	;
		hash_update_dup(pcd->sht, phk->keys[i], pstr);
	}

	return(true);
}

/*
	get compiler data

	pcd : compiler data structure
	c_id : compiler id

	return : compiler cell structure
*/

comp_cell *comp_get(comp_data *pcd, char *c_id) {
	return(hash_get(pcd->cht, c_id));
}

/*
	get compiler descr
*/

char *comp_get_descr(comp_data *pcd, char *c_id) {
	comp_cell	*pcell;

	pcell = hash_get(pcd->cht, c_id);

	return(pcell->descr);
}

/*
	parse data from PMKCOMP_DATA file

	cdfile : compilers data file
	pht: config hash table

	return : compiler data structure or NULL
*/

comp_data *parse_comp_file_adv(char *cdfile, htable *pht) {
	FILE		*fd;
	bool		 rval;
	char		*osname = NULL;
	comp_data	*cdata;
	prscell		*pcell;
	prsdata		*pdata;

	/* init compiler data structure */
	cdata = compdata_init(MAX_COMP, MAX_OS);
	if (cdata == NULL) {
		return(NULL);
	}

	/* initialize parsing structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		compdata_destroy(cdata);
		errorf("cannot initialize prsdata.");
		return(NULL);
	}

	fd = fopen(cdfile, "r");
	if (fd == NULL) {
		compdata_destroy(cdata);
		prsdata_destroy(pdata);
		errorf("cannot open '%s' : %s.", cdfile, strerror(errno));
		return(NULL);
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fd, pdata, kw_pmkcomp, nbkwpc);
	fclose(fd);

	if (rval == true) {
		pcell = pdata->tree->first;

		if (pht != NULL) {
			osname = hash_get(pht, PMKCONF_OS_NAME);
		}

		while (pcell != NULL) {
			switch(pcell->token) {
				case PCC_TOK_ADDC :
/*debugf("calling add_compiler()");*/
					if (add_compiler(cdata, pcell->data) == false) {
						errorf("add_compiler() failed in parse_comp_file_adv().");
						compdata_destroy(cdata);
						prsdata_destroy(pdata);
						return(NULL);
					}
					break;

				case PCC_TOK_ADDS :
/*debugf("calling add_system()");*/
					if (osname != NULL) {
						if (add_system(cdata, pcell->data, osname) == false) {
							errorf("add_system() failed in parse_comp_file_adv().");
							compdata_destroy(cdata);
							prsdata_destroy(pdata);
							return(NULL);
						}
					}
					break;
			
				default :
					errorf("parsing of data file failed.");
					compdata_destroy(cdata);
					prsdata_destroy(pdata);
					return(NULL);
					break;
			}

			pcell = pcell->next;
		}
	} else {
		errorf("parsing of data file failed.");
		compdata_destroy(cdata);
		prsdata_destroy(pdata);
		return(NULL);
	}

	/* parsing data no longer needed */
	prsdata_destroy(pdata);

	return(cdata);
}

/*
	parse data from PMKCOMP_DATA file

	cdfile : compilers data file

	return : compiler data structure or NULL
*/

comp_data *parse_comp_file(char *cdfile) {
	return(parse_comp_file_adv(cdfile, NULL));
}

/*
	generate test file

	fp : target file
	pcd : compiler data structure

	return : boolean
*/

bool gen_test_file(FILE *fp, comp_data *pcd) {
	comp_cell	*pcell;
	hkeys		*phk;
	htable		*pht;
	unsigned int	 i;

	pht = pcd->cht;

	phk = hash_keys(pht);
	if (phk == NULL) {
		debugf("cannot get hash keys.");
	}

	fprintf(fp, COMP_TEST_HEADER);

	for(i = 0 ; i < phk->nkey ; i++) {
		pcell = hash_get(pht, phk->keys[i]);
		fprintf(fp, COMP_TEST_FORMAT, pcell->descr,
			pcell->c_macro, pcell->c_id, pcell->v_macro);
	}

	fprintf(fp, COMP_TEST_FOOTER);

	return(true);
}

/*
	detect compiler

	cpath : compiler path
	blog : buildlog file or /dev/null
	pcd : compiler data structure
	cinfo : compiler info structure

	return : boolean
*/

bool detect_compiler(char *cpath, char *blog, comp_data *pcd, comp_info *cinfo) {
	FILE		*tfp,
			*rpipe;
	bool		 failed = false;
	char		 cfgcmd[MAXPATHLEN],
			 ftmp[MAXPATHLEN],
			 pipebuf[TMP_BUF_LEN];
	int		 r;

	tfp = tmps_open(CC_TEST_FILE, "w", ftmp, sizeof(ftmp), strlen(CC_TFILE_EXT));
	if (tfp != NULL) {
		/* fill test file */
		gen_test_file(tfp, pcd);
		fclose(tfp);
	} else {
		errorf("cannot open test file ('%s').", ftmp);
		return(false);
	}
	/* build compiler command */
	snprintf(cfgcmd, sizeof(cfgcmd), CC_TEST_FORMAT,
		cpath, CC_TEST_BIN, ftmp, blog); /* XXX check */

	/* get result */
	r = system(cfgcmd);

	/* test file no longer needed */
	if (unlink(ftmp) == -1) {
		/* cannot remove temporary file */
		errorf("cannot remove %s : %s.", ftmp, strerror(errno));
	}

	if (r == 0) {
		rpipe = popen(CC_TEST_BIN, "r");
		if (rpipe != NULL) {
			if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
				errorf("cannot get compiler id.");
				failed = true;
			} else {
				/* get compiler id */
				cinfo->c_id = strdup(pipebuf);

				if (get_line(rpipe, pipebuf, sizeof(pipebuf)) == false) {
					errorf("cannot get compiler version.");
					failed = true;
				} else {
					cinfo->version = strdup(pipebuf);
				}
			}

			pclose(rpipe);

		} else {
			errorf("failed to get output from test binary.");
		}

		/* delete binary */
		if (unlink(CC_TEST_BIN) == -1) {
			errorf("cannot remove %s : %s.",
				CC_TEST_BIN, strerror(errno));
		}

		if (failed == true)
			return(false);
	} else {
		errorf("failed to build test binary : %s.", strerror(errno));
		return(false);
	}

	return(true);
}

