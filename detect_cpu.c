/* $Id$ */

/*
 * Copyright (c) 2004-2006 Damien Couderc
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
/* include it first as if it was <sys/types.h> - this will avoid errors */
#include "compat/pmk_sys_types.h"
#include <sys/utsname.h>
#include <errno.h>

#include "compat/pmk_string.h"
#include "compat/pmk_stdio.h"
#include "common.h"
#include "detect_cpu.h"
#include "detect_cpu_asm.h"


/***************
 * common code *
 ***********************************************************************/


/* config tools data file keyword */
prskw	kw_pmkcpu[] = {
	{"LIST_ARCH_EQUIV",			LIST_ARCH_EQUIV,		PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"LIST_X86_CPU_VENDOR",		LIST_X86_CPU_VENDOR,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"LIST_X86_CPU_MODEL",		LIST_X86_CPU_MODEL,		PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"LIST_X86_CPU_CLASS",		LIST_X86_CPU_CLASS,		PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"LIST_ALPHA_CPU_CLASS",	LIST_ALPHA_CPU_CLASS,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL},
	{"LIST_IA64_CPU_CLASS",		LIST_IA64_CPU_CLASS,	PRS_KW_CELL,	PRS_TOK_NULL,	NULL}
};
size_t	nbkwc = sizeof(kw_pmkcpu) / sizeof(prskw);


arch_cell	arch_tab[] = {
	{"x86_32",		PMK_ARCH_X86_32},
	{"x86_64",		PMK_ARCH_X86_64},
	{"sparc32",		PMK_ARCH_SPARC},
	{"sparc64",		PMK_ARCH_SPARC64},
	{"ia_64",		PMK_ARCH_IA_64},
	{"ppc",			PMK_ARCH_PPC},
	{"alpha",		PMK_ARCH_ALPHA},
	{"m68k",		PMK_ARCH_M68K},
	{"parisc_64",	PMK_ARCH_PARISC},
	{"vax",			PMK_ARCH_VAX}
};
size_t	nbarch = sizeof(arch_tab) / sizeof(arch_cell);


/**************
 * seek_key() *
 ***********************************************************************
 DESCR
	seek parse data key

 IN
	pdata : parse data structure
	token : key token to find

 OUT
	key data or NULL
 ***********************************************************************/

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


/********************
 * parse_cpu_data() *
 ***********************************************************************
 DESCR
	parse cpu data file

 IN
	fname : data file name

 OUT
	prsdata structure
 ***********************************************************************/

prsdata *parse_cpu_data(char *fname) {
	FILE		*fp;
	bool		 rval;
	prsdata		*pdata;

	/* initialize parsing structure */
	pdata = prsdata_init();
	if (pdata == NULL) {
		errorf("cannot intialize parsing data structure.");
		return(NULL);
	}

	fp = fopen(fname, "r");
	if (fp == NULL) {
		prsdata_destroy(pdata);
		errorf("cannot open '%s' : %s.",
				fname, strerror(errno));
		return(NULL);
	}

	/* parse data file and fill prsdata strucutre */
	rval = parse_pmkfile(fp, pdata, kw_pmkcpu, nbkwc);

	fclose(fp);

	if (rval == false) {
		/* parsing failed, pdata is useless, cleaning */
		prsdata_destroy(pdata);
		return(NULL);
	}

	return(pdata);
}


/********************
 * check_cpu_arch() *
 ***********************************************************************
 DESCR
	check the cpu architecture

 IN
	uname_m : uname machine string

 OUT
	cpu architecture string
 ***********************************************************************/

char *check_cpu_arch(char *uname_m, prsdata *pdata) {
	char		*pstr = NULL;
	htable_t		*pht;
	void		*ptmp;

	pht = (htable_t *) seek_key(pdata, LIST_ARCH_EQUIV);
	if (pht != NULL) {
		ptmp = hash_get(pht, uname_m); /* no check needed */
		if (ptmp != NULL)
			pstr = po_get_str(ptmp); /* no check needed */
	}

	if (pstr == NULL)
		pstr = PMK_ARCH_STR_UNKNOWN;

	return(strdup(pstr)); /* no check needed, NULL will be automatically returned */
}


/*********************
 * arch_name_to_id() *
 ***********************************************************************
 DESCR
	convert arch name to id

 IN
	arch_name : architecture string

 OUT
	architecture identifier
 ***********************************************************************/

