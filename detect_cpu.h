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
#include "parse.h"

#define CPU_ARCH_ADD		1
#define LIST_X86_CPU_VENDOR	2
#define LIST_X86_CPU_MODEL	3


/* x86 specific */
#ifdef ARCH_X86
#define MASK_X86_CPU_EXTFAM	0x0ff00000
#define MASK_X86_CPU_EXTMOD	0x000f0000
#define MASK_X86_CPU_TYPE	0x0000f000
#define MASK_X86_CPU_FAMILY	0x00000f00
#define MASK_X86_CPU_MODEL	0x000000f0
#endif /* ARCH_X86 */


#ifdef ARCH_X86
typedef struct {
	bool		cpuid;
	char		vendor[13],
			cpuname[49];
	unsigned char	pmkfam,
			pmkmod,
			family,
			model,
			extfam,
			extmod;
	uint32_t	level;
} x86_cpu_cell;
#endif


prsdata	*parse_cpu_data(char *);
char	*check_cpu_arch(char *, prsdata *);

#ifdef ARCH_X86
char	*x86_get_std_cpu_vendor(prsdata *, char *);
bool	 x86_get_cpuid_data(x86_cpu_cell *);
#endif /* ARCH_X86 */

#endif /* _DETECT_CPU_H_ */

