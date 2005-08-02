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


#ifndef _DETECT_CPU_H_
#define _DETECT_CPU_H_

#include <inttypes.h>
#include "cpu_arch_def.h"
#include "hash.h"
#include "parse.h"


/***************
 * common code *
 ***********************************************************************/


#define LIST_ARCH_EQUIV		1
#define LIST_X86_CPU_VENDOR	2
#define LIST_X86_CPU_MODEL	3
#define LIST_X86_CPU_CLASS	4
#define LIST_ALPHA_CPU_CLASS	5
#define LIST_IA64_CPU_CLASS	6

#define	PMK_ARCH_UNKNOWN	0
#define	PMK_ARCH_X86_32		1
#define	PMK_ARCH_X86_64		2
#define	PMK_ARCH_SPARC		3
#define	PMK_ARCH_SPARC64	4
#define	PMK_ARCH_IA_64		5
#define	PMK_ARCH_PPC		6
#define	PMK_ARCH_ALPHA		7
#define	PMK_ARCH_M68K		8
#define	PMK_ARCH_PARISC		9
#define	PMK_ARCH_VAX		10

#define	PMK_ARCH_STR_UNKNOWN	"unknown"

#define ARCH_NB_MAX		32


#define PMKCONF_HW_PPC_CPU_ALTIVEC	"HW_PPC_CPU_ALTIVEC"	/* powerpc altivec feature */


typedef struct {
	char		*name;
	unsigned char	 id;
} arch_cell;


void		*seek_key(prsdata *, int);
prsdata		*parse_cpu_data(char *);
char		*check_cpu_arch(char *, prsdata *);
htable		*arch_wrapper(prsdata *, char *);
unsigned char	 arch_name_to_id(char *);

/****************
 * x86 specific *
 ***********************************************************************/

#if defined(ARCH_X86_32) || defined(ARCH_X86_64)

/* register masks */
#define X86_CPU_MASK_EXTFAM	0x0ff00000
#define X86_CPU_MASK_EXTMOD	0x000f0000
#define X86_CPU_MASK_TYPE	0x0000f000
#define X86_CPU_MASK_FAMILY	0x00000f00
#define X86_CPU_MASK_MODEL	0x000000f0


/* feature register 1 (edx) */
#define X86_CPU_MASK_FEAT_FPU	0x00000001	/* bit  0 */
#define X86_CPU_MASK_FEAT_VME	0x00000002	/* bit  1 */
#define X86_CPU_MASK_FEAT_DE	0x00000004	/* bit  2 */
#define X86_CPU_MASK_FEAT_PSE	0x00000008	/* bit  3 */
#define X86_CPU_MASK_FEAT_TSC	0x00000010	/* bit  4 */
#define X86_CPU_MASK_FEAT_MSR	0x00000020	/* bit  5 */
#define X86_CPU_MASK_FEAT_PAE	0x00000040	/* bit  6 */
#define X86_CPU_MASK_FEAT_MCE	0x00000080	/* bit  7 */
#define X86_CPU_MASK_FEAT_CX8	0x00000100	/* bit  8 */
#define X86_CPU_MASK_FEAT_APIC	0x00000200	/* bit  9 */
/*		RESERVED			   bit 10 */
#define X86_CPU_MASK_FEAT_SEP	0x00000800	/* bit 11 */
#define X86_CPU_MASK_FEAT_MTRR	0x00001000	/* bit 12 */
#define X86_CPU_MASK_FEAT_PGE	0x00002000	/* bit 13 */
#define X86_CPU_MASK_FEAT_MCA	0x00004000	/* bit 14 */
#define X86_CPU_MASK_FEAT_CMOV	0x00008000	/* bit 15 */
#define X86_CPU_MASK_FEAT_PAT	0x00010000	/* bit 16 */
#define X86_CPU_MASK_FEAT_PSE36	0x00020000	/* bit 17 */
#define X86_CPU_MASK_FEAT_PSN	0x00040000	/* bit 18 */
#define X86_CPU_MASK_FEAT_CLFL	0x00080000	/* bit 19 */
/*		RESERVED			   bit 20 */
#define X86_CPU_MASK_FEAT_DTES	0x00200000	/* bit 21 */
#define X86_CPU_MASK_FEAT_ACPI	0x00400000	/* bit 22 */
#define X86_CPU_MASK_FEAT_MMX	0x00800000	/* bit 23 */
#define X86_CPU_MASK_FEAT_FXR	0x01000000	/* bit 24 */
#define X86_CPU_MASK_FEAT_SSE	0x02000000	/* bit 25 */
#define X86_CPU_MASK_FEAT_SSE2	0x04000000	/* bit 26 */
#define X86_CPU_MASK_FEAT_SS	0x00000000	/* bit 27 */
#define X86_CPU_MASK_FEAT_HTT	0x10000000	/* bit 28 */
#define X86_CPU_MASK_FEAT_TM1	0x20000000	/* bit 29 */
#define X86_CPU_MASK_FEAT_IA64	0x40000000	/* bit 30 */
#define X86_CPU_MASK_FEAT_PBE	0x80000000	/* bit 31 */