unsigned char arch_name_to_id(char *arch_name) {
	unsigned int	i;
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


/******************
 * arch_wrapper() *
 ***********************************************************************
 DESCR
	architecture wrapper

 IN
	pdata : parsing data structure
	arch_name : architecture name

 OUT
	hash table with values or NULL
 ***********************************************************************/

htable_t *arch_wrapper(prsdata *pdata, char *arch_name) {
	htable_t			*pht;
	unsigned char	 arch_id;

#if defined(ARCH_X86_32) || defined(ARCH_X86_64)
	x86_cpu_cell	*pcell;
#endif /* ARCH_X86_32 || ARCH_X86_64 */

	arch_id = arch_name_to_id(arch_name);

	pht = hash_create_simple(ARCH_NB_MAX);
	if (pht == NULL) {
		printf("DEBUG pht init failed !\n");
		return(NULL);
	}

	switch (arch_id) {
		case PMK_ARCH_X86_32 :
		case PMK_ARCH_X86_64 :
#if defined(ARCH_X86_32) || defined(ARCH_X86_64)
			pcell = x86_cpu_cell_init();
			if (pcell == NULL)
				return(NULL);

			if (x86_get_cpuid_data(pcell) == false) {
				errorf("unable to get x86 cpuid data.");
				return(NULL);
			}

			pcell->stdvendor = x86_get_std_cpu_vendor(pdata, pcell->vendor);
			if (x86_set_cpu_data(pdata, pcell, pht) == false) {
				errorf("failed to record cpu data.");
				return(NULL);
			}

			x86_cpu_cell_destroy(pcell);
#else /* ARCH_X86_32 || ARCH_X86_64 */
			errorf("architecture mismatch for '%s'.", arch_name);
			return(NULL);
#endif /* ARCH_X86_32 || ARCH_X86_64 */
			break;

		case PMK_ARCH_ALPHA :
#if defined(ARCH_ALPHA)
			if (alpha_set_cpu_data(pdata, pht) == false) {
				errorf("failed to record cpu data.");
				return(NULL);
			}
#else /* ARCH_ALPHA */
			errorf("architecture mismatch for '%s'.", arch_name);
			return(NULL);
#endif /* ARCH_ALPHA */
			break;

		case PMK_ARCH_IA_64 :
#if defined(ARCH_IA64)
			if (ia64_get_cpuid_data(pdata, pht) == false) {
				errorf("failed to record cpu data.");
				return(NULL);
			}
#else /* ARCH_IA64 */
			errorf("architecture mismatch for '%s'.", arch_name);
			return(NULL);
#endif /* ARCH_IA64 */

			break;
	}

	return(pht);
}


/****************
 * x86 specific *
 ***********************************************************************/

#if defined(ARCH_X86_32) || defined(ARCH_X86_64)

x86_cpu_feature	x86_cpu_feat_reg1[] = {
	{X86_CPU_MASK_FEAT_FPU,		"FPU"},
	{X86_CPU_MASK_FEAT_VME,		"VME"},
	{X86_CPU_MASK_FEAT_DE,		"DE"},
	{X86_CPU_MASK_FEAT_PSE,		"PSE"},
	{X86_CPU_MASK_FEAT_TSC,		"TSC"},
	{X86_CPU_MASK_FEAT_MSR,		"MSR"},
	{X86_CPU_MASK_FEAT_PAE,		"PAE"},
	{X86_CPU_MASK_FEAT_MCE,		"MCE"},
	{X86_CPU_MASK_FEAT_CX8,		"CX8"},
	{X86_CPU_MASK_FEAT_APIC,	"APIC"},
	{X86_CPU_MASK_FEAT_SEP,		"SEP"},
	{X86_CPU_MASK_FEAT_MTRR,	"MTRR"},
	{X86_CPU_MASK_FEAT_PGE,		"PGE"},
	{X86_CPU_MASK_FEAT_MCA,		"MCA"},
	{X86_CPU_MASK_FEAT_CMOV,	"CMOV"},
	{X86_CPU_MASK_FEAT_PAT,		"PAT"},
	{X86_CPU_MASK_FEAT_PSE36,	"PSE36"},
	{X86_CPU_MASK_FEAT_PSN,		"PSN"},
	{X86_CPU_MASK_FEAT_CLFL,	"CLFL"},
	{X86_CPU_MASK_FEAT_DTES,	"DTES"},
	{X86_CPU_MASK_FEAT_ACPI,	"ACPI"},
	{X86_CPU_MASK_FEAT_MMX,		"MMX"},
	{X86_CPU_MASK_FEAT_FXR,		"FXR"},
	{X86_CPU_MASK_FEAT_SSE,		"SSE"},
	{X86_CPU_MASK_FEAT_SSE2,	"SSE2"},
	{X86_CPU_MASK_FEAT_SS,		"SS"},
	{X86_CPU_MASK_FEAT_HTT,		"HTT"},
	{X86_CPU_MASK_FEAT_TM1,		"TM1"},
	{X86_CPU_MASK_FEAT_IA64,	"IA64"},
	{X86_CPU_MASK_FEAT_PBE,		"PBE"}
};
size_t nb_feat_reg1 = sizeof(x86_cpu_feat_reg1) / sizeof(x86_cpu_feature);

x86_cpu_feature	x86_cpu_feat_reg2[] = {
	{X86_CPU_MASK_FEAT_FPU,		"FPU"},
	{X86_CPU_MASK_FEAT_MON,		"MON"},
	{X86_CPU_MASK_FEAT_DSCPL,	"DSCPL"},
	{X86_CPU_MASK_FEAT_EST,		"EST"},
	{X86_CPU_MASK_FEAT_TM2,		"TM2"},
	{X86_CPU_MASK_FEAT_CID,		"CID"},
	{X86_CPU_MASK_FEAT_CX16,	"CX16"},
	{X86_CPU_MASK_FEAT_ETPRD,	"ETPRD"}
};
size_t nb_feat_reg2 = sizeof(x86_cpu_feat_reg2) / sizeof(x86_cpu_feature);

x86_cpu_feature	x86_cpu_feat_extreg[] = {
	{X86_CPU_MASK_FEAT_LM,		"LM"},
	{X86_CPU_MASK_FEAT_EXT3DN,	"EXT3DN"},
	{X86_CPU_MASK_FEAT_3DNOW,	"3DNOW"}
};
size_t nb_feat_extreg = sizeof(x86_cpu_feat_extreg) / sizeof(x86_cpu_feature);


/****
 functions
****/

/***********************
 * x86_cpu_cell_init() *
 ***********************************************************************
 DESCR
	initialise x86 cpu cell

 IN
	NONE

 OUT
	cpu cell or NULL
 ***********************************************************************/

x86_cpu_cell *x86_cpu_cell_init(void) {
	x86_cpu_cell	*pcell;

	pcell = (x86_cpu_cell *) malloc(sizeof(x86_cpu_cell));
	if (pcell == NULL)
		return(NULL);

	pcell->vendor = NULL;
	pcell->stdvendor = NULL;
	pcell->cpuname = NULL;
	pcell->features = NULL;
	pcell->family = 0;
	pcell->model = 0;
	pcell->extfam = 0;
	pcell->extmod = 0;

	return(pcell);
}


/**************************
 * x86_cpu_cell_destroy() *
 ***********************************************************************
 DESCR
	free x86 cpu cell structure

 IN
	pcell :	structure to free

 OUT
	NONE
 ***********************************************************************/

void x86_cpu_cell_destroy(x86_cpu_cell *pcell) {
	if (pcell->vendor != NULL)
		free(pcell->vendor);
	if (pcell->stdvendor != NULL)
		free(pcell->stdvendor);
	if (pcell->cpuname != NULL)
		free(pcell->cpuname);
	free(pcell);
}


/****************************
 * x86_get_std_cpu_vendor() *
 ***********************************************************************
 DESCR
	get (pmk) vendor identifier based on data from cpuid

 IN
	pdata :	parsing structure

 OUT
	cpu vendor string
 ***********************************************************************/

char *x86_get_std_cpu_vendor(prsdata *pdata, char *civendor) {
	char		*vendor = NULL;
	htable_t		*pht;
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

	/* in case we don't find anything */
	if (vendor == NULL)
		vendor = PMK_ARCH_STR_UNKNOWN;

	return(strdup(vendor)); /* no check needed, NULL will be automatically returned */
}


/************************
 * x86_get_cpuid_data() *
 ***********************************************************************
 DESCR
	gather x86 cpuid data

 IN
	cell :	x86 cpu cell

 OUT
	boolean
 ***********************************************************************/

bool x86_get_cpuid_data(x86_cpu_cell *cell) {
	char			 feat_str[TMP_BUF_LEN] = "";
	unsigned int	 i;
	uint32_t		 buffer[13],
					 extlevel;

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

	cell->vendor = strdup((char *) buffer);
	if (cell->vendor == NULL) {
		return(false);
	}

	/* get the cpu type */
	x86_exec_cpuid(1);

	cell->family = (unsigned int) ((x86_cpu_reg_eax & X86_CPU_MASK_FAMILY) >> 8);
	if (cell->family == 15) {
		/* extended family */
		cell->extfam = (unsigned int) ((x86_cpu_reg_eax & X86_CPU_MASK_EXTFAM) >> 20);
	}

	cell->model = (unsigned int) ((x86_cpu_reg_eax & X86_CPU_MASK_MODEL) >> 4);

	/* processing feature register 1 */
	for (i = 0 ; i < nb_feat_reg1 ; i++) {
		if ((x86_cpu_reg_edx & x86_cpu_feat_reg1[i].mask) != 0) {
			strlcat(feat_str, x86_cpu_feat_reg1[i].descr, sizeof(feat_str)); /* no check */
			if (strlcat_b(feat_str, " ", sizeof(feat_str)) == false) {
				return(false);
			}
		}
	}
	/* processing feature register 2 */
	for (i = 0 ; i < nb_feat_reg2 ; i++) {
		if ((x86_cpu_reg_edx & x86_cpu_feat_reg2[i].mask) != 0) {
			strlcat(feat_str, x86_cpu_feat_reg2[i].descr, sizeof(feat_str)); /* no check */
			if (strlcat_b(feat_str, " ", sizeof(feat_str)) == false) {
				return(false);
			}
		}
	}

	/* check extended cpu level */
	x86_exec_cpuid(0x80000000);
	extlevel = x86_cpu_reg_eax;

	if (extlevel >= 0x80000001) {
		/* get extended cpu features */
		x86_exec_cpuid(0x80000001);

		/* processing extended feature register */
		for (i = 0 ; i < nb_feat_extreg ; i++) {
			if ((x86_cpu_reg_edx & x86_cpu_feat_extreg[i].mask) != 0) {
				strlcat(feat_str, x86_cpu_feat_extreg[i].descr, sizeof(feat_str)); /* no check */
				if (strlcat_b(feat_str, " ", sizeof(feat_str)) == false) {
					return(false);
				}
			}
		}
	}

	/* save feature string */
	cell->features = strdup(feat_str);

	if (extlevel >= 0x80000002) {
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
		cell->cpuname = strdup((char *) buffer);
		if (cell->cpuname == NULL) {
			return(false);
		}
	} else {
		cell->cpuname = NULL;
	}

	return(true);
}


/**********************
 * x86_set_cpu_data() *
 ***********************************************************************
 DESCR
	record cpu data in hash table

 IN
	pdata :	parsing data
	pcell :	x86 cpu data cell
	pht :	storage hash table

 OUT
	boolean
 ***********************************************************************/

bool x86_set_cpu_data(prsdata *pdata, x86_cpu_cell *pcell, htable_t *pht) {
	char	 buffer[TMP_BUF_LEN],
			*pstr;
	htable_t	*phtbis;

	if (snprintf_b(buffer, sizeof(buffer), "%u", pcell->family) == false) {
		return(false);
	}

	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_FAMILY, buffer) == false) {
		return(false);
	}

	if (snprintf_b(buffer, sizeof(buffer), "%u", pcell->model) == false) {
		return(false);
	}

	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_MODEL, buffer) == false) {
		return(false);
	}

	if (snprintf_b(buffer, sizeof(buffer), "%u", pcell->extfam) == false) {
		return(false);
	}

	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_EXTFAM, buffer) == false) {
		return(false);
	}

	if (snprintf_b(buffer, sizeof(buffer), "%u", pcell->extmod) == false) {
		return(false);
	}

	if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_EXTMOD, buffer) == false) {
		return(false);
	}

	if (pcell->cpuname != NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_NAME,
				pcell->cpuname) == false) {
			return(false);
		}
	} /* else put unknown ? */

	if (pcell->vendor != NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_VENDOR,
				pcell->vendor) == false) {
			return(false);
		}
	} /* else put unknown ? */

	if (pcell->stdvendor != NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_STD_VENDOR,
				pcell->stdvendor) == false) {
			return(false);
		}
	} /* else put unknown ? */

	if (pcell->features != NULL) {
		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_FEATURES,
				pcell->features) == false) {
			return(false);
		}
	} /* else put empty string ? */

	phtbis = (htable_t *) seek_key(pdata, LIST_X86_CPU_CLASS);
	if (phtbis != NULL) {
		if (pcell->family < 15) {
			snprintf_b(buffer, sizeof(buffer), /* no check needed */
					X86_CPU_CLASS_FAMILY_FMT,
					pcell->stdvendor, pcell->family);
			pstr = po_get_str(hash_get(phtbis, buffer)); /* no check needed */
		} else {
			snprintf_b(buffer, sizeof(buffer), /* no check needed */
					X86_CPU_CLASS_EXTFAM_FMT,
					pcell->stdvendor, pcell->extfam);
			pstr = po_get_str(hash_get(phtbis, buffer)); /* no check needed */
		}

		if (pstr == NULL) {
			/* not found, get default */
			pstr = po_get_str(hash_get(phtbis, "DEFAULT"));
			if (pstr == NULL) {
				errorf("failed to get default value for cpu family (%s).", buffer);
				return(false);
			}
		}

		if (hash_update_dup(pht, PMKCONF_HW_X86_CPU_CLASS, pstr) == false) {
			return(false);
		}
	} else {
		errorf("failed to find '%s' key\n", LIST_X86_CPU_CLASS);
		return(false);
	}

	return(true);
}

