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


#ifndef _DETECT_CPU_H_
#define _DETECT_CPU_H_

#include <inttypes.h>
#include "cpu_arch_def.h"
#include "hash.h"
#include "parse.h"


/***************
 * common code *
 ***************/


#define LIST_ARCH_EQUIV		1
#define LIST_X86_CPU_VENDOR	2
#define LIST_X86_CPU_MODEL	3
#define LIST_X86_CPU_CLASS	4

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

#define PMKCONF_HW_PPC_CPU_ALTIVEC	"HW_PPC_CPU_ALTIVEC"	/* powerpc altivec feature */


typedef struct {
	char		*name;
	unsigned char	 id;
} arch_cell;


void	*seek_key(prsdata *, int);
prsdata	*parse_cpu_data(char *);
char	*check_cpu_arch(char *, prsdata *);
htable	*arch_wrapper(prsdata *, char *);


/****************
 * x86 specific *
 ****************/

#ifdef ARCH_X86

#define X86_CPU_MASK_EXTFAM	0x0ff00000
#define X86_CPU_MASK_EXTMOD	0x000f0000
#define X86_CPU_MASK_TYPE	0x0000f000
#define X86_CPU_MASK_FAMILY	0x00000f00
#define X86_CPU_MASK_MODEL	0x000000f0

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
			*stdvendor;
	unsigned char	 family,
			 model,
			 extfam,
			 extmod;
	uint32_t	 level;
} x86_cpu_cell;

x86_cpu_cell	*x86_cpu_cell_init(void);
void		 x86_cpu_cell_destroy(x86_cpu_cell *);
char		*x86_get_std_cpu_vendor(prsdata *, char *);
bool		 x86_get_cpuid_data(x86_cpu_cell *);
bool		 x86_set_cpu_data(prsdata *, x86_cpu_cell *, htable *);
#endif /* ARCH_X86 */

#endif /* _DETECT_CPU_H_ */