/* feature register 2 (ecx) */
#define X86_CPU_MASK_FEAT_FPU	0x00000001	/* bit  0 */
/*		RESERVED			   bit  1 */
/*		RESERVED			   bit  2 */
#define X86_CPU_MASK_FEAT_MON	0x00000008	/* bit  3 */
#define X86_CPU_MASK_FEAT_DSCPL	0x00000010	/* bit  4 */
/*		RESERVED			   bit  5 */
/*		RESERVED			   bit  6 */
#define X86_CPU_MASK_FEAT_EST	0x00000080	/* bit  7 */
#define X86_CPU_MASK_FEAT_TM2	0x00000100	/* bit  8 */
/*		RESERVED			   bit  9 */
#define X86_CPU_MASK_FEAT_CID	0x00000400	/* bit 10 */
/*		RESERVED			   bit 11 */
/*		RESERVED			   bit 12 */
#define X86_CPU_MASK_FEAT_CX16	0x00002000	/* bit 13 */
#define X86_CPU_MASK_FEAT_ETPRD	0x00004000	/* bit 14 */

/* extended feature register (edx) */
#define X86_CPU_MASK_FEAT_LM		0x20000000	/* bit 29 */
#define X86_CPU_MASK_FEAT_EXT3DN	0x40000000	/* bit 30 */
#define X86_CPU_MASK_FEAT_3DNOW		0x80000000	/* bit 31 */


#define PMKCONF_HW_X86_CPU_FAMILY	"HW_X86_CPU_FAMILY"	/* family */
#define PMKCONF_HW_X86_CPU_MODEL	"HW_X86_CPU_MODEL"	/* model */
#define PMKCONF_HW_X86_CPU_EXTFAM	"HW_X86_CPU_EXTFAM"	/* extended family */
#define PMKCONF_HW_X86_CPU_EXTMOD	"HW_X86_CPU_EXTMOD"	/* extended model */
#define PMKCONF_HW_X86_CPU_NAME		"HW_X86_CPU_NAME"	/* name optionaly provided by cpuid */
#define PMKCONF_HW_X86_CPU_FEATURES	"HW_X86_CPU_FEATURES"	/* MMX,SSE,SSE2,HTT, etc ... */
#define PMKCONF_HW_X86_CPU_VENDOR	"HW_X86_CPU_VENDOR"	/* vendor name from cpuid */
#define PMKCONF_HW_X86_CPU_STD_VENDOR	"HW_X86_CPU_STD_VENDOR"	/* "standard" vendor name : INTEL, AMD, etc ... */
#define PMKCONF_HW_X86_CPU_CLASS	"HW_X86_CPU_CLASS"	/* ex: i386, i486, i586, etc ... */

#define X86_CPU_CLASS_FAMILY_FMT	"%s_FAM%d"		/* standard vendor, family */
#define X86_CPU_CLASS_EXTFAM_FMT	"%s_EFAM%d"		/* standard vendor, extended family */


typedef struct {
	bool		 cpuid;
	char		*vendor,
			*cpuname,
			*stdvendor,
			*features;
	unsigned char	 family,
			 model,
			 extfam,
			 extmod;
	uint32_t	 level;
} x86_cpu_cell;