#endif /* ARCH_X86_32 || ARCH_X86_64 */


/******************
 * alpha specific *
 ***********************************************************************/

#if defined(ARCH_ALPHA)

alpha_cpu_feature	alpha_cpu_feat[] = {
	{ALPHA_CPU_MASK_FEAT_BWX,	"BWX"},
	{ALPHA_CPU_MASK_FEAT_FIX,	"FIX"},
	{ALPHA_CPU_MASK_FEAT_CIX,	"CIX"},
	{ALPHA_CPU_MASK_FEAT_MVI,	"MVI"},
	{ALPHA_CPU_MASK_FEAT_PAT,	"PAT"},
	{ALPHA_CPU_MASK_FEAT_PMI,	"PMI"},
};
int nb_feat = sizeof(alpha_cpu_feat) / sizeof(alpha_cpu_feature);


/************************
 * alpha_set_cpu_data() *
 ***********************************************************************
 DESCR
	set alpha cpu data

 IN
	pdata:	parsing data structure
	pht:	data storage hash table

 OUT
	boolean
 ***********************************************************************/

bool alpha_set_cpu_data(prsdata *pdata, htable_t *pht) {
	char			 buffer[16],
					 feat_str[TMP_BUF_LEN] = "",
					*pstr;
	htable_t			*phtbis;
	unsigned int	 i;
	uint64_t		 implver,
					 amask;

/*debugf("alpha_set_cpu_data() : end");*/
	phtbis = (htable_t *) seek_key(pdata, LIST_ALPHA_CPU_CLASS);
	if (phtbis != NULL) {
		/* get cpu class */
		implver = alpha_exec_implver();
/*debugf("alpha_set_cpu_data() : implver = '%u'", implver);*/

		if (snprintf_b(buffer, sizeof(buffer), ALPHA_CPU_CLASS_FMT, implver) == false) {
			return(false);
		}
/*debugf("alpha_set_cpu_data() : buffer = '%s'", buffer);*/
		pstr = po_get_str(hash_get(phtbis, buffer));
		if (pstr == NULL) {
			pstr = ALPHA_CPU_UNKNOWN;
		}

		if (hash_update_dup(pht, PMKCONF_HW_ALPHA_CPU_CLASS, pstr) == false) {
			return(false);
		}

		/* get cpu features */
		amask = alpha_exec_amask();

		/* processing feature mask */
		for (i = 0 ; i < nb_feat ; i++) {
			if ((amask & alpha_cpu_feat[i].mask) != 0) {
				strlcat(feat_str, alpha_cpu_feat[i].descr, sizeof(feat_str)); /* no check */
				if (strlcat_b(feat_str, " ", sizeof(feat_str)) == false) {
					return(false);
				}
			}
		}

		/* save feature string */
		if (hash_update_dup(pht, PMKCONF_HW_ALPHA_CPU_FEATURES, feat_str) == false) {
			return(false);
		}

/*debugf("alpha_set_cpu_data() : amask = %x", amask);*/
	}


/*debugf("alpha_set_cpu_data() : end");*/
	return(true);
}

