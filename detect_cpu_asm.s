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


#include "cpu_arch_def.h"

#ifdef ARCH_X86_32

.text

/*
 *  X86 32 bits code
 */


    .align  16,0x90


/*
    check if cpuid is available
*/

    .globl  x86_check_cpuid_flag

x86_check_cpuid_flag:
	/* save ecx register in the stack */
	pushl	%ecx

	/* push flags into stack */
	pushfl

	/* pop EFLAGS register from stack into eax */
	popl	%eax

	/* save original EFLAGS into ecx */
	movl	%eax,%ecx

	/* flip CPUID flag (bit 21 of EFLAGS) for testing purpose */
	xorl	$0x200000,%eax

	/* put back modified EFLAGS into stack */
	pushl	%eax

	/* reload flags from stack */
	popfl

	/* put flags again into stack to check it */
	pushfl

	/* pop EFLAGS register from stack into eax */
	popl	%eax

	/* push the original EFLAGS value stored in ecx into stack */
	pushl	%ecx

	/* put original state back from the stack */
	popfl

	/* xor between original EFLAGS saved in ecx and current in eax */
	xorl	%ecx,%eax

	/* keep only CPUID flag */
	andl	$0x200000,%eax

	/* and shift it to bit 0 */
	rorl	$21,%eax

	/* restore ecx register that was previously stored in stack */
	popl	%ecx

	/* return value of CPUID in bit 0 of eax */
	ret

/*
    exec cpuid function

    returns: pointer to static buffer
*/

    .globl x86_exec_cpuid

x86_exec_cpuid:
    /* get function number */
    movl    4(%esp),%eax

    pushl   %ebx
    pushl   %ecx
    pushl   %edx

    cpuid

    /* copy register values */
    movl    %eax,x86_cpu_reg_eax
    movl    %ebx,x86_cpu_reg_ebx
    movl    %ecx,x86_cpu_reg_ecx
    movl    %edx,x86_cpu_reg_edx

    popl    %edx
    popl    %ecx
    popl    %ebx

    ret


#endif /* ARCH_X86_32 */

#ifdef ARCH_X86_64

.text

/*
 *  X86 64 bits code
 */


    .align  16,0x90


/*
    check if cpuid is available
*/

    .globl  x86_check_cpuid_flag

x86_check_cpuid_flag:
	pushq	%rcx        /* save rcx register */

	pushfq
	popq    %rax        /* get eflags */

	movq    %rax,%rcx   /* save flags */

	xorq    $0x200000,%rax  /* clear CPU ID flag */

	pushq   %rax
	popfq           /* load eflags */

	pushfq
	popq    %rax        /* get current eflags state */

	pushq   %rcx
	popfq           /* put original state back */

	popq    %rcx        /* restore ebx register */

	andq    $0x200000,%rax  /* keep CPU ID  flag only */
	rorq    $21,%rax    /* and shift it to bit 0 */

	ret

/*
    exec cpuid function

    returns: pointer to static buffer
*/

    .globl x86_exec_cpuid

x86_exec_cpuid:
    /* get function number (arg0 in rdi register) */
    movq    %rdi,%rax

    pushq   %rbx
    pushq   %rcx
    pushq   %rdx

    cpuid

    /* copy register values */
    movl    %eax,x86_cpu_reg_eax
    movl    %ebx,x86_cpu_reg_ebx
    movl    %ecx,x86_cpu_reg_ecx
    movl    %edx,x86_cpu_reg_edx

    popq    %rdx
    popq    %rcx
    popq    %rbx

    ret


#endif /* ARCH_X86_64 */


#if defined(ARCH_X86_32) || defined(ARCH_X86_64)

.data

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

#endif /* ARCH_X86_32 || ARCH_X86_64 */


/*
 *  ALPHA code
 */

#if defined(ARCH_ALPHA)
.text
        .globl alpha_exec_implver
        .ent alpha_exec_implver
alpha_exec_implver:
        .frame $30,0,$26,0
$alpha_exec_implver..ng:
        .prologue 0
        implver $0      /* get implver value */
        ret $31,($26),1
        .end alpha_exec_implver


        .globl alpha_exec_amask
        .ent alpha_exec_amask
alpha_exec_amask:
        .frame $30,0,$26,0
$alpha_exec_amask..ng:
        .prologue 0
        lda $0,-1       /* set all bits to 1 */
        amask $0,$0     /* get amask */
        ret $31,($26),1
        .end alpha_exec_amask

#endif /* ARCH_ALPHA */

/*
 *  IA64 code
 */

#if defined(ARCH_IA64)
.text

    .globl ia64_get_cpuid_register
    .proc ia64_get_cpuid_register
ia64_get_cpuid_register:
    .prologue
    .body
    mov r8 = cpuid[r32] /* return cpuid register value in r8 */
    br.ret.sptk.many b0
    .endp ia64_get_cpuid_register

#endif /* ARCH_IA64 */

/* vim: set noexpandtab tabstop=4 softtabstop=4 shiftwidth=4: */