typedef struct {
	uint32_t	 mask;
	char		*descr;
} x86_cpu_feature;

x86_cpu_cell	*x86_cpu_cell_init(void);
void		 x86_cpu_cell_destroy(x86_cpu_cell *);
char		*x86_get_std_cpu_vendor(prsdata *, char *);
bool		 x86_get_cpuid_data(x86_cpu_cell *);
bool		 x86_set_cpu_data(prsdata *, x86_cpu_cell *, htable *);

#endif /* ARCH_X86_32 || ARCH_X86_64 */


/******************
 * alpha specific *
 ***********************************************************************/

#if defined(ARCH_ALPHA)

#define ALPHA_IMPLVER_EV4	0
#define ALPHA_IMPLVER_EV5	1
#define ALPHA_IMPLVER_EV6	2

#define ALPHA_CPU_CLASS_FMT	"IMPLVER_%u"
#define ALPHA_CPU_UNKNOWN	"unknown"

#define ALPHA_CPU_MASK_FEAT_BWX		0x00000001	/* bit  0 */
#define ALPHA_CPU_MASK_FEAT_FIX		0x00000002	/* bit  1 */
#define ALPHA_CPU_MASK_FEAT_CIX		0x00000004	/* bit  2 */
#define ALPHA_CPU_MASK_FEAT_MVI		0x00000100	/* bit  8 */
#define ALPHA_CPU_MASK_FEAT_PAT		0x00000200	/* bit  9 */
#define ALPHA_CPU_MASK_FEAT_PMI		0x00001000	/* bit 12 */


#define PMKCONF_HW_ALPHA_CPU_CLASS	"HW_ALPHA_CPU_CLASS"	/* ex: EV4, EV5, EV6 */
#define PMKCONF_HW_ALPHA_CPU_FEATURES	"HW_ALPHA_CPU_FEATURES"	/* BWX, FIX, CIX, MVI, PAT, PMI */


typedef struct {
	uint64_t	 mask;
	char		*descr;
} alpha_cpu_feature;


bool alpha_set_cpu_data(prsdata *, htable *);

#endif /* ARCH_ALPHA */


/******************
 * ia64 specific *
 ***********************************************************************/

#if defined(ARCH_IA64)

/* register masks */
#define IA64_CPU_MASK_LEVEL	0x000000000ff
#define IA64_CPU_MASK_REVISION	0x0000000ff00
#define IA64_CPU_MASK_MODEL	0x00000ff0000
#define IA64_CPU_MASK_FAMILY	0x000ff000000
#define IA64_CPU_MASK_ARCHREV	0x0ff00000000

/* feature register */
#define IA64_CPU_MASK_FEAT_LB	0x0000000000000001	/* bit  0 */
#define IA64_CPU_MASK_FEAT_SD	0x0000000000000002	/* bit  1 */
#define IA64_CPU_MASK_FEAT_AO	0x0000000000000004	/* bit  2 */


#define PMKCONF_HW_IA64_CPU_REVISION	"HW_IA64_CPU_REVISION"	/* revision */
#define PMKCONF_HW_IA64_CPU_MODEL	"HW_IA64_CPU_MODEL"	/* model */
#define PMKCONF_HW_IA64_CPU_FAMILY	"HW_IA64_CPU_FAMILY"	/* family */
#define PMKCONF_HW_IA64_CPU_ARCHREV	"HW_IA64_CPU_ARCHREV"	/* architecture revision */
#define PMKCONF_HW_IA64_CPU_FEATURES	"HW_IA64_CPU_FEATURES"	/* features */
#define PMKCONF_HW_IA64_CPU_VENDOR	"HW_IA64_CPU_VENDOR"	/* vendor name */
#define PMKCONF_HW_IA64_CPU_CLASS	"HW_IA64_CPU_CLASS"	/* cpu class */


#define IA64_CPU_CLASS_FAMILY_FMT	"FAM%d"		/* family */


typedef struct {
	uint64_t	 mask;
	char		*descr;
} ia64_cpu_feature;


bool ia64_get_cpuid_data(prsdata *, htable *);

#endif /* ARCH_IA64 */

#endif /* _DETECT_CPU_H_ */