#endif /* ARCH_ALPHA */


/*****************
 * ia64 specific *
 ***********************************************************************/

#if defined(ARCH_IA64)

ia64_cpu_feature ia64_cpu_feat[] = {
	{IA64_CPU_MASK_FEAT_LB,	"LB"},
	{IA64_CPU_MASK_FEAT_SD,	"SD"},
	{IA64_CPU_MASK_FEAT_AO,	"AO"}
};
size_t nb_feat = sizeof(ia64_cpu_feat) / sizeof(ia64_cpu_feature);


/****
 functions
****/

/*************************
 * ia64_get_cpuid_data() *
 ***********************************************************************
 DESCR
	gather and save ia64 cpu data

 IN
	pdata:	parsing data structure
	pht:	storage hash table

 OUT
	boolean
 ***********************************************************************/

bool ia64_get_cpuid_data(prsdata *pdata, htable_t *pht) {
	char			 buffer[TMP_BUF_LEN],
					*pstr;
	htable_t			*phtbis;
	uint64_t		 regbuf[3],
					 level,
					 rslt;
	unsigned int	 i;

	regbuf[0] = ia64_get_cpuid_register(0);
	regbuf[1] = ia64_get_cpuid_register(1);
	regbuf[2] = 0;
	if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_VENDOR, regbuf) == false)
		return(false);
