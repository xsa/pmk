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
	{"LIST_ARCH_EQUIV",	LIST_ARCH_EQUIV,	PRS_KW_CELL},
	{"LIST_X86_CPU_VENDOR",	LIST_X86_CPU_VENDOR,	PRS_KW_CELL},
	{"LIST_X86_CPU_MODEL",	LIST_X86_CPU_MODEL,	PRS_KW_CELL},
	{"LIST_X86_CPU_CLASS",	LIST_X86_CPU_CLASS,	PRS_KW_CELL}
};
int	nbkwc = sizeof(kw_pmkcpu) / sizeof(prskw);


arch_cell	arch_tab[] = {
	{"x86_32",	PMK_ARCH_X86_32},
	{"x86_64",	PMK_ARCH_X86_64},
	{"sparc32",	PMK_ARCH_SPARC},
	{"sparc64",	PMK_ARCH_SPARC64},
	{"ia_64",	PMK_ARCH_IA_64},
	{"ppc",		PMK_ARCH_PPC},
	{"alpha",	PMK_ARCH_ALPHA},
	{"m68k",	PMK_ARCH_M68K},
	{"parisc_64",	PMK_ARCH_PARISC},
	{"vax",		PMK_ARCH_VAX}
};
int	nbarch = sizeof(arch_tab) / sizeof(arch_cell);


/*
	XXX
*/

void *seek_key(prsdata *pdata, int token) {
	prscell		*pcell;

	pcell = pdata->tree->first;
	while (pcell != NULL) {
		if (pcell->token == token) {
			return(pcell->data);
		}

		pcell = pcell->next;
	}

	return(NULL);
}

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
	void		*ptmp;

	pht = (htable *) seek_key(pdata, LIST_ARCH_EQUIV);
	if (pht != NULL) {
		ptmp = hash_get(pht, uname_m); /* no check needed */
		if (ptmp != NULL)
			pstr = po_get_str(ptmp); /* no check needed */
	}

	if (pstr == NULL)
		pstr = PMK_ARCH_STR_UNKNOWN;

	return(strdup(pstr)); /* no check needed, NULL will be automatically returned */
}

/*
	XXX

	returns:  architecture identifier
*/

unsigned char arch_name_to_id(char *arch_name) {
	int		i;
	unsigned char	id = PMK_ARCH_UNKNOWN;

	for(i = 0 ; i < nbarch ; i++) {
		/* compare arch name with record key */
		if (strncmp(arch_name, arch_tab[i].name, strlen(arch_name)) == 0) {
			/* they are same, get the id */
			id = arch_tab[i].id;
			break;
		}
	}

	return(id);
}

/*
	XXX

	returns: hash table with values or NULL
*/

htable *arch_wrapper(prsdata *pdata, char *arch_name) {
	htable		*pht;
	unsigned char	 arch_id;
#ifdef ARCH_X86
	x86_cpu_cell	*pcell;
#endif

	arch_id = arch_name_to_id(arch_name);

	pht = hash_init(16); /* XXX hardcode, check */
	if (pht == NULL) {
		printf("DEBUG pht init failed !\n");
		return(NULL);
	}

	switch (arch_id) {
		case PMK_ARCH_X86_32 :
#ifdef ARCH_X86
		pcell = x86_cpu_cell_init();
		if (pcell == NULL)
			return(NULL);

		x86_get_cpuid_data(pcell); /* XXX check */
		pcell->stdvendor = x86_get_std_cpu_vendor(pdata, pcell->vendor);
		x86_set_cpu_data(pdata, pcell, pht); /* XXX check */

		x86_cpu_cell_destroy(pcell);
#else
		errorf("architecture mismatch."); /* XXX debug message ? */
		return(NULL);
#endif
			break;
	}

	return(pht);
}


/****************
 * x86 specific *
 ****************/

#ifdef ARCH_X86

/****
 functions
****/

/*
	XXX
*/

x86_cpu_cell *x86_cpu_cell_init(void) {
	x86_cpu_cell	*pcell;

	pcell = (x86_cpu_cell *) malloc(sizeof(x86_cpu_cell));
	if (pcell == NULL)
		return(NULL);

	pcell->vendor = NULL;
	pcell->stdvendor = NULL;
	pcell->cpuname = NULL;
	pcell->family = 0;
	pcell->model = 0;
	pcell->extfam = 0;
	pcell->extmod = 0;

	return(pcell);
}

/*
	XXX
*/

void x86_cpu_cell_destroy(x86_cpu_cell *pcell) {
	if (pcell->vendor != NULL)
		free(pcell->vendor);
	if (pcell->stdvendor != NULL)
		free(pcell->stdvendor);
	if (pcell->cpuname != NULL)
		free(pcell->cpuname);
	free(pcell);
}

/*
	get (pmk) vendor identifier based on data from cpuid

	pdata : parsung structure

	returns: XXX
*/

char *x86_get_std_cpu_vendor(prsdata *pdata, char *civendor) {
	char		*vendor = NULL;
	htable		*pht;
	prscell		*pcell;

	/* look for vendor list */
	pcell = pdata->tree->first;
	while ((pcell != NULL) && (vendor == NULL)) {
		if (pcell->token == LIST_X86_CPU_VENDOR) {
			/* got list ! */
			pht = pcell->data;
			/* look for standard vendor name */
			vendor = po_get_str(hash_get(pht, civendor));
		}
		pcell = pcell->next;
	}

	/* XXX useless ? */
	if (vendor == NULL)
		vendor = PMK_ARCH_STR_UNKNOWN;

	return(strdup(vendor)); /* no check needed, NULL will be automatically returned */
}


