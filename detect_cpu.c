/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
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
/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"
#include <sys/utsname.h>

#include "compat/pmk_string.h"
#include "detect_cpu.h"
#include "detect_cpu_asm.h"


/***************
 * common code *
 ***************/


/* config tools data file keyword */
prskw	kw_pmkcpu[] = {
	{"ADD_CPU_ARCH",	CPU_ARCH_ADD,		PRS_KW_CELL},
	{"ADD_X86_CPU_VENDOR",	CPU_X86_VENDOR_ADD,	PRS_KW_CELL},
	{"LIST_X86_CPU_VENDOR",	LIST_X86_CPU_VENDOR,	PRS_KW_CELL}
};
int	nbkwc = sizeof(kw_pmkcpu) / sizeof(prskw);



/*
	parse cpu data file

	fname : data file name

	returns : prsdata structure
*/

prsdata *parse_cpu_data(char *fname) {
	FILE		*fp;
	bool		 rval;
	prsdata		*pdata;

	/* initialize parsing structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		/*errorf("cannot intialize prsdata.");*/
		return(NULL);
	}

	fp = fopen(fname, "r");
	if (fp == NULL) {
		prsdata_destroy(pdata);
		/*errorf("cannot open '%s' : %s.",      */
		/*        fname, strerror(errno));*/
		return(NULL);
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fp, pdata, kw_pmkcpu, nbkwc);

	fclose(fp);

	if (rval == false) {
		prsdata_destroy(pdata);
		pdata = NULL;
	}

	return(pdata);
}


/*
	check the cpu architecture 

	uname_m : uname machine string 

	returns:  cpu architecture string or NULL
*/

char *check_cpu_arch(char *uname_m, prsdata *pdata) {
	char		*pstr = NULL;
	dynary		*da;
	htable		*pht;
	int		 i,
			 n;
	prscell		*pcell;

	pcell = pdata->tree->first;
	while ((pcell != NULL) && (pstr == NULL)) {
		if (pcell->token == CPU_ARCH_ADD) {
			pht = pcell->data;

			da = po_get_list(hash_get(pht, "LIST")); /* XXX , hardcode!, check */
			n = da_usize(da);
			for (i=0 ; i<n ; i++) {
				/*
				 * could handle regex later
				 *

				if da_idx(da, i) ~ uname_m !XXX!
					pstr = po_get_str(hash_get(pht, "NAME"));
					return(pstr);
				*/
				if (strncmp(uname_m, da_idx(da, i), sizeof(uname_m)) == 0) {
					pstr = po_get_str(hash_get(pht, "NAME"));
				}
			}
		}

		pcell = pcell->next;
	}

	if (pstr == NULL)
		pstr = "unknown";

	return(pstr); /* no check needed, NULL will be automatically returned */
}


/****************
 * x86 specific *
 ****************/

/****
 defines
****/

#define MASK_X86_CPU_EXTFAM	0x0ff00000
#define MASK_X86_CPU_EXTMOD	0x000f0000
#define MASK_X86_CPU_TYPE	0x0000f000
#define MASK_X86_CPU_FAMILY	0x00000f00
#define MASK_X86_CPU_MODEL	0x000000f0


/****
 functions
****/

/*
	get (pmk) vendor identifier based on data from cpuid

	pdata : parsung structure

	returns: XXX
*/

char *get_x86_std_cpu_vendor(prsdata *pdata) {
#ifdef ARCH_X86
	char		*pstr,
			*vendor = NULL;
	htable		*pht;
	prscell		*pcell;

	/* get the cpuid vendor */
	pstr = get_x86_cpu_vendor();

	/* look for vendor list */
	pcell = pdata->tree->first;
	while ((pcell != NULL) && (vendor == NULL)) {
		if (pcell->token == LIST_X86_CPU_VENDOR) {
			/* got list ! */
			pht = pcell->data;
			/* look for standard vendor name */
			vendor = po_get_str(hash_get(pht, pstr));
		}
		pcell = pcell->next;
	}

	/* XXX useless ? */
	if (vendor == NULL)
		vendor = "unknown";

	return(vendor); /* no check needed, NULL will be automatically returned */
#else
	/* unused*/
	pdata = NULL;
	return(pdata);
#endif
}