/*debugf("cpuid register 0 = %x", regbuf[0]);*/
/*debugf("cpuid register 1 = %x", regbuf[1]);*/
/*debugf("vendor = '%s'", regbuf);           */

	regbuf[0] = ia64_get_cpuid_register(3);
/*debugf("cpuid register 3 = %x", regbuf[0]);*/

	level = regbuf[0] & IA64_CPU_MASK_LEVEL;
/*debugf("level = %x", level);*/

	rslt = (regbuf[0] & IA64_CPU_MASK_REVISION) >> 8;
	if (snprintf_b(buffer, sizeof(buffer), "%u", rslt) == false) {
		return(false);
	}
	if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_REVISION, buffer) == false) {
		return(false);
	}
/*debugf("revision = %x", rslt);*/

	rslt = (regbuf[0] & IA64_CPU_MASK_MODEL) >> 16;
	if (snprintf_b(buffer, sizeof(buffer), "%u", rslt) == false) {
		return(false);
	}
	if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_MODEL, buffer) == false) {
		return(false);
	}
/*debugf("model = %x", rslt);*/

	rslt = (regbuf[0] & IA64_CPU_MASK_FAMILY) >> 24;
	if (snprintf_b(buffer, sizeof(buffer), "%u", rslt) == false) {
		return(false);
	}
	if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_FAMILY, buffer) == false) {
		return(false);
	}
