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


#ifndef _DETECT_CPU_ASM_H_
#define _DETECT_CPU_ASM_H_

#include <inttypes.h>
#include "cpu_arch_def.h"


/*
	x86 architecture
*/

#if defined(ARCH_X86_32) || defined(ARCH_X86_64)

/* declare X86 assembly variables */
extern uint32_t	x86_cpu_reg_eax;
extern uint32_t	x86_cpu_reg_ebx;
extern uint32_t	x86_cpu_reg_ecx;
extern uint32_t	x86_cpu_reg_edx;

/* declare X86 assembly functions */
extern uint32_t	 x86_check_cpuid_flag(void);
extern void	 x86_exec_cpuid(uint32_t); /* cpuid function */

#endif /* ARCH_X86_32 || ARCH_X86_64 */

/*
	alpha architecture
*/

#if defined(ARCH_ALPHA)

uint64_t	alpha_exec_implver(void);
uint64_t	alpha_exec_amask(void);

#endif /* ARCH_ALPHA */

/*
	ia64 architecture
*/

#if defined(ARCH_IA64)

uint64_t	ia64_get_cpuid_register(uint64_t); /* cpuid register */

#endif /* ARCH_IA64 */


#endif /* _DETECT_CPU_ASM_H_ */