/*
	XXX
*/

bool x86_get_cpuid_data(x86_cpu_cell *cell) {
	uint32_t	 buffer[13];

	if (x86_check_cpuid_flag() == 0) {
		/* no cpuid flag => 386 or old 486 */
		cell->cpuid = false;
		cell->family = 3; /* fake family */
		return(true);
	}

	cell->cpuid = true;

	/* get the cpuid vendor */
	x86_exec_cpuid(0);

	cell->level = x86_cpu_reg_eax;

	buffer[0] = x86_cpu_reg_ebx;
	buffer[1] = x86_cpu_reg_edx;
	buffer[2] = x86_cpu_reg_ecx;
	buffer[3] = 0;	/* terminate string */

	cell->vendor = strdup((char *) buffer); /* XXX check */

	/* get the cpu type */
	x86_exec_cpuid(1);

	cell->family = (unsigned int) ((x86_cpu_reg_eax & X86_CPU_MASK_FAMILY) >> 8);
	if (cell->family == 15) {
		/* extended family */
		cell->extfam = (unsigned int) ((x86_cpu_reg_eax & X86_CPU_MASK_EXTFAM) >> 20);
	}

	cell->model = (unsigned int) ((x86_cpu_reg_eax & X86_CPU_MASK_MODEL) >> 4);


	x86_exec_cpuid(0x80000000);
	if (x86_cpu_reg_eax >= 0x80000002) {
		/* get the cpu name */
		x86_exec_cpuid(0x80000002);
		buffer[0] = x86_cpu_reg_eax;
		buffer[1] = x86_cpu_reg_ebx;
		buffer[2] = x86_cpu_reg_ecx;
		buffer[3] = x86_cpu_reg_edx;

		x86_exec_cpuid(0x80000003);
		buffer[4] = x86_cpu_reg_eax;
		buffer[5] = x86_cpu_reg_ebx;
		buffer[6] = x86_cpu_reg_ecx;
		buffer[7] = x86_cpu_reg_edx;

		x86_exec_cpuid(0x80000004);
		buffer[8] = x86_cpu_reg_eax;
		buffer[9] = x86_cpu_reg_ebx;
		buffer[10] = x86_cpu_reg_ecx;
		buffer[11] = x86_cpu_reg_edx;

		buffer[12] = 0;	/* terminate string */
		cell->cpuname = strdup((char *) buffer); /* XXX check */
	} else {
		cell->cpuname[0] = CHAR_EOS;
	}

	return(true);
}

/*
	XXX
*/

bool x86_set_cpu_data(prsdata *pdata, x86_cpu_cell *pcell, htable *pht) {
	char	 buffer[TMP_BUF_LEN],
		*pstr;
	htable	*phtbis;

	snprintf(buffer, sizeof(buffer), "%u", pcell->family); /* XXX check */
	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_FAMILY, buffer) == HASH_ADD_FAIL) {
		return(false);
	}

	snprintf(buffer, sizeof(buffer), "%u", pcell->model); /* XXX check */
	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_MODEL, buffer) == HASH_ADD_FAIL) {
		return(false);
	}

	snprintf(buffer, sizeof(buffer), "%u", pcell->extfam); /* XXX check */
	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_EXTFAM, buffer) == HASH_ADD_FAIL) {
		return(false);
	}

	snprintf(buffer, sizeof(buffer), "%u", pcell->extmod); /* XXX check */
	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_EXTMOD, buffer) == HASH_ADD_FAIL) {
		return(false);
	}

	if (pcell->cpuname == NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_NAME,
				pcell->cpuname) == HASH_ADD_FAIL) {
			return(false);
		}
	} /* else put unknown ? */

	if (pcell->vendor != NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_VENDOR,
				pcell->vendor) == HASH_ADD_FAIL) {
			return(false);
		}
	} /* else put unknown ? */

	if (pcell->stdvendor != NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_STD_VENDOR,
				pcell->stdvendor) == HASH_ADD_FAIL) {
			return(false);
		}
	} /* else put unknown ? */

	phtbis = (htable *) seek_key(pdata, LIST_X86_CPU_CLASS);
	if (phtbis != NULL) {
		if (pcell->family < 15) {
			snprintf(buffer, sizeof(buffer), X86_CPU_CLASS_FAMILY_FMT,
					pcell->stdvendor, pcell->family);
/*debugf("key = '%s'", buffer);*/
			pstr = po_get_str(hash_get(phtbis, buffer)); /* no check needed */
		} else {
			snprintf(buffer, sizeof(buffer), X86_CPU_CLASS_EXTFAM_FMT,
					pcell->stdvendor, pcell->extfam);
/*debugf("key = '%s'", buffer);*/
			pstr = po_get_str(hash_get(phtbis, buffer)); /* no check needed */
		}

		if (pstr == NULL) {
			/* not found, get default */
/*debugf("getting default");*/
			pstr = po_get_str(hash_get(phtbis, "DEFAULT")); /* XXX check */
		}

		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_CLASS, pstr) == HASH_ADD_FAIL) {
			return(false);
		}
	} else {
		printf("DEBUG key not found\n");
	}

	return(true);
}

#endif /* ARCH_X86 */