/*debugf("family = %x", rslt);*/

	phtbis = (htable_t *) seek_key(pdata, LIST_IA64_CPU_CLASS);
	if (phtbis != NULL) {
		snprintf_b(buffer, sizeof(buffer), /* no check needed */
				IA64_CPU_CLASS_FAMILY_FMT, rslt);
/*debugf("buffer = '%s'", buffer);*/

		pstr = po_get_str(hash_get(phtbis, buffer)); /* no check needed */
		if (pstr == NULL) {
/*debugf("getting default");*/
			/* not found, get default */
			pstr = po_get_str(hash_get(phtbis, "DEFAULT"));
			if (pstr == NULL) {
				errorf("failed to get default value for cpu class family.");
				return(false);
			}
		}
/*debugf("pstr = '%s'", pstr);*/

		if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_CLASS, pstr) == false) {
			/* err msg ? */
			return(false);
		}
	} else {
		errorf("failed to find '%s' key\n", LIST_IA64_CPU_CLASS);
		return(false);
	}


	rslt = (regbuf[0] & IA64_CPU_MASK_ARCHREV) >> 32;
	if (snprintf_b(buffer, sizeof(buffer), "%u", rslt) == false) {
		return(false);
	}
	if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_ARCHREV, buffer) == false) {
		return(false);
	}
	/*printf("archrev = %x\n", rslt);*/

	if (level >= 0x04) {
		/* getting feature register */
		rslt = ia64_get_cpuid_register(4);
		/*printf("cpuid register 4 = %x\n", regbuf[0]);*/

		/* processing feature mask */
		buffer[0] = CHAR_EOS;
		for (i = 0 ; i < nb_feat ; i++) {
			if ((rslt & ia64_cpu_feat[i].mask) != 0) {
				strlcat(buffer, ia64_cpu_feat[i].descr,
						sizeof(buffer)); /* no check */
				if (strlcat_b(buffer, " ", sizeof(buffer)) == false)
					return(false); /* err msg ? */
			}
		}
		if (hash_update_dup(pht, PMKCONF_HW_IA64_CPU_FEATURES, buffer) == false) {
			/* err msg ? */
			return(false);
		}
	}

	return(true);
}

#endif /* ARCH_IA64 */

