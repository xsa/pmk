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


#include "cpu_arch_def.h"


.text

#ifdef ARCH_X86

	.align	16,0x90


/*
	check if cpuid is available
*/

	.globl	x86_check_cpuid_flag

x86_check_cpuid_flag:
	pushl	%ecx		/* save ebx register */

	pushfl
	popl	%eax		/* get eflags */

	movl	%eax,%ecx	/* save flags */

	xorl	$0x200000,%eax	/* clear CPU ID flag */

	pushl	%eax
	popfl			/* load eflags */

	pushfl
	popl	%eax		/* get current eflags state */

	pushl	%ecx
	popfl			/* put original state back */

	popl	%ecx		/* restore ebx register */

	andl	$0x200000,%eax	/* keep CPU ID  flag only */
	rorl	$21,%eax	/* and shift it to bit 0 */

	ret

/*
	exec cpuid function

	returns: pointer to static buffer
*/

	.globl x86_exec_cpuid

x86_exec_cpuid:
	/* get function number */
	movl	4(%esp),%eax

	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	cpuid

	/* copy register values */
	movl	%eax,x86_cpu_reg_eax
	movl	%ebx,x86_cpu_reg_ebx
	movl	%ecx,x86_cpu_reg_ecx
	movl	%edx,x86_cpu_reg_edx

	popl	%edx
	popl	%ecx
	popl	%ebx

	ret


#endif


.data

#ifdef ARCH_X86

	.globl x86_cpu_reg_eax
x86_cpu_reg_eax:
	.long 0

	.globl x86_cpu_reg_ebx
x86_cpu_reg_ebx:
	.long 0

	.globl x86_cpu_reg_ecx
x86_cpu_reg_ecx:
	.long 0

	.globl x86_cpu_reg_edx
x86_cpu_reg_edx:
	.long 0

#endif


